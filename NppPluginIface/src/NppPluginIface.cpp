/* NppPluginIface.cpp
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
 *  Notepad++ Plugin Interface Lib enhances the standard Notepad++ plugin interface defined in
 *  Notepad++'s PluginInterface.h.
 *
 *  Using the namespace 'npp_plugin' for public functions, and nested unnamed namespaces 
 *  for private ones.  Extensions to the lib are done with nested namespaces.
 *
 */

//  <--- STL --->
#include <algorithm>            //  Utility functions for working with the vectors.
#include <iterator>             //  Iterator for merging the vectors, and crtdgb.h for _ASSERT.

//  <--- Windows --->
#include "Shlwapi.h"
#pragma comment( lib, "Shlwapi.lib" )

# include "NppPluginIface.h"	//  Provides definitions for the public plugin interface.

namespace npp_plugin {	

//  <--- Unnamed namespace for private plugin variables --->
namespace {

//  <--- Npp Data --->
tstring _Name;					//  Name passed to NPP for use in the 'Plugins' menu.
tstring _ModuleName;			//	Module name with the .dll extension.
tstring _ModuleBaseName;		//  Moudle name without the .dll extension.
HANDLE _hModule;				//  Notepad++'s handle for this plugin module.
HWND _hNpp;						//  Handle to Notepad++ for messaging.
HWND _hScintillaMain;			//  Handle to Notepad++'s main view for messaging.
HWND _hScintillaSecond;			//  Handle to Notepad++'s second view for messaging.
HWND _hCurrView;				//  Handle to the currently active Notepad++ view for messaging.
bool _bRefreshCurrViewH;		//  Indicates if hCurrView handle needs updating.
bool _isReady;					//  Flag if Notepad++ has finished initalizing.

//  <--- Function item vectors --->
std::vector<FuncItem> _PluginFuncVector;	//  Container for this plugin's function commands.

//  <--- Private functions --->
#ifdef UNICODE 

//  Notepad++ Required Function
//  This tells N++ if plugin was compiled for unicode or not.
extern "C" __declspec(dllexport) BOOL isUnicode() {	return TRUE;}

#endif	//  End: UNICODE

//  Notepad++ Required Function
//  N++ PluginsManager forwards handle information for use in messaging.
extern "C" __declspec(dllexport) void setInfo(NppData NppData)
{
	_ASSERT(!_hNpp);		//  Assert that these handles haven't already been set.

	_hNpp = NppData._nppHandle;
	_hScintillaMain = NppData._scintillaMainHandle;
	_hScintillaSecond = NppData._scintillaSecondHandle;
}

//  Update handle pointer to the currently active view.
HWND updateCurrViewHandle()
{
	static HWND hPrevCurrView;
	_bRefreshCurrViewH = false;

	int currentEdit;
	::SendMessage(_hNpp, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
	
	if ( currentEdit != -1 ) {
		hPrevCurrView = ( currentEdit == 0 ) ? ( _hScintillaMain ): ( _hScintillaSecond );
	}
	return ( hPrevCurrView );
}

}  // End: Unnamed namespace for private variables


//   Notepad++ Plugin Initialization:
// 
//   A plugin is really a singleton from the point of view of the plugin so control over
//   initialization and variable updates is required, this should happen in your sources'
//   DllMain function.
void initPlugin(tstring name, HANDLE hModule)
{
	_ASSERT( _Name.empty() );			//  Assert that these values haven't already been set.
	_ASSERT( _Name.size() <= nbChar );	//  Ensure this plugin stays within name size limits.
							
	_Name.assign(name);

	TCHAR tmpName[MAX_PATH];
	::GetModuleFileName( (HMODULE)hModule, tmpName, MAX_PATH );
	PathStripPath( tmpName );
	_ModuleName.assign( tmpName );
	PathRemoveExtension( tmpName );
	_ModuleBaseName.assign( tmpName );

	_hModule = hModule;
}

//  This function stores Function Items to the FuncItem vector for retrieval by N++.
void setPluginFuncItem(tstring Name, PFUNCPLUGINCMD pFunction,
											 int cmdID, bool init2Check, ShortcutKey* pShKey)
{
	/*
	 *  If three basic entries 'separator', 'help', and 'about' are stored as PluginFuncItems
	 *  and an extension to this namespace provides another FuncItems vector, the two can
	 *  be merged while at the same time ensuring the main plugins menu functions are at
	 *  the bottom (or top) of the plugin's menu in Npp.
	 *  
	 */

	// Notify if length is too long.
	if ( !( Name.length() < nbChar ) )
		::MessageBox(npp_plugin::hNpp(),
					TEXT("Function name is too long and will be truncated."),
					TEXT("Function Item Name Alert"),
					MB_ICONINFORMATION);

	FuncItem thisFunction;

	Name.copy(thisFunction._itemName, Name.length());
	thisFunction._itemName[ Name.length() ] = '\0';

	thisFunction._cmdID = cmdID;
	thisFunction._pFunc = pFunction;
	thisFunction._init2Check = init2Check;
	thisFunction._pShKey = pShKey;

	_PluginFuncVector.push_back(thisFunction);
}

//  Returns this plugin module's handle.
HANDLE hModule() { return _hModule; }

//  Returns this module's full name.
tstring* getModuleName() { return &_ModuleName; }

//  Returns this module's name without the .dll extension.
tstring* getModuleBaseName() { return &_ModuleBaseName; }

//  Returns the main Notepad++ handle.
HWND hNpp() { return _hNpp; }

//  Returns the handle used by the view with matching int.  ( MAIN_VIEW or SUB_VIEW )
HWND hViewByInt(int view) { return ( ( view == 0 ) ? ( _hScintillaMain ) : ( _hScintillaSecond ) ); }

//  Returns the string value name of the matching int view.
tstring getViewString( int view ) { return ( ( view == 0 ) ? ( TEXT("MAIN_VIEW") ) : ( TEXT("SUB_VIEW") ) ); }

//  Returns the handle used in messaging Notepad++'s main view.
HWND hMainView() { return _hScintillaMain; }

//  Returns handle used in messaging Notepad++'s secondary view.
HWND hSecondView() { return _hScintillaSecond; }

//  Returns handle for the currently focused Notepad++ view.
//  Call hCurrViewNeedsUpdate() prior to this function to force an update of the handle
//  during run-time.
/*
 *  Use hMainView and hSecondView instead of the hCurrView when a message must reach both views.
 *  For instance a lexer plugin that works with indicators or markers and makes a change to
 *  a definition value will only have it updated in the view the message is sent to.
 *
 */
HWND hCurrView()
{
	if ( _bRefreshCurrViewH || !_isReady ) {
		_hCurrView = updateCurrViewHandle();
	}
	return _hCurrView;
}

//  Returns the handle for the Notepad++ view not currently focused.
HWND hAltView() { return ( ( hCurrView() == _hScintillaMain ) ? ( _hScintillaSecond ) : ( _hScintillaMain ) ); }

//  Returns the current target of hCurrView().  0 for main view and 1 for second view.
int intCurrView() { return ( ( hCurrView() == _hScintillaMain ) ? ( 0 ) : ( 1 ) ); }

//  Sets flag to force update of view handle upon next call of hCurrView()
void hCurrViewNeedsUpdate() { _bRefreshCurrViewH = true; }

//  Returns Notepad++'s initialization state.
//
//  Until setNppReady() is called this will return false which causes hCurrView() to forcefully
//  update the current view's handle on each call.
bool isNppReady() { return _isReady; }

// Signify that N++ has finished all startup activities.
// The NppReady flag is useful in controlling a plugins activites during Npp startup.
// hCurrView() forcefully updates the current view's handle on each call until this is set.
void setNppReady() { _isReady = true; }


//  Provides a response to PluginsManager's getFuncArray call and allows for extensions
//  to work with generating an array to send back that includes their own FuncItems.
FuncItem * getPluginFuncArray() { return ( &_PluginFuncVector[0] ); }

//  Provides a response for a namespace's extension to retrieve and work with the plugin's
//  function item vector.  This is useful in merging and ordering function items together
//  before responding to the Npp plugin manager's getFuncArray() call.
//
//	  * See NppPluginIface_ExtLexer for an example.
//
std::vector<FuncItem> getPluginFuncVector() { return ( _PluginFuncVector ); }

//  Provides a response to PluginsManager's getFuncArray call and allows for extensions
//  to work with generating an array to send back that includes their own FuncItems.
int getPluginFuncCount() { return _PluginFuncVector.size(); }

//  <---  Required Exported Function --->
//  Returns this plugins registered name.
//
//  PluginsManager retrieves this name for messaging and function pointer control.
extern "C" __declspec(dllexport) const TCHAR * getName() { return &_Name[0]; }

}  //  End namespace: NppPluginIface
