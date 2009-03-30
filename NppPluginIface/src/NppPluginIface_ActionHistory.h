/* NppPluginIface_ActionHistory.h
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
 *  Notepad++ Plugin Interface Lib extension providing history action storage indexed to the
 *  current Scintilla action count for a document.
 *
 *  This IS NOT a replacement for Scintilla's Undo/Redo handlers, it is meant as only an
 *  indexer for plugins to store and retrieve internal action changes that need to be matched
 *  up to a documents change history.  It is up to the plugin to update both the history
 *  and Scintilla.
 *  
 *  For an example of using this see the NppPlugin_ChangeMarker plugin sources.
 *
 */

#ifndef NPP_PLUGININTERFACE_ACTIONHISTORY_EXTENSION_H
#define NPP_PLUGININTERFACE_ACTIONHISTORY_EXTENSION_H

#include "NppPluginIface.h"
#include "NppPluginIface_ActionIndex.h"

//  <--- Boost ( used for history action tracking ) --->
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace npp_plugin {

//  Namespace for action history tracking;  everything is defined publicly to give plugins
//  maximum access to the action history set.
namespace actionhistory{

using boost::multi_index::multi_index_container;
using namespace boost::multi_index;

//  Action object for a plugin to use and pass to the DocumentActionHistory insert function.
struct HistoryAction {
	int type;
	int id;
	int handle;
	int preState;
	int postState;
	int posStart;
	int posEnd;
	bool isSaved;

	HistoryAction(int type):type(type), id(-1), handle(0), preState(0), postState(0),
		posStart(-1), posEnd(-1), isSaved(false){};
};

//  Action history class for objects meant to be inserted into the HistoryAction_set.
struct ActionHistory {
	int _index;
	int _entry;
	int _referenceIndex;

	int type;
	int id;
	int handle;
	int preState;
	int postState;
	int posStart;
	int posEnd;
	bool isSaved;

	ActionHistory(int index, int entry, int referenceIndex, int type, int id, int handle, int preState,
		int postState, int posStart, int posEnd, bool isSaved )
		:_index(index), _entry(entry), _referenceIndex(referenceIndex),
		type(type), id(id), handle(handle), preState(preState), postState(postState),
		posStart(posStart), posEnd(posEnd), isSaved(isSaved){};

	bool operator<(const ActionHistory& ah)const{ return ( ( _index < ah._index ) && ( _entry < ah._entry ) ); };
};

//  ActionHistory_set tags.
struct ah_index_key{};
struct ah_reference_key{};
struct ah_typeAndId_key{};
struct ah_type_key{};
struct ah_id_key;
struct ah_handle_key{};
struct ah_saved_key{};

// ActionHistory_set composite key for a unique id.
struct comp_actionUID_key:composite_key<
	ActionHistory,
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _index),
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _entry)
>{};

struct comp_typeId_key:composite_key<
	ActionHistory,
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, type),
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, id)
>{};

/* Multi Index for Action History tracking */
typedef multi_index_container<
	ActionHistory,
	indexed_by<
		ordered_unique< comp_actionUID_key >,
		ordered_non_unique<	tag<ah_index_key>, member<ActionHistory, int, &ActionHistory::_index > >,
		ordered_non_unique<	tag<ah_reference_key>, member<ActionHistory, int, &ActionHistory::_referenceIndex >	>,
		ordered_non_unique< tag<ah_typeAndId_key>, comp_typeId_key >,
		ordered_non_unique< tag<ah_type_key>, member<ActionHistory, int, &ActionHistory::type > >,
		ordered_non_unique< tag<ah_id_key>, member<ActionHistory, int, &ActionHistory::id > >,
		ordered_non_unique< tag<ah_handle_key>, member<ActionHistory, int, &ActionHistory::handle > >,
		ordered_non_unique< tag<ah_saved_key>, member<ActionHistory, bool, &ActionHistory::isSaved > >
	>
> ActionHistory_set;

//  Action History set index types.
typedef ActionHistory_set::nth_index<0>::type ahs_by_uid;
typedef ActionHistory_set::index<ah_index_key>::type ahs_by_action;
typedef ActionHistory_set::index<ah_reference_key>::type ahs_by_reference;
typedef ActionHistory_set::index<ah_typeAndId_key>::type ahs_by_typeAndId;
typedef ActionHistory_set::index<ah_type_key>::type ahs_by_type;
typedef ActionHistory_set::index<ah_id_key>::type ahs_by_id;
typedef ActionHistory_set::index<ah_handle_key>::type ahs_by_handle;
typedef ActionHistory_set::index<ah_saved_key>::type ahs_by_saved;

//  Index iterator types.
typedef ahs_by_uid::iterator uid_iter;
typedef ahs_by_action::iterator action_iter;
typedef ahs_by_reference::iterator reference_iter;
typedef ahs_by_typeAndId::iterator typeAndId_iter;
typedef ahs_by_type::iterator type_iter;
typedef ahs_by_id::iterator id_iter;
typedef ahs_by_handle::iterator handle_iter;
typedef ahs_by_saved::iterator saved_iter;

//  This plugins action history set.
class DocumentActionHistory {
		int _currDoc;
		int _prevActionIndex;
		int _actionEntryID;

	public:
		ActionHistory_set ahs;

		//  Indexes < see the constructor >
		ahs_by_action& action_index;
		ahs_by_reference& reference_index;
		ahs_by_typeAndId& typeAndId_index;
		ahs_by_type& type_index;
		ahs_by_id& id_index;
		ahs_by_handle& handle_index;
		ahs_by_saved& saved_index;

		//  Iterators
		action_iter a_iter;
		reference_iter r_iter;
		typeAndId_iter typeId_iter;
		type_iter t_iter;
		id_iter i_iter;
		handle_iter h_iter;
		saved_iter s_iter;

		// Contructor sets the indexes.
		DocumentActionHistory(int pDoc)
			:_currDoc(pDoc),
			_prevActionIndex(0), _actionEntryID(0),
			action_index( ahs.get<ah_index_key>() ),
			reference_index( ahs.get<ah_reference_key>() ),
			typeAndId_index( ahs.get<ah_typeAndId_key>() ),
			type_index( ahs.get<ah_type_key>() ),
			id_index( ahs.get<ah_id_key>() ),
			handle_index( ahs.get<ah_handle_key>() ),
			saved_index( ahs.get<ah_saved_key>() )
		{};

		bool setActionIndex( int actionIndex );
		bool setReferenceIndex( int reference );
		bool set_compTypeAndIDIndex( int type, int id );
		bool setTypeIndex( int type );
		bool setIdIndex( int id );
		bool setHandleIndex( int handle );
		bool setSavedIndex( bool saved );
		bool insert_at_CurrActionIndex( HistoryAction* action, int referenceIndex );
		bool insert_at_NextActionIndex( HistoryAction* action, int referenceIndex );
		std::pair<ActionHistory_set::iterator, bool> insertAction( int actionIndex, int actionEntryID,
			int referenceIndex, HistoryAction* action );
		void truncateActions();
		void truncateActionsAtNextIndex();
	};

} // End: namespace action_history

} // End: namespace npp_plugin

#endif //  End include guard: NPP_PLUGININTERFACE_ACTIONHISTORY_EXTENSION_H