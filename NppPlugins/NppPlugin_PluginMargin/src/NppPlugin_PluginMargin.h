/* NppPlugin_PluginMargin.h
 *
 * This file is part of the Notepad++ Change Marker Plugin.
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

//  Plugin Margin show/hide view control for use with Notepad++.

#include "NppPlugin.h"


//  N++ Change Marker Plugin Specific
namespace npp_plugin_pluginmargin {
	//  Menu Command IDs
enum MENU_COMMANDS {
	CMD_TOGGLEMV = 1,
	CMD_TOGGLESV,
};

void initPluginMargin();
void toggleMV();
void toggleSV();

} // End namespace: npp_plugin_pluginmargin