/* NppPlugin_PluginMargin.cpp
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

//  Plugin Margin show/hide view control for use with Notepad++.

#include "NppPlugin_PluginMargin.h"


//  N++ Change Marker Plugin Specific
namespace npp_plugin_pluginmargin {

	using namespace npp_plugin;
	namespace xml = npp_plugin::xmlconfig;

//  Un-named namespace for private functions.
namespace {

bool showMargin( int view )
{
	return ( xml::getGUIConfigValue( getViewString( view ) , TEXT("pluginMargin") ) == TEXT("show") );
}

//  Returns true if margin should now be showing.
bool toggleMargin( int view )
{
	//  Get the config files setting for the margin.
	bool showing = (
		xml::getGUIConfigValue( getViewString( view ) , TEXT("pluginMargin") ) == TEXT("show") );

	// Verify that another plugin didn't alter the margin manually.
	int currWidth = ::SendMessage( hViewByInt( view ), SCI_GETMARGINWIDTHN, 4, 0 );

	//  If another plugin altered the show/hide manually re-synch.
	if (! (( (showing)&&(currWidth > 0) ) || ( (!showing)&&(currWidth == 0) )) ) showing = !showing;

	//  Set the new value.
	tstring newValue;
	newValue = ( showing ) ? ( TEXT("hide") ) : ( TEXT("show") );
	xml::setGUIConfigValue( getViewString( view ), TEXT("pluginMargin"), newValue );

	initPluginMargin();

	return (! showing );
}

} //  End: unnamed namespace.


void initPluginMargin()
{
	//  Set menu items
	for ( int view = MAIN_VIEW; view <= SUB_VIEW; view++ ) {
		int cmdId;
		cmdId = getCmdId( view + 1 );
		if ( showMargin( view ) ) {
			::SendMessage( hNpp(), NPPM_SETMENUITEMCHECK, cmdId, true );
			::SendMessage( hViewByInt( view ), SCI_SETMARGINWIDTHN, 4, 16 );
		}
		else {
			::SendMessage( hNpp(), NPPM_SETMENUITEMCHECK, cmdId, false );
			::SendMessage( hViewByInt( view ), SCI_SETMARGINWIDTHN, 4, 0 );
		}
	}
}

void toggleMV()
{
	toggleMargin( MAIN_VIEW );
}

void toggleSV()
{
	toggleMargin( SUB_VIEW );
}

} // End namespace: npp_plugin_pluginmargin