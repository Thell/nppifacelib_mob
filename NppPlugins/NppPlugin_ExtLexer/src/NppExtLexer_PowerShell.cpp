// NppExtLexer_PowerShell.cpp

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
//           External Lexer Implementation for PowerShell Script Files
//				   for use the Notepad++ External Lexers Plugin
///////////////////////////////////////////////////////////////////////////////////////////////

	/***
	 *
	 *  The commenting in this code may seem excessive, yet it is there for people who would
	 *  like to better understand how the Lexer plugin system works and how it can be used.
	 *
	 ***/

///////////////////////////////////////////////////////////////////////////////////////////////
//  Include Directives

#include "NppExtLexer_PowerShell.h"	//  Provides all of the PowerShell specific definitions.


///////////////////////////////////////////////////////////////////////////////////////////////
//  <--- Namespace Aliases --->
namespace sID = NppExtLexer_PowerShell::Styles;		//  All of the style state codes.
namespace lex = NppExtLexer_PowerShell;				//  This lexer's namespace.


namespace NppExtLexer_PowerShell {

//---------------------------------------------------------------------------------------------
//  <--- Namespace Global Pointers --->
PowerShell_Lexer lexer;

//  <--- Base wordlist and character set classes --->
 /*
  *  I've place these in the global (to this namespace) location because the trade-off between
  *  the overhead of creating them each time Colourise_Doc is called vs the memory usage of
  *  keeping them around seems to be in favor of using of speed.
  */

//								keywordClass/[array]	Style					Colourise_Doc Switch Case
WordList wl_keywords;			// instre1/[0]			KEYWORD(15)				IDENTIFIER
WordList wl_cmdlets;			// instre2/[1]			CMDLET(16)				IDENTIFIER
WordList wl_aliases;			// type 1/[2]			ALIAS(17)				IDENTIFIER
WordList wl_operators;			// type 2/[3]			OPERATOR(1)				PARAMETER
WordList wl_cmdletParam;		// type 3/[4]			CMLDPARAMATTRIB(20)		TYPE
WordList wl_parameterValues;	// type 4/[5]			CMDLETPARAMPARAM(19)	IDENTIFIER

CharacterSet setWordStart(CharacterSet::setAlphaNum, "_", 0x80, true);
CharacterSet setVarName(CharacterSet::setAlphaNum,":_", 0x80, true);
CharacterSet setCmdlet(CharacterSet::setAlphaNum, "-_", 0x80, true);
CharacterSet setTypes(CharacterSet::setAlphaNum, "._':", 0x80, true);
CharacterSet setOperator(CharacterSet::setNone,"%^&*-+=|:;<>,/?!.~@$[]{}()",0x80, true);
CharacterSet setGroupStart(CharacterSet::setNone, "{([", 0x80, true);
CharacterSet setGroupEnd(CharacterSet::setNone, "])}", 0x80, true);


///////////////////////////////////////////////////////////////////////////////////////////////
//  Main Lexer Functions

//  <---  Colourise --->
void Colourise_Doc(unsigned int startPos, int length, int initStyle, Accessor &styler)
//void Colourise_Doc(unsigned int startPos, int length, int initStyle, WordList *keywordlists[], Accessor &styler)
{

	// Nest tracking setup
	unsigned int iGroupLevel = 0;
	StateNest sn[100];				// how many levels to track (hopefully this is ridiculously high)
	unsigned int iNL = 0;			// Nested Level Index
	int prevWordState = initStyle;	// Tracks states for previous non-operator and non-whitespace
	bool bVarNameExt = false;		// flag for extended variable names

	// Initialize state nesting
	sn[iNL].preNestState = initStyle;
	sn[iNL].prevWordState = initStyle;
	sn[iNL].GroupLevel = 0;

	// Should be ready to process now; setup the class parms
	StyleContext sc(startPos, length, initStyle, styler);

	// Main lexing loop
	for (; sc.More(); sc.Forward()) {

		if (sc.ch == '`') lexer.CaptureEscapeChars(sc);

		// Keep track of grouping levels for nest tracking
		// Beware of forwarding onto or over group indicators (see STRING cases for example)
		if (setGroupStart.Contains(sc.ch)) iGroupLevel++;
		if (setGroupEnd.Contains(sc.ch)) iGroupLevel--;

		// Process current style state
		switch(sc.state) {
			// Double quoted strings and here-strings highlight embedded items
			case sID::STRING:
				if(sc.ch == '\"') {
					sc.ForwardSetState(sID::DEFAULT);
					if (sc.chPrev == '\"') {
						// Double quotes can cause missed characters, when it is a Group open or close it messes up the counts
						if (setGroupStart.Contains(sc.ch)) iGroupLevel++;
						if (setGroupEnd.Contains(sc.ch)) iGroupLevel--;
					}
				}
				// Highlight nested variables and evaluations in double quoted strings
				else if(sc.ch == '$') {
					iNL++;
					sn[iNL].preNestState = sID::STRING;
					if(sc.Match("$(")) {
						sn[iNL].NestedState = sID::EVALUATION;
						sn[iNL].GroupLevel = iGroupLevel;
						sn[iNL].GroupTerminator = ')';
						sn[iNL].GroupStartPos = sc.currentPos;
					}
					else if(sc.ch == '$') {
						sn[iNL].NestedState = sID::VARIABLE;
					}
					sc.SetState(sID::DEFAULT);
				}
				break;
			case sID::HERESTRING:
				if (sc.atLineStart && sc.Match('\"','@')) {
					sc.Forward();
					sc.ForwardSetState(sID::DEFAULT);
				}
				// Highlight nested variables and evaluations in double quoted strings
				else if(sc.ch == '$') {
					iNL++;
					sn[iNL].preNestState = sID::HERESTRING;
					if(sc.Match("$(")) {
						sn[iNL].NestedState = sID::EVALUATION;
						sn[iNL].GroupLevel = iGroupLevel;
						sn[iNL].GroupTerminator = ')';
						sn[iNL].GroupStartPos = sc.currentPos;
					}
					else if(sc.ch == '$') {
						sn[iNL].NestedState = sID::VARIABLE;
					}
					sc.SetState(sID::DEFAULT);
				}
				break;
				// Literal strings don't take any embedding and can't escape the '\'' delimiter.
			case sID::STRINGLITERAL:
				if(sc.ch == '\'') {
					sc.ForwardSetState(sID::DEFAULT);
					if (sc.chPrev == '\'') {
						// Double quotes can cause missed characters, when it is a Group open or close it messes up the counts
						if (setGroupStart.Contains(sc.ch)) iGroupLevel++;
						if (setGroupEnd.Contains(sc.ch)) iGroupLevel--;
					}
				}
				break;
			case sID::HERESTRINGLITERAL:
				if(sc.atLineStart && sc.Match('\'','@')) {
					sc.Forward();
					sc.ForwardSetState(sID::DEFAULT);
				}
				break;
			case sID::COMMENT:
				if (sc.atLineStart) {
					sc.SetState(sID::DEFAULT);
				}
				break;
			case sID::MULTILINECOMMENT:
				if(sc.Match('#','>')) {
					sc.Forward();
					sc.ForwardSetState(sID::DEFAULT);
				}
				break;
			case  sID::VARIABLE:
				if(bVarNameExt || (sc.LengthCurrent() == 1 && sc.ch == '{' )) {
					if (!bVarNameExt) {
						bVarNameExt = true;
					}
					// Pretty much everything is allowable in a variable name block (even new lines)
					if (sc.ch == '}') {
						sc.ForwardSetState(sID::DEFAULT);
						bVarNameExt = false;
					}
				}
				else if(!setVarName.Contains(sc.ch) || sc.Match(':', ':')) {
					sc.SetState(sID::DEFAULT);
				}
				break;
			case sID::NUMBER:
				if (!IsADigit(sc.ch)) {
					sc.SetState(sID::DEFAULT);
				} 
				break;
			case sID::PARAMETER:
				if (!setWordStart.Contains(sc.ch)) {
					char s[100];
					sc.GetCurrentLowered(s, sizeof(s));

					// Since named operators also start with a '-' check for matches here
					if (wl_operators.InList(s)) {
						sc.ChangeState(sID::OPERATOR);
					}
					sc.SetState(sID::DEFAULT);
				}
				break;
			case sID::TYPE:
				// Initialized with '['
				if (!setTypes.Contains(sc.ch)) {
					char s[1000];
					sc.GetCurrentLowered(s, sizeof(s));

					// Check for specified parm type
					if(wl_cmdletParam.InList(s)) {
						sc.ChangeState(sID::CMDLETPARAMATTRIB);
					}
					sc.SetState(sID::DEFAULT);
				}
				break;
			case sID::IDENTIFIER:
				if (!setCmdlet.Contains(sc.ch)) {
					char s[100];
					sc.GetCurrentLowered(s, sizeof(s));

					if (wl_keywords.InList(s)) {
						sc.ChangeState(sID::KEYWORD);
					} else if (wl_cmdlets.InList(s)) {
						sc.ChangeState(sID::CMDLET);
					} else if (wl_aliases.InList(s)) {
						sc.ChangeState(sID::ALIAS);
					} else if (wl_parameterValues.InList(s)) {
						sc.ChangeState(sID::CMDLETPARAMPARAM);  //what is a param param?
					} else {
						// Naked strings should be treated as arguments?
						prevWordState = sID::DEFAULT;
						sc.ChangeState(sID::DEFAULT);
					}
					sc.SetState(sID::DEFAULT);
				}
				break;
			case sID::MEMBER:
				if (sc.atLineEnd || !setVarName.Contains(sc.ch)) {
					if (sc.ch == '(') {
						sc.ChangeState(sID::METHOD);
					} else {
						sc.ChangeState(sID::PROPERTY);
					}
					sc.SetState(sID::DEFAULT);
				}
				break;
			case sID::OPERATOR:
				// symbols only (operator keywords are processed in the parameters section)
				sc.SetState(sID::DEFAULT);
				break;
		}

		// Determine if a new state should be entered.
		if (sc.state == sID::DEFAULT)
		{
			// Comments
			if (sc.ch == '#') {
				sc.SetState(sID::COMMENT);
			}
			// Multiline Comments
			else if (sc.Match('<','#')) {
				sc.SetState(sID::MULTILINECOMMENT);
			}
			// Strings
			else if (sc.ch == '\"') {
				sc.SetState(sID::STRING);
			}
			else if (sc.ch == '\'') {
				sc.SetState(sID::STRINGLITERAL);
			}
			// Here-Strings
			else if(sc.Match('@','\"') && lexer.IsEOL(sc.GetRelative(3),sc.GetRelative(4))) {
				sc.SetState(sID::HERESTRING);
			}
			else if(sc.Match('@','\'') && lexer.IsEOL(sc.GetRelative(3),sc.GetRelative(4))) {
				sc.SetState(sID::HERESTRINGLITERAL);
			}
			// Numerics
			else if (IsADigit(sc.ch) && !isalpha(sc.chPrev)) {
				sc.SetState(sID::NUMBER);
			}
			// Parameters
			else if (sc.chPrev == '-' && !(IsADigit(sc.ch) || IsASpaceOrTab(sc.ch)) && 
				(!setOperator.Contains(sc.ch) || sc.chNext == '$')) {
					sc.SetState(sID::PARAMETER);
			}
			// Variables
			else if (sc.ch == '$' && sc.chNext != '(') {
				sc.SetState(sID::VARIABLE);
			}
			// Types
			else if((sc.chPrev == '[' && setWordStart.Contains(sc.ch)) &&
				!(IsADigit(sc.ch) || (sc.ch == '-' && IsADigit(sc.chNext)))){
					sc.SetState(sID::TYPE);
			}
			// STATIC MEMBERs:	start with letters and follow '::'
			else if (styler.Match(sc.currentPos - 2, "::") && isalpha(sc.ch)) {
				sc.SetState(sID::MEMBER);	
			}
			// MEMBERs:	Start with letters, follow '.' and don't come after "default" stuff
			else if (prevWordState != sID::DEFAULT && sc.chPrev != '*' &&
				sc.ch == '.' && isalpha(sc.chNext)) {
					sc.SetState(sID::OPERATOR);
					sc.ForwardSetState(sID::MEMBER);
			}
			// Operators
			else if (setOperator.Contains(sc.ch)) {
				sc.SetState(sID::OPERATOR);
			}
			// Keyword Identifiers
			else if (setCmdlet.Contains(sc.ch)) {
				sc.SetState(sID::IDENTIFIER);
			}
		}


		// Process nesting state change
		// Sometimes the nest is terminating on the same character that is supposed to terminate the preNestState, since
		// we are only using nesting within double quotes it is a fairly easy check
		if ( sc.atLineEnd ) {

			if ( iNL >> 0 ) {
				lexer.lineState.flagMultilineStyle( styler.GetLine( sc.currentPos ) );
			}

			if ( iNL == 0 && (! lexer.fullDocProcessing ) &&
					( lexer.lineState.IsMultilineStyle( styler.GetLine( sc.currentPos ) ) ) ) {
				lexer.lineState.clearMultilineStyle( sc.currentPos );
			}

		}

		if (iNL >> 0) {
			switch (sn[iNL].NestedState) {
				case sID::VARIABLE:
					if (sc.state != sID::VARIABLE) {
						sc.SetState(sn[iNL].preNestState);
						iNL--;
						if (sc.state == sID::STRING && sc.ch == '\"') {
							sc.ForwardSetState(sn[iNL].preNestState);
						}
					}
					break;
				case sID::EVALUATION:
					if (sc.ch == sn[iNL].GroupTerminator && iGroupLevel == sn[iNL].GroupLevel) {
						sc.ForwardSetState(sn[iNL].preNestState);
						if (sc.ch == '`') lexer.CaptureEscapeChars(sc);
						iNL--;
						if (sc.state == sID::STRING && sc.ch == '\"') {
							sc.ForwardSetState(sID::DEFAULT);
							// Fix for cases when using the previous ForwardSetState bypasses
							// GroupTerminators directly after the '\"'
							while(sc.ch == sn[iNL].GroupTerminator ||
								(iNL == 0 && setOperator.Contains(sc.ch))) {
									if (setGroupStart.Contains(sc.ch)) iGroupLevel++;
									if (setGroupEnd.Contains(sc.ch)) iGroupLevel--;
									sc.SetState(sID::OPERATOR);
									sc.ForwardSetState(sn[iNL].preNestState);
							}
						}
					}
					break;
			}
		}


		if(!setOperator.Contains(sc.ch) && !IsASpaceOrTab(sc.ch)) {
			prevWordState = sc.state;
		}
	}

	// All done
	sc.Complete();
	// TODO if (bFirstPass) styler.Flush();
}


//  <--- Highlight --->
void Highlight_Doc(unsigned int startPos, int length, int initStyle, Accessor &styler)
{
	//  Clear any existing highlights within range & check if we need to draw any highlights.
	bool drawHlites = false;

	for ( int i = 0; i < INDICMAX; i++ ) {
		if ( lexer.Hlite[i].Active ) {
			drawHlites = true;
		}
		//  Using a zero value param through the accessor will clear an indicator
		styler.IndicatorFill( startPos, length -1 , i, NULL );
	}

	lexer.lineState.ClearMultilineState(styler.GetLine(startPos), styler.GetLine(startPos + length));


	if ( drawHlites ) {
		int currPos;
		int currStyle;
		int currHlite;
		int braceMatchPos;				//  Used for multiline highlight control.
		int thisLineDiff;		//  Used for multiline highlight control.

		//  Draw indicators in range.  When modifying pay attention to the placement of
		//  'pos=currPos' to save processing time; don't do this when highlights can overlap!
		for (unsigned int pos = startPos; pos < (startPos + length); pos++) {

			currStyle = styler.StyleAt(pos);

			switch (currStyle)
			{
				case sID::OPERATOR:			// Capture expressions and apply highlight
					currHlite = sID::INDICEXPR - INDICBASE;
					if ( ( lexer.Hlite[currHlite].Active ) &&
							styler.Match( pos, "$(" ) ) {

						// Make sure we at least have a matching brace to highlight to.
						braceMatchPos = messageProc( SCI_BRACEMATCH, pos + 1, 0 );
						if ( braceMatchPos == -1 ) {
							braceMatchPos = pos;
						}
						braceMatchPos++;	//  Brace match position is always -1 from our target.

						styler.IndicatorFill(pos, braceMatchPos, currHlite,	sID::INDICEXPR);

						//  Handle multiline instances of this highlighter.
						thisLineDiff = ( styler.GetLine(braceMatchPos) - styler.GetLine(pos) );
						
						if (! thisLineDiff == 0 ) {
							lexer.lineState.setMultlineState( styler.GetLine(pos), styler.GetLine(braceMatchPos) );

							if ( lexer.Hlite[currHlite].clearWhiteSpace ) {
								currPos = styler.LineStart( styler.GetLine(pos) + 1 );
								do {

									int j = currPos;
									while ( IsASpaceOrTab( styler.SafeGetCharAt(j) ) ) j++;
									if ( j > currPos ) {
										styler.IndicatorFill( currPos, j, currHlite, NULL );
										currPos = styler.LineStart( styler.GetLine(currPos) + 1 );
									}
									else {
										currPos = styler.LineStart(styler.GetLine(currPos) + 1);
									}

								} while (currPos < braceMatchPos);
							}
						}
					}
					break;
				case sID::PARAMETER:
					currHlite = sID::INDICPARM - INDICBASE;
					if ( lexer.Hlite[currHlite].Active ) {
						// Find the end of the current style and highlight to it
						for (currPos = pos + 1; styler.StyleAt(currPos) == currStyle; currPos++);
						styler.IndicatorFill(pos, currPos, currHlite, sID::INDICPARM);
						pos = currPos;
					}
					break;
				case sID::PROPERTY:
					currHlite = sID::INDICPROP - INDICBASE;
					if ( lexer.Hlite[currHlite].Active ) {
						// Find the end of the current style and highlight to it
						for (currPos = pos + 1; styler.StyleAt(currPos) == currStyle; currPos++);
						styler.IndicatorFill(pos, currPos, currHlite, sID::INDICPROP);
						pos = currPos;
					}
					break;
				case sID::METHOD:
					currHlite = sID::INDICMETH - INDICBASE;
					if ( lexer.Hlite[currHlite].Active ) {
						for (currPos = pos + 1; styler.StyleAt(currPos) == currStyle; currPos++);
						// Make sure we at least have a matching brace to highlight to
						braceMatchPos = messageProc(SCI_BRACEMATCH, currPos, 0);
						if (braceMatchPos == -1 ) {
							braceMatchPos = pos;
						}
						braceMatchPos++;

						styler.IndicatorFill(pos, braceMatchPos, currHlite,	sID::INDICMETH);

						//  Handle multiline instances of this highlighter.
						thisLineDiff = (styler.GetLine(braceMatchPos) - styler.GetLine(pos));
						
						if (! thisLineDiff == 0) {
							lexer.lineState.setMultlineState( styler.GetLine(pos), styler.GetLine(braceMatchPos) );

							if ( lexer.Hlite[currHlite].clearWhiteSpace ) {
								currPos = styler.LineStart( styler.GetLine(pos) + 1 );
								do {

									int j = currPos;
									while (IsASpaceOrTab(styler.SafeGetCharAt(j) ) ) j++;
									if (j > currPos) {
										styler.IndicatorFill( currPos, j, currHlite, NULL );
										currPos = styler.LineStart(styler.GetLine(currPos) + 1);
									}
									else {
										currPos = styler.LineStart(styler.GetLine(currPos) + 1);
									}

								} while (currPos < braceMatchPos);
							}
						}
					}
					break;
			}	// End:  switch(currStyle)
		}	// End: main pos loop
	}  // End:  drawHlites
}

//  <--- Fold --->
void Fold_Doc(unsigned int startPos, int length, int initStyle, Accessor &styler)
//void Fold_Doc(unsigned int startPos, int length, int initStyle, WordList *[], Accessor &styler)
{
	// Store both the current line's fold level and the next lines in the
	// level store to make it easy to pick up with each increment
	// and to make it possible to fiddle the current level for "} else {".

	/*
	Couldn't find any documentation about Notepad++ and .properties, but to be used with editors that
	make use of properties files GetPropertyInt is used, but for Notepad++ provide a default value.
	*/

	// We might not even want folding...
	if (styler.GetPropertyInt("fold") == 0)	return;

	// Initialize fold settings
	bool foldComment = styler.GetPropertyInt("fold.comment", 1) != 0;
	bool foldSuccessiveHashComment = styler.GetPropertyInt("fold.successivehashcomment", 1) != 0;
	bool foldMultilineComment = styler.GetPropertyInt("fold.multilinecomment", 1) != 0;
	// foldWithinMultilineComment allows for brace folding within <* *> markers
	bool foldWithinMultilineComment = styler.GetPropertyInt("fold.withinmultilinecomment", 1) != 0;
	bool foldAtElse = styler.GetPropertyInt("fold.at.else", 1) != 0;
	bool foldCompact = styler.GetPropertyInt("fold.compact", 1) != 0;

	// Initialize values
	// Store both the current line's fold level and the next lines in the
	// level store to make it easy to pick up with each increment
	// and to make it possible to fiddle the current level for "} else {".
	unsigned int endPos = startPos + length;
	int visibleChars = 0;
	int lineCurrent = styler.GetLine(startPos);
	int levelCurrent = SC_FOLDLEVELBASE;
	if (lineCurrent > 0)
		levelCurrent = styler.LevelAt(lineCurrent-1) >> 16;
	int levelMinCurrent = levelCurrent;
	int levelNext = levelCurrent;
	int styleNext = styler.StyleAt(startPos);
	int style = initStyle;
	char chNext = styler[startPos];

	// Line Processing
	for (unsigned int i = startPos; i < endPos; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		int stylePrev = style;
		style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		bool atEOL = lexer.IsEOL(ch, chNext);

		// Comment Folding
		if ( foldComment && lexer.IsCommentStyle(style) ) {
			// Successive single line comment folding
			if (atEOL) {
				if (foldSuccessiveHashComment && (style == sID::COMMENT)) {
					if (! lexer.IsHashCommentLine(lineCurrent - 1) &&
							lexer.IsHashCommentLine(lineCurrent + 1)) {
						levelNext++;
					}
					else if (lexer.IsHashCommentLine(lineCurrent - 1) &&
							!lexer.IsHashCommentLine(lineCurrent + 1)) {
						levelNext--;
					}
				}
			}
			// Multiline comment folding
			if (foldMultilineComment && (style == sID::MULTILINECOMMENT)) {
				if (stylePrev != sID::MULTILINECOMMENT) {
					levelNext++;
				}
				else if ((styleNext != sID::MULTILINECOMMENT) && !atEOL) {
					// Comments don't end at end of line and the next character may be unstyled.
					levelNext--;
				}
			}

			// Manual fold point marker
			if ((ch == '#') && (chNext == '#')) {
				char chNext2 = styler.SafeGetCharAt(i + 2);
				if (chNext2 == '{') {
					levelNext++;
				} else if (chNext2 == '}') {
					levelNext--;
				}
			}
		}

		// Operator Folding
		if (style == sID::OPERATOR ||
			(foldWithinMultilineComment && (style == sID::MULTILINECOMMENT))) {
				if (ch == '{' || ch == '(') {
					// Measure the minimum before a '{' to allow
					// folding on "} else {"
					if (levelMinCurrent > levelNext) {
						levelMinCurrent = levelNext;
					}
					levelNext++;
				} else if (ch == '}' || ch == ')') {
					levelNext--;
				}
		}

		// Flag and level controls
		if (!IsASpace(ch))
			visibleChars++;

		if (atEOL || (i == endPos-1)) {
			int levelUse = levelCurrent;
			if (foldAtElse) {
				levelUse = levelMinCurrent;
			}
			int lev = levelUse | levelNext << 16;
			if (visibleChars == 0 && foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;
			if (levelUse < levelNext)
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelCurrent = levelNext;
			levelMinCurrent = levelCurrent;
			visibleChars = 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
//  Lexer Helper Functions

//  Updates wordlists.
void PowerShell_Lexer::updateWordlists(char* words[])
{

	wl_keywords.Set(words[0]);
	wl_cmdlets.Set(words[1]);
	wl_aliases.Set(words[2]);
	wl_operators.Set(words[3]);
	wl_cmdletParam.Set(words[4]);
	wl_parameterValues.Set(words[5]);

}


//  Updates highlighters.
void PowerShell_Lexer::updateHighlighterStyles()
{

	std::vector<Highlighter>::iterator currHlite = Hlite.begin();

	for ( currHlite; currHlite < Hlite.end(); currHlite++ ) {
		//  Check for updates to highlighter styles.
		Highlighter tmpHlite;
		tmpHlite = currHlite->init();
		if ( tmpHlite.StyleChanged ) {

			lexer.HliteStyleChanged = true;

			//  Notepad++ and Scintilla will change the colors and styles but not 
			//  over/under and drawing/erasing.
			if ( currHlite->Active != tmpHlite.Active ||
					( currHlite->Active && ( currHlite->SCI_INDICUNDER != tmpHlite.SCI_INDICUNDER ))) {

				*currHlite = tmpHlite;
				currHlite->forceReDraw = true;

			}
			else {

				//  Otherwise Notepad++ and Scintilla will take care of colouring.
				*currHlite = tmpHlite;

			}

			currHlite->StyleChanged = false;
		}
	}

}

//  Determine if the full document should be processed.
bool PowerShell_Lexer::doFullDoc( int startPos )
{
	//  When initially lexing a doc the whole doc needs to be processed to ensure that
	//  highlighters are applied correctly and to avoid future calls to apply highlighters
	//  when they aren't needed.

	bool retVal = false;

	retVal = !npp_plugin::isNppReady();

	if ( lexer.languageChanged ) {
		lexer.languageChanged = false;
		retVal = true;
	}

	if ( lexer.docModified ) {
		lexer.docModified = false;
			if ( startPos == 0 ) retVal = true;
	}

	if ( lexer.HliteStyleChanged ) {
		lexer.HliteStyleChanged = false;
		retVal = true;
	}

	if ( lexer.StylesUpdatedCall ) {
		lexer.StylesUpdatedCall = false;
		retVal = true;
	}

	if ( retVal ) lexer.fullDocProcessing = true;

	return ( retVal );

}

//  Re-Activates the Notepad++ view that was active prior to processing WORDSTYLESUPDATED when
//  a second view needed updating as well.
void PowerShell_Lexer::activateAltView()
{

	lexer.StylesUpdatedCall = true;
	lexer.processAltView = false;
	int targetView = ( npp_plugin::intCurrView() == 0 ) ? ( 1 ) : ( 0 );
	int targetIndex = messageProc( NPPM_GETCURRENTDOCINDEX, 0, targetView );

	messageProc( NPPM_ACTIVATEDOC, targetView, targetIndex );

	//  This routine would typically be called only when the Style Configurator is active, so
	//  focus needs to get back to it.
	//HWND dlgTarget = ::FindWindowW( NULL, TEXT("Style Configurator") );
	if (lexer.prevHwndFocused) ::SetFocus( lexer.prevHwndFocused );
	lexer.prevHwndFocused = NULL;

}


//  Returns a start position that shouldn't break multiline embedded styles.
int PowerShell_Lexer::getSafe_Style_StartPos( int currPos )
{

	int	currLine = _pAccessor->GetLine(currPos);

	bool bSafePos = false;
	
	while ( ( currLine > 0 ) && !bSafePos ) {
		
		if (! lexer.IsSafe_Style_currPos(  _pAccessor->StyleAt( _pAccessor->LineStart( currLine ) ) ) ||
			( lexer.lineState.IsMultilineStyle( currLine ) ) ||
			( ( currLine > 1 ) &&
			(! lexer.IsSafe_Style_prevEOL( _pAccessor->StyleAt( _pAccessor->LineStart( currLine - 1 ) ) ) ) ||
			( lexer.lineState.IsMultilineStyle( currLine - 1 ) ) ) ) {
			currLine--;
		}
		else bSafePos = true;

	}

	return ( _pAccessor->LineStart( currLine ) );

}

//  If a style is a multiline style that can have embedded styles it isn't safe.
bool PowerShell_Lexer::IsSafe_Style_currPos( int style )
{

	// List initial styles that can cause styling errors to occur
	return	style != sID::HERESTRING &&
			style != sID::STRING &&
			style != sID::MULTILINECOMMENT &&
			style != sID::TYPE &&
			style != sID::VARIABLE;

}

//  If a style is a multiline style that can have embedded styles it isn't safe.
bool PowerShell_Lexer::IsSafe_Style_prevEOL( int style )
{

	// List initial styles can cause styling errors to occur
	return	style != sID::HERESTRING &&
			style != sID::MULTILINECOMMENT &&
			style != sID::TYPE &&
			style != sID::VARIABLE;

}

//  This function compares ints passed to it for equality, terminate with '-1'.
bool PowerShell_Lexer::IsEqual( const int first, ... ) 
{

	int i = first, diff = 0;
	va_list marker;

	va_start( marker, first );     // Initialize variable arguments.

	int firstVal = i;

	while( i != -1 )
	{
		diff = firstVal - i;
		if (diff) break;
		i = va_arg( marker, int);
	}
	va_end( marker );              // Reset variable arguments.

	return( diff == 0 );

}


// Capture escaped characters before processing next char.
void PowerShell_Lexer::CaptureEscapeChars(StyleContext &sc)
{
	if (sc.state != sID::STRINGLITERAL &&
		sc.state != sID::HERESTRINGLITERAL &&
		sc.state != sID::COMMENT &&
		sc.state != sID::MULTILINECOMMENT) {
			bool bEscape = false;
			int tmpState;
			if (sc.ch == '`') bEscape = true;
			while (bEscape) {
				tmpState = sc.state;
				sc.SetState(sID::ESCAPE);
				sc.Forward(2);
				sc.SetState(tmpState);
				if (sc.ch != '`') bEscape = false;
			}
	}
	return;
}

// Identify language comment styles (useful for doc folding)
bool PowerShell_Lexer::IsCommentStyle(int style)
{

	return	style == sID::COMMENT ||
			style == sID::MULTILINECOMMENT;

}

// This routine is used in identifying successive HashComment lines
bool PowerShell_Lexer::IsHashCommentLine(int line)
{

	int pos = _pAccessor->LineStart(line);
	int eol_pos = _pAccessor->LineStart(line + 1) - 1;

	for (int i = pos; i < eol_pos; i++) {
		char ch = _pAccessor->SafeGetCharAt( i );
		if (ch == '#')
			return true;
		else if (ch != ' ' && ch != '\t')
			return false;
	}

	return false;

}

int PowerShell_Lexer::getSafe_Hlite_Pos(int currPos, int length, bool direction)
{

	//	This function checks for a line that does not have a linestate value indicating a
	//  multiline indicator and returns the lines start or end position depending on direction.

	int lineChange;
	int extremeLine;
	int currLine;

	// Search parameters
	if ( direction ) {			// DIR_UP

		lineChange = -1;
		extremeLine = 1;
		currLine = _pAccessor->GetLine(currPos);

	}
	else {						// DIR_DOWN

		lineChange = 1;
		extremeLine = _pAccessor->GetLine( _pAccessor->Length() );
		currLine = _pAccessor->GetLine( currPos + length );

	}

	// If we are already at the top or bottom of the buffer just return
	if (currLine == extremeLine) return -1;

	int tmpLine = currLine;
	do {

		/*
		 *  This works by checking the current line for a multiline state, if one is found
		 *  the current line changes to that value.  If one is found, but the value is the same
		 *  ( or higher - which shouldn't happen ) the current line value changes by one and 
		 *  the check is done again until successfully finding a blank line or the extreme
		 *  top or bottom of the doc is hit.
		 *  
		 */

		if ( direction ) {

			tmpLine = lexer.lineState.getMultilineStart( currLine );
			if (! tmpLine ) break;			//  A blank line.  Forcefully escape the do loop.
			
			currLine = ( tmpLine < currLine ) ? ( tmpLine ) : ( currLine + lineChange );

		}
		else {

			tmpLine = lexer.lineState.getMultilineEnd( currLine );
			if (! tmpLine ) break;			//  A blank line.  Forcefully escape the do loop.
			
			currLine = tmpLine > currLine ? tmpLine : currLine + lineChange;

		}

	} while ( ( direction ) ? ( currLine > extremeLine ) : ( currLine < extremeLine ) );


	return ( _pAccessor->LineStart( currLine ) );

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

	// Set the currView handle to at update at least once per lexer call.
	npp_plugin::hCurrViewNeedsUpdate();

	// Initialize the lexer.  Ater the first run this just updates the Accessor.
	lexer.initLexer(words, wa);

	if( lexer.StylesUpdatedCall ) {
		//  When a wordlist style change happens this updates the wordlists.
		lexer.updateWordlists( words );
	}


	// foldOrLex is false for lex and true for fold
	if (foldOrLex) {
		// Back up a line to fix broken states/folds
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
		Fold_Doc(startPos, length, initStyle, wa);

	}
	else {
		// Styling entry point.
		
		int colourStartPos = startPos;
		int colourLength = length;
		int colourInitStyle = initStyle;
		int hliteStartPos = startPos;
		int hliteLength = length;
		int hliteInitStyle = initStyle;

		if ( lexer.doFullDoc( startPos ) ) {

			// This always yields a full document lex from pos 0 to length()
			colourStartPos = 0;
			colourLength = wa.Length();
			hliteStartPos = 0;
			hliteLength = wa.Length();

		}
		else {

			// Find a safe entry point to ensure styles and highlighters aren't broken.
			if (startPos > 0) {

				//  Find the colouring startPos.
				colourStartPos = lexer.getSafe_Style_StartPos( startPos );
				colourLength += startPos - colourStartPos;

				//  Find the highlighting startPos and endPos.
				hliteStartPos = lexer.getSafe_Hlite_Pos( startPos, length, DIR_UP );
				int hliteEndPos = lexer.getSafe_Hlite_Pos( startPos, length, DIR_DOWN );
				int tmphliteLength = ( hliteEndPos == -1 ) ? ( length ) : ( hliteEndPos - hliteStartPos );
				int currLineLength = sizeof( wa.GetLine( hliteStartPos ) );
				hliteLength = ( tmphliteLength > currLineLength ) ? ( tmphliteLength ) : ( currLineLength );
			}
		}

		// Do the main coloring routine
		colourInitStyle = wa.StyleAt( colourStartPos - 1 );
		Colourise_Doc( colourStartPos, colourLength, colourInitStyle, wa );

		wa.Flush();

		hliteInitStyle = wa.StyleAt( hliteStartPos - 1 );
		Highlight_Doc( hliteStartPos, hliteLength, hliteInitStyle, wa );

		lexer.fullDocProcessing = false;	//  Make sure this is turned off!

	}

	// clean up before leaving
	wa.Flush();

	if ( lexer.processAltView ) lexer.activateAltView();

}

//  Notepad++ dialog entry point.
void menuDlg()
{

	::MessageBox(npp_plugin::hNpp(),
		TEXT("Thank you for using the PowerShell Script File lexer, brought to you\n")
		TEXT("by the some of the fine folks in #PowerShell on the Freenode network! \r\n\r\n")
		TEXT("Make your way onto irc and give almostautomated and jaykul some feedback.\n")
		TEXT("FYI:  There is a language updater script file available for this lexer that\n")
		TEXT("updates the language file to include your custom aliases, cmdlets, and so on.\n")
		TEXT("Visit #PowerShell and pester meson for it!\r\n\r\n")
		TEXT("Also, don't forget to try out the cool highlighting capabilities by using the\n")
		TEXT("Notepad++ Style Configurator dialog! \r\n")
		TEXT("	'Bold'= Enable highlighter.\n")
		TEXT("	'Italic'= Erase highlight from leading white-space.\n")
		TEXT("	'Underline' = Draw highlight 'under' the styled words.\r\n"),
		TEXT("Notepad++ External PowerShell Lexer"), MB_OK);

}

//  This function returns a pointer to the PowerShell Lexer object.
PowerShell_Lexer* getLexerObj()
{

	return ( &lexer);

}


//*********************************************************************************************
//  Notification Handlers.

//  This notification handler updates the keyword lists and highlighters and forces a new lexing.
void WORDSTYLESUPDATEDproc()
{

	lexer.StylesUpdatedCall = true;
	lexer.updateHighlighterStyles();

	//  If another view open with a PowerShell document we need to update it as well.
	int targetView = ( npp_plugin::intCurrView() == MAIN_VIEW ) ? ( SUB_VIEW ) : ( MAIN_VIEW );
	int targetIndex = messageProc( NPPM_GETCURRENTDOCINDEX, 0, targetView );

	HWND hTargetView = ( targetView == MAIN_VIEW ) ? ( npp_plugin::hMainView() ) : ( npp_plugin::hSecondView() );
	int targetSCILEXERID = ::SendMessage( hTargetView, SCI_GETLEXER, 0, 0 );

	lexer.prevHwndFocused = ::GetFocus();
	
	if ( ( targetIndex >= 0 ) &&  ( targetSCILEXERID == lIface::getSCILexerIDByName("PowerShell*") ) ) {

		//  The doc in the alternate view is open and needs updating.
		lexer.processAltView = true;
		messageProc( NPPM_ACTIVATEDOC, targetView, targetIndex );

	}
	else {

		//  Restart styling for this view only.
		messageProc(SCI_STARTSTYLING, -1, 0);

	}
}

void setDocModified( bool modified ) {
	lexer.docModified = modified;
};

void setLanguageChanged( bool changed) {
	lexer.languageChanged = changed;
};


}	// End: namespace NppExtLexer_PowerShell

