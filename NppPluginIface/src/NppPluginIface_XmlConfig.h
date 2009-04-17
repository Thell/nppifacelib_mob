/* NppPluginIface_XmlConfig.h
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
 *  Notepad++ Plugin Interface Lib extension providing TinyXML access for retrieval and storage
 *  of a plugin's configuration parameters stored in the plugins default xml file.
 *
 */

#ifndef NPP_PLUGININTERFACE_XMLCONFIG_EXTENSION_H
#define NPP_PLUGININTERFACE_XMLCONFIG_EXTENSION_H

//  <--- STL --->
#include <algorithm>
#include <iterator>
#include <string>

//  <--- Npp Plugin Interface Lib --->
#include "NppPluginIface.h"

//  <--- Windows --->
#include "Shlwapi.h"
#pragma comment( lib, "Shlwapi.lib" )


//  <--- Notepad++ --->
#define TIXML_USE_STL
#include "tinyxml.h"
/*
	Additional Dependencies against the N++ .obj files for:
	"tinystr"
	"tinyxml"
	"tinyxmlerror"
	"tinyxmlparser"
*/

//  <--- Boost --->
//  Should be added to your Visual Studio VC++ directories.
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>

namespace npp_plugin {

//  Namespace extension for plugin XMLConfig data.
namespace xmlconfig {

tstring getGUIConfigValue( tstring name, tstring attrib );
bool setGUIConfigValue( tstring name, tstring attrib, tstring value );
TiXmlDocument* get_pXmlPluginConfigDoc( bool silent = false );

}  // End Namespace: xmlconfig

}  // End Namespace: npp_plugin

#endif // End include guard:  NPP_PLUGININTERFACE_XMLCONFIG_EXTENSION_H