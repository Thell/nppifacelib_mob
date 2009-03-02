/* NppPluginIface_SciMarkerSymbol.h
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
 *  Notepad++ Plugin providing access to Scintilla's marker container to and returning the
 *  currently assigned marker symbol ID.  If the marker is set to the same as Scintilla's
 *  default LineMarker and the marker is in the plugin range SC_MARK_AVAILABLE is returned.
 *
 *  SCI_MARKERSYMBOL is defined in the npp_plugin::msgs namespace.
 *  SC_MARK_AVAILABLE is defined in the npp_plugin::markers namespace.
 *
 */

#ifndef NPP_PLUGIN_SCIMARKERSYMBOL_H
#define NPP_PLUGIN_SCIMARKERSYMBOL_H

#include "NppPlugin.h"

#include "Platform.h"
#include "UniConversion.h"
#include "Indicator.h"
#include "XPM.h"
#include "LineMarker.h"
#include "Style.h"
#include "ViewStyle.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "RunStyles.h"
#include "Decoration.h"
#include "CharClassify.h"
#include "CellBuffer.h"
#include "Document.h"
#include "PositionCache.h"
#include "KeyMap.h"
#include "ContractionState.h"

//  Easier to just add this here than include other files.
struct RangeToFormat {
	HDC hdc;
	HDC hdcTarget;
	RECT rc;
	RECT rcPage;
	CharacterRange chrg;
};

#include "Editor.h"

//  N++ marker allotment:
//		const int MARK_BOOKMARK = 24; 
//		const int MARK_HIDELINESBEGIN = 23; 
//		const int MARK_HIDELINESEND = 22; 
//		// 24 - 16 reserved for Notepad++ internal used 
//		// 15 - 0 are free to use for plugins 
//
//  Ref thread: https://sourceforge.net/forum/forum.php?thread_id=2984130&forum_id=482781

namespace npp_plugin_scimarkersymbol {

const int NB_MAX_PLUGINMARKERS = 16;

int MARKERSYMBOL(int markerNumber, unsigned int targetView);

} //  End namespace: npp_plugin_scimarkersymbol

#endif // End include guard:  NPP_PLUGIN_SCIMARKERSYMBOL_H