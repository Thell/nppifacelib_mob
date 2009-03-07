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

namespace npp_plugin_changemarker {

Change_Mark* cm[NB_CHANGEMARKERS];

namespace {


} // End un-named namedspace.

//  Initializes the plugin and sets up config values.
void initPlugin()
{
	namespace xml = npp_plugin::xmlconfig;
	namespace mark = npp_plugin::markers;


	//  Global Plugin Settings
	bool _track = false;
	if ( xml::getGUIConfigValue( TEXT("SciMarkers"), TEXT("trackUNDOREDO") ) == TEXT("true") ) _track = true;

	bool _active = false;
	if ( xml::getGUIConfigValue( TEXT("SciMarkers"), TEXT("active") ) == TEXT("true") ) _active = true;

	int _margin = mark::string2margin( xml::getGUIConfigValue( TEXT("SciMarkers"), TEXT("margin") ) );

	//  N++ Plugin Margin Setting ( there should be an external control for this!)
	bool _mvMarginShow =
		( xml::getGUIConfigValue( TEXT("MAIN_VIEW"), TEXT("pluginMargin") ) == TEXT("show") ) ? ( true ) : ( false );
	bool _svMarginShow = 
		( xml::getGUIConfigValue( TEXT("SUB_VIEW"), TEXT("pluginMargin") ) == TEXT("show") ) ? ( true ) : ( false );

	//  Marker Specific Settings
	for ( int i = 0; i < NB_CHANGEMARKERS; i++ ) {
		Change_Mark* currCM = new Change_Mark;
		//cm[i] = new Change_Mark;
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
		currCM->margin.setTarget( MARGIN(_margin) );
		currCM->margin.svp[MAIN_VIEW]->_pluginMarginShow = _mvMarginShow;
		currCM->margin.svp[MAIN_VIEW]->_hView = hMainView();
		currCM->margin.svp[SUB_VIEW]->_pluginMarginShow = _svMarginShow;
		currCM->margin.svp[SUB_VIEW]->_hView = hSecondView();

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
			if ( currMarker == NB_CHANGEMARKERS ) break;
			currMarker++;
		}
	}

}

//  Checks and updates marker styles when notified.
void wordStylesUpdatedHandler()
{
	namespace xml = npp_plugin::xmlconfig;
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

void modificationHandler ( SCNotification* scn )
{
	npp_plugin::hCurrViewNeedsUpdate();

	//  Process document modification messages
	//  Each UserPerformed SC_STARTACTION increments our action counter.
	if ( scn->modificationType & ( SC_PERFORMED_USER & SC_STARTACTION ) ) {
		//actionCount++;
	}

	if ( scn->modificationType & ( SC_MOD_BEFOREINSERT | SC_MOD_BEFOREDELETE ) ) {
		//
	}
	if ( scn->modificationType & ( SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT ) ) {
		//  Just a generic change and no marker reset is needed.
		for ( int i = 0; i <= NB_MAX_PLUGINMARKERS; i++ ) {
			::SendMessage( hCurrView(), SCI_MARKERADD, i, i );
		}
		bool stop = true;
	}

	if ( scn->modificationType & ( SC_STARTACTION ^ SC_PERFORMED_USER ) ) {
		if ( scn->modificationType & ( SC_PERFORMED_UNDO ) ) {
		//	actionCount--;
		}

		if ( scn->modificationType & ( SC_MOD_DELETETEXT) ) {
			//  An undo of an insert is actually a delete so we need to remove/reset markers.
		//	undoredo_RemoveHandler(scn);
		}
		else if ( scn->modificationType & ( SC_MOD_INSERTTEXT ) ) {
			//  An undo of a delete is an actually an insert so we need to add/reset markers.
		//	undoredo_InsertHandler(scn);
		}
	}

		else if (  scn->modificationType & ( SC_PERFORMED_REDO ) ) {
			if ( scn->modificationType & ( SC_MOD_DELETETEXT) ) {
				//  An redo of a delete is still a delete so we need to remove/reset markers.
			//	undoredo_RemoveHandler(scn);
			}
			else if ( scn->modificationType & ( SC_MOD_INSERTTEXT ) ) {
				//  An redo of an insert is still an insert so we need to add/reset markers.
			//	undoredo_InsertHandler(scn);
			}
		}
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
	int cmdId = npp_plugin::getCmdId( CMD_LINENUMBER );
	::SendMessage(npp_plugin::hNpp(), NPPM_SETMENUITEMCHECK, cmdId, true);
}

//  Global display margin control
void displayWithBookMarks()
{
	int cmdId = npp_plugin::getCmdId( CMD_BOOKMARK );
	::SendMessage(npp_plugin::hNpp(), NPPM_SETMENUITEMCHECK, cmdId, true);
}

//  Global display margin control ( plugin margin )
void displayInPluginMargin()
{
	int cmdId = npp_plugin::getCmdId( CMD_PLUGIN );
	::SendMessage(npp_plugin::hNpp(), NPPM_SETMENUITEMCHECK, cmdId, true);
}

//  Global display margin control ( no margin )
//  Removes the change marker from each margin marker mask, forcing the markers to
//  be displayed as line highlights.
void displayAsHighlight()
{
	int cmdId = npp_plugin::getCmdId( CMD_HIGHLIGHT );
	::SendMessage(npp_plugin::hNpp(), NPPM_SETMENUITEMCHECK, cmdId, true);
}

//  Clear marker history and disable change marker tracking.
void disable()
{
	int cmdId = npp_plugin::getCmdId( CMD_DISABLE );
	::SendMessage(npp_plugin::hNpp(), NPPM_SETMENUITEMCHECK, cmdId, true);
}


}  //End namespace:: npp_plugin_changemarker
