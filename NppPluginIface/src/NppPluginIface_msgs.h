/* NppPlugin_msgs.h
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
 *  Notepad++ Plugin Interface Lib enhances the standard Notepad++ plugin interface defined in
 *  Notepad++'s PluginInterface.h.
 *
 *  Using the namespace 'npp_plugin' for public functions, and nested unnamed namespaces 
 *  for private ones.  Extensions to the lib are done with nested namespaces.
 *
 */

/*
 *  This file is a common file that defines inter plugin messages, using 'const unsigned int'
 *  instead of a global #define macro.  Add more to this file and post back to the Notepad++
 *  Plugin Interface Lib thread at:
 *    *****  Place thread pointer here *****
 *
 *
 *  To make use of the inter-plugin messages both the sender and reciever need to agree on the
 *  messages being sent, this is done via the N++ message NPPM_MSGTOPLUGIN which is defined:
 *
 *    // BOOL NPPM_MSGTOPLUGIN(TCHAR *destModuleName, CommunicationInfo *info)
 *    // return value is TRUE when the message arrive to the destination plugins.
 *    // if destModule or info is NULL, then return value is FALSE
 *    struct CommunicationInfo {
 *      long internalMsg;
 *      const TCHAR * srcModuleName;
 *      void * info; // defined by plugin
 *    };
 *
 */

#ifndef NPP_PLUGININTERFACE_MSGS_H
#define NPP_PLUGININTERFACE_MSGS_H

#include "Notepad_plus_msgs.h"

namespace npp_plugin {

namespace messages {

//  <---  NppPlugin messages.  --->
const int NPPP_MSG = ( NPPMSG + 10000 );
const int NPPP_RMSG = ( NPPMSG + 11000 );

//  <---  NppPlugin_SciMarkerSymbol.dll messages --->
const int NPPP_MSG_MARKERSYMBOL = ( NPPP_MSG + 1 );
const int NPPP_MSG_MARKERUNDEFINE = ( NPPP_MSG + 2);

	//  MSG_MARKERSYMBOL info structure.
	struct info_MARKERSYMBOL {
		int markerNumber;
		int targetView;
		int markerSymbol;
		info_MARKERSYMBOL( int markerNumber, int targetView )
			:markerNumber(markerNumber), targetView(targetView), markerSymbol(-1){};
	};

}  //  End namespace: messages

}  //  End namespace: npp_plugin

#endif  //  End include guard:  NPP_PLUGININTERFACE_MSGS_H