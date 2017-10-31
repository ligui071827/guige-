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
#include "inflexionui/engine/ifxui_engine_porting.h"
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


/*-------------------------------------------------------------------------*//**
	Safe construction only
*/
NdhsCMenuCon::NdhsCMenuCon()
{
	m_messageSending		= false;
	m_callbacksEnabled		= true;
}

/*-------------------------------------------------------------------------*//**
	Destruction
*/
NdhsCMenuCon::~NdhsCMenuCon()
{
	// Disable callbacks because we are about
	// to destroy the page manager
	m_callbacksEnabled = false;

	// Clean up any existing theme
	destroyPageManager();

	m_plugin.destroy();

	// Clean up any link types.
	m_linkTypeList.clear();

}

/*-------------------------------------------------------------------------*//**
	Should be called by derived platform-specific class
*/
void NdhsCMenuCon::construct(	const LcTmString& language,
								const LcTmString& displayMode,
								const LcTmString& themeName,bool restoreSavedState)
{
	LcTaOwner<NdhsCLaundry> laundry = NdhsCLaundry::create(this);
	m_laundry = laundry;

	LcTaOwner<LcCSerializeMaster> serializeMaster;
	if(restoreSavedState)
	{
#ifdef IFX_SERIALIZATION
		serializeMaster=LcCSerializeMaster::create(NULL);
		restoreSavedState=serializeMaster->load();
#endif /* IFX_SERIALIZATION */
	}

	// Create the page manager
	LcTaOwner<NdhsCPageManager> newPageManager = createPageManager(language, displayMode, themeName);
	m_pageManager = newPageManager;
	m_tokenStack  = m_pageManager->getTokenStack();
	m_pageManager->realize(m_space.ptr());

	// Finally apply the first theme
	LcTaString err;
	bool loadAppConfigFromXml = false;
	if(restoreSavedState)
	{
#ifdef IFX_SERIALIZATION
		loadAppConfigFromXml = m_pageManager->loadAppConfigFromState(err,serializeMaster.ptr());
		if(loadAppConfigFromXml)
			m_pageManager->restoreState(serializeMaster.ptr());
#endif /* IFX_SERIALIZATION */
	}
	else
	{
		loadAppConfigFromXml = m_pageManager->loadAppConfigFromXml(err);
	}
	if (loadAppConfigFromXml)
	{
		// Start the main program
#if !defined(NDHS_PREVIEWER) && !defined(NDHS_JNI_INTERFACE) // Delay the theme applying in the previewer
		m_pageManager->applyFirstTheme();
#endif
	}
	else
	{
		if (err.isEmpty())
		{
			err = "The ";
			err += NDHS_APP_SETTINGS_FILENAME;
			err += " file is invalid";
		}
		displayHaltError(err, true);
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPlugin* NdhsCMenuCon::addPlugin()
{
	NdhsCPlugin* newPlugin = getPlugin();

	if (newPlugin)
	{
		// Extract the plugin link types and add them to our list.
		int numLinks = newPlugin->getLinkTypeCount();
		if (numLinks > 0)
		{
			// Iterate the linktypes from the dll.
			for (int i = 0; i < numLinks; i++)
			{
				LcTaString linkPrefix = "";
				ENdhsLinkType linkType = newPlugin->getLinkTypeData(i, linkPrefix);

				// Add the linktype data to the linkType list.
				if (linkType != ENdhsLinkTypeUnknown &&
					linkPrefix.length() > 0)
				{
					TLinkTypeData newLinkData;
					newLinkData.m_plugin	= newPlugin;
					newLinkData.m_linkType	= linkType;

					m_linkTypeList[linkPrefix.toLower()] = newLinkData;
				}
			}
		}
	}

	return newPlugin;
}

/*-------------------------------------------------------------------------*//**
	Manages the plugin cache.
*/
NdhsCPlugin* NdhsCMenuCon::getPlugin()
{
	NdhsCPlugin* pPlugin = NULL;

	if (m_plugin)
	{
		pPlugin = m_plugin.ptr();
	}
	else
	{
		pPlugin = loadPlugin();
	}

	return pPlugin;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT NdhsCPlugin* NdhsCMenuCon::loadPlugin()
{
	// Plugin not cached, so load it
	LcTaOwner<NdhsCPlugin> pPlugin = NdhsCPlugin::create(this);
	if (pPlugin)
	{
		if (pPlugin->initialize())
		{
			NdhsCPlugin* pPluginPtr = pPlugin.ptr();
			addPluginToList(pPlugin);
			return pPluginPtr;
		}
	}

	// Failure
	return NULL;
}

/*-------------------------------------------------------------------------*//**
	This will return the linkType first looking at the linkType table then
	checking the built in link types.
*/
ENdhsLinkType NdhsCMenuCon::getTRLinkType(const LcTmString& linkName)
{
	// return nothing if no link name.
	if (linkName.length() == 0)
		return ENdhsLinkTypeUnknown;

	// Detect the link types and return its type.
	ENdhsLinkType linkType = ENdhsLinkTypeUnknown;

	if (linkName.compareNoCase("menu") == 0)
	{
		// Menu
		linkType = ENdhsLinkTypeMenu;
	}
	if (linkName.compareNoCase("page") == 0 || linkName.compareNoCase("node") == 0)
	{
		// Page
		linkType = ENdhsLinkTypePage;
	}
	else if (linkName.compareNoCase("theme") == 0)
	{
		// Theme
		linkType = ENdhsLinkTypeTheme;
	}
	else
	{
		// Search the plugin link types to see if it is one of them.
		TmMLinkTypeList::iterator pos;
		pos = m_linkTypeList.find(linkName.toLower());
		if (pos != m_linkTypeList.end())
			linkType = pos->second.m_linkType;

	}

	return linkType;
}

/*-------------------------------------------------------------------------*//**
	Get the plugin pointer for a particular link prefix.
*/
NdhsCPlugin* NdhsCMenuCon::getTRLinkTypePlugin(const LcTmString& linkName)
{
	NdhsCPlugin* retVal = NULL;

	// returning nothing if linkName empty.
	if (linkName.length() > 0)
	{
		// Search the plugin link types to see if it is one of them.
		TmMLinkTypeList::iterator pos;
		pos = m_linkTypeList.find(linkName.toLower());
		if (pos != m_linkTypeList.end())
			retVal = pos->second.m_plugin;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Destroy current theme
*/
void NdhsCMenuCon::destroyPageManager()
{
	// Destroy existing theme if there is one
	if (m_pageManager)
	{
		m_tokenStack = NULL;

		// Ensure space does not try to forward key events to
		// the applet that we are about to destroy
		m_space->setFocus(NULL);

		// Destroy page manager
		m_pageManager->retire();
		m_pageManager.destroy();
	}
}

/*-------------------------------------------------------------------------*//**
	Called when app deliberately brought to top via key press
*/
void NdhsCMenuCon::resetToTop()
{
	m_pageManager->resetThemeToTop();
	m_space->repaint(true);
}

/*-------------------------------------------------------------------------*//**
	Launch the menu item specified
*/
bool NdhsCMenuCon::launchMenuItem(NdhsCMenuItem* mi, int stackLevel)
{
	bool		result	= false;
	LcTaString	err		= "";

	// Now we process any tokens in the menu item's link data
	// before execution (the replacement values could be
	// global settings, or they could have been set by the
	// user as a result of this item opening)
	// For all link types, the data is given in URL format
	ENdhsLinkType	linkType	= getTRLinkType(mi->getTRLinkPrefix(m_tokenStack, stackLevel));
	LcTaString		link	    = mi->getTRLink(m_tokenStack, stackLevel);

	switch (linkType)
	{
		case ENdhsLinkTypeMenuPlugin:
		case ENdhsLinkTypeSyncLinkPlugin:
		case ENdhsLinkTypeAsyncLinkPlugin:
		{
			NdhsCPlugin* linkPlugin = getTRLinkTypePlugin(mi->getTRLinkPrefix(m_tokenStack, stackLevel));

			if (linkPlugin)
			{
				result = linkPlugin->launchLink(link);

				// Asynchronous calls block the product.
				if (!result && (linkType == ENdhsLinkTypeAsyncLinkPlugin))
					resetAsyncLinkBlocking();
			}

			if (!result)
			{
				err = "Unable to launch the link";
			}
		}
		break;

		default:
			err = "Invalid action";
		break;
	}

	// Finally handle failures
	if (!result)
	{
		// Display a warning message
		displayWarning(err);
	}

	return result;
}

/*-------------------------------------------------------------------------*//**
	Launches a link when a named event occurs
*/
bool NdhsCMenuCon::launchEventLink(const LcTaString& eventName, int stackLevel)
{
	bool result = false;

	LcTaString linkAttr;
	m_tokenStack->getValue("event_" + eventName + "_link", linkAttr, stackLevel);

	if (linkAttr.length() > 0)
	{
		LcTaOwner<NdhsCMenuItem> miRef;
		miRef = NdhsCMenuItem::create(NULL);

		miRef->setLinkAttr(linkAttr);

		launchMenuItem(miRef.ptr(), stackLevel);

		result = true;
	}
	else
	{
		// No link specified, so nothing to do
		result = true;
	}

	return result;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuCon::launchLink(NdhsCMenuItem* item, int stackLevel)
{
	return launchMenuItem(item, stackLevel);
}

/*-------------------------------------------------------------------------*//**
*/
ENdhsWidgetType NdhsCMenuCon::getWidgetType(const LcTmString& data)
{
	// Get file extension from icon attribute (if any)
	LcTaString ext = data.getWord(-1, '.');
	if (ext.length() == 0)
		return ENdhsWidgetTypeUnknown;

	// Work out the icon type
#if defined(LC_USE_MESHES)
	if ((ext.compareNoCase("dae") == 0) || (ext.compareNoCase("nd3") == 0))
	{
		// 3D object
		return ENdhsWidgetTypeColoredMesh;
	}
	else
#endif
	{
		// Image
		return ENdhsWidgetTypeBitmap;
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<NdhsCMenu> NdhsCMenuCon::getMenu()
{
	return LcTaOwner<NdhsCMenu>(NdhsCMenu::create(	this,
													getPageManager()->getPlugin(),
													getPageManager()->getCurrentLanguage(),
													getPageManager()->getCurrentScreenMode()));
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<NdhsCPage> NdhsCMenuCon::getPage()
{
	return LcTaOwner<NdhsCPage>(NdhsCPage::create(	this, getPageManager() ));
}

/*-------------------------------------------------------------------------*//**
	Display a warning alert.
*/
LC_EXPORT void NdhsCMenuCon::displayWarning(const LcTmString& text)
{
	IFXP_Error_Print(text.bufWChar());
}
/*-------------------------------------------------------------------------*//**
	Display an error and waits for the user to acknowledge it.
*/
LC_EXPORT void NdhsCMenuCon::displayHaltError(const LcTmString& text, bool exitApp)
{
#if defined(IFX_MEMORYTEST_STARTUP)
	// We dont want to exit
	return;
#endif
	IFXP_Display_Error_Note(text.bufWChar());

	if (exitApp)
		this->exitApp();
	else
		getSpace()->revalidate();
}

/*-------------------------------------------------------------------------*//**
	Returns the drive that the specified file is on.
*/
LcTaString NdhsCMenuCon::getDriveLetter(const LcTmString& fileName, const LcTmString& path)
{
	LcTaString drive = "";

	LcTaString driveList = IFX_BASE_PACKAGE_DRIVES;

	while (driveList.length())
	{
		// Check that we are in the comma separated list.
		if (driveList[0] >= 'A' && driveList[0] <= 'Z')
		{
			LcTaString currentDrive = driveList.subString(0, 1) + ":" + NDHS_DIR_SEP;
			// Try to open the settings file.
			LcTaOwner<LcCReadOnlyFile> file = LcCReadOnlyFile::openFile(currentDrive + path + fileName);
			if (file.ptr() != NULL)
			{
				// File exists on this drive
				file->close();

				return currentDrive;
			}
		}

		// Remove all up to and including the next ','
		int pos = driveList.find(",");
		if (pos > 0)
		{
			driveList = driveList.tail(pos+1);
		}
		else
		{
			driveList = driveList.tail(1);
		}
	}

	return drive;
}

/*-------------------------------------------------------------------------*//**
	Work out the app path based on IFX_BASE_PACKAGE_DRIVES.
	This is used by ifxui_engine to gain access to settings.xml early on.
	It is also used by getAppPath below. The result can not be cached inside
	this function as static data because the NGD uses multiple instances
	with different path values.
	As a result, getAppPath must not be overloaded otherwise the path data
	returned will be different between the static and overloaded versions.
*/
LcTaString NdhsCMenuCon::findAppPath()
{
	LcTaString appPath = "";

	LcTaString driveList = IFX_BASE_PACKAGE_DRIVES;

	while (driveList.length())
	{
		// Check that we are in the comma separated list.
		if (driveList[0] >= 'A' && driveList[0] <= 'Z')
		{
			LcTaString currentDrive = driveList.subString(0, 1) + ":" + NDHS_DIR_SEP;
			// Try to open the settings file.
			LcTaOwner<LcCReadOnlyFile> file = LcCReadOnlyFile::openFile(currentDrive + NDHS_APP_SETTINGS_FILENAME);
			if (file.ptr() != NULL)
			{
				// File exists on this drive
				file->close();
				appPath = currentDrive;
				break;
			}
		}

		// Remove all up to and including the next ','
		int pos = driveList.find(",");
		if (pos > 0)
		{
			driveList = driveList.tail(pos+1);
		}
		else
		{
			driveList = driveList.tail(1);
		}
	}

	return appPath;
}

#if defined (IFX_USE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
	Work out the batch/script path based on IFX_BASE_PACKAGE_DRIVES.
	This is used by batch/script execution.
*/
LcTaString NdhsCMenuCon::findBatchPath(LcTmString& fileName)
{
	LcTaString appPath = "";

	LcTaString driveList = IFX_BASE_PACKAGE_DRIVES;

	while (driveList.length())
	{
		// Check that we are in the comma separated list.
		if (driveList[0] >= 'A' && driveList[0] <= 'Z')
		{
			LcTaString currentDrive = driveList.subString(0, 1) + ":" + NDHS_DIR_SEP;
			// Try to open the settings file.
			LcTaOwner<LcCReadOnlyFile> file = LcCReadOnlyFile::openFile(currentDrive + fileName);
			if (file.ptr() != NULL)
			{
				// File exists on this drive
				file->close();
				appPath = currentDrive;
				break;
			}
		}

		// Remove all up to and including the next ','
		int pos = driveList.find(",");
		if (pos > 0)
		{
			driveList = driveList.tail(pos+1);
		}
		else
		{
			driveList = driveList.tail(1);
		}
	}

	return appPath;
}

#endif
/*-------------------------------------------------------------------------*//**
	Cached class member version of the app path.
*/
LcTaString NdhsCMenuCon::getAppPath()
{
	if (m_appPath.isEmpty())
		m_appPath = NdhsCMenuCon::findAppPath();

	return m_appPath;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuCon::isAsyncBlockingMessageScheduled()
{
	if (m_pageManager)
		return m_pageManager->isAsyncBlockingMessageScheduled();
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuCon::doAsyncLaunchComplete()
{
	if (m_pageManager)
		m_pageManager->doAsyncLaunchComplete();
}