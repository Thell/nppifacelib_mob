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
	if ( xml::getGUIConfigValue( TEXT("SciMarkers"), TEXT("active") ) == TEXT("true") _active = true;

	int _margin = mark::string2margin( xml::getGUIConfigValue( TEXT("SciMarkers"), TEXT("margin") ) );

	//  N++ Plugin Margin Setting ( there should be an external control for this!)
	bool _mvMarginShow =
		( xml::getGUIConfigValue( TEXT("MAIN_VIEW"), TEXT("pluginMargin") ) == TEXT("show") ) ? ( true ) : ( false );
	bool _svMarginShow = 
		( xml::getGUIConfigValue( TEXT("SUB_VIEW"), TEXT("pluginMargin") ) == TEXT("show") ) ? ( true ) : ( false );

	//  Marker Specific Settings
	for ( int i = 0; i < NB_CHANGEMARKERS; i++ {
		//  ID Settings.
		cm[i]->markName = ( i == CM_SAVED ) ? ( TEXT("CM_SAVED") ) : ( TEXT("CM_NOTSAVED") );
		cm[i]->styleName = ( i == CM_SAVED ) ? ( TEXT("Changes: Saved") ) : ( TEXT("Changes: Not Saved") );

		//  Style Settings.
		cm[i]->fore = 
			::_tcstol( (LPCTSTR)(xml::getGUIConfigValue( cm[i]->styleName, TEXT("fgColor") ).c_str() ), NULL, 16 );
		cm[i]->back = 
			::_tcstol( (LPCTSTR)(xml::getGUIConfigValue( cm[i]->styleName, TEXT("bgColor") ).c_str() ), NULL, 16 );

		//  Marker View Settings
		cm[i]->active = _active;
		cm[i]->display =
			( xml::getGUIConfigValue( cm[i]->markName, TEXT("displayMark") ) == TEXT("true") ) ? ( true ) : ( false );
		cm[i]->type = mark::string2marker( xml::getGUIConfigValue( cm[i]->markName, TEXT("markType") ) );
		if ( cm[i]->type == SC_MARK_PIXMAP ) {
			cm[i]->xpm.assign( xml::getGUIConfigValue( cm[i]->markName, TEXT("xpm") ) );

			// This should be in the marker extension.
			if (! cm[i]->xpm.compare( cm[i]->xpm.length() - 4, 4, TEXT(".xpm") == 0 ) cm[i]->xpm.append( TEXT(".xpm") );
		}

		//  Margin View Settings
		cm[i]->margin.svp[MAIN_VIEW]._pluginMarginShow = _mvMarginShow;
		cm[i]->margin.svp[SUB_VIEW]._pluginMarginShow = _svMarginShow;
	}

	//  Now that all the config values are set...
	if (! _track ) {
		npp_plugin_changemarker::disable();
	}
	else {
		//  Call for marker IDs to initialize.
		mark::MARKERGETAVAIL( NB_CHANGEMARKERS );
	}

}

//  Initalizes the marker reserved for this plugin.
void initMarker( int markerNumber[] )
{
	if ( markerNumber[0] = -1 ) {
		//  no available markers where identified!
		//  guess we use '0'? for them?
	}
	else {
		for ( int i = 0, i < NB_MAX_PLUGINMARKERS; i++ ) {
			if (! markerNumber[i] == 0 ) {
				//  initialize this marker
			}
		}
	}
}

//  Checks and updates marker styles when notified.
void wordStylesUpdatedHandler()
{
	//  Check for style updates

	//  Change styles if needed.
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
