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
#include <unordered_map>
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

//  Requires communication with NppPlugin_SciMarkerSymbol.dll!
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

//  Sends message to SciMarkerSymbols.dll to undefine a Scintilla marker for both Npp views and
//  remove any existing markers from margins.
void setMarkerAvailable( int markerNumber )
{
	namespace msg = npp_plugin::messages;

	CommunicationInfo comm;
	comm.internalMsg = msg::NPPP_MSG_MARKERUNDEFINE;
	comm.srcModuleName = getModuleName()->c_str();
	msg::info_MARKERSYMBOL _info( markerNumber , NULL );

	for ( int currView = MAIN_VIEW; currView <= SUB_VIEW; currView++ ) {
		_info.targetView = currView;
		comm.info = &_info;
		::SendMessage( hNpp(), NPPM_MSGTOPLUGIN, 
				(WPARAM)TEXT("NppPlugin_SciMarkerSymbol.dll"), (LPARAM)&comm );
	}
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

//  Converts a string representation of a Notepad++ margin into the numeric value. 
int string2margin( tstring szMargin )
{
	static std::map<tstring, int> scmargin;

	if ( scmargin.empty() ) {
		scmargin[ TEXT("MARGIN_NONE") ] = MARGIN_NONE;
		scmargin[ TEXT("MARGIN_LINENUMBER") ] = MARGIN_LINENUMBER;
		scmargin[ TEXT("MARGIN_BOOKMARK") ] = MARGIN_BOOKMARK;
		scmargin[ TEXT("MARGIN_FOLD") ] = MARGIN_FOLD;
		scmargin[ TEXT("MARGIN_CHANGES") ] = MARGIN_CHANGES;
		scmargin[ TEXT("MARGIN_PLUGIN") ] = MARGIN_PLUGIN;
	}

	std::map<tstring, int>::const_iterator iter = scmargin.find( szMargin );

	if( iter == scmargin.end() ) {
		return 1;  //  Default to symbol (bookmark) margin.
	}

    return ( iter->second );
}

//  Returns the string representation of a Notepad++ margin enum
//  Converts a string representation of a Notepad++ margin into the numeric value. 
tstring margin2string( int margin )
{
	static std::map<int, tstring> scmarginstrings;

	if ( scmarginstrings.empty() ) {
		scmarginstrings[ MARGIN_NONE ] = TEXT("MARGIN_NONE");
		scmarginstrings[ MARGIN_LINENUMBER ] = TEXT("MARGIN_LINENUMBER");
		scmarginstrings[ MARGIN_BOOKMARK ] = TEXT("MARGIN_BOOKMARK");
		scmarginstrings[ MARGIN_FOLD ] = TEXT("MARGIN_FOLD");
		scmarginstrings[ MARGIN_CHANGES ] = TEXT("MARGIN_CHANGES");
		scmarginstrings[ MARGIN_PLUGIN ] = TEXT("MARGIN_PLUGIN");
	}

	std::map<int, tstring>::const_iterator iter = scmarginstrings.find( margin );

	if( iter == scmarginstrings.end() ) {
		return NULL;
	}

    return ( iter->second );
}

//  Sets the margin this plugin's markers will be shown in by setting the target margin and
//  setting the mask on markers so the marker will be properly assigned.
void Margin::setTarget( MARGIN target, int markerID )
{
	if ( (target == MARGIN_FOLD) ) return;
	
	_prevTarget = _target;
	_target = target;

	setMasks ( markerID );
}

//  Sets the mask for all margins in both views so the marker is on the target margin.
void Margin::setMasks( int markerID )
{
	int markerMask = ( 1 << markerID );
	int tmpMask = 0;

	for ( int currView = MAIN_VIEW; currView <= SUB_VIEW; currView++ ) {
		for ( int i = 0; i < NB_MARGINS; i++ ) {
			tmpMask = ::SendMessage( hViewByInt(currView), SCI_GETMARGINMASKN, i, 0 );
			if ( i == _target ) {
				// set mask to include marker
				tmpMask |= markerMask;
			}
			else {
				// set mask to exclude marker
				tmpMask &= ~markerMask;
			}
			::SendMessage( hViewByInt(currView), SCI_SETMARGINMASKN, i, tmpMask );
		}
	}
}

//  Unsets the mask for all margins in both views so the marker is on the previous target margin.
void Margin::restorePrevTarget( int markerID ) { setTarget( _prevTarget, markerID ); }

//  Initializes Notepad++ and Scintilla to be able to use the markers.
void Plugin_Line_Marker::init( int markNum )
{
	id = markNum;

	if ( type == SC_MARK_PIXMAP && pXpmFields.empty() ) {
		if ( xpmFileName.empty() ) type = 0; // Same default as Scintilla
		else if (! getXpmDataFile() ) type = 0;
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
		HWND hView = npp_plugin::hViewByInt( currView );
		if ( type == SC_MARK_PIXMAP ) {
			SendMessage( hView, SCI_MARKERDEFINEPIXMAP, id, (LPARAM)getXpmData() );
		}
		else {
			SendMessage( hView, SCI_MARKERDEFINE, id, type );
		}
		SendMessage( hView, SCI_MARKERSETFORE, id, fore);
		SendMessage( hView, SCI_MARKERSETBACK, id, back);
		SendMessage( hView, SCI_MARKERSETALPHA, id, alpha);
	}
}

//  Gets the file pointer for the XPM data from a .xpm file in the ..config\icons\ directory.
bool Plugin_Line_Marker::getXpmDataFile()
{
	//  Before defining the marker see about getting the xpm data (if needed)

	//  First try the application data folder.
	TCHAR filePath[MAX_PATH];
	::SendMessage( npp_plugin::hNpp(), NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)filePath );
	PathAppend( filePath, TEXT("\\..\\icons") );
	PathAppend( filePath, xpmFileName.c_str() );

	if (!PathFileExists(filePath)) {
		lstrcpyn( filePath, TEXT("\0"), MAX_PATH );
		::SendMessage( npp_plugin::hNpp(), NPPM_GETNPPDIRECTORY, MAX_PATH, (LPARAM)filePath );
		PathAppend( filePath, TEXT("plugins\\icons") );
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

	if(! XpmFile2Buffer( filePath ) ) return ( false );

	return ( true );
}

//  Reads the XPM text fields to a vector for later transmission to Scintilla.
int Plugin_Line_Marker::XpmFile2Buffer( TCHAR filename[MAX_PATH] )
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

//  Returns the pointer array to the XPM data to send to Scintilla.
const char** Plugin_Line_Marker::getXpmData() { return ( &pXpmFields[0] ); }

} //  End namespace: marker

}  // End Namespace: npp_plugin
