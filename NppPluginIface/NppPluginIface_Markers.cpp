/* NppPluginIface_Markers.cpp
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
 *  Notepad++ Plugin Interface Lib extension providing helper functions for implementing
 *  Scintilla LineMarker symbol management and margin control within a Notepad++ plugin.
 *
 */

#include <string>
#include <map>

#include "NppPluginIface_Markers.h"
#include "NppPluginIface_msgs.h"

namespace npp_plugin {

//  Namespace extension for plugin symbol marker management.
namespace markers {

int _mvMarkCheck[NB_MAX_PLUGINMARKERS] = { -1 };  // -1 = hasn't been checked, 0 = not avail, 1 = avail
int _svMarkCheck[NB_MAX_PLUGINMARKERS] = { -1 };
int _availMarkResult[NB_MAX_PLUGINMARKERS] = { -1 };
int _markersNeeded = 0;
int _markersFound = 0;

//  Sends out messages to SciMarkerSymbol to retrieve available marker ids.
void _getNextMarkerType()
{
	namespace msg = npp_plugin::messages;

	static int currMarker = 0;
	static int targetView = 0;
	static int msgTryCount = 0;

	msg::info_MARKERSYMBOL* _info = new msg::info_MARKERSYMBOL( currMarker, targetView );

	CommunicationInfo* comm = new CommunicationInfo;
	comm->internalMsg = msg::NPPP_MSG_MARKERSYMBOL;
	comm->srcModuleName = getModuleName().c_str();
	comm->info = _info;

	bool msgSent = ::SendMessage( hNpp(), NPPM_MSGTOPLUGIN, (WPARAM)TEXT("SciMarkerSymbol.dll"), (LPARAM)comm);

	if ( msgSent ) {
		msgTryCount = 0;
		if ( targetView == SUB_VIEW ) currMarker++;
		targetView = ( targetView == MAIN_VIEW ) ? ( SUB_VIEW ) : ( MAIN_VIEW );
	}
	else {
		msgTryCount++;
		if ( msgTryCount > 100 ) {
			tstring errMsg = TEXT("A communication error has occurred between this plugin (");
			errMsg.append( getModuleBaseName() );
			errMsg.append( TEXT(") and SciMarkerSymbol while configuring markers.\r\n") );
			errMsg.append( TEXT("This could be due to plugin load order, would you like to try again?") );

			int msgboxReply =
				::MessageBox( hNpp(), errMsg.c_str(), TEXT("Plugin Communication Error!"), MB_RETRYCANCEL| MB_ICONWARNING );

			if ( msgboxReply == IDRETRY ) {
				msgTryCount = 0;
				_getNextMarkerType();
			}
		}
		else {
			_getNextMarkerType();
		}
	}		
}

//  Processes the reply message from SciMarkerSymbol.
void _gotMarkerTypeReply( int markerNumber, int markerType, int targetView )
{
	namespace msg = npp_plugin::messages;

	if ( targetView == MAIN_VIEW ) {
		_mvMarkCheck[markerNumber] = ( markerType == SC_MARK_AVAILABLE ) ? ( 1 ) : ( 0 );
	}
	else {
		_svMarkCheck[markerNumber] = ( markerType == SC_MARK_AVAILABLE ) ? ( 1 ) : ( 0 );

		if ( ( markerType == SC_MARK_AVAILABLE ) && ( _mvMarkCheck[markerNumber] == 1 ) ) {
			_markersFound++;
		}
	}

	if ( _markersFound == _markersNeeded ) {
		for ( int i = 0; i < NB_MAX_PLUGINMARKERS; i++ ) {
			if ( _mvMarkCheck[i] >=0 && _svMarkCheck[i] >=0 ) {
				_availMarkResult[i] = ( (_mvMarkCheck[i] == 1) && (_svMarkCheck[i] == 1) ) ? ( 1 ) : ( 0 );
			}
		}
		::messageProc( msg::NPPP_RMSG_GETAVAILABLEMARKERS, 0, (LPARAM)&_availMarkResult );
	}
	else {
		_getNextMarkerType();
	}
}

//  Iterates through markers looking for available ones until the number of markers needed or
//  NB_MAX_PLUGINMARKERS is reached.  Then sends a NPPP_RMSG_MARKERSAVAIL message to the
//  plugins beNotified message handler.
void getAvailableMarkers( int nb_markers_needed )
{
	_markersNeeded = nb_markers_needed;
	_getNextMarkerType();
}

//  Converts a string representation of an Scintilla mark into the numeric value.
int string2marker( tstring szMark )
{
		static std::map<tstring, int> scmark;

		if ( scmark.empty() ) {
		scmark[ TEXT("SC_MARK_CIRCLE") ] = 0;
		scmark[ TEXT("SC_MARK_ROUNDRECT") ] = 1;
		scmark[ TEXT("SC_MARK_ARROW") ] = 2;
		scmark[ TEXT("SC_MARK_SMALLRECT") ] = 3;
		scmark[ TEXT("SC_MARK_SHORTARROW") ] = 4;
		scmark[ TEXT("SC_MARK_EMPTY") ] = 5;
		scmark[ TEXT("SC_MARK_ARROWDOWN") ] = 6;
		scmark[ TEXT("SC_MARK_MINUS") ] = 7;
		scmark[ TEXT("SC_MARK_PLUS") ] = 8;
		scmark[ TEXT("SC_MARK_VLINE") ] = 9;
		scmark[ TEXT("SC_MARK_LCORNER") ] = 10;
		scmark[ TEXT("SC_MARK_TCORNER") ] = 11;
		scmark[ TEXT("SC_MARK_BOXPLUS") ] = 12;
		scmark[ TEXT("SC_MARK_BOXPLUSCONNECTED") ] = 13;
		scmark[ TEXT("SC_MARK_BOXMINUS") ] = 14;
		scmark[ TEXT("SC_MARK_BOXMINUSCONNECTED") ] = 15;
		scmark[ TEXT("SC_MARK_LCORNERCURVE") ] = 16;
		scmark[ TEXT("SC_MARK_TCORNERCURVE") ] = 17;
		scmark[ TEXT("SC_MARK_CIRCLEPLUS") ] = 18;
		scmark[ TEXT("SC_MARK_CIRCLEPLUSCONNECTED") ] = 19;
		scmark[ TEXT("SC_MARK_CIRCLEMINUS") ] = 20;
		scmark[ TEXT("SC_MARK_CIRCLEMINUSCONNECTED") ] = 21;
		scmark[ TEXT("SC_MARK_BACKGROUND") ] = 22;
		scmark[ TEXT("SC_MARK_DOTDOTDOT") ] = 23;
		scmark[ TEXT("SC_MARK_ARROWS") ] = 24;
		scmark[ TEXT("SC_MARK_PIXMAP") ] = 25;
		scmark[ TEXT("SC_MARK_FULLRECT") ] = 26;
		scmark[ TEXT("SC_MARK_LEFTRECT") ] = 27;
		scmark[ TEXT("SC_MARK_CHARACTER") ] = 10000;
	}

	std::map<tstring, int>::const_iterator iter = scmark.find( szMark );

	if( iter == scmark.end() ) {
		return 0;  //  Default to full circle just like Scintilla does.
	}

    return ( iter->second );
}

int string2margin( tstring szMargin )
{
	static std::map<tstring, int> scmargin;

	if ( scmargin.empty() ) {
		scmargin[ TEXT("MARGIN_NONE") ] = -1;
		scmargin[ TEXT("MARGIN_LINENUMBER") ] = 0;
		scmargin[ TEXT("MARGIN_BOOKMARK") ] = 1;
		scmargin[ TEXT("MARGIN_FOLD") ] = 2;
		scmargin[ TEXT("MARGIN_RESERVED") ] = 3;
		scmargin[ TEXT("MARGIN_PLUGIN") ] = 4;
	}

	std::map<tstring, int>::const_iterator iter = scmargin.find( szMargin );

	if( iter == scmargin.end() ) {
		return 1;  //  Default to symbol (bookmark) margin.
	}

    return ( iter->second );
}



//  Increases or decreases the margins width by 'width' amount.
//  Returns false if the markers target margin is not supported.
bool Margin::adjustWidth( int width )
{
	if ( _target == MARGIN_NONE || _target == MARGIN_LINENUMBER ) return ( false );

	//  Store the size information and set the new size information
	_widthOrig = ::SendMessage( svp->_hView, SCI_GETMARGINWIDTHN, _target, 0);
	_widthSetByPlugin = _widthOrig + width;

	::SendMessage( svp->_hView, SCI_SETMARGINWIDTHN, _target, _widthSetByPlugin );

	return ( true );
}

//  Restore the margin width to what it was when set.  If it has changed since being set
//  remove the size this plugin added/removed.
void Margin::restoreWidth()
{
	if ( _target == MARGIN_NONE || _target == MARGIN_LINENUMBER ) return;

	_widthCurrent = ::SendMessage( svp->_hView, SCI_GETMARGINWIDTHN, _target, 0 );
	
	//  Some other plugin or N++ may have reset the size already.  In that case, don't
	// change it again.
	if (! ( _widthCurrent == _widthOrig ) || ( _widthCurrent < _widthSetByPlugin ) ) {

		// Otherwise remove the width we added from the existing width.
		int tmpWidth = ( ( _widthCurrent - _widthSetByPlugin ) + _widthOrig );
		::SendMessage( svp->_hView, SCI_SETMARGINWIDTHN, _target, tmpWidth );
	}
}

//  Initializes Notepad++ and Scintilla to be able to use the markers from information set from
//  the xml configuration data.
int Plugin_Line_Marker::init()
{
	int retVal = 0;  //  this will be the Scintilla maker pointer.

	return retVal;
}

//  Sends Scintilla the parameters to setup a marker.
//
//  xpm must be defined for markerType SC_MARK_PIXMAP.
//  alpha will be calculated from back if the passed value is NULL.
int Plugin_Line_Marker::define(int markerNum, int markerType, COLORREF fore, COLORREF back,
						  char *xpm = NULL, int alpha = NULL)
{
	int retVal = SendMessage( margin.svp->_hView, SCI_MARKERDEFINE, markerNum, markerType );

	if ( (markerType == SC_MARK_PIXMAP) && xpm ) {
		SendMessage( margin.svp->_hView, SCI_MARKERDEFINEPIXMAP, markerNum, (LPARAM) xpm);
	}

	//  These colors are used when marker is treated as a highlight instead of symbol.  Which
	//  can happen when the margin is hidden.
	SendMessage( margin.svp->_hView, SCI_MARKERSETFORE, markerNum, fore);
	SendMessage( margin.svp->_hView, SCI_MARKERSETBACK, markerNum, back);

	// Calculate and assign an alpha blend based on the background color.
	if ( ( markerType != SC_MARK_PIXMAP ) || ( (markerType == SC_MARK_PIXMAP) && (! alpha) ) ) {
		int R = GetRValue( back );
		int G = GetGValue( back );
		int B = GetBValue( back );
		int iMax = max(R, max(G, B));
		int iMin = min(R, min(G, B));
		alpha = 255 - (((iMax + iMin) * 240 + 255) / 510);
	}
	SendMessage( margin.svp->_hView, SCI_MARKERSETALPHA, markerNum, alpha);

	return ( retVal );
}

} //  End namespace: marker

}  // End Namespace: npp_plugin
