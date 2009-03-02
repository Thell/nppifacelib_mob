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
 *  Notepad++ Plugin providing access to Scintilla's marker container to determine if a marker
 *  is available for use by a plugin.  It checks both Notepad++ views for markers are are still
 *  defined with Scintilla's default LineMarker settings.
 *
 */

#include "NppPlugin_SciMarkerSymbol.h"

namespace npp_plugin_scimarkersymbol {

//  Returns the marker symbol id for markerNum in the target view.
int MARKERSYMBOL(int markerNumber, unsigned int targetView)
{
	if ( (markerNumber > npp_plugin::markers::NB_MAX_PLUGINMARKERS) || (markerNumber < 0) ||
		(targetView > SUB_VIEW) ) return -1;

	//  Make the protected ViewStyle public to allow access to the marker array.
	class MarkerAccessor : public Editor {
	public:
		using Editor::vs;
	};

	MarkerAccessor* ma = reinterpret_cast<MarkerAccessor *>(::GetWindowLongPtr( npp_plugin::hViewByInt( targetView ), 0));

	int markType = -1;
	LineMarker* lm = &ma->vs.markers[markerNumber];	//  Mark to test.
	LineMarker em;							//  Empty mark to test against.

	markType = lm->markType;

	//  Test if the marker is still at default values.
	if ( ( em.markType == lm->markType ) &&
			( em.alpha == lm->alpha ) &&
			( em.fore.allocated.AsLong() == lm->fore.allocated.AsLong() ) &&
			( em.back.allocated.AsLong() == lm->back.allocated.AsLong() ) ) {
		markType = npp_plugin::markers::SC_MARK_AVAILABLE;
	}

	return ( markType );
}

} //  End namespace: npp_plugin_scimarkersymbol