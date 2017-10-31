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
	Create
*/
LcTaOwner<NdhsCMenuItem> NdhsCMenuItem::create(NdhsCMenu* owner)
{
	LcTaOwner<NdhsCMenuItem> ref;
	ref.set(new NdhsCMenuItem(owner));
   	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Construct
*/
void NdhsCMenuItem::construct()
{
	m_elementCachedClasses.clear();
	m_menuAddition = false;
	m_index = -1;
	m_refCount = 0;
}

/*-------------------------------------------------------------------------*//**
	Add a new class item.
*/
void NdhsCMenuItem::addElementCacheClass(const LcTmString& elementClass, const LcTmString& value)
{
	// Convert to lower case.
	LcTaString lowerElementClass = elementClass.toLower();

	m_elementCachedClasses[lowerElementClass] = value;
}

/*-------------------------------------------------------------------------*//**
	Add a set of new class items.
*/
void NdhsCMenuItem::addElementCacheClasses(TmMElementClassData& elementClasses)
{
	TmMElementClassData::iterator it = elementClasses.begin();

	for (; it != elementClasses.end(); it++)
	{
		m_elementCachedClasses[it->first] = it->second;
	}
}

/*-------------------------------------------------------------------------*//**
	Get the class value.
*/
bool NdhsCMenuItem::getFieldData(NdhsCElement* element, const LcTmString& elementClass, LcTmString& value)
{
	LcTaString linkData;

	if ((m_menuAddition == false) && m_owner && (m_owner->getMenuSource() == NdhsCMenu::EPlugin))
	{
		if (m_owner->getMenuPlugin()->getMenuItemElementData(element, m_index, elementClass, value))
			return true;
	}
	else
	{
		bool bPluginChecked = false;
		NdhsCPlugin* plugin = NULL;

		if (m_owner)
			plugin = m_owner->getPlugin();

#ifdef IFX_USE_PLUGIN_ELEMENTS
		// If we have element context, we must check the plugin first
		// but without menu or item context because its an xml menu.
		if (plugin && element && element->getPluginElement())
		{
			if (plugin->getElementData(-1, elementClass, IFX_FIELD, element, NULL, value))
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
		if (!bPluginChecked && plugin)
		{
			if (plugin->getElementData(-1, elementClass, IFX_FIELD, NULL, NULL, value))
				return true;
		}
	}

	// Item not found
	value = "";
	return false;
}

/*-------------------------------------------------------------------------*//**
																			 */
void NdhsCMenuItem::addField(const LcTmString& fieldName, LcTaOwner<NdhsCField>& value)
{
	// Convert search string to lower case.
	LcTaString lowerFieldName = fieldName.toLower();

	m_staticMenuFields.add_element(lowerFieldName,value);
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCMenuItem::getField(const LcTmString& fieldName, NdhsCElement* element)
{
	NdhsCField* retVal = NULL;

	if (m_owner)
	{
		if (m_owner->getMenuSource() != NdhsCMenu::EPlugin)
		{
			TmMStaticMenuFields::iterator iter = m_staticMenuFields.find(fieldName.toLower());

			if (iter != m_staticMenuFields.end())
			{
				retVal = iter->second;
			}
		}
		else
		{
			retVal = m_owner->getField(fieldName, m_index, element);
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Get the cached class value.  This is required because plugin
	menus now only cache for the sorting step
*/
bool NdhsCMenuItem::getCachedFieldData(const LcTmString& elementClass, LcTmString& value)
{
	// Convert search string to lower case.
	LcTaString lowerElementClass = elementClass.toLower();

	// If the class exists then return the data.
	TmMElementClassData::iterator pos = m_elementCachedClasses.find(lowerElementClass);
	if (pos != m_elementCachedClasses.end())
	{
		value = pos->second;
		return true;
	}

	// Item not found
	value = "";
	return false;
}

/*-------------------------------------------------------------------------*//**
	Retrieve the link prefix (before the ://) of the menu item link after
	tokens have been replaced.
*/
LcTaString NdhsCMenuItem::getTRLinkPrefix(NdhsCTokenStack* tokenStack, int stackLevel)
{
	LcTaString linkData;

	if ((m_menuAddition == false) && m_owner && (m_owner->getMenuSource() == NdhsCMenu::EPlugin))
	{
		m_owner->getMenuPlugin()->getMenuItemElementLink(m_index, linkData);
	}
	else
	{
		linkData = m_link;
	}

	// If the link is blank return unknown.
	if (linkData.length() == 0)
		return "";

	// Extract the link and token replace it to identify the type.
	LcTaString linkName;
	tokenStack->replaceTokens(linkData, linkName, NULL, NULL, this, NULL, stackLevel);
	linkName = linkName.subString(0, linkName.find(":"));

	return linkName;
}

/*-------------------------------------------------------------------------*//**
	Retrieve the link data (after the ://) of the menu item link after tokens
	have been replaced.
*/
LcTaString NdhsCMenuItem::getTRLinkData(NdhsCTokenStack* tokenStack, int stackLevel)
{
	LcTaString linkData;

	if ((m_menuAddition == false) && m_owner && (m_owner->getMenuSource() == NdhsCMenu::EPlugin))
	{
		m_owner->getMenuPlugin()->getMenuItemElementLink(m_index, linkData);
	}
	else
	{
		linkData = m_link;
	}

	// If the link is blank return nothing
	if (linkData.length() == 0)
		return "";

	// Extract the type and token replace it.
	LcTaString outLinkData;
	tokenStack->replaceTokens(linkData, outLinkData, NULL, NULL, this, NULL, stackLevel);
	return outLinkData.tail(outLinkData.find("://") + 3);
}

/*-------------------------------------------------------------------------*//**
Retrieve the whole link after tokens have been replaced.
*/
LcTaString NdhsCMenuItem::getTRLink(NdhsCTokenStack* tokenStack, int stackLevel)
{
	LcTaString link;

	if ((m_menuAddition == false) && m_owner && (m_owner->getMenuSource() == NdhsCMenu::EPlugin))
	{
		m_owner->getMenuPlugin()->getMenuItemElementLink(m_index, link);
	}
	else
	{
	link = m_link;
	}

	// If the link is blank return nothing
	if (link.length() == 0)
		return "";

	// Extract the type and token replace it.
	LcTaString outLinkData;
	tokenStack->replaceTokens(link, outLinkData, NULL, NULL, this, NULL, stackLevel);
	return outLinkData;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCMenuItem::getLinkAttr()
{
	if ((m_menuAddition == false) && m_owner && (m_owner->getMenuSource() == NdhsCMenu::EPlugin))
	{
		LcTaString linkAttr;
		m_owner->getMenuPlugin()->getMenuItemElementLink(m_index, linkAttr);
		return linkAttr;
	}
	else
	{
		return m_link;
	}
}
