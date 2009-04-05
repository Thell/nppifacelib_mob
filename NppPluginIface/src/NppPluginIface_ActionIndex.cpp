/* NppPluginIface_ActionIndex.cpp
 *
 * This file is part of the Notepad++ Plugin Interface Lib.
 * Copyright 2008 - 2009 Thell Fowler (thell@almostautomated.com)
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program;
 * if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 *  Notepad++ Plugin Interface Lib extension providing tracking of actions that are indexed
 *  to the same count as Scintilla currAction counter for a document.
 *
 *  For an example of using this see the NppPlugin_ChangeMarker plugin sources.
 *
 */

/*
 * From the Scintilla Documentation:
 *   A Scintilla window and the document that it displays are separate entities.  When you
 *   create a new window, you also create a new, empty document. Each document has a reference
 *   count that is initially set to 1. The document also has a list of the Scintilla windows
 *   that are linked to it so when any window changes the document, all other windows in which
 *   it appears are notified to cause them to update.
 *
 * When tracking action counts for a Document the notifications received from the extra
 * references must be discarded or the count is thrown out of synch with Scintilla's internal
 * action counter.
 *
 * This source filters out the extra messages by comparing where the notification
 * originated and determing if the notification should be handled by a particular view.
 *
 */


#include "NppPluginIface_ActionIndex.h"
#include "NppPluginIface_DocTabMap.h"

//  <--- TR1 --->
#include <unordered_map>

//  <--- Notepad++ Scintilla Components for BufferID to pDoc --->
#define TIXMLA_USE_STL
#include "Buffer.h"

namespace npp_plugin {

namespace actionindex {

//  Un-named namespace for private classes, variables, and functions.
namespace {

	//  A few namespace scope globals for convience.
	const bool PROCESS = false;
	const bool EXCLUDE = true;

	//  Container for storing action counts
	std::tr1::unordered_map< int, int > _ActionIndex;


	//  Determine if SCN_MODIFIED notification should be excluded from action count processing.
	bool doExclude ( SCNotification* scn )
	{
		bool retVal =  EXCLUDE;
		//  Current view setup.
		HWND hView = reinterpret_cast<HWND>(scn->nmhdr.hwndFrom);
		Document scn_targetDoc = (Document)::SendMessage( hView, SCI_GETDOCPOINTER, 0, 0);
		Document mv_pDoc = (Document)npp_plugin::doctabmap::getVisibleDocId_by_View( MAIN_VIEW );
		Document sv_pDoc = (Document)npp_plugin::doctabmap::getVisibleDocId_by_View( SUB_VIEW );

		//  Main view to main doc notification.
		if ( ( hView == hMainView() ) && ( scn_targetDoc == mv_pDoc ) ) {
			retVal = PROCESS;
		}

		//  Second view to second doc notification, except when cloned from active main view.
		if ( ( hView == hSecondView() ) && ( scn_targetDoc == sv_pDoc ) &&
			( scn_targetDoc != mv_pDoc ) ) {
			retVal = PROCESS;
		}

		//  Hidden view notifications.
		if ( ( hView != hMainView() ) && ( hView != hSecondView() ) ) {
			if ( ( scn_targetDoc != mv_pDoc ) &&
					( scn_targetDoc != sv_pDoc ) ) {
				retVal = PROCESS;
			}
			else retVal = EXCLUDE;
		}

		return ( retVal );
	}

};	//  End un-named namespace


//  Returns a target index for tracking actions, the index increments and decrements in relation
//  with the messages reported via SCNotifications.
//  Be sure to use the returned prevActionIndex value when dealing with UNDO messages.  You are
//  travelling backwards after all.
//  'SC_MOD_BEFORE...' messages process as a 'Dry Run', the action count is returned
//  but not stored, and the following SC_MOD_ message will still be processed as normal.
//  The tuple returned is ( int pDoc, int prevActionIndex, int currActionCount, bool excluded, bool dryrun )
//  If returned pDoc field is NULL the notification was not processed.
boost::tuples::tuple< int, int, int, bool, bool> processSCNotification( SCNotification* scn )
{
	using namespace boost::tuples;

	int modFlags = scn->modificationType;

	if ( (! ( modFlags & ( SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT ) ||
			modFlags & ( SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE ) ) ) &&
			( npp_plugin::isNppReady() ) ) {
		return ( make_tuple( 0, 0, 0, 0, 0 ) );
	}

	//  Get the document pointer.
	HWND hView = reinterpret_cast<HWND>(scn->nmhdr.hwndFrom);
	int pDoc = (LRESULT)::SendMessage( hView, SCI_GETDOCPOINTER, 0, 0);

	//  We always return the existing action index.
	int currIndex = _ActionIndex[pDoc];

	//  Filter the notifications by sender handle and target.
	if ( doExclude( scn ) ) {
		return ( make_tuple( pDoc, currIndex, -1, true, false ) );
	}

	//  Filter extra index change on UNDO/REDO MULTILINE notifications.
	if ( modFlags & ( SC_MULTILINEUNDOREDO ) && 
		(! ( modFlags & ( SC_LASTSTEPINUNDOREDO ) ) ) ) {
		return ( make_tuple( pDoc, currIndex, -1, true, false ) );
	}

	//  <---  NOTIFICATION PROCESSING BEGINS HERE --->

	//  Get a starting action count.
	int newIndex = currIndex;

	//  Allow 'dry-run' of SC_MOD_BEFORE...' messages.
	bool dryrun = false;

	if ( modFlags & ( SC_PERFORMED_USER | SC_PERFORMED_REDO ) ) {
		if ( modFlags & ( SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT ) ) {
			newIndex++;
		}
		else if ( modFlags & ( SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE ) ) {
			newIndex++;
			dryrun = true;
		}
	}

	if ( modFlags & ( SC_PERFORMED_UNDO ) ) {
		if ( modFlags & ( SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT ) ) {
			newIndex--;
		}
		else if ( modFlags & ( SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE ) ) {
			newIndex--;
			dryrun = true;
		}
	}

	//  Store the new action count if this isn't a dry-run.
	if (! dryrun ) _ActionIndex[pDoc] = newIndex;

	return ( make_tuple( pDoc, currIndex, newIndex, false, dryrun ) );
}

//  Returns the current action index value for an identified document.
int getCurrActionIndex( int pDoc ) { return ( _ActionIndex[pDoc] ); }


} // End namespace: actionindex

} // End namespace: npp_plugin

