/* NppPluginIface_ExtLexer_SciCommon.h
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
 *  Notepad++ Plugin Common Include file providing all the Scintilla includes required for a
 *  plugin to implement Scintilla External Lexers within a Notepad++ plugin.
 *
 *  No additional helper functions are providing with the inclusion of this file!  Those are
 *  available using the NppPluginIface_ExtLexer file, and this file is NOT required for the
 *  NppPluginIface_ExtLexer file to be included in a plugin.
 *
 */

#ifndef NPP_PLUGININTERFACE_EXTLEXER_SCICOMMON_H
#define NPP_PLUGININTERFACE_EXTLEXER_SCICOMMON_H


#define WIN32_LEAN_AND_MEAN

///////////////////////////////////////////////////////////////////////////////////////////////
//            Common includes that each added language will need
//              for use with Notepad++ External Lexers Plugin
///////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////
//  Required includes needed for Lexer plugin definitions.

/*
 *  See the lower part of this document for includes that are not required but are included
 *  because they add functionality that benefits lexers.
 *
 */

//---------------------------------------------------------------------------------------------
//  Standard library includes, using C++ headers instead of C.

#include <cstdlib>
										/*
										 *  Standard Lib
										 *  Included here to met requirements for PropSet.h
										 *
										 */

//---------------------------------------------------------------------------------------------
//  Scintilla specific includes.

#include "Accessor.h"
										/*
										 *  Defines the interface for document buffer access.
										 *  No additional includes come from this file.
										 *
										 */


#include "PropSet.h"
										/*
										 *  Scintilla class definitions
										 *  Includes:
										 *    # "SString.h" -> String container/buffer access.
										 *
										 *  Required for Window Accessor
										 *
										 */


#include "WindowAccessor.h"				/*
										 *  Scintilla Accessor derived class
										 *  Required for External Lexer
										 *  No additional includes come from this file.
										 *
										 */


#include "KeyWords.h"
										/*
										 *  Scintilla definitions enabling LexerModule
										 *  Required for ExternalLexer
										 *  No additional includes come from this file.
										 *
										 */


#include "ExternalLexer.h"
										/*
										 *  Scintilla External Lexer classes
										 *  Required for external lexer control and access
										 *  No additional includes come from this file.
										 */ 

#include "StyleContext.h"
										/*
										 *  Scintilla Styling
										 *  Not really required but all the newer lexers use
										 *    the style context accessor interface.
										 *  No additional includes come from this file.
										 *
										 */

///////////////////////////////////////////////////////////////////////////////////////////////
//  Additional Includes

#include "CharacterSet.h"
										/*
										 *  Scintilla CharacterSet Class
										 *  Encapsulates a set of characters.
										 *  Used to test if a character is within a set.
										 *
										 */


///////////////////////////////////////////////////////////////////////////////////////////////
//  Common Namespace Aliases

//  Having these included at this level is not good but there are already lexers setup to use
//  these conventions so consider it deprecated, and instead define them in the source.

namespace pIface = npp_plugin;					//  The base plugin interface.
namespace lIface = npp_plugin::external_lexer;	//  The external lexer extension to the base interface.
namespace plugin = npp_plugin;				//  The plugin where this lexer was initialized.


#endif  //  NPPEXTLEXER_COMMON_H
