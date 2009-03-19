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
#include <unordered_map>
#include <vector>

//  <--- headers for testing actioncounter --->
#include "Platform.h"
#include "UniConversion.h"
#include "Indicator.h"
#include "XPM.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "Decoration.h"
#include "CharClassify.h"
#include "CellBuffer.h"
#include "Document.h"
#include "PositionCache.h"
#include "KeyMap.h"
#include "ContractionState.h"

//  Easier to just add this here than include other files.
struct RangeToFormat {
	HDC hdc;
	HDC hdcTarget;
	RECT rc;
	RECT rcPage;
	CharacterRange chrg;
};

#include "Editor.h"

//  ^^^^  End headers for testing action counter.

namespace npp_plugin_changemarker {

using namespace npp_plugin;
namespace xml = npp_plugin::xmlconfig;
namespace mark = npp_plugin::markers;
namespace a_index = npp_plugin::actionindex;
namespace a_history = npp_plugin::actionhistory;

Change_Mark* cm[NB_CHANGEMARKERS];
bool _doDisable = false;

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
		case MARGIN_BOOKMARK:
			cmdID = getCmdId( CMD_BOOKMARK );
			break;
		case MARGIN_LINENUMBER:
			if ( ( i == CHECK ) && ( this->type != SC_MARK_LEFTRECT ) ) {
				//  Only allow _LEFTRECT type in LineNumber margin.
				this->markerOverride( SC_MARK_LEFTRECT );
			}
			else if ( ( i == UNCHECK ) && (menuTarget == MARGIN_LINENUMBER) &&
				( this->_tempMarkerOverride ) ) this->resetOverride();

			cmdID = getCmdId( CMD_LINENUMBER );
			break;
		case MARGIN_PLUGIN:
			cmdID = getCmdId( CMD_PLUGIN );
			break;
		default:
			cmdID = getCmdId( CMD_HIGHLIGHT );
			break;
		}
		::SendMessage( hNpp(), NPPM_SETMENUITEMCHECK, cmdID, i );
	}

	//  Set the new target marker margin.
	this->margin.setTarget( target, this->id );
	//  Write config value to file.
	xml::setGUIConfigValue( TEXT("SciMarkers"), TEXT("targetMargin"), mark::margin2string(target) );

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

//  Initializes the plugin and sets up config values.
void initPlugin()
{
	//  Global Plugin Settings
	bool _track = false;
	if ( xml::getGUIConfigValue( TEXT("SciMarkers"), TEXT("trackUNDOREDO") ) == TEXT("true") ) _track = true;

	bool _active = false;
	if ( xml::getGUIConfigValue( TEXT("SciMarkers"), TEXT("active") ) == TEXT("true") ) _active = true;

	int _margin = mark::string2margin( xml::getGUIConfigValue( TEXT("SciMarkers"), TEXT("margin") ) );

	//  Marker Specific Settings
	for ( int i = 0; i < NB_CHANGEMARKERS; i++ ) {
		Change_Mark* currCM = new Change_Mark;

		//  ID Settings.
		currCM->markName = ( i == CM_SAVED ) ? ( TEXT("CM_SAVED") ) : ( TEXT("CM_NOTSAVED") );
		currCM->styleName = ( i == CM_SAVED ) ? ( TEXT("Changes: Saved") ) : ( TEXT("Changes: Not Saved") );

		//  Style Settings.  From WodsStyle node ( outside of GuiConfig element )
		TiXmlHandle hXmlDoc( xml::get_pXmlPluginConfigDoc() );
		TiXmlElement* style_Node = hXmlDoc.FirstChild( 
			TEXT("NotepadPlus")).FirstChild( TEXT("LexerStyles")).FirstChild(
			TEXT("LexerType")).FirstChildElement( TEXT("WordsStyle") ).Element();

		for( style_Node; style_Node; style_Node = style_Node->NextSiblingElement() ) {
			tstring elemAttr_Name( style_Node->Attribute( TEXT("name") ) );
			if ( elemAttr_Name.compare( currCM->styleName ) == 0 ) {
				unsigned long result = ::_tcstol( (LPCTSTR)style_Node->Attribute( TEXT("bgColor") ), NULL, 16 );
				currCM->back = (RGB((result >> 16) & 0xFF, (result >> 8) & 0xFF, result & 0xFF)) | (result & 0xFF000000);
				break;
			}
		}

		//  Marker View Settings
		currCM->active = _active;
		currCM->display =
			( xml::getGUIConfigValue( currCM->markName, TEXT("displayMark") ) == TEXT("true") ) ? ( true ) : ( false );
		currCM->type = mark::string2marker( xml::getGUIConfigValue( currCM->markName, TEXT("markType") ) );
		if ( currCM->type == SC_MARK_PIXMAP ) {
			currCM->xpmFileName.assign( xml::getGUIConfigValue( currCM->markName, TEXT("xpm") ) );

			// This should be in the marker extension.
			if (! currCM->xpmFileName.compare( currCM->xpmFileName.length() - 4, 4, TEXT(".xpm") ) == 0 ) 
				currCM->xpmFileName.append( TEXT(".xpm") );
		}
		currCM->alpha = 
			::_tcstol( (LPCTSTR)(xml::getGUIConfigValue( currCM->markName, TEXT("alpha") ).c_str() ), NULL, 16 );

		//  Margin View Settings
		currCM->_origTargetMargin = _margin;

		//  Handle linenumber margin special markertype case.
		//  The only marker that should be used in the linenumber margin is _LEFTRECT
		cm[i] = currCM;
	}

	//  Now that all the config values are set...
	if (! _track ) {
		npp_plugin_changemarker::disable();
		return;
	}

	bool retry = true;
	while ( retry ) {
		retry = false;
		int* markResults = getAvailableMarkers( NB_CHANGEMARKERS );
		
		if ( ( markResults[0] == -1 ) && ( markResults[NB_MAX_PLUGINMARKERS - 1] == -1 ) ) {

			tstring errMsg = TEXT("A communication error has occurred between this plugin (");
			errMsg.append( getModuleBaseName()->c_str() );
			errMsg.append( TEXT(") and SciMarkerSymbol while configuring markers.\r\n") );
			errMsg.append( TEXT("This could be due to plugin load order or a missing plugin") );
			errMsg.append( TEXT(" file, would you like to try again?") );

			int msgboxReply =
				::MessageBox( hNpp(), errMsg.c_str(), TEXT("Plugin Communication Error!"), MB_RETRYCANCEL| MB_ICONWARNING );

			if ( msgboxReply == IDRETRY ) {
				retry = true;
			}
			else {
				npp_plugin_changemarker::disable();
				break;
			}
		}
		else {
			initMarker( markResults );
			break;
		}
	}
}

//  Initalizes the markers found for this plugin.
void initMarker( int* markerArray )
{
	int currMarker = 0;
	for ( int i = 0; i < NB_MAX_PLUGINMARKERS; i++ ) {
		if ( markerArray[i] == 1 ) {
			//  initialize this marker
			cm[currMarker]->init(i);
			cm[currMarker]->margin.setTarget( MARGIN( cm[currMarker]->_origTargetMargin ), cm[currMarker]->id );
			cm[currMarker]->setTargetMarginMenuItem( cm[currMarker]->margin.getTarget() );
			if ( currMarker == NB_CHANGEMARKERS ) break;
			currMarker++;
		}
	}

}

//  Checks and updates marker styles when notified.
void wordStylesUpdatedHandler()
{
	TiXmlDocument* pluginConfigDoc = xml::get_pXmlPluginConfigDoc();
	if (! pluginConfigDoc->LoadFile() ) return;

	//  Marker Specific Settings
	for ( int i = 0; i < NB_CHANGEMARKERS; i++ ) {
		Change_Mark* newCM = new Change_Mark;
		newCM->markName = ( i == CM_SAVED ) ? ( TEXT("CM_SAVED") ) : ( TEXT("CM_NOTSAVED") );
		newCM->styleName = ( i == CM_SAVED ) ? ( TEXT("Changes: Saved") ) : ( TEXT("Changes: Not Saved") );

		//  Style Settings.  From WodsStyle node ( outside of GuiConfig element )
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
			//  If an alpha wasn't defined in the xml force a recalculation of it based on the new color.
			if ( xml::getGUIConfigValue( newCM->styleName, TEXT("alpha") ).empty() ) cm[i]->alpha = -1;
			
			cm[i]->init( cm[i]->id );
		}
	}
}

//  Document modification handler for 'BEFORE' messages to track marker handle changes before
//  the doc is actually modified.
void preModificationHandler ( SCNotification* scn )
{
}

//  Document modification handler to identify and track line changes.
void modificationHandler ( SCNotification* scn )
{

#define MSG_DEBUGGING
#ifdef MSG_DEBUGGING
	int dbgmsg = scn->nmhdr.code;
	int dbgflags = scn->modificationType;
	TCHAR dbgflagHEX[65];
	::_itot(dbgflags, dbgflagHEX, 16);
	TCHAR dbgflag2[65];
	::_itot(dbgflags, dbgflag2, 2);
	int dbgLines = scn->linesAdded;
#endif

	//  Static state variable for just BEFOREDELETE notifications, which are always followed
	//  by the actual, so no need to identify by pdoc.
	static bool prevWasBeforeDelete = false;

	//  <---  Leave if we can. --->
	if ( _doDisable ) return;

	//  Send the notification for processing and get back this documents history state.
	int pDoc;
	int prevIndex;
	int currIndex;
	bool excluded;
	bool dryrun;
	boost::tuples::tie( pDoc, prevIndex, currIndex, excluded, dryrun) =
		a_index::processSCNotification( scn );

	if ( excluded ) {
		return;
	}

	if ( dryrun && ( scn->modificationType & ( SC_PERFORMED_UNDO | SC_PERFORMED_REDO ) ) ) {
		prevWasBeforeDelete = false;
		return;
	}

	//  Notification is truly a modify that we need to handle.
	HWND hView = reinterpret_cast<HWND>(scn->nmhdr.hwndFrom);
	int currLine = ::SendMessage( hView, SCI_LINEFROMPOSITION, scn->position, 0);
	int startLine = currLine;
	int modFlags = scn->modificationType;

	//  <---  Marker Control --->

	//  Undo actions.
	if ( modFlags & ( SC_PERFORMED_UNDO ) ) {
		//  Using the prevIndex since we are going backwards.
		if ( cm[CM_NOTSAVED]->history.setTargetIndex( pDoc, prevIndex ) ) {

			int nb_actions_at_index = cm[CM_NOTSAVED]->history.action_index.count(
				boost::make_tuple( pDoc, prevIndex ) );

			if ( nb_actions_at_index > 1 ) 
				advance( cm[CM_NOTSAVED]->history.a_iter, ( nb_actions_at_index - 1 ) );

			while ( cm[CM_NOTSAVED]->history.a_iter->_actionIndex == prevIndex ) {

				//  Make what-ever modifications need to be made to keep current.
				ActionHistory thisAction = *(cm[CM_NOTSAVED]->history.a_iter);
				bool updated = false;

				bool dbgWatch = false;
				//  dbgstring: Action: {s1}:{s2} type: {s3} handle: {s4} line: {s5}
				docAction_iter s0 = cm[CM_NOTSAVED]->history.a_iter;
				int s1 = thisAction._actionIndex;
				int s2 = thisAction._actionEntryID;
				int s3 = thisAction._action;
				int s4 = thisAction._actionHandle;
				int s5 = thisAction._referenceIndex;

				switch (thisAction._action)
				{
					case PLM_MARKERADD:
					{
						int tmpAH =  cm[CM_NOTSAVED]->tmpActionHandle;
						::SendMessage( hView, SCI_MARKERDELETEHANDLE, thisAction._actionHandle, 0 );
						thisAction._actionHandle = cm[CM_NOTSAVED]->tmpActionHandle--;
						int tmpAH2 =  cm[CM_NOTSAVED]->tmpActionHandle;
						updated = true;						

						dbgWatch = true;
						break;
					}
					case PLM_MARKERDELETE:
					{
						int markerID = ::SendMessage( hView, SCI_MARKERADD, thisAction._referenceIndex, cm[CM_NOTSAVED]->id );
						thisAction._actionHandle = markerID;
						updated = true;						

						dbgWatch = true;
						break;
					}

						dbgWatch = true;
						break;

					case PLM_MARKERFORWARD:
						//  need to modify pointers?
						dbgWatch = true;
						break;

					default:
						//  there shouldn't be a time this is hit.
						dbgWatch = true;
						break;
				}

				//  Replace the current indexed action with thisAction.
				if ( updated ) {
					cm[CM_NOTSAVED]->history.action_index.replace(
						cm[CM_NOTSAVED]->history.a_iter, thisAction );
					//  Since replace will forward iterate.
					//cm[CM_NOTSAVED]->history.a_iter--;
				}
				
				//  Go to the next action.
				if ( cm[CM_NOTSAVED]->history.a_iter == cm[CM_NOTSAVED]->history.action_index.begin() )
					break;

				cm[CM_NOTSAVED]->history.a_iter--;
			}
		}
		else {
			// Nothing to do?
			bool dbgStop = true;
		}
	}

	//  Redo actions.
	else if ( modFlags & ( SC_PERFORMED_REDO ) ) {
		//  Redo Actions.
		if ( cm[CM_NOTSAVED]->history.setTargetIndex( pDoc, currIndex ) ) {

			int nb_actions_at_index = cm[CM_NOTSAVED]->history.action_index.count(
				boost::make_tuple( pDoc, currIndex ) );

			while ( cm[CM_NOTSAVED]->history.a_iter->_actionIndex == currIndex ) {

				//  Make what-ever modifications need to be made to keep current.
				ActionHistory thisAction = *(cm[CM_NOTSAVED]->history.a_iter);
				bool updated = false;

				bool dbgWatch = false;

				//  dbgstring: Action: {s0} at {s1}:{s2} type: {s3} handle: {s4} line: {s5}
				docAction_iter s0 = cm[CM_NOTSAVED]->history.a_iter;
				int s1 = thisAction._actionIndex;
				int s2 = thisAction._actionEntryID;
				int s3 = thisAction._action;
				int s4 = thisAction._actionHandle;
				int s5 = thisAction._referenceIndex;


				switch (thisAction._action)
				{
					case PLM_MARKERADD:
					{
						int markerID = ::SendMessage( hView, SCI_MARKERADD, thisAction._referenceIndex, cm[CM_NOTSAVED]->id );
						thisAction._actionHandle = markerID;
						updated = true;						

						dbgWatch = true;
						break;
					}
					case PLM_MARKERDELETE:
					{
						::SendMessage( hView, SCI_MARKERDELETEHANDLE, thisAction._actionHandle, 0 );
						thisAction._actionHandle = cm[CM_NOTSAVED]->tmpActionHandle;
						cm[CM_NOTSAVED]->tmpActionHandle--;
						updated = true;						

						dbgWatch = true;
						break;
					}

					case PLM_MARKERFORWARD:
						//  need to modify pointers?
						dbgWatch = true;
						break;

					default:
						//  there shouldn't be a time this is hit.
						dbgWatch = true;
						break;
				}

				//  Replace the current indexed action with thisAction.
				if ( updated ) {
					cm[CM_NOTSAVED]->history.action_index.replace(
						cm[CM_NOTSAVED]->history.a_iter, thisAction );
					//  Since replace will forward iterate.
					//cm[CM_NOTSAVED]->history.a_iter--;
				}
				
				//  Go to the next action.
				if ( cm[CM_NOTSAVED]->history.a_iter == cm[CM_NOTSAVED]->history.action_index.end() )
					break;

				cm[CM_NOTSAVED]->history.a_iter++;
			}
		}
		else {
			// Nothing to do?
			bool dbgStop = true;
		}

	}

	//  New actions.
	else {
		//  Truncate existing history entries if needed.
		if ( cm[CM_NOTSAVED]->history.setTargetIndex( pDoc, currIndex ) ) {
			if (! prevWasBeforeDelete ) cm[CM_NOTSAVED]->history.truncateActions();
		}
		prevWasBeforeDelete = false;

		//  Track removal of existing markers.
		if ( modFlags & ( SC_MOD_BEFOREDELETE ) ) {
			currLine = startLine;
			int endLine = ::SendMessage( hView, SCI_LINEFROMPOSITION, ( scn->position + scn->length ), 0 );
			if ( currLine <= endLine ) {
				prevWasBeforeDelete = true;
				do {
					//  tmpActionHandle is a negative int that is used as a placeholder for deleted
					//  marker handles.
					if ( ::SendMessage( hView, SCI_MARKERGET, currLine, 0 & ( 1 << cm[CM_NOTSAVED]->id ) ) ) {
						cm[CM_NOTSAVED]->tmpActionHandle--;
						int dbgTmpActionHandle = cm[CM_NOTSAVED]->tmpActionHandle;
						cm[CM_NOTSAVED]->history.insertAction_at_NextIndex( PLM_MARKERDELETE,
								cm[CM_NOTSAVED]->tmpActionHandle, currLine, false );
					}
					currLine++;
				} while ( currLine <= endLine );
			}
		}

		//  Track new insertions.
		else {
			//  Send Scintilla the marker messages and store the action in the history tracker.
			int markerHandle;
			do {
				//  Right now this happens for EVERY change, need a line to handle map!
				markerHandle = ::SendMessage( hView, SCI_MARKERADD, currLine, cm[CM_NOTSAVED]->id );
				cm[CM_NOTSAVED]->history.insertAction_at_CurrIndex(	PLM_MARKERADD, markerHandle, currLine, false );
				cm[CM_NOTSAVED]->prevActionHandle = markerHandle;
				cm[CM_NOTSAVED]->prevActionHandleRefIndex = currIndex;
				currLine++;
			} while ( currLine <= ( startLine + scn->linesAdded ) );
		}

	}



//#if(0)
	//  <--- Area for testing action counter --->
	int actionCount = currIndex;

	class ActionCounter : public Editor {
	public:
		using Editor::pdoc;
	};
	ActionCounter* ac = reinterpret_cast<ActionCounter *>(::GetWindowLongPtr( hView, 0));
	Document* ac_pDoc = ac->pdoc;
	//  Use a watch on ac_pDoc->cb.currentAction for Scintilla count.

	//  ^^^^^^  end area for action counter testing
//#endif

}

//  Alters the state of current Changes: Not Saved markers to Changes: Saved.
void fileSaveHandler (SCNotification *scn)
{
	//  Iterate through the current document.
}

//  Movement control function.
void jumpPrevChange()
{
	::MessageBox(npp_plugin::hNpp(),
	TEXT(""),
	TEXT("jumpPrevChange"),
	MB_OK);
}


//  Movement control function.
void jumpNextChange()
{
	::MessageBox(npp_plugin::hNpp(),
	TEXT(""),
	TEXT("jumpNextChange"),
	MB_OK);
}

//  Global display margin control
void displayWithLineNumbers()
{
	//  Change the target
	cm[CM_SAVED]->setTargetMarginMenuItem( MARGIN_LINENUMBER );
	cm[CM_NOTSAVED]->setTargetMarginMenuItem( MARGIN_LINENUMBER );
}

//  Global display margin control
void displayWithBookMarks()
{
	//  Change the target
	cm[CM_SAVED]->setTargetMarginMenuItem( MARGIN_BOOKMARK );
	cm[CM_NOTSAVED]->setTargetMarginMenuItem( MARGIN_BOOKMARK );
}

//  Global display margin control ( plugin margin )
void displayInPluginMargin()
{
	//  Change the target
	cm[CM_SAVED]->setTargetMarginMenuItem( MARGIN_PLUGIN );
	cm[CM_NOTSAVED]->setTargetMarginMenuItem( MARGIN_PLUGIN );
}

//  Global display margin control ( no margin )
//  Removes the change marker from each margin marker mask, forcing the markers to
//  be displayed as line highlights.
void displayAsHighlight()
{
	//  Change the target
	cm[CM_SAVED]->setTargetMarginMenuItem( MARGIN_NONE );
	cm[CM_NOTSAVED]->setTargetMarginMenuItem( MARGIN_NONE );
}

//  Clear marker history and disable change marker tracking and menu items.
void disable()
{
	_doDisable = !_doDisable;

	HMENU hMenu = (HMENU)( ::SendMessage( hNpp(), NPPM_GETMENUHANDLE, NPPPLUGINMENU, 0 ) );

	for ( int menuItem = 1; menuItem < NB_MENU_COMMANDS; menuItem++ ) {
		int cmdID = getCmdId( menuItem );

		switch ( menuItem )
		{
		case CMD_DISABLE:
			::SendMessage(npp_plugin::hNpp(), NPPM_SETMENUITEMCHECK, cmdID, _doDisable );
			if ( _doDisable ) {
				//  Remove Scintilla marker definitions.
				mark::setMarkerAvailable( cm[CM_SAVED]->id );
				mark::setMarkerAvailable( cm[CM_NOTSAVED]->id );

				//  Cleanup the Change_Marks
				delete cm[CM_SAVED];
				delete cm[CM_NOTSAVED];

				//  Store to config file value to xml
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
