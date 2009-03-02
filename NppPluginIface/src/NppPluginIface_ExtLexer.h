/* NppPluginIface_ExtLexer.h
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
 *  Scintilla External Lexers within a Notepad++ plugin.
 *
 */

#ifndef NPP_PLUGININTERFACE_EXTLEXER_EXTENSION_H
#define NPP_PLUGININTERFACE_EXTLEXER_EXTENSION_H

#include "NppPluginIface.h"
								/*
								 *  Defines the interface for interacting with Notepad++
								 *  Includes:
								 *    # <vector>
								 *    # <algorithm>
								 *    # <iterator>
								 *    # "PluginInterface.h"
								 *
								 *  These files define most items needed for plugin messaging
								 *  and setup/registration with Notepad++'s plugin manager.
								 *
								 */

#include "Platform.h"
								/*
								 *  Scintilla platform definitions
								 *  Required for Window Accessor
								 *  No additional includes come from this file.
								 *
								 */

#include "SciLexer.h"			//  Included for SCLEX_AUTOMATIC; no additional includes.


//  Required Exported LexOrFold Function Definition.
//  Export defined in NppPluginIface_ExtLexer.def
typedef void (*NppExtLexerFunction)(bool LexOrFold, unsigned int startPos, int lengthDoc, int initStyle,
                  char *words[], WindowID window, char *props);

namespace npp_plugin {

//  Namespace Extension for External Lexer Interface
namespace external_lexer {

//  Struct for storing plugin's external lexers data.
struct Lexer
{
	std::string _name;			// Use of char instead of TCHAR since Scintilla expects char.
	tstring _description;
	NppExtLexerFunction _pLexOrFold;
	int SCI_LEXERID;
};

//  <--- Initialization --->
void initLexer(std::string Name, tstring statusText, NppExtLexerFunction pLexOrFold,
			   PFUNCPLUGINCMD pMenuDlg);		//  Setup a lexer definition.
void setLexerFuncItem(tstring Name, PFUNCPLUGINCMD pFunction, int cmdID = NULL,
				bool init2Check = false, ShortcutKey* pShKey = NULL);	//  Store additional lexer FuncItem commands.

//  <--- Data Retrieval --->
int getSCILexerIDByIndex( int index );			//  Returns the Scintilla lexer ID for this vector index.
int getSCILexerIDByName( std::string name );	//  Returns the Scintilla lexer ID for vector index matching name.
std::vector<FuncItem> getLexerFuncVector();		//  Return a copy of this extensions Function vector.
std::vector<Lexer> getLexerDetailVector();		//  Return a copy of this extensions Lexer vector.

//  'Virtualized' base plugin FuncItem functions. 
namespace virtual_plugin_func {

int getPluginFuncCount();						//  'Virtualize' the base plugin's getPluginFuncCount() function.
FuncItem * getPluginFuncArray();				//  'Virtualize' the base plugin's getPluginFuncArray() function.

}  // End Namespace: virtual_plugin_func

}  // End Namespace: external_lexer

}  // End Namespace: npp_plugin.

#endif  // End include guard: NPP_PLUGININTERFACE_EXTLEXER_EXTENSION_H