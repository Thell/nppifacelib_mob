// NppExtLexer_MYUSERLANG.cpp

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

#include "NppExtLexer_MYUSERLANG.h"	//  Provides the lexer specific definitions.

///////////////////////////////////////////////////////////////////////////////////////////////
//  <--- Namespace Aliases --->
namespace sID = NppExtLexer_MYUSERLANG::Styles;		//  All of the style state codes.
namespace lex = NppExtLexer_MYUSERLANG;				//  This lexer's namespace.


namespace NppExtLexer_MYUSERLANG {

///////////////////////////////////////////////////////////////////////////////////////////////
//  Main Lexer Functions
/*------------------------------------------------------------------------------------
this file is part of notepad++
Copyright (C)2003 Don HO < donho@altern.org >

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
----------------------------------------------------------------------------------------*/
//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
//#include <stdio.h>
//#include <stdarg.h>
//#include <windows.h>
//
//#include "Platform.h"
//
//#include "PropSet.h"
//#include "Accessor.h"
//#include "StyleContext.h"
//#include "KeyWords.h"
//#include "Scintilla.h"
//#include "SciLexer.h"

const int KEYWORD_BOXHEADER = 1;
const int KEYWORD_FOLDCONTRACTED = 2;
/*
const char EOString = '\0';
const char EOLine = '\n';
const char EOWord = ' ';
*/
static bool isInOpList(WordList & opList, char op)
{
	for (int i = 0 ; i < opList.len ; i++)
		if (op == *(opList.words[i]))
			return true;
	return false;
}

static int cmpString(const void *a1, const void *a2) {
	// Can't work out the correct incantation to use modern casts here
	return strcmp(*(char**)(a1), *(char**)(a2));
}

static int cmpStringNoCase(const void *a1, const void *a2) {
	// Can't work out the correct incantation to use modern casts here
	return CompareCaseInsensitive(*(char**)(a1), *(char**)(a2));
}


static bool isInList(WordList & list, const char *s, bool specialMode, bool ignoreCase) 
{
	if (!list.words)
		return false;

	if (!list.sorted) 
	{
		list.sorted = true;
		qsort(reinterpret_cast<void*>(list.words), list.len, sizeof(*list.words), cmpString);

		for (unsigned int k = 0; k < (sizeof(list.starts) / sizeof(list.starts[0])); k++)
			list.starts[k] = -1;
		for (int l = list.len - 1; l >= 0; l--) {
			unsigned char indexChar = list.words[l][0];
			list.starts[indexChar] = l;
		}
	}
	unsigned char firstChar = s[0];
	int j = list.starts[firstChar];

	if (j >= 0)
	{
		while ((ignoreCase?toupper(list.words[j][0]):list.words[j][0]) == (ignoreCase?toupper(s[0]):s[0]))
		{
			if (!list.words[j][1])
			{
				if (specialMode)
				{
					return true;
				}
				else
				{
					if (!s[1])
						return true;
				}
			}
			int a1 = ignoreCase?toupper(list.words[j][1]):list.words[j][1];
			int b1 = ignoreCase?toupper(s[1]):s[1];
			if (a1 == b1) 
			{
				
				const char *a = list.words[j] + 1;
				int An = ignoreCase?toupper((int)*a):(int)*a;
				
				const char *b = s + 1;
				int Bn = ignoreCase?toupper((int)*b):(int)*b;

				
				while (An && (An == Bn))
				{
					a++;
					An = ignoreCase?toupper((int)*a):(int)*a;
					b++;
					Bn = ignoreCase?toupper((int)*b):(int)*b;
				}
				if (specialMode)
				{
					if (!An)
						return true;
				}
				else
				{
					if (!An && !Bn)
						return true;
				}
			}
			j++;
		}
	}

	if (ignoreCase)
	{
		// if the 1st char is not a letter, no need to test one more time
		if (!isalpha(s[0]))
			return false;

		firstChar = isupper(s[0])?tolower(s[0]):toupper(s[0]);
		j = list.starts[firstChar];
		if (j >= 0) 
		{
			while (toupper(list.words[j][0]) == toupper(s[0])) 
			{
				if (!list.words[j][1])
				{
					if (specialMode)
					{
						return true;
					}
					else
					{
						if (!s[1])
							return true;
					}
				}
				int a1 = toupper(list.words[j][1]);
				int b1 = toupper(s[1]);
				if (a1 == b1) 
				{
					const char *a = list.words[j] + 1;
					int An = toupper((int)*a);

					const char *b = s + 1;
					int Bn = toupper((int)*b);

					while (An && (An == Bn))
					{
						a++;
						An = toupper((int)*a);
						b++;
						Bn = toupper((int)*b);
					}
					if (specialMode)
					{
						if (!*a)
							return true;
					}
					else
					{
						if (!*a && !*b)
							return true;
					}
				}
				j++;
			}
		}
	}

	return false;
}
/*
static void getRange(unsigned int start, unsigned int end, Accessor &styler, char *s, unsigned int len) 
{
	unsigned int i = 0;
	while ((i < end - start + 1) && (i < len-1)) 
	{
		s[i] = static_cast<char>(styler[start + i]);
		i++;
	}
	s[i] = '\0';
}
*/
static inline bool isAWordChar(const int ch) {
	//return (ch < 0x80) && (isalnum(ch) || ch == '.' || ch == '_');
	return ((ch > 0x20) && (ch <= 0xFF) && (ch != ' ') && (ch != '\n'));
}

static inline bool isAWordStart(const int ch) {
	return isAWordChar(ch);
}

static void Colourise_Doc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[],
                            Accessor &styler) {

	// It seems that there're only 9 keywordlists
	WordList &keywords = *keywordlists[0];
	WordList &blockOpenWords = *keywordlists[1];
	WordList &blockCloseWords = *keywordlists[2];
	WordList &symbols = *keywordlists[3];
	WordList &comments = *keywordlists[4];
	WordList &keywords5 = *keywordlists[5];
	WordList &keywords6 = *keywordlists[6];
    WordList &keywords7 = *keywordlists[7];
	WordList &keywords8 = *keywordlists[8];
	//WordList &keywords9 = *keywordlists[9];
	//WordList &keywords10 = *keywordlists[10];

	int chPrevNonWhite = ' ';
	int visibleChars = 0;

	bool isCaseIgnored = styler.GetPropertyInt("userDefine.ignoreCase", 0) != 0;
	bool isCommentLineSymbol = styler.GetPropertyInt("userDefine.commentLineSymbol", 0) != 0;
	bool isCommentSymbol = styler.GetPropertyInt("userDefine.commentSymbol", 0) != 0;

	bool doPrefix4G1 = styler.GetPropertyInt("userDefine.g1Prefix", 0) != 0;
	bool doPrefix4G2 = styler.GetPropertyInt("userDefine.g2Prefix", 0) != 0;
	bool doPrefix4G3 = styler.GetPropertyInt("userDefine.g3Prefix", 0) != 0;
	bool doPrefix4G4 = styler.GetPropertyInt("userDefine.g4Prefix", 0) != 0;
	
	char delimOpen[3];
	char delimClose[3];

	for (int i = 0 ; i < 3 ; i++)
	{
		delimOpen[i] = keywords.words[0][i] == '0'?'\0':keywords.words[0][i];
		delimClose[i] = keywords.words[0][i+3] == '0'?'\0':keywords.words[0][i+3];
	}

	StyleContext sc(startPos, length, initStyle, styler);

	for (; sc.More(); sc.Forward()) 
	{
		// Determine if the current state should terminate.
		switch (sc.state)
		{			
			case sID::NUMBER :
			{
				//if (!isAWordChar(sc.ch))
				if (!IsADigit(sc.ch))
					sc.SetState(sID::DEFAULT);
				break;
			}

			case sID::DELIMITER1 :
			{
				if (delimClose[0] && (sc.ch == delimClose[0]))
					sc.ForwardSetState(sID::DEFAULT);
				break;
			}

			case sID::DELIMITER2 :
			{
				if (delimClose[0] && (sc.ch == delimClose[1]))
					sc.ForwardSetState(sID::DEFAULT);
				break;
			}

			case sID::DELIMITER3 :
			{
				if (delimClose[0] && (sc.ch == delimClose[2]))
					sc.ForwardSetState(sID::DEFAULT);
				break;
			}
			
			case sID::IDENTIFIER : 
			{
				bool isSymbol = isInOpList(symbols, sc.ch);

				if (!isAWordChar(sc.ch)  || isSymbol)
				{
					bool doDefault = true;
					const int tokenLen = 100;
					char s[tokenLen];
					sc.GetCurrent(s, sizeof(s));
					char commentLineStr[tokenLen+10] = "0";
					char *p = commentLineStr+1;
					strcpy(p, s);
					char commentOpen[tokenLen+10] = "1";
					p = commentOpen+1;
					strcpy(p, s);
					
					if (isInList(keywords5, s, doPrefix4G1, isCaseIgnored))
						sc.ChangeState(sID::WORD1);
					else if (isInList(keywords6, s, doPrefix4G2, isCaseIgnored))
						sc.ChangeState(sID::WORD2);
					else if (isInList(keywords7, s, doPrefix4G3, isCaseIgnored))
						sc.ChangeState(sID::WORD3);
					else if (isInList(keywords8, s, doPrefix4G4, isCaseIgnored)) 
						sc.ChangeState(sID::WORD4);

					//else if (blockOpenWords.InList(s)) 
					else if (isInList(blockOpenWords, s, false, isCaseIgnored)) 
						sc.ChangeState(sID::BLOCK_OPERATOR_OPEN);
					//else if (blockCloseWords.InList(s)) 
					else if (isInList(blockCloseWords, s, false, isCaseIgnored))
						sc.ChangeState(sID::BLOCK_OPERATOR_CLOSE);
					else if (isInList(comments,commentLineStr, isCommentLineSymbol, isCaseIgnored))
					{
						sc.ChangeState(sID::COMMENTLINE);
						doDefault = false;
					}
					else if (isInList(comments, commentOpen, isCommentSymbol, isCaseIgnored)) 
					{
					      sc.ChangeState(sID::COMMENT);
					      doDefault = false;
					}
					if (doDefault)
						sc.SetState(sID::DEFAULT);
				}
				break;
			}
			
			case sID::COMMENT :
			{
				char *pCommentClose = NULL;
				for (int i = 0 ; i < comments.len ; i++)
				{
					if (comments.words[i][0] == '2')
					{
						pCommentClose = comments.words[i] + 1;
						break;
					}
				}
				if (pCommentClose)
				{
					int len = strlen(pCommentClose);
					if (len == 1)
					{
						if (sc.Match(pCommentClose[0])) 
						{
							sc.Forward();
							sc.SetState(sID::DEFAULT);
						}
					}
					else 
					{
						if (sc.Match(pCommentClose)) 
						{
							for (int i = 0 ; i < len ; i++)
								sc.Forward();
							sc.SetState(sID::DEFAULT);
						}
					}
				}
				break;
			} 
			
			case sID::COMMENTLINE :
			{
				if (sc.atLineEnd) 
				{
					sc.SetState(sID::DEFAULT);
					visibleChars = 0;
				}
				break;
			} 
			
			case sID::OPERATOR :
			{
				sc.SetState(sID::DEFAULT);
				break;
			} 
			
			default :
				break;
		}

		// Determine if a new state should be entered.
		if (sc.state == sID::DEFAULT) 
		{
			//char aSymbol[2] = {sc.ch, '\0'};

			if (IsADigit(sc.ch))
				sc.SetState(sID::NUMBER);
			//else if (symbols.InList(aSymbol))
			else if (delimOpen[0] && (sc.ch == delimOpen[0]))
				sc.SetState(sID::DELIMITER1);
			else if (delimOpen[0] && (sc.ch == delimOpen[1]))
				sc.SetState(sID::DELIMITER2);
			else if (delimOpen[0] && (sc.ch == delimOpen[2]))
				sc.SetState(sID::DELIMITER3);
			else if (isInOpList(symbols, sc.ch))
				sc.SetState(sID::OPERATOR);
			else if (isAWordStart(sc.ch)) 
				sc.SetState(sID::IDENTIFIER);
		}

		if (sc.atLineEnd) 
		{
			// Reset states to begining of colourise so no surprises
			// if different sets of lines lexed.
		   if (sc.state == sID::COMMENTLINE)
				sc.SetState(sID::DEFAULT);
			
			chPrevNonWhite = ' ';
			visibleChars = 0;
		}
		if (!IsASpace(sc.ch)) {
			chPrevNonWhite = sc.ch;
			visibleChars++;
		}
		
	}
	sc.Complete();

}


static void Fold_Doc(unsigned int startPos, int length, int initStyle, WordList *[],  Accessor &styler) 
{
	unsigned int endPos = startPos + length;
	int visibleChars = 0;
	int lineCurrent = styler.GetLine(startPos);
	int levelPrev = styler.LevelAt(lineCurrent) & SC_FOLDLEVELNUMBERMASK;
	int levelCurrent = levelPrev;
	char chNext = styler[startPos];
	int styleNext = styler.StyleAt(startPos);
	int style = initStyle;
	
	for (unsigned int i = startPos; i < endPos; i++) 
	{
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		int stylePrev = style;
		style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');

		if (stylePrev != sID::BLOCK_OPERATOR_OPEN && style == sID::BLOCK_OPERATOR_OPEN)
		{
			levelCurrent++;
		} 

		if (stylePrev != sID::BLOCK_OPERATOR_CLOSE && style == sID::BLOCK_OPERATOR_CLOSE)
		{
			levelCurrent--;
		}

		if (atEOL) 
		{
			int lev = levelPrev;

			if ((levelCurrent > levelPrev) && (visibleChars > 0))
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent))
			{
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

//
//
//static const char * const userDefineWordLists[] = {
//            "Primary keywords and identifiers",
//            "Secondary keywords and identifiers",
//            "Documentation comment keywords",
//            "Fold header keywords",
//            0,
//        };



//LexerModule lmUserDefine(SCLEX_USER, ColouriseUserDoc, "user", FoldUserDoc, userDefineWordLists);

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

}  //  End:  namespace NppExtLexer_MYUSERLANG.
