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
		setPluginFuncItem(TEXT("Display: With Line Numbers"), p_cm::displayWithLineNumbers, p_cm::CMD_LINENUMBER , true);
		setPluginFuncItem(TEXT("Display: With Bookmarks"), p_cm::displayWithBookMarks, p_cm::CMD_BOOKMARK, true);
		setPluginFuncItem(TEXT("Display: Left of Line Text"), p_cm::displayInPluginMargin, p_cm::CMD_PLUGIN, true);
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

	switch (notifyCode->nmhdr.code) 
	{
	case SCN_MODIFIED:
		if ( npp_plugin::isNppReady() ) p_cm::modificationHandler(notifyCode);

	case NPPN_READY:
		npp_plugin::setNppReady();
		break;

	case NPPN_TBMODIFICATION:
		p_cm::init();
		break;

	case NPPN_WORDSTYLESUPDATED:
		p_cm::wordStylesUpdatedHandler();
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

				case msg::NPPP_RMSG_MARKERSYMBOL:
				{
					msg::info_MARKERSYMBOL* _info = reinterprest_cast<msg::info_MARKERSYMBOL *>( comm->info );
					mark::_gotMarkerTypeReply( _info->markerNumber, _info->markerSymbol );
				break;
				}

				default:
					break;

			break;
			}

		break;
		}
		
		//  <--- Internal Plugin Messages --->
		case msg::NPPP_RMSG_MARKERGETAVAIL:
			p_cm::initMarker( lParam );

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
