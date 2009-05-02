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
#include <map>
#include <unordered_map>

//  N++ Change Marker Plugin Specific
namespace npp_plugin_changemarker {

using namespace npp_plugin;
using namespace npp_plugin::markers;
using namespace npp_plugin::actionhistory;

//  Menu Command IDs
const int CMD_OFFSET = 3; // Offset between MENU_COMMAND and MARGIN for conversion between them.
enum MENU_COMMANDS {
	CMD_JUMPCHANGEPREV = 1,
	CMD_JUMPCHANGENEXT,
	CMD_JUMPLINEUP,
	CMD_JUMPLINEDOWN,
	CMD_JUMPINCLUDESAVED,
	CMD_LINENUMBER,
	CMD_CHANGEMARK,
	CMD_HIGHLIGHT,
	CMD_HIDEMARKS,
	CMD_DISABLEDOC,
	CMD_DISABLEPLUGIN,
	NB_MENU_COMMANDS
};

//  History Actions
enum CM_ACTION {
	CM_MARKERADD = 0x01,			// Used to match up with SCI_MARKERADD
	CM_MARKERFORWARD = 0x02,		// Marker already exists on the line and is reused in a new action.
	CM_MARKERDELETE = 0x04,			// Used to match up with SCI_MARKERDELETE
	CM_MARKERMOVE = 0x08,			// Used to move markers during undo/redo after line deletions and insertions.
	CM_LINEINSERT = 0x10,			// Scintilla inserted a line.
	CM_LINEDELETE = 0x20,			// Scintilla deleted a line. 
	CM_SAVEPOINT = 0x100,			// Scintilla Document savepoint.
	CM_MARKERACTION = 0x1000,
	CM_LINECOUNTCHANGE = 0x2000,
};


//  Markers
const int CM_BASEID = 20;
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
	void setDisplay(bool show);
};

//  <---  Changed Lines Mapping --->
typedef std::multimap<int, int> handle_map;
typedef handle_map::iterator hm_pos;
typedef std::pair<handle_map::iterator, handle_map::iterator> hm_range;
typedef std::map<int, handle_map> line_map;
typedef line_map::iterator lm_pos;

class CM_LineMap {
	int currMaxMarkerHandle;
public:
	line_map _lines;

	void insertLines( int startLine, int nb_lines );
	void deleteLines( int startLine, int nb_lines );
	void addHandleToLine( int line, int marker, int handle );
	void deleteHandleFromLine( int line, int marker, int handle);
	void modifyHandleOnLine( int line, int marker, int oldHandle, int newHandle );
	int getHandleFromLine( int line, int marker );
	int getLineFromHandle( int marker, int handle );

	//  Returns the most recent marker handle assigned by Scintilla.
	int getCurrMaxMarkerHandle(){ return ( currMaxMarkerHandle ); };

	CM_LineMap():currMaxMarkerHandle(0){};
};


//  <--- Document State Tracking --->.
class ChangedDocument {
	friend class CM_LineMap;

	int addMarker( int line, int marker );
	int deleteMarker( int line, int marker, int handle = NULL );
	int replaceMarker( int line, int oldHandle, int newMarkerID );
	void modifyMarkerHandle( int oldHandle, int newHandle );
	bool doUndo( ActionHistory* thisAction );
	bool doRedo( ActionHistory* thisAction );

public:
	CM_LineMap lm;

	int _pDoc;						//  Scintilla Docmuent ID.
	int _tmpActionHandle;			//  Deleted Scintilla handles get assigned a temp handle.
	int _prevInsertLine;			//  Line where the last 'new action' took place.
	int _savePointIndex;			//  History index when document was last saved.
	HWND hView;						//  Target view to send messages to.
	int targetIndex;
	int currChangePositionTarget;	//  Marker handle of the marker currently jumped to via menu.

	DocumentActionHistory hist;
	void processInsert(int startLine, int endLine );
	void processDelete(int startLine, int endLine );
	void processUndo();
	void processRedo();
	void processFileSave();

	int getNextChangeLine(bool direction);

	ChangedDocument(int pDoc):_pDoc(pDoc), _tmpActionHandle(-1),
		_prevInsertLine(-1), _savePointIndex(0), hist(_pDoc), targetIndex(0){};
};

//  Change Marker Functions
void initPlugin();
bool generateDefaultConfigXml();
void modificationHandler( SCNotification *scn );
void bufferActivatedHandler( SCNotification *scn );
void fileSaveHandler();
void fileBeforeCloseHandler( SCNotification *scn );
void wordStylesUpdatedHandler();
void jumpChangedLines( bool direction );
void setMenuState( bool enabled = true );

//  Menu functions items
void jumpChangePrev();
void jumpChangeNext();
void jumpLineUp();
void jumpLineDown();
void jumpIncludeSaved();
void displayWithLineNumbers();
void displayWithChangeMarks();
void displayAsHighlight();
void disableDoc();
void disablePlugin();

}  //  End namespace:  npp_plugin_changemarker
