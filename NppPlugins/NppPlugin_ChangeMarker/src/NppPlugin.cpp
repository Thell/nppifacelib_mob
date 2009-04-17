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
		setPluginFuncItem(TEXT("Jump: Prev Change"), p_cm::jumpChangePrev,p_cm::CMD_JUMPCHANGEPREV);
		setPluginFuncItem(TEXT("Jump: Next Change"), p_cm::jumpChangeNext,p_cm::CMD_JUMPCHANGENEXT);
		setPluginFuncItem(TEXT("Jump: Changed Line Up"), p_cm::jumpLineUp,p_cm::CMD_JUMPLINEUP);
		setPluginFuncItem(TEXT("Jump: Changed Line Down"), p_cm::jumpLineDown,p_cm::CMD_JUMPLINEDOWN);
		setPluginFuncItem(TEXT(""), NULL);	//  A separator line.
		setPluginFuncItem(TEXT("Display: Line Number Margin"), p_cm::displayWithLineNumbers, p_cm::CMD_LINENUMBER , true);
		setPluginFuncItem(TEXT("Display: Change Mark Margin"), p_cm::displayWithChangeMarks, p_cm::CMD_CHANGEMARK, true);
		setPluginFuncItem(TEXT("Display: As Line Highlight"), p_cm::displayAsHighlight, p_cm::CMD_HIGHLIGHT, true);
		setPluginFuncItem(TEXT(""), NULL);	//  A separator line.
		setPluginFuncItem(TEXT("Disable Tracking for this Document"), p_cm::disableDoc, p_cm::CMD_DISABLEDOC, true);
		setPluginFuncItem(TEXT("Disable Plugin"), p_cm::disablePlugin, p_cm::CMD_DISABLEPLUGIN, true);
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


//  This function gives access to Notepad++'s notification facilities including forwarded
//  notifications from Scintilla.
extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	using namespace npp_plugin;

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
		p_cm::wordStylesUpdatedHandler();	//  Force an init of the style controller.
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
			p_cm::bufferActivatedHandler( notifyCode );
		}
		break;

	case NPPN_FILEBEFORECLOSE:
		npp_plugin::hCurrViewNeedsUpdate();
		p_cm::fileBeforeCloseHandler( notifyCode );
		break;

	case NPPN_FILESAVED:
		npp_plugin::hCurrViewNeedsUpdate();
		p_cm::fileSaveHandler();
		break;

	default:
		break;
	}
}


extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	//  Normally a plugin would use a switch case to check the Messages, but
	//  this plugin only makes use of a single internal message...

	if ( Message == npp_plugin::PIFACE_MSG_NPPDATASET ) {
		//  Confirm availability of the config file.
		TiXmlDocument * xmlDoc( npp_plugin::xmlconfig::get_pXmlPluginConfigDoc( true ) );
		if (! xmlDoc ) {
			//  Create a new default config file.
			p_cm::generateDefaultConfigXml();
		}
	}

	return TRUE;
}

//  Plugin Helper Functions
void npp_plugin::About_func() 
{
	::MessageBox(npp_plugin::hNpp(),
		TEXT("  Change Markers is a plugin that marks lines that have changed in a document since it was")
		TEXT("  since it was last loaded, and since it was last saved.\r\n\r\n")
		TEXT("  You can view lines with un-saved changes using the Jump menu commands.\n" )
		TEXT("  Use Jump: Prev Change and Next Change to move through the changes in the order they were made.\n")
		TEXT("  Use Jump: Changed Line Up and Down to move to the first un-saved change found in the direction chosen.\r\n\r\n")
		TEXT("  Disabling change tracking for a document will clear all markers and reset the change tracker.  This\n")
		TEXT("  can be used to clear old change marks, and keep your undo history, and not need to reload the document.\n") 
		TEXT("  Disabling the whole plugin will stop all change processing.  If you have several large documents and\n")
		TEXT("  will be doing bulk changes disabling the plugin will help speed up the process.\r\n\r\n")
		TEXT("  Hopefully you find this to be a useful tool.  Enjoy!\r\n")
		TEXT("  Thell Fowler (almostautomated)"),
		TEXT("Change Markers 1.1.0"), MB_OK);
}
