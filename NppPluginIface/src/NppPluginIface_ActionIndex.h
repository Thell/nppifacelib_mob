/* NppPluginIface_ActionIndex.h
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
 *  Notepad++ Plugin Interface Lib extension providing tracking of document actions
 *  that are indexed to the same count as Scintilla currAction counter for a document.
 *
 *  For an example of using this see the NppPlugin_ChangeMarker plugin sources.
 *
 */


#ifndef NPP_PLUGININTERFACE_ACTIONINDEX_EXTENSION_H
#define NPP_PLUGININTERFACE_ACTIONINDEX_EXTENSION_H

#include "NppPluginIface.h"

//  <--- Boost --->
#include <boost/tuple/tuple.hpp>

namespace npp_plugin {

namespace actionindex {

boost::tuples::tuple< int, int, int, bool, bool> processSCNotification( SCNotification* scn );
int getCurrActionIndex( int pDoc );

} // End namespace: actionindex

} // End namespace: npp_plugin

#endif //  End include guard: NPP_PLUGININTERFACE_ACTIONINDEX_EXTENSION_H
