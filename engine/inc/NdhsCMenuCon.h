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
#ifndef NdhsCMenuConH
#define NdhsCMenuConH

#include "inflexionui/engine/inc/NdhsCPageManager.h"
#include "inflexionui/engine/inc/LcCTokenReplacer.h"
#include "inflexionui/engine/inc/LcDefs.h"
#include "inflexionui/engine/inc/NdhsCMenu.h"

class NdhsCMenuItem;
class NdhsCMenuTheme;
class NdhsCPlugin;
class NdhsCLaundry;

/*-------------------------------------------------------------------------*//**
	NdhsCMenuCon forms the platform-independent controller part of the
	NDE menuing system.  Platform-specific
*/
class NdhsCMenuCon : public LcCBase,public ISerializeable
{
public:

LC_PRIVATE_INTERNAL_PUBLIC:
	// This stores the additional link types retrieved from xml.
	struct TLinkTypeData
	{
		ENdhsLinkType				m_linkType;
		NdhsCPlugin*				m_plugin;
	};

private:

	// Core object model
	LcTmOwner<NdhsCPageManager>		m_pageManager;
	LcTmOwner<LcCSpace>				m_space;

	LcTmOwner<NdhsCPlugin>			m_plugin;
	LcTmOwner<NdhsCLaundry> 		m_laundry;

	typedef LcTmMap<LcTmString, TLinkTypeData>	TmMLinkTypeList;
	TmMLinkTypeList					m_linkTypeList;


	// App settings
	NdhsCTokenStack*				m_tokenStack;

	// State
	bool							m_handlingUIEvent;
	bool							m_messageSending;
	bool							m_callbacksEnabled;
	bool							m_asyncLinkBlocking;

	// Cached info
	LcTmString						m_appPath;


protected:

	// Construction only by derived class
									NdhsCMenuCon();
	void							construct(const LcTmString& language,
											  const LcTmString& displayMode,
											  const LcTmString& themeName,bool restoreState);

	// These are used to maintain the plug-in list.
	virtual NdhsCPlugin*			loadPlugin();

	void							destroyPageManager();

	void							addPluginToList(LcTmOwner<NdhsCPlugin>& newPlugin) { m_plugin = newPlugin; }

	// Set pointer to the space.
	void							setSpace(LcTmOwner<LcCSpace>& newSpace) { m_space.set(newSpace.release()); }

	// Derived class MUST override these:

	// Create a theme object of the named type
	virtual LcTaOwner<NdhsCPageManager> createPageManager(const LcTmString& language,
														  const LcTmString& displayMode,
														  const LcTmString& themeName)		= 0;

	// Return application executable name with extension.
	virtual LcTaString				getFullAppName()										= 0;
	// Return unique (OS-specific) ID of application
	virtual int						getAppId()												= 0;

	// Derived classes MAY override these:

	// Called after a key or command has completed (i.e. any dialog dismissed)
	virtual void					onDoneHandlingUIEvent()							{}
	// Set the key handling.
	virtual void					setHandlingUIEvent(bool enabled)				{ m_handlingUIEvent = enabled; }

	// Launch a menu item
	virtual bool					launchMenuItem(NdhsCMenuItem* mi, int stackLevel);

LC_PROTECTED_INTERNAL_PUBLIC:
	// Get space
	LcCSpace*						getSpace()										{ return m_space.ptr(); }
	virtual NdhsCPlugin*			getPlugin();

public:

#ifdef IFX_SERIALIZATION
	virtual SerializeHandle			serialize(LcCSerializeMaster *serializeMaster,bool force=false){return -1;}
			bool					isMenuItemChild(){return false;}
#endif /* IFX_SERIALIZATION */

	// Destruction
	virtual							~NdhsCMenuCon();

	// Are we in a key handler?
	bool							isHandlingUIEvent() 		{ return m_handlingUIEvent; }

	// Returns the drive that the specified file is on
	virtual	LcTaString				getDriveLetter(const LcTmString& file, const LcTmString& path);

	// Update non-NDE screen furniture
	virtual void					updateScreenFurniture()							{}

	NdhsCPlugin*					addPlugin();

	// This will return the linkType first looking at the linkType table then
	// checking the built in link types.
	virtual ENdhsLinkType			getTRLinkType(const LcTmString& linkName);

	// This will return the linktype plug-in from the linkType table.
	virtual NdhsCPlugin*			getTRLinkTypePlugin(const LcTmString& linkName);

	virtual ENdhsWidgetType			getWidgetType(const LcTmString& data);

	bool							launchLink(NdhsCMenuItem* item, int stackLevel);

	// Get a pointer to the page manager
	NdhsCPageManager*				getPageManager()		{ return m_pageManager.ptr(); }
	NdhsCLaundry*					getLaundry()			{ return m_laundry.ptr(); }

	bool							callbacksEnabled()		{ return m_callbacksEnabled; }
	void							setCallbacksEnabled(bool enabled) { m_callbacksEnabled = enabled; }

	// The theme path is the same on all drives, so we simply strip off the drive letter and :
	virtual LcTaString				getThemePath()
		{ return getAppPath().tail(getDriveLetter(getFullAppName(), getAppPath()).length()) + NDHS_PACKAGE_DIR NDHS_DIR_SEP; }

	// Do platform or app-specific configuration
	virtual void					configureScreenCanvas(bool statusPane)	{ LC_UNUSED(statusPane) }

	virtual void					setIntegrated(bool integrated) { LC_UNUSED(integrated) }

	virtual void					suspendApp() { IFXP_Command(IFXP_CMD_SUSPEND); }
	virtual void					exitApp() { IFXP_Command(IFXP_CMD_EXIT); }
	virtual void					shutDown() {}

	// Launches a link when a named event occurs
	virtual bool					launchEventLink(const LcTaString& eventName, int stackLevel);

	// Find the path to the application folder
	static  LcTaString				findAppPath();
			LcTaString				getAppPath();

#if defined (IFX_USE_SCRIPTS)
	// Find path to batch/script execution
	static  LcTaString				findBatchPath(LcTmString& fileName);
#endif

	// Return application executable name
	virtual LcTaString				getAppName()											= 0;
	// Return path to which application can write
	virtual LcTaString				getDataPath()											= 0;

	// Display a warning pop-up
	virtual void					displayWarning(const LcTmString& text);

	// Create a generic menu instance - certain platforms may overload this method to create a
	// Platform-specific menu instance
	virtual LcTaOwner<NdhsCMenu>	getMenu();

	// Create a generic page instance - certain platforms may overload this method to create a
	// Platform-specific page instance
	virtual LcTaOwner<NdhsCPage>	getPage();

	// This will display an error and require user acknowledgment to continue.
	virtual void					displayHaltError(const LcTmString& text, bool exitApp);
	// Call when app key has been clicked to bring to foreground
	void							resetToTop		();

	void							engageAsyncLinkBlocking()			{ m_asyncLinkBlocking = true; }
	void							resetAsyncLinkBlocking()			{ m_asyncLinkBlocking = false; }
	bool							isAsyncLinkBlocking()				{ return m_asyncLinkBlocking; }
	bool							isAsyncBlockingMessageScheduled();
	virtual void					doAsyncLaunchComplete();
};

#endif
