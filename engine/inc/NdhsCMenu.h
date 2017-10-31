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
#ifndef NdhsCMenuH
#define NdhsCMenuH

#include "inflexionui/engine/inc/NdhsCPlugin.h"

// LOD_THRESHOLD specifies number of pages always loaded when LOD is required
// Load on demand will be used if the number of items exceeds slotCount * LOD_THRESHOLD
#define LOD_THRESHOLD 			3

// To load a menu, first you must call the relevant
// openMenu function.  After this, you can call
// getMenuName() to decide which page template to
// use.  To fill the menu with items and menu header
// classes, you must call populateMenu().

class NdhsCMenu : public LcCBase, public ISerializeable
{
LC_PRIVATE_INTERNAL_PUBLIC:

	typedef enum {
		EAscending = 0,
		EDescending
	} ESortDirection;

	typedef enum {
		ECaseSensitive = 0,
		ECaseInsensitive
	} ESortType;

	typedef enum {
		EXml = 0,
		ENodeXml,
		EPlugin
	} EMenuSource;

private:

	typedef LcTmArray<LcTmString> 			TmAString;
	typedef LcTmArray<NdhsCMenuItem*> 		TmAMenuItems;
	typedef LcTmOwnerArray<NdhsCMenuItem> 	TmOAMenuItems;
	typedef LcTmMap<LcTmString, LcTmString>	TmMElementClassData;
	typedef LcTmArray<LcTmString> 			TmAElementClasses;

	// Define a map for static menu field values.
	typedef LcTmOwnerMap<LcTmString, NdhsCField> TmMStaticMenuFields;

	TmAMenuItems					m_menuItems;
	TmOAMenuItems					m_menuItemObjects;
	LcTmString						m_menuName;
	LcTmString						m_menuLink;
	int								m_menuOriginalLength;

	LcTmString						m_menuSortClass;
	ESortType						m_menuSortType;
	ESortDirection					m_menuSortDirection;

	LcTmString						m_package;
	TmMElementClassData				m_elementCachedClasses;
	TmAElementClasses				m_itemClasses;
	EMenuSource						m_menuSource;

	TmMStaticMenuFields				m_staticMenuFields;

	NdhsCMenuCon*					m_menuCon;
	NdhsCPlugin*					m_plugin;
	LcTmString						m_language;
	LcTmString						m_screenSize;

	bool							m_duplicatedMenu;

	int								m_firstActiveItem;
	int								m_firstActiveItemPluginIndex;
	int								m_itemCount;

	// Xml menu members
	LcTmString						m_menuFile;
	LcTmOwner<LcCXmlElem>			m_menuFileXml;

	// This will point to root of menu, by using
	// we will be able to change root at run time,
	// to point to correct menu in page file
	LcCXmlElem*						m_menuFileRoot;

	// Plugin menu members
	LcTmOwner<NdhsCPlugin::NdhsCPluginMenu>	m_menuPlugin;

	// LOD members
	bool							m_lodRequired;
	bool							m_lodDecided;

	void							cleanupMenuData();

	bool 							loadXmlMenu();

	bool							populateMenuFromXml();
	bool							populateMenuFromPlugin();

	NdhsCMenuItem*					addXmlMenuItem(LcCXmlElem* item, LcCTokenReplacer* tokenReplacer = NULL,NdhsCTokenStack *tokenStack=NULL);
	void							addPluginMenuItem(int itemIndex);

	int								findMenuItem(int itemPluginIndex);

LC_PRIVATE_INTERNAL_PUBLIC:

	LcTaString						getSortClass()		{ return m_menuSortClass; }
	int								getSortType()		{ return m_menuSortType; }
	int								getSortDirection()	{ return m_menuSortDirection; }
	EMenuSource						getMenuSource()		{ return m_menuSource; }
	NdhsCPlugin::NdhsCPluginMenu*	getMenuPlugin()		{ return m_menuPlugin.ptr(); }
	NdhsCPlugin*					getPlugin()			{ return m_plugin; }

protected:

	// Two-phase construction
									NdhsCMenu(
										NdhsCMenuCon* con,
										NdhsCPlugin* plugin,
										const LcTmString& language,
										const LcTmString& screenSize);

	LC_VIRTUAL bool					populateMenuWithAdditions(
										const LcTmString& package,
										const LcTmString& menuFile);
	LC_IMPORT void					addElementCacheClass(const LcTmString& elementClass, const LcTmString& value);
	void							addField(const LcTmString& fieldName, LcTaOwner<NdhsCField>& value);
LC_PROTECTED_INTERNAL_PUBLIC:
	LC_IMPORT bool					addMenuAdditions(const LcTmString& packagePath);

public:
	NdhsCMenuCon*					getMenuCon()		{ return m_menuCon; }

	static LcTaOwner<NdhsCMenu>		create(NdhsCMenuCon* con, NdhsCPlugin* plugin, const LcTmString& language, const LcTmString& screenSize);
	virtual							~NdhsCMenu();

	inline bool						isLodRequired()		{ return m_lodRequired; }
	inline LcTaString				getMenuLink()	{ return m_menuLink; }
	inline void						setMenuLink(const LcTmString& menuLink) { m_menuLink = menuLink; }
	LC_IMPORT bool					openMenuFromXml(const LcTmString& package, const LcTmString& menuFile);
	LC_IMPORT bool					openMenuFromPlugin(
										const LcTmString& package,
										const LcTmString& link);

	LC_IMPORT void					setLODRequired(int slotCount);
	LC_IMPORT bool					populateMenu();
	LC_IMPORT bool					reloadMenu();

	void							replicateMenuItems(int factor);

	LcTaString						getPackageName(){ return m_package; }
	LcTaString						getMenuName()	{ return m_menuName; }
	inline int						getItemCount()	{ return (int)m_menuItems.size(); }
	NdhsCMenuItem*					getMenuItem(int itemIndex);
	int								getMenuItemIndex(int itemIndex);
	bool							menuItemExist(int itemIndex);
	inline int						getFirstActiveItem()	{ return m_firstActiveItem; }
	void							acquireMenuItem(int itemIndex);
	void							releaseMenuItem(int itemIndex);

	// Field functions
	LC_IMPORT bool					getFieldData(NdhsCElement* element, const LcTmString& elementClass, LcTmString& value);
	NdhsCField*						getField(const LcTmString& fieldName,
											int menuItemIndex,
											NdhsCElement* element);
	LcTaOwner<NdhsCField>			createField(const LcTmString& fieldName,
											int menuItemIndex,
											NdhsCElement* element,
											IFXI_FIELD_SCOPE scope,
											const LcTmString& dataType,
											LcTaString value);
	bool							updateFields(LcTTime timestamp);
	void 							refreshField(	const LcTmString& fieldName,
													int menuItemIndex);
	void							setPluginFieldsDirty(bool requiresLaundry);
	void							stopFields();

	void							activeItemUpdated(int itemIndex);
	bool							isItemField(const LcTmString& fieldName);
	bool							populateMenuFromPageFile(const LcTmString& package, const LcTmString& pageFile, const LcTmString& menuName);
#ifdef IFX_SERIALIZATION
	virtual SerializeHandle			serialize(LcCSerializeMaster *serializeMaster,bool force=false){return -1;}
	bool							isMenuItemChild(){return false;}
#endif /* IFX_SERIALIZATION */

#ifdef NDHS_JNI_INTERFACE
	LcTaString						getMenuFile()	{ return m_menuFile; }
#endif
};
#endif
