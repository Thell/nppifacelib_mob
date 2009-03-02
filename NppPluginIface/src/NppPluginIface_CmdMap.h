/* NppPluginIface_CmdMap.h
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
 *  Notepad++ Plugin Interface Lib extension providing funcItem cmdID mapping from the plugin's
 *  internal cmdID to the funcItems N++ assigned/modified cmdID.
 *
 */

#ifndef NPP_PLUGININTERFACE_CMDMAP_EXTENSION_H
#define NPP_PLUGININTERFACE_CMDMAP_EXTENSION_H

#include "NppPluginIface.h"

namespace npp_plugin {

void createCmdIdMap ();	//  Creates a cmdID to funcItem mapping prior.
int getCmdId ( int cmdId );		//  Returns the N++ assigned cmdId matching the plugin's assigned cmdId.


} // End namespace: npp_plugin

#endif //  End include guard: NPP_PLUGININTERFACE_CMDMAP_EXTENSION_H