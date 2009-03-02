/* NppPlugin.h
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
  *  These includes provide plugins with functions that enhance the N++ plugin interface.
  *  The basic interface enhancement provides plugin registration and handle controls.
  *  The ExtLexer extension provides styler registration and forces N++ to read a styles
  *  xml, which makes it easy to add 'Global Styles'.
  *  Both of those also allow for function registration.  The basic difference between the
  *  two function registrations is the base functions display under a separator in the order
  *  registered, and the ExtLexer registered functions get sorted and display above the
  *  separator.
  *
  *  The XmlConfig extension makes it easy for a plugin to read/write configuration data in
  *  the same way that N++ does.  So, if the plugin provides styling, the plugins' styles
  *  xml can also be used for config params.
  *
  *  The Markers extension provides plugins with functions to read and react to the basic
  *  Notepad++/Scintilla margin and marker settings.  For instance if the plugin provides
  *  a marker the defineMarker function will check for a valid and free marker number to use,
  *  as well as set the mask information for the margins in both views.
  *
  *  TODO:  Two other extensions that would be very nice to have available are dialog related.
  *    1)  A Preferences dialog hook into the Edit Components panel.
  *    2)  A Styler Configurator hook to allow widget registration similar to the 'Global
  *        Overrides' checkboxes.
  *
  */

#ifndef NPP_PLUGIN_H
#define NPP_PLUGIN_H

//  <--- Notepad++ Plugin Interface Library Includes --->
#include "NppPluginIface.h"
#include "NppPluginIface_msgs.h"
//#include "NppPluginIface_XmlConfig.h"
#include "NppPluginIface_Markers.h"
//#include "NppPluginIface_CmdMap.h"
//#include "NppPluginIface_ExtLexer.h"

namespace npp_plugin {

//  This is the base plugin's default menu item.
void About_func();

}  // End: namespace npp_plugin_base


#endif	//  Include Guard:  NPP_PLUGIN_H
