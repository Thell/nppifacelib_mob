#ifndef NPPEXTLEXER_MYUSERLANG_H
#define NPPEXTLEXER_MYUSERLANG_H

// NppExtLexer_MYUSERLANG.h

// This file is part of the Notepad++ External Lexers Plugin.
// Copyright 2008 - 2009 Thell Fowler (thell@almostautomated.com)
//
// This program is free software; you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


///////////////////////////////////////////////////////////////////////////////////////////////
//           Example Lexer Definition of Conversion from User Defined Language
//                     for use with Notepad++ External Lexers Plugin
///////////////////////////////////////////////////////////////////////////////////////////////

	/***
	 *
	 *  Some interesting notes for other NPP lexer writers:
	 *
	 *    > Notepad ++ reads up to 31 styles per language from the styling xml language node
	 *      which are available in the style preferences dialog for configuration.
	 *
	 *    > Scintilla can use 255 styles with styles 32 to 39 reserved for itself.
	 *
	 *    > Scintilla also allows for a maximum of 31 indicators to be used, with the first 7
	 *      set aside for lexers.
	 *
	 *  To have indicator color values set from the preferences dialog a style ID needs to be
	 *  created for it, so they must also be within the first 31 styles.
	 *
	 *  Also, to clarify the difference between indicator and highlight, an indicator does not
	 *  need to be a highlight, it can be an underline, a cross-out line, or others.
	 *
	 ***/

	/*
	 *  One other note:  You don't even need to add much to get a lexer functioning, just the
	 *  style definitions, your Colourise/Fold routine logic, and the one liner lexer
	 *  registration line in the NppExtLexer_Plugin.cpp DLLMAIN function.
	 *
	 */

///////////////////////////////////////////////////////////////////////////////////////////////
//  Include Directives

#include "NppPlugin.h"				//  Provides all of the interface, messaging definitions,
									//  and namespace aliases.


///////////////////////////////////////////////////////////////////////////////////////////////
// Language Specific definitions

namespace NppExtLexer_MYUSERLANG {

//*********************************************************************************************
//  Global ( within the namespace ) constants.

	
//*********************************************************************************************
// Nested namespace to place only the style state IDs
namespace Styles {

const int  DEFAULT = 0;
const int  COMMENT = 1;
const int  COMMENTLINE = 2;
const int  NUMBER = 4;
const int  WORD1 = 5;
const int  WORD2 = 6;
const int  WORD3 = 7;
const int  WORD4 = 8;
const int  OPERATOR = 10;
const int  IDENTIFIER = 11;
const int  BLOCK_OPERATOR_OPEN = 12;
const int  BLOCK_OPERATOR_CLOSE = 13;
const int  DELIMITER1 = 14;
const int  DELIMITER2 = 15;
const int  DELIMITER3 = 16;

};  // End namespace Style

//*********************************************************************************************
//  Lexer Function Delarations.

	/*
	 *  Place any special class, structs, lexer specific function declarations here.
	 *
	 */


//---------------------------------------------------------------------------------------------
//  Generic Externally called functions.

//  These are the functions registered in the ExtLexer_Plugin's DLLMAIN routine.

//  This is the entry point called by the external lexer interface.
void LexOrFold(bool LexorFold, unsigned int startPos, int length, int initStyle,
                  char *words[], WindowID window, char *props);

//  This is the menu dialog function item that shows up in the Notepad++ 'Plugins' menu under
//  your lexer's name.
void menuDlg();


}  //  End:  namespace NppExtLexer_MYUSERLANG.

#endif  //  End: Include guard.