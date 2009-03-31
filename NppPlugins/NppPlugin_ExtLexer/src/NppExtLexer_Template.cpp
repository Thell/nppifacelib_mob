// NppExtLexer_Template.cpp

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
//                           Bare Bones Template
//              for use with Notepad++ External Lexers Plugin
///////////////////////////////////////////////////////////////////////////////////////////////

	/*
	 *  Feel free to use this file as a template for creating and adding your own
	 *  external lexer to Notepad++.
	 *
	 *                    ENJOY !
	 *
	 */

///////////////////////////////////////////////////////////////////////////////////////////////
//  Include Directives

#include "NppExtLexer_Template.h"	//  Provides the lexer specific definitions.


///////////////////////////////////////////////////////////////////////////////////////////////
//  <--- Namespace Aliases --->
namespace sID = NppExtLexer_Template::Styles;		//  All of the style state codes.
namespace lex = NppExtLexer_Template;				//  This lexer's namespace.


namespace NppExtLexer_Template {

///////////////////////////////////////////////////////////////////////////////////////////////
//  Main Lexer Functions

//  <---  Colourise --->
void Colourise_Doc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[], Accessor &styler)
{

	//
	//  This is where you'd put some nifty styling logic.  Take a look at the many colouring
	//  routines for ideas on how to get your styles to look the way you want.
	//

}

//  <--- Fold --->
void Fold_Doc(unsigned int startPos, int length, int initStyle, WordList *[], Accessor &styler)
{

	//
	//  This is where you'd put some nifty folding logic.  Take a look at the many folding
	//  routines for ideas on how to get your folds to work the way you want.
	//

}


///////////////////////////////////////////////////////////////////////////////////////////////
//  Entry Point Functions.

//  Scintilla lexer entry point.
void LexOrFold(bool foldOrLex, unsigned int startPos, int length, int initStyle,
                  char *words[], WindowID window, char *props)
{

	// Create and initialize the WindowAccessor (including contained PropSet)
	PropSet ps;
	ps.SetMultiple(props);
	WindowAccessor wa(window, ps);

	//  Create and initialize WordList(s).
	//  If you have an extremely large word file, or lots of styling rules you may want to speed
	//  up processing by storing the wordlists instead of reprocessing them on each call.
	int nWL = 0;
	for (; words[nWL]; nWL++) ;	// count # of WordList PTRs needed
	WordList** wl = new WordList* [nWL + 1];// alloc WordList PTRs
	int i = 0;
	for (; i < nWL; i++) {
		wl[i] = new WordList();	// (works or THROWS bad_alloc EXCEPTION)
		wl[i]->Set(words[i]);
	}
	wl[i] = 0;


	// Set the currView handle to update at least once per lexer call.
	npp_plugin::hCurrViewNeedsUpdate();


	//  Call the internal folding and styling functions.
	// foldOrLex is false for lex and true for fold
	if (foldOrLex) {

		// This is a nice helpful routine to back up a line to fix broken folds.
		int lineCurrent = wa.GetLine(startPos);
		if (lineCurrent > 0) {
			lineCurrent--;
			int newStartPos = wa.LineStart(lineCurrent);
			length += startPos - newStartPos;
			startPos = newStartPos;
			initStyle = 0;
			if (startPos > 0) {
				initStyle = wa.StyleAt(startPos - 1);
			}
		}

		Fold_Doc(startPos, length, initStyle, wl, wa);

	}
	else {

		//  You may want to put a routine here to backtrack past leaking styles, typically
		//  multiline styles, or just put such logic in the Colour_Doc function itself.  Just
		//  be sure to do it prior to creating your Style Context.

		Colourise_Doc(startPos, length, initStyle, wl, wa);

	}

	//  The flush function is what actually finalizes settings the styles you just coloured.
	wa.Flush();

	// Clean up the wordlists before leaving.
	for (i = nWL - 1; i >= 0; i--)
		delete wl[i];
	delete [] wl;

}



//  Notepad++ dialog entry point.
void menuDlg()
{

	::MessageBox(npp_plugin::hNpp(), 
		TEXT("This is just a template dialog. \r\n\r\n")
		TEXT("Had it been the real thing, there may have been some nifty information\n")
		TEXT("here, like credits, or specific notes about an external lexer.\n "),
		TEXT("N++ External Lexer Template"),
		MB_OK);

}

}  //  End:  namespace NppExtLexer_Template.
