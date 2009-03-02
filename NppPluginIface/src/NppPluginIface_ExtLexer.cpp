/* NppPluginIface_ExtLexer.cpp
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

/*
 *  Both Notepad's PluginsManager and Scintilla's LexerManager need to know the details of
 *  lexers included in this plugin.
 *
 *  Notepad++ fills in the language menu and the style preferences dialog, as well as handles
 *  style properties control with Scintilla.
 *
 *  Scintilla does all of the accessing of the document, application of styles, folding, and so
 *  on using the information provided by Notepad++ and this plugin's messaging.
 *
 */

#include "NppPluginIface_ExtLexer.h"

namespace npp_plugin {

//  Namespace Extension for External Lexer Interface
namespace external_lexer {

//  Unnamed namespace for private variables
namespace {

//  Parameter constants - From Npp's Parameters.h
const int NB_MAX_EXTERNAL_LANG = 30;			
const int MAX_EXTERNAL_LEXER_NAME_LEN = 16;
const int MAX_EXTERNAL_LEXER_DESC_LEN = 32;

//  <--- Just for a little clarity in the function calls --->
const bool FOLD = true;
const bool LEX = false;

// <--- Containers --->
std::vector<FuncItem> _LexerFuncVector;	//  Container for lexer function commands.
std::vector<FuncItem> ReturnVector;		//  Container for the ordered function commands.
std::vector<Lexer> _LexerDetailVector;  //  Container for lexer details.

//  Required Notepad++ External Lexer Function
//  Returns a count of lexers initialized from this plugin.
int __stdcall GetLexerCount() {	return _LexerDetailVector.size(); }

//  Returns a count of all lexer menu function items in this plugin.
int getLexerFuncCount () { return _LexerFuncVector.size(); }

//  Both Notepad++ and Scintilla use this.  Scintilla expects char and Notepad++ will
//  convert it for unicode if needed.
void __stdcall GetLexerName(unsigned int Index, char *name, int buflength)
{
	std::string currLangName = _LexerDetailVector.at(Index)._name;

	if (buflength > 0 ) {
		buflength--;
		int n = currLangName.length();
		if (n > buflength) n = buflength;

		memcpy(name, currLangName.c_str(), n), name[n] = '\0';
	}
}



//  Notepad++ uses this, but Scintilla does not.  Which allows use of TCHAR vs char.
void __stdcall GetLexerStatusText(unsigned int Index, TCHAR *desc, int buflength)
{
	tstring currLangDesc = _LexerDetailVector.at(Index)._description;;

	if (buflength > 0) {
		buflength--;
		int n = currLangDesc.length();
		if (n > buflength) n = buflength;

		TMEMCPY(desc, currLangDesc.c_str(), n), desc[n] = '\0';
	}
}

//  Lexer Function called by Scintilla
//  This is a forwarder to one of this plugins registered lexers LexOrFold functions.
//  Upon an initial call from Scintilla the Scintilla assigned lexer ID is also stored.
void __stdcall Lex(unsigned int langID, unsigned int startPos, int length, int initStyle,
				   char *words[], WindowID window, char *props)
{
	Lexer* currLexer = &_LexerDetailVector.at(langID);

	if (! ( currLexer->SCI_LEXERID >= SCLEX_AUTOMATIC ) || ( npp_plugin::isNppReady() ) ) {
		//  Each lexer has a LEXERID that Scintilla assigns.  For external lexers this ID isn't
		//  known until Scintilla's first call for a lexer to lex a document.
		//  When that happens the ID is stored and used for filtering notification messages.
		int lexerID = ::SendMessage(npp_plugin::hCurrView(), SCI_GETLEXER, 0, 0);
		currLexer->SCI_LEXERID = lexerID;
	}
	else if ( currLexer->SCI_LEXERID < SCLEX_AUTOMATIC ) {
		
		::MessageBox(npp_plugin::hNpp(), TEXT(" SCI_LEXERID is less than SCLEX_AUTOMATIC and Npp reports ready! " ),
			TEXT(" Npp External Lexer Problem "), MB_OK | MB_ICONEXCLAMATION );
		return;	// This shouldn't happen!
	}

	currLexer->_pLexOrFold(0, startPos, length, initStyle, words, window, props);
}

//  Lexer Function called by Scintilla
//  This is a forwarder to one of this plugins registered lexers LexOrFold functions.
void __stdcall Fold(int langID, unsigned int startPos, int length,	int initStyle,
				char *words[], WindowID window, char *props)
{
		Lexer* currLexer = &_LexerDetailVector.at(langID);
		currLexer->_pLexOrFold(1, startPos, length, initStyle, words, window, props);
}

}  //  End:: Unnamed namespace for private implementation.

//  Notepad++ External Lexer Initialization: used by both N++ and Scintilla
//
//  Plugins can contain multiple external lexers registered in the DLLMain, each lexer must
//  have a unique name.
//    name:  text that appears in the N++ languages menu.	/* "LexerName"				*/
//    statusText:  text that appears in the N++ status bar.	/* TEXT("Status-Bar Text")	*/
//    pLexOrFold:  pointer to the lexers LexOrFold funtion.	/* NameSpace::LexOrFold		*/
//    pMenuDlg:  lexer's main menu dialog function.			/* NameSpace::MenuDlg */
void initLexer(std::string Name, tstring statusText, NppExtLexerFunction pLexOrFold,
				PFUNCPLUGINCMD pMenuDlg)
{
	// Notify if length is too long.
	if ( Name.length() > MAX_EXTERNAL_LEXER_NAME_LEN )
		::MessageBox(npp_plugin::hNpp(),
					TEXT("Lexer name is too long and will be truncated."),
					TEXT("Lexer Name Alert"),
					MB_ICONINFORMATION);

	if ( statusText.length() > MAX_EXTERNAL_LEXER_DESC_LEN )
		::MessageBox(npp_plugin::hNpp(),
					TEXT("Lexer description is too long and will be truncated."),
					TEXT("Lexer Description Alert"),
					MB_ICONINFORMATION);


	//  The lexer details vector is used to store the SCI lexerID and the pointer to the
	//  lexers entry point function ( usually LexOrFold() ).
	Lexer thisLexer;

	thisLexer._name.assign(Name);
	thisLexer._description.assign(statusText);
	thisLexer._pLexOrFold = pLexOrFold;
	thisLexer.SCI_LEXERID = NULL;

	_LexerDetailVector.push_back(thisLexer);


	/*
	 *  This plugin extension uses the lexer's name for the menu.  Since the lexer name is also
	 *  used by Notepad++ and Scintilla as a char* value and the FuncItem expects a TCHAR*
	 *  convert it prior to registering the FuncItem.
	 *
	 */

	//  Create a tstring and copy the standard character string into it.  Should work for both
	//  unicode and ansi.
	int len = Name.length();
	tstring itemName = tstring(len, '\0');
	std::copy(Name.begin(), Name.end(), itemName.begin());

	setLexerFuncItem(itemName, pMenuDlg);

}

//  Registers additional menu function items for a lexer that will be displayed in the N++
//  Plugins menu.
void setLexerFuncItem(tstring Name, PFUNCPLUGINCMD pFunction,
				int cmdID, bool init2Check, ShortcutKey* pShKey)
{
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

	_LexerFuncVector.push_back(thisFunction);
}

// Returns the SCI_LEXERID at vector index.
int getSCILexerIDByIndex(int index) { return ( _LexerDetailVector.at(index).SCI_LEXERID ); }

//  Returns the SCI_LEXERID matching name.
//  Returns -1 if no match found.
int getSCILexerIDByName( std::string name )
{
	std::vector<Lexer>::iterator currLexer = _LexerDetailVector.begin();

	for (currLexer; currLexer < _LexerDetailVector.end(); currLexer++ ) {
		if ( currLexer->_name.compare( name ) == 0 ) return currLexer->SCI_LEXERID;
	}

	return ( -1 );
}

//  Provides a response for a namespace's extension to retrieve and work with the
//  plugin's function item vector.
//
//  Useful to merge additional function items together before responding to the
//  Npp plugin manager's getFuncArray() call.
std::vector<FuncItem> getLexerFuncVector() { return ( _LexerFuncVector ); }

//  Provides a response for a namespace's extension to retrieve and work with the
//  plugin's function item vector.
//
//  Useful to merge additional function items together before responding to the
//  Npp plugin manager's getFuncArray() call.
std::vector<Lexer> getLexerDetailVector() { return ( _LexerDetailVector ); }


//  'Virtualized' base plugin FuncItem functions.
//
//  These functions will be used instead of the base plugin functions to allow for the
//  inclusion of a sorted lexer FuncItem list in the menu above the base plugins.
namespace virtual_plugin_func {

//  Provides a response to PluginsManager's getFuncArray.
//  Return this plugin's sorted FuncItem array.
FuncItem * getPluginFuncArray()
{
	/*
	 *  In order to have the function items show up in the Notepad++ plugins menu with
	 *  the base plugins' function items at the bottom a new vector is created and the existing
	 *  vectors are copied into it with the lexer functions first.
	 *
	 */

	//  <--- Local function for sorting criteria predicate. --->
	struct sortByName
	{
	     bool operator()(FuncItem& f1, FuncItem& f2)
	     {
		      return ( _tcscmp(f1._itemName, f2._itemName) < 0 );
		 }
	};

	if ( _LexerFuncVector.empty() ) {
		//  Doesn't look like there are any lexer function items so just send the plugin's.
		ReturnVector = npp_plugin::getPluginFuncVector();
	}
	else {
		//  Sort the lexer function items vector.
		std::sort(_LexerFuncVector.begin(), _LexerFuncVector.end(), sortByName());

		//  Copy it to the vector that will be returned to Notepad++'s PluginsManager.
		ReturnVector = _LexerFuncVector;

		//  Append the base plugin's function items (ie 'Help', 'About', etc...
		std::vector<FuncItem> tmpVector = npp_plugin::getPluginFuncVector();
		std::copy(tmpVector.begin(), tmpVector.end(), std::back_inserter(ReturnVector));
	}

	return ( &ReturnVector[0] );
}

//  Provides a response to PluginsManager's getFuncArray call 'size' and allows for extensions
//  to work with generating an array to send back that includes their own FuncItems.
int getPluginFuncCount()
{
	return ( npp_plugin::getPluginFuncCount() + getLexerFuncCount() );
}


/*
******************  Put this somewhere to keep **********************
*  TODO:  Modify this to open a registered lexer's style configuration.

//  Opens the Notepad++ Style Configurator to the specified Language.
void dlg_StyleConfig()
{
	//  This function doesn't actually change the styles, that is done when Notepad++
	//  sends the WORDSSTYLESUPDATED notification.

	::SendMessage( pIface::hNpp(), NPPM_MENUCOMMAND, 0, IDM_LANGSTYLE_CONFIG_DLG);
	HWND hStyleDlg = ::FindWindow( NULL, TEXT("Style Configurator") );

	int langID = ::SendDlgItemMessage( hStyleDlg, IDC_LANGUAGES_LIST, LB_SELECTSTRING, -1, (LPARAM)TEXT("Changed Line") );
	WPARAM msg = ( LBN_SELCHANGE << 16 ) | (IDC_LANGUAGES_LIST);
	::SendMessage( hStyleDlg, WM_COMMAND, msg, 0);
}
*/

}  // End namespace:  virtual_plugin_func

}  // End namespace:  external_lexer

}  // End namespace:  npp_plugin
