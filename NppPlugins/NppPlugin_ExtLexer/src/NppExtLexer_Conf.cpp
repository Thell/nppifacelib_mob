// NppExtLexer_Conf.cpp

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

#include "NppExtLexer_Conf.h"		//  Provides the lexer specific definitions.

///////////////////////////////////////////////////////////////////////////////////////////////
//  <--- Namespace Aliases --->
namespace sID = NppExtLexer_Conf::Styles;		//  All of the style state codes.
namespace lex = NppExtLexer_Conf;				//  This lexer's namespace.


namespace NppExtLexer_Conf {

///////////////////////////////////////////////////////////////////////////////////////////////
//  Main Lexer Functions

	//  Copied from LexConf.cxx

// Scintilla source code edit control
/** @file LexConf.cxx
 ** Lexer for Apache Configuration Files.
 **
 ** First working version contributed by Ahmad Zawawi <zeus_go64@hotmail.com> on October 28, 2000.
 ** i created this lexer because i needed something pretty when dealing
 ** when Apache Configuration files...
 **/
// Copyright 1998-2001 by Neil Hodgson <neilh@scintilla.org>
// The License.txt file describes the conditions under which this software may be distributed.


static void Colourise_Doc(unsigned int startPos, int length, int, WordList *keywordLists[], Accessor &styler)
{
	int state = sID::DEFAULT;
	char chNext = styler[startPos];
	int lengthDoc = startPos + length;
	// create a buffer large enough to take the largest chunk...
	char *buffer = new char[length];
	int bufferCount = 0;

	// this assumes that we have 2 keyword list in conf.properties
	WordList &directives = *keywordLists[0];
	WordList &params = *keywordLists[1];
	WordList &USERDEF = *keywordLists[2];

	// go through all provided text segment
	// using the hand-written state machine shown below
	styler.StartAt(startPos);
	styler.StartSegment(startPos);
	for (int i = startPos; i < lengthDoc; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);

		if (styler.IsLeadByte(ch)) {
			chNext = styler.SafeGetCharAt(i + 2);
			i++;
			continue;
		}
		switch(state) {
			case sID::DEFAULT:
				if( ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ') {
					// whitespace is simply ignored here...
					styler.ColourTo(i,sID::DEFAULT);
					break;
				} else if( ch == '#' ) {
					// signals the start of a comment...
					state = sID::COMMENT;
					styler.ColourTo(i,sID::COMMENT);
				} else if( ch == '.' /*|| ch == '/'*/) {
					// signals the start of a file...
					state = sID::EXTENSION;
					styler.ColourTo(i,sID::EXTENSION);
				} else if( ch == '"') {
					state = sID::STRING;
					styler.ColourTo(i,sID::STRING);
				} else if( ispunct(ch) ) {
					// signals an operator...
					// no state jump necessary for this
					// simple case...
					styler.ColourTo(i,sID::OPERATOR);
				} else if( isalpha(ch) ) {
					// signals the start of an identifier
					bufferCount = 0;
					buffer[bufferCount++] = static_cast<char>(tolower(ch));
					state = sID::IDENTIFIER;
				} else if( isdigit(ch) ) {
					// signals the start of a number
					bufferCount = 0;
					buffer[bufferCount++] = ch;
					//styler.ColourTo(i,sID::NUMBER);
					state = sID::NUMBER;
				} else {
					// style it the default style..
					styler.ColourTo(i,sID::DEFAULT);
				}
				break;

			case sID::COMMENT:
				// if we find a newline here,
				// we simply go to default state
				// else continue to work on it...
				if( ch == '\n' || ch == '\r' ) {
					state = sID::DEFAULT;
				} else {
					styler.ColourTo(i,sID::COMMENT);
				}
				break;

			case sID::EXTENSION:
				// if we find a non-alphanumeric char,
				// we simply go to default state
				// else we're still dealing with an extension...
				if( isalnum(ch) || (ch == '_') ||
					(ch == '-') || (ch == '$') ||
					(ch == '/') || (ch == '.') || (ch == '*') )
				{
					styler.ColourTo(i,sID::EXTENSION);
				} else {
					state = sID::DEFAULT;
					chNext = styler[i--];
				}
				break;

			case sID::STRING:
				// if we find the end of a string char, we simply go to default state
				// else we're still dealing with an string...
				if( (ch == '"' && styler.SafeGetCharAt(i-1)!='\\') || (ch == '\n') || (ch == '\r') ) {
					state = sID::DEFAULT;
				}
				styler.ColourTo(i,sID::STRING);
				break;

			case sID::IDENTIFIER:
				// stay  in CONF_IDENTIFIER state until we find a non-alphanumeric
				if( isalnum(ch) || (ch == '_') || (ch == '-') || (ch == '/') || (ch == '$') || (ch == '.') || (ch == '*')) {
					buffer[bufferCount++] = static_cast<char>(tolower(ch));
				} else {
					state = sID::DEFAULT;
					buffer[bufferCount] = '\0';

					// check if the buffer contains a keyword, and highlight it if it is a keyword...
					if(directives.InList(buffer)) {
						styler.ColourTo(i-1,sID::DIRECTIVE );
					} else if(params.InList(buffer)) {
						styler.ColourTo(i-1,sID::PARAMETER );
					} else if(USERDEF.InList(buffer)) {
						styler.ColourTo(i-1,sID::USERDEF );
					} else if(strchr(buffer,'/') || strchr(buffer,'.')) {
						styler.ColourTo(i-1,sID::EXTENSION);
					} else {
						styler.ColourTo(i-1,sID::DEFAULT);
					}

					// push back the faulty character
					chNext = styler[i--];

				}
				break;

			case sID::NUMBER:
				// stay  in CONF_NUMBER state until we find a non-numeric
				if( isdigit(ch) || ch == '.') {
					buffer[bufferCount++] = ch;
				} else {
					state = sID::DEFAULT;
					buffer[bufferCount] = '\0';

					// Colourize here...
					if( strchr(buffer,'.') ) {
						// it is an IP address...
						styler.ColourTo(i-1,sID::IP);
					} else {
						// normal number
						styler.ColourTo(i-1,sID::NUMBER);
					}

					// push back a character
					chNext = styler[i--];
				}
				break;

		}
	}
	delete []buffer;
}

//  <--- Fold --->
static bool IsCommentLine(int line, Accessor &styler) {
	int pos = styler.LineStart(line);
	int eol_pos = styler.LineStart(line + 1) - 1;
	for (int i = pos; i < eol_pos; i++) {
		char ch = styler[i];
		if (ch == '#')
			return true;
		else if (ch != ' ' && ch != '\t')
			return false;
	}
	return false;
}


void Fold_Doc(unsigned int startPos, int length, int initStyle, WordList *[], Accessor &styler)
{

	bool foldComment = styler.GetPropertyInt("fold.comment") != 0;
	bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;
	unsigned int endPos = startPos + length;
	int visibleChars = 0;
	int lineCurrent = styler.GetLine(startPos);
	int levelPrev = styler.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent = levelPrev;
	char chNext = styler[startPos];
	int styleNext = styler.StyleAt(startPos);
	for (unsigned int i = startPos; i < endPos; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		int style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
        // Comment folding
		if (foldComment && atEOL && IsCommentLine(lineCurrent, styler))
        {
            if (!IsCommentLine(lineCurrent - 1, styler)
                && IsCommentLine(lineCurrent + 1, styler))
                levelCurrent++;
            else if (IsCommentLine(lineCurrent - 1, styler)
                     && !IsCommentLine(lineCurrent+1, styler))
                levelCurrent--;
        }
		if (style == sID::OPERATOR) {
			if ( ch == '<' && chNext != '/' ) {
				levelCurrent++;
			} else if (ch == '<' && chNext == '/') {
				levelCurrent--;
			}
		}
		if (atEOL) {
			int lev = levelPrev;
			if (visibleChars == 0 && foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;
			if ((levelCurrent > levelPrev) && (visibleChars > 0))
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelPrev = levelCurrent;
			visibleChars = 0;
		}
		if (!isspacechar(ch))
			visibleChars++;
	}
	// Fill in the real level of the next line, keeping the current flags as they will be filled in later
	int flagsNext = styler.LevelAt(lineCurrent) & ~SC_FOLDLEVELNUMBERMASK;
	styler.SetLevel(lineCurrent, levelPrev | flagsNext);
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

	::MessageBox( npp_plugin::hNpp(), 
		TEXT("This is just a simple example lexer that handles Apache.conf files. \r\n"),
		TEXT("N++ External Apache Config File Lexer"),
		MB_OK);

}

}  //  End:  namespace NppExtLexer_Conf.
