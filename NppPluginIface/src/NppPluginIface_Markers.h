/* NppPluginIface_Markers.h
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
 *  Notepad++ Plugin Interface Lib extension providing helper functions for implementing
 *  Scintilla LineMarker symbol management and margin control within a Notepad++ plugin.
 *
 */

#ifndef NPP_PLUGININTERFACE_MARKERS_EXTENSION_H
#define NPP_PLUGININTERFACE_MARKERS_EXTENSION_H

#include "NppPluginIface.h"

#include "Platform.h"

//#include "Indicator.h"

namespace npp_plugin {

//  Namespace extension for plugin symbol marker and margin management.
//
//  N++ marker allotment:
//		const int MARK_BOOKMARK = 24; 
//		const int MARK_HIDELINESBEGIN = 23; 
//		const int MARK_HIDELINESEND = 22; 
//		// 24 - 16 reserved for Notepad++ internal used 
//		// 15 - 0 are free to use for plugins 
//
//  Ref thread: https://sourceforge.net/forum/forum.php?thread_id=2984130&forum_id=482781
namespace markers {

const int NB_MAX_PLUGINMARKERS = 16;
const int SC_MARK_AVAILABLE = 28;

//  N++ Margins
enum MARGIN {
	MARGIN_NONE = -1,
	MARGIN_LINENUMBER,
	MARGIN_BOOKMARK,
	MARGIN_FOLD,
	MARGIN_RESERVED,
	MARGIN_PLUGIN,
	NB_MARGINS
};

struct NppGUI
{
	NppGUI() : _enablePluginMargin(true){
	};
	bool _enablePluginMargin;
};

struct ScintillaViewParams
{
	ScintillaViewParams() : _pluginMarginShow(false){};
	bool _pluginMarginShow;
	HWND _hView;
};

class Margin
{
	MARGIN _target;
	int _widthOrig;
	int _widthCurrent;
	int _widthSetByPlugin;
	int _maskOrig;
	int _maskCurrent;
	int _maskSetByPlugin;

public:
	bool adjustWidth( int value );
	void restoreWidth();
	void setTarget(MARGIN target)
		{ if (! (target == MARGIN_FOLD) || (target == MARGIN_RESERVED) ) _target = target; };
	bool setMask(int mask);
	void restoreMask();

	ScintillaViewParams* svp;
	NppGUI* nppGUI;
};

struct Plugin_Line_Marker {
	COLORREF back;
	COLORREF fore;
	int type;
	tstring xpm;

	bool trackUNDOREDO;
	bool active;
	bool display;

	Margin margin;

	int define( int markerNum, int markerType, COLORREF fore, COLORREF back, char* xpm, int alpha );
	int init();
	void insert(int line);
	void remove(int line);
};

enum actionType { insertAction, removeAction, startAction };

/**
 * Actions are used to store all the information required to perform one undo/redo step.
 */
class Action {
public:
	actionType at;
	int line;
	char *data;
	int lenData;

	Action();
	~Action();
	void Create(actionType at_, int position_=0, char *data_=0, int lenData_=0, bool mayCoalesce_=true);
	void Destroy();
	void Grab(Action *source);
};

class Marker_History {
	Action *actions;
	int lenActions;
	int maxAction;
	int currentAction;
	int undoSequenceDepth;
	int savePoint;

	void EnsureUndoRoom();

public:
	Marker_History();
	~Marker_History();

	void AppendAction(actionType at, int position, char *data, int length, bool &startSequence);

	void BeginUndoAction();
	void EndUndoAction();
	void DropUndoSequence();
	void DeleteUndoHistory();

	/// The save point is a marker in the undo stack where the container has stated that
	/// the buffer was saved. Undo and redo can move over the save point.
	void SetSavePoint();
	bool IsSavePoint() const;

	/// To perform an undo, StartUndo is called to retrieve the number of steps, then UndoStep is
	/// called that many times. Similarly for redo.
	bool CanUndo() const;
	int StartUndo();
	const Action &GetUndoStep() const;
	void CompletedUndoStep();
	bool CanRedo() const;
	int StartRedo();
	const Action &GetRedoStep() const;
	void CompletedRedoStep();
};

void getAvailableMarkers( int nb_markers_needed );
void _getNextMarkerType();
void _gotMarkerTypeReply(int markerNumber, int markerType, int targetView);

void setMarkerAvailable( int markerNumber );

int string2marker(tstring string);
int string2margin(tstring szMargin);

} //  End namespace: markers

}  // End Namespace: npp_plugin

#endif // End include guard:  NPP_PLUGININTERFACE_MARKERS_EXTENSION_H