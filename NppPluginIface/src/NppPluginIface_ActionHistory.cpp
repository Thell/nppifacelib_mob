/* NppPluginIface_ActionHistory.cpp
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

#include "NppPluginIface_ActionHistory.h"

namespace npp_plugin {

namespace actionhistory{

//  Sets the current target doc and index values that the action_iter uses.
//  Returns true if actions already exists at the index.
bool PluginActionHistory::setTargetIndex( int pDoc, int actionIndex )
{
	_currDoc = pDoc;
	_currActionIndex = actionIndex;

	// Assign the iterator.
	a_iter = action_index.find( boost::make_tuple( _currDoc, _currActionIndex ) );

	if ( a_iter != action_index.end() ) {
		return ( true );
	}

	return ( false );
}

//  Sets the current reference index r_iter uses on reference_index.
//  Returns true if actions already exist at the index.
bool PluginActionHistory::setReferenceIndex( int refIndex )
{
	_currReferenceIndex = refIndex;

	//  Update the iterator.
	r_iter = reference_index.find( boost::make_tuple( _currDoc, _currReferenceIndex ) );

	if ( r_iter != reference_index.end() ) {
		return ( true );
	}

	return ( false );
}

//  Sets the current action handle index that h_iter uses on handle_index.
bool PluginActionHistory::setHandleIndex( int handleIndex )
{
	_currHandleIndex = handleIndex;

	//  Update the iterator.
	h_iter = handle_index.find( boost::make_tuple( _currDoc, _currHandleIndex ) );

	if ( h_iter != handle_index.end() ) {
		return ( true );
	}

	return ( false );
}

//  Adds an action item to history set for the current targeted Scintilla Document at the
//  current Scintilla action index.  This does not allow insertion at an earlier index.
bool PluginActionHistory::insertAction_at_CurrIndex( int action, int actionHandle, int referenceIndex, bool isSavePoint )
{
	int actionIndex = npp_plugin::actionindex::getCurrActionIndex( _currDoc );
	if ( actionIndex == _prevActionIndex ) {
		_actionEntryID++;
	}
	else {
		_prevActionIndex = actionIndex;
		_actionEntryID = 0;
	}

	std::pair<ActionHistory_set::iterator, bool> retVal = 
		insertAction( _currDoc, actionIndex, _actionEntryID, action, actionHandle, referenceIndex, isSavePoint );


	if (! retVal.second ) {
		ActionHistory dbgAH = *(retVal.first);
		bool dbgStop = true;
	}

	return ( retVal.second );
}

//  Adds an action item to history set for the current targeted Scintilla Document at the
//  next Scintilla action index.  Useful when working with SC_MOD_BEFORE...  notifications.
bool PluginActionHistory::insertAction_at_NextIndex( int action, int actionHandle, int referenceIndex, bool isSavePoint )
{
	int actionIndex = ( npp_plugin::actionindex::getCurrActionIndex( _currDoc ) + 1 );
	if ( actionIndex == _prevActionIndex ) {
		_actionEntryID++;
	}
	else {
		_prevActionIndex = actionIndex;
		_actionEntryID = 0;
	}

	std::pair<ActionHistory_set::iterator, bool> retVal =
		insertAction( _currDoc, actionIndex, _actionEntryID, action, actionHandle, referenceIndex, isSavePoint );


	if (! retVal.second ) {
		ActionHistory dbgAH = *(retVal.first);
		bool dbgStop = true;
	}

	return ( retVal.second );
}

//  Adds an action item to history set.  It is recommended to use the insertAction_At_currIndex
//  and insertAction_At_NextIndex instead of using this directly.
//  This allows insertion at any index, so be careful.
//  std::pair<ActionHistory_set::iterator, bool> is returned.  If bool is false insertion failed
//  at the ActionHistory returned, else the iterator points to the inserted iterator.
std::pair<ActionHistory_set::iterator, bool> PluginActionHistory::insertAction( int pDoc, int actionIndex, int actionEntryID, int action,
									   int actionHandle, int referenceIndex, bool isSavePoint )
{
	std::pair<ActionHistory_set::iterator, bool> retVal = ahs.insert(
		ActionHistory( pDoc, actionIndex, actionEntryID, action, actionHandle, referenceIndex, isSavePoint )
	);

	return ( retVal );
}

//  Removes all actions item from the marker history after the current index.
void PluginActionHistory::truncateActions()
{
	int targetIndex = npp_plugin::actionindex::getCurrActionIndex( _currDoc );

	a_iter = action_index.find( boost::make_tuple( _currDoc, targetIndex ) );
	
	if ( a_iter != action_index.end() ) {
		action_index.erase( a_iter, action_index.end() );
	}
	else {
		bool dbgStop = true;
	}
}

} // End: namespace action_history

} // End: namespace npp_plugin
