#ifndef NPPEXTLEXER_POWERSHELL_H
#define NPPEXTLEXER_POWERSHELL_H

// NppExtLexer_PowerShell.h

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
//          External Lexer Definitions for PowerShell Script Files
//				for use the Notepad++ External Lexers Plugin
///////////////////////////////////////////////////////////////////////////////////////////////


//  <--- Include Directives --->
#include "NppPlugin.h"				//  Provides all of the interface, messaging definitions,
									//  and namespace aliases.


namespace NppExtLexer_PowerShell {

//  Constants used in initializing and tracking highlighters.
const unsigned int INDICBASE = 21;		//  SCI indic value + this base equals style state ID value
const unsigned int INDICMAX = 4;		//  Number of indicators
const bool DIR_UP = true;
const bool DIR_DOWN = false;

//*********************************************************************************************
// Nested namespace to place only the style state IDs
namespace Styles {

	/*
	 *  Style State IDs ( see the .cpp file for keyword mapping notes )
	 *	A list of all of our styleIDs as found in the style xml, along with a few that are used
	 *  only for internal uses.
	 */

const int DEFAULT = 0;
const int OPERATOR = 1;					// Raw operators and matched within KeywordClass
const int ESCAPE = 2;
const int NUMBER = 3;
const int COMMENT = 4;
const int MULTILINECOMMENT = 5;
const int STRING = 6;
const int STRINGLITERAL = 7;
const int HERESTRING = 8;
const int HERESTRINGLITERAL = 9;
const int VARIABLE = 10;
const int PROPERTY = 11;
const int METHOD = 12;
const int IDENTIFIER = 13;
const int TYPE = 14;
const int KEYWORD = 15;					// Matched within KeywordClass
const int ALIAS = 16;					// Matched within KeywordClass
const int CMDLET = 17;					// Matched within KeywordClass
const int PARAMETER = 18;
const int CMDLETPARAMPARAM = 19;		// Matched within KeywordClass
const int CMDLETPARAMATTRIB = 20;		// Matched within KeywordClass

const int INDICEXPR = 21;				// highlight for '$(' to matching ')'
const int INDICPARM = 22;				// highlight for PARAMETER
const int INDICPROP = 23;				// highlight for PROPERTY
const int INDICMETH = 24;				// highlight for METHOD to matching ')'

const int MEMBER = 40;					// group state for properties and methods
const int EVALUATION = 41;				// state used for style nesting determinations

};  // End namespace Style


//*********************************************************************************************
//  Lexer Class

	/*
	 *  This lexer class provides an interface to some common, but language specific, functions
	 *  as well as providing some status tracking to help minimize broken states and resetting
	 *  of highlighter data.
	 *
	 */

// Nested state tracking
struct StateNest
{
	int preNestState;
	int NestedState;
	int prevWordState;
	unsigned int GroupLevel;
	int GroupStartPos;
	char GroupTerminator;
};


class PowerShell_Lexer
{
public:
	bool StylesUpdatedCall;		//  Flags the next lex call to only do Updating of Wordlists.
	bool processAltView;		//  True if two views are open for this lang and WORDSTYLESCHANGED notification arrives.
	HWND prevHwndFocused;
	bool languageChanged;		//  Flag to ensure a full doc lexing when switching from a different language.
	bool docModified;			//  Flag to help determine if lexing actually needs to be redone.
	bool HliteStyleChanged;		//  Flag to cause all highlights to be removed from the doc and re-drawn. 
	bool fullDocProcessing;		//  Flag to indicate that safe-entry routines shouldn't be called.

	//=======================================================//

	class Highlighter
	{
	public:
		int SCI_INDICID;
		int NPP_STYLEID;
		bool Active;
		bool clearWhiteSpace;			//  For multiline indicators, flag if whitespace should be colored.
		int SCI_INDICFORE;				//  RGB color.
		int SCI_INDICALPHA;				//  An alpha blend transparency.
		bool SCI_INDICUNDER;			//  Controls if the highlighter is drawn over the text or under it.
		bool StyleChanged;				//  Flag highlighter style changes in the preferences dialog.
		bool forceReDraw;				//  Forces complete erasure and redraw of the indicator.
	
		Highlighter()
			:Active(false), StyleChanged(false)
		{
			//
		};
		~Highlighter(){};

		Highlighter init()
		{
			//  Setup a temporary highlighter and compare to the existing to check for style change.

			Highlighter tmpHlite;
			tmpHlite.SCI_INDICID = this->SCI_INDICID;
			tmpHlite.NPP_STYLEID = ( INDICBASE + tmpHlite.SCI_INDICID );
			
			//  TODO: create a config control to mark active/inactive.  For now use the style's 'bold' setting.
			tmpHlite.Active = ( 1 == messageProc( SCI_STYLEGETBOLD, tmpHlite.NPP_STYLEID, 0 ) );

			tmpHlite.SCI_INDICUNDER = ( 1 ==
				messageProc( SCI_STYLEGETUNDERLINE, tmpHlite.NPP_STYLEID, 0 ) );

			tmpHlite.clearWhiteSpace = ( 1 ==
				messageProc( SCI_STYLEGETITALIC, tmpHlite.NPP_STYLEID, 0 ) );

			// Setup the indicator color values
			tmpHlite.SCI_INDICFORE = messageProc(SCI_STYLEGETBACK, tmpHlite.NPP_STYLEID, 0);
			
			// Create a nice alpha blend value for the indicator
			int R = GetRValue(tmpHlite.SCI_INDICFORE);
			int G = GetGValue(tmpHlite.SCI_INDICFORE);
			int B = GetBValue(tmpHlite.SCI_INDICFORE);
			int iMax = max(R, max(G, B));
			int iMin = min(R, min(G, B));
			tmpHlite.SCI_INDICALPHA = 255 - (((iMax + iMin) * 240 + 255) / 510);

			if (! ( tmpHlite.Active == this->Active &&
					tmpHlite.SCI_INDICFORE == this->SCI_INDICFORE &&
					tmpHlite.SCI_INDICALPHA == this->SCI_INDICALPHA &&
					tmpHlite.clearWhiteSpace == this->clearWhiteSpace &&
					tmpHlite.SCI_INDICUNDER == this->SCI_INDICUNDER ) ||
					! npp_plugin::isNppReady() ) {
				//  A style setting changed, update Scintilla indicator values.
				tmpHlite.StyleChanged = true;

				if ( tmpHlite.Active ) {
					// Send Scintilla the new indicator settings.
					messageProc(SCI_INDICSETSTYLE, tmpHlite.SCI_INDICID , INDIC_ROUNDBOX);
					messageProc(SCI_INDICSETFORE, tmpHlite.SCI_INDICID, tmpHlite.SCI_INDICFORE);
					messageProc(SCI_INDICSETALPHA, tmpHlite.SCI_INDICID, tmpHlite.SCI_INDICALPHA);
					messageProc(SCI_INDICSETUNDER, tmpHlite.SCI_INDICID, tmpHlite.SCI_INDICUNDER);
				}
				else {
					// Highlighter is no longer active, so remove it from Scintilla.
					messageProc(SCI_INDICSETSTYLE, tmpHlite.SCI_INDICID , 0);
					messageProc(SCI_INDICSETFORE, tmpHlite.SCI_INDICID, 0);
					messageProc(SCI_INDICSETALPHA, tmpHlite.SCI_INDICID, 0);
					messageProc(SCI_INDICSETUNDER, tmpHlite.SCI_INDICID, 0);
				}
			}

			return ( tmpHlite );

		};

	};  // End: class Highlighter.

	std::vector<PowerShell_Lexer::Highlighter> Hlite;	// Storage container for Highlighter data.

	class LineState
	{
		friend class PowerShell_Lexer;

	public:
		bool IsMultilineStyle ( int line )
		{
			_lineState.SCI_LineState = _accessor->GetLineState( line );
			return ( _lineState.bits.multilineStyle );
		};

		void flagMultilineStyle ( int line )
		{

			_lineState.SCI_LineState = _accessor->GetLineState( line );
			_lineState.bits.multilineStyle = true;
			_accessor->SetLineState( line, _lineState.SCI_LineState );

		};

		void clearMultilineStyle ( int line )
		{

			_lineState.SCI_LineState = _accessor->GetLineState ( line );
			_lineState.bits.multilineStyle = false;
			_accessor->SetLineState( line, _lineState.SCI_LineState );

		};

		bool IsMultiline( int line )
		{

			if (! ( line == _prevHliteLineChecked ) ) {
				_lineState.SCI_LineState = _accessor->GetLineState( line );
				setLineChecked( line );
			}
			
			return ( _lineState.bits.multilineHlite );

		};

		int getMultilineStart( int line )
		{

			if (! ( line == _prevHliteLineChecked ) ) {
				_lineState.SCI_LineState = _accessor->GetLineState( line );
				setLineChecked( line );
			}

			return ( _lineState.bits.startLine );

		};

		int getMultilineEnd( int line )
		{

			if (! ( line == _prevHliteLineChecked ) ) {
				_lineState.SCI_LineState = _accessor->GetLineState( line );
				setLineChecked(line);
			}

			return ( _lineState.bits.endLine );

		};

		void setMultlineState(int startLine, int endLine)
		{

			for (int currLine = startLine; currLine <= endLine; currLine++) {

				_lineState.SCI_LineState = _accessor->GetLineState( currLine );
				_lineState.bits.multilineHlite = true;
				_lineState.bits.startLine = startLine;
				_lineState.bits.endLine = endLine;
				_accessor->SetLineState( currLine, _lineState.SCI_LineState );

			}

		};

		//  Clears the line state data from an Accessor.
		void ClearMultilineState(int startLine, int endLine)
		{

			for (int currLine = startLine; currLine <= endLine; currLine++) {

				_lineState.SCI_LineState = _accessor->GetLineState( currLine );
				_lineState.bits.multilineHlite = false;
				_lineState.bits.startLine = 0;
				_lineState.bits.endLine = 0;
				_accessor->SetLineState( currLine, _lineState.SCI_LineState );

			}

		};

		//  Sets the accessor pointer
		void setAccessor ( Accessor& accessor )
		{
			_accessor = &accessor;
		};
	
	private:
		typedef union
		{
			/*
			 *  This union is used to store beginning and ending information for highlighters
			 *  that span multiple lines.  Normally a call to Scintilla asking for the start 
			 *  and end positions would work just fine.  This lexer can't use that due to
			 *  removing the indicator from leading white space on multiline highlights.
			 *
			 */

			struct 
			{	
				// 32 bit field for filling in styler linestate value
				unsigned multilineStyle :1;		// Flags multiline style states that can embed other styles.
				unsigned multilineHlite	:1;		// Flag for a highlight on the current line that spans multiple lines.
				unsigned unused			:6;
				unsigned startLine		:12;	// values up to 4095
				unsigned endLine		:12;	// values up to 4095

			} bits;

			unsigned int SCI_LineState;

		} LineStateStruct;
		Accessor* _accessor;

		void setLineChecked(int line) {
			_prevHliteLineChecked = line;
		};

		LineStateStruct _lineState;
		int _prevHliteLineChecked;


	}; // End: Class LineState

	friend class LineState;
	PowerShell_Lexer::LineState lineState;

	//======================================================//
	PowerShell_Lexer()
		:_Initialized(false), StylesUpdatedCall(false)
	{
		//  Allocate the space needed for highlighters.
		Hlite.reserve(INDICMAX);

	};

	~PowerShell_Lexer(){};

	void initLexer( char* words[], Accessor& accessor )
	{ 
		_pAccessor = &accessor;
		lineState.setAccessor( accessor );
		
		if(! _Initialized ) {

			//  Initialize Wordlists.
			updateWordlists( words );

			//  Initialize Highlighters.
			if ( Hlite.size() == 0 ) {
				for( int i = 0; i < INDICMAX; i++) {
					Highlighter tmpHlite;
					Hlite.push_back(tmpHlite);
					Hlite.at(i).SCI_INDICID = i;
					tmpHlite = Hlite.at(i).init();
					Hlite.at(i) = tmpHlite;
					//  Since this is an initial setup reset the StyleChanged flag.
					Hlite.at(i).StyleChanged = false;
				}
			}
			else {
				updateHighlighterStyles();
			}

			if ( npp_plugin::isNppReady() ) _Initialized = true;

		}
	};

	void updateHighlighterStyles();
	void updateWordlists(char* words[]);
	void PowerShell_Lexer::activateAltView();
	bool doFullDoc(int startPos);

	//  <--- Lexer Helper Functions --->
	void CaptureEscapeChars( StyleContext &sc );
	bool IsSafe_Style_currPos( int style );
	bool IsSafe_Style_prevEOL( int style );
	int getSafe_Style_StartPos( int currPos );
	bool IsEqual( const int first, ... );

	inline bool IsEOL(const int ch, const int chNext)
	{
		return (ch == '\r' && chNext != '\n') || (ch == '\n');
	};


	//  <--- Folder Helper Functions --->
	bool IsCommentStyle(int style);
	bool IsHashCommentLine(int line);

	//  <--- Highlighter Helper Functions --->
	int getSafe_Hlite_Pos(int currPos, int length, bool direction);


private:

	Accessor* _pAccessor;
	bool _Initialized;			//  The first run of init() forces wordlist and highlighter initialization.

};

///////////////////////////////////////////////////////////////////////////////////////////////
//  Generic Externally called functions.

//  This is the entry point called by the external lexer interface.
void LexOrFold(bool LexorFold, unsigned int startPos, int length, int initStyle,
                  char *words[], WindowID window, char *props);

//  This is the menu dialog function item.
void menuDlg();

//  This will return the pointer to this the PowerShell lexer object.
PowerShell_Lexer* getLexerObj();

//  Notification Handlers
void WORDSTYLESUPDATEDproc();
void setDocModified(bool modified);
void setLanguageChanged(bool changed);


}	// End: namespace NppExtLexer_PowerShell

#endif  //  End: include guard NPPEXTLEXER_POWERSHELL_H
