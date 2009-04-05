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
bool DocumentActionHistory::setActionIndex( int actionIndex )
{
	a_iter = action_index.find( actionIndex );

	if ( a_iter != action_index.end() ) {
		return ( true );
	}

	return ( false );
}

//  Sets the current reference index r_iter uses on reference_index.
bool DocumentActionHistory::setReferenceIndex( int reference )
{
	r_iter = reference_index.find( reference );

	if ( r_iter != reference_index.end() ) {
		return ( true );
	}

	return ( false );
}

//  Sets the current composite key index for type and id, as well as both individual type and
//  id target indexes.
bool DocumentActionHistory::set_compTypeAndIDIndex( int type, int id )
{
	typeId_iter = typeAndId_index.find( boost::make_tuple( type, id ) );

	if ( typeId_iter != typeAndId_index.end() ) {
		return ( true );
	}

	return ( false );
}

//  Sets the current action type index that t_iter uses on type_index.
bool DocumentActionHistory::setTypeIndex( int type )
{
	t_iter = type_index.find( type );

	if ( t_iter != type_index.end() ) {
		return ( true );
	}

	return ( false );
}

//  Sets the current action id index that i_iter uses on id_index.
bool DocumentActionHistory::setIdIndex( int id )
{
	i_iter = id_index.find( id );

	if ( i_iter != id_index.end() ) {
		return ( true );
	}

	return ( false );
}

//  Sets the current action handle index that h_iter uses on handle_index.
bool DocumentActionHistory::setHandleIndex( int handle )
{
	h_iter = handle_index.find( handle );

	if ( h_iter != handle_index.end() ) {
		return ( true );
	}

	return ( false );
}

//  Sets the current saved action index that s_iter uses on saved_index.
bool DocumentActionHistory::setSavedIndex( bool saved )
{
	s_iter = saved_index.find( saved );

	if ( s_iter != saved_index.end() ) {
		return ( true );
	}

	return ( false );
}

//  Adds an action item to history set for the current targeted Scintilla Document at the
//  current Scintilla action index.  This does not allow insertion at an earlier index.
bool DocumentActionHistory::insert_at_CurrActionIndex( HistoryAction* action, int referenceIndex )
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
		insertAction( actionIndex, _actionEntryID, referenceIndex, action );


	if (! retVal.second ) {
		ActionHistory dbgAH = *(retVal.first);
		bool dbgStop = true;
	}

	return ( retVal.second );
}

//  Adds an action item to history set for the current targeted Scintilla Document at the
//  next Scintilla action index.  Useful when working with SC_MOD_BEFORE...  notifications.
bool DocumentActionHistory::insert_at_NextActionIndex( HistoryAction* action, int referenceIndex )
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
		insertAction( actionIndex, _actionEntryID, referenceIndex, action );


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
std::pair<ActionHistory_set::iterator, bool> DocumentActionHistory::insertAction(
	int actionIndex, int actionEntryID,int referenceIndex, HistoryAction* action )
{
	std::pair<ActionHistory_set::iterator, bool> retVal = ahs.insert(
		ActionHistory(
			actionIndex,
			actionEntryID,
			referenceIndex,
			action->type,
			action->id,
			action->handle,
			action->preState,
			action->postState,
			action->posStart,
			action->posEnd,
			action->isSaved
		)
	);

	return ( retVal );
}

//  Removes all actions item from the action history after the current index.
void DocumentActionHistory::truncateActions()
{
	int targetIndex = npp_plugin::actionindex::getCurrActionIndex( _currDoc );

	a_iter = action_index.find( targetIndex );
	
	if ( a_iter != action_index.end() ) {
		action_index.erase( a_iter, action_index.end() );
		_prevActionIndex = -1;
		_actionEntryID = 0;
	}
	else {
		bool dbgStop = true;
	}
}

//  Removes all actions item from the marker history at the next index.
void DocumentActionHistory::truncateActionsAtNextIndex()
{
	int targetIndex = npp_plugin::actionindex::getCurrActionIndex( _currDoc );

	a_iter = action_index.find( targetIndex + 1 );
	
	if ( a_iter != action_index.end() ) {
		action_index.erase( a_iter, action_index.end() );
		_prevActionIndex = -1;
		_actionEntryID = 0;
	}
	else {
		bool dbgStop = true;
	}
}


} // End: namespace action_history

} // End: namespace npp_plugin
