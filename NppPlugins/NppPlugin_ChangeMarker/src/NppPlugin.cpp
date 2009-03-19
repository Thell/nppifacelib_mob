/* NppPlugin.cpp
 *
 * This file is part of the Notepad++ Plugin Interface Lib.
 * Copyright 2009 Thell Fowler (thell@almostautomated.com)
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
 *  This is the main Notepad++ Plugin file for use with Notepad++ Plugin Interface Lib.
 *
 */

#include "NppPlugin.h"
#include "NppPlugin_ChangeMarker.h"

/*
 *  The v_getfuncarray namespace alias allows for emulation of a class's 'virtual' function by
 *  providing a 'symlink' like pointer to whichever npp_plugin namsespace extension that will
 *  be responsible for responding to the N++ PluginsManager getfuncArray call.
 *
 *  An example of this is npp_plugin::external_lexer::getfuncArray() which adds lexer funcItems
 *  to the array and sorts them, then appends the plugin's primary funcItems in the order
 *  the were registered.
 *
 */
namespace v_getfuncarray = npp_plugin;

//  This plugins namespace alias
namespace p_cm = npp_plugin_changemarker;

//  <--- Required Plugin Interface Routines --->
BOOL APIENTRY DllMain(HANDLE hModule, DWORD reasonForCall, LPVOID /*lpReserved*/)
{
	using namespace npp_plugin;

	switch (reasonForCall)
	{

	case DLL_PROCESS_ATTACH:

		// <--- Base initialization of the plugin object --->
		npp_plugin::initPlugin(TEXT("Change Markers"), hModule);

		// <--- Base menu function items setup --->
		setPluginFuncItem(TEXT("Jump: Prev Change"), p_cm::jumpPrevChange,p_cm::CMD_JUMPPREV);
		setPluginFuncItem(TEXT("Jump: Next Change"), p_cm::jumpNextChange,p_cm::CMD_JUMPNEXT);
		setPluginFuncItem(TEXT(""), NULL);	//  A separator line.
		setPluginFuncItem(TEXT("Display: Line Number Margin"), p_cm::displayWithLineNumbers, p_cm::CMD_LINENUMBER , true);
		setPluginFuncItem(TEXT("Display: Bookmark Margin"), p_cm::displayWithBookMarks, p_cm::CMD_BOOKMARK, true);
		setPluginFuncItem(TEXT("Display: Plugin Marker Margin"), p_cm::displayInPluginMargin, p_cm::CMD_PLUGIN, true);
		setPluginFuncItem(TEXT("Display: As Line Highlight"), p_cm::displayAsHighlight, p_cm::CMD_HIGHLIGHT, true);
		setPluginFuncItem(TEXT(""), NULL);	//  A separator line.
		setPluginFuncItem(TEXT("Disable"), p_cm::disable, p_cm::CMD_DISABLE, true);
		setPluginFuncItem(TEXT("About..."), npp_plugin::About_func);

#ifdef NPP_PLUGININTERFACE_CMDMAP_EXTENSION_H
		// <--- Initalize internal cmdId to funcItem cmdId map. --->
		createCmdIdMap();
#endif
		break;

	case DLL_PROCESS_DETACH:
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	}

	return TRUE;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *FuncsArraySize)
{
	/*
	 *  PluginsManager stores pointers to the functions that are exposed, allowing dialog and
	 *  menu interaction.  The base plugin's functions to retrieve the size and array can be
	 *  'virtualized' by altering the namespace alias located near the top this file.
	 *
	 */

	*FuncsArraySize = v_getfuncarray::getPluginFuncCount();

	return ( v_getfuncarray::getPluginFuncArray() );

}


//---------------------------------------------------------------------------------------------
//  Notepad++ and Scintilla Communications Processing
				
	/*
	 *  Listings of notifications and messages are in Notepad_plus_msgs.h and Scintilla.h.
	 *
	 *  Notifications use the SCNotification structure described in Scintilla.h.
	 *
	 */

//  This function gives access to Notepad++'s notification facilities including forwarded
//  notifications from Scintilla.
extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	/*
	 *  
	 *  Notifications can be filtered, and language specific handlers called using a
	 *  Namespace::Function() call.
	 *
	 *  To route a notification to one of this plugin's registered lexers use the
	 *  external_lexer::getSCILexerIDByName("LexerName") and compare with the  value returned
	 *  by messageProc(SCI_GETLEXER, 0, 0).
	 *
	 */
	using namespace npp_plugin;

//#define MSG_DEBUGGING
#ifdef MSG_DEBUGGING
	npp_plugin::hCurrViewNeedsUpdate();
	HWND hView = reinterpret_cast<HWND>(notifyCode->nmhdr.hwndFrom);
	tstring ViewName;
	if ( hView == hMainView() ) ViewName.assign( TEXT("MAIN_VIEW") );
	else if ( hView == hSecondView() ) ViewName.assign( TEXT("SUB_VIEW") );
	else if ( hView == hNpp() ) ViewName.assign( TEXT("Notepad++") );
	else ViewName.assign( TEXT("NON_VIEW") );  // ie: Find/Replace

	uptr_t idFrom = notifyCode->nmhdr.idFrom;
	int idPos = ::SendMessage( hNpp(), NPPM_GETPOSFROMBUFFERID, idFrom, 0);
	int idPos1 = idPos >> 30;
	int idPos2 = idPos & 0xdf;
	int bufferID = ::SendMessage( hView, NPPM_GETCURRENTBUFFERID, 0, 0);
	static int mvFocusedBuffID = 0;
	static int svFocusedBuffID = 0;
	int buffPos = ::SendMessage( hNpp(), NPPM_GETPOSFROMBUFFERID, bufferID, 0);
	int buffPos1 = buffPos >> 30;
	int buffPos2 = buffPos & 0xdf;
	int msg = notifyCode->nmhdr.code;
	int pDoc = (LRESULT)::SendMessage( hView, SCI_GETDOCPOINTER, 0, 0);
	int mvPDoc = (LRESULT)::SendMessage( hMainView(), SCI_GETDOCPOINTER, 0, 0);
	static int mvFocusedPDoc = 0;
	int svPDoc = (LRESULT)::SendMessage( hSecondView(), SCI_GETDOCPOINTER, 0, 0);
	static int svFocusedPDoc = 0;
	int flags = notifyCode->modificationType;
	TCHAR flagHEX[65];
	::_itot(flags, flagHEX, 16);
	TCHAR flag2[65];
	::_itot(flags, flag2, 2);
	// Breakpoint string:
	// {msg} {ViewName} idFrom:{idFrom}:[{idPos1}]:[{idPos2}] buffID:{bufferID}:[{buffPos1}]:[{buffPos2}] Doc:(curr){pDoc}|mv[{mvPDoc}]|sv[{svPDoc}] flags:({flags}):({flagHEX}):({flag2})
#endif
	
	switch (notifyCode->nmhdr.code) 
	{
	case SCN_MODIFIED:
		if ( npp_plugin::isNppReady() ) {
			if ( ( notifyCode->modificationType & ( SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT ) ) ||
				( notifyCode->modificationType & ( SC_MOD_BEFOREDELETE ) ) ) {
				npp_plugin::hCurrViewNeedsUpdate();
				p_cm::modificationHandler(notifyCode);
			}
		}
		break;

	case NPPN_READY:
		npp_plugin::setNppReady();
		npp_plugin::hCurrViewNeedsUpdate();
		npp_plugin::doctabmap::update_DocTabMap();
		break;

	case NPPN_TBMODIFICATION:
		p_cm::initPlugin();
		break;

	case NPPN_WORDSTYLESUPDATED:
		p_cm::wordStylesUpdatedHandler();
		break;

	case NPPN_BUFFERACTIVATED:
		if ( isNppReady() ) {
			npp_plugin::hCurrViewNeedsUpdate();
			npp_plugin::doctabmap::update_DocTabMap();
		}
		break;

	case NPPN_FILEBEFOREOPEN:
		npp_plugin::hCurrViewNeedsUpdate();
		break;

	case NPPN_FILEOPENED:
		npp_plugin::hCurrViewNeedsUpdate();
		break;

	case NPPN_FILESAVED:
		p_cm::fileSaveHandler(notifyCode);
		break;

	default:
		break;
	}
}


extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	/*
	 *  This function give access to Notepad++'s messaging facilities.  It was originally
	 *  intended for the relay of Notepad++ messages and inter-plugin messages, yet having
	 *    ::SendMessage( hCurrentView, SOME_MESSAGE_TOKENNAME, value, value );
	 *  mixed in all over the place was ugly so this plugin uses messageProc to keep all of
	 *  these in one area.
	 *
	 *  This function either returns true or the return value that comes from each individual
	 *  handler.  So be sure to explicitly state the return in the switch case.
	 *
	 *  Use the npp_plugin:: hCurrView, hMainView, hSecondView, and hNpp for the standard handles,
	 *  some messages needs to be sent to both the main and second handle to get the desired
	 *  results.  ( Indicator setup messages are one example. )
	 *
	 *  See Notepad_plus_msgs.h and Scintilla.h for notification IDs.
	 *
	 *  Some messages sent to N++ get forwarded back here by N++ plugin notify.  For instance
	 *  NPPM_DOOPEN sent to messageProc will recieved a messageProc message again by the
	 *  pluginNotify routine, so it needs to be sent via ::SendMessage
	 *
	 */

	using namespace npp_plugin;
	namespace msg = npp_plugin::messages;
	namespace mark = npp_plugin::markers;

	// ===>  Include optional messaging handlers here.
	switch (Message)
	{

		//  <---  Notepad++ Messages --->
		case NPPM_MSGTOPLUGIN:
		{
			//  Inter-Plugin messaging
			CommunicationInfo* comm = reinterpret_cast<CommunicationInfo *>(lParam);
			
			switch ( comm->internalMsg )
			{

				default:
					break;

			break;
			}

		break;
		}
		
		default:
			return false;

	}  // End: Switch ( Message )

	return TRUE;
}

//  Plugin Helper Functions
void npp_plugin::About_func() 
{
	::MessageBox(npp_plugin::hNpp(),
		TEXT("  Change Markers is a plugin that marks lines that have changed\n")
		TEXT("  in a document since it was last saved and since it was last loaded.\r\n\r\n")
		TEXT("  You can also setup a shortcut for jumpPrevChange and jumpNextChange to\n")
		TEXT("  move through the current document.\r\n\r\n")
		TEXT("  Hopefully you find it to be a useful tool.  Enjoy!\r\n")
		TEXT("  Thell Fowler (almostautomated)"),
		TEXT("About This Plugin"), MB_OK);
}
