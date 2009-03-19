/* NppPluginIface_CmdMap.cpp
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

//#include <unordered_map>
#include <boost/tr1/unordered_map.hpp>  // Storage and mapping of original FuncItem cmdIDs.

#include "NppPluginIface_CmdMap.h"

namespace npp_plugin {

//  Unnamed namespace for private variable and functions.
namespace {

std::tr1::unordered_map<int, FuncItem*> _cmdIdMap;
std::tr1::unordered_map<int, FuncItem*>::iterator cmdIdMap_AcIter;
typedef std::pair<int, FuncItem*> CmdId_Pair;


} // End un-named namespace

//  Returns the N++ assigned command id matching a set plugin menu function's cmdID.
int getCmdId ( int cmdId )
{ 
	int retVal = 0;

	cmdIdMap_AcIter = _cmdIdMap.find( cmdId );

	if ( cmdIdMap_AcIter == _cmdIdMap.end() ) {
		retVal = 0;
	}
	else {
		FuncItem* tstFuncItem = cmdIdMap_AcIter->second;
		retVal = cmdIdMap_AcIter->second->_cmdID;
	}

	return ( retVal );
}

//  Creates a mapping of the plugins internal funcItem CmdIDs to the funcItem array pointers
//  send to N++ during the PluginsManager getFuncItemArray call during plugin loading.
//  This makes it possible to use an enum of cmd id values within the plugin without having to
//  ensure a particular order to the menu funcItems during funcItem initialization.
void createCmdIdMap ()
{
	int nb_funcItem;
	FuncItem* funcItems = getFuncsArray( &nb_funcItem);

	if ( (! funcItems ) || ( nb_funcItem <= 0 ) ) return;

	for ( int i = 0 ; i < nb_funcItem; i++ ) {
		if (! funcItems[i]._cmdID == NULL ) {
			_cmdIdMap.insert(CmdId_Pair( funcItems[i]._cmdID, &funcItems[i] ) );
		}
	}
}


} // End namespace:  npp_plugin

