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
#ifndef NdhsCPageH
#define NdhsCPageH

#include "inflexionui/engine/inc/NdhsCPageManager.h"

class NdhsCPage : public LcCBase, public ISerializeable
{
private:

	LcTmString						m_pageName;

	typedef LcTmMap<LcTmString, LcTmString>	TmMPairs;

	// List of the pages to which we can navigate from
	// current page, using map so that retreival is fast
	TmMPairs						m_validPairs;

	NdhsCMenuCon*					m_con;
	NdhsCPageManager*				m_pageManager;

	// page link along with paramters
	LcTmString						m_pageLink;

	// Xml page members
	LcTmString						m_pageFile;
	LcTmString						m_packageName;
	LcTmOwner<LcCXmlElem>			m_pageFileXml;
	LcCXmlElem*						m_menuRoot;

	bool							m_allowNavigationToAllNodes;

protected:

	// Two-phase construction
									NdhsCPage(NdhsCMenuCon* con, NdhsCPageManager* pageManager);

public:

	static LcTaOwner<NdhsCPage>		create(NdhsCMenuCon* con, NdhsCPageManager* pageManager);
	virtual							~NdhsCPage();

	inline LcTaString				getPageName()	{ return m_pageName; }
	inline void						setPageLink (const LcTmString& pageLink) { m_pageLink = pageLink; }
	inline LcTaString				getPageLink () { return m_pageLink; }
	void							cleanupMenuData();

	LC_EXPORT bool 					loadXmlPage();
	LC_EXPORT bool 					openPageFromXml(const LcTmString& package, const LcTmString& pageFile);
	LC_EXPORT void 					populatePageNodesMap();
	LC_EXPORT bool 					isPageInList(const LcTmString& page);
	LC_EXPORT LcCXmlElem* 			getMenuRoot(const LcTmString& name);

	NdhsCField*						getField(const LcTmString& fieldName,
											int menuItemIndex,
											NdhsCElement* element)
									{
										LC_UNUSED(fieldName) LC_UNUSED(menuItemIndex)
										LC_UNUSED(element)
										return NULL;
									}
	LC_EXPORT bool 					getFieldData(NdhsCElement* element, const LcTmString& elementClass, LcTmString& value);

	LcTaString						getPageFile()	{ return m_pageFile; }
	bool							allowNavigation() { return m_allowNavigationToAllNodes; }

#ifdef IFX_SERIALIZATION
	virtual	SerializeHandle			serialize(LcCSerializeMaster *serializeMaster,bool force=false){return -1;}
			bool					isMenuItemChild(){return false;}
#endif /* IFX_SERIALIZATION */

};
#endif
