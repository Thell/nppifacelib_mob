/* NppPlugin_ChangeMarker.cpp
 *
 * This file is part of the Notepad++ Change Marker Plugin.
 * Copyright 2009 Thell Fowler (thell@almostautomated.com)
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

//  Change Marker Plugin for use with Notepad++.

#include "NppPlugin_ChangeMarker.h"
#include <vector>
#include <set>

namespace npp_plugin_changemarker {

using namespace npp_plugin;
namespace xml = npp_plugin::xmlconfig;
namespace mark = npp_plugin::markers;

//  Namespace public static variables.
bool _doDisable = false;
Change_Mark* cm[NB_CHANGEMARKERS];
typedef std::set<int> ChangedDocs_Set;
ChangedDocs_Set _doc_set;
ChangedDocs_Set _doc_disabled_set;
std::tr1::unordered_map<int, ChangedDocument*> _doc_map;

//  Set both markers target margin for, set menu item checks, and save to config file.
void Change_Mark::setTargetMarginMenuItem( MARGIN target )
{
	// Two passes, the first to unset current marked menu item, second to set the new one.
	int menuTarget;
	int UNCHECK = 0;
	int CHECK = 1;
	for ( int i = UNCHECK; i <= CHECK; i++ ) {
		menuTarget = ( i == UNCHECK ) ? ( this->margin.getTarget() ) : ( target );

		int cmdID;
		switch ( menuTarget )
		{
		case MARGIN_CHANGES:
			if ( i == UNCHECK ) {
				::SendMessage( hMainView(), SCI_SETMARGINWIDTHN, MARGIN_CHANGES, 0 );
				::SendMessage( hSecondView(), SCI_SETMARGINWIDTHN, MARGIN_CHANGES, 0 );
			}
			else {
				//  Since this margin is reserved by N++ we can completely exclude other markers from it.
				::SendMessage( hMainView(), SCI_SETMARGINMASKN, MARGIN_CHANGES, 0 );
				::SendMessage( hMainView(), SCI_SETMARGINWIDTHN, MARGIN_CHANGES, 3 );
				::SendMessage( hSecondView(), SCI_SETMARGINMASKN, MARGIN_CHANGES, 0 );
				::SendMessage( hSecondView(), SCI_SETMARGINWIDTHN, MARGIN_CHANGES, 3 );
			}
			cmdID = getCmdId( CMD_CHANGEMARK );
			break;
		case MARGIN_LINENUMBER:
			cmdID = getCmdId( CMD_LINENUMBER );
			break;
		case MARGIN_NONE:
			cmdID = getCmdId( CMD_HIGHLIGHT );
			break;
		}
		::SendMessage( hNpp(), NPPM_SETMENUITEMCHECK, cmdID, i );
	}

	//  Set the new target marker margin.
	this->margin.setTarget( target, this->id );
	//  Write config value to file.
	xml::setGUIConfigValue( TEXT("SciMarkers"), TEXT("margin"), mark::margin2string(target) );

}

//  Override the current marker type id without writing out to the config file to handle cases
//  where the current marker type would not work in the margin.  ie: _PIXMAP in linenumber margin.
void Change_Mark::markerOverride(int markerType)
{
	//  Temp override of the current marker when changing margins.  Does not alter config file.
	this->_tempMarkerOverride = true;
	this->_overRiddenMarkerType = this->type;
	this->type = markerType;
	this->init( this->id );
}

//  Resets the marker type to values prior to being overridden.
void Change_Mark::resetOverride()
{
	this->_tempMarkerOverride = false;	
	this->type = this->_overRiddenMarkerType;
	this->init( this->id );
}

//  Inserts lines into the line map and renames the keys for existing lines below the point of
//  insertion.
void CM_LineMap::insertLines(int startLine, int nb_lines)
{
	//  See if there are any lines below the insertion point that need to be altered.
	if (! ( _lines.lower_bound(startLine) == _lines.end() ) ) {
		line_map  tmpLines;

		//  Copy the original lines that are above the target pos.
		tmpLines.insert(_lines.begin(), _lines.upper_bound( startLine - 1 ) );

		//  Copy the remaining original lines while increasing their line numbers (the key).
		line_map::iterator pos;
		for ( pos = _lines.lower_bound(startLine); pos != _lines.end(); ++pos ) {
			tmpLines[ (pos->first + nb_lines ) ] = pos->second;
		}

		_lines.swap( tmpLines );
	}
}

//  Deletes line from the line map and renames the keys for existing lines below the point of
//  deletion.
void CM_LineMap::deleteLines( int startLine, int nb_lines )
{
	//  See if there are any lines below the deletion point that need to be altered.
	if (! ( _lines.lower_bound(startLine) == _lines.end() ) ) {
		line_map  tmpLines;

		//  Copy the original lines that are above the target pos.
		tmpLines.insert(_lines.begin(), _lines.upper_bound( startLine - 1 ) );

		//  Copy the remaining lines that are below the last line deleted while decreasing
		//  their line numbers.
		line_map::iterator pos;
		for ( pos = _lines.lower_bound(startLine + nb_lines); pos != _lines.end(); ++pos ) {
			tmpLines[ (pos->first - nb_lines ) ] = pos->second;
		}

		_lines.swap( tmpLines );
	}
}

//  Inserts a handle for marker into the handle map for the line.
void CM_LineMap::addHandleToLine( int line, int marker, int handle )
{
	handle_map* hm = &( _lines[line] );
	hm->insert( std::make_pair( marker, handle ) );

	if ( handle > currMaxMarkerHandle ) currMaxMarkerHandle = handle;

}

//  Removes a marker handle from the handle map for the line.
void CM_LineMap::deleteHandleFromLine(int line, int marker, int handle)
{
	handle_map* hm = &( _lines[line] );

	//  There should only ever be one elem for handle, but just in case.
	for( hm_pos pos = hm->find(marker); pos != hm->end(); ) {
		if ( pos->second == handle ) {
			hm->erase( pos++ );
		}
		else {
			++pos;
		}
	}
}

//  Modifies a handle for the marker in the handle map for the line.
void CM_LineMap::modifyHandleOnLine(int line, int marker, int oldHandle, int newHandle)
{
	handle_map* hm = &( _lines[line] );
	hm_range range = hm->equal_range(marker);

	for( hm_pos pos = range.first; pos != range.second; ++pos ) {
		if ( pos->second == oldHandle ) {
			pos->second = newHandle;
			if ( newHandle > currMaxMarkerHandle ) currMaxMarkerHandle = newHandle;
		}
	}
}

//  Returns the most recently created handle for marker on line.
int CM_LineMap::getHandleFromLine( int line, int marker )
{
	handle_map* hm = &( _lines[line] );
	hm_range range = hm->equal_range(marker);
	int markerHandle = 0;

	for ( hm_pos pos = range.first; pos != range.second; ++pos ) {
		markerHandle = pos->second;
	}

	return ( markerHandle );
}

//  Returns the internal Line_Map line for handle.
int CM_LineMap::getLineFromHandle( int marker, int handle )
{
	handle_map* hm;
	for ( lm_pos lpos = _lines.begin(); lpos != _lines.end(); ++lpos ) {
		hm = &( lpos->second );
		for ( hm_pos hpos = hm->begin(); hpos != hm->end(); ++ hpos ) {
			if ( ( hpos->first == marker ) && ( hpos->second == handle ) ) {
				return ( lpos->first );
				break;
			}
		}
	}
	return ( -1 );
}

//  Sends Scintilla the message to add a marker and adds the handle to the internal line map.
//  Returns the new handle.
int ChangedDocument::addMarker( int line, int marker )
{
	int markerHandle = ::SendMessage( hView, SCI_MARKERADD, line, marker );
	lm.addHandleToLine( line, marker, markerHandle );
	return ( markerHandle );
}

//  Sends Scintilla the message to delete a marker by handle, deletes the handle from the
//  internal line map and returns the handle.
int ChangedDocument::deleteMarker( int line, int marker, int handle )
{
	int oldHandle = lm.getHandleFromLine( line, marker );
	if ( ( handle ) && ( oldHandle != handle ) ) {
		int sciLine = ::SendMessage( hView, SCI_MARKERLINEFROMHANDLE, handle, 0 );
		if ( line != sciLine ) {
			//  Find the internal line the handle is on.
			int lmLine = lm.getLineFromHandle( marker, handle );
			bool dbgStop = true;
		}
	}
	::SendMessage( hView, SCI_MARKERDELETEHANDLE, oldHandle, 0 );
	lm.deleteHandleFromLine( line, marker, oldHandle );
	return ( oldHandle );
}

//  Sends Scintilla the delete notification for 'handle' and the 'add' notification for
//  the 'newMarkerID' on 'line'.  Returns the newHandle.
//  This function does not modify either the line map or history set!
int ChangedDocument::replaceMarker(int line, int oldHandle, int newMarkerID)
{
	int sciLine = ::SendMessage( hView, SCI_MARKERLINEFROMHANDLE, oldHandle, 0 );
	if ( line != sciLine ) {
		bool dbgStop = true;
	}

	::SendMessage( hView, SCI_MARKERDELETEHANDLE, oldHandle, 0 );
	return ( ::SendMessage( hView, SCI_MARKERADD, line, newMarkerID ) );
}

//  Update all history actions using an old handle to a new handle.
void ChangedDocument::modifyMarkerHandle( int oldHandle, int newHandle )
{
	if ( hist.setHandleIndex( oldHandle ) ) {
		do {
			ActionHistory thisAction( *(hist.h_iter) );
			thisAction.handle = newHandle;
			hist.handle_index.replace( hist.h_iter, thisAction );
		} while ( hist.setHandleIndex( oldHandle ) );
	}
}

//  Processes new line action entries.
void ChangedDocument::processInsert(int startLine, int endLine)
{
	int currLine = startLine;

	//  Check the startLine for an existing changemark.
	int preMarker_State = ::SendMessage( hView, SCI_MARKERGET, currLine, 0 );
	if ( ( preMarker_State ) && ( preMarker_State & ( 1 << cm[CM_NOTSAVED]->id ) ) ) {
		HistoryAction thisAction( CM_MARKERFORWARD );
		thisAction.id = cm[CM_NOTSAVED]->id;
		thisAction.handle = lm.getHandleFromLine( currLine, cm[CM_NOTSAVED]->id );
		thisAction.posStart = currLine;
		hist.insert_at_CurrActionIndex( &thisAction, NULL );
		++currLine;
	}

	//  Insert newly inserted lines into the line map.
	if ( ( endLine > startLine ) && ( endLine > currLine ) ) {
		lm.insertLines( currLine, ( endLine - startLine ) );
		HistoryAction thisAction( CM_LINECOUNTCHANGE );
		thisAction.id = CM_LINEINSERT;
		thisAction.posStart = currLine;
		thisAction.posEnd = endLine;
		hist.insert_at_CurrActionIndex( &thisAction, NULL );
	}

	//  Add new markers.
	while ( currLine <= endLine || ( ( endLine < startLine ) && ( currLine == startLine ) ) ) {
		HistoryAction thisAction( CM_MARKERADD );
		thisAction.id = cm[CM_NOTSAVED]->id;
		thisAction.handle = addMarker( currLine, cm[CM_NOTSAVED]->id );
		thisAction.posStart = currLine;
		hist.insert_at_CurrActionIndex( &thisAction, NULL );

		currLine++;
	}
}

//  Process an exisitng marker on a WHOLE LINE being deleted for proper undo/redo.
void ChangedDocument::processDelete( int startLine, int endLine )
{
	//  Update the history
	int currLine = startLine;
	int prevMark_State = 0;

	do {
		prevMark_State = ::SendMessage( hView, SCI_MARKERGET, currLine, 0 );
		if ( prevMark_State ) {

			//  We are only interested in the lines marked as changed.
			for ( int currMark = 0; currMark < NB_CHANGEMARKERS; currMark++ ) {
				if ( prevMark_State & ( 1 << cm[currMark]->id ) ) {
					int oldHandle = deleteMarker( currLine, cm[currMark]->id );

					HistoryAction thisAction( CM_MARKERDELETE );
					thisAction.id = cm[currMark]->id;	
					thisAction.handle = _tmpActionHandle--;
					thisAction.posStart = currLine;
					hist.insert_at_NextActionIndex( &thisAction, NULL );

					//  Update any previous history actions using 'oldHandle'.
					modifyMarkerHandle( oldHandle, thisAction.handle );
				}
			}

		}
		currLine++;
	} while ( currLine < endLine );

	//  In Scintilla, when lines are deleted, if there is a marker on the line after the
	//  deleted lines it won't get moved when an undo re-inserts those lines.
	//  This attempts to fix that.
	prevMark_State = ::SendMessage( hView, SCI_MARKERGET, currLine, 0 );
	if ( prevMark_State ) {

		//  Once again this only happens for if the line is marked as changed.
		for ( int currMark = 0; currMark < NB_CHANGEMARKERS; currMark++ ) {
			if ( prevMark_State & ( 1 << cm[currMark]->id ) ) {
				HistoryAction thisAction( CM_MARKERMOVE );
				thisAction.id = cm[currMark]->id;	
				thisAction.handle = lm.getHandleFromLine( currLine, cm[currMark]->id );
				//  posStart is the move from and posEnd is the move to for an undo.
				thisAction.posStart = startLine;
				thisAction.posEnd = currLine;
				hist.insert_at_NextActionIndex( &thisAction, NULL );
			}
		}

	}

	//  Store the line change in the history_set
	HistoryAction thisAction( CM_LINECOUNTCHANGE );
	thisAction.id = CM_LINEDELETE;
	thisAction.posStart = startLine;
	thisAction.posEnd = endLine;
	hist.insert_at_NextActionIndex( &thisAction, NULL );

	//  Remove the lines from the line map.
	lm.deleteLines( startLine, ( endLine - startLine ) );
}

//  Undo an action and return the updated action.  First value is true if action was
//  modified, else returns false and null.
void ChangedDocument::processUndo()
{
	int nb_actions_at_index = hist.action_index.count( targetIndex );
	if ( nb_actions_at_index > 1 ) advance( hist.a_iter, ( nb_actions_at_index - 1 ) );

	while ( hist.a_iter->_index == targetIndex ) {
		ActionHistory thisAction = *(hist.a_iter);
		if ( doUndo( &thisAction ) ) {
			hist.action_index.replace( hist.a_iter, thisAction );
		}
		if ( hist.a_iter == hist.action_index.begin() )	break;
		else hist.a_iter--;
	}
}

//  Does the actual undoing of an action.  Returns true if HistoryAction is updated.
bool ChangedDocument::doUndo( npp_plugin::actionhistory::ActionHistory *thisAction )
{
	bool updated = false;

	// CM_MARKERFORWARD is safely ignored since it is only used as a positioning mark for the
	// user to jump around and not as a state change action.
	switch ( thisAction->type)
	{
		case CM_MARKERADD:
		{
			//  When undoing a markerAdd the actions and lines aren't deleted, they are just
			//  modified since there still may be a redo.
			int oldHandle = deleteMarker( thisAction->posStart, thisAction->id, thisAction->handle );
			thisAction->handle = _tmpActionHandle--;
			modifyMarkerHandle( oldHandle, thisAction->handle );
			updated = true;						
			break;
		}

		case CM_MARKERDELETE:
		{
			int oldHandle = thisAction->handle;
			thisAction->handle = addMarker( thisAction->posStart, thisAction->id );
			modifyMarkerHandle( oldHandle, thisAction->handle );
			lm.modifyHandleOnLine( thisAction->posStart, thisAction->id, oldHandle, thisAction->handle );
			updated = true;
			break;
		}

		case CM_MARKERMOVE:
		{
			//  To move a marker, it gets deleted then added to the correct line.
			int oldHandle = thisAction->handle;
			::SendMessage( hView, SCI_MARKERDELETEHANDLE, oldHandle, 0 );
			int newHandle = ::SendMessage( hView, SCI_MARKERADD, thisAction->posEnd, thisAction->id );
			modifyMarkerHandle( oldHandle, newHandle );
			lm.modifyHandleOnLine( thisAction->posEnd, thisAction->id, oldHandle, newHandle );
			break;
		}

		case CM_LINECOUNTCHANGE:

			switch ( thisAction->id )
			{
				case CM_LINEINSERT:
					lm.deleteLines( thisAction->posStart, ( thisAction->posEnd - thisAction->posStart ) );
					break;

				case CM_LINEDELETE:
					lm.insertLines( thisAction->posStart, ( thisAction->posEnd - thisAction->posStart ) );
					break;

				default:
					break;
			}

			break;

		default:
			break;
	}

	return ( updated );
}


//  Undo an action and return the updated action.  First value is true if action was
//  modified, else returns false and null.
void ChangedDocument::processRedo()
{

	while ( hist.a_iter->_index == targetIndex ) {
		ActionHistory thisAction = *(hist.a_iter);
		if ( doRedo( &thisAction ) ) {
			hist.action_index.replace( hist.a_iter, thisAction );
		}
		if ( hist.a_iter == hist.action_index.end() ) break;
		else hist.a_iter++;
	}
}

//  Does the actual re-doing of an action.  Returns true if ActionHistory is updated.
bool ChangedDocument::doRedo( npp_plugin::actionhistory::ActionHistory *thisAction )
{
	bool updated = false;

	switch ( thisAction->type)
	{
		case CM_MARKERADD:
		{
			int oldHandle = thisAction->handle;
			thisAction->handle = addMarker( thisAction->posStart, thisAction->id );
			modifyMarkerHandle( oldHandle, thisAction->handle );
			lm.modifyHandleOnLine( thisAction->posStart, thisAction->id, oldHandle, thisAction->handle );
			updated = true;						
			break;
		}

		case CM_MARKERDELETE:
		{
			int oldHandle = deleteMarker( thisAction->posStart, thisAction->id, thisAction->handle );
			thisAction->handle = _tmpActionHandle--;
			modifyMarkerHandle( oldHandle, thisAction->handle );
			updated = true;
			break;
		}

		case CM_LINECOUNTCHANGE:

			switch ( thisAction->id )
			{
				case CM_LINEINSERT:
					lm.insertLines( thisAction->posStart, ( thisAction->posEnd - thisAction->posStart ) );
					break;

				case CM_LINEDELETE:
					lm.deleteLines( thisAction->posStart, ( thisAction->posEnd - thisAction->posStart ) );
					break;

				default:
					break;
			}

			break;

		default:
			break;
	}


	return ( updated );
}

//  Applies the CM_SAVED change marker to a document and updates it's history set.
void ChangedDocument::processFileSave()
{
	int newHandle = 0;
	int prevSavePoint = _savePointIndex;
	_savePointIndex = npp_plugin::actionindex::getCurrActionIndex( _pDoc );

	//  Set any previously SAVED markers that are currently not visible back to CM_UNSAVED.
	if ( hist.setIdIndex( cm[CM_SAVED]->id ) ) {
		while ( hist.i_iter->id == cm[CM_SAVED]->id ) {
			if ( hist.i_iter->handle < 0 ) {
				ActionHistory thisAction( *(hist.i_iter) );
				thisAction.id = cm[CM_NOTSAVED]->id;
				thisAction._referenceIndex = 0;
				thisAction.isSaved = false;
				hist.id_index.replace( hist.i_iter++, thisAction );
			}
			else {
				++hist.i_iter;
			}
		}
	}

	//  Set all the visible markers ( from the line map ) to CM_SAVED
	int lineMax = ::SendMessage( hCurrView(), SCI_GETLINECOUNT, 0, 0 );
	handle_map* hm;

	//  This is a good time to cleanup any possible stray markers as well.
	::SendMessage( hView, SCI_MARKERDELETEALL, cm[CM_NOTSAVED]->id, 0 );

	for ( lm_pos lpos = lm._lines.begin(); lpos != lm._lines.end(); ++lpos ) {
		if ( lpos->first > lineMax ) continue;

		hm = &( lpos->second );
		for ( hm_pos hpos = hm->begin(); hpos != hm->end(); ) {
			if ( ( hpos->first == cm[CM_SAVED]->id ) || ( hpos->second <= 0 ) ) {
					++hpos;
					continue;
			}

			if ( hist.setHandleIndex( hpos->second ) ) {
				newHandle = replaceMarker( lpos->first, hpos->second, cm[CM_SAVED]->id );
				lm.addHandleToLine( lpos->first, cm[CM_SAVED]->id, newHandle );

				handle_iter currSP_iter = hist.h_iter;
				do {
					ActionHistory thisAction( *(hist.h_iter) );
					thisAction.id = cm[CM_SAVED]->id;
					thisAction.handle = newHandle;
					hist.handle_index.replace( hist.h_iter, thisAction );

					if ( hist.h_iter->_index > _savePointIndex ) continue;

					if ( ( hist.h_iter->_index > currSP_iter->_index ) ||
							( ( hist.h_iter->_index == currSP_iter->_index ) &&
								( hist.h_iter->_entry > currSP_iter->_entry ) ) ) {
						currSP_iter = hist.h_iter;
					}

				} while ( hist.setHandleIndex( hpos->second ) );
				hm->erase( hpos++ );

				ActionHistory sp_action( *(currSP_iter) );
				sp_action.isSaved = true;
				sp_action._referenceIndex = _savePointIndex;
				hist.handle_index.replace( currSP_iter, sp_action );
			}
			else {
				// This should never happen.
				bool dbgStop = true;
			}
		}
	}
}
//  Returns the position of the next change.  Direction 'true' goes to the next most recent change.
//  This function uses the marker handle to determine which change is more or less recent.
int ChangedDocument::getNextChangeLine(bool direction)
{
	if (! hist.setHandleIndex( currChangePositionTarget ) ) return ( -1 );

	bool checkNext = true;
	int targetLine = -1;

	if ( direction ) {
		do {
			if ( hist.h_iter == hist.handle_index.end() ) checkNext = false;
			if ( hist.h_iter->handle > currChangePositionTarget ) {
				targetLine = lm.getLineFromHandle( cm[CM_NOTSAVED]->id, hist.h_iter->handle );
				if ( targetLine >= 0 ) {
					currChangePositionTarget = hist.h_iter->handle;
					checkNext = false;
				}
			}
			if ( checkNext ) ++hist.h_iter;
		} while ( checkNext );
	}

	else {
		do {
			if ( hist.h_iter == hist.handle_index.begin() ) checkNext = false;
			if ( hist.h_iter->handle < currChangePositionTarget ) {
				targetLine = lm.getLineFromHandle( cm[CM_NOTSAVED]->id, hist.h_iter->handle );
				if ( targetLine >= 0 ) {
					currChangePositionTarget = hist.h_iter->handle;
					checkNext = false;
				}
			}
			if ( checkNext ) --hist.h_iter;
		} while ( checkNext );
	}

	return ( targetLine );
}

//  Initializes the plugin and sets up config values.
void initPlugin()
{
	int _margin = mark::string2margin( xml::getGUIConfigValue( TEXT("SciMarkers"), TEXT("margin") ) );

	//  Marker Specific Settings
	for ( int i = 0; i < NB_CHANGEMARKERS; i++ ) {
		cm[i] = new Change_Mark;

		//  Style Settings.  From WordsStyle node ( outside of GuiConfig element )
		cm[i]->styleName = ( i == CM_SAVED ) ? ( TEXT("Changes: Saved") ) : ( TEXT("Changes: Not Saved") );
		cm[i]->type = SC_MARK_LEFTRECT;

		TiXmlHandle hXmlDoc( xml::get_pXmlPluginConfigDoc() );

		TiXmlElement* style_Node = hXmlDoc.FirstChild( 
			TEXT("NotepadPlus")).FirstChild( TEXT("LexerStyles")).FirstChild(
			TEXT("LexerType")).FirstChildElement( TEXT("WordsStyle") ).Element();

		for( style_Node; style_Node; style_Node = style_Node->NextSiblingElement() ) {
			tstring elemAttr_Name( style_Node->Attribute( TEXT("name") ) );
			if ( elemAttr_Name.compare( cm[i]->styleName ) == 0 ) {
				unsigned long result = ::_tcstol( (LPCTSTR)style_Node->Attribute( TEXT("bgColor") ), NULL, 16 );
				cm[i]->back = (RGB((result >> 16) & 0xFF, (result >> 8) & 0xFF, result & 0xFF)) | (result & 0xFF000000);
				break;
			}
		}

		cm[i]->_origTargetMargin = _margin;

		// Initialize the marker
		cm[i]->init( CM_BASEID + i);
		if ( _margin == MARGIN_CHANGES ) {
			::SendMessage( hMainView(), SCI_SETMARGINMASKN, MARGIN_CHANGES, 1 << cm[i]->id );
			::SendMessage( hSecondView(), SCI_SETMARGINMASKN, MARGIN_CHANGES, 1 << cm[i]->id );
		}
		cm[i]->margin.setTarget( MARGIN( _margin ), cm[i]->id );
		cm[i]->setTargetMarginMenuItem( cm[i]->margin.getTarget() );
	}

	//  Now that all the config values are set...
	if (! ( xml::getGUIConfigValue( TEXT("SciMarkers"), TEXT("trackUNDOREDO") ) == TEXT("true") ) ) {
		npp_plugin_changemarker::disablePlugin();
		return;
	}
}

//  When a config file is not able to be found this routine generates a default one.
bool generateDefaultConfigXml()
{
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( TEXT("1.0"), TEXT("Windows-1252"), TEXT("No") );
	doc.LinkEndChild( decl );

	TiXmlComment * comment1 = new TiXmlComment();
	comment1->SetValue( TEXT("This is the configuration and state file for Notepad++ Change Markers.") );
	doc.LinkEndChild( comment1 );

	TiXmlComment * comment2 = new TiXmlComment();
	comment2->SetValue( TEXT("These values should not be changed manually.") );
	doc.LinkEndChild( comment2 );

	TiXmlElement * root = new TiXmlElement( TEXT("NotepadPlus") );
	doc.LinkEndChild( root );

	TiXmlElement * node_language = new TiXmlElement( TEXT("Languages") );
	root->LinkEndChild( node_language );

	TiXmlElement * element_language = new TiXmlElement( TEXT("Language") );
	element_language->SetAttribute( TEXT("name"), TEXT("Change Marks") );
	element_language->SetAttribute( TEXT("ext"), TEXT("") );
	node_language->LinkEndChild( element_language );

	TiXmlElement * node_lexerStyle = new TiXmlElement( TEXT("LexerStyles") );
	root->LinkEndChild( node_lexerStyle );

	TiXmlElement * element_lexerType = new TiXmlElement( TEXT("LexerType") );
	element_lexerType->SetAttribute( TEXT("name"), TEXT("Change Marks") );
	element_lexerType->SetAttribute( TEXT("desc"), TEXT("Change Marks") );
	element_lexerType->SetAttribute( TEXT("ext"), TEXT("") );
	element_lexerType->SetAttribute( TEXT("excluded"), TEXT("yes") );
	node_lexerStyle->LinkEndChild( element_lexerType );

	TiXmlElement * element_wordStyle1 = new TiXmlElement( TEXT("WordsStyle") );
	element_wordStyle1->SetAttribute( TEXT("name"), TEXT("Changes: Saved") );
	element_wordStyle1->SetAttribute( TEXT("styleID"), TEXT("0") );
	element_wordStyle1->SetAttribute( TEXT("bgColor"), TEXT("A4FFA4") );
	element_lexerType->LinkEndChild( element_wordStyle1 );

	TiXmlElement * element_wordStyle2 = new TiXmlElement( TEXT("WordsStyle") );
	element_wordStyle2->SetAttribute( TEXT("name"), TEXT("Changes: Not Saved") );
	element_wordStyle2->SetAttribute( TEXT("styleID"), TEXT("0") );
	element_wordStyle2->SetAttribute( TEXT("bgColor"), TEXT("FFFC71") );
	element_lexerType->LinkEndChild( element_wordStyle2 );

	TiXmlElement * node_guiConfig = new TiXmlElement( TEXT("GUIConfigs") );
	root->LinkEndChild( node_guiConfig );

	TiXmlElement * element_guiConfig = new TiXmlElement( TEXT("GUIConfig") );
	element_guiConfig->SetAttribute( TEXT("name"), TEXT("SciMarkers") );
	element_guiConfig->SetAttribute( TEXT("trackUNDOREDO"), TEXT("true") );
	element_guiConfig->SetAttribute( TEXT("margin"), TEXT("MARGIN_CHANGES") );
	node_guiConfig->LinkEndChild( element_guiConfig );

	tstring baseModuleName = npp_plugin::getModuleBaseName()->c_str();
	TCHAR targetPath[MAX_PATH];
	::SendMessage( hNpp(), NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)targetPath );
	PathAppend( targetPath, baseModuleName.c_str() );
	PathAddExtension( targetPath, TEXT(".xml") );

	return ( doc.SaveFile( targetPath ) );
}

//  Change Jumping.  Direction 'true' moves to more recent change.
void jumpChanges( bool direction )
{
	int pDoc = npp_plugin::doctabmap::getVisibleDocId_by_View( npp_plugin::intCurrView() );
	if ( _doc_set.find( pDoc ) == _doc_set.end() ) {
		::MessageBox( hCurrView(),
			TEXT("No line change information was found for this document!"),
			TEXT("Jump to Change"),
			MB_OK );
		return;
	}
	ChangedDocument* thisDoc = _doc_map[pDoc];

	int targetLine = thisDoc->getNextChangeLine( direction );

	if ( targetLine < 0 ) {
		::MessageBox( hCurrView(),
			TEXT("You have reached the end of the un-saved changes in this direction."),
			TEXT("Jump to Change"),
			MB_OK );
	}
	else {
		::SendMessage( hCurrView(), SCI_ENSUREVISIBLEENFORCEPOLICY, targetLine, 0 );
		::SendMessage( hCurrView(), SCI_GOTOLINE, targetLine, 0 );
	}
}

//  Changed Line Jumping direction 'true' is down.
//  Pretty much the same as the normal bookmark jumping in N++.
void jumpChangedLines( bool direction )
{
	int posStart = ::SendMessage( hCurrView(), SCI_GETCURRENTPOS, 0, 0 );
	int lineStart = ::SendMessage( hCurrView(), SCI_LINEFROMPOSITION, posStart, 0 );
	int lineMax = ::SendMessage( hCurrView(), SCI_GETLINECOUNT, 0, 0 );

	int currLine;
	int nextLine;
	int sci_marker_direction;
	int sci_search_mask = ( 1 << cm[CM_NOTSAVED]->id );

	if ( direction ) {
		currLine = ( lineStart < lineMax ) ? ( lineStart + 1 ) : ( 0 );
		sci_marker_direction = SCI_MARKERNEXT;
	}
	else {
		currLine = ( lineStart > 0 ) ? ( lineStart - 1 ) : ( lineMax );
		sci_marker_direction = SCI_MARKERPREVIOUS;
	}

	nextLine = ::SendMessage( hCurrView(), sci_marker_direction, currLine, sci_search_mask );

	if ( nextLine < 0 ) {
		currLine = ( direction ) ? ( 0 ) : ( lineMax );
		nextLine = ::SendMessage( hCurrView(), sci_marker_direction, currLine, sci_search_mask );
	}

	if ( nextLine < 0 ) {
		::MessageBox( hCurrView(), TEXT("No un-saved changes found!" ), TEXT("Jump to Changed Line"), MB_OK );
		return;
	}

	::SendMessage( hCurrView(), SCI_ENSUREVISIBLEENFORCEPOLICY, nextLine, 0 );
	::SendMessage( hCurrView(), SCI_GOTOLINE, nextLine, 0 );
}

//  Checks and updates marker styles when notified.
void wordStylesUpdatedHandler()
{
	TiXmlDocument* pluginConfigDoc = xml::get_pXmlPluginConfigDoc();
	if (! pluginConfigDoc->LoadFile() ) return;

	//  Marker Specific Settings
	for ( int i = 0; i < NB_CHANGEMARKERS; i++ ) {
		Change_Mark* newCM = new Change_Mark;
		newCM->styleName = ( i == CM_SAVED ) ? ( TEXT("Changes: Saved") ) : ( TEXT("Changes: Not Saved") );

		//  Style Settings.  From WordsStyle node ( outside of GuiConfig element )
		TiXmlHandle hXmlDoc( xml::get_pXmlPluginConfigDoc() );
		TiXmlElement* style_Node = hXmlDoc.FirstChild( 
			TEXT("NotepadPlus")).FirstChild( TEXT("LexerStyles")).FirstChild(
			TEXT("LexerType")).FirstChildElement( TEXT("WordsStyle") ).Element();

		for( style_Node; style_Node; style_Node = style_Node->NextSiblingElement() ) {
			tstring elemAttr_Name( style_Node->Attribute( TEXT("name") ) );
			if ( elemAttr_Name.compare( newCM->styleName ) == 0 ) {
				unsigned long result = ::_tcstol( (LPCTSTR)style_Node->Attribute( TEXT("bgColor") ), NULL, 16 );
				newCM->back = (RGB((result >> 16) & 0xFF, (result >> 8) & 0xFF, result & 0xFF)) | (result & 0xFF000000);
				break;
			}
		}

		Change_Mark* currCM = cm[i];
		if (! ( currCM->fore == newCM->fore && currCM->back == newCM->back ) ) {
			cm[i]->back = newCM->back;
			cm[i]->init( cm[i]->id );
		}
	}
}

//  Document modification handler to identify and track line changes.
void modificationHandler ( SCNotification* scn )
{
	//  Static state variables
	static bool prevWasBeforeDelete = false;		//  Used for multiline delete tracking.

	//  Send the notification for processing and get back this documents history state.
	int pDoc;
	int prevIndex;
	int currIndex;
	bool excluded;
	bool dryrun;
	boost::tuples::tie( pDoc, prevIndex, currIndex, excluded, dryrun) =
		npp_plugin::actionindex::processSCNotification( scn );

	//  <---  Leave if we can. --->
	if ( _doDisable || excluded ) return;

	if ( dryrun && ( scn->modificationType & ( SC_PERFORMED_UNDO | SC_PERFORMED_REDO ) ) ) {
		prevWasBeforeDelete = false;
		return;
	}

	//  Notification is truly a modify that we need to handle.
	HWND hView = reinterpret_cast<HWND>(scn->nmhdr.hwndFrom);
	int currLine = ::SendMessage( hView, SCI_LINEFROMPOSITION, scn->position, 0);
	int modFlags = scn->modificationType;

	//  Get a ChangedDocument object for this Document.
	if ( _doc_set.find( pDoc ) == _doc_set.end() ) {
		//  Only create new ChangedDocument objects for open documents that do not have change
		//  markers disabled.
		if ( ( npp_plugin::doctabmap::fileIsOpen( pDoc ) ) &&
				( _doc_disabled_set.find( pDoc ) == _doc_disabled_set.end() ) ) {
			_doc_set.insert(pDoc);
			ChangedDocument* newDoc = new ChangedDocument(pDoc);
			_doc_map.insert( std::make_pair( pDoc, newDoc ) );
		}
		else {
			return;
		}
	}
	ChangedDocument* thisDoc = _doc_map[pDoc];
	thisDoc->hView = hView;
	thisDoc->currChangePositionTarget = thisDoc->lm.getCurrMaxMarkerHandle();

	//  <---  Marker Control --->

	//  Undo actions.
	if ( modFlags & ( SC_PERFORMED_UNDO ) ) {
		thisDoc->_prevInsertLine = -1;
		//  Use the prevIndex since we are going backwards.
		if ( thisDoc->hist.setActionIndex( prevIndex ) ) {
			thisDoc->targetIndex = prevIndex;
			thisDoc->processUndo();
		}
	}

	//  Redo actions.
	else if ( modFlags & ( SC_PERFORMED_REDO ) ) {
		thisDoc->_prevInsertLine = -1;
		if ( thisDoc->hist.setActionIndex( currIndex ) ) {
			thisDoc->targetIndex = currIndex;
			thisDoc->processRedo();
		}
	}

	//  New actions.
	else {
		//  Set the target action index and truncate existing history entries if needed.
		if ( thisDoc->hist.setActionIndex( currIndex ) ) {
			//  Multiline deletes store actions in currIndex + 1.
			if ( (! prevWasBeforeDelete ) && (! dryrun ) ) {
				thisDoc->hist.truncateActions();
			}
			else if ( dryrun ) {
				thisDoc->hist.truncateActionsAtNextIndex();
			}
		}
		prevWasBeforeDelete = false;

		//  Track deleted line marker states.
		if ( modFlags & ( SC_MOD_BEFOREDELETE ) ) {
			thisDoc->_prevInsertLine = -1;
			int endLine = ::SendMessage( hView, SCI_LINEFROMPOSITION, ( scn->position + scn->length ), 0 );
			if ( ( currLine - endLine ) != 0 ) {
				prevWasBeforeDelete = true;
				thisDoc->targetIndex = currIndex;
				thisDoc->processDelete( currLine, endLine );
			}
		}

		//  Track new actions.
		else if ( ( thisDoc->_prevInsertLine != currLine ) ||
				( ( thisDoc->_prevInsertLine == currLine ) && ( scn->linesAdded != 0 ) ) ) {
			thisDoc->_prevInsertLine = currLine;
			thisDoc->targetIndex = currIndex;
			thisDoc->processInsert( currLine, ( currLine + scn->linesAdded ) );
		}
	}

	if ( ( currIndex == 0 ) || ( currIndex == thisDoc->_savePointIndex ) ) {
		//  This is a good time to cleanup any possible stray markers.
		::SendMessage( hView, SCI_MARKERDELETEALL, cm[CM_NOTSAVED]->id, 0 );
		if ( currIndex == 0 ) ::SendMessage( hView, SCI_MARKERDELETEALL, cm[CM_SAVED]->id, 0 );
	}
}


//  Verifies the menu state is properly activated for the activated buffer.
void bufferActivatedHandler( SCNotification *scn )
{
	if ( _doDisable ) return;

	int pDoc = npp_plugin::doctabmap::getVisibleDocId_by_View( npp_plugin::intCurrView() );

	bool menu_enabled = true;

	if ( _doc_disabled_set.find( pDoc ) != _doc_disabled_set.end() ) {
		menu_enabled = false;
	}

	setMenuState( menu_enabled );
}

//  Alters the state of current Changes: Not Saved markers to Changes: Saved.
void fileSaveHandler ()
{
	int pDoc = npp_plugin::doctabmap::getVisibleDocId_by_View( npp_plugin::intCurrView() );
	if ( _doc_set.find( pDoc ) != _doc_set.end() ) {
		ChangedDocument* thisDoc = _doc_map[pDoc];
		thisDoc->processFileSave();
	}
}

//  Clean up ChangedDocument object for the closing document.
void fileBeforeCloseHandler( SCNotification *scn )
{
	int pDoc = npp_plugin::doctabmap::getDocIdFromBufferId( scn->nmhdr.idFrom );
	if ( _doc_set.find( pDoc ) != _doc_set.end() ) {
		delete _doc_map[pDoc];
		_doc_map.erase( _doc_map.find( pDoc ) );
		_doc_set.erase( pDoc );
	}
}

//  Movement control function.
void jumpChangePrev() { jumpChanges( false ); }

//  Movement control function.
void jumpChangeNext() { jumpChanges( true ); }

//  Movement Control function.
void jumpLineUp() {	jumpChangedLines( false ); }

//  Movement Control function.
void jumpLineDown() { jumpChangedLines( true ); }

//  Global display margin control
void displayWithLineNumbers()
{
	cm[CM_SAVED]->setTargetMarginMenuItem( MARGIN_LINENUMBER );
	cm[CM_NOTSAVED]->setTargetMarginMenuItem( MARGIN_LINENUMBER );
}

//  Global display margin control
void displayWithChangeMarks()
{
	cm[CM_SAVED]->setTargetMarginMenuItem( MARGIN_CHANGES );
	cm[CM_NOTSAVED]->setTargetMarginMenuItem( MARGIN_CHANGES );
}

//  Global display margin control
//  Removes the change marker from each margin marker mask, forcing the markers to
//  be displayed as line highlights using the highlighter style information.
//  If the markers are already being displayed as highlights this hides the markers.
void displayAsHighlight()
{
	cm[CM_SAVED]->setTargetMarginMenuItem( MARGIN_NONE );
	cm[CM_NOTSAVED]->setTargetMarginMenuItem( MARGIN_NONE );
}

//  Clear the marker history for the currently focused document and disable change marker
//  tracking for it.
void disableDoc()
{
	int pDoc = npp_plugin::doctabmap::getVisibleDocId_by_View( npp_plugin::intCurrView() );
	bool menu_enabled = true;
	
	if ( _doc_disabled_set.find( pDoc ) != _doc_disabled_set.end() ) {
		//  Disabled, so remove from disabled list and enable menu.
		_doc_disabled_set.erase( pDoc );
	}

	else if ( _doc_set.find( pDoc ) != _doc_set.end() ) {
		//  Enabled: add to disabled list, cleanup existing ChangedDocument, and disable menu.
		_doc_disabled_set.insert( pDoc );
		delete _doc_map[pDoc];
		_doc_map.erase( _doc_map.find( pDoc ) );
		_doc_set.erase( pDoc );
		menu_enabled = false;
		::SendMessage( hCurrView(), SCI_MARKERDELETEALL, cm[CM_NOTSAVED]->id, 0 );
		::SendMessage( hCurrView(), SCI_MARKERDELETEALL, cm[CM_SAVED]->id, 0 );
	}

	else {
		//  Not even initialized, so add to disabled list and disable menu.
		_doc_disabled_set.insert( pDoc );
		menu_enabled = true;
	}

	setMenuState( menu_enabled );
}

void setMenuState( bool enabled )
{
	enabled = !enabled;

	HMENU hMenu = (HMENU)( ::SendMessage( hNpp(), NPPM_GETMENUHANDLE, NPPPLUGINMENU, 0 ) );

	for ( int menuItem = 1; menuItem < NB_MENU_COMMANDS; menuItem++ ) {
		int cmdID = getCmdId( menuItem );

		switch ( menuItem )
		{
		case CMD_DISABLEDOC:
			::SendMessage(npp_plugin::hNpp(), NPPM_SETMENUITEMCHECK, cmdID, enabled );
			break;

		default:
			::EnableMenuItem( hMenu, cmdID, enabled );
		}
	}
}

//  Clear marker history and disable change marker tracking and menu items.
void disablePlugin()
{
	_doDisable = !_doDisable;

	HMENU hMenu = (HMENU)( ::SendMessage( hNpp(), NPPM_GETMENUHANDLE, NPPPLUGINMENU, 0 ) );

	for ( int menuItem = 1; menuItem < NB_MENU_COMMANDS; menuItem++ ) {
		int cmdID = getCmdId( menuItem );

		switch ( menuItem )
		{
		case CMD_DISABLEPLUGIN:
			::SendMessage(npp_plugin::hNpp(), NPPM_SETMENUITEMCHECK, cmdID, _doDisable );
			if ( _doDisable ) {
				//  Remove Scintilla marker definitions.
				mark::setMarkerAvailable( cm[CM_SAVED]->id );
				mark::setMarkerAvailable( cm[CM_NOTSAVED]->id );

				//  Cleanup existing Change_Marks.
				for ( int view = MAIN_VIEW; view <= SUB_VIEW; view++ ) {
					int nb_openfiles_view = ( view == MAIN_VIEW ) ? ( PRIMARY_VIEW ) : ( SECOND_VIEW );
					int nb_Tabs = ::SendMessage( hNpp(), NPPM_GETNBOPENFILES, 0, nb_openfiles_view );
					if ( nb_Tabs > 0 ) {
						int index2Restore = ::SendMessage( hNpp(), NPPM_GETCURRENTDOCINDEX, view, 0 );
						for ( int tab = 0; tab < nb_Tabs; tab++ ) {
							::SendMessage( hNpp(), NPPM_ACTIVATEDOC, view, tab);
							::SendMessage( hViewByInt( view ), SCI_MARKERDELETEALL, cm[CM_SAVED]->id, 0 );
							::SendMessage( hViewByInt( view ), SCI_MARKERDELETEALL, cm[CM_NOTSAVED]->id, 0 );
						}
						::SendMessage( hNpp(), NPPM_ACTIVATEDOC, view, index2Restore );
					}
				}

				delete cm[CM_SAVED];
				delete cm[CM_NOTSAVED];

				//  Cleanup existing documents.
				while (! _doc_map.empty() ) {
					if ( _doc_map.size() == 0 ) {
						bool dbgStop = true;
					}
					_doc_set.erase( _doc_map.begin()->first );
					delete _doc_map.begin()->second;
					_doc_map.erase( _doc_map.begin() );
				}

				//  Store config file tracking value to xml
				xml::setGUIConfigValue( TEXT("SciMarkers"), TEXT("trackUNDOREDO"), TEXT("false") );

				//  TODO: add a way to clear out the xml

			}
			else {
				//  Store to config file
				xml::setGUIConfigValue( TEXT("SciMarkers"), TEXT("trackUNDOREDO"), TEXT("true") );

				//  Re-initialize the plugin
				initPlugin();
			}
			break;
		default:
			::EnableMenuItem( hMenu, cmdID, _doDisable );
		}
	}
}

}  //End namespace:: npp_plugin_changemarker
