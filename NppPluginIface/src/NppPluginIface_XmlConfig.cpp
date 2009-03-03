/* NppPluginIface_XmlConfig.cpp
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

#include "NppPluginIface_XmlConfig.h"

namespace npp_plugin {

//  Namespace extension for plugin XMLConfig data.
namespace xmlconfig {

//  Un-named namespace for GUIConfig_set variables and functions.
namespace {

using boost::multi_index::multi_index_container;
using namespace boost::multi_index;

//  Structure of GUIConfig nodes within the XML config file.
struct GUIConfig
{
	int         id;
	tstring attribute;
	tstring value;

	GUIConfig(int id, tstring attribute, tstring value)
		:id(id),attribute(attribute),value(value){}

	bool operator<(const GUIConfig& gc)const{return id<gc.id;}
};

/*  Tags for accessing the corresponsing indices of GUIConfig. */
struct id{};
struct attribute{};
struct value{};
struct comp{};

//  multi_index container of GUIConfigs for this plugin.
typedef multi_index_container<
	GUIConfig,
	indexed_by<
		ordered_unique< tag<comp>, composite_key<GUIConfig,
			member<GUIConfig, int,&GUIConfig::id>,
			member<GUIConfig, tstring,&GUIConfig::attribute> > >,
		ordered_non_unique<	tag<id>, member<GUIConfig,int,&GUIConfig::id> >,
		ordered_non_unique< tag<attribute>, member<GUIConfig,tstring,&GUIConfig::attribute> >,
		ordered_non_unique< tag<value>, member<GUIConfig,tstring,&GUIConfig::value> >
	>
> GUIConfig_set;


//  Pointer to this plugin's xml configuration file.
TiXmlDocument *_pXmlPluginConfigDoc;

//  Container for the GUI_Config data
GUIConfig_set gcs;
bool _GUIConfig_set_initialized = false;

//  Initialize pointer to the TiXmlDocument Plugin Configuration Document for this plugin.
//  Checks for existence in user config directory then in the N++ plugins config directory.
//  Returns true if successful.
bool initXmlPluginConfig()
{
	bool retVal = false;
	if ( _pXmlPluginConfigDoc ) return true;

	tstring baseModuleName = npp_plugin::getModuleBaseName()->c_str();

	//  First try the application data folder.
	TCHAR xmlPath[MAX_PATH];
	::SendMessage( npp_plugin::hNpp(), NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)xmlPath );
	PathAppend( xmlPath, baseModuleName.c_str() );
	PathAddExtension(xmlPath, TEXT(".xml"));

	if (!PathFileExists(xmlPath))
	{
		lstrcpyn( xmlPath, TEXT("\0"), MAX_PATH );
		::SendMessage( npp_plugin::hNpp(), NPPM_GETNPPDIRECTORY, MAX_PATH, (LPARAM)xmlPath );
		PathAppend( xmlPath, TEXT("plugins\\Config") );
		PathAppend( xmlPath, baseModuleName.c_str() );
		PathAddExtension(xmlPath, TEXT(".xml"));

		if (!PathFileExists(xmlPath))
		{
			tstring msgText;
			msgText.assign( TEXT("The configuration file ") );
			msgText.append( baseModuleName );
			msgText.append( TEXT(".xml for the '") );
			msgText.append( npp_plugin::getName() );
			msgText.append( TEXT("' plugin was not found!") );

			::MessageBox( npp_plugin::hNpp(), msgText.c_str() , TEXT("Plugin File Load Error"), MB_ICONERROR );
			return false;
		}
	}

	_pXmlPluginConfigDoc = new TiXmlDocument(xmlPath);

	if ( !_pXmlPluginConfigDoc->LoadFile() )
	{
		delete _pXmlPluginConfigDoc;
		_pXmlPluginConfigDoc = NULL;
			
		tstring msgText;
		msgText.assign( TEXT("The configuration file for ") );
		msgText.append( npp_plugin::getName() );
		msgText.append( TEXT(" failed to load!") );

		::MessageBox( npp_plugin::hNpp(), msgText.c_str() , TEXT("Plugin File Load Error"), MB_ICONERROR );
		retVal = false;
	}

	if ( _pXmlPluginConfigDoc ) retVal = true;

	return ( retVal );
}

//  Retrieves the GUIConfig elements from the xml config doc and places them in the GUIConfig_set
bool getGUIConfigFromXmlTree()
{
	if (! initXmlPluginConfig() ) return false;
	if ( _GUIConfig_set_initialized ) return true;

	TiXmlNode *root = _pXmlPluginConfigDoc->FirstChild( TEXT("NotepadPlus") );
	if (!root) return false;

	TiXmlNode *GUIRoot = root->FirstChildElement( TEXT("GUIConfigs") );
	if (!GUIRoot) return false;

	int id = 0;
	for (TiXmlNode *childNode = GUIRoot->FirstChildElement(TEXT("GUIConfig"));
		childNode ;
		childNode = childNode->NextSibling(TEXT("GUIConfig")) )
	{
		TiXmlElement *element = childNode->ToElement();

		TiXmlAttribute *attrib = element->FirstAttribute();
		while (attrib)	{
			tstring tmpAttrib(attrib->Name());
			tstring tmpValue(attrib->Value());

			gcs.insert( GUIConfig( id, tmpAttrib, tmpValue ) );
			attrib=attrib->Next();
		}
		id++;
	}

	_GUIConfig_set_initialized = true;
	return true;
}

//  Writes the targeted GUIConfig attribute to file.
bool writeXmlPluginConfig(int nodeID, tstring attrib_, tstring value_ )
{
	bool retVal = false;

	if ( initXmlPluginConfig() ) {
		TiXmlNode *root = _pXmlPluginConfigDoc->FirstChild( TEXT("NotepadPlus") );
		if (! root ) return retVal;

		TiXmlNode *GUIRoot = root->FirstChildElement( TEXT("GUIConfigs") );
		if (! GUIRoot ) return retVal;

		int id = 0;
		for (TiXmlNode *childNode = GUIRoot->FirstChildElement(TEXT("GUIConfig"));
			childNode ;
			childNode = childNode->NextSibling(TEXT("GUIConfig")) )
		{
			if ( id == nodeID ) {
				if ( retVal == true ) break;

				TiXmlElement *element = childNode->ToElement();
				TiXmlAttribute *attrib = element->FirstAttribute();
				while ( attrib )	{
					if ( attrib_.compare( attrib->Name() ) == 0  ) {
						attrib->SetValue( value_.c_str() );
						retVal= _pXmlPluginConfigDoc->SaveFile();
						if (! retVal ) {
							//  Error writing to the file; need a handler for this.
						}
						break;
					}
					attrib=attrib->Next();
				}
			}
			id++;
		}
	}

	return ( retVal );

}

}  // End Namedspace:  Un-named

//  Returns a tstring reference to the value of a named attribute from the GUIConfig element
//  with the matching 'name' attribute label.
//
//  example:  getGUIConfigValue( TEXT("MAIN_VIEW"), TEXT("changeMark") );
tstring getGUIConfigValue( tstring name, tstring attrib )
{
	tstring retVal;

	if (! getGUIConfigFromXmlTree() ) return retVal;

	typedef GUIConfig_set::index<value>::type gcs_by_value;
	gcs_by_value& value_index=gcs.get<value>();

	//  Multiple nodes may exist with the same 'name' field, but a 'name' node 'attribute' label
	//  should be unique so we return the value of only the first one encountered.
	for ( gcs_by_value::iterator itNode = value_index.find(name), itNode_end(value_index.end());
		itNode != itNode_end;
		++itNode )
	{
		GUIConfig_set::iterator itEntry=gcs.find(boost::make_tuple(itNode->id,attrib));
		if ( itEntry != gcs.end() ) {
			retVal = itEntry->value;
			break;
		}
	}

	return ( retVal );
}

//  Writes a GUIConfig value to the registered plugins Xml Plugin Config file.
//
//  ex: writeGUIConfigValue( TEXT("MAIN_VIEW"), TEXT("changeMark"), TEXT("hide") );
bool setGUIConfigValue( tstring name, tstring attrib, tstring value_ )
{
	bool retVal = false;

	typedef GUIConfig_set::index<value>::type gcs_by_value;
	gcs_by_value& value_index=gcs.get<value>();

	//  Multiple nodes may exist with the same 'name' field, but a 'name' node 'attribute' label
	//  should be unique so we target the value of only the first one encountered.
	for ( gcs_by_value::iterator itNode = value_index.find(name), itNode_end(value_index.end());
		itNode != itNode_end;
		++itNode )
	{
		GUIConfig_set::iterator itEntry=gcs.find(boost::make_tuple(itNode->id,attrib));
		if ( itEntry != gcs.end() ) {
			//  Change the internal representation of the attribute value.
			if ( writeXmlPluginConfig( itEntry->id, attrib, value_ ) ) {
				GUIConfig targetAttrib = *itEntry;
				targetAttrib.value = value_;
				gcs_by_value::iterator itTarget=gcs.project<value>(itEntry);
				value_index.replace(itTarget, targetAttrib);
				retVal = true;
				break;
			}
			else {
				retVal = false;
			}
		}
	}

	return ( retVal );
}


}  // End Namespace: xml_config

}  // End Namespace: npp_plugin