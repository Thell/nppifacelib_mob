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

//  <--- STL --->
#include <string>
#include <map>
#include <fstream>

//  <--- Windows --->
#include "Shlwapi.h"
#pragma comment( lib, "Shlwapi.lib" )

#include "NppPluginIface_Markers.h"
#include "NppPluginIface_msgs.h"

namespace npp_plugin {

//  Namespace extension for plugin symbol marker management.
namespace markers {

int _availMarkResult[NB_MAX_PLUGINMARKERS];

//  Iterate through markers looking for available ones until the number of markers needed or
//  NB_MAX_PLUGINMARKERS is reached.
//  Returns array with values: -1 = hasn't been checked, 0 = not avail, 1 = avail
int * getAvailableMarkers( int nb_markers_needed )
{
	namespace msg = npp_plugin::messages;

	_availMarkResult[NB_MAX_PLUGINMARKERS];
	for (int i = 0; i< NB_MAX_PLUGINMARKERS; i++) _availMarkResult[i] = -1;

	if ( nb_markers_needed > NB_MAX_PLUGINMARKERS ) return _availMarkResult;

	CommunicationInfo comm;
	comm.internalMsg = msg::NPPP_MSG_MARKERSYMBOL;
	comm.srcModuleName = getModuleName()->c_str();
	msg::info_MARKERSYMBOL _info(NULL, NULL);

	int mvSymbol;
	int svSymbol;
	int foundMarks = 0;
	for ( int currMark = 0; currMark < NB_MAX_PLUGINMARKERS; currMark++ ) {
		_info.markerNumber = currMark;

		_info.targetView = MAIN_VIEW;
		comm.info = &_info;
		bool mvMsg = ::SendMessage( hNpp(), NPPM_MSGTOPLUGIN, 
				(WPARAM)TEXT("NppPlugin_SciMarkerSymbol.dll"), (LPARAM)&comm );
		if ( mvMsg ) mvSymbol = _info.markerSymbol;

		_info.targetView = SUB_VIEW;
		comm.info = &_info;
		bool svMsg = ::SendMessage( hNpp(), NPPM_MSGTOPLUGIN, 
				(WPARAM)TEXT("NppPlugin_SciMarkerSymbol.dll"), (LPARAM)&comm );
		if ( svMsg ) svSymbol = _info.markerSymbol;

		if ( mvMsg && svMsg ) {
			if ( ( mvSymbol == SC_MARK_AVAILABLE ) && ( svSymbol == SC_MARK_AVAILABLE ) ) {
				_availMarkResult[currMark] = 1;
				foundMarks++;
			}
			else {
				_availMarkResult[currMark] = 0;
			}
		}

		if ( foundMarks == nb_markers_needed ) break;
	}

	return ( _availMarkResult );
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
bool Margin::adjustWidth( int width, int view )
{
	if ( _target == MARGIN_NONE || _target == MARGIN_LINENUMBER ) return ( false );

	//  Store the size information and set the new size information
	_widthOrig = ::SendMessage( svp[view]->_hView, SCI_GETMARGINWIDTHN, _target, 0);
	_widthSetByPlugin = _widthOrig + width;

	::SendMessage( svp[view]->_hView, SCI_SETMARGINWIDTHN, _target, _widthSetByPlugin );

	return ( true );
}

//  Restore the margin width to what it was when set.  If it has changed since being set
//  remove the size this plugin added/removed.
void Margin::restoreWidth( int view )
{
	if ( _target == MARGIN_NONE || _target == MARGIN_LINENUMBER ) return;

	_widthCurrent = ::SendMessage( svp[view]->_hView, SCI_GETMARGINWIDTHN, _target, 0 );
	
	//  Some other plugin or N++ may have reset the size already.  In that case, don't
	// change it again.
	if (! ( _widthCurrent == _widthOrig ) || ( _widthCurrent < _widthSetByPlugin ) ) {

		// Otherwise remove the width we added from the existing width.
		int tmpWidth = ( ( _widthCurrent - _widthSetByPlugin ) + _widthOrig );
		::SendMessage( svp[view]->_hView, SCI_SETMARGINWIDTHN, _target, tmpWidth );
	}
}

//  Initializes Notepad++ and Scintilla to be able to use the markers from information set from
//  the xml configuration data.
void Plugin_Line_Marker::init( int markNum )
{
	id = markNum;

	if ( type == SC_MARK_PIXMAP && pXpmFields.empty() ) {
		if ( xpmFileName.empty() ) type = 0; // Same default as Scintilla
		else if (! readXpmDataFile() ) type = 0;
	}

	// Calculate and assign an alpha blend based on the background color.
	// Unless it is already within range.
	if ( ( alpha < 0 ) || ( alpha > 255 ) ) {
		int R = GetRValue( back );
		int G = GetGValue( back );
		int B = GetBValue( back );
		int iMax = max(R, max(G, B));
		int iMin = min(R, min(G, B));
		alpha = 255 - (((iMax + iMin) * 240 + 255) / 510);
	}
	
	for ( int currView = MAIN_VIEW; currView <= SUB_VIEW; currView++ ) {
		if ( type == SC_MARK_PIXMAP ) {
			SendMessage( margin.svp[currView]->_hView, SCI_MARKERDEFINEPIXMAP, id, (LPARAM)getXpmData() );
		}
		else {
			SendMessage( margin.svp[currView]->_hView, SCI_MARKERDEFINE, id, type );
		}
		SendMessage( margin.svp[currView]->_hView, SCI_MARKERSETFORE, id, fore);
		SendMessage( margin.svp[currView]->_hView, SCI_MARKERSETBACK, id, back);
		SendMessage( margin.svp[currView]->_hView, SCI_MARKERSETALPHA, id, alpha);
	}
}

//  Read XPM data from a .xpm file.
bool Plugin_Line_Marker::readXpmDataFile()
{
	//  Before defining the marker see about getting the xpm data (if needed)

	//  First try the application data folder.
	TCHAR filePath[MAX_PATH];
	::SendMessage( npp_plugin::hNpp(), NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)filePath );
	PathAppend( filePath, TEXT("\\icons") );
	PathAppend( filePath, xpmFileName.c_str() );

	if (!PathFileExists(filePath)) {
		lstrcpyn( filePath, TEXT("\0"), MAX_PATH );
		::SendMessage( npp_plugin::hNpp(), NPPM_GETNPPDIRECTORY, MAX_PATH, (LPARAM)filePath );
		PathAppend( filePath, TEXT("plugins\\Config\\icons") );
		PathAppend( filePath, xpmFileName.c_str() );

		if (!PathFileExists(filePath)) {
			tstring msgText;
			msgText.assign( TEXT("The marker configuration file ") );
			msgText.append( xpmFileName );
			msgText.append( TEXT(" for the '") );
			msgText.append( npp_plugin::getName() );
			msgText.append( TEXT("' plugin was not found!\n") );

			::MessageBox( npp_plugin::hNpp(), msgText.c_str() , TEXT("Plugin File Load Error"), MB_ICONERROR );

			this->type = 0;
			return ( false );
		}
	}

	if(! XpmReadFileToBuffer( filePath ) ) return ( false );

	return ( true );
}


int Plugin_Line_Marker::XpmReadFileToBuffer( TCHAR filename[MAX_PATH] )
{
	std::fstream xpmFile(filename);
	std::string tmpXpmBuff;

	while(! xpmFile.eof() ) {
		xpmFile.ignore(100, '\"');
		xpmFile.putback('\"');
		std::getline( xpmFile, tmpXpmBuff, xpmFile.widen(',') );

		if ( xpmFile.eof() ) {
			tmpXpmBuff = tmpXpmBuff.substr(0, tmpXpmBuff.find_last_of('\"') + 1 );
		}

		tmpXpmBuff = tmpXpmBuff.substr( 1, tmpXpmBuff.length() - 2 );
		xpmFields.push_back(tmpXpmBuff);
		tmpXpmBuff.clear();
	}

	pXpmFields.reserve(xpmFields.size());
	std::vector<std::string>::iterator insIter = xpmFields.begin();
	while ( insIter != xpmFields.end() ) {
		pXpmFields.push_back( insIter->c_str() );
		insIter++;
	}

    return ( !xpmFile.fail() );
}


const char** Plugin_Line_Marker::getXpmData()
{
	const char ** test = &pXpmFields[0];

	return ( &pXpmFields[0] );
}

} //  End namespace: marker

}  // End Namespace: npp_plugin
