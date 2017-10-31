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
*/
LcTaOwner<NdhsCMenu> NdhsCMenu::create(NdhsCMenuCon* con, NdhsCPlugin* plugin, const LcTmString& language, const LcTmString& screenSize)
{
	LcTaOwner<NdhsCMenu> ref;
	ref.set(new NdhsCMenu(con, plugin, language, screenSize));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenu::NdhsCMenu(NdhsCMenuCon* con, NdhsCPlugin* plugin, const LcTmString& language, const LcTmString& screenSize)
{
	m_menuCon = con;
	m_language = language;
	m_screenSize = screenSize;
	m_duplicatedMenu = false;
	m_menuOriginalLength = 0;
	m_plugin = plugin;
	m_firstActiveItemPluginIndex = -1;
	m_firstActiveItem = -1;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenu::~NdhsCMenu()
{
	if (m_menuFileXml)
	{
		m_menuFileXml.destroy();
		m_menuFileRoot = NULL;
	}

	cleanupMenuData();
	m_menuPlugin.destroy();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenu::cleanupMenuData()
{
	// Unload any menus already loaded
	m_menuItemObjects.clear();

	m_menuSortClass="";
	m_menuItems.clear();
	m_elementCachedClasses.clear();
	m_staticMenuFields.clear();

	m_firstActiveItemPluginIndex = -1;
	m_firstActiveItem = -1;

	// NOTE: Don't destroy the plugin in here or any members set
	// in the openMenu functions
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool NdhsCMenu::populateMenuWithAdditions(const LcTmString& package,
															const LcTmString& menuFile)
{
	LcTaString menu;
	LcTaString filePath;
	typedef LcTaMap<LcTaString, LcTaString> tStringMap;
	tStringMap packages;
	tStringMap::iterator package_iterator;

	for (int i = 0; i < menuFile.length(); i++)
	{
		// Replace the '.'s with '-'s
		if (menuFile[i] == '.')
			menu += '-';
		else
			menu += (char)menuFile[i];
	}

	// Build the folder name wild-card <menu's package name>_<menu name>_*
	LcTaString searchTerm = package.subString(0, package.length() - 1) + "_" + menu + "_";

	LcTaString searchFile = getMenuCon()->getThemePath() + searchTerm + "*";

	// Loop through each of the drives in IFX_PACKAGE_DRIVES looking for packages.
	LcTaString driveList = IFX_PACKAGE_DRIVES;
	while (driveList.length())
	{
		LcTaString currentDrive = driveList.subString(0, 1) + ":" + NDHS_DIR_SEP;
#if defined (IFX_USE_ROM_FILES)
		if (currentDrive[0] == IFX_ROM_FILE_DRIVE_LETTER)
		{
			// Check ROM Files
			LcTaOwner<LcCRomFileSearch> romSearch = LcCRomFileSearch::create();

			if (romSearch.ptr() != NULL)
			{
				// ROM Search does NOT want a drive letter prefix
				if (romSearch->fileFindFirst(searchFile, LC_FILE_SEARCH_TYPE_DIR, filePath) == true)
				{
					do
					{
						packages[filePath] = currentDrive + getMenuCon()->getThemePath() + filePath + NDHS_DIR_SEP;
					}
					while (romSearch->fileFindNext(filePath) == true);

					romSearch->fileFindClose();
				}

				romSearch.destroy();
			}
		}
		else
#endif

#if defined(IFX_USE_PLATFORM_FILES)
		if (currentDrive[0] >= 'A' && currentDrive[0] <= 'Z')
		{
			// Check the native file system
			LcTaOwner<LcCPlatformFileSearch> platformSearch = LcCPlatformFileSearch::create();

			if (platformSearch.ptr() != NULL)
			{
				if (platformSearch->fileFindFirst(currentDrive + searchFile, LC_FILE_SEARCH_TYPE_DIR, filePath) == true)
				{
					do
					{
						packages[filePath] = currentDrive + getMenuCon()->getThemePath() + filePath + NDHS_DIR_SEP;
					}
					while (platformSearch->fileFindNext(filePath) == true);

					platformSearch->fileFindClose();
				}

				platformSearch.destroy();
			}
		}
		else
#endif
		{
		}

		// Remove all up to and including the next ','
		int pos = driveList.find(",");
		if (pos > 0)
		{
			driveList = driveList.tail(pos + 1);
		}
		else
		{
			driveList = driveList.tail(1);
		}

	} //while (driveList.length())

	//now insert packages that were found
	if (packages.empty() == false)
	{
		for (package_iterator = packages.begin(); package_iterator != packages.end(); package_iterator++)
		{
			addMenuAdditions(package_iterator->second);
		}
	}
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenu::loadXmlMenu()
{
	bool retVal = false;
	LcTaString err;

	// Set the directory slash separators to the non default if required.
	LcTaString localPath = m_menuFile;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif

	// Load the file.
	LcTaOwner<LcCXmlElem> tempXmlElem = LcCXmlElem::load(localPath, err);
	m_menuFileXml = tempXmlElem;
	if (m_menuFileXml)
	{
		m_menuFileRoot = m_menuFileXml.ptr();
		retVal = true;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenu::openMenuFromXml(const LcTmString& package, const LcTmString& menuFile)
{

	bool retVal = false;

	// make sure there is no previous menu data
	cleanupMenuData();
	m_menuPlugin.destroy();

	// Set the menu source
	m_menuSource = EXml;
	m_package = package;
	m_menuFile = menuFile;

	if (loadXmlMenu())
	{
		LcCXmlElem* menu = m_menuFileXml->find(NDHS_TP_XML_MENU);
		if (menu)
		{
			// Get menu name from root node
			m_menuName = menu->getAttr(NDHS_TP_XML_NAME);

			if (m_menuName.length() > 0)
				retVal = true;
		}

		// If the transaction has failed then throw away the xml.
		if (!retVal)
		{
			m_menuFileXml.destroy();
		}
	}


	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenu::openMenuFromPlugin(const LcTmString& package, const LcTmString& link)
{
	LC_ASSERT(m_plugin);

	// make sure there is no previous menu data
	cleanupMenuData();
	m_menuPlugin.destroy();

	// Set the menu source
	m_menuSource = EPlugin;

	m_package = package;

	m_menuLink=link;

	// Get a plugin menu session
	LcTaOwner<NdhsCPlugin::NdhsCPluginMenu> newMenuPlugin = m_plugin->openMenu(link);
	m_menuPlugin = newMenuPlugin;
	if (!m_menuPlugin)
		return false;

	// Get menu name from the plugin
	m_menuName = m_menuPlugin->getMenuName();
	if (m_menuName.length() == 0)
		return false;

	return true;
}

/*-------------------------------------------------------------------------*//**
	Call this to populate the menu the first time
*/
LC_EXPORT bool NdhsCMenu::populateMenu()
{
	return reloadMenu();
}

/*-------------------------------------------------------------------------*//**
	Comparison function called by sort
*/
bool itemCompare(NdhsCMenuItem* a, NdhsCMenuItem* b)
{
	NdhsCMenu*	menu = a->getOwner();

	LC_ASSERT(menu);
	LC_ASSERT(menu == b->getOwner());

	LcTaString sortClass = menu->getSortClass();
	int sortType = menu->getSortType();
	int sortDirection = menu->getSortDirection();

	LcTaString lhs, rhs;

	a->getCachedFieldData(sortClass, lhs);
	b->getCachedFieldData(sortClass, rhs);

	int returnValue;

	// If any item has no element for the given class, we force it at
	// the end of the list regardless of sort direction
	if (lhs.length() == 0)
	{
		return false;
	}
	else if (rhs.length() == 0)
	{
		return true;
	}
	else
	{
		switch(sortType)
		{
			case NdhsCMenu::ECaseInsensitive:
			{
				returnValue = lhs.compareNoCase(rhs);
			}
			break;

			case NdhsCMenu::ECaseSensitive:
			default:
			{
				returnValue = lhs.compare(rhs);
			}
			break;
		}
	}

	if (sortDirection == NdhsCMenu::EAscending)
	{
		if (returnValue >= 0)
			return false;
	}
	else
	{
		if (returnValue <= 0)
			return false;
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
	If the menu data has changed, call this to reload.  It will use the
	current configuration.
*/
LC_EXPORT bool NdhsCMenu::reloadMenu()
{
	bool result = false;

	// As menu is being reloaded so have to make sure that the item duplication works for the new menu items.
	m_duplicatedMenu = false;

	// Do another cleanupMenuData, just in case this isn't the first time this function is called
	cleanupMenuData();

	switch (m_menuSource)
	{
		case ENodeXml:
		{
			populateMenuFromPageFile(m_package,m_menuFile,m_menuName);
		}
		break;
		case EXml:
		{
			result = populateMenuFromXml();
		}
		break;

		case EPlugin:
		{
			result = populateMenuFromPlugin();
		}
		break;

		default:
			break;
	}

	if (result && ((m_menuSource == EXml || m_menuSource== ENodeXml) || !isLodRequired()))
	{
		//
		// Now add the menu additions
		//
		populateMenuWithAdditions(m_package, m_menuName);

		//
		// Now sort the menu items
		//
		if (m_menuSortClass.length() > 0 && (m_menuSource == EXml || m_menuSource== ENodeXml))
		{
			IFX_ShellSort(m_menuItems.begin(), m_menuItems.end(), itemCompare);
		}

		// We could clean up the element data for plugin menu items
		// because it was only loaded for the sort, which is now complete.
		// If we did clean up the data, it can only be done for non
		// menu-addition items, so it will slow things down.  For now,
		// the data is just left because it should be minimal.
	}

	// Store length before any chance of double menu items being called
	m_menuOriginalLength = (int)m_menuItems.size();

	return result;
}

/*-------------------------------------------------------------------------*//**
	When populating from Xml, we just load all element class data that
	is available, and not just the classes defined by the template
*/
bool NdhsCMenu::populateMenuFromXml()
{
	// If the menu file has not already been loaded, load it again.
	if (!m_menuFileRoot)
	{
		if (!loadXmlMenu())
		{
			return false;
		}
	}


	// First get the global menu data
	LcCXmlElem* menuData = m_menuFileRoot->find(NDHS_TP_XML_MENU);
	if (menuData)
	{
		m_menuSortClass = menuData->getAttr(NDHS_TP_XML_SORT_FIELD);

		LcTaString sortType = menuData->getAttr(NDHS_TP_XML_SORT_TYPE, NDHS_TP_XML_VAL_CASE_SENSITIVE);

		if (sortType.compareNoCase(NDHS_TP_XML_VAL_CASE_INSENSITIVE) == 0)
		{
			m_menuSortType = ECaseInsensitive;
		}
		else
		{
			// Default to case sensitive
			m_menuSortType = ECaseSensitive;
		}

		LcTaString sortDirection = menuData->getAttr(NDHS_TP_XML_SORT_DIRECTION);

		m_menuSortDirection = EAscending;

		if (sortDirection.compareNoCase(NDHS_TP_XML_VAL_DESCENDING) == 0)
		{
			m_menuSortDirection = EDescending;
		}

		m_lodRequired = LcCXmlAttr::strToBool(menuData->getAttr(NDHS_TP_XML_LOAD_ON_DEMAND, "false"));

		// If we load OK, get the items and menu data now
		LcCXmlElem* data = menuData->getFirstChild();
		for (; data != NULL; data = data->getNext())
		{
			if (0 == data->getName().compareNoCase(NDHS_TP_XML_DATA))
			{
				// Extract the class name.
				LcTaString elementClass = data->getAttr(NDHS_TP_XML_FIELD, "");
				if (elementClass.length() > 0)
				{
					// Extract the class and type value.
					LcTaString value = data->getAttr(NDHS_TP_XML_VALUE, "");

					LcTaString dataType = data->getAttr(NDHS_TP_XML_VALUE_TYPE, "string");

					LcTaOwner<NdhsCField> field = createField(elementClass, -1, NULL, IFXI_FIELD_SCOPE_MENU, dataType, value);

					if (field)
					{
						addField(elementClass, field);
					}

					// cache the field data
					addElementCacheClass(elementClass, value);
				}
			}
		}
	}



	// Get the items now
	LcCXmlElem* item = m_menuFileRoot->getFirstChild();
	for (; item != NULL; item = item->getNext())
	{
		if (0 == item->getName().compareNoCase(NDHS_TP_XML_ITEM))
		{
			addXmlMenuItem(item,NULL,m_menuCon->getPageManager()->getTokenStack());
		}
	}


	// Throw away the XML
	m_menuFileXml.destroy();
	m_menuFileRoot = NULL;

	return true;
}

/*-------------------------------------------------------------------------*//**
	Creates replicated sets of menu items pointers, so increasing menu length
	by specified factor
*/
void NdhsCMenu::replicateMenuItems(int factor)
{
	if (!m_duplicatedMenu)
	{
		int menuLen = (int)m_menuItems.size();
		int count = 0;

		// Update menu length in case it's changed previously
		m_menuOriginalLength = menuLen;

		for (count = 0; count < menuLen * (factor - 1); count++)
		{
			NdhsCMenuItem* item = m_menuItems.at(count % menuLen);

			// Re-add menu pointer
			m_menuItems.push_back(item);
		}

		m_duplicatedMenu = true;
	}
}

/*-------------------------------------------------------------------------*//**
	Adds a menu item from xml
*/
NdhsCMenuItem* NdhsCMenu::addXmlMenuItem(LcCXmlElem* item, LcCTokenReplacer* tokenReplacer,NdhsCTokenStack *tokenStack)
{
	LcTaOwner<NdhsCMenuItem> miRef = NdhsCMenuItem::create(this);

	// Add the header class data to the menu item to fit
	// in with the new plugin way of doing things
	miRef->addElementCacheClasses(m_elementCachedClasses);
	miRef->setIndex(m_menuItems.size());

	// Create the reserved types first.
	// Get properties
	LcTaString linkAttr		= item->getAttr(NDHS_TP_XML_LINK, "");

	// NOTE: Do not token replace link for
	// additions with the tokens in the package.xml.  This
	// attribute will use the proper package tokens because
	// the package is pushed to the stack before launching
	// any link.

	// Add the reserved item types to the menu item.
	// These could be overwritten by a param class later on, if one exists.
	miRef->setLinkAttr(linkAttr);

	// Iterate the classes for the param class data.
	LcCXmlElem* data = item->getFirstChild();
	for (; data != NULL; data = data->getNext())
	{
		if (data->getName().compareNoCase(NDHS_TP_XML_DATA) != 0)
			continue;

		// Extract the class name.
		LcTaString elementClass = data->getAttr(NDHS_TP_XML_FIELD, "");
		if (elementClass.length() > 0)
		{
			// If the class was already supplied as a global menu param
			// We don't add it as a menu item param
			LcTaString val;
			if (getFieldData(NULL, elementClass, val))
				continue;

			// Extract the class and type value.
			LcTaString value = data->getAttr(NDHS_TP_XML_VALUE, "");

			// Replace tokens now for menu additions
			if (tokenReplacer)
			{
				LcTaString outValue;
				tokenReplacer->replaceTokens(value, outValue, false);
				value = outValue;
			}
			else if(tokenStack)
			{
				LcTaString outValue;
				tokenStack->replaceTokens(value,outValue,NULL,NULL,NULL,NULL,-1);
				value = outValue;
			}

			LcTaString dataType = data->getAttr(NDHS_TP_XML_VALUE_TYPE, "string");

			LcTaOwner<NdhsCField> field = createField(elementClass, m_menuItemObjects.size(), NULL, IFXI_FIELD_SCOPE_MENU, dataType, value);

			if (field)
			{
				miRef->addField(elementClass, field);
			}

			miRef->addElementCacheClass(elementClass, value);

			// Add to item classes
			if (!isItemField(elementClass)) // Check if already registered
				m_itemClasses.push_back(elementClass.toLower());
		}
	}

	// Attach the menu item to the list item - note that we have
	// one array for ownership, and another for working with!
	m_menuItemObjects.push_back(miRef);
	m_menuItems.push_back(m_menuItemObjects.back());

	return m_menuItems.back();
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenu::populateMenuFromPlugin()
{
	m_lodRequired = false;
	m_lodDecided = false;

	m_menuPlugin->clearFieldCache();

	m_itemCount = m_menuPlugin->getItemCount();
	// get total number of menu items.
	m_menuItems.resize(m_itemCount, NULL);

	// Get the first active item, and force it to be -1 if not a valid index
	m_firstActiveItemPluginIndex = m_menuPlugin->getMenuFirstActiveItem();

	if (m_firstActiveItemPluginIndex < 0 || m_firstActiveItemPluginIndex >= m_itemCount)
	{
		m_firstActiveItemPluginIndex = -1;
	}

	// Unless the menu is sorted, the first item will be the same as the plugin index
	m_firstActiveItem = m_firstActiveItemPluginIndex;

	return true;
}

/*-------------------------------------------------------------------------*//**
	Adds a menu item from a plugin.
*/
void NdhsCMenu::addPluginMenuItem(int itemIndex)
{
	// Retrieve the menu's plugin data only for the sort
	// field so that we can sort.  The rest of the data
	// will be pulled from the plugin on every request.

	// Create a new list and menu item
	LcTaOwner<NdhsCMenuItem> miRef	= NdhsCMenuItem::create(this);
	miRef->setIndex(itemIndex);


	if ((m_menuSortClass.length() > 0) && (!isLodRequired()))
	{
		LcTaString value;
		if (m_menuPlugin->getMenuItemElementData(NULL, itemIndex, m_menuSortClass, value))
			miRef->addElementCacheClass(m_menuSortClass, value);
	}

	// Attach the menu item to the list item
	m_menuItemObjects.push_back(miRef);
	m_menuItems[itemIndex] = m_menuItemObjects.back();
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuItem* NdhsCMenu::getMenuItem(int itemIndex)
{
	NdhsCMenuItem* mItem = NULL;
	int menuItemCount = m_menuItems.size();

	// Make sure we are indexing within the bounds of menu item list
	if ( (menuItemCount > 0) && (itemIndex < menuItemCount) ) 
	{
		if (m_menuItems[itemIndex] == NULL)
		{
			addPluginMenuItem(itemIndex);
		}

		mItem = m_menuItems[itemIndex];
	}

	return mItem;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenu::menuItemExist(int itemIndex)
{
	bool result = false;
	int menuItemCount = m_menuItems.size();

	// Make sure we are indexing within the bounds of menu item list
	if ( (menuItemCount > 0) && (itemIndex < menuItemCount) )
	{
		if (m_menuItems[itemIndex] != NULL)
		{
			result = true;
		}
	}

	return result;
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCMenu::getMenuItemIndex(int itemIndex)
{
	int index	= -1;

	if (itemIndex != -1)
	{
		NdhsCMenuItem* item=getMenuItem(itemIndex);
		if (item != NULL)
			index=item->getIndex();
	}

	return index;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenu::releaseMenuItem(int itemIndex)
{
	if (m_menuSource==EPlugin && (itemIndex > -1) && (m_menuItems[itemIndex] != NULL))
	{
		LcTmOwnerArray<NdhsCMenuItem>::iterator it = m_menuItemObjects.begin();
		for(; it != m_menuItemObjects.end(); it++)
		{
			if( *it == (LcCBase*)(m_menuItems[itemIndex]) )
			{
				(*it)->decRefCount();
				if ((*it)->getRefCount() <= 0
					&& m_lodRequired)
				{
					m_menuItemObjects.erase(it);
					m_menuItems[itemIndex] = NULL;
				}
				break;
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenu::acquireMenuItem(int itemIndex)
{
	if (m_menuSource==EPlugin && (itemIndex > -1) && (m_menuItems[itemIndex] != NULL))
	{
		LcTmOwnerArray<NdhsCMenuItem>::iterator it = m_menuItemObjects.begin();
		for(; it != m_menuItemObjects.end(); it++)
		{
			if( *it == (LcCBase*)(m_menuItems[itemIndex]) )
			{
				(*it)->incRefCount();
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenu::addMenuAdditions(const LcTmString& packagePath)
{
	LcTaString err;
	// Set the directory slash separators to the non default if required.
	LcTaString localPath = packagePath + NDHS_PACKAGE_FILENAME;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif

#ifdef NDHS_JNI_INTERFACE
	m_menuCon->getPageManager()->openPackage(packagePath);
#endif

	LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(localPath, err);
	if (root)
	{
		// Load the tokens for the menu item data
		LcTaOwner<LcCTokenReplacer> tokenReplacer = LcCTokenReplacer::create(m_language, m_screenSize);
		tokenReplacer->initFromXml(root->find(NDHS_TP_XML_TOKENS));

		// Iterate through each menu insert
		LcCXmlElem* insert = root->getFirstChild();
		for (; insert != NULL; insert = insert->getNext())
		{
			if (insert->getName().compareNoCase(NDHS_TP_XML_MENU_INSERT) != 0)
				continue;

			NdhsCMenuItem* item = addXmlMenuItem(insert, tokenReplacer.ptr());

			// Set the item as an addition
			item->setMenuAddition();
			item->setPackagePath(packagePath);
		}
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void NdhsCMenu::addField(const LcTmString& fieldName, LcTaOwner<NdhsCField>& value)
{
	// Convert search string to lower case.
	LcTaString lowerFieldName = fieldName.toLower();

	m_staticMenuFields.add_element(lowerFieldName,value);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCMenu::addElementCacheClass(const LcTmString& elementClass, const LcTmString& value)
{
	// Convert search string to lower case.
	LcTaString lowerElementClass = elementClass.toLower();

	m_elementCachedClasses[lowerElementClass] = value;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenu::getFieldData(NdhsCElement* element, const LcTmString& elementClass, LcTmString& value)
{
	if (m_menuSource == EPlugin)
	{
		if (m_menuPlugin->getMenuElementData(element, elementClass, value))
			return true;
	}
	else
	{
		bool bPluginChecked = false;

#ifdef IFX_USE_PLUGIN_ELEMENTS
		// If we have element context, we must check the plugin first
		// but without menu context because its an xml menu.
		if (m_plugin && element && element->getPluginElement())
		{
			if (m_plugin->getElementData(-1, elementClass, IFX_FIELD, element, NULL, value))
				return true;

			bPluginChecked = true;
		}
#endif

		// No match, so carry on and check the menu xml fields

		// Convert search string to lower case.
		LcTaString lowerElementClass = elementClass.toLower();

		// If the field exists then return the data.
		TmMStaticMenuFields::iterator pos = m_staticMenuFields.find(lowerElementClass);
		if (pos != m_staticMenuFields.end())
		{
			value = pos->second->getFieldData(element);
			return true;
		}

		// If we didn't ask the plugin before,
		// we should do it now, but with no context
		if (!bPluginChecked && m_plugin)
		{
			if (m_plugin->getElementData(-1, elementClass, IFX_FIELD, NULL, NULL, value))
				return true;
		}
	}

	// Item not found
	value = "";
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCMenu::findMenuItem(int itemPluginIndex)
{
	int itCount = 0;
	TmAMenuItems::iterator it = m_menuItems.begin();

	for (; it != m_menuItems.end(); itCount++, it++)
	{
		if (*it)
		{
			if ((*it)->getIndex() == itemPluginIndex)
			{
				return itCount;
			}
		}
		else
		{
			// NULL pointer in item list
			LC_ASSERT(0);
		}
	}

	return -1;
}

/*-------------------------------------------------------------------------*//**
																			 */
LcTaOwner<NdhsCField> NdhsCMenu::createField(	const LcTmString& fieldName,
													  int menuItemIndex,
													  NdhsCElement* element,
													  IFXI_FIELD_SCOPE scope,
													  const LcTmString& dataType,
													  LcTaString value)
{
	LcTaOwner<NdhsCField> retVal;

	IFXI_FIELDDATA_TYPE type = IFXI_FIELDDATA_STRING;

	if (dataType.compareNoCase("int") == 0)
	{
		type = IFXI_FIELDDATA_INT;
	}
	else if (dataType.compareNoCase("boolean") == 0)
	{
		type = IFXI_FIELDDATA_BOOL;
	}
	else if (dataType.compareNoCase("float") == 0)
	{
		type = IFXI_FIELDDATA_FLOAT;
	}
	else if (dataType.compareNoCase("time") == 0)
	{
		type = IFXI_FIELDDATA_TIME;
	}

	LcTaString lowercaseFieldName = fieldName.toLower();
	retVal = NdhsCField::create(m_plugin->getCon(), lowercaseFieldName, false,
															NULL, menuItemIndex, element, IFXI_FIELD_MODE_OUTPUT, scope, type, value);

	return retVal;
}


/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCMenu::getField(const LcTmString& fieldName,
								int menuItemIndex,
								NdhsCElement* element)
{
	NdhsCField* retVal = NULL;

	if (m_menuSource != EPlugin)
	{
		NdhsCMenuItem *menuItem = (menuItemIndex != -1)? getMenuItem(menuItemIndex) : NULL;
		if (menuItem)
		{
			retVal = menuItem->getField(fieldName,element);
		}
		else
		{
			// Convert search string to lower case.
			LcTaString lowerFieldName = fieldName.toLower();

			// If the field exists then return the data.
			TmMStaticMenuFields::iterator pos = m_staticMenuFields.find(lowerFieldName);
			if (pos != m_staticMenuFields.end())
			{
				retVal = pos->second;
			}
		}
	}

	if (!retVal && m_plugin)
	{
		// It's fine if m_menuPlugin.ptr() == NULL, it will find the global field instead
		retVal = m_plugin->getField(fieldName, m_menuPlugin.ptr(), menuItemIndex, element);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenu::updateFields(LcTTime timestamp)
{
	bool reschedule = false;

	if (m_menuPlugin)
	{
		reschedule = m_menuPlugin->updateFields(timestamp);
	}

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenu::refreshField(	const LcTmString& fieldName,
								int menuItemIndex)
{
	if (m_menuPlugin)
	{
		m_menuPlugin->refreshField(fieldName, menuItemIndex);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenu::setPluginFieldsDirty(bool requiresLaundry)
{
	if (m_menuPlugin)
	{
		m_menuPlugin->setPluginFieldsDirty(requiresLaundry);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenu::stopFields()
{
	if (m_menuPlugin)
	{
		m_menuPlugin->stopFields();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenu::activeItemUpdated(int itemIndex)
{
	m_firstActiveItem = itemIndex;

	if (m_menuPlugin)
	{
		if(m_menuItems.size()>0 && itemIndex!=-1)
			itemIndex=getMenuItemIndex(itemIndex);
			
		m_menuPlugin->activeItemUpdated(itemIndex);
		m_firstActiveItemPluginIndex = itemIndex;
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenu::isItemField(const LcTmString& fieldName)
{
	bool retVal = false;

	switch (m_menuSource)
	{
		case ENodeXml:
		case EXml:
		{
			// Check if we found the field name while loading the menu
			TmAElementClasses::iterator it = m_itemClasses.begin();
			for (; it != m_itemClasses.end() && !retVal; it++)
			{
				retVal = (fieldName.compareNoCase(*it) == 0);
			}
		}
		break;

		case EPlugin:
		{
			// Ask the plugin for info about the field
			if (m_plugin)
			{
				IFXI_FIELD_MODE mode;
				IFXI_FIELD_SCOPE scope;
				IFXI_FIELDDATA_TYPE variant;

				if (m_plugin->getFieldInfo(fieldName, m_menuPlugin.ptr(), 0, mode, scope, variant))
				{
					retVal = (IFXI_FIELD_SCOPE_ITEM == scope);
				}
			}
		}
		break;

		default:
			break;
	}

	return retVal;
}


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenu::populateMenuFromPageFile(const LcTmString& package, const LcTmString& pageFile, const LcTmString& menuName)
{
	if (pageFile.isEmpty() || menuName.isEmpty())
		return false;

	cleanupMenuData();

	m_package = package;
	m_menuFile = pageFile;
	m_menuName=menuName;

	if (loadXmlMenu())
	{
		// Get root page menu node, <pageMenus>
		LcCXmlElem* ePageMenus = m_menuFileXml->find(NDHS_TP_XML_NODE_MENUS);
		if (ePageMenus)
		{
			// Look for the child <pageMenu>
			LcCXmlElem* ePageMenu = ePageMenus->getFirstChild();

			for (; ePageMenu; ePageMenu = ePageMenu->getNext())
			{

				// Look for the child <menu> node
				LcCXmlElem* eMenu = ePageMenu->getFirstChild();

				if (eMenu)
				{
					// Get page Uri from node
					if (eMenu->getAttr(NDHS_TP_XML_NAME).toLower().compareNoCase(menuName) == 0)
					{
						m_menuFileRoot = ePageMenu;

						if (populateMenuFromXml())
						{
							//
							// Now add the menu additions
							//
							populateMenuWithAdditions(m_package, menuName);

							// Store length before any chance of double menu items being called
							m_menuOriginalLength = (int)m_menuItems.size();
							m_menuSource = ENodeXml;
							return true;
						}
					}
				}
			}
		}
	}

	// Menu does not exists in current node
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenu::setLODRequired(int slotCount)
{
	if(m_lodDecided && !m_lodRequired)
		return;

	// Set the LOD attribute
	if (m_menuSource == EXml || m_menuSource == ENodeXml)
	{
		// If total number of items are below LOD threshold
		if ((int)m_menuItems.size() <= slotCount * LOD_THRESHOLD)
			m_lodRequired = false;
	}
	else if (m_menuSource == EPlugin)
	{
		m_lodRequired = m_menuPlugin->getMenuLOD();

		//	 If total number of items are below LOD threshold
		if (m_itemCount <= slotCount * LOD_THRESHOLD)
			m_lodRequired = false;

		if (m_lodRequired)
		{
			m_menuSortClass = "";
		}
//	12.3 都不需要排序，值都是""	
//	12.3 的m_menuSortClass.length() 为0			
#if 0 /*ligui added here*/
		else
		{
			// We only sort when LOD is not enabled
//	12.3 都不需要排序，值都是""		
			m_menuSortClass = m_menuPlugin->getMenuSortClass();

			// if sort class is empty or itemCount is 0, no need to proceed
//	12.3 的m_menuSortClass.length() 为0		
			if((m_menuSortClass.length() > 0) && (m_itemCount > 0))
			{
				LcTaString sortType = m_menuPlugin->getMenuSortType();

				if (sortType.compareNoCase(NDHS_TP_XML_VAL_CASE_INSENSITIVE) == 0)
				{
				   m_menuSortType = ECaseInsensitive;
				}
				else
				{
					// Default to case sensitive
					m_menuSortType = ECaseSensitive;
				}

				LcTaString sortDirection = m_menuPlugin->getMenuSortDirection();

				m_menuSortDirection = EAscending;

				if (sortDirection.compareNoCase(NDHS_TP_XML_VAL_DESCENDING) == 0)
				{
					m_menuSortDirection = EDescending;
				}

				// Iterate the plugin menu items to get sort data.
				for (int item = 0; item < m_itemCount; item++)
				{
					addPluginMenuItem(item);
				}

				//
				// Now sort the menu items
				//
				if (m_menuSortClass.length() > 0)
				{
					IFX_ShellSort(m_menuItems.begin(), m_menuItems.end(), itemCompare);

					// Update first active item
					if (m_menuSource == EPlugin && m_firstActiveItemPluginIndex > -1)
					{
						m_firstActiveItem = max(0, findMenuItem(m_firstActiveItemPluginIndex));
					}
				}
			}
		}
#endif
	}

	m_lodDecided = true;
}
