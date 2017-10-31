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


#define NDHS_MAIN_PACKAGE_NAME					"main/"

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPage> NdhsCPage::create(NdhsCMenuCon* con, NdhsCPageManager* pageManager)
{
	LcTaOwner<NdhsCPage> ref;
	ref.set(new NdhsCPage(con, pageManager));
//	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPage::NdhsCPage(NdhsCMenuCon* con, NdhsCPageManager* pageManager)
{
	m_con = con;
	m_pageManager = pageManager;
	m_allowNavigationToAllNodes = true;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPage::~NdhsCPage()
{
	cleanupMenuData();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPage::cleanupMenuData()
{
	if (m_pageFileXml)
		m_pageFileXml.destroy();

	m_validPairs.clear();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCPage::loadXmlPage()
{
	bool retVal = false;
	LcTaString err;

	// Set the directory slash separators to the non default if required.
	LcTaString localPath = m_pageFile;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif

	// Load the file.
	LcTaOwner<LcCXmlElem> tempXmlElem = LcCXmlElem::load(localPath, err);
	m_pageFileXml = tempXmlElem;
	if (m_pageFileXml)
		retVal = true;

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCPage::openPageFromXml(const LcTmString& package, const LcTmString& pageFile)
{

	bool retVal = false;

	// make sure there is no previous menu data
	cleanupMenuData();

	m_pageFile = pageFile;
	m_packageName = package;

	if (loadXmlPage())
	{
		m_pageName = m_pageFileXml->getAttr(NDHS_TP_XML_NAME);
		if (m_pageName.length() > 0)
		{
			retVal = true;
			populatePageNodesMap();
		}

		// If the transaction has failed then throw away the xml.
		if (!retVal)
		{
			m_pageFileXml.destroy();
		}
	}


	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPage::populatePageNodesMap()
{
	// 	m_pageFileXml is already valid
	LcCXmlElem* ePageNodes = m_pageFileXml->find(NDHS_TP_XML_UI_FLOW_NODES);
	if (ePageNodes)
	{
		// Look for the child components
		LcCXmlElem* ePageNode = ePageNodes->getFirstChild();

		// As per rules if UIX generate empty uiFlowNodes element
		// disallow navigation to any node from current node
		if (ePageNode == NULL)
		{
			m_allowNavigationToAllNodes = false;
		}

		for (; ePageNode; ePageNode = ePageNode->getNext())
		{
			// Get page Uri from node
			LcTaString nodeUri = ePageNode->getAttr(NDHS_TP_XML_NODE).toLower();

			// If there is an empty node then add it to map to prevent navigation.
			// Add in map, so that retreival is fast
			m_validPairs[nodeUri] = nodeUri;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCPage::isPageInList(const LcTmString& page)
{
	// If the page exists then return the data.
	return ((m_validPairs.size() == 0) || (m_validPairs.find(page.toLower()) != m_validPairs.end()));
}


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCPage::getFieldData(NdhsCElement* element, const LcTmString& elementClass, LcTmString& value)
{
	// Page dont have any field
	return false;
}

