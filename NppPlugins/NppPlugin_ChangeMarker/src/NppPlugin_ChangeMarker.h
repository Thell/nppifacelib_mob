/* NppPlugin_ChangeMarker.h
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

//  Change Marker Plugin for use with Notepad++.

#include "NppPlugin.h"


//  N++ Change Marker Plugin Specific
namespace npp_plugin_changemarker {

using namespace npp_plugin;
using namespace npp_plugin::markers;
using namespace npp_plugin::actionhistory;

//  Menu Command IDs
const int CMD_OFFSET = 1; // Offset between MENU_COMMAND and MARGIN for conversion between them.
enum MENU_COMMANDS {
	CMD_JUMPPREV = 1,
	CMD_JUMPNEXT,
	CMD_LINENUMBER,
	CMD_BOOKMARK,
	CMD_PLUGIN,
	CMD_HIGHLIGHT,	
	CMD_DISABLE,
	NB_MENU_COMMANDS
};


//  Markers
enum CHANGE_MARK {
	CM_SAVED,
	CM_NOTSAVED,
	NB_CHANGEMARKERS,
};

//  Change markers class
class Change_Mark: public Plugin_Line_Marker {
	bool _tempMarkerOverride;
	int _overRiddenMarkerType;
public:
	tstring markName;
	tstring styleName;
	int _origTargetMargin;	//  Placeholder for target margin until marker id is aquired.
	void setTargetMarginMenuItem( MARGIN target );
	void markerOverride( int markerType );
	void resetOverride();

	//  Marker History
	int prevActionLine;
	int prevActionHandle;
	int prevActionHandleRefIndex;
	int tmpActionHandle;
	PluginActionHistory history;

	Change_Mark()
		:prevActionHandle(0), tmpActionHandle(0){};
};


//  Change Marker Functions
void initPlugin();
void initMarker(int* markerArray );
void modificationHandler(SCNotification *scn);
void fileSaveHandler(SCNotification *scn);
void wordStylesUpdatedHandler();

//  Menu functions items
void jumpPrevChange();
void jumpNextChange();
void displayWithLineNumbers();
void displayWithBookMarks();
void displayInPluginMargin();
void displayAsHighlight();
void disable();

}  //  End namespace:  npp_plugin_changemarker
