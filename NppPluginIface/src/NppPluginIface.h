/* NppPluginIface.h
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
 *
 */

/*
 *  Notepad++ Plugin Interface Lib enhances the standard Notepad++ plugin interface defined in
 *  Notepad++'s PluginInterface.h.
 *
 *  Using the namespace 'npp_plugin' for public functions, and nested unnamed namespaces 
 *  for private ones.  Extensions to the lib are done with nested namespaces.
 *
 */

#ifndef NPP_PLUGININTERFACE_H
#define NPP_PLUGININTERFACE_H

// <--- STL --->
#include <stdlib.h>
#include <vector>               //  Dynamic array container for Function Items.
#include <algorithm>            //  Utility functions for working with the vectors.
#include <iterator>             //  Iterator for merging the vectors, and crtdgb.h for _ASSERT.
#include <TCHAR.h>				//  Provides MS specific routines for working with Unicode/ANSI.
#include <string>

// <--- Notepad++ --->
#include "PluginInterface.h"
								/*
								 *  .\..\..\PowerEditor\src\MISC\PluginsManager
								 *
								 *  Defines the interface for interacting with Notepad++
								 *  Includes:
								 *    # <windows.h> Master include for Windows.
								 *    # "Scintilla.h" Main include for Scintilla definitions.
								 *    # "Notepad_plus_msgs.h" Main include file for Notepad++
								 *       messaging
								 *
								 *  These files define most items needed for plugin messaging.
								 *
								 */

namespace npp_plugin {

//  Npp uses either ansi or unicode, we might as well have a basic TCHAR string type.
typedef std::basic_string< TCHAR > tstring;

//  Might as well also have a MACRO to define which memcpy we'll use when needed.
#ifdef UNICODE
	#define TMEMCPY wmemcpy
#else
	#define TMEMCPY memcpy
#endif

//  <--- Initialization Functions --->
void initPlugin(tstring name, HANDLE hModule);	//  Set the module name, and save the module handle.
void setPluginFuncItem(tstring Name, PFUNCPLUGINCMD pFunction, int cmdID = NULL,
	   bool init2Check = false, ShortcutKey* pShKey = NULL); //  Store a FuncItem command.
void setNppReady();								//  Sets flag when Notepad++ is done with start up.
void hCurrViewNeedsUpdate();					//  Sets flag to update hCurrView.

//  <--- Data Retrieval Functions --->
HANDLE hModule();								//  Returns the plugin module's handle.
tstring* getModuleName();						//  Returns the dll modules proper name.
tstring* getModuleBaseName();					//  Returns the dll modules proper name without the .dll extension.
HWND hNpp();									//  Returns the main Notepad++ handle.
HWND hViewByInt(int view);						//  Returns handle for the view by int.
tstring getViewString(int view);				//  Returns string value of a views name.
HWND hMainView();								//  Returns handle used in messaging the main view.
HWND hSecondView();								//  Returns handle used in messaging the secondary view.
HWND hCurrView();								//  Returns handle for the currently focused view.
HWND hAltView();								//  Returns handle for the non-focused view.
int intCurrView();								//  Returns 0 for main view and 1 for second view.
bool isNppReady();								//  Returns Notepad++ initialization state.
int getPluginFuncCount();						//  Returns the size of the Function vector.
std::vector<FuncItem> getPluginFuncVector();	//  Return a copy of the plugins function vector.
FuncItem * getPluginFuncArray();				//  Returns this plugin's Function vector as an array.

//  <--- Required Exported Functions --->
extern "C" __declspec(dllexport) const TCHAR * getName();

}  // End Namespace: npp_plugin

#endif // End include guard:  NPP_PLUGININTERFACE_H