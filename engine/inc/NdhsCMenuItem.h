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
#ifndef NdhsCMenuItemH
#define NdhsCMenuItemH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTString.h"

class LcCTokenReplacer;

/*-------------------------------------------------------------------------*//**
	NdhsCMenuItem holds the view-independent information about an item
	that can appear in a menu.
*/
class NdhsCMenuItem : public LcCBase
{
public:

	// Define a map to store the class name and class value.
	typedef LcTmMap<LcTmString, LcTmString>	TmMElementClassData;

	// Define a map for static menu field values.
	typedef LcTmOwnerMap<LcTmString, NdhsCField> TmMStaticMenuFields;

private:

	// Menu that owns this item
	NdhsCMenu*			m_owner;

	// The index of the item
	int					m_index;

	// reference Count
	int					m_refCount;

	// Store the element class data.
	TmMElementClassData	m_elementCachedClasses;

	TmMStaticMenuFields m_staticMenuFields;

	LcTmString			m_link;

	// Data for menu addition items
	bool				m_menuAddition;
	LcTmString			m_packagePath;

	// Two-phase construction
						NdhsCMenuItem(NdhsCMenu* owner) { m_owner = owner; }
	void				construct();

LC_PRIVATE_INTERNAL_PUBLIC:
	NdhsCMenu*			getOwner() { return m_owner; }

public:

	// Create/destruct
	static LcTaOwner<NdhsCMenuItem> create(NdhsCMenu* owner);

	virtual				~NdhsCMenuItem() {}

	// Add a new class item.
	void				addElementCacheClass(const LcTmString& elementClass, const LcTmString& value);
	void				addElementCacheClasses(TmMElementClassData& elementClasses);

	// Property access:

	// Get the class value and type.
	bool				getFieldData(NdhsCElement* element, const LcTmString& elementClass, LcTmString& value);
	NdhsCField*			getField(const LcTmString& fieldName, NdhsCElement* element);
	void				addField(const LcTmString& fieldName, LcTaOwner<NdhsCField>& value);
	bool				getCachedFieldData(const LcTmString& elementClass, LcTmString& value);

	inline void			setIndex(int index)								{ m_index = index; }
	inline int			getIndex()										{ return m_index; }

	inline void			setLinkAttr(const LcTmString& link)				{ m_link = link; }

	LcTaString			getLinkAttr();

	inline void			setMenuAddition()								{ m_menuAddition = true; }
	inline void			setPackagePath(const LcTmString& packagePath)	{ m_packagePath = packagePath; }

	inline bool			isMenuAddition()								{ return m_menuAddition; }
	inline LcTaString	getPackagePath()								{ return m_packagePath; }


	// reference count handler
	inline int			getRefCount()									{ return m_refCount; }
	inline void			incRefCount()									{ ++m_refCount; }
	inline void			decRefCount()									{ --m_refCount; }

	// Extracts the link prefix and link data from a token replaced link attribute.
	LcTaString			getTRLinkPrefix(NdhsCTokenStack* tokenStack, int stackLevel);
	LcTaString			getTRLinkData(NdhsCTokenStack* tokenStack, int stackLevel);
	LcTaString			getTRLink(NdhsCTokenStack* tokenStack, int stackLevel);
};
#endif
