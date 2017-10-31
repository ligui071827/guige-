/***************************************************************************
*
*               Copyright 2006 Mentor Graphics Corporation
*                         All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
****************************************************************************/

#include "inflexionui/engine/inc/LcAll.h"
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


/*-------------------------------------------------------------------------*//**
	Creation
*/
LcTaOwner<LcCKeyValueList> LcCKeyValueList::create(const LcTmString& lang, const LcTmString& screen)
{
	LcTaOwner<LcCKeyValueList> ref;
	ref.set(new LcCKeyValueList());
	ref->construct(lang, screen);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Construction
*/
void LcCKeyValueList::construct(const LcTmString& lang, const LcTmString& screen)
{
	m_language = lang;
	m_screenSize = screen;
}

/*-------------------------------------------------------------------------*//**
	Cleanup on destruction
*/
LcCKeyValueList::~LcCKeyValueList()
{
	m_pairs.clear();
}

/*-------------------------------------------------------------------------*//**
*/
bool LcCKeyValueList::getValue(const LcTmString& key, LcTmString& value)
{
	value = "";

	// If the map doesn't contain this key, we return an empty string.
	// When the key is found we first return the temporary value because if
	// it exists then the program is working with data that may get canceled.
	TmMPairs::iterator it = m_pairs.find(key.toLower());
	if (it != m_pairs.end())
	{
		value = it->second;
		return true;
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
	This will find the best match for a value based on the <item>.home.extra
	format. The items are specified in a key/value pair and extracted to this
	store. This routine then find the best match item for the key it is given.
	If it cannot match anything it will return "".
*/
bool LcCKeyValueList::getBestMatchValue(const LcTmString& searchKey, LcTmString& value)
{
	const LcTaString	NAME_SEP	= ".";

	LcTaString			retString	= "";
	LcTaString			searchStr	= "";
	int					dotPos		= 0;
	int					findDotPos	= 0;

	value = "";

	searchStr = searchKey;
	do
	{
		if (getValue(searchStr, retString))
		{
			value = retString;
			return true;
		}

		findDotPos = searchStr.find(NAME_SEP);
		dotPos = findDotPos;
		if (dotPos == LcTmString::npos)
			break;

		while(findDotPos != LcTmString::npos)
		{
			dotPos = findDotPos;
			findDotPos = searchStr.find(NAME_SEP, dotPos + 1);
		}
		searchStr = searchStr.subString(0, dotPos);

	} while (true);

	return false;
}


/*-------------------------------------------------------------------------*//**
	Wrapper for initFromXml() to get key-value pairs from file.
	Allows us to load a list of key-value pairs from an XML file.  If a
	non-zero tagFilter is specified, only pairs with the specified tag
	will be loaded.  This allows us to load from multiple files, each
	successive one taking precedence over the others.  If tagFilter
	is non-zero, any pairs that already existed and are overwritten
	as a result of this load will inherit the new tagFilter value.
*/
bool LcCKeyValueList::loadFromXml(
	const LcTmString& path,
	const LcTmString& filename)
{
	// Load info from XML
	LcTaString err;
	// Set the directory slash separators to the non default if required.
	LcTaString localPath = path + filename;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif
	LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(localPath, err);

	// Abort if unable to load
	if (!root)
		return false;

	// Call initFromXml() to get the pairs into our list
	initFromXml(root.ptr());

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
void LcCKeyValueList::configureScreenFromXml(LcCXmlElem* screen)
{
	if (screen == NULL)
		return;

	// Get pointer to the first item
	LcCXmlElem* item = screen->getFirstChild();
	for (; item != NULL; item = item->getNext())
	{
		// Check item is of the correct type
		if (item->getName().compareNoCase("token") != 0)
			continue;

		// Get key and value
		LcTaString	key   = item->getAttr(NDHS_TP_XML_KEY);
		LcTaString	value = item->getAttr(NDHS_TP_XML_VALUE);

		// We're only interested in pairs with a valid key
		if (key.length() != 0)
			setValue(key, value);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcCKeyValueList::configureLanguageFromXml(LcCXmlElem* lang)
{
	LcCXmlElem* allScreens = NULL;
	LcCXmlElem* userScreen = NULL;

	if (lang)
	{
		LcCXmlElem* screen = lang->getFirstChild();
		for (; screen != NULL; screen = screen->getNext())
		{
			// Check item is of the correct type
			if (screen->getName().compareNoCase("screen") != 0)
				continue;

			LcTaString screenSize = screen->getAttr(NDHS_TP_XML_ID);
			if (screenSize.compareNoCase("all") == 0)
			{
				allScreens = screen;
			}
			else if (screenSize.compareNoCase(m_screenSize) == 0)
			{
				userScreen = screen;
			}
		}
	}

	configureScreenFromXml(allScreens);
	configureScreenFromXml(userScreen);
}

/*-------------------------------------------------------------------------*//**
	Initialize key-value pairs from the supplied XML node.
	Allows us to load a list of key-value pairs from XML.
*/
void LcCKeyValueList::initFromXml(LcCXmlElem* root)
{
	if (root == NULL)
		return;

	LcCXmlElem* allLangs = NULL;
	LcCXmlElem* userLang = NULL;

	// Find the all languages and user language sections
	LcCXmlElem* lang = root->getFirstChild();
	for (; lang != NULL; lang = lang->getNext())
	{
		// Check item is of the correct type
		if (lang->getName().compareNoCase("language") != 0)
			continue;

		LcTaString language = lang->getAttr(NDHS_TP_XML_ID);
		if (language.compareNoCase("all") == 0)
		{
			allLangs = lang;
		}
		else if (language.compareNoCase(m_language) == 0)
		{
			userLang = lang;
		}
	}

	configureLanguageFromXml(allLangs);
	configureLanguageFromXml(userLang);
}

#if defined(LC_USE_XML_SAVE)
/*-------------------------------------------------------------------------*//**
	Allows us to save list of key-value pairs to an XML file.  If a
	non-zero tagFilter is specified, only pairs with the specified tag
	will be saved.
*/
bool LcCKeyValueList::saveToXml(
	const LcTmString& path,
	const LcTmString& filename)
{
	// First build up temporary XML
	LcTaOwner<LcCXmlElem> root = LcCXmlElem::create("uiTokens");

	// Setup the language element
	LcCXmlElem* language = root->createElem("language");
	language->setAttr(NDHS_TP_XML_ID, "all");

	// Setup the screen size element
	LcCXmlElem* screenSize = language->createElem("screen");
	screenSize->setAttr(NDHS_TP_XML_ID, "all");

	// Add each pair in turn
	TmMPairs::iterator it = m_pairs.begin();
	for (; it != m_pairs.end(); ++it)
	{
		// Create an element for the pair
		LcCXmlElem* item = screenSize->createElem("token");
		item->setAttr(NDHS_TP_XML_KEY, it->first);
		item->setAttr(NDHS_TP_XML_VALUE, it->second);
	}

	// Save info to XML
	LcTaString err;
	bool saved = root->save(path + filename, err);

	// Return whether or not we are successful
	return saved;
}
#endif
