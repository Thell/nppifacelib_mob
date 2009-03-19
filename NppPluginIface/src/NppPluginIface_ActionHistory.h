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


//  Action history class for objects meant to be inserted into the HistoryAction_set.
struct ActionHistory {
	//  Scintilla Document ID.
	int _pDoc;
	/* The _pDoc, _actionIndex, _actionEntryID create a unique identifier for an action item. */
	//  The Scintilla action count for the document when the action took place.
	int _actionIndex;
	//  The entry id of an action at this action index.
	int _actionEntryID;
	//  Action identifier being stored.
	int _action;			
	//  An internal handle to be used by the plugin.
	int _actionHandle;		
	// Used to give an action the ability to reference a different action index.  Useful for
	// allowing a single actionHandle to be re-used in later actions, if the actionHandle is
	// removed/altered a future undo action can restore based on this value.
	int _referenceIndex;	
	//  Tracks the action count of a document when it was saved to work with UNDO/REDO.
	bool _isSavePoint;

	//  Action history object meant to be inserted into the HistoryAction_set.
	ActionHistory(int pDoc, int actionIndex, int actionEntryID, int action, int actionHandle,
		int referenceIndex, bool isSavePoint)
		:_pDoc(pDoc), _actionIndex(actionIndex), _actionEntryID(actionEntryID), _action(action),
		_actionHandle(actionHandle), _referenceIndex(referenceIndex), _isSavePoint(isSavePoint){};

	bool operator<(const ActionHistory& ah)const{ return ( _actionIndex < ah._actionIndex); };
};

// ActionHistory_set composite key for a unique id.
struct comp_actionUID_key:composite_key<
	ActionHistory,
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _pDoc),
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _actionIndex),
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _actionEntryID)
>{};

// ActionHistory_set tag.
struct ahIndex_key{};

// ActionHistory_set composite key.
struct comp_docindex_key:composite_key<
	ActionHistory,
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _pDoc),
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _actionIndex)
>{};


// ActionHistory_set tag.
struct ahSavepoint_key{}; /* Index Tag */

// ActionHistory_set composite key.
struct comp_savepoint_key:composite_key<
	ActionHistory,
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _pDoc),
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, bool, _isSavePoint)
>{};

// ActionHistory_set tag.
struct ahReference_key{};

// ActionHistory_set composite key
struct comp_reference_key:composite_key<
	ActionHistory,
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _pDoc),
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _referenceIndex)
>{};

// ActionHistory_set tag.
struct ahHandle_key{};

// ActionHistory_set composite key.
struct comp_handle_key:composite_key<
	ActionHistory,
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _pDoc),
	BOOST_MULTI_INDEX_MEMBER(ActionHistory, int, _actionHandle)
>{};


/* Multi Index for Marker_History tracking */
typedef multi_index_container<
	ActionHistory,
	indexed_by<
		ordered_unique< comp_actionUID_key >,
		ordered_non_unique< tag<ahIndex_key>, comp_docindex_key >,
		ordered_non_unique< tag<ahSavepoint_key>, comp_savepoint_key >,
		ordered_non_unique< tag<ahReference_key>, comp_reference_key >,
		ordered_non_unique< tag<ahHandle_key>, comp_handle_key >
	>
> ActionHistory_set;

//  Action History set index types.
typedef ActionHistory_set::nth_index<1>::type ahs_by_docAction;
typedef ActionHistory_set::nth_index<2>::type ahs_by_docSavePoint;
typedef ActionHistory_set::nth_index<3>::type ahs_by_docReference;
typedef ActionHistory_set::nth_index<4>::type ahs_by_docHandle;

//  Index iterator types.
typedef ahs_by_docAction::iterator docAction_iter;
typedef ahs_by_docReference::iterator docReference_iter;
typedef ahs_by_docSavePoint::iterator docSavePoint_iter;
typedef ahs_by_docHandle::iterator docHandle_iter;

//  This plugins action history set.
class PluginActionHistory {

		int _currDoc;
		int _currActionIndex;
		int _currReferenceIndex;
		int _currHandleIndex;

		int _prevActionIndex;
		int _actionEntryID;


	public:
		ActionHistory_set ahs;

		ahs_by_docAction& action_index;			// = ahs.get<ahIndex_key>();
		ahs_by_docSavePoint& savepoint_index;	// = ahs.get<ahSavepoint_key>();
		ahs_by_docReference& reference_index;	// = ahs.get<ahReference_key>();
		ahs_by_docHandle& handle_index;			// = ahs.get<ahHandle_key>();

		docAction_iter a_iter;
		docReference_iter r_iter;
		docSavePoint_iter s_iter;
		docHandle_iter h_iter;

		PluginActionHistory()
			:_currDoc(0), _currActionIndex(0), _currReferenceIndex(0), _currHandleIndex(0),
			_prevActionIndex(0), _actionEntryID(0),
			action_index( ahs.get<ahIndex_key>() ),
			savepoint_index( ahs.get<ahSavepoint_key>() ),
			reference_index( ahs.get<ahReference_key>() ),
			handle_index( ahs.get<ahHandle_key>() )
		{};

		bool setTargetIndex( int pDoc, int actionIndex );
		bool setReferenceIndex( int refIndex );
		bool setHandleIndex( int handleIndex );
		bool insertAction_at_CurrIndex( int action, int actionhandle, int referenceIndex, bool isSavePoint = false );
		bool insertAction_at_NextIndex( int action, int actionHandle, int referenceIndex, bool isSavePoint );
		std::pair<ActionHistory_set::iterator, bool> insertAction( int pDoc, int actionIndex, int actionEntryID,
			int action, int actionHandle, int referenceIndex, bool isSavePoint );
		void truncateActions();
	};

} // End: namespace action_history

} // End: namespace npp_plugin

#endif //  End include guard: NPP_PLUGININTERFACE_ACTIONHISTORY_EXTENSION_H