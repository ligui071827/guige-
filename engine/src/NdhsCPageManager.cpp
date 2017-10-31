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
#include "inflexionui/engine/ifxui_uriparser.h"
#include "inflexionui/engine/inc/NdhsCEntryPointMapStack.h"

#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


#include <stdlib.h>

#define NDHS_MAIN_PACKAGE_NAME					"main/"
#define NDHS_FOLDER_SEPARATOR_CHAR				'_'

// Defines for configuration message
#define NDHS_CONFIG_THEME_PACKAGE				0x01
#define NDHS_CONFIG_DISPLAY_MODE				0x02
#define NDHS_CONFIG_LANGUAGE					0x04
#define NDHS_CONFIG_ENTRYPOINT_ID				0x08

#ifdef NDHS_PREVIEWER
	#include "NDHS Theme Previewer.h"
	#include "NDHS Theme PreviewerDlg.h"
	#include "inflexionui/engine/inc/NdhsCLog.h"
	#include "NDHS Theme LogHtmlDlg.h"
#endif
#ifdef NDHS_JNI_INTERFACE
	#include "inflexionui/engine/inc/NdhsCLog.h"
#endif

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager> NdhsCPageManager::create(	const LcTmString& appName,
														NdhsCMenuCon* con,
														const LcTmString& language,
														const LcTmString& screenSize,
														const LcTmString& themeName)
{
	LcTaOwner<NdhsCPageManager> ref;
	ref.set(new NdhsCPageManager(con));
	ref->construct(appName, language, screenSize, themeName);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageManager::NdhsCPageManager(NdhsCMenuCon* con)
							:	m_eventListHead(NULL),
								m_eventListTail(NULL),
								m_destroyUnwantedPagesMessage(this, EDestroyUnwantedPagesMsg),
								m_openThemeMessage(this, EOpenThemeMsg),
								m_configureExternalMessage(this, EConfigureExternalMsg),
								m_eventMessage(this, EEventMsg),
								m_launchLinkMessage(this, ELaunchLinkMsg),
								m_simulateKeyMessage(this, ESimulateKeyMsg),
								m_postTransitionCompleteMessage(this, EPostTransitionCompleteMsg),
								m_changeBackgroundMessage(this, EChangeBackgroundMsg),
								m_asynchronousLaunchCompleteMessage(this, EAsynchronousLaunchCompleteMsg)
#ifdef LC_USE_MOUSEOVER
								,m_doMouseOverHitTestMessage(this, EDoMouseOverHitTest)
#endif
{
	m_con = con;
	m_plugin = NULL;

	m_activePage = -1;

	m_isLaunchingMenuItem = true;
	m_paletteMap.clear();

	m_terminalTime = 0;
	m_terminalVelocityProfile = ENdhsVelocityProfileUnknown;

	m_action = ENoAction;

	m_nestedComponentLevel = NESTED_COMPONENT_LEVEL;
	m_isCyclic = false;

	m_bIsAsynchronousLaunchCompleteMessagePending = false;

#ifdef LC_USE_STYLUS
	m_mouseState = EMouseUp;
#endif

#ifdef LC_USE_MOUSEOVER
	// Initially contain dummy values
	m_mouseOverCurrentPt.x = -999999;
	m_mouseOverCurrentPt.y = -999999;
	m_doMouseOverHitTesting = false;
#endif

#if defined(NDHS_JNI_INTERFACE)
	// identifier will be used in 'in-place edit' mode, to
	// set the unique identifier for a component, and when
	// we will go for restoring state we will use this to select
	// layout in state manager, so by using this we can reach to
	// exact component, with out matching each and every attribute
	m_identifier = 2;
#endif
}

/*-------------------------------------------------------------------------*//**
	Initialize and create widgets
*/
void NdhsCPageManager::construct(	const LcTmString& appName,
									const LcTmString& language,
									const LcTmString& screenSize,
									const LcTmString& themeName)
{
	// Make a copy of the initial values.
	m_language		= "";
	m_screenSize	= "";
	m_designSize	= "";
	m_reqThemeName 	= "";

	m_language		= language.toLower();
	m_screenSize	= screenSize.toLower();
	m_reqThemeName  = themeName.toLower();

	LcCApplet::construct(appName);

	// Create the manifest stack.
	LcTaOwner<NdhsCManifestStack> newManifestStack = NdhsCManifestStack::create(this);
	m_manifestStack = newManifestStack;

	// Create the token stack.
	LcTaOwner<NdhsCTokenStack> newTokenStack = NdhsCTokenStack::create(m_language, m_screenSize);
	m_tokenStack    = newTokenStack;

	// Create the launchable menu item.
	LcTaOwner<NdhsCMenuItem> newLaunchItem = NdhsCMenuItem::create(NULL);
	m_launchItem    = newLaunchItem;

	// Create the entry point map stack
	LcTaOwner<NdhsCEntryPointMapStack> newEntryPointStack = NdhsCEntryPointMapStack::create();
	m_entryPointStack = newEntryPointStack;

	m_dataPath      = m_con->getDataPath();
	m_iniTokenFile  = m_con->getAppName() + "_Ini.xml";
	m_usrTokenFile  = m_con->getAppName() + "_Usr.xml";

	LcTaOwner<LcCMutex> newMutex = m_con->getSpace()->createMutex(LcTaString("IFX_EventQueue"));
	m_eventQueueMutex = newMutex;

	LcTaOwner<NdhsCPath> newPath = NdhsCPath::create();
	m_primaryLightPath = newPath;
	m_primaryLightOverride = false;

	// Create the terminal state change field
	LcTaOwner<NdhsCScrollPosField> tempPos = NdhsCScrollPosField::create(m_con, NULL, 0.0f, 1.0f, false, 0);
	m_primaryLightPos = tempPos;
	m_primaryLightPos->setIgnoreLaundry(true);

#ifdef IFX_GENERATE_SCRIPTS
	if (NdhsCScriptGenerator::getInstance())
	{
		m_scriptGenerator = NdhsCScriptGenerator::getInstance();
		if (m_scriptGenerator)
		{
			m_scriptGenerator->setPageManager(this);
#ifndef NDHS_JNI_INTERFACE
			m_scriptGenerator->startScriptGeneration(false);
#endif
		}
	}
#endif //IFX_GENERATE_SCRIPTS


#ifdef IFX_USE_SCRIPTS
	if (NdhsCScriptExecutor::getInstance())
	{
		m_scriptExecutor = 	NdhsCScriptExecutor::getInstance();
		if (m_scriptExecutor)
			m_scriptExecutor->setPageManager(this);
	}
#endif //IFX_USE_SCRIPTS

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	m_effect = NdhsCEffect::create(this->getCon()->getSpace());
	m_effect->configureDefaultEffectsFromXml();
#endif	/* defineed(IFX_RENDER_DIRECT_OPENGL_20) */

}

#ifdef IFX_SERIALIZATION

bool NdhsCPageManager::saveState(int *abortSave)
{
	volatile int memError=0;
	LC_CLEANUP_PUSH_FRAME(memError);
	LcTaOwner<LcCSerializeMaster> serializeM=LcCSerializeMaster::create(abortSave);
	serialize(serializeM.ptr());
	if(serializeM->serializeBreadthFirst())
	{
		serializeM->flush();
	}
	LC_CLEANUP_POP_FRAME(memError);
	return true;
}

bool NdhsCPageManager::restoreState(LcCSerializeMaster *serializeMaster)
{
	deSerialize(0,serializeMaster);
	m_con->updateScreenFurniture();
	setMatchedBackground(m_pageStack[m_activePage]->getStackLevel());
	serializeMaster->deflate();
	for (int i = m_activePage; i >= 0; --i)
	{
		m_pageStack[i]->onResume();
		m_pageStack[i]->realize(this);
	}
	onTransitionComplete(false);

#ifdef IFX_USE_PLUGIN_ELEMENTS
	if(serializeMaster->getFullScreenElement()!=NULL)
	{
		makeFullScreen((IFX_HELEMENT)serializeMaster->getFullScreenElement(),true);
	}
#endif

	if(m_activePage>=0 && m_pageStack[m_activePage]->getPageState()==ENdhsPageStateLaunch)
		scheduleAsynchronousLaunchComplete();
	return true;
}

SerializeHandle	NdhsCPageManager::serialize(LcCSerializeMaster *serializeMaster,bool force)
{
	SerializeHandle handle=-1;
	if(!force)
	{
		handle=serializeMaster->getHandle(this);
		if(handle!=-1 && serializeMaster->isSerialized(handle))
		{
			return handle;
		}
		else if(handle==-1)
		{
			handle=serializeMaster->newHandle(this);
		}
	}
	else
	{
		handle=serializeMaster->newHandle(this);
	}

	int templateCount=m_templates.size();
	int nodeCount=m_nodes.size();
	int menuCount=m_menus.size();
	int paletteCount=m_paletteMap.size();
	int outputSize = sizeof(NdhsCPageManager)+sizeof(IFX_INT32)*10+sizeof(IFX_INT32)*7*(templateCount+nodeCount+menuCount+paletteCount);
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SERIALIZE_String(m_designSize,serializeMaster,cPtr)
	SERIALIZE_String(m_screenSize,serializeMaster,cPtr)
	SERIALIZE_String(m_language,serializeMaster,cPtr)
	SERIALIZE_Owner(m_manifestStack,serializeMaster,cPtr);

	SerializeHandle h=-1;
	h=serializeMaster->reserveHandle((NdhsCPlugin*)m_plugin);
	SERIALIZE(h,serializeMaster,cPtr);
	serializeMaster->setData(h,0,NULL);

	h=serializeMaster->reserveHandle((NdhsCMenuCon*)m_con);
	SERIALIZE(h,serializeMaster,cPtr);
	serializeMaster->setData(h,0,NULL);

	SERIALIZE_String(m_dataPath,serializeMaster,cPtr)
	SERIALIZE_String(m_mainMenuLink,serializeMaster,cPtr)
	SERIALIZE_String(m_defaultTheme,serializeMaster,cPtr)
	SERIALIZE_String(m_reqLanguage,serializeMaster,cPtr)
	SERIALIZE_String(m_reqDisplayMode,serializeMaster,cPtr)
	SERIALIZE_String(m_reqThemeName,serializeMaster,cPtr)
	SERIALIZE(m_bIsAsynchronousLaunchCompleteMessagePending,serializeMaster,cPtr)
	SERIALIZE(m_tokenStack,serializeMaster,cPtr);

	LcTaArray<CTemplateItem*> templateSortedArray;
	for(TmMPageTemplateCache::iterator iter=m_templates.begin();iter!=m_templates.end();iter++)
	{
		ETemplateType type=iter->second->templateFile->getTemplateType();
		if(type==ETemplateTypeInvalid || type==ETemplateTypePage)
			templateSortedArray.push_back(iter->second);
	}
	for(TmMPageTemplateCache::iterator iter=m_templates.begin();iter!=m_templates.end();iter++)
	{
		ETemplateType type=iter->second->templateFile->getTemplateType();
		if(type==ETemplateTypeComponent || type==ETemplateTypeMenuComponent)
			templateSortedArray.push_back(iter->second);
	}

	SERIALIZE(templateCount,serializeMaster,cPtr);
	for(LcTaArray<CTemplateItem*>::iterator iter=templateSortedArray.begin();iter!=templateSortedArray.end();iter++)
	{
		CTemplateItem *item =*iter;
		LcTaString key=item->key;
		SERIALIZE_String(key,serializeMaster,cPtr)
		SERIALIZE(item->refCount,serializeMaster,cPtr)
		SERIALIZE(item->stackLevel,serializeMaster,cPtr)
		h=serializeMaster->reserveHandle((NdhsCTemplate*)item->templateFile.ptr());
		SERIALIZE(h,serializeMaster,cPtr)
		serializeMaster->setData(h,0,NULL);
	}

	SERIALIZE(nodeCount,serializeMaster,cPtr);
	for(TmANodeCache::iterator iter=m_nodes.begin();iter!=m_nodes.end();iter++)
	{
		LcTaString key=iter->first;
		SERIALIZE_String(key,serializeMaster,cPtr)
		CNodeItem *item =iter->second;
		SERIALIZE(item->refCount,serializeMaster,cPtr)
		SERIALIZE(item->stackLevel,serializeMaster,cPtr)
		h=serializeMaster->reserveHandle((NdhsCPage*)item->uiNode.ptr());
		SERIALIZE(h,serializeMaster,cPtr)
		serializeMaster->setData(h,0,NULL);
	}

	SERIALIZE(paletteCount,serializeMaster,cPtr);
	for(TmMPaletteCache::iterator iter=m_paletteMap.begin();iter!=m_paletteMap.end();iter++)
	{
		LcTaString key=iter->first;
		SERIALIZE_String(key,serializeMaster,cPtr)
		CPaletteItem *item =iter->second;
		SERIALIZE(item->refCount,serializeMaster,cPtr)
		SERIALIZE_Reserve(item->paletteManifest.ptr(),serializeMaster,cPtr)
	}

	SERIALIZE(menuCount,serializeMaster,cPtr);
	for(TmMMenuCache::iterator iter=m_menus.begin();iter!=m_menus.end();iter++)
	{
		LcTaString key=iter->first;
		SERIALIZE_String(key,serializeMaster,cPtr)
		CMenuItem *item =iter->second;
		SERIALIZE(item->stackLevel,serializeMaster,cPtr)
		SERIALIZE_String(item->menu->getMenuLink(),serializeMaster,cPtr)
		SERIALIZE_Reserve(item->paletteManifest,serializeMaster,cPtr)
		NdhsCPlugin::NdhsCPluginMenu *pluginMenu=item->menu->getMenuPlugin();
		h=serializeMaster->reserveHandle(item->menu.ptr());
		SERIALIZE(h,serializeMaster,cPtr)
		serializeMaster->setData(h,0,NULL);
		h=serializeMaster->reserveHandle(pluginMenu);
		SERIALIZE(h,serializeMaster,cPtr)
		serializeMaster->setData(h,0,NULL);
	}

	SERIALIZE(m_nestedComponentLevel,serializeMaster,cPtr)
	SERIALIZE(m_primaryLightOverride,serializeMaster,cPtr)
	SERIALIZE(m_isLaunchingMenuItem,serializeMaster,cPtr)
	SERIALIZE(m_action,serializeMaster,cPtr)
	SERIALIZE(m_activePage,serializeMaster,cPtr)
#ifdef LC_USE_MOUSEOVER
	SERIALIZE(m_currentMouseOverTestElement,serializeMaster,cPtr)
#endif
	SERIALIZE(m_animatingStateChange,serializeMaster,cPtr)
	SERIALIZE_String(m_lastGoodThemeName,serializeMaster,cPtr)
	SERIALIZE(m_primaryLightDuration,serializeMaster,cPtr)
	SERIALIZE(m_primaryLightOverrideTransitonStarted,serializeMaster,cPtr)
	SERIALIZE(m_primaryLightOverrideTransitonEnded,serializeMaster,cPtr)
	SERIALIZE_Placement(m_suspendPrimaryLightPlacement,serializeMaster,cPtr)
	h=-1;
	h=serializeMaster->serialize(m_pageStack, serializeMaster);
	SERIALIZE(h,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}


bool NdhsCPageManager::loadAppConfigFromState(LcTmString& errorString,LcCSerializeMaster *serializeMaster)
{
	SerializeHandle pageManagerHandle=0;	// handle 0 of pagemanager
	SerializeHandle h=-1;
	void * ptr=serializeMaster->getOffset(pageManagerHandle);
	serializeMaster->setPointer(pageManagerHandle,this);
	void *cPtr=ptr;
	// Push the manifest first
	LcTaString manifest = m_con->getAppPath() + NDHS_MANIFEST_FILENAME;
	LcTaString package = NDHS_MAIN_PACKAGE_NAME;
	LcTaString		savedDesignSize="";
	LcTaString		savedLanguage="";
	LcTaString		savedScreen="";
	DESERIALIZE_String(savedDesignSize,serializeMaster,cPtr)
	DESERIALIZE_String(savedScreen,serializeMaster,cPtr)
	DESERIALIZE_String(savedLanguage,serializeMaster,cPtr)
	if (m_manifestStack->pushManifest(manifest, package))
	{

		DESERIALIZE(h,serializeMaster,cPtr)
		serializeMaster->setPointer(h,((NdhsCManifestStack*)m_manifestStack.ptr()));
		m_manifestStack->deSerialize(h,serializeMaster);

#ifdef IFX_VALIDATE_PROJECTS
		// Don't validate signatures in debug builds
		// Note that we don't check the version number of app config files,
		// since these files cannot be separately installed like themes are
		NdhsCProjectValidator pv;
		int error = pv.loadProject(m_manifestStack->getManifest(m_manifestStack->getStackHeight() - 1));
		if ((error != NdhsCProjectValidator::ESuccess) || (pv.validateProject() == false))
		{
			m_manifestStack->popManifest(1);
			errorString = "The ";
			errorString += m_con->getAppPath();
			errorString += NDHS_MANIFEST_FILENAME;
			// Convert all the forward slashes to back slashes if required.
			#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
				errorString.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
			#endif
			errorString += " file validation is incorrect";
			return false;
		}
#endif
	}

	// App-specific default settings and look and feel settings are read
	// from "[appName].xml" in "[appPath]" folder.
	// This is hard coded in version 1.0
	LcTaString err;
	// Set the directory slash separators to the non default if required.
	LcTaString localPath = m_con->getAppPath() + NDHS_APP_SETTINGS_FILENAME;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif
	LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(localPath, err);
	if (root)
	{
		LcTaString guid = "";

		//
		// Get the settings section
		//

		LcCXmlElem* settings = root->find("settings");
		if (settings)
		{
			// Check for integrated mode
			bool integrated = LcCXmlAttr::strToBool(settings->getAttr("integrated", "true"));
			m_con->setIntegrated(integrated);

			m_defaultTheme = settings->getAttr("defaultTheme");

			LcCXmlAttr* a = settings->findAttr("GUID");
			if (!a)
			{
				// Trace on error
				NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - GUID missing");

				return false;
			}

			guid = a->getVal();

			// Load the entry point map

			LcCXmlElem* elemEntryPointMap = settings->find("entryPointMap");

			if (elemEntryPointMap)
			{
				LcCXmlElem* entryPoint = elemEntryPointMap->getFirstChild();

				for (; entryPoint; entryPoint = entryPoint->getNext())
				{
					LcTaString id = entryPoint->getAttr("entryPointID").toLower();
					LcTaString uri = entryPoint->getAttr("nodeUri").toLower();

					// check we got valid id and uri
					if (id.isEmpty() == false && uri.isEmpty() == false)
					{
						m_entryPointStack->pushMap(id, uri);
					}
				}
			}

			// Load the font settings

			LcCXmlElem* elemFont = settings->find("font");
			if (elemFont)
			{
				LcCXmlAttr* attr = elemFont->findAttr("face");
				if (attr)
					m_defaultFontFace = NdhsCExpression::CExprSkeleton::create(attr->getVal());

				attr = elemFont->findAttr("style");
				if (attr)
					m_defaultFontStyle = LcCXmlAttr::strToFontStyle(attr->getVal());

				attr = elemFont->findAttr("color");
				if (attr)
					m_defaultFontColor = attr->getVal();
			}
		}
		else
		{
			// Trace on error
			NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - settings section missing");

			return false;
		}

		//
		// Get the design sizes section
		//

		LcCXmlElem* designSizes = root->find(NDHS_TP_XML_DESIGN_SIZES);
		if (designSizes)
		{
			LcCXmlElem* designSize = designSizes->getFirstChild();

			for (; designSize; designSize = designSize->getNext())
			{
				if (designSize->getName().compareNoCase(NDHS_TP_XML_DESIGN_SIZE) == 0)
				{
					TDesignSize ds;

					LcTaString name = designSize->getAttr(NDHS_TP_XML_NAME).toLower();
					ds.width		= designSize->getAttr(NDHS_TP_XML_WIDTH, "-1").toInt();
					ds.origin		= designSize->getAttr(NDHS_TP_XML_ORIGIN, "-1").toInt();

					if (name.length() != 0 && ds.width != -1 && ds.origin != -1)
					{
						m_designSizes[name] = ds;
					}
				}
			}
		}
		else
		{
			// Trace on error
			NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - no design size specified");

			return false;
		}

		//
		// Get the displays section
		//

		LcCXmlElem* displays = root->find("displays");
		if (displays)
		{
			LcCXmlElem* display = displays->getFirstChild();

			for (; display; display = display->getNext())
			{
				if (display->getName().compareNoCase("display") == 0)
				{
					LcTaOwner<CDisplay> disp = CDisplay::create();

					LcTaString name = display->getAttr("name").toLower();

					if (name.length() == 0)
					{
						disp.destroy();
					}
					else
					{
						LcCXmlElem* displayMode = display->getFirstChild();

						for (; displayMode; displayMode = displayMode->getNext())
						{
							if (displayMode->getName().compareNoCase("mode") == 0)
							{
								LcTaOwner<CDisplay::CDisplayMode> dm = CDisplay::CDisplayMode::create();

								LcTaString name = displayMode->getAttr("name").toLower();
								dm->width		= displayMode->getAttr("width", "-1").toInt();
								dm->height		= displayMode->getAttr("height", "-1").toInt();
								dm->designSize  = displayMode->getAttr("designSize").toLower();
								dm->defaultMenu = displayMode->getAttr("defaultEntryPointID");
								dm->defaultLink = displayMode->getAttr("defaultLink");

								if (name.length() == 0 || dm->width == -1
									|| dm->height == -1 || dm->designSize.length() == 0)
								{
									dm.destroy();
								}
								else
								{
									disp->displayModes.add_element(name, dm);
								}
							}
						}

						m_displays.add_element(name, disp);
					}
				}
			}
		}
		else
		{
			// Trace on error
			NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - no display specified");

			return false;
		}

		//
		// Get the keys section
		//

		LcCXmlElem* keys = root->find("keys");
		if (keys)
		{
			LcCXmlElem* key = keys->getFirstChild();

			for (; key; key = key->getNext())
			{
				if (key->getName().compareNoCase("key") == 0)
				{
					int code = -1;

				#ifdef NU_SIMULATION
					// in case of SimTest, try to find simtestCode first.
					code = key->getAttr(NDHS_TP_XML_SIMTEST_CODE, "-1").toInt();
				#endif

					if (-1 == code)
					{
						// otherwise use scanCode
						code = key->getAttr(NDHS_TP_XML_SCAN_CODE, "-1").toInt();
					}

					if (-1 != code)
					{
						// Check if key name already exists?
						LcTaString name = key->getAttr("name").toLower();
						TmMKeys::iterator iter = m_keys.find(name);

						if (iter == m_keys.end())
						{
							// add new entry
							LcTaArray<int> tempArray;
							tempArray.push_back(code);

							LcTmMap<LcTmString, LcTmArray<int> >::value_type temp(name, tempArray);

							m_keys.insert(temp);
						}
						else
						{
							iter->second.push_back(code);
						}
					}
				}
			}
		}
		// Load palette
		loadPalette(root.ptr());

		setCurrentDesignSize();
		setCurrentDefaultLinkAndMenu();

		int width = 0;
		int height = 0;

		getScreenSize(m_screenSize, width, height);

		//getSpace()->setPortraitMode(width <= height);		!!! portrait mode ?

		// Add to tokens
		m_tokenStack->pushTokens(m_con->getAppPath());

		// Add the Ini tokens
		m_tokenStack->loadIniTokens(m_dataPath, m_iniTokenFile);


		if(m_designSize.compare(savedDesignSize)!=0
			|| m_screenSize.compare(savedScreen)!=0
			|| m_language.compare(savedLanguage)!=0)
		{
			errorString="Design size or Screen size not matching with the saved size";
			return false;
		}

		//
		// Get the plugin section
		//

		m_plugin = m_con->addPlugin();
		DESERIALIZE(h,serializeMaster,cPtr)

		if (m_plugin != NULL)
		{
			m_tokenStack->setPlugin(m_plugin);
			serializeMaster->setPointer(h,((NdhsCPlugin*)m_plugin));
		}

#ifdef IFX_VALIDATE_PROJECTS
		// Validate the GUID
		if (m_plugin == NULL || m_plugin->validateGUID(guid) == false)
		{
			// Trace on error
			NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - Invalid GUID");
			errorString = "Plugin GUID validation failed.";

			return false;
		}
		else
#endif
		{
			return true;
		}
	}
	else
	{
		// Set the external error.
		errorString = err;

		// Trace on error
		NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - " + err);
	}

	return false;
}

LcTaOwner<NdhsCMenu>
NdhsCPageManager::deSerializeMenu(const LcTmString& menuLink, volatile bool& menuLoaded, NdhsCManifest* palManifest, int stackLevel)
{
	// First see if we need to load menu from page file
	LcTaOwner<NdhsCMenu> menu;
	LcTaString linkPrefix="";
	LcTaString linkBody="";

	switch(menuToLoadFrom(menuLink))
	{
		case ENdhsModule:
		case ENdhsMenuFile:
		{
			menu = loadMenuFromXML(menuLink, menuLoaded, palManifest, stackLevel);
			break;
		}
		case ENdhsOtherNode:
		{
			if(!extractLinkPrefixAndLinkBody(menuLink,linkPrefix,linkBody))
				break;

			NdhsCPage* pageNode;
			LcTaOwner<NdhsCMenuItem> mi;
			mi = NdhsCMenuItem::create(NULL);
			mi->setLinkAttr("node://" + linkBody);
			pageNode = getPage(mi.ptr(), stackLevel);
			if (pageNode)
			{
				LcTaString key= "name", value;
				if (getParamterByName(menuLink, key, value))
				{
					if (stackLevel > 0)
						stackLevel--;

					LcTaString packageName = m_manifestStack.ptr()->getManifest(stackLevel)->getPackageName();
					menu= getCon()->getMenu();
					if (menu->populateMenuFromPageFile(packageName, pageNode->getPageFile(), value) == false)
					{
						menuLoaded = false;
					}
					else
					{
						menu->setMenuLink(menuLink);
						menuLoaded = true;
					}
				}
			}
			break;
		}
		case ENdhsCurrentNode:
		{
			NDHS_TRACE(ENdhsTraceLevelError, NULL, "menu from current node should not occur");
			break;
		}
		default:
		{
			// Invalid Link
			break;
		}
	}
	return menu;
}

void NdhsCPageManager::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle h=-1;
	DESERIALIZE_String(m_designSize,serializeMaster,cPtr)
	DESERIALIZE_String(m_screenSize,serializeMaster,cPtr)
	DESERIALIZE_String(m_language,serializeMaster,cPtr)
	DESERIALIZE(h,serializeMaster,cPtr);
	DESERIALIZE(h,serializeMaster,cPtr);
	DESERIALIZE(h,serializeMaster,cPtr);
	serializeMaster->setPointer(h,m_con);
	DESERIALIZE_String(m_dataPath,serializeMaster,cPtr)
	DESERIALIZE_String(m_mainMenuLink,serializeMaster,cPtr)
	DESERIALIZE_String(m_defaultTheme,serializeMaster,cPtr)
	DESERIALIZE_String(m_reqLanguage,serializeMaster,cPtr)
	DESERIALIZE_String(m_reqDisplayMode,serializeMaster,cPtr)
	DESERIALIZE_String(m_reqThemeName,serializeMaster,cPtr)
	DESERIALIZE(m_bIsAsynchronousLaunchCompleteMessagePending,serializeMaster,cPtr)
	DESERIALIZE(h,serializeMaster,cPtr)			//token stack
	m_tokenStack->deSerialize(h,serializeMaster);

	int templateCount=0;
	int nodeCount=0;
	int paletteCount=0;
	int menuCount=0;
	int refCount=0;
	int stackLevel=0;
	int nestedComponentLevel=getNestedComponentLevel();
	DESERIALIZE(templateCount,serializeMaster,cPtr);
	for(int i=0;i<templateCount;++i)
	{
		LcTaString componentFile="";
		DESERIALIZE_String(componentFile,serializeMaster,cPtr);
		DESERIALIZE(refCount,serializeMaster,cPtr);
		DESERIALIZE(stackLevel,serializeMaster,cPtr);
		DESERIALIZE(h,serializeMaster,cPtr);
		LcTaString outComponentFile=componentFile;

		// Lets see if its been cached
		TmMPageTemplateCache::iterator it = m_templates.find(outComponentFile);
		if (it != m_templates.end())
		{
			CTemplateItem* ptItem = it->second;
			serializeMaster->setPointer(h,ptItem->templateFile.ptr());
		}
		else
		{
			LcTaString err;
			// Set the directory slash separators to the non default if required.
			#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
				componentFile.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
			#endif
			LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(componentFile, err);
			LcTaOwner<NdhsCTemplate> newComponentTemplate;

			#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
				componentFile.replace(NDHS_PLAT_DIR_SEP_CHAR, NDHS_DIR_SEP_CHAR);
			#endif

			if (root)
			{
				ETemplateType templateType=getTemplateType(root->find("settings"));
				switch(templateType)
				{
					case ETemplateTypeMenuComponent:
					{
						// Menu Component
						newComponentTemplate = NdhsCMenuComponentTemplate::create(this, root);
						break;
					}
					case ETemplateTypeComponent:
					{
						// Non menu component
						newComponentTemplate = NdhsCTemplate::create(this, root);
						break;
					}
					case ETemplateTypePage:
					case ETemplateTypeInvalid:
					{
						newComponentTemplate = NdhsCPageTemplate::create(this, componentFile);
						break;
					}
				}
				// Not cached, so create a new template
				LcTaOwner<CTemplateItem> ptItem = CTemplateItem::create();
				ptItem->templateFile = newComponentTemplate;

				if (ptItem->templateFile && ptItem->templateFile->configureFromXml(m_designSize, stackLevel, nestedComponentLevel))
				{
					CTemplateItem* ptItemPtr = ptItem.ptr();
					if(templateType==ETemplateTypePage || templateType==ETemplateTypeInvalid)
						ptItem->refCount = refCount;
					ptItemPtr->key=outComponentFile;
					ptItem->stackLevel=stackLevel;
					m_templates.add_element(outComponentFile, ptItem);
					serializeMaster->setPointer(h,ptItemPtr->templateFile.ptr());
				}
			}
		}
	}


	m_con->configureScreenCanvas(false);

	DESERIALIZE(nodeCount,serializeMaster,cPtr);
	for(int i=0;i<nodeCount;++i)
	{
		LcTaString nodeLink="";
		DESERIALIZE_String(nodeLink,serializeMaster,cPtr);
		DESERIALIZE(refCount,serializeMaster,cPtr);
		DESERIALIZE(stackLevel,serializeMaster,cPtr);
		DESERIALIZE(h,serializeMaster,cPtr);
		m_launchItem->setLinkAttr(nodeLink);
		LcTaOwner<CNodeItem> ptItem = CNodeItem::create();
		LcTaOwner<NdhsCPage> newPage = loadPage(m_launchItem.ptr(), stackLevel);
		ptItem->uiNode = newPage;
		if (ptItem->uiNode)
		{
			CNodeItem* ptItemPtr = ptItem.ptr();
			ptItem->refCount = 1;
			ptItem->stackLevel=stackLevel;
			m_nodes.add_element(nodeLink, ptItem);
			serializeMaster->setPointer(h,ptItemPtr->uiNode.ptr());
		}
	}

	DESERIALIZE(paletteCount,serializeMaster,cPtr);
	for(int i=0;i<paletteCount;++i)
	{
		LcTaString key="";
		DESERIALIZE_String(key,serializeMaster,cPtr);
		DESERIALIZE(refCount,serializeMaster,cPtr);
		DESERIALIZE(h,serializeMaster,cPtr);
		TmMPaletteCache::iterator it = m_paletteMap.find(key);
		if (it != m_paletteMap.end())
		{
			// Get the manifest.
			CPaletteItem* ptItem = it->second;
			serializeMaster->setPointer(h,ptItem->paletteManifest.ptr());
		}
		else
		{
			NdhsCManifest * palettePtr=(NdhsCManifest*)serializeMaster->getPointer(h);
			if(h!=-1 && palettePtr==NULL)
			{
				palettePtr = NdhsCManifest::loadState(h,serializeMaster);
			}

			if(palettePtr!=NULL)
			{
				LcTaOwner<CPaletteItem> ptItem = CPaletteItem::create();
				ptItem->refCount=refCount;
				ptItem->paletteManifest.set(palettePtr);
				serializeMaster->setPointer(h,ptItem->paletteManifest.ptr());
				m_paletteMap.add_element(key,ptItem);
			}
		}
	}

	DESERIALIZE(menuCount,serializeMaster,cPtr);
	for(int i=0;i<menuCount;++i)
	{
		LcTaString key="";
		LcTaString menuLink="";
		bool menuLoaded=false;
		DESERIALIZE_String(key,serializeMaster,cPtr)
		LcTaOwner<CMenuItem> item =CMenuItem::create();
		DESERIALIZE(stackLevel,serializeMaster,cPtr);
		DESERIALIZE_String(menuLink,serializeMaster,cPtr);
		DESERIALIZE(h,serializeMaster,cPtr);
		item->stackLevel=stackLevel;
		item->paletteManifest=(NdhsCManifest*)serializeMaster->getPointer(h);
		item->menu=deSerializeMenu(menuLink,menuLoaded,item->paletteManifest,stackLevel);
		if(menuLoaded)
		{
			NdhsCPlugin::NdhsCPluginMenu *pluginMenu=NULL;
			pluginMenu=item->menu->getMenuPlugin();
			DESERIALIZE(h,serializeMaster,cPtr);
			serializeMaster->setPointer(h,item->menu.ptr());
			m_menus.add_element(key,item);
			DESERIALIZE(h,serializeMaster,cPtr)	// plugin menu
			serializeMaster->setPointer(h,pluginMenu);
		}
		else
		{
			DESERIALIZE(h,serializeMaster,cPtr)
			DESERIALIZE(h,serializeMaster,cPtr)
		}
	}

	DESERIALIZE(m_nestedComponentLevel,serializeMaster,cPtr)
	DESERIALIZE(m_primaryLightOverride,serializeMaster,cPtr)
	DESERIALIZE(m_isLaunchingMenuItem,serializeMaster,cPtr)
	DESERIALIZE(m_action,serializeMaster,cPtr)
	DESERIALIZE(m_activePage,serializeMaster,cPtr)
#ifdef LC_USE_MOUSEOVER
	DESERIALIZE(m_currentMouseOverTestElement,serializeMaster,cPtr)
#endif
	DESERIALIZE(m_animatingStateChange,serializeMaster,cPtr)
	DESERIALIZE_String(m_lastGoodThemeName,serializeMaster,cPtr)
	DESERIALIZE(m_primaryLightDuration,serializeMaster,cPtr)
	DESERIALIZE(m_primaryLightOverrideTransitonStarted,serializeMaster,cPtr)
	DESERIALIZE(m_primaryLightOverrideTransitonEnded,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_suspendPrimaryLightPlacement,serializeMaster,cPtr)
	DESERIALIZE(h,serializeMaster,cPtr);		//pageStack
	serializeMaster->deSerialize(m_pageStack,h,serializeMaster);
}
#endif /* IFX_SERIALIZATION */
/*-------------------------------------------------------------------------*//**
*/
NdhsCPageManager::~NdhsCPageManager()
{
	savePersistentData();

	if	(m_destroyUnwantedPagesMessage.isScheduled())
	{
		m_destroyUnwantedPagesMessage.cancel();
	}

	if (m_openThemeMessage.isScheduled())
	{
		m_openThemeMessage.cancel();
	}

	if (m_configureExternalMessage.isScheduled())
	{
		m_configureExternalMessage.cancel();
	}

	if (m_launchLinkMessage.isScheduled())
	{
		m_launchLinkMessage.cancel();
	}

	if (m_postTransitionCompleteMessage.isScheduled())
	{
		m_postTransitionCompleteMessage.cancel();
	}

	if (m_asynchronousLaunchCompleteMessage.isScheduled())
	{
		m_asynchronousLaunchCompleteMessage.cancel();
		m_bIsAsynchronousLaunchCompleteMessagePending = false;
	}

	if (m_changeBackgroundMessage.isScheduled())
	{
		m_changeBackgroundMessage.cancel();
	}

	if (m_simulateKeyMessage.isScheduled())
	{
		m_simulateKeyMessage.cancel();
	}

	if (m_eventMessage.isScheduled())
	{
		m_eventMessage.cancel();
	}

	// Clean up any scheduled refresh events
	if (m_eventQueueMutex)
	{
		m_eventQueueMutex->lock(LcCMutex::EInfinite);
		destroyEventQueue();
		m_eventQueueMutex->unlock();
	}
	m_activePage = -1;
	destroyUnwantedPages(true);

	m_pageStack.clear();

#if defined(NDHS_JNI_INTERFACE)
	m_currentStateSnapshot.clear();
#endif

	if (m_manifestStack)
		m_manifestStack->popManifestToLevel(-1);
	if (m_tokenStack)
		m_tokenStack->popTokensToLevel(-1);

	// Do this after the manifest stack has been cleared.
	m_paletteMap.clear();

	m_designSizes.clear();
	m_keys.clear();

	m_plugin = NULL;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager::CDisplay> NdhsCPageManager::CDisplay::create()
{
	LcTaOwner<CDisplay> ref;
	ref.set(new CDisplay());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager::CDisplay::CDisplayMode> NdhsCPageManager::CDisplay::CDisplayMode::create()
{
	LcTaOwner<CDisplayMode> ref;
	ref.set(new CDisplayMode());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager::CTemplateItem> NdhsCPageManager::CTemplateItem::create()
{
	LcTaOwner<CTemplateItem> ref;
	ref.set(new CTemplateItem());
	//ref->construct();
	return ref;
}



/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::loadAppConfigFromXml(LcTmString& errorString)
{
	// Push the manifest first
	LcTaString manifest = m_con->getAppPath() + NDHS_MANIFEST_FILENAME;
	LcTaString package = NDHS_MAIN_PACKAGE_NAME;
	if (m_manifestStack->pushManifest(manifest, package))
	{
#ifdef IFX_VALIDATE_PROJECTS
		// Don't validate signatures in debug builds
		// Note that we don't check the version number of app config files,
		// since these files cannot be separately installed like themes are
		NdhsCProjectValidator pv;
		int error = pv.loadProject(m_manifestStack->getManifest(m_manifestStack->getStackHeight() - 1));
		if ((error != NdhsCProjectValidator::ESuccess) || (pv.validateProject() == false))
		{
			m_manifestStack->popManifest(1);
			errorString = "The ";
			errorString += m_con->getAppPath();
			errorString += NDHS_MANIFEST_FILENAME;
			// Convert all the forward slashes to back slashes if required.
			#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
				errorString.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
			#endif
			errorString += " file validation is incorrect";
			return false;
		}
#endif
	}

	// App-specific default settings and look and feel settings are read
	// from "[appName].xml" in "[appPath]" folder.
	// This is hard coded in version 1.0
	LcTaString err;
	// Set the directory slash separators to the non default if required.
	LcTaString localPath = m_con->getAppPath() + NDHS_APP_SETTINGS_FILENAME;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif
	LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(localPath, err);
	if (root)
	{
		LcTaString guid = "";

		//
		// Get the settings section
		//

		LcCXmlElem* settings = root->find(NDHS_TP_XML_SETTINGS);
		if (settings)
		{
			// Check for integrated mode
			bool integrated = LcCXmlAttr::strToBool(settings->getAttr(NDHS_TP_XML_INTEGRATED, "true"));
			m_con->setIntegrated(integrated);

			m_defaultTheme = settings->getAttr(NDHS_TP_XML_DEFAULT_THEME);
#ifdef LC_USE_MOUSEOVER
			// Mouse over delay attribute
			LcTaString attrMouseOverDelay = settings->getAttr(NDHS_TP_XML_MOUSEOVER_DELAY);
			if(attrMouseOverDelay.isEmpty() == false)
			{
				m_mouseOverDelay = attrMouseOverDelay.toInt();
			}
			else
			{
				m_mouseOverDelay = 0;
			}
#endif
			LcCXmlAttr* a = settings->findAttr(NDHS_TP_XML_GUID);
			if (!a)
			{
				// Trace on error
				NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - GUID missing");

				return false;
			}

			guid = a->getVal();

			// Load the entry point map

			LcCXmlElem* elemEntryPointMap = settings->find(NDHS_TP_XML_ENTRY_POINT_MAP);

			if (elemEntryPointMap)
			{
				LcCXmlElem* entryPoint = elemEntryPointMap->getFirstChild();

				for (; entryPoint; entryPoint = entryPoint->getNext())
				{
					LcTaString id = entryPoint->getAttr(NDHS_TP_XML_ENTRY_POINT_ID).toLower();
					LcTaString uri = entryPoint->getAttr(NDHS_TP_XML_NODE_URI).toLower();

					// check we got valid id and uri
					if (id.isEmpty() == false && uri.isEmpty() == false)
					{
						m_entryPointStack->pushMap(id, uri);
					}
				}
			}

			// Load the font settings

			LcCXmlElem* elemFont = settings->find(NDHS_TP_XML_FONT);
			if (elemFont)
			{
				LcCXmlAttr* attr = elemFont->findAttr(NDHS_TP_XML_FACE);
				if (attr)
					m_defaultFontFace = NdhsCExpression::CExprSkeleton::create(attr->getVal());

				attr = elemFont->findAttr(NDHS_TP_XML_STYLE);
				if (attr)
					m_defaultFontStyle = LcCXmlAttr::strToFontStyle(attr->getVal());

				attr = elemFont->findAttr(NDHS_TP_XML_COLOR);
				if (attr)
					m_defaultFontColor = attr->getVal();
			}
		}
		else
		{
			// Trace on error
			NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - settings section missing");

			return false;
		}

		//
		// Get the design sizes section
		//

		LcCXmlElem* designSizes = root->find(NDHS_TP_XML_DESIGN_SIZES);
		if (designSizes)
		{
			LcCXmlElem* designSize = designSizes->getFirstChild();

			for (; designSize; designSize = designSize->getNext())
			{
				if (designSize->getName().compareNoCase(NDHS_TP_XML_DESIGN_SIZE) == 0)
				{
					TDesignSize ds;

					LcTaString name = designSize->getAttr(NDHS_TP_XML_NAME).toLower();
					ds.width		= designSize->getAttr(NDHS_TP_XML_WIDTH, "-1").toInt();
					ds.origin		= designSize->getAttr(NDHS_TP_XML_ORIGIN, "-1").toInt();

					if (name.length() != 0 && ds.width != -1 && ds.origin != -1)
					{
						m_designSizes[name] = ds;
					}
				}
			}
		}
		else
		{
			// Trace on error
			NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - no design size specified");

			return false;
		}

		//
		// Get the displays section
		//

		LcCXmlElem* displays = root->find(NDHS_TP_XML_DISPLAYS);
		if (displays)
		{
			LcCXmlElem* display = displays->getFirstChild();

			for (; display; display = display->getNext())
			{
				if (display->getName().compareNoCase(NDHS_TP_XML_DISPLAY) == 0)
				{
					LcTaOwner<CDisplay> disp = CDisplay::create();

					LcTaString name = display->getAttr(NDHS_TP_XML_NAME).toLower();

					if (name.length() == 0)
					{
						disp.destroy();
					}
					else
					{
						LcCXmlElem* displayMode = display->getFirstChild();

						for (; displayMode; displayMode = displayMode->getNext())
						{
							if (displayMode->getName().compareNoCase(NDHS_TP_XML_MODE) == 0)
							{
								LcTaOwner<CDisplay::CDisplayMode> dm = CDisplay::CDisplayMode::create();

								LcTaString name = displayMode->getAttr(NDHS_TP_XML_NAME).toLower();
								dm->width		= displayMode->getAttr(NDHS_TP_XML_WIDTH, "-1").toInt();
								dm->height		= displayMode->getAttr(NDHS_TP_XML_HEIGHT, "-1").toInt();
								dm->designSize  = displayMode->getAttr(NDHS_TP_XML_DESIGN_SIZE).toLower();
								dm->defaultMenu = displayMode->getAttr(NDHS_TP_XML_DEFAULT_ENTRY_POINT_ID);
								dm->defaultLink = displayMode->getAttr(NDHS_TP_XML_DEFAULT_LINK);

								if (name.length() == 0 || dm->width == -1
									|| dm->height == -1 || dm->designSize.length() == 0)
								{
									dm.destroy();
								}
								else
								{
									disp->displayModes.add_element(name, dm);
								}
							}
						}

						m_displays.add_element(name, disp);
					}
				}
			}
		}
		else
		{
			// Trace on error
			NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - no display specified");

			return false;
		}

		//
		// Get the keys section
		//

		LcCXmlElem* keys = root->find(NDHS_TP_XML_KEYS);
		if (keys)
		{
			LcCXmlElem* key = keys->getFirstChild();

			for (; key; key = key->getNext())
			{
				if (key->getName().compareNoCase(NDHS_TP_XML_KEY) == 0)
				{
					int code = -1;

				#ifdef NU_SIMULATION
					// in case of SimTest, try to find simtestCode first.
					code = key->getAttr(NDHS_TP_XML_SIMTEST_CODE, "-1").toInt();
				#endif

					if (-1 == code)
					{
						// otherwise use scanCode
						code = key->getAttr(NDHS_TP_XML_SCAN_CODE, "-1").toInt();
					}

					if (-1 != code)
					{
						// Check if key name already exists?
						LcTaString name = key->getAttr(NDHS_TP_XML_NAME).toLower();
						TmMKeys::iterator iter = m_keys.find(name);

						if (iter == m_keys.end())
						{
							// add new entry
							LcTaArray<int> tempArray;
							tempArray.push_back(code);

							LcTmMap<LcTmString, LcTmArray<int> >::value_type temp(name, tempArray);

							m_keys.insert(temp);
						}
						else
						{
							iter->second.push_back(code);
						}
					}
				}
			}
		}
		// Load palette
		loadPalette(root.ptr());

		setCurrentDesignSize();
		setCurrentDefaultLinkAndMenu();

		int width = 0;
		int height = 0;

		getScreenSize(m_screenSize, width, height);

		getSpace()->setPortraitMode(width <= height);

		// Add to tokens
		m_tokenStack->pushTokens(m_con->getAppPath());

		// Add the Ini tokens
		m_tokenStack->loadIniTokens(m_dataPath, m_iniTokenFile);

		//
		// Get the plugin section
		//

		m_plugin = m_con->addPlugin();
		if (m_plugin != NULL)
			m_tokenStack->setPlugin(m_plugin);

#ifdef IFX_VALIDATE_PROJECTS
		// Validate the GUID
		if (m_plugin == NULL || m_plugin->validateGUID(guid) == false)
		{
			// Trace on error
			NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - Invalid GUID");
			errorString = "Plugin GUID validation failed.";

			return false;
		}
		else
#endif
		{
			return true;
		}
	}
	else
	{
		// Set the external error.
		errorString = err;

		// Trace on error
		NDHS_TRACE(ENdhsTraceLevelError, NULL, localPath + " - " + err);
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::setCurrentDefaultLinkAndMenu()
{
	CDisplay::CDisplayMode* displayMode = getDisplayMode(m_screenSize);
	if (displayMode)
	{
		m_defaultEntryPointID = displayMode->defaultMenu;
		m_mainMenuLink = displayMode->defaultLink;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::setCurrentDesignSize()
{
	CDisplay::CDisplayMode* displayMode = getDisplayMode(m_screenSize);
	if (displayMode)
	{
		m_designSize = displayMode->designSize;
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::getCurrentDesignSize(int& width, int& origin)
{
	TmMDesignSizes::iterator itDesignSize = m_designSizes.find(m_designSize);

	if (itDesignSize != m_designSizes.end())
	{
		TDesignSize designSize = itDesignSize->second;

		width = designSize.width;
		origin = designSize.origin;

		return true;
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::getCurrentScreenSize(int& width, int& height)
{
	return getScreenSize(m_screenSize, width, height);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::getScreenSize(const LcTmString& displayMode, int& width, int& height)
{
	CDisplay::CDisplayMode* pDisplayMode = getDisplayMode(displayMode);
	if (pDisplayMode)
	{
		width = pDisplayMode->width;
		height = pDisplayMode->height;

		return true;
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
*/
LcTmArray<int>*	NdhsCPageManager::getKeyCodes(const LcTmString& key)
{
	TmMKeys::iterator iter = m_keys.find(key);

	return ( (iter == m_keys.end()) ? NULL : &(iter->second) );
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageManager::CDisplay::CDisplayMode* NdhsCPageManager::getDisplayMode(const LcTmString& displayMode)
{
	typedef LcTmOwnerMap<LcTmString, CDisplay::CDisplayMode> TmMDisplayModes;

	TmMDisplays::iterator itDisplay = m_displays.find(NDHS_TP_XML_PRIMARY);

	if (itDisplay != m_displays.end())
	{
		CDisplay* display = itDisplay->second;
		if (display)
		{
			TmMDisplayModes::iterator itDisplayMode = display->displayModes.find(displayMode.toLower());

			if (itDisplayMode != display->displayModes.end())
			{
				return itDisplayMode->second;
			}
		}
	}

	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::setLanguageAndScreenSize(const LcTmString& language, const LcTmString& screenSize)
{
	// As script generations is started we need to reset theme to root menu, the best way to do this is to
	// change language to some invalid language so that theme language is not changed but it reset to root
	// menu, and in case of script generation we may need to change language multiple times so we don't
	// need below condition

#ifdef IFX_GENERATE_SCRIPTS
	if (m_scriptGenerator == NULL || m_scriptGenerator->languageChange() == false)
	{
#endif
		// If no change, nothing to do
		if (m_language.compareNoCase(language) == 0
			&& m_screenSize.compareNoCase(screenSize) == 0)
			return;
#ifdef IFX_GENERATE_SCRIPTS
	}
#endif

	// Make sure animations have finished.
	jumpTransitionToEnd();

	// Save out the tokens
	m_tokenStack->saveIniTokens(m_dataPath, m_iniTokenFile);

	m_tokenStack->flushAllTokens();

	// Destroy all pages
	m_activePage = -1;
	destroyUnwantedPages(true);

	m_screenSize = screenSize.toLower();

	setCurrentDesignSize();
	setCurrentDefaultLinkAndMenu();
	m_language   = language;
	m_tokenStack->setScreenSize(m_screenSize);
	m_tokenStack->setLanguage(m_language);

	int width = 0;
	int height = 0;

	getScreenSize(m_screenSize, width, height);

	getSpace()->setPortraitMode(width <= height);

	// Add the new tokens (screen size specific)
	m_tokenStack->pushTokens(m_con->getAppPath());

	// Add the Ini tokens
	m_tokenStack->loadIniTokens(m_dataPath, m_iniTokenFile);

#ifdef IFX_GENERATE_SCRIPTS
	if (m_scriptGenerator)
	{
		m_scriptGenerator->setFirstTime();
	}
#endif

#ifndef NDHS_JNI_INTERFACE
	applyFirstTheme();
#endif //NDHS_JNI_INTERFACE
}

/*-------------------------------------------------------------------------*//**
	Create widgets etc on realization
*/
void NdhsCPageManager::onRealize()
{
	// This is hard coded to app path for version 1.0
	getSpace()->setBasePath(m_con->getAppPath());
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::onRetire()
{
	jumpTransitionToEnd();
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::applyFirstTheme()
{
	// It should be false, initially
	bool success = false;

	// Destroy all pages
	m_activePage = -1;
	destroyUnwantedPages(true);

	//
	// Try to apply a requested theme (if any)
	//
	LcTmString localName = m_reqThemeName;
	if (success == false &&
		!localName.isEmpty())
	{
		LcTaString fullPackagePath;

		// Work out the package path
		fullPackagePath = this->getFullPackagePath(m_con->getThemePath() + localName);
		if (!fullPackagePath.isEmpty())
		{
			success = loadTheme(fullPackagePath, true);
		}

		// We have finished with the request string now.
		m_reqThemeName = "";
	}

	//
	// Try to apply the last known good theme
	//
	if (success == false &&
		!m_lastGoodThemeName.isEmpty())
	{
		success = loadTheme(m_lastGoodThemeName, true);
	}

	//
	// Now try and apply the package in the themeCurrent token
	//
	LcTaString package;
	if (success == false
		&& m_tokenStack->getIniTokens()->getValue(NDHS_TP_XML_THEME_CURRENT, package)
		&& package.length() > 0)
	{
		success = loadTheme(package, true);
	}

	//
	// Now try and apply the default theme package
	//

	if (success == false && m_defaultTheme.length() > 0)
	{
		LcTaString fullPackagePath = m_con->getThemePath() + m_defaultTheme;
#ifdef NDHS_JNI_INTERFACE
		fullPackagePath = m_con->getAppPath() + fullPackagePath + NDHS_DIR_SEP;
#else
		fullPackagePath = this->getFullPackagePath(fullPackagePath);
#endif
		if (!fullPackagePath.isEmpty())
		{
			success = loadTheme(fullPackagePath, true);
		}
	}

	return success;
}

/*---------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::isValidURIToExecuteFromCallBack (const LcTmString& uri)
{
	if (uri.isEmpty())
		return false;

	// Modules are forbidden from executing "node://" links
	if (uri.find("node://") != -1)
		return false;

	// Valid URI
	return true;
}

/*---------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::getParamterByName(const LcTmString& link, const LcTmString& paramName, LcTmString& value)
{
	IFX_HUTILITY hUtility;
	LcTWChar* ptr = NULL;
	bool status = true;
	LcTaString key = "", val = "";

	// Initialize URI Parser
	if (IFXU_UriParser_Initialize( &hUtility, link.bufWChar()) != IFX_SUCCESS)
		return false;

	IFX_INT32 size = 0;

	if (IFXU_UriParser_GetParamStringSize(hUtility, paramName.bufWChar(), &size, NULL) == IFX_SUCCESS)
	{
		ptr = LcTmAlloc<LcTWChar>::allocUnsafe(size);

		if (ptr)
		{
			if (IFXU_UriParser_GetParamStringData(hUtility, paramName.bufWChar(), ptr, NULL) == IFX_SUCCESS)
			{
				value.fromBufWChar(ptr, size);
			}
			else
			{
				status = false;
			}

			LcTmAlloc<LcTWChar>::freeUnsafe(ptr);
		}
	}
	else
	{
		status = false;
	}

	// Destory URI Parser
	IFXU_UriParser_Destroy( hUtility );

	return status;
}

/*------------------------------------------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::extractLinkPrefixAndLinkBody(const LcTmString& nodeUri, LcTmString& linkPrefix, LcTmString& linkBody)
{
	IFX_HUTILITY hUtility;
	IFX_INT32 size = 0;
	LcTWChar* ptr = NULL;
	bool status = true;

	// Initialize Uri Parser
	if (IFXU_UriParser_Initialize(&hUtility, nodeUri.bufWChar()) != IFX_SUCCESS)
		return false;

	// Get Link prefix size
	if (IFXU_UriParser_GetLinkPrefixSize(hUtility, &size) != IFX_SUCCESS)
		status = false;

	if (status)
	{
		// Allocate enough memory so that we can hold link prefix
		ptr = LcTmAlloc<LcTWChar>::allocUnsafe(size);

		if (ptr)
		{
			// Get Link prefix data
			if (IFXU_UriParser_GetLinkPrefixData(hUtility, ptr) == IFX_SUCCESS)
			{
				// Set Link Prefix
				linkPrefix.fromBufWChar(ptr, size);
			}
			LcTmAlloc<LcTWChar>::freeUnsafe(ptr);
		}
	}

	// reset status
	status = true;
	size = 0;

	// Get Link body size
	if (IFXU_UriParser_GetLinkBodySize(hUtility, &size, 0) != IFX_SUCCESS)
		status = false;

	if (status)
	{
		// Allocate enough memory so that we can hold link body
		ptr = LcTmAlloc<LcTWChar>::allocUnsafe(size);

		if (ptr)
		{
			// Get Link body data
			if (IFXU_UriParser_GetLinkBodyStringData(hUtility, ptr, 0) == IFX_SUCCESS)
			{
				// Set Link Body
				linkBody.fromBufWChar(ptr, size);
			}
			LcTmAlloc<LcTWChar>::freeUnsafe(ptr);
		}
	}

	// Destory URI Parser
	if (IFXU_UriParser_Destroy(hUtility) != IFX_SUCCESS)
		status = false;

	return status;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCPageManager::entryPointIDToNodeURI(const LcTmString& entryPoint)
{
	 return m_entryPointStack->getEntryPoint(entryPoint);
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS NdhsCPageManager::openEntryPoint(bool openDefault, const LcTmString& entryPoint)
{
	LcTaString nodeUri = "";
	IFX_RETURN_STATUS status = IFX_SUCCESS;
	int stackLevel = 0;

	if (openDefault == true)
	{
		// Try to get page URI against the entry point passed
		m_currentEntryPointId = entryPoint;
		nodeUri = m_entryPointStack->getEntryPoint(entryPoint);

		if (nodeUri.isEmpty())
		{
			// Try to get entry point against default menu link
			m_currentEntryPointId = m_defaultEntryPointID;
			nodeUri = m_entryPointStack->getEntryPoint(m_defaultEntryPointID);

			if (nodeUri.isEmpty())
			{
				// Try to get entry point from default link, Note currently
				// we are assuming entry point can be specified in default link
				// also
				nodeUri = m_entryPointStack->getEntryPoint(m_mainMenuLink);

				// Last try if we are not able to find page URI against default menu link
				// try to use it as it is
				if (nodeUri.isEmpty())
				{
					nodeUri = m_mainMenuLink;

					if (nodeUri.isEmpty())
					{
						NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleGeneral, "Default link not found for current display mode.");
					}
				}
			}
		}
	}
	else
	{
		if (m_activePage != -1 && entryPoint.isEmpty() == false)
		{
			nodeUri = m_entryPointStack->getEntryPoint(entryPoint);

			// Check we got valid page Uri
			if (nodeUri.isEmpty() == false)
			{
				LcTaString linkPrefix, linkBody;

				if (extractLinkPrefixAndLinkBody(nodeUri, linkPrefix, linkBody) == false)
				{
					// We cant extract link prefix and link body
					status = IFX_ERROR;
				}

				// See if navigation allowed or not
				if (m_pageStack[m_activePage]->checkNaviation(linkBody) == false)
				{
					status = IFX_ERROR_NAVIGATION_DISALLOWED;
				}
			}
			else
			{
				status = IFX_ERROR;
			}
		}
		else
		{
			status = IFX_ERROR;
		}
	}

	if (status == IFX_SUCCESS)
	{
		LcTaOwner<NdhsCMenuItem> mi = NdhsCMenuItem::create(NULL);
		mi->setLinkAttr(nodeUri);

		if (m_manifestStack)
		{
			stackLevel = m_manifestStack->getStackHeight() - 1;
		}

		if (openPage(mi.ptr(), true, false, stackLevel) == false)
		{
			status = IFX_ERROR;
		}
	}

#if defined(IFX_WIN_PLAYER)
	getSpace()->emitIfxPlayerSignal(IFX_SPLASH_OFF);
#endif

	return status;
}

/*-------------------------------------------------------------------------*//**
*/
#if defined(NDHS_PREVIEWER) || defined(NDHS_JNI_INTERFACE)
bool NdhsCPageManager::applyPackage(LcTmString& package, bool inStaticPreview)
{
	// Destroy all pages
	m_activePage = -1;
	destroyUnwantedPages(true);

	if (package.compareNoCase(NDHS_MAIN_PACKAGE_NAME) == 0)
	{
		// If they selected NDHS_APP_SETTINGS_FILENAME, we need to determine the
		// configured menu and open it

		LcTaOwner<NdhsCMenuItem> mi = NdhsCMenuItem::create(NULL);
		mi->setLinkAttr(m_mainMenuLink);
		return openPage(mi.ptr(), true, false, 0);
	}
	else
	{
#ifdef NDHS_JNI_INTERFACE
		bool result = false;

		if (package.length() == 0)
		{
			result = applyFirstTheme();
			// We don't need to open default entry point at this level, in case of static preview
			if(inStaticPreview == false)
			{
				if (openEntryPoint(true, "") != IFX_SUCCESS)
				{
					result = false;
				}
				else
				{
					result = true;
				}
			}
			else
			{
				result = true;
			}
		}
		else
		{
			LcTaString packagePath = m_con->getAppPath() + m_con->getThemePath() + package;
			if(loadTheme(packagePath, true))
			{
				// We don't need to open default entry point at this level, in case of static preview
				if(inStaticPreview == false)
				{
					if (openEntryPoint(true, "") == IFX_SUCCESS)
						result = true;
				}
				else
				{
					result = true;
				}
			}
		}

		return result;
#else
		return loadTheme(package, true);
#endif
	}

	return false;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::applyConfiguration(const LcTmString& language, const LcTmString& displayMode, const LcTmString& package, const LcTmString entryPoint_Id)
{
	bool retValue = true;
	int updateMask = 0;

	// Schedule a message and store the package name in allocated memory
	if (package.compare(getCurrentThemeString())!=0)
	{
		m_reqThemeName = package;
		updateMask |= NDHS_CONFIG_THEME_PACKAGE;
	}

	// Schedule a message and store the display mode in allocated memory
	if (displayMode.compare(getCurrentScreenModeString())!=0)
	{
		m_reqDisplayMode = displayMode;
		updateMask |= NDHS_CONFIG_DISPLAY_MODE;
	}

	// Schedule a message and store the language in allocated memory
	if (language.compare(getCurrentLanguageString())!=0)
	{
		m_reqLanguage = language;
		updateMask |= NDHS_CONFIG_LANGUAGE;
	}

	if(m_currentEntryPointId.compare(entryPoint_Id)!=0)
	{
		m_reqEntryPointId = entryPoint_Id;
		updateMask |= NDHS_CONFIG_ENTRYPOINT_ID;
	}

	// Apply the changes
	if (updateMask)
	{
		// Must set a timer so that we can perform this
		// from a clean execution thread.
		if (m_configureExternalMessage.isScheduled())
		{
			m_configureExternalMessage.cancel();
		}

		m_configureExternalMessage.schedule(getSpace()->getTimer(), updateMask, 0);
		retValue = true;
	}

	return retValue;
}


/*-------------------------------------------------------------------------*//**
	Handle options menu events
*/
bool NdhsCPageManager::onOptionsMenuTrigger(LcTmArray<NdhsCTemplate::CAction::CAttempt*>* optionsAttempts)
{
	if (m_activePage < 0)
		return false;

	LcTaString emptyString = "";

	bubbleTrigger((int)ENdhsNavigationOptionsMenu, -1, emptyString, optionsAttempts, false);

	return true;
}

/*-------------------------------------------------------------------------*//**
	Handle simulate link events
*/
bool NdhsCPageManager::onSimulateLinkTrigger(LcTmArray<NdhsCTemplate::CAction::CAttempt*>* linkAttempt)
{
	if (m_activePage < 0)
		return false;

	LcTaString emptyString = "";

	bubbleTrigger((int)ENdhsNavigationOpenLink, -1, emptyString, linkAttempt, true);

	return true;
}

/*-------------------------------------------------------------------------*//**
	Translate key codes to special keys
*/
void NdhsCPageManager::translateKeyCode(int &c)
{
	switch (c)
	{
		case LcCSpace::KEY_BACK:
		{
			c=(int)ENdhsNavigationSoftKeyBack;
			break;
		}
		case LcCSpace::KEY_SELECT:
		{
			c=(int)ENdhsNavigationKeySelect;
			break;
		}
		case LcCSpace::KEY_UP:
		{
			c=(int)ENdhsNavigationKeyUp;
			break;
		}
		case LcCSpace::KEY_DOWN:
		{
			c=(int)ENdhsNavigationKeyDown;
			break;
		}
		case LcCSpace::KEY_LEFT:
		{
			c=(int)ENdhsNavigationKeyLeft;
			break;
		}
		case LcCSpace::KEY_RIGHT:
		{
			c=(int)ENdhsNavigationKeyRight;
			break;
		}
		case LcCSpace::KEY_HASH:
		{
			c=(int)ENdhsNavigationKeyHash;
			break;
		}
		case LcCSpace::KEY_ASTERISK:
		{
			c=(int)ENdhsNavigationKeyAsterisk;
			break;
		}
		// unknown key, do nothing in case scancode has a mapping set
		// 0-9 and a-z also handled
		default:
			break;
	}
}

/*-------------------------------------------------------------------------*//**
	Handle keyDown events
*/
bool NdhsCPageManager::onKeyDown(int c, bool fromModule)
{
	if (m_activePage < 0)
		return false;

	// emit key down event
#ifdef IFX_GENERATE_SCRIPTS
	if (m_scriptGenerator)
		m_scriptGenerator->attachKeyEvent(c, KeyEventDown, getSpace()->getTimestamp());
#endif //IFX_GENERATE_SCRIPTS

	LcTaString emptyString = "";
	translateKeyCode(c);
	bubbleTrigger(c, -1, emptyString, NULL, fromModule);

	// We always consume the key
	return true;
}

/*-------------------------------------------------------------------------*//**
	Bubble a trigger down the page stack
*/
bool NdhsCPageManager::bubbleTrigger(	int code,
										int slot,
										LcTmString& elementClass,
										LcTmArray<NdhsCTemplate::CAction::CAttempt*>* optionsAttempts,
										bool fromModule)
{
	bool consumed = false;

	// Pass the trigger to each page, starting
	// at the active until it is consumed
	for (int i = m_activePage; i >= 0 && !consumed; --i)
	{
		consumed = m_pageStack[i]->processTrigger(code, slot, elementClass, optionsAttempts, fromModule);
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
	Handle keyDown events
*/
bool NdhsCPageManager::onKeyUp(int c)
{
	// Key up is only used by the focused plugin
	// element or event handler, which must be on
	// the active page.  So there is no point
	// in bubbling the key

	if (m_activePage < 0)
		return false;

	// emit key up event
#ifdef IFX_GENERATE_SCRIPTS
	if (m_scriptGenerator)
		m_scriptGenerator->attachKeyEvent(c, KeyEventUp, getSpace()->getTimestamp());
#endif //IFX_GENERATE_SCRIPTS

	// The page currently active or becoming active
	NdhsCPageModel* page = m_pageStack[m_activePage];
	page->processKeyUp(c);

	// We always consume the key
	return true;
}

/*-------------------------------------------------------------------------*//**
	Simulate a key trigger
*/
void NdhsCPageManager::simulateKeyDown(int c)
{
	// Must schedule this to be handled since we may end
	// up destroying a widget that is above us in the call stack

	if(m_simulateKeyMessage.isScheduled())
	{
		m_simulateKeyMessage.cancel();
	}

	m_simulateKeyMessage.schedule( getSpace()->getTimer(), c, 0);
}

/*-------------------------------------------------------------------------*//**
	Add a key trigger event to the event queue
*/
void NdhsCPageManager::queueKeyDownEvent(int c)
{
	TEventInfo* info = TEventInfo::createUnsafe();
	if (info)
	{
		info->eventType = EEventTrigger;
		info->fieldOrUri = NULL;
		info->element = NULL;
		info->key = c;
		info->next = NULL;

		if(queueEvent(info) == false)
			TEventInfo::destroy(info);
	}
}

/*-------------------------------------------------------------------------*//**
	Add a link event to the event queue
*/
void NdhsCPageManager::queueLinkEvent(const IFX_WCHAR* uri)
{
	if (uri)
	{
		TEventInfo* info = TEventInfo::createUnsafe();
		if (info)
		{
			info->eventType = EEventLink;
			info->fieldOrUri = (IFX_WCHAR*)LcTmAlloc<LcTByte>::allocUnsafe(((int)lc_wcslen((const IFX_WCHAR*)uri) + 1) * (int)sizeof(IFX_WCHAR));
			info->element = NULL;
			info->key = 0;
			info->next = NULL;

			if (info->fieldOrUri)
			{
				lc_wcscpy((IFX_WCHAR*)info->fieldOrUri, (const IFX_WCHAR*)uri);
				if(queueEvent(info) == false)
					TEventInfo::destroy(info);
			}
			else
			{
				TEventInfo::destroy(info);
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Add a link event to the event queue
*/
void NdhsCPageManager::queueSetActiveItemEvent(IFX_HMENU hMenu, int item)
{
	TEventInfo* info = TEventInfo::createUnsafe();
	if (info)
	{
		info->eventType = EEventSetActiveItem;
		info->menu = hMenu;
		info->item = item;
		if(queueEvent(info) == false)
			TEventInfo::destroy(info);
	}
}

/*-------------------------------------------------------------------------*//**
	Schedules or cancels post transition complete timer
*/
void NdhsCPageManager::schedulePostTransitionComplete()
{
	// If we're not scheduled and we want to, schedule with 0 delay
	if (m_postTransitionCompleteMessage.isScheduled() == false)
	{
		m_postTransitionCompleteMessage.schedule(getSpace()->getTimer(), 0, 0);
	}
}

/*-------------------------------------------------------------------------*//**
	Schedules a post asysnchronous launch complete timer.
*/
void NdhsCPageManager::scheduleAsynchronousLaunchComplete()
{
	// If we're not scheduled and we want to, schedule with 0 delay
	if (m_asynchronousLaunchCompleteMessage.isScheduled() == false)
	{
		m_asynchronousLaunchCompleteMessage.schedule(getSpace()->getTimer(), 0, 0);
		m_bIsAsynchronousLaunchCompleteMessagePending = true;
	}
}

/*-------------------------------------------------------------------------*//**
	Resets the current theme to the top menu.
	This will not perform any animation
*/
void NdhsCPageManager::resetThemeToTop()
{
	if (m_activePage < 0)
		return;

	// Stop any inter-page transitions
	jumpTransitionToEnd();

	// Make sure the active page is not animating internally
	m_pageStack[m_activePage]->jumpTransitionToEnd(false);
	m_pageStack[m_activePage]->componentsJumpTransitionToEnd(false);

	// Fail if already at root, but not before
	// stopping any animators
	if (m_activePage == 0)
		return;

	int oldActivePage = m_activePage;
	m_activePage = 0;

	setupPrimaryLightTransitionOverrideFlag(0, oldActivePage);

	m_action = EResetToTop;

	m_animatingStateChange = false;

	m_pageStack[m_activePage]->setVisible(true);

	m_con->updateScreenFurniture();

	// Let it use its own transition time
	m_terminalTime = m_pageStack[m_activePage]->getDefaultTerminalTime();
	m_terminalVelocityProfile = m_pageStack[m_activePage]->getDefaultTerminalVelocityProfile();

	// Get the root page back to its interactive state
	// The page works out the active slot itself, so we can just pass in 0
	m_animatingStateChange = m_pageStack[m_activePage]->changeState(ENdhsPageStateInteractive, 0, NULL, false);

	setupPrimaryLightTransitionOverrideTiming(0, oldActivePage);

	// Note that active page will need an 'onTransitionComplete'
	m_action = EAnimateActivePage;

	if (!m_animatingStateChange)
		onTransitionComplete(false);
#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
	else
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_STARTED);
#endif

	// Force destroy the pages
	destroyUnwantedPages(true);

	LcCEnvironment::get()->compressHeap();
}

#ifdef NDHS_PREVIEWER
/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::refreshActivePage()
{
	// This function is just for the previewer to be able to
	// add menu additions.  It needs to be able to refresh
	// non-plugin menus, so passing NULL will refresh the
	// active page on the previewer only
	refreshPage(NULL, true);
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::refreshPage(IFX_HMENU hMenu, bool immediate)
{
	// Must schedule this to be handled since we may end
	// up destroying a widget that is above us in the call stack

	TEventInfo* info = TEventInfo::createUnsafe();
	if (info)
	{
		info->eventType = EEventRefreshPage;
		info->menu = hMenu;
		info->immediate = immediate;
		info->fieldOrUri = NULL;
		info->element = NULL;
		info->next = NULL;

		if(queueEvent(info) == false)
			TEventInfo::destroy(info);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::refreshField(IFX_HMENU hMenu, int item, const IFX_WCHAR* field)
{
	// Must schedule this to be handled since we may end
	// up destroying a widget that is above us in the call stack

	TEventInfo* info = TEventInfo::createUnsafe();
	if (info)
	{
		info->eventType = EEventRefreshField;
		info->menu = hMenu;
		info->item = item;
		info->element = NULL;
		info->fieldOrUri = NULL;
		info->next = NULL;

		if (field)
		{
			info->fieldOrUri = (IFX_WCHAR*)LcTmAlloc<LcTByte>::allocUnsafe(((int)lc_wcslen((const IFX_WCHAR*)field) + 1) * (int)sizeof(IFX_WCHAR));

			if (info->fieldOrUri)
			{
				lc_wcscpy((IFX_WCHAR*)info->fieldOrUri, (const IFX_WCHAR*)field);
			}
		}

		if ((field == NULL) || (field && info->fieldOrUri))
		{
			if(queueEvent(info) == false)
				TEventInfo::destroy(info);
		}
		else
		{
			TEventInfo::destroy(info);
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Adds a refresh event to the queue
	info - Event to add. Ownership transferred
*/
bool NdhsCPageManager::queueEvent(TEventInfo* info)
{
	LC_ASSERT(info);

	if (m_eventQueueMutex.ptr() == NULL || m_eventQueueMutex->lock(LcCMutex::EInfinite) == false)
	{
		return false;
	}
	else
	{
		// Append event to the list
		if (m_eventListHead)
		{
			m_eventListTail->next = info;
		}
		else
		{
			m_eventListHead = info;
		}
		// setting the tail
		m_eventListTail = info;

		// Release lock
		m_eventQueueMutex->unlock();

		// Schedule event if necessary
		if(m_eventMessage.isScheduled() == false)
			m_eventMessage.schedule(getSpace()->getTimer(), 0, 0);
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
	Destroys a refresh queue
	info - Start of the queue to destroy
	Should only be used by the destructor since it
	is not protected by the mutex
*/
void NdhsCPageManager::destroyEventQueue()
{
	// Destroy entire queue
	TEventInfo* next = NULL;

	while (m_eventListHead)
	{
		next = m_eventListHead->next;
		TEventInfo::destroy(m_eventListHead);
		m_eventListHead = next;
	}
}

#ifdef IFX_USE_PLUGIN_ELEMENTS
/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::refreshBufferedElement(IFX_HELEMENT hElement)
{
	// Must schedule this to be handled since it may be
	// called from another task

	TEventInfo* info = TEventInfo::createUnsafe();
	if (info)
	{

		info->eventType = EEventRefreshBufferedElement;
		info->menu = NULL;
		info->item = -1;
		info->fieldOrUri = NULL;
		info->element = hElement;
		info->next = NULL;

		if(queueEvent(info) == false)
			TEventInfo::destroy(info);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::resizeBufferedElement(IFX_HELEMENT hElement, IFX_BUFFER_SIZE* bufferSize)
{
	bool retVal = false;

	// Find the plugin element that owns this handle
	NdhsCPlugin::NdhsCPluginHElement* element = findOwnerPluginHandler(hElement);

	if (element != NULL && bufferSize != NULL)
		retVal = element->resizeBufferedElement(bufferSize->x, bufferSize->y);

	return retVal;
}

#endif

/*-------------------------------------------------------------------------*//**
	Retrieve the plug-in index of the active item on a specified plug-in menu.
	Returns -1 for an error.
*/
int NdhsCPageManager::getHMenuActiveItemIndex(IFX_HMENU hMenu)
{
	int menuIndex;

	// Find the correct menu - Search backwards because it is more likely to
	// be the last item.
	for (menuIndex = (int)m_pageStack.size()-1; menuIndex >= 0 ; menuIndex--)
	{
		NdhsCPageModel*	currentPage = m_pageStack.at(menuIndex);
		int index = currentPage->getHMenuActiveItemIndex(hMenu);

		if (index != -1)
			return index;
	}
	return -1;
}

/*-------------------------------------------------------------------------*//**
	Retrieve the full path of a file for a specified plug-in menu manifest file.
	Returns false for an error.
*/
bool NdhsCPageManager::getFullFilePath(IFX_HMENU hMenu, const IFX_WCHAR* pInput, IFX_WCHAR* returnFilePath)
{
	bool retVal = false;
	int menuIndex;
	volatile int memError = 0;

	if (pInput == NULL || returnFilePath == NULL)
		return retVal;

	// Prepare cleanup frame
	LC_CLEANUP_PUSH_FRAME(memError);

	if (memError == 0)
	{
		LcTaString searchFile;

		searchFile.fromBufWChar((const IFX_WCHAR*)pInput, (int)lc_wcslen(pInput));

		if (!searchFile.isEmpty())
		{
			LcTaString retSearchPath = "";
			// Find the correct menu - Search backwards because it is more likely to
			// be the last item.
			for (menuIndex = (int)m_pageStack.size()-1; menuIndex >= 0 ; menuIndex--)
			{
				NdhsCPageModel*	currentPage = m_pageStack.at(menuIndex);

				if (currentPage != NULL && currentPage->getFullFilePath(hMenu, searchFile, retSearchPath, menuIndex))
				{
					retVal = true;
					break;
				}
			}

			if (retVal == true)
			{
				int strLen = retSearchPath.length() + 1;
				if (strLen < IFX_MAX_PATH_LENGTH)
				{
					lc_wcsncpy(returnFilePath, retSearchPath.bufWChar(), strLen);
				}
				else
				{
					retVal = false;
				}
			}
		}
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memError);

	// Check to see if we suffered an OOM error
	if (memError != 0)
	{
		retVal = false;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::onTransitionComplete(bool setIdle)
{
	m_animatingStateChange = false;

	if (m_activePage < 0)
	{
		return;
	}

	switch (m_action)
	{
		case EBackDestroyChildren:
		{
			// Any page could be transitioning,
			// so iterate the entire page stack
			for (int i = (int)m_pageStack.size() - 1; i >= 0; --i)
			{
				if (m_pageStack[i]->isTransitioning())
				{
					m_pageStack[i]->onTransitionComplete(setIdle);

					switch (m_pageStack[i]->getPageState())
					{
						case ENdhsPageStateClose:
						case ENdhsPageStateHide:
						{
							m_pageStack[i]->setVisible(false);
							break;
						}

						case ENdhsPageStateInteractive:
						case ENdhsPageStateSelected:
						{
							m_pageStack[i]->setVisible(true);
							break;
						}

						default:
							break;
					}
				}
			}

			setMatchedBackground(m_pageStack[m_activePage]->getStackLevel());

			// We cant just call deleteUnwantedPages() now because
			// this function might have been called by the sub-page
			// as it completed the transition (assuming it completed
			// after the parent).  We would then end up deleting the
			// object that called us.  Must schedule the deletion.

			// Set a timer to trigger destroyUnwantedPages
			if(m_destroyUnwantedPagesMessage.isScheduled())
			{
				m_destroyUnwantedPagesMessage.cancel();
			}

			m_destroyUnwantedPagesMessage.schedule( getSpace()->getTimer(), 0, 0);

			break;
		}

		case ELaunchLink:
		{
			// The active page was transitioning
			m_pageStack[m_activePage]->onTransitionComplete(setIdle);

			// We are here because of an animator event.  We must schedule a timer
			// to do handle the end transition because the space hasn't had chance
			// to clean out the animator list yet

			// Note, even when setting idle, we still allow the link to launch,
			// which will ultimately result in a transition back.  It
			// was a user action and should take priority

			if(m_launchLinkMessage.isScheduled())
			{
				m_launchLinkMessage.cancel();
			}

			m_launchLinkMessage.schedule( getSpace()->getTimer(), 0, 0);

			break;
		}

		case EOpenMenuHideDirectParent:
		{
			// The active page was transitioning
			m_pageStack[m_activePage]->onTransitionComplete(setIdle);

			// make the parent page invisible
			if (m_activePage > 0)
			{
				m_pageStack[m_activePage - 1]->onTransitionComplete(setIdle);

				m_pageStack[m_activePage - 1]->setVisible(false);
			}

			setMatchedBackground(m_pageStack[m_activePage]->getStackLevel());

			break;
		}

		case EOpenMenuShowDirectParent:
		{
			// The active page was transitioning
			m_pageStack[m_activePage]->onTransitionComplete(setIdle);

			// make the parent page invisible
			if (m_activePage > 0)
			{
				m_pageStack[m_activePage - 1]->onTransitionComplete(setIdle);

				m_pageStack[m_activePage - 1]->setVisible(true);
			}

			setMatchedBackground(m_pageStack[m_activePage]->getStackLevel());

			break;
		}

		case EOpenMenuHideAllParents:
		{
			// The active page was transitioning
			m_pageStack[m_activePage]->onTransitionComplete(setIdle);

			// make the parent page invisible
			if (m_activePage > 0)
			{
				for (int i = m_activePage - 1; i >= 0; --i)
				{
					// We only tweened pages to the hide state if
					// there were not already hidden.
					if (m_pageStack[i]->isTransitioning())
					{
						m_pageStack[i]->onTransitionComplete(setIdle);

						m_pageStack[i]->setVisible(false);
					}
				}
			}

			setMatchedBackground(m_pageStack[m_activePage]->getStackLevel());

			break;
		}

		case EOpenMenuHideAllShowRoot:
		{
			// The active page was transitioning
			m_pageStack[m_activePage]->onTransitionComplete(setIdle);

			// make the parent page invisible
			if (m_activePage > 0)
			{
				for (int i = m_activePage - 1; i > 0; --i)
				{
					// We only tweened pages to the hide state if
					// there were not already hidden.
					if (m_pageStack[i]->isTransitioning())
					{
						m_pageStack[i]->onTransitionComplete(setIdle);

						m_pageStack[i]->setVisible(false);
					}
				}

				// Now handle the root
				if (m_pageStack[0]->isTransitioning())
				{
					m_pageStack[0]->onTransitionComplete(setIdle);

					m_pageStack[0]->setVisible(true);
				}
			}

			setMatchedBackground(m_pageStack[m_activePage]->getStackLevel());

			break;
		}

		case EResetToTop:
		{
			// The active page was transitioning
			m_pageStack[m_activePage]->onTransitionComplete(setIdle);

			setMatchedBackground(m_pageStack[m_activePage]->getStackLevel());

			break;
		}

		case EAnimateActivePage:
		{
			// The active page was transitioning
			m_pageStack[m_activePage]->onTransitionComplete(setIdle);

			break;
		}

		case EShowStaticPreview:
		{
			// The active page was transitioning
			m_pageStack[m_activePage]->onTransitionComplete(setIdle);
#if defined(NDHS_JNI_INTERFACE)
			m_pageStack[m_activePage]->componentOnTransitionComplete(setIdle);
#endif
			break;
		}

		case ENoAction:
		{
			break;
		}

		case EHideToShow:
		{
			// Any page could be involved
			for (int i = m_activePage; i >= 0; --i)
			{
				if (m_pageStack[i]->isTransitioning())
				{
					m_pageStack[i]->onTransitionComplete(setIdle);
				}
			}

			break;
		}

		case ESkipToClose:
		{
			// Any above the new active page could be involved
			for (int i = (int)m_pageStack.size() - 1; i > m_activePage; --i)
			{
				if (m_pageStack[i]->isTransitioning())
				{
					m_pageStack[i]->onTransitionComplete(setIdle);

					m_pageStack[i]->setVisible(false);
				}
			}

			break;
		}

		case EBackFromLink:
		{
			// The active page was transitioning
			m_pageStack[m_activePage]->onTransitionComplete(setIdle);

			break;
		}

		default:
			break;
	}

	m_action = ENoAction;

	if (m_primaryLightOverride)
	{
		// Make sure the light is updated with its final placement
		LcTPlacement pl;
		m_primaryLightPath->getPlacement(1.0, pl);
		int mask = m_primaryLightPath->getMask();
		getSpace()->setPrimaryLightPlacement(pl, mask);

		// Turn off the override
		m_primaryLightOverride = false;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::jumpTransitionToEnd()
{
	// Iterate through the menu stack from the top page
	// and tell the pages to stop animating
	//
	// Note: the page could be doing a static animation

	for (int i = (int)m_pageStack.size() - 1; i >=0; i--)
	{
		if (m_pageStack[i]->isTransitioning())
		{
			m_pageStack[i]->jumpTransitionToEnd(false);
			m_pageStack[i]->componentsJumpTransitionToEnd(false);
		}
	}

	// Now do the 'on transition complete' step.
	onTransitionComplete(false);

	// Set a timer to trigger destroyUnwantedPages
	if(m_destroyUnwantedPagesMessage.isScheduled())
	{
		m_destroyUnwantedPagesMessage.cancel();
	}

	m_destroyUnwantedPagesMessage.scheduleImmediate( getSpace()->getTimer(), 0);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::setIdle()
{
	// Iterate through the menu stack from the top page
	// and tell the pages to stop everything

	stopAllFields();

	for (int i = (int)m_pageStack.size() - 1; i >=0; i--)
	{
		m_pageStack[i]->jumpTransitionToEnd(true);
		m_pageStack[i]->componentsJumpTransitionToEnd(true);
	}

	// Now do the 'on transition complete' step.
	onTransitionComplete(true);

	destroyUnwantedPages(false);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::resumeStaticAnimations()
{
	// Iterate through page stack and let them know to resume
	// static animations
	for (int i = (int)m_pageStack.size() - 1; i >=0; i--)
	{
		m_pageStack[i]->resumeStaticAnimations();
	}
}

/*-------------------------------------------------------------------------*//**
	Clean the page stack
*/
void NdhsCPageManager::destroyPageStack()
{
	for (int i = m_pageStack.size() - 1; i >= 0; i--)
	{
		m_pageStack[i]->retire();
	}

	//This should clean the stack completly
	m_activePage = -1;
	destroyUnwantedPages(true);

	// Deal with background image
	getSpace()->setBackgroundImage("");
}

/*-------------------------------------------------------------------------*//**
	If forcing deletion, make sure that all animators are stopped first
*/
void NdhsCPageManager::destroyUnwantedPages(bool force)
{
	bool pageDeleted = false;
	// Iterate through the menu stack from the top page
	// down to one above the active page and destroy them
	// if they are ready to be destroyed

	for (int i = (int)m_pageStack.size() - 1; i > m_activePage; i--)
	{
		NdhsCPageModel* pageModel = m_pageStack[i];

		// Either force deletion or ask the page
		// if it is ready to be deleted
		if (force || (pageModel && pageModel->canBeDeleted()))
		{
			NdhsCPageTemplate* pageTemplate = (NdhsCPageTemplate*)pageModel->getTemplate();

			LcTmOwnerArray<NdhsCPageModel>::iterator it = m_pageStack.begin();
			for(; it < m_pageStack.end(); it++)
			{
				if (pageModel == *it)
				{
#ifdef LC_USE_STYLUS
					// Clear element list when we are about to destroy page
					// so that element list does not contain invalid page ref
					if (m_activePage == -1)
						onMouseCancel(NULL, NULL, false);
#endif
					m_pageStack.erase(it);
					pageDeleted = true;
					break;
				}
			}
			releaseTemplate(pageTemplate);
		}
		else
		{
			// As soon as we hit a page that is not ready
			// for deletion, stop so that we cant end up
			// with holes in the stack
			break;
		}
	}

	if (pageDeleted)
	{
		int stackLevel = 0;

		// Now resize the page stack to get rid of the destroyed elements
		if ((int)m_pageStack.size() > 0)
		{
			stackLevel = m_pageStack[m_pageStack.size() - 1]->getStackLevel();
		}

		// Now get rid of any token stack levels that are no longer needed
		if (m_tokenStack)
			m_tokenStack->popTokensToLevel(stackLevel);
		if (m_manifestStack)
			m_manifestStack->popManifestToLevel(stackLevel);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::launchItem(NdhsCMenuItem* item, bool menuItem, int stackLevel)
{
	// Open the package if its a menu addition
	if (item->isMenuAddition())
	{
		if (false == openPackage(item->getPackagePath()))
		{
			m_con->displayWarning("Unable to open package!");
			return false;
		}
		else
		{
			stackLevel = m_manifestStack->getStackHeight() - 1;
		}
	}

	ENdhsLinkType linkType = getCon()->getTRLinkType(item->getTRLinkPrefix(m_tokenStack.ptr(), stackLevel));

	bool retValue = true;

	switch (linkType)
	{
		case ENdhsLinkTypeTheme:
		{
#ifndef NDHS_PREVIEWER
			// Cant allow re-theming in the previewer
			// Must be done via the dialog

			// Get rid of the package we just opened,
			// loadTheme will open it again
			if (item->isMenuAddition())
			{
				destroyUnwantedPages(true);

				// Must set a timer so that we can perform this
				// from a clean execution thread.  We are going to delete
				// all page models, and one of them is waiting for this
				// function to return.
				if(m_openThemeMessage.isScheduled())
				{
					m_openThemeMessage.cancel();
				}

				m_openThemeMessage.schedule( getSpace()->getTimer(), (int)item, 0);
				retValue = true;
			}
			else
			{
				// Only a menu addition can have a theme link
				// This is a bug in the theme if we get here
				retValue = false;
			}
#endif
			break;
		}

		case ENdhsLinkTypePage:
		{
			LcTaString linkUri = item->getLinkAttr();
			LcTaString linkPrefix, linkBody;

			// Extract link prefix and body for later use
			if (extractLinkPrefixAndLinkBody(linkUri, linkPrefix, linkBody))
			{
				// Check whether we this is legal navigation or not
				if (m_pageStack[m_activePage]->checkNaviation(linkBody))
				{
					// Should we display a warning if this fails???
					retValue = openPage(item, true, menuItem, stackLevel);

					if (!retValue)
					{
						destroyUnwantedPages(true);
						m_con->displayWarning("Unable to open page!");
					}
				}
				else
				{
					retValue = false;
				}
			}
			break;
		}

		case ENdhsLinkTypeUnknown:
		{
			retValue = false;
			break;
		}

		default:
		{
			m_action = ELaunchLink;
			m_itemToLaunch = item;

			// Let it use its own transition time
			m_terminalTime = m_pageStack[m_activePage]->getDefaultTerminalTime();
			m_terminalVelocityProfile = m_pageStack[m_activePage]->getDefaultTerminalVelocityProfile();

			// The page works out the active slot itself, so we can just pass in 0
			m_animatingStateChange = m_pageStack[m_activePage]->changeState(ENdhsPageStateLaunch, 0, NULL, true);

			// For async link types, engage ui event blocking straight away
			if (linkType == ENdhsLinkTypeAsyncLinkPlugin)
				m_con->engageAsyncLinkBlocking();

			if (!m_animatingStateChange)
				onTransitionComplete(false);
#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
			else
				getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_STARTED);
#endif

			retValue = true;
			break;
		}
	}

	// Default drop through.
	return retValue;
}

#if defined(NDHS_JNI_INTERFACE)

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::setEngineState(LcTmString package, LcTmString link, LcTmString templ, LcTmString layout, int slot, bool animate,
										LcTmString componentClassName, LcTmString componentPath, LcTmString componentLayoutName, bool editorSwitched)
{
	if (layout.isEmpty())
		return false;

	int matchStackLevel = -1;
	int nestedComponentLevel = getNestedComponentLevel();
	m_isCyclic = false;
	m_inStaticPreviewMode = true;
	m_identifier = 2;

	LcCSpace* space = m_con->getSpace();
	// Retain all bitmaps during page destroy, as we are going to use them in next step
	if (space)
	{
		space->retainResource(true);
	}

	m_activePage = -1;
	destroyUnwantedPages(true);

	if(componentClassName.isEmpty() == false && componentPath.isEmpty() == false && componentLayoutName.isEmpty() == false)
	{
		m_inplaceMode = true;

		LcTaOwner<NdhsCPageManager::CStaticPreviewCache> cacheEntry = NdhsCPageManager::CStaticPreviewCache::create();

		if (cacheEntry)
		{
			cacheEntry->slotNumber = slot;
			cacheEntry->componentClassName = componentClassName;
			cacheEntry->componentPath = componentPath;
			cacheEntry->layoutName = componentLayoutName;
			cacheEntry->identifier = m_currentStateSnapshot.size() + 1;
			setInPlacePreviewCacheEntry(cacheEntry.ptr());
		}
	}
	else
	{
		m_inplaceMode = false;
	}

	if (package.length() > 0)
	{
		LcTaString packagePath = m_con->getAppPath() + m_con->getThemePath() + package;

		if (openPackage(packagePath))
		{
			// Load the package file
			LcTaString err;
			LcTaString localPath = packagePath + NDHS_PACKAGE_FILENAME;
			#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
				localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
			#endif
			LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(localPath, err);
			loadPalette(root.ptr());
		}
	}

	LcTaString outLinkData;

	int stackLevel = m_manifestStack->getStackHeight() - 1;

	NdhsCPage* pageNode = NULL;
	LcTaOwner<NdhsCMenuItem> mi;

	mi = NdhsCMenuItem::create(NULL);
	mi->setLinkAttr(link);

	pageNode = getPage(mi.ptr(), stackLevel);

	if (pageNode == NULL)
	{
		if (space)
			space->retainResource(false);
		return false;
	}

	if (editorSwitched && m_con)
		m_con->configureScreenCanvas(false);

	NdhsCPageTemplate* pageTemplate = NULL;

	LcTaString templateFile = templ;

	LcTaString outTemplateFile;

	// Get palette name.
	NdhsCManifest* paletteMan = getPaletteManifest(templateFile);

	if (m_manifestStack->findFile(templateFile, outTemplateFile, paletteMan, stackLevel))
	{
		templateFile = outTemplateFile;

		// We dont need cached template file here, other wise element update in editor
		// will not be visible
		LcTaOwner<CTemplateItem> ptItem = CTemplateItem::create();
		ptItem->templateFile = NdhsCPageTemplate::create(this, templateFile);
		ptItem->templateFile->skipFirstActiveCheck();
		if (ptItem->templateFile && ptItem->templateFile->configureFromXml(m_designSize, stackLevel, nestedComponentLevel))
		{
			CTemplateItem* ptItemPtr = ptItem.ptr();
			ptItem->refCount = 1;
			ptItem->stackLevel=stackLevel;
			ptItem->key=templateFile;
			m_templates.add_element(templateFile, ptItem);
			pageTemplate = (NdhsCPageTemplate*)ptItemPtr->templateFile.ptr();
		}
		else
		{
			ptItem->templateFile.destroy();
			ptItem.destroy();
		}
	}

	if (pageTemplate == NULL)
	{
		if (space)
			space->retainResource(false);

		return false;
	}

	setupPrimaryLightTransitionOverrideFlag(m_activePage + 1, m_activePage);

	// override the selectable/active settings
	if (slot != -1)
		pageTemplate->setSelectable(slot, slot, slot);

	// For the new page, we use the top stack frame
	stackLevel = m_manifestStack->getStackHeight() - 1;

	// Must delete any pages marked for destruction here before we mess with the
	// page stack
	if	(m_destroyUnwantedPagesMessage.isScheduled())
	{
		m_destroyUnwantedPagesMessage.cancel();
		destroyUnwantedPages(true);
	}

	// Create the page model
	LcTaOwner<NdhsCPageModel> page = NdhsCPageModel::create(this, pageTemplate, pageNode, link, stackLevel, templ);
	if (page.ptr() == NULL)
	{
		if (space)
			space->retainResource(false);

		// failed to create the page model
		releaseTemplate(pageTemplate);
		return false;
	}
	NdhsCPageModel* currentPage = page.ptr();
	m_pageStack.push_back(page);

	m_activePage++;

	currentPage->postConstruct();

	// Bypass the State Manager and set the layout to the one supplied.
	m_pageStack[m_activePage]->forceLayout(layout);

	m_action = ENoAction;

	m_con->updateScreenFurniture();

	setMatchedBackground(stackLevel);

	m_animatingStateChange = m_pageStack[m_activePage]->changeState(ENdhsPageStateInteractive, slot, NULL, animate, true);

	setupPrimaryLightTransitionOverrideTiming(m_activePage, m_activePage - 1);

	// Note that active page will need an 'onTransitionComplete'
	m_action = EShowStaticPreview;
	onTransitionComplete(false);

	if (space)
		space->retainResource(false);

	m_inStaticPreviewMode = false;

	return true;
}

#endif //NDHS_JNI_INTERFACE

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::openPage(NdhsCMenuItem* item, bool animate, bool menuItem, int stackLevel)
{

	volatile int memError = 0;
	volatile bool success = true;
	NdhsCPageTemplate* pageTemplate = NULL;
	volatile NdhsCPageTemplate* pCleanupTemplate = NULL;
	volatile bool bCleanupPage = false;
	volatile bool bCleanupReapplyStates = false;
	LcTaString	templatePath;

#ifdef IFX_ENABLE_BENCHMARKING_FRAME_RATE
	getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_OPEN_PAGE_STARTED);
#endif/*IFX_ENABLE_BENCHMARKING_FRAME_RATE*/

	// Prepare cleanup frame
	LC_CLEANUP_PUSH_FRAME(memError);

	if (memError == 0)
	{

		NdhsCPage* uiNode = getPage(item, stackLevel);

		if (!uiNode)
			success = false;  // Couldn't load associated node

		if (success)
		{
			// Check for 'first page open' tasks - note that configureScreenCanvas
			// must be called before the template is loaded in case custom shaders
			// are used (in the OGL2 case)
			if (m_activePage == -1)
			{
#ifdef LC_PLAT_OGL
				// Destroy the current background on
				// OpenGL before its handle is invalidated
				getSpace()->setBackgroundImage("");
#endif
				// This must be the root page, so configure the canvas
				m_con->configureScreenCanvas(false);

				// Reset current background var
				m_currentBackground = "";
			}
		}

		if (success)
		{
			// Now we must load the page template
			pageTemplate = getPageTemplate(uiNode, stackLevel, templatePath);
			// The template is reference counted, so note that we need
			// to remove this reference on an OOM exception
			pCleanupTemplate = pageTemplate;
			if (pageTemplate == NULL)
				success = false;
		}


		if (success)
		{

			// Must delete any pages marked for destruction here before we mess with the
			// page stack
			if	(m_destroyUnwantedPagesMessage.isScheduled())
			{
				m_destroyUnwantedPagesMessage.cancel();
				destroyUnwantedPages(true);
			}

			// For the new page, we use the top stack frame
			stackLevel = m_manifestStack->getStackHeight() - 1;

			// Create the page model
			LcTaOwner<NdhsCPageModel> page = NdhsCPageModel::create(this, pageTemplate, uiNode, item->getLinkAttr(), stackLevel, templatePath);
			if (page.ptr() == NULL)
			{
				// failed to create the page model
				releaseTemplate(pageTemplate);
				success = false;
			}
			else
			{
				NdhsCPageModel* currentPage = page.ptr();

				m_pageStack.push_back(page);
				m_activePage++;

				// Inflate Page now, it is required to inflate page here so that when child
				// ask for getMenu(), we should have a page on stack
				currentPage->postConstruct();

				// Note that the page is now on the page stack in case of a
				// subsequent OOM exception
				bCleanupPage = true;
			}
		}

		if (success)
		{
			// Initialize state to open so that all widgets get their initial placements
			setupPrimaryLightTransitionOverrideFlag(m_activePage, m_activePage - 1);
			m_action = ENoAction;
			m_pageStack[m_activePage]->changeState(ENdhsPageStateOpen, 0, NULL, false);


			// Let the con update the softkey menu
			m_con->updateScreenFurniture();

			// Use the terminal time of the sub menu

			// Past this point, we can't guarantee the correctness of the states of the
			// existing page stack, so note that we'll have to reset them on an OOM
			// error
			bCleanupReapplyStates = true;

			// Now update the states of the other open menus
			updatePageStates(animate, menuItem, stackLevel);

			// Change the state of the new page
			// The page works out the active slot itself, so we can just pass in 0
			m_animatingStateChange |= m_pageStack[m_activePage]->changeState(ENdhsPageStateInteractive, 0, NULL, animate);

			setupPrimaryLightTransitionOverrideTiming(m_activePage, m_activePage - 1);

			// The active page might need an 'onTransitionComplete'
			if (m_action == ENoAction)
				m_action = EAnimateActivePage;

			if (!m_animatingStateChange)
				onTransitionComplete(false);
#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
			else
				getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_STARTED);
#endif

			// Set up the background transition
			setupChangeBackgroundMessage(m_activePage);

		}

		// Headroom calculation
		if (success)
		{
			// Widgets
			int totalHeadroom = getSpace()->getWidgetCount() * 500;

			totalHeadroom += 600 + pageTemplate->getFurnitureElementCount() * 200;

			// Plugin refresh queue
			totalHeadroom += 1040;

			// Try to temporarily allocate the memory
			LcTaAlloc<LcTByte> tempBuffer(totalHeadroom);
		}
	}
	else
	{
		if (memError == IFX_ERROR_GRAPHICS)
		{
			IFXP_Display_Error_Note((IFX_WCHAR*)L"Graphics Error - Internal graphics error occured");
		}
		else if (memError == IFX_ERROR_MEMORY)
		{
			IFXP_Display_Error_Note((IFX_WCHAR*)L"Out Of Memory - Could not allocate resources");
		}
		else
		{
			IFXP_Display_Error_Note((IFX_WCHAR*)L"General Error - An internal error occured");
		}

		if (bCleanupPage)
		{
			// pop menu off the page stack - this will also release the template
			m_activePage--;
			destroyUnwantedPages(true);
			stackLevel = m_manifestStack->getStackHeight() - 1;
		}
		else if (pCleanupTemplate)
		{
			// Release the template reference manually
			NdhsCPageTemplate* pT = (NdhsCPageTemplate*) pCleanupTemplate;
			releaseTemplate(pT);
		}

		if (bCleanupReapplyStates && (m_activePage != -1))
		{
			// remaining page states may be incorrect, so re-apply.  This
			// should work now a page has been destroyed.
			m_pageStack[m_activePage]->changeState(ENdhsPageStateOpen, 0, NULL, false);

			updatePageStates(false, menuItem, stackLevel);

			m_pageStack[m_activePage]->changeState(ENdhsPageStateInteractive, 0, NULL, false);
		}

		// Schedule a repaint to clean up the error note
		getSpace()->revalidate();
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memError);

	// Check to see if we suffered an OOM error
	if (memError != 0)
	{
		if (memError == IFX_ERROR_GRAPHICS)
		{
			LC_CLEANUP_THROW(IFX_ERROR_GRAPHICS);
		}
		success = false;
	}

#ifdef IFX_ENABLE_BENCHMARKING_FRAME_RATE
	getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_OPEN_PAGE_COMPLETE);
#endif/*IFX_ENABLE_BENCHMARKING_FRAME_RATE*/

	return success;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPage* NdhsCPageManager::getPage(NdhsCMenuItem* item, int stackLevel)
{
	if (item == NULL)
		return NULL;

	NdhsCPage* uiNode = NULL;

	if (item->getLinkAttr().isEmpty() == false)
	{
		LcTaString link = item->getLinkAttr().toLower();

		// Lets see if its been cached
		TmANodeCache::iterator it = m_nodes.find(link);

		if (it != m_nodes.end())
		{
			// Its cached, so increment the ref count and return it
			CNodeItem* ptItemPtr = it->second;
			ptItemPtr->refCount++;
			uiNode = ptItemPtr->uiNode.ptr();
		}
		else
		{
			// Not cached, so create a new page
			LcTaOwner<NdhsCPage> newPage = loadPage(item, stackLevel);
			if (newPage)
			{
				LcTaOwner<CNodeItem> ptItem = CNodeItem::create();
				ptItem->uiNode = newPage;
				CNodeItem* ptItemPtr = ptItem.ptr();
				ptItem->refCount = 1;
				ptItem->stackLevel = stackLevel;
				m_nodes.add_element(link, ptItem);

				uiNode = ptItemPtr->uiNode.ptr();
			}
		}
	}

	return uiNode;
}

/*-------------------------------------------------------------------------*//**
	Component will this funciton for loading menu
*/
LcTaOwner<NdhsCPage>
NdhsCPageManager::loadPage(NdhsCMenuItem* item, int stackLevel)
{
	LcTaOwner<NdhsCPage> page = getCon()->getPage();
	ENdhsLinkType linkType = getCon()->getTRLinkType(item->getTRLinkPrefix(m_tokenStack.ptr(), stackLevel));
	LcTaString linkData    = item->getTRLinkData(m_tokenStack.ptr(), stackLevel);
	bool pageLoaded = false;

	// Open the page so that we can get a name to pick the correct page template
	if (linkType == ENdhsLinkTypePage)
	{
		int matchStackLevel = -1;

		LcTaString outLinkData;
		LcTaString ext;
		LcTaString linkPrefix, linkBody;
		LcTaString uri = item->getLinkAttr();
		page->setPageLink (uri);

		if (extractLinkPrefixAndLinkBody(uri, linkPrefix, linkBody))
			linkData = linkBody;

#ifdef NDHS_RESOURCE_FILE_EXTENSION_REPLACEMENT
			// Get file extension from file
		ext = linkData.getWord(-1, '.');

		// Before loading replace extension for our file type
		linkData = linkData.subString(0, linkData.length() - ext.length()) + "node";
#endif

		// Get palette name.
		NdhsCManifest* paletteMan = getPaletteManifest(linkData);

		if (m_manifestStack->findFile(linkData, outLinkData, paletteMan, stackLevel, &matchStackLevel))
		{
			linkData = outLinkData;

			LcTaString packageName = m_manifestStack->getManifest(matchStackLevel)->getPackageName();

			if (page->openPageFromXml(packageName, linkData))
			{
				pageLoaded = true;
			}
		}
	}

	if (!pageLoaded)
		page.destroy();

	return page;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager::CMenuItem> NdhsCPageManager::CMenuItem::create()
{
	LcTaOwner<CMenuItem> ref;
	ref.set(new CMenuItem());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::CMenuItem::addReference(NdhsCMenuComponent* newRef)
{
	if (newRef == NULL)
		return;

	LcTmArray<NdhsCMenuComponent*>::iterator it = refBy.begin();
	for(; it != refBy.end(); it++)
	{
		if ((*it) == newRef)
		{
			// Component already in reference list, don't add
			return;
		}
	}

	refBy.push_back(newRef);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::CMenuItem::removeReference(NdhsCMenuComponent* ref)
{
	LcTmArray<NdhsCMenuComponent*>::iterator it = refBy.begin();
	for(; it != refBy.end(); it++)
	{
		if ((*it) == ref)
		{
			refBy.erase(it);
			break;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::CMenuItem::notifyMenuReload(bool firstPhase)
{
	// Careful here - the refresh process may lead menu components
	// in the refBy array to de-register from the menu (including the
	// component being refreshed at the time, thus invalidating
	// any iterator we are currently using.
	// We take a copy of the refBy array and notify each component
	// in the copy, first checking that it is still in the master
	// each time

	LcTaArray<NdhsCMenuComponent*> refByCopy(refBy);

	LcTaArray<NdhsCMenuComponent*>::iterator copyIt = refByCopy.begin();
	for(; copyIt != refByCopy.end(); copyIt++)
	{
		bool skip = true;
		LcTmArray<NdhsCMenuComponent*>::iterator masterIt = refBy.begin();
		for(; masterIt != refBy.end(); masterIt++)
		{
			if ((*copyIt) == (*masterIt))
				skip = false;
		}

		// If we couldn't find the menu component in the master list
		// then it's probably been destroyed and certainly shouldn't
		// be refreshed!
		if (skip)
			continue;

		if (firstPhase)
			(*copyIt)->refreshMenuFirstStep();
		else
			(*copyIt)->refreshMenuSecondStep();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::CMenuItem::notifyActiveItemChange(int newActiveItem)
{
	// To guard against refBy being updated during this call
	// we take a copy of the refBy array and notify each component
	// in the copy, first checking that it is still in the master
	// each time - may avoid a nasty bug.
	LcTaArray<NdhsCMenuComponent*> refByCopy(refBy);


	LcTaArray<NdhsCMenuComponent*>::iterator copyIt = refByCopy.begin();
	for(; copyIt != refByCopy.end(); copyIt++)
	{
		bool skip = true;
		LcTmArray<NdhsCMenuComponent*>::iterator masterIt = refBy.begin();
		for(; masterIt != refBy.end(); masterIt++)
		{
			if ((*copyIt) == (*masterIt))
				skip = false;
		}

		// If we couldn't find the menu component in the master list
		// then it's probably been destroyed.
		if (skip)
			continue;

		(*copyIt)->requestSetActiveItem(newActiveItem);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager::CNodeItem> NdhsCPageManager::CNodeItem::create()
{
	LcTaOwner<CNodeItem> ref;
	ref.set(new CNodeItem());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Component will this function for loading menu
*/
bool
NdhsCPageManager::expandLink(const LcTmString& menuLink, LcTmString& expandedLink, int stackLevel)
{
	NdhsCPage* pageNav = NULL;
	bool status = false;

	switch(menuToLoadFrom(menuLink))
	{
		case ENdhsModule:
		case ENdhsMenuFile:
		{
			//							PackageName											  Menu
			expandedLink = m_manifestStack->getManifest(stackLevel)->getPackageName() + "_" + menuLink;
			status = true;
			break;
		}
		case ENdhsOtherNode:
		{
			pageNav = getPageByLink(menuLink, stackLevel);
			if (pageNav)
			{
				//							PackageName												PageFile						Menu
				expandedLink = m_manifestStack->getManifest(stackLevel)->getPackageName() + "_" + pageNav->getPageLink() + "_" + menuLink;
				status = true;
			}
			break;
		}
		case ENdhsCurrentNode:
		{
			// Get page node
			pageNav = m_pageStack[m_activePage]->getPage();
			if (pageNav)
			{
				//							PackageName												PageFile						Menu
				expandedLink = m_manifestStack->getManifest(stackLevel)->getPackageName() + "_" + pageNav->getPageLink() + "_" + menuLink;
				status = true;
			}
			break;
		}
		default:
		{
			status = false;
			// Invalid Link
			break;
		}
	}
	return status;
}

/*-------------------------------------------------------------------------*//**
	Component will this funciton for loading menu
*/
LcTaOwner<NdhsCMenu>
NdhsCPageManager::loadMenu(const LcTmString& menuLink, volatile bool& menuLoaded, NdhsCManifest* palManifest, int stackLevel)
{
	// First see if we need to load menu from page file
	LcTaOwner<NdhsCMenu> menu;

	switch(menuToLoadFrom(menuLink))
	{
		case ENdhsModule:
		case ENdhsMenuFile:
		{
			menu = loadMenuFromXML(menuLink, menuLoaded, palManifest, stackLevel);
			break;
		}
		case ENdhsOtherNode:
		{
			menu = loadMenuFromPageFile(menuLink, menuLoaded, false, stackLevel);
			break;
		}
		case ENdhsCurrentNode:
		{
			menu = loadMenuFromPageFile(menuLink, menuLoaded, true, stackLevel);
			break;
		}
		default:
		{
			// Invalid Link
			break;
		}
	}
	return menu;
}

/*-------------------------------------------------------------------------*//**
	Load menu from XML
*/
 LcTaOwner<NdhsCMenu>
NdhsCPageManager::loadMenuFromXML(const LcTmString& menuLink, volatile bool& menuLoaded, NdhsCManifest* palManifest, int stackLevel)
{
	LcTaOwner<NdhsCMenuItem> item = NdhsCMenuItem::create(NULL);
	item->setLinkAttr(menuLink);

	LcTaOwner<NdhsCMenu> menu = m_con->getMenu();
	ENdhsLinkType linkType = m_con->getTRLinkType(item->getTRLinkPrefix(m_tokenStack.ptr(), stackLevel));
	LcTaString linkData    = item->getTRLinkData(m_tokenStack.ptr(), stackLevel);

	// Dirty the field cache
	setFieldCacheDirty();

	// Open the menu so that we can get a name to pick the correct page template
	if (linkType == ENdhsLinkTypeMenu)
	{
		int matchStackLevel = -1;

		LcTaString outLinkData;
		LcTaString ext;
		LcTaString linkPrefix, linkBody;
		LcTaString uri = item->getLinkAttr();

#ifdef NDHS_RESOURCE_FILE_EXTENSION_REPLACEMENT
			// Get file extension from file
		ext = linkData.getWord(-1, '.');

		// Before loading replace extension for our file type
		linkData = linkData.subString(0, linkData.length() - ext.length()) + "menu";
#endif

		if (m_manifestStack.ptr()->findFile(linkData, outLinkData, palManifest, stackLevel, &matchStackLevel))
		{
			linkData = outLinkData;

			LcTaString packageName = NDHS_MAIN_PACKAGE_NAME;

			if (matchStackLevel != -1)
				packageName = m_manifestStack.ptr()->getManifest(matchStackLevel)->getPackageName();

			if (menu->openMenuFromXml(packageName, linkData))
			{
				menu->setMenuLink(menuLink);
				menuLoaded = true;
			}
		}
	}
	else if (linkType == ENdhsLinkTypeMenuPlugin)
	{
		NdhsCPlugin* plugin = m_con->getTRLinkTypePlugin(item->getTRLinkPrefix(m_tokenStack.ptr(), stackLevel));

		if (plugin)
		{
			// In the future when packages can contain plugins,
			// the package name will depend on which package owns the plugin
			LcTaString packageName = NDHS_MAIN_PACKAGE_NAME;

			if (menu->openMenuFromPlugin(packageName, item->getTRLink(m_tokenStack.ptr(), stackLevel)))
			{
				menuLoaded = true;
			}
		}
	}

	// Now we can populate the menu
	if (menuLoaded)
	{
		menuLoaded = menu->populateMenu();
	}

	if (!menuLoaded)
		menu.destroy();

	return menu;
}

/*-------------------------------------------------------------------------*//**
	Load menu from page file
*/
 LcTaOwner<NdhsCMenu>
NdhsCPageManager::loadMenuFromPageFile(const LcTmString& menuLink, volatile bool& menuLoaded, bool nodeInfo, int stackLevel)
{
	LcTaOwner<NdhsCMenu> menu = getCon()->getMenu();
	NdhsCPage* pageNav = NULL;
	LcTaString mLink=menuLink;
	LcTaString linkPrefix="";
	LcTaString linkBody="";

	// If we are here it mean valid link, no need to check again parameters

	// True, mean get menu from current pageNav node of current page
	// False mean get menu from specified pageNav node
	if (nodeInfo)
	{
		// Get page node
		pageNav = m_pageStack[m_activePage]->getPage();
		mLink=pageNav->getPageLink();
		if(extractLinkPrefixAndLinkBody(mLink,linkPrefix,linkBody))
		{
			mLink="menu://"+linkBody;
		}
	}
	else
	{
		pageNav = getPageByLink(menuLink, stackLevel);
	}

	if (pageNav)
	{
		LcTaString key= "name", value;
		if (getParamterByName(menuLink, key, value))
		{
			if (stackLevel > 0)
				stackLevel--;

			LcTaString packageName = m_manifestStack.ptr()->getManifest(stackLevel)->getPackageName();


			if (menu->populateMenuFromPageFile(packageName, pageNav->getPageFile(), value) == false)
			{
				menuLoaded = false;
			}
			else
			{
				if(nodeInfo)
				{
					mLink=mLink.bufUtf8()+("?name="+value);
				}
				menu->setMenuLink(mLink);
				menuLoaded = true;
			}
		}
	}
	return menu;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPage* NdhsCPageManager::getPageByLink(const LcTmString& link, int stackLevel)
{
	NdhsCPage* retVal = NULL;
	LcTaString linkPrefix, linkBody;
	LcTaString firstLinkBody;

	if (extractLinkPrefixAndLinkBody(link, linkPrefix, firstLinkBody) == false)
		return retVal;

	NdhsCPage* page = NULL;
	page = m_pageStack[m_activePage]->getPage();

	if (page)
	{
		if (extractLinkPrefixAndLinkBody(page->getPageLink(), linkPrefix, linkBody))
		{
			if (linkBody.compareNoCase(firstLinkBody) == 0)
			{
				retVal = page;
			}
			else
			{
				if (page->isPageInList(firstLinkBody))
				{
					NdhsCPage* pageNode;
					LcTaOwner<NdhsCMenuItem> mi;

					mi = NdhsCMenuItem::create(NULL);
					mi->setLinkAttr("node://" + firstLinkBody);
					pageNode = getPage(mi.ptr(), stackLevel);

					if (pageNode)
					{
						retVal = pageNode;
					}
				}
			}
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Component will use this function for retreving a menu
*/
NdhsCMenu*
NdhsCPageManager::getMenu(const LcTmString& menuLink, volatile bool& menuLoaded, NdhsCManifest* palManifest, int stackLevel, NdhsCMenuComponent *caller)
{
	LcTmString expandedLink = "";

	// If we are not able to expand link go back
	if (expandLink(menuLink, expandedLink, stackLevel) == false)
		return NULL;

	NdhsCMenu* uiMenu = NULL;

	// Lets see if its been cached
	TmMMenuCache::iterator it = m_menus.find(expandedLink.toLower());

	if (it != m_menus.end())
	{
		// Its cached, so increment the ref count and return it
		CMenuItem* ptItem = it->second;
		ptItem->addReference(caller);
		menuLoaded = true;
		uiMenu = ptItem->menu.ptr();
	}
	else
	{
		// Not cached, so create a new menu
		LcTaOwner<CMenuItem> ptItem = CMenuItem::create();
		LcTaOwner<NdhsCMenu> newMenu = loadMenu(menuLink, menuLoaded, palManifest, stackLevel);
		ptItem->menu = newMenu;
		if (ptItem->menu)
		{
			CMenuItem* ptItemPtr = ptItem.ptr();
			ptItem->addReference(caller);
			ptItem->stackLevel=stackLevel;
			ptItem->paletteManifest=palManifest;
			m_menus.add_element(expandedLink.toLower(), ptItem);
			menuLoaded = true;
			uiMenu = ptItemPtr->menu.ptr();
		}
		else
		{
			// Failed to create the menu
			ptItem->menu.destroy();
			ptItem.destroy();
		}
	}
	return uiMenu;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager::CPaletteItem> NdhsCPageManager::CPaletteItem::create()
{
	LcTaOwner<CPaletteItem> ref;
	ref.set(new CPaletteItem());
	//ref->construct();
	return ref;
}

#if defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager::CStaticPreviewCache> NdhsCPageManager::CStaticPreviewCache::create()
{
	LcTaOwner<CStaticPreviewCache> ref;
	ref.set(new CStaticPreviewCache());
	//ref->construct();
	return ref;
}
#endif

/*-------------------------------------------------------------------------*//**
	This will retrieve the manifest for a specified palette.
*/
NdhsCManifest* NdhsCPageManager::getPaletteManifest(const LcTmString& palettePath)
{
	NdhsCManifest* foundManifest = NULL;

	// Extract the palettes/<paletteName>
	int paletteNamePos = palettePath.find(NDHS_DIR_SEP);
	paletteNamePos = palettePath.find(NDHS_DIR_SEP, paletteNamePos + 1);
	LcTaString paletteName = palettePath.subString(0, paletteNamePos).toLower();

	// Lets see if its been loaded.
	TmMPaletteCache::iterator it = m_paletteMap.find(paletteName);
	if (it != m_paletteMap.end())
	{
		// Get the manifest.
		CPaletteItem* ptItem = it->second;
		if (ptItem)
		{
			foundManifest = ptItem->paletteManifest.ptr();
		}
	}

	return foundManifest;
}

/*-------------------------------------------------------------------------*//**
*/
ENdhsMenuToLoadFrom
NdhsCPageManager::menuToLoadFrom(const LcTmString& menuLink)
{
	// Link should not be empty
	if (menuLink.isEmpty())
		return ENdhsUnknown;

	LcTaString linkPrefix;
	LcTaString linkBody;
	LcTaString linkUri = menuLink;

	ENdhsMenuToLoadFrom loadFrom = ENdhsUnknown;

	if (extractLinkPrefixAndLinkBody(linkUri, linkPrefix, linkBody))
	{
		// Check if we need to get menu from XML
		if (linkPrefix.compareNoCase("menu") == 0 &&  linkBody.find( ".menu", 0) != -1)
		{
			loadFrom = ENdhsMenuFile;
		}
		// Check if we need to get menu from  page file
		else if (linkPrefix.compareNoCase("menu") == 0 &&  linkBody.find( ".node", 0) != -1)
		{
			loadFrom = ENdhsOtherNode;
		}
		// Check if we need to get menu from  current page file
		else if (linkPrefix.compareNoCase("menu") == 0 &&  linkBody.find( ".node", 0) == -1)
		{
			loadFrom = ENdhsCurrentNode;
		}
		// Probably a dynamic menu
		else if (linkPrefix.isEmpty() == false &&  linkBody.find( ".node", 0) == -1)
		{
			loadFrom = ENdhsModule;
		}
	}
	else
	{
		loadFrom = ENdhsUnknown;
	}
	return loadFrom;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::updatePageStates(bool animate, bool menuItem, int stackLevel)
{
	m_terminalTime = m_pageStack[m_activePage]->getDefaultTerminalTime();
	m_terminalVelocityProfile = m_pageStack[m_activePage]->getDefaultTerminalVelocityProfile();

	// Assume nothing animates
	m_animatingStateChange = false;

#ifdef LC_USE_STYLUS
	// Update mouseover/touchdown statuses
	onMouseCancel(NULL, NULL, true);
#endif

	// Are we opening the root page?
	if (m_activePage > 0)
	{
		// Work out the multi-page visibility rules
		ENdhsParentVisibility parentVisibility = m_pageStack[m_activePage]->getParentVisibility();
		ENdhsPageState parentTargetState = ENdhsPageStateHide;

		switch (parentVisibility)
		{
			case ENdhsParentVisibilityHideAllParents:
			{
				m_action = EOpenMenuHideAllParents;
				parentTargetState = ENdhsPageStateHide;

				break;
			}

			case ENdhsParentVisibilityHideAllShowRoot:
			{
				m_action = EOpenMenuHideAllShowRoot;

				if (m_activePage == 1)
				{
					parentTargetState = ENdhsPageStateSelected;
				}
				else
				{
					parentTargetState = ENdhsPageStateHide;
				}

				break;
			}

			case ENdhsParentVisibilityShowDirectParent:
			{
				m_action = EOpenMenuShowDirectParent;
				parentTargetState = ENdhsPageStateSelected;

				break;
			}

			case ENdhsParentVisibilityHideDirectParent:
			default:
			{
				if (m_pageStack[m_activePage - 1]->getShowWhenInSub())
				{
					m_action = EOpenMenuShowDirectParent;
					parentTargetState = ENdhsPageStateSelected;
				}
				else
				{
					m_action = EOpenMenuHideDirectParent;
					parentTargetState = ENdhsPageStateHide;
				}

				break;
			}
		}

		// Change the state of the root page
		if (m_activePage > 1)
		{
			if (parentVisibility == ENdhsParentVisibilityHideAllParents)
			{
				if (m_pageStack[0]->getPageState() == ENdhsPageStateSelected)
				{
					m_animatingStateChange |= m_pageStack[0]->changeState(ENdhsPageStateHide, 0, NULL, animate);
				}
			}
			else if (parentVisibility == ENdhsParentVisibilityHideAllShowRoot)
			{
				if (m_pageStack[0]->getPageState() == ENdhsPageStateHide)
				{
					EAction backupAction = m_action;

					// Must silently put it in the show state first
					m_action = EHideToShow;
					m_pageStack[0]->changeState(ENdhsPageStateShow, 0, NULL, false);

					m_action = backupAction;
					m_animatingStateChange |= m_pageStack[0]->changeState(ENdhsPageStateSelected, 0, NULL, animate);
				}
			}
		}

		// Change the state of the pages between the parent and root page
		if (m_activePage > 2)
		{
			if (parentVisibility == ENdhsParentVisibilityHideAllParents
				|| parentVisibility == ENdhsParentVisibilityHideAllShowRoot)
			{
				for (int i = m_activePage - 2; i > 0; --i)
				{
					if (m_pageStack[i]->getPageState() == ENdhsPageStateSelected)
					{
						m_animatingStateChange |= m_pageStack[i]->changeState(ENdhsPageStateHide, 0, NULL, animate);
					}
				}
			}
		}

		// Change the state of the parent page
		if (menuItem)
		{
			// The page works out the active slot itself, so we can just pass in 0
			m_animatingStateChange |= m_pageStack[m_activePage - 1]->changeState(parentTargetState, 0, NULL, animate);
		}
		else
		{
			// The menu is being launched by furniture or the options menu
			// so we put the parent in a hide inactive state because a slot is not
			// being activated

			m_animatingStateChange |= m_pageStack[m_activePage - 1]->changeState(parentTargetState, -1, NULL, animate);
		}
	}
	else
	{
		// Must be the root page
		m_action = EOpenMenuHideDirectParent;
		setMatchedBackground(stackLevel);
	}

	if (!m_animatingStateChange)
		onTransitionComplete(false);
#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
	else
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_STARTED);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::loadTheme(const LcTmString& packagePath, bool animate)
{
	bool success = true;

	volatile int memError = 0;

	// Prepare cleanup frame
	LC_CLEANUP_PUSH_FRAME(memError);

	if (memError == 0)
	{

		// First, close all open pages
		m_activePage = -1;
		destroyUnwantedPages(true);

		LcCEnvironment::get()->compressHeap();

		// Now we can open the new theme
		if (openPackage(packagePath))
		{
			// Load the package file
			LcTaString err;
			// Set the directory slash separators to the non default if required.
			LcTaString localPath = packagePath + NDHS_PACKAGE_FILENAME;
			#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
				localPath.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
			#endif
			LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(localPath, err);
			if (root)
			{
				LcCXmlElem* settings = root->find(NDHS_TP_XML_SETTINGS);
				if (settings)
				{
#ifdef LC_USE_MOUSEOVER
					// Mouse over delay attribute
					LcTaString attrMouseOverDelay = settings->getAttr(NDHS_TP_XML_MOUSEOVER_DELAY);
					if(attrMouseOverDelay.isEmpty() == false)
					{
						m_mouseOverDelay = attrMouseOverDelay.toInt();
					}
#endif
					m_defaultEntryPointID = settings->getAttr(NDHS_TP_XML_DEFAULT_ENTRY_POINT_ID, m_defaultEntryPointID);

					//
					// Load the entry point map
					//
					LcCXmlElem* elemEntryPointMap = settings->find(NDHS_TP_XML_ENTRY_POINT_MAP);

					if (elemEntryPointMap)
					{
						LcCXmlElem* entryPoint = elemEntryPointMap->getFirstChild();

						for (; entryPoint; entryPoint = entryPoint->getNext())
						{
							LcTaString id = entryPoint->getAttr(NDHS_TP_XML_ENTRY_POINT_ID).toLower();
							LcTaString uri = entryPoint->getAttr(NDHS_TP_XML_NODE_URI).toLower();

							// check we got valid id and uri
							if (id.isEmpty() == false && uri.isEmpty() == false)
							{
								m_entryPointStack->pushMap(id, uri);
							}
						}
					}
				}

				//
				// Load the palettes.
				//
				loadPalette(root.ptr());

				if (m_defaultEntryPointID.length() > 0)
				{
					// We will be opening this entry point on next go...
					success = true;
				}

				if (success)
				{
					// Store the last known good theme name.
					m_tokenStack->getIniTokens()->setValue(NDHS_TP_XML_THEME_CURRENT, packagePath);
					m_lastGoodThemeName = packagePath;
				}
			}
			else
			{
				NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleGeneral, err);
			}

			// Cleanup the package if failed
			if (!success)
			{
				m_tokenStack->popTokens();
				m_manifestStack->popManifest();
			}
			else
			{
				m_tokenStack->saveIniTokens(m_dataPath, m_iniTokenFile);
			}
		}
	}
	else
	{
		IFXP_Display_Error_Note((IFX_WCHAR*)L"Theme could not be changed");

		// Schedule a repaint to clean up the error note
		getSpace()->revalidate();

		success = false;
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memError);

	return success;
}

// Get Manifest from the package path (if available)
NdhsCManifest* NdhsCPageManager::getPackageManifest(const LcTmString& packagePath)
{
	LcTaString newPackageName = packagePath.tail(packagePath.findLastChar(NDHS_FOLDER_SEPARATOR_CHAR) + 1);

	TmMPackageManifestCache::iterator iter=m_packageManifestMap.find(newPackageName);
	if(iter==m_packageManifestMap.end())
	{
		LcTaOwner<NdhsCManifest> man = NdhsCManifest::create(packagePath + NDHS_MANIFEST_FILENAME, newPackageName);
		if (!man->loadManifest())
		{
			man.destroy();
		}
		m_packageManifestMap.add_element(newPackageName, man);
		iter=m_packageManifestMap.find(newPackageName);
	}
	return iter->second;
}

/*-------------------------------------------------------------------------*//**
 * read file manifest information if the path is an absolute path but is part of package
 */
LcTaString NdhsCPageManager::getFileData(LcTmString& absolutePath,LcTaArray<NdhsCManifest::CManifestFile*> *fileData)
{
	LcTaString outFile="";
	NdhsCManifest * manifest=NULL;
	LcTaString pathForManifest ="";

	/* If it's a valid package file absolute path coming from inside the engine then
	   the Package name will be after the C:\Packages\ , where C can be any drive letter. */
	int packageNameLoc = (1 + IFX_PLAT_DRIVE_SEP_POS + 1 + NDHS_PALETTE_DIR_LENGTH + 1);

	/* if its an absolute path, and has enough length to keep the package file path */
	if ((absolutePath[IFX_PLAT_DRIVE_SEP_POS] == IFX_PLAT_DRIVE_SEP_CHAR)
			&& (absolutePath.length() > packageNameLoc))
	{
		/* find string "Packages" in path after the drive letter and separator */
		int index = absolutePath.find(NDHS_PACKAGE_DIR,(IFX_PLAT_DRIVE_SEP_POS + 1));

		/* check if the index of "Packages" is correct according to the packages absolute path,
		   After C:\, Where C can be any drive letter */
		if (index == (IFX_PLAT_DRIVE_SEP_POS + 2))
		{
			/* find the location from where the relative path starts and get the path of
			   package and relative path of file */
			int packagePathLen = absolutePath.find(NDHS_DIR_SEP,packageNameLoc)+1;
			pathForManifest=absolutePath.tail(packagePathLen);
			LcTaString packagePath=absolutePath.subString(0,packagePathLen);

			/* get package manifest */
			manifest=getPackageManifest(packagePath);
		}

		/* If package manifest is found then find the prepared path in package manifest */
		if ((manifest != NULL) && manifest->fileExists(pathForManifest, outFile, true, fileData))
		{
			LcTaString ext = outFile.getWord(-1, '.');
			outFile = absolutePath.subString(0, absolutePath.length() - ext.length()) + ext;
		}
	}

	return outFile;
}

#ifdef IFX_USE_PLUGIN_ELEMENTS
/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::releasePluginElement(NdhsCPlugin::NdhsCPluginHElement* pluginElement)
{
	if (pluginElement == NULL)
		return;

	TmMPluginElementCache::iterator it = m_pluginElements.begin();
	for (; it != m_pluginElements.end(); it++)
	{
		CPluginElementItem* ptItem = it->second;

		if (ptItem->pluginElement.ptr() == pluginElement)
		{
			ptItem->pluginElement->release();
			if (ptItem->pluginElement->getRefCount() <= 0)
			{
				m_pluginElements.erase(it);
			}

			return;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager::CPluginElementItem> NdhsCPageManager::CPluginElementItem::create()
{
	LcTaOwner<CPluginElementItem> ref;
	ref.set(new CPluginElementItem());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPlugin::NdhsCPluginHElement* NdhsCPageManager::getPluginElement(NdhsCPlugin::NdhsCPluginMenu *pluginMenu,
																								int itemIndex,
																								LcTaString linkName,
																								IFX_ELEMENT_PROPERTY *elementProperty)
{
	NdhsCPlugin::NdhsCPluginHElement* pluginElement = NULL;

	// Lets see if its been cached
	TmMPluginElementCache::iterator it;
	LcTaString key = linkName.toLower();

	// We only cache 'graphic' mode plugins - pure event handlers (where elementProperty
	// will be NULL) are never cached
	if (elementProperty)
	{
		it = m_pluginElements.find(key);
	}
	else
	{
		// Generate unique key for this event handler
		m_eventHandlerElemCount++;
		key.fromInt(m_eventHandlerElemCount);
		key = "eventhandler_" + key;
		it = m_pluginElements.end();
	}

	if (it != m_pluginElements.end())
	{
		// Its cached, so increment the ref count and return it
		CPluginElementItem* ptItem = it->second;
		pluginElement = ptItem->pluginElement.ptr();
		pluginElement->acquire();
	}
	else
	{
		// Not cached, so create a new plugin element instance
		LcTaOwner<CPluginElementItem> ptItem = CPluginElementItem::create();
		LcTaOwner<NdhsCPlugin::NdhsCPluginHElement> newPluginElement;

		LcTmString creationLink;
		creationLink = linkName.subString(0, linkName.find(":"));

		// Now acquire the correct integrated module handle for the link prefix
		NdhsCPlugin* pPlugin = getCon()->getTRLinkTypePlugin(creationLink);

		// Create the embedded element instance
		if(pPlugin != NULL)
		{
			newPluginElement = pPlugin->createElement(pluginMenu,
															this,
															itemIndex,
															linkName,
															elementProperty);
		}

		ptItem->pluginElement = newPluginElement;
		if (ptItem->pluginElement)
		{
			CPluginElementItem* ptItemPtr = ptItem.ptr();
			ptItem->pluginElement->acquire();
			m_pluginElements.add_element(key, ptItem);
			pluginElement = ptItemPtr->pluginElement.ptr();
		}
		else
		{
			// Failed to create the plugin element instance
			ptItem->pluginElement.destroy();
			ptItem.destroy();
		}
	}

	if (elementProperty && pluginElement)
	{
		IFX_ELEMENT_PROPERTY *cachedProperties = pluginElement->getCachedProperties();
		*elementProperty = *cachedProperties;
	}

	return pluginElement;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::openPackage(const LcTmString& packagePath)
{
	LcTaString newPackageName = packagePath.tail(packagePath.findLastChar(NDHS_FOLDER_SEPARATOR_CHAR) + 1);

	// Add to the token replacer stack
	m_tokenStack->pushTokens(packagePath);

	// Add to the manifest stack
	m_manifestStack->pushManifest(packagePath + NDHS_MANIFEST_FILENAME, newPackageName);

	int stackLevel = m_manifestStack->getStackHeight() - 1;

#ifndef NDHS_TOOLKIT
	NdhsCProjectValidator pv;
	int error = pv.loadProject(m_manifestStack->getManifest(stackLevel));
	LcTaString errMessage;

	// Check that theme version is OK
	if (error == NdhsCProjectValidator::EErrorVersion)
	{
		errMessage = "Theme requires software update";

		// Trigger the apps update menu item
		if (!m_con->launchEventLink("productUpdate", stackLevel - 1))
		{
			// If they have not updated the theme show message.
			m_con->displayHaltError(errMessage, false);
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleGeneral, errMessage);
		}

		m_tokenStack->popTokens();
		m_manifestStack->popManifest();

		return false;
	}
#endif

#ifdef IFX_VALIDATE_PROJECTS
	// Now check file signatures (except in debug builds)
	if ((error != NdhsCProjectValidator::ESuccess) || !pv.validateProject())
	{
		m_tokenStack->popTokens();
		m_manifestStack->popManifest();

		errMessage = "Package files corrupted";

		m_con->displayHaltError(errMessage, false);
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleGeneral, errMessage);

		return false;
	}
#endif

	return true;
}

/*-------------------------------------------------------------------------*//**
	This will load the palette
*/
void NdhsCPageManager::loadPalette(LcCXmlElem* root)
{
	if(root)
	{
		LcCXmlElem* elemPalettesList = root->find(NDHS_TP_XML_PALETTES);
		if (elemPalettesList)
		{
			LcCXmlElem* paletteItem = elemPalettesList->getFirstChild();

			for (; paletteItem; paletteItem = paletteItem->getNext())
			{
				LcTaString tempPalPath = paletteItem->getAttr(NDHS_TP_XML_NAME);

				// Check we have got a valid name.
				if (tempPalPath.isEmpty() == false)
				{
					openPalette(NDHS_PALETTE_DIR NDHS_DIR_SEP + tempPalPath);
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
	This will open the palette
*/
bool NdhsCPageManager::openPalette(const LcTmString& palettePath)
{
	bool manifestLoaded = false;
	int stackLevel = m_manifestStack->getStackHeight() - 1;
	NdhsCManifest* topManifestStack = m_manifestStack->getManifest(stackLevel);
	LcTaString lowerCasePalettePath = palettePath.toLower();

	if (topManifestStack == NULL)
		return false;

	// If the palette manifest is not already cached then load it.
	TmMPaletteCache::iterator it = m_paletteMap.find(lowerCasePalettePath);
	if (it != m_paletteMap.end())
	{
		// Its cached, so increment the ref count.
		CPaletteItem* ptItem = it->second;
		ptItem->refCount++;

		// Link it to the package that loaded it.
		topManifestStack->addLinkedPalette(ptItem->paletteManifest.ptr());

		return true;
	}
	else
	{
		// Not cached, so create a new palette and manifest.
		LcTaOwner<CPaletteItem> ptItem = CPaletteItem::create();

		// Work out the package path
		LcTaString localPalettePath = getFullPalettePath(palettePath);
		if (!localPalettePath.isEmpty())
		{
			LcTaString newPaletteName = palettePath.tail(palettePath.findLastChar(NDHS_DIR_SEP_CHAR) + 1);
			ptItem->paletteManifest = NdhsCManifest::create(localPalettePath + NDHS_MANIFEST_FILENAME, newPaletteName);
			if (ptItem->paletteManifest)
			{
				if (ptItem->paletteManifest->loadManifest())
				{
					ptItem->refCount = 1;

					// Link it to the package that loaded it.
					topManifestStack->addLinkedPalette(ptItem->paletteManifest.ptr());

					m_paletteMap.add_element(lowerCasePalettePath, ptItem);
					manifestLoaded = true;
				}
			}
			else
			{
				// Failed to create the menu
				ptItem->paletteManifest.destroy();
				ptItem.destroy();
			}
		}
	}

	if (manifestLoaded)
	{
		int stackLevel = m_manifestStack->getStackHeight() - 1;

#ifndef NDHS_TOOLKIT
		NdhsCProjectValidator pv;
		int error = pv.loadProject(m_manifestStack->getManifest(stackLevel));
		LcTaString errMessage;

		// Check that theme version is OK
		if (error == NdhsCProjectValidator::EErrorVersion)
		{
			errMessage = "Theme requires software update";

			// Trigger the apps update menu item
			if (!m_con->launchEventLink("productUpdate", stackLevel - 1))
			{
				// If they have not updated the theme show message.
				m_con->displayHaltError(errMessage, false);
			}

			m_tokenStack->popTokens();
			m_manifestStack->popManifest();

			return false;
		}
#endif

#ifdef IFX_VALIDATE_PROJECTS
		// Now check file signatures (except in debug builds)
		if ((error != NdhsCProjectValidator::ESuccess) || !pv.validateProject())
		{
			m_tokenStack->popTokens();
			m_manifestStack->popManifest();

			errMessage = "Package files corrupted";

			m_con->displayHaltError(errMessage, false);

			return false;
		}
#endif
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
	This will close the palette, whilst reference counting.
*/
bool NdhsCPageManager::closePalette(NdhsCManifest* paletteManifest)
{
	// Find the palette manifest.
	TmMPaletteCache::iterator palIter = m_paletteMap.begin();
	for (; palIter != m_paletteMap.end(); ++palIter)
	{
		CPaletteItem* ptItem = palIter->second;
		if (ptItem->paletteManifest.ptr() == paletteManifest)
		{
			// Decrement the ref count.
			ptItem->refCount--;

			// If this is the last one then delete the manifest.
			if (ptItem->refCount == 0)
			{
				// Failed to create the menu
				ptItem->paletteManifest.destroy();
				m_paletteMap.erase(palIter);
			}
			return true;
		}
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCPageManager::getPageStackLevel(NdhsCPageModel* page)
{
	int stackLevel = -1;

	for (int i = (int)m_pageStack.size() - 1; i >=0 && stackLevel == -1; --i)
	{
		if (m_pageStack[i] == page)
		{
			stackLevel = i;
		}
	}

	return stackLevel;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::processAttempt(NdhsCTemplate::CAction::CAttempt* attempt,
										int stackLevel,
										const LcTmString& additionalInfo)
{
	bool retValue = false;

	// Assume no animation
	m_animatingStateChange = false;

	if (attempt)
	{
		switch (attempt->attemptType)
		{

			case NdhsCMenuComponentTemplate::CAction::CAttempt::EBack:
			{
				// Fail if already at root
				if (m_activePage == 0)
				{
					retValue = false;
				}
				else
				{
					// Stop any inter-page transitions
					jumpTransitionToEnd();

					// Have to cancel previosly scheduled destroy events
					// Otherwise the current page will be destroyed early
					if	(m_destroyUnwantedPagesMessage.isScheduled())
					{
						m_destroyUnwantedPagesMessage.cancel();
					}

					retValue = true;

					// Dirty the field cache
					setFieldCacheDirty();

					// Set the new active page first to make sure that any
					// new triggers are sent to the new active page
					bool activePageSet = false;
					int prevActivePage = m_activePage;

					// Figure out which page is becoming active
					if (((NdhsCTemplate::CAction::CBack*)attempt)->backToLevel > -1)
					{
						// The page has done a "back to me"
						m_activePage = ((NdhsCTemplate::CAction::CBack*)attempt)->backToLevel;
						activePageSet = true;
					}
					else if (!((NdhsCTemplate::CAction::CBack*)attempt)->page.isEmpty())
					{
						for (int i = m_activePage - 1; activePageSet == false && i >= 0; --i)
						{
							if (m_pageStack[i]->getPage()->getPageName().compareNoCase(((NdhsCTemplate::CAction::CBack*)attempt)->page) == 0)
							{
								m_activePage = i;
								activePageSet = true;
							}
						}
					}

					if (!activePageSet)
					{
						// Must be going back one level
						m_activePage--;
					}

					setupPrimaryLightTransitionOverrideFlag(m_activePage, prevActivePage);

					// Let the con update the softkey menu
					m_con->updateScreenFurniture();

					// Any page that needs to go to the show state must
					// be changed before beginning any other state changes
					// to ensure they get their transition complete message

					// If the page was in a hide state, we must jump it
					// to the show state before we bring it back
					if (m_pageStack[m_activePage]->getPageState() == ENdhsPageStateHide)
					{
						m_action = EHideToShow;
						(void)m_pageStack[m_activePage]->changeState(ENdhsPageStateShow, 0, NULL, false, false);
					}

					ENdhsParentVisibility visibility = m_pageStack[m_activePage]->getParentVisibility();

					bool bDone = false;
					int i;	// This causes compiler errors in ADS if declared in the loop.

					for (i = m_activePage; i > 0 && !bDone; --i)
					{
						if (visibility != ENdhsParentVisibilityHideAllParents)
						{
							// May need to change some pages to the show state
							if (visibility == ENdhsParentVisibilityHideAllShowRoot)
							{
								// This covers all remaining pages, so were done
								bDone = true;

								// If the root page is hidden, send it to the show state
								if (m_pageStack[0]->getPageState() == ENdhsPageStateHide)
								{
									m_action = EHideToShow;
									(void)m_pageStack[0]->changeState(ENdhsPageStateShow, 0, NULL, false);
								}
							}
							else
							{
								// Check the visibility or the parent page
								if (visibility == ENdhsParentVisibilityShowDirectParent
									|| m_pageStack[i - 1]->getShowWhenInSub())
								{
									if (m_pageStack[i - 1]->getPageState() == ENdhsPageStateHide)
									{
										m_action = EHideToShow;
										(void)m_pageStack[i - 1]->changeState(ENdhsPageStateShow, 0, NULL, false);
									}
								}

								visibility = m_pageStack[i - 1]->getParentVisibility();
							}
						}
						else
						{
							// If its hide all parents, were done
							bDone = true;
						}
					}

					// The pages between the previous active and new active
					// will immediately skip to the close state
					for (int j = prevActivePage - 1; j > m_activePage; --j)
					{
						m_action = ESkipToClose;
						m_pageStack[j]->unsetFocus();
						(void)m_pageStack[j]->changeState(ENdhsPageStateClose, 0, NULL, false);
					}

					m_action = EBackDestroyChildren;

					// Use the terminal time of the previous active page
					m_terminalTime = m_pageStack[prevActivePage]->getDefaultTerminalTime();
					m_terminalVelocityProfile = m_pageStack[prevActivePage]->getDefaultTerminalVelocityProfile();

					// Now change the state of all the pages that are tweening

					// The previously active page tweens to the close state
					m_pageStack[prevActivePage]->unsetFocus();
					m_animatingStateChange |= m_pageStack[prevActivePage]->changeState(ENdhsPageStateClose, 0, NULL, true);

					visibility = m_pageStack[m_activePage]->getParentVisibility();

					bDone = false;

					for (i = m_activePage; i > 0 && !bDone; --i)
					{
						// May need to change some pages to the show state
						if (visibility == ENdhsParentVisibilityHideAllParents
							|| visibility == ENdhsParentVisibilityHideAllShowRoot)
						{
							// This covers all remaining pages, so were done
							bDone = true;

							for (int k = i - 1; k > 0; --k)
							{
								if (m_pageStack[k]->getPageState() == ENdhsPageStateSelected)
								{
									m_animatingStateChange |= m_pageStack[k]->changeState(ENdhsPageStateHide, 0, NULL, true);
								}
							}

							// Now handle the root
							if (visibility == ENdhsParentVisibilityHideAllShowRoot)
							{
								// Showing the root
								if (m_pageStack[0]->getPageState() == ENdhsPageStateShow)
								{
									m_animatingStateChange |= m_pageStack[0]->changeState(ENdhsPageStateSelected, 0, NULL, true);
								}
							}
							else
							{
								// Hiding the root
								// Showing the root
								if (m_pageStack[0]->getPageState() == ENdhsPageStateSelected)
								{
									m_animatingStateChange |= m_pageStack[0]->changeState(ENdhsPageStateHide, 0, NULL, true);
								}
							}
						}
						else
						{
							// Check the visibility or the parent page
							if (visibility == ENdhsParentVisibilityShowDirectParent
								|| m_pageStack[i - 1]->getShowWhenInSub())
							{
								// Showing the parent
								if (m_pageStack[i - 1]->getPageState() == ENdhsPageStateShow)
								{
									m_animatingStateChange |= m_pageStack[i - 1]->changeState(ENdhsPageStateSelected, 0, NULL, true);
								}
							}
							else
							{
								// Hiding the parent
								if (m_pageStack[i - 1]->getPageState() == ENdhsPageStateSelected)
								{
									m_animatingStateChange |= m_pageStack[i - 1]->changeState(ENdhsPageStateHide, 0, NULL, true);
								}
							}

							visibility = m_pageStack[i - 1]->getParentVisibility();
						}
					}

					// Change the state of the new active page
					//
					// we have already incremented the animator count
					// The page works out the active slot itself, so we can just pass in 0
					m_animatingStateChange |= m_pageStack[m_activePage]->changeState(ENdhsPageStateInteractive, 0, NULL, true);

					setupPrimaryLightTransitionOverrideTiming(m_activePage, prevActivePage);

					if (!m_animatingStateChange)
						onTransitionComplete(false);
#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
					else
						getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_STARTED);
#endif

					// Set the change-background message up
					setupChangeBackgroundMessage(prevActivePage);
				}
				break;
			}

			case NdhsCMenuComponentTemplate::CAction::CAttempt::ELink:
			{
				// Stop any inter-page transitions
				jumpTransitionToEnd();

				// Now just launch the link - note that the page model may have pre-token replaced
				// the uri (using the relevant contexts), in which case use that uri rather
				// than the one in the attempt structure
				if (additionalInfo.isEmpty())
				{
					LcTaString unexpandedLink = ((NdhsCTemplate::CAction::CLink*)attempt)->uriString;

					NdhsCExpression::CExprSkeleton* uriCFExpr = (((NdhsCTemplate::CAction::CLink*)attempt)->uri).ptr();
					if (unexpandedLink.isEmpty() && uriCFExpr)
					{
						NdhsCExpression* uriExpr = uriCFExpr->getContextFreeExpression();
						if (uriExpr)
						{
							uriExpr->evaluate(NULL, -1, NULL);

							if (uriExpr->isError())
							{
								uriExpr->errorDiagnostics("Link action URI", true);
							}
							else
							{
								unexpandedLink = uriExpr->getValueString();
							}
						}
					}

					m_launchItem->setLinkAttr(unexpandedLink);
				}
				else
					m_launchItem->setLinkAttr(additionalInfo);

				m_isLaunchingMenuItem = false;

				retValue = launchItem(m_launchItem.ptr(), false, stackLevel);
				break;
			}

			case NdhsCMenuComponentTemplate::CAction::CAttempt::ESuspend:
			{
				// Stop any inter-page transitions
				jumpTransitionToEnd();

				// Must tell NdhsCAppUI to suspend the app in
				// a platform specific way
				m_con->suspendApp();
				retValue = true;
				break;
			}

			case NdhsCMenuComponentTemplate::CAction::CAttempt::EExit:
			{
				// Stop any inter-page transitions
				jumpTransitionToEnd();

				// Must tell NdhsCAppUI to exit the app in
				// a platform specific way
				m_con->exitApp();
				retValue = true;
				break;
			}

			default:
			{
				break;
			}
		}
	}

#ifdef NDHS_PREVIEWER
	// We update the log window in the previewer if something new
	// has been logged
	if (NdhsCLog::Instance()->hasChanged())
	{
		// Get pointer to log pop-up
		CNDHSThemeLogHtmlDlg* ld = (CNDHSThemeLogHtmlDlg*)
			theApp.GetPreviewerDlg()->getPopup(PREVIEWER_POPUP_LOG);
		// Force a log window update
		ld->refreshLog();
	}
#endif

	return retValue;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::onMessage(int iID, int iParam)
{
	switch (iID)
	{
		case EDestroyUnwantedPagesMsg:
		{
			destroyUnwantedPages(false);
			LcCEnvironment::get()->compressHeap();

			break;
		}

		case EOpenThemeMsg:
		{
			// Load the theme
			if (loadTheme(((NdhsCMenuItem*)iParam)->getPackagePath(), true) == false)
			{
				// Failed to load theme, so re-apply the old one
				LcTaString package;
				if (m_tokenStack->getIniTokens()->getValue(NDHS_TP_XML_THEME_CURRENT, package)
					&& package.length() > 0)
				{
					destroyUnwantedPages(true);
					m_con->displayWarning("Unable to load theme!");

					loadTheme(package, true);
				}
			}

			openEntryPoint(true, m_defaultEntryPointID);

			break;
		}

		case EConfigureExternalMsg:
		{
			LcTaString entryPoint = "";
			bool requireEntryPoint = false;

			if ((iParam & NDHS_CONFIG_DISPLAY_MODE) && (iParam & NDHS_CONFIG_LANGUAGE))
			{
				// Set the language and display mode.
				setLanguageAndScreenSize(m_reqLanguage, m_reqDisplayMode);

				// Clear the display mode string.
				m_reqDisplayMode = "";

				// Clear the language string.
				m_reqLanguage = "";

				entryPoint = m_currentEntryPointId;
				requireEntryPoint = true;
			}
			else if (iParam & NDHS_CONFIG_DISPLAY_MODE)
			{
				// Set the display mode.
				setScreenSize(m_reqDisplayMode);

				// Clear the display mode string.
				m_reqDisplayMode = "";

				entryPoint = m_currentEntryPointId;
				requireEntryPoint = true;
			}
			else if (iParam & NDHS_CONFIG_LANGUAGE)
			{
				// Set the language.
				setLanguage(m_reqLanguage);

				// Clear the language string.
				m_reqLanguage = "";

				entryPoint = m_currentEntryPointId;
				requireEntryPoint = true;
			}
			// Only apply the theme here if there were no language / display mode changes
			// as these will update the theme for us if m_reqThemeName is non-zero length.
			else if (iParam & NDHS_CONFIG_THEME_PACKAGE)
			{
				LcTaString fullPackagePath;

				// Get the full package Path
				fullPackagePath = this->getFullPackagePath(m_con->getThemePath() + m_reqThemeName);

				if (!fullPackagePath.isEmpty())
				{
					// Load the theme
					if (loadTheme(fullPackagePath, true) == false)
					{
						// Failed to load theme, so re-apply the old one
						LcTaString package;
						if (m_tokenStack->getIniTokens()->getValue(NDHS_TP_XML_THEME_CURRENT, package)
							&& package.length() > 0)
						{
							destroyUnwantedPages(true);
							m_con->displayWarning("Unable to load theme!");

							loadTheme(package, true);
						}
					}
					entryPoint = m_currentEntryPointId = m_defaultEntryPointID;
					requireEntryPoint = true;
				}
				else
				{
					m_con->displayWarning("Could not find theme!");
				}

				// Clear the package name string.
				m_reqThemeName = "";
			}

			bool defaultEntryPoint = (m_activePage==-1);

#ifndef NDHS_JNI_INTERFACE
			if(iParam & NDHS_CONFIG_ENTRYPOINT_ID)
			{
				entryPoint = m_reqEntryPointId;
				requireEntryPoint = true;
			}

			if(requireEntryPoint
					&& (openEntryPoint(defaultEntryPoint, entryPoint) == IFX_SUCCESS)
					&& !defaultEntryPoint)
			{
				m_currentEntryPointId = entryPoint;
			}
			break;
#endif
		}

		case ELaunchLinkMsg:
		{
			if (m_activePage < 0)
				return;

			// Launch the link
			volatile int memError = 0;

			// Prepare cleanup frame
			LC_CLEANUP_PUSH_FRAME(memError);

			if (memError == 0)
			{
				NdhsCMenuItem* item;

				if (m_isLaunchingMenuItem == true)
				{
					item = m_itemToLaunch;
					m_itemToLaunch = NULL;
				}
				else
				{
					item = m_launchItem.ptr();
				}

				int stackLevel = m_pageStack[m_activePage]->getStackLevel();

				if (item->isMenuAddition())
				{
					// Set the stack level to be the top of the stack if its a menu addition
					stackLevel = m_manifestStack->getStackHeight() - 1;
				}

				volatile int launchError;
				LC_CLEANUP_TRAP(launchError, getCon()->launchLink(item, stackLevel));

				// Absorb the error, unless it is fatal
				if (IFX_ERROR_RESTART == launchError)
				{
					IFXP_Display_Error_Note((IFX_WCHAR*)L"Action could not be completed");
					LC_CLEANUP_THROW(IFX_ERROR_RESTART);
				}

				if (item->isMenuAddition())
				{
					// Now we can get rid of the package if it
					// was a menu addition
					destroyUnwantedPages(true);
				}

				// For successful asynchronous links we block the end transition until we return.
				if (!getCon()->isAsyncLinkBlocking())
					postLaunchItemAction();
			}
			else
			{
				IFXP_Display_Error_Note((IFX_WCHAR*)L"Action could not be completed");
				LC_CLEANUP_THROW(IFX_ERROR_RESTART);
			}

			// Finished with cleanup stack
			LC_CLEANUP_POP_FRAME(memError);

			break;
		}

		case EEventMsg:
		{
			volatile int memError = 0;

			// Prepare cleanup frame
			LC_CLEANUP_PUSH_FRAME(memError);

			if (memError == 0)
			{
				// See if we can perform multiple refreshes at once - reschedule the
				// event timer if there are still events to process.
				if(processRefreshEvents(true))
					m_eventMessage.schedule(getSpace()->getTimer(), 0, 0);
			}
			else
			{
				// If event queue has been corrupted, this is serious enough to restart
				LC_CLEANUP_THROW(IFX_ERROR_RESTART);
			}

			// Finished with cleanup stack
			LC_CLEANUP_POP_FRAME(memError);

			break;
		}

		case ESimulateKeyMsg:
		{
			onKeyDown(iParam);

			break;
		}

		case EPostTransitionCompleteMsg:
		{
			for (int i = (int)m_pageStack.size() - 1; i >= 0; i--)
			{
				m_pageStack[i]->postTransitionComplete();
			}

			break;
		}

		case EChangeBackgroundMsg:
		{
			setMatchedBackground(m_pageStack[m_activePage]->getStackLevel());

			break;
		}

		case EAsynchronousLaunchCompleteMsg:
		{
			doAsyncLaunchComplete();

			break;
		}
#ifdef LC_USE_MOUSEOVER
		case EDoMouseOverHitTest:
		{
			m_doMouseOverHitTesting = true;
			onMouseMove(m_mouseOverCurrentPt);
			break;
		}
#endif

		default:
			break;
	}
}

/*-------------------------------------------------------------------------*//**
	This will complete the launch item transition.
*/
void NdhsCPageManager::postLaunchItemAction()
{
	// Flush out the temp menu item
	if (m_isLaunchingMenuItem == false)
	{
		LcTaString emptyString = "";

		m_launchItem->setLinkAttr(emptyString);
	}

	// Now get us back to the interactive state
	m_action = EBackFromLink;

	// Let it use its own transition time
	m_terminalTime = m_pageStack[m_activePage]->getDefaultTerminalTime();
	m_terminalVelocityProfile = m_pageStack[m_activePage]->getDefaultTerminalVelocityProfile();


	// The page works out the active slot itself, so we can just pass in 0
	m_animatingStateChange |= m_pageStack[m_activePage]->changeState(ENdhsPageStateInteractive, 0, NULL, true);

	if (!m_animatingStateChange)
		onTransitionComplete(false);
#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
	else
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_STARTED);
#endif
}

/*-------------------------------------------------------------------------*//**
	We need to find the best page template file using the dot notation
	to find the best match.  We must check the search paths for each file
*/
NdhsCPageTemplate* NdhsCPageManager::getPageTemplate(NdhsCPage* page, int stackLevel, LcTmString& templatePath)
{
	LcTaString pageName = page->getPageName();
	LcTaString returnedVal;
	LcTaString ext;
	LcTaString templateFile = "template." + pageName;

	int nestedComponentLevel = getNestedComponentLevel();
	m_isCyclic = false;

	if (m_tokenStack->getBestMatchValue(templateFile, templateFile, stackLevel))
	{
		LcTaString outTemplateFile;

		NDHS_TRACE(ENdhsTraceLevelInfo, ENdhsTraceModuleGeneral, "Template for this node is " + templateFile);

		m_tokenStack->replaceTokens(templateFile, outTemplateFile, NULL, NULL, NULL, NULL, stackLevel);

		templateFile = outTemplateFile;

#ifdef NDHS_RESOURCE_FILE_EXTENSION_REPLACEMENT
		// Get file extension from file
		ext = templateFile.getWord(-1, '.');

		// Before loading replace extension for our file type
		templateFile = templateFile.subString(0, templateFile.length() - ext.length()) + "template";
#endif

		// Get palette name.
		NdhsCManifest* paletteMan = getPaletteManifest(templateFile);

		if (m_manifestStack->findFile(templateFile, outTemplateFile, paletteMan, stackLevel))
		{
			templateFile = outTemplateFile;
			templatePath = outTemplateFile;

			// Lets see if its been cached
			TmMPageTemplateCache::iterator it = m_templates.find(templateFile);

			if (it != m_templates.end())
			{
				// Its cached, so increment the ref count and return it
				CTemplateItem* ptItem = it->second;
				ptItem->refCount++;
				return (NdhsCPageTemplate*)ptItem->templateFile.ptr();
			}
			else
			{
				// Not cached, so create a new template
				LcTaOwner<CTemplateItem> ptItem = CTemplateItem::create();
				LcTaOwner<NdhsCPageTemplate> newPageTemplate = NdhsCPageTemplate::create(this, templateFile);
				newPageTemplate->setPaletteManifest(paletteMan);
				ptItem->templateFile = newPageTemplate;
				if (ptItem->templateFile && ptItem->templateFile->configureFromXml(m_designSize, stackLevel, nestedComponentLevel))
				{
					// We can associate page file only with page templates
					if (ptItem->templateFile->getTemplateType() != ETemplateTypePage)
					{
						ptItem->templateFile.destroy();
						ptItem.destroy();
						return NULL;
					}

					CTemplateItem* ptItemPtr = ptItem.ptr();
					ptItem->refCount = 1;
					ptItem->stackLevel=stackLevel;
					ptItem->key=templateFile;
					m_templates.add_element(templateFile, ptItem);
					return (NdhsCPageTemplate*)ptItemPtr->templateFile.ptr();
				}
				else
				{
					// Failed to create the template
					ptItem->templateFile.destroy();
					ptItem.destroy();
					return NULL;
				}
			}
		}
		else
		{
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleGeneral, "Could not load template file " + templateFile);
		}
	}

	// The template does not exist
	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCTemplate* NdhsCPageManager::getComponentTemplate(	LcTmString&		componentFile,
														int				stackLevel,
														NdhsCManifest*	paletteManifest,
														int&			nestedComponentLevel)
{
	LcTaString returnedVal;
	LcTaString ext;

	LcTaString outComponentFile;

	if (nestedComponentLevel <= 0 || m_isCyclic)
	{
		// Only generate error message for once
		if (m_isCyclic == false)
		{
			NDHS_TRACE(ENdhsTraceLevelError, NULL, "Component hierarchy is cyclic, so loading only limited hierarchy, Please remove cycle from component hierarchy");
		}
		m_isCyclic = true;
		return NULL;
	}

#ifdef NDHS_RESOURCE_FILE_EXTENSION_REPLACEMENT
	// Get file extension from file
	ext = componentFile.getWord(-1, '.');

	// Before loading replace extension for our file type
	componentFile = componentFile.subString(0, componentFile.length() - ext.length()) + "component";
#endif

	// Get palette manifest for this palette.
	// Get it from the parent if it does not exist.
	NdhsCManifest* paletteMan = getPaletteManifest(componentFile);
	if (NULL == paletteMan)
		paletteMan = paletteManifest;

	if (m_manifestStack->findFile(componentFile, outComponentFile, paletteMan, stackLevel))
	{
		componentFile = outComponentFile;

		// Lets see if its been cached
		TmMPageTemplateCache::iterator it = m_templates.find(outComponentFile);

		if (it != m_templates.end())
		{
			// Its cached, note reference count will be incremented as component
			// created, becasue template caching is difference from template
			// caching, placing ref count logic here may fail in some scenarios
			CTemplateItem* ptItem = it->second;
			ptItem->refCount++;
			return (NdhsCTemplate*)ptItem->templateFile.ptr();
		}
		else
		{
			LcTaString err;

			// Set the directory slash separators to the non default if required.
			#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
				componentFile.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
			#endif
			LcTaOwner<LcCXmlElem> root = LcCXmlElem::load(componentFile, err);
			LcTaOwner<NdhsCTemplate> newComponentTemplate;

			#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
				componentFile.replace(NDHS_PLAT_DIR_SEP_CHAR, NDHS_DIR_SEP_CHAR);
			#endif

			if (root)
			{
				switch((getTemplateType(root->find(NDHS_TP_XML_SETTINGS))))
				{
					case ETemplateTypeMenuComponent:
					{
						// Menu Component
						newComponentTemplate = NdhsCMenuComponentTemplate::create(this, root);
						newComponentTemplate->setPaletteManifest(paletteMan);
						break;
					}
					case ETemplateTypeComponent:
					{
						// Non menu component
						newComponentTemplate = NdhsCTemplate::create(this, root);
						newComponentTemplate->setPaletteManifest(paletteMan);
						break;
					}
					case ETemplateTypePage:
					case ETemplateTypeInvalid:
					{
						// should not come here
						break;
					}
				}
			}
			else
			{
				return NULL;
			}

			// Not cached, so create a new template
			LcTaOwner<CTemplateItem> ptItem = CTemplateItem::create();
			ptItem->templateFile = newComponentTemplate;
			if (ptItem->templateFile && ptItem->templateFile->configureFromXml(m_designSize, stackLevel, nestedComponentLevel))
			{
				// In case of nested cyclic components, we need to use cache, also
				TmMPageTemplateCache::iterator it = m_templates.find(outComponentFile);

				if (it != m_templates.end())
				{
					// Its cached, note reference count will be incremented as component
					// created, becasue template caching is difference from template
					// caching, placing ref count logic here may fail in some scenarios
					CTemplateItem* ptItem = it->second;
					ptItem->refCount++;
					return (NdhsCTemplate*)ptItem->templateFile.ptr();
				}
				else
				{
					CTemplateItem* ptItemPtr = ptItem.ptr();
					ptItem->refCount = 1;
					ptItem->stackLevel=stackLevel;
					ptItem->key=outComponentFile;
					m_templates.add_element(outComponentFile, ptItem);
					return (NdhsCTemplate*)ptItemPtr->templateFile.ptr();
				}
			}
			else
			{
				// Failed to create the template
				ptItem->templateFile.destroy();
				ptItem.destroy();
				return NULL;
			}
		}
	}
	// The component template does not exist
	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
ETemplateType NdhsCPageManager::getTemplateType(LcCXmlElem* eSettings)
{
	// check for valid setting element
	if (!eSettings)
		return ETemplateTypeInvalid;

	// Design Width
	LcTaString templateType = eSettings->getAttr(NDHS_TP_XML_TEMPLATE_TYPE);

	if (templateType.compareNoCase("component") == 0)
		return ETemplateTypeComponent;
	else if (templateType.compareNoCase("menuComponent") == 0)
		return ETemplateTypeMenuComponent;
	else if (templateType.compareNoCase("page") == 0)
		return ETemplateTypePage;

	return ETemplateTypeInvalid;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::releaseTemplate(NdhsCTemplate* templateFile)
{
	TmMPageTemplateCache::iterator it = m_templates.begin();
	for (; it != m_templates.end(); it++)
	{
		CTemplateItem* ptItem = it->second;

		if (ptItem->templateFile.ptr() == templateFile)
		{
			if (--ptItem->refCount <= 0)
			{
				// No longer needed, so destroy
				ptItem->templateFile.destroy();

				m_templates.erase(it);
			}

			return;
		}
	}
}

/*--------------------------------------------*//**
*/
void NdhsCPageManager::releaseMenu(NdhsCMenu* menu, NdhsCMenuComponent *caller)
{
	TmMMenuCache::iterator it = m_menus.begin();
	for (; it != m_menus.end(); it++)
	{
		CMenuItem* ptItem = it->second;

		if (ptItem->menu.ptr() == menu)
		{
			ptItem->removeReference(caller);

			if (ptItem->isUnreferenced())
			{
				// No longer needed, so destroy
				ptItem->menu.destroy();

				m_menus.erase(it);
			}
			return;
		}
	}
}

#ifdef IFX_SERIALIZATION

/*--------------------------------------------*//**
*/
void NdhsCPageManager::incMenuRefCount(NdhsCMenu* menu, NdhsCMenuComponent *caller)
{
	TmMMenuCache::iterator it = m_menus.begin();
	for (; it != m_menus.end(); it++)
	{
		CMenuItem* ptItem = it->second;

		if (ptItem->menu.ptr() == menu)
		{
			ptItem->addReference(caller);
			return;
		}
	}
}

#endif /* IFX_SERIALIZATION */

/*--------------------------------------------*//**
*/
void NdhsCPageManager::releaseUiNode(NdhsCPage* uiNode)
{
	TmANodeCache::iterator it = m_nodes.begin();
	for (; it != m_nodes.end(); it++)
	{
		CNodeItem* ptItem = it->second;

		if (ptItem->uiNode.ptr() == uiNode)
		{
			if (--ptItem->refCount <= 0)
			{
				// No longer needed, so destroy
				ptItem->uiNode.destroy();

				m_nodes.erase(it);
			}
			return;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcCBitmap* NdhsCPageManager::getBitmap(const LcTmString&	file,
										int					stackLevel,
										NdhsCManifest*		paletteManifest)
{
	LcTaString bitmap = file;
	LcTaString farBitmap = file;
	LcTaString outBitmap;
	LcTaString ext;
	LcTaArray<NdhsCManifest::CManifestFile*> fileData;

	// Get file extension from file
	ext = bitmap.getWord(-1, '.');

	LcTaString appPath=getCon()->getAppPath();

	// If its an absolute path, do not need to use the manifest
	if (bitmap[IFX_PLAT_DRIVE_SEP_POS] == IFX_PLAT_DRIVE_SEP_CHAR)
	{
		outBitmap=getFileData(bitmap,&fileData);
		if(outBitmap.isEmpty())
		{
			int appPathPos=bitmap.find(appPath,0);
			if(appPathPos==-1 || bitmap.length()<=appPath.length())
			{
				// Set the directory slash separators to the non default if required.
				#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
					bitmap.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
				#endif

				return getSpace()->getBitmap(bitmap);
			}
			else
			{
				bitmap=bitmap.subString(appPath.length(),bitmap.length()-appPath.length());
			}
		}
	}
	if(fileData.size()!=0 || m_manifestStack->findFile(bitmap, outBitmap, paletteManifest, stackLevel,NULL,true,&fileData))
	{
		int index=-1;
		if(findBitmapFile(&fileData,index))
		{
			bitmap = fileData[index]->absolutePath;
			return  getSpace()->getBitmap(bitmap,
									fileData[index]->m_marginLeft,
									fileData[index]->m_marginRight,
									fileData[index]->m_marginTop,
									fileData[index]->m_marginBottom,
									fileData[index]->m_frameCount);
		}
	}
#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
	farBitmap.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
#endif
	return getSpace()->getBitmap(farBitmap);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::findBitmapFile(LcTaArray<NdhsCManifest::CManifestFile*> *fileData, int &index)
{
	bool output=false;
	LcTaString bitmapPath="";
	LcCSpace  *space=getSpace();

	if(fileData==NULL || fileData->size()==0)
		return output;

	index=0;
	LcTmArray<NdhsCManifest::CManifestFile*>::iterator iter=fileData->begin();
	for(;iter!=fileData->end();++iter)
	{
		if((*iter)!=NULL)
		{
			bitmapPath =(*iter)->absolutePath;
			output=space->isBitmapFile(bitmapPath);
			if(output)
				break;
		}
		++index;
	}
	return output;
}

/*-------------------------------------------------------------------------*//**
*/
LcCFont* NdhsCPageManager::getFont(const LcTmString&	name,
									LcCFont::EStyle		style,
									int					stackLevel)
{
	LcTaString font = name;
	LcTaString outFont;
	LcTaString ext;

	m_tokenStack->replaceTokens(name, font, NULL, NULL, NULL, NULL, stackLevel);

	if (font.length() > 0)
	{
		// Get file extension from file
		ext = font.getWord(-1, '.');

		// Bitmap font found
		if (ext.compareNoCase("png") == 0)
		{
			// Before loading replace extension for our file type
			font = font.subString(0, font.length() - ext.length()) + "ndi";
		}

		// If its an absolute path, do need to use the manifest
		if (font[IFX_PLAT_DRIVE_SEP_POS] == IFX_PLAT_DRIVE_SEP_CHAR)
		{
			// Set the directory slash separators to the non default if required.
			#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
				font.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
			#endif

			return getSpace()->getFont(font, style);
		}

		// See if there is a parent manifest to take account of.
		NdhsCManifest* paletteManifest = getPaletteManifest(font);
		if (NULL == paletteManifest && m_lastRealizedElement)
		{
			if (m_lastRealizedElement->getElementType() == ENdhsElementTypeText)
				paletteManifest = ((NdhsCTextElement*)m_lastRealizedElement)->getParentPaletteMan();
		}

		// Not an absolute path, so check the manifest
		if (m_manifestStack->findFile(font, outFont, paletteManifest, stackLevel))
		{
			font = outFont;
		}

		return getSpace()->getFont(font, style);
	}

	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
LcCFont* NdhsCPageManager::getFont(const LcTmString& name, LcCFont::EStyle style)
{
	return getFont(name, style, m_manifestStack->getStackHeight() - 1);
}

#ifdef LC_USE_MESHES
/*-------------------------------------------------------------------------*//**
*/
LcCMesh* NdhsCPageManager::getMesh(const LcTmString& file,
									NdhsCElement* element,
									NdhsCMenu* menu,
									NdhsCMenuItem* menuItem,
									int stackLevel,
									NdhsCManifest* paletteManifest)
{
	LcTaString mesh;
	LcTaString texturePath="";
	LcTaString outMesh;
	LcTaString ext;

	m_tokenStack->replaceTokens(file, mesh, element, menu, menuItem,  NULL, stackLevel);

	// Get file extension from file
	ext = mesh.getWord(-1, '.');

	// Before loading replace extension for our file type
	mesh = mesh.subString(0, mesh.length() - ext.length()) + "nd3";

	TBitmapLoader bitmapLoader;
	bitmapLoader.stackLevel = stackLevel;
	bitmapLoader.paletteManifest = paletteManifest;
	bitmapLoader.pageManager = this;

	// If its an absolute path, do need to use the manifest
	if (mesh[IFX_PLAT_DRIVE_SEP_POS] == IFX_PLAT_DRIVE_SEP_CHAR)
	{
		// Set the directory slash separators to the non default if required.
		#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
			mesh.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
		#endif

		LcCMesh* pMesh = getSpace()->getMesh(mesh);
		if (pMesh)
		{
			pMesh->initializeData(&bitmapLoader);

			return pMesh;
		}
	}

	// Not an absolute path, so check the manifest
	if (m_manifestStack->findFile(mesh, outMesh, paletteManifest, stackLevel))
	{
		texturePath = mesh.subString(0, mesh.length() - mesh.getWord(-1, NDHS_DIR_SEP_CHAR).length());
		mesh = outMesh;

		LcCMesh* pMesh = getSpace()->getMesh(mesh);
		if (pMesh)
		{
			pMesh->setTextureRelativePath(texturePath);
			pMesh->initializeData(&bitmapLoader);

			return pMesh;
		}
	}
	return NULL;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::setMatchedBackground(int stackLevel)
{
	LcTaString	newBackground = "";
	LcTaString	returnedKeyVal = "";

	if (m_activePage < 0)
		return;

	int pagePos = 0;

	// If the background change message is scheduled,
	// we may as well cancel it
	if (m_changeBackgroundMessage.isScheduled())
	{
		m_changeBackgroundMessage.cancel();
	}

	// Back up through entire page stack checking for last available background if current menu doesn't have one
	for (pagePos = m_activePage; pagePos >= 0; pagePos--)
	{
		// Try to find a matched token for the current menu.
		// If one does not exist then use the default theme background.
		if (m_tokenStack->getBestMatchValue("background." + m_pageStack[pagePos]->getPage()->getPageName(), returnedKeyVal, stackLevel))
		{
			m_tokenStack->replaceTokens(returnedKeyVal, newBackground, NULL, NULL, NULL, NULL, stackLevel);
			break;
		}
	}

	(void)setBackground(newBackground, stackLevel);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::setBackground(const LcTmString& file, int stackLevel)
{
	bool found = false;
	int index=-1;
	LcTaString fullPath;

	// Protect this section against OOM events
	volatile int memoryException = 0;
	LC_CLEANUP_PUSH_FRAME(memoryException);

	// If we have an exception, then we will jump back to here with
	// memoryException non-zero, so handle the error in the else.

	if (!memoryException)
	{
		LcTaArray<NdhsCManifest::CManifestFile*> fileData;
		// Get palette name.
		NdhsCManifest* paletteMan = getActivePage()->getTemplate()->getPaletteManifest();

		if (paletteMan == NULL)
			paletteMan = getPaletteManifest(file);

		if (m_manifestStack->findFile(file, fullPath, paletteMan, stackLevel,NULL,true,&fileData))
		{
			if(findBitmapFile(&fileData,index))
				fullPath=fileData[index]->absolutePath;
			if (fullPath.compareNoCase(m_currentBackground) == 0)
			{
				// Background already set, so no need to set it again
				found = true;
			}
			else
			{
				found = getSpace()->setBackgroundImage(fullPath);
			}
		}
		else
		{
			getSpace()->setBackgroundImage("");
		}
	}
	else
	{
		IFXP_Display_Error_Note((IFX_WCHAR*)L"Out of memory - could not load background image");
		found = false;
	}

	// Get rid of cleanup frame
	LC_CLEANUP_POP_FRAME(memoryException);

	if (found)
		m_currentBackground = fullPath;
	else
	{
		// If the background was not found then paint the canvas to clear it.
		m_currentBackground = "";
		getSpace()->repaintBackground();
	}

	return found;
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::onMouseDown(const LcTPixelPoint& pt)
{
	// As far as the calling app is concerned, we
	// have consumed the down event unless the
	// widget list returned by the space is empty
	// and there is no catch all

	bool bConsumed = false;
	bool bCatchAll = false;

	// 1) Get a list of widgets from space. These are in the correct Z order.
	// 2) Ask each page model in turn for a list of widget <-> element mappings.
	// 3) Query each page model in order to determine if a catch all is present.
	// 4) Run through the list of widgets calling the page model mouse down for each

	// First, make sure any existing drag is canceled
	// This should do nothing other than reset the drag
	// state if everything is working correctly
	onMouseCancel(NULL, NULL, false);

#ifdef LC_USE_MOUSEOVER
	m_currentMouseOverElement = NULL;
#endif

	// Update mouse state to 'down' overriding any other state
	m_mouseState = EMouseDown;

	TmAWidgets widgetList;
	widgetList.clear();

	bool widgetsFound = getSpace()->findWidgetsAt(pt, widgetList);

	// We now have a list of widgets (in Z order)
	// Reserve memory for an array (+1 for the potential catch all)
	m_pageWidgetElemList.clear();
	m_pageWidgetElemList.reserve(sizeof(TPageWidgetElem) *  (widgetList.size() + 1));

	// Copy the list of widgets into this array.
	TPageWidgetElem nullEntry;

	TmAWidgets::iterator itWidgets;
	for (itWidgets = widgetList.begin(); itWidgets != widgetList.end(); itWidgets++)
	{
		m_pageWidgetElemList.push_back(nullEntry);
	}

	// Ask each page in turn to populate the element for each widget.
	// Ask each page if it has a catch all. Stop if it does.
	for (int i = m_activePage; (i >= 0) && !bCatchAll; --i)
	{
		if (widgetsFound)
			m_pageStack[i]->populateWidgetElementMap(widgetList, m_pageWidgetElemList);

		// so not handling catch all
		if (m_pageStack[i]->isCatchAll())
		{
			// Special case where the widget and element pointers are NULL.
			// This indicates the catch all case, where the event is sent to
			// the page model with a null widget / element causing the catch all
			// to be executed.
			TPageWidgetElem catchAllEntry(NULL, m_pageStack[i], false, false);
			bCatchAll = true;

			// Add catch all entry to the list.
			m_pageWidgetElemList.push_back(catchAllEntry);
		}
	}

	if (!bCatchAll)
	{
		// If no catchall has been set, add a NULL one so it can be directly accessed
		m_pageWidgetElemList.push_back(nullEntry);
	}

	TmAPageWidgetElem::iterator it;
	bool bLocalConsumed = false;
	bool bTopElementFound = false;
	bool isDragScrolling=false;

	// Iterate through the list of elements calling the page model mouse down method on each.

	// We also need to remove any graphic, text or plugin
	// elements that are behind others.  All drag regions
	// and the catch-all can stay.

	for (it = m_pageWidgetElemList.begin(); (it != m_pageWidgetElemList.end() && !bLocalConsumed); it++)
	{
		TPageWidgetElem* entry = &(*it);

		if (entry && entry->eventManager)
		{
			// Just default to drag region, only the catch-all
			// should be affected by this so this will get it
			// through the logic checks.
			ENdhsElementType elementType = ENdhsElementTypeDragRegion;
			if (entry->element)
				elementType = entry->element->getElementType();

			bool tapCatcher = (elementType ==  ENdhsElementTypeGraphic
#ifdef IFX_USE_PLUGIN_ELEMENTS
				|| elementType == ENdhsElementTypePlugin
#endif
				|| elementType == ENdhsElementTypeText);

#ifdef LC_USE_STYLUS
			if (elementType == ENdhsElementTypeDragRegion)
			{
				NdhsCDragRegionElement* dragRegion = (NdhsCDragRegionElement*)entry->element;

				if (dragRegion)
				{
					tapCatcher |= dragRegion->isTapCatcher();
				}
			}
#endif

			if (!bTopElementFound || !tapCatcher)
			{
				isDragScrolling = false;

				if(entry->element)
				{
					isDragScrolling = entry->element->isDragging();
				}

				bLocalConsumed = entry->eventManager->onMouseDown(entry, pt);

				// Set the mouse down flag
				entry->mouseDownSent = true;

				if (bLocalConsumed)
				{
					// Since we got capture on the down event,
					// we can skip directly to the dragging state
					m_dragCapturedEntry = entry;

					if(isDragScrolling)
					{
						m_mouseState = EMouseDragging;
						onMouseCancel(NULL, NULL, true);

						if(entry->element)
						{
							entry->element->setTouchdownStatus(true, true);
						}
					}
				}

				if (!bTopElementFound && tapCatcher)
				{
					bTopElementFound = true;
				}
			}
			else
			{
				// We already have our top tappable element
				// So we are not interested in this one
				entry->mouseDownSent = false;
				entry->element = NULL;
				entry->eventManager = NULL;
			}
		}
	}

	// Now decide whether it was consumed
	// for the purpose of the calling app
	if (widgetsFound || bCatchAll)
	{
		bConsumed = true;
	}

#ifdef IFX_GENERATE_SCRIPTS
	//capture mouse Down event
	if (m_scriptGenerator)
		m_scriptGenerator->attachTouchEvent(pt, TouchEventDown, getSpace()->getTimestamp());
#endif //IFX_GENERATE_SCRIPTS

	return bConsumed;
}
#endif

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::onMouseMove(const LcTPixelPoint& pt)
{
	bool bConsumed = false;
	TmAPageWidgetElem::iterator wit;

	if (m_dragCapturedEntry && (m_mouseState == EMouseDragging))
	{
		bConsumed = m_dragCapturedEntry->eventManager->onMouseMove(m_dragCapturedEntry, pt);

		if (!bConsumed)
		{
			// If the captured element releases capture, we
			// must not handle any more mouse events until
			// the next mouse down event.
			onMouseCancel(NULL, NULL, false);
		}
#ifdef IFX_GENERATE_SCRIPTS
		//capture mouse Move event
		if (m_scriptGenerator)
			m_scriptGenerator->attachTouchEvent(pt, TouchEventMove, getSpace()->getTimestamp());
#endif //IFX_GENERATE_SCRIPTS

		return bConsumed;
	}

	// Either the above code was not executed, or
	// the captured entry does not consume the event
	// so it has released its capture.

#ifdef LC_USE_MOUSEOVER
	// See if we need to reset mouse over related intrinsics
	if (m_currentMouseOverElement == NULL)
	{
		resetMouseOverDisplacement(pt);
	}

	// If we're looking for mouseover, only regenerate the Widget list if the move threshold is breached
	if ((m_mouseState != EMouseDown) && (m_doMouseOverHitTesting || scheduleMouseOverHitTest(pt)))
	{
		// 1) Get a list of Widgets from space. These are in the correct Z order.
		// 2) Ask each page model in turn for a list of widget <-> element mappings.

		TmAWidgets WidgetList;
		WidgetList.clear();
		bool WidgetsFound = getSpace()->findWidgetsAt(pt, WidgetList);

		if (WidgetsFound)
		{
			// We are in mouse over state
			m_mouseState = EMouseOver;

			// Clear element list
			m_pageWidgetElemList.clear();

			// Reserve memory for an array
			m_pageWidgetElemList.reserve(sizeof(TPageWidgetElem) *  (WidgetList.size()));

			// Copy the list of Widgets into this array.
			TPageWidgetElem nullEntry;

			TmAWidgets::iterator itWidgets;
			for (itWidgets = WidgetList.begin(); itWidgets != WidgetList.end(); itWidgets++)
			{
				m_pageWidgetElemList.push_back(nullEntry);
			}

			// Ask each page in turn to populate the element for each Widget.
			for (int i = m_activePage; (i >= 0); --i)
			{
				m_pageStack[i]->populateWidgetElementMap(WidgetList, m_pageWidgetElemList);
			}
		}
	}
#endif

	// We only want to send a move event if the mouse was previously pressed or we have a mouseover event
	if (m_mouseState == EMouseDown || m_mouseState == EMouseOver)
	{
		// Iterate through the list of elements calling the mouse moved method on each in case of mouse down, for mouseOver make sure
		// we call mouseMove only if we had a successful mouseOver hit testing earlier
		for (wit = m_pageWidgetElemList.begin(); (wit != m_pageWidgetElemList.end()) && !bConsumed && (m_mouseState == EMouseDown
#ifdef LC_USE_MOUSEOVER
																					|| (m_mouseState == EMouseOver && m_doMouseOverHitTesting
																					&&  m_currentMouseOverElement == NULL)
#endif
																					); wit++)
		{
			TPageWidgetElem* entry = &(*wit);
			if (entry && entry->eventManager)
			{

				bConsumed = entry->eventManager->onMouseMove(entry, pt);
			}

			if (bConsumed)
			{
				if (m_mouseState == EMouseDown)
				{
					m_mouseState = EMouseDragging;
					m_dragCapturedEntry = entry;

					// Cancel all the others
					onMouseCancel(NULL, NULL, true);

					if(entry->element)
					{
						entry->element->setTouchdownStatus(true, true);
					}
				}
			}
		}
#ifdef LC_USE_MOUSEOVER
		if (m_mouseState == EMouseOver && m_currentMouseOverElement == NULL)
		{
			m_mouseState = EMouseUp;
		}
#endif
	}

#ifdef LC_USE_MOUSEOVER
	// Reset hit testing flag
	m_doMouseOverHitTesting = false;
#endif

#ifdef IFX_GENERATE_SCRIPTS
	//capture mouse Move event
	if (m_scriptGenerator)
		m_scriptGenerator->attachTouchEvent(pt, TouchEventMove, getSpace()->getTimestamp());
#endif //IFX_GENERATE_SCRIPTS

	return bConsumed;
}
#endif

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::onMouseUp(const LcTPixelPoint& pt)
{
	bool bConsumed = false;

#ifdef LC_USE_MOUSEOVER
	m_currentMouseOverElement = NULL;
#endif

	if (m_dragCapturedEntry && (m_mouseState == EMouseDragging))
	{
		bConsumed = m_dragCapturedEntry->eventManager->onMouseUp(m_dragCapturedEntry, pt);

		m_mouseState = EMouseUp;
		m_dragCapturedEntry = NULL;
		m_pageWidgetElemList.clear();

#ifdef IFX_GENERATE_SCRIPTS
	//capture mouse Up event
	if (m_scriptGenerator)
		m_scriptGenerator->attachTouchEvent(pt, TouchEventUp, getSpace()->getTimestamp());
#endif //IFX_GENERATE_SCRIPTS

		return bConsumed;
	}

	// Either the above code was not executed, or
	// the captured entry does not consume the event
	// so it has released its capture.

	// We only want to send a up event if the mouse was previously pressed
	if (m_mouseState == EMouseDown)
	{
		if (m_pageWidgetElemList.size() > 0)
		{
			TmAPageWidgetElem::iterator it = m_pageWidgetElemList.begin();
			TmAPageWidgetElem::iterator catchallIterator = m_pageWidgetElemList.end() - 1;

			// Iterate through the list of elements calling the page model mouse up method on each.
			while ((it != m_pageWidgetElemList.end()) && !bConsumed)
			{
				TPageWidgetElem* entry = &(*it);
				if (entry && entry->eventManager)
				{
					bConsumed = entry->eventManager->onMouseUp(entry, pt);

					// Set the mouse down flag to false so that
					// the element doesn't get a cancel event
					entry->mouseDownSent = false;

					if (!bConsumed && entry->element && entry->element->getElementType() != ENdhsElementTypeDragRegion
						&& it != catchallIterator)
					{
						// If the tap hasn't been consumed, and we're not looking at a drag region, and we're not
						// already doing the catchall trigger, then jump ahead to try the catchall trigger next
						it = catchallIterator;
					}
					else
					{
						it++;
					}
				}
				else
				{
					it++;
				}
			}
		}
	}


	// Reset the state machine and
	// send any cancel events
	onMouseCancel(NULL, NULL, false);

#ifdef LC_USE_MOUSEOVER
	if (m_mouseState == EMouseUp)
	{
		scheduleMouseOverHitTest(pt);
	}
#endif

#ifdef IFX_GENERATE_SCRIPTS
	//capture mouse Up event
	if (m_scriptGenerator)
		m_scriptGenerator->attachTouchEvent(pt, TouchEventUp, getSpace()->getTimestamp());
#endif //IFX_GENERATE_SCRIPTS

	return bConsumed;
}
#endif

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::onMouseCancel(NdhsCElementGroup* page, NdhsCElement* element, bool allButCaptured)
{
	// First, allButActive is the highest priority.  It cancels
	// all elements except the active.  If false, it allows
	// the other filters to kick in.
	//
	// Second, the element context it used to cancel a
	// specific element. We have to assume that the
	// supplied element is on the supplied page
	//
	// Third, the page context is used to cancel all
	// elements on a page.
	//
	// Finally, all elements are canceled if no context.

	// Elements are only notified of a mouse cancel
	// if they were notified of the mouse down.

	// Normally, we can assume that cancel events have
	// been issued if capture has been granted, except
	// in the case where we are explicitly canceling
	// all but captured

	if (allButCaptured)
	{
		TmAPageWidgetElem::iterator it;

		// Iterate through the list of elements to cancel them
		for (it = m_pageWidgetElemList.begin(); it != m_pageWidgetElemList.end(); it++)
		{
			TPageWidgetElem* entry = &(*it);
			if (entry)
			{
				// Make sure its not the captured element
				if (!(m_dragCapturedEntry
					&& (m_dragCapturedEntry->element == entry->element)
					&& (m_mouseState == EMouseDragging)))
				{
					if (entry->element && entry->mouseDownSent
						&& entry->eventManager
						&& entry->ignoreEntry == false
						&& entry->eventManager->isGroupUnloaded() == false)
					{
						// Must cancel if it had received a mouse down
						entry->element->onMouseCancel();
					}

					entry->mouseDownSent = false;
					entry->element = NULL;
					entry->eventManager = NULL;
				}
			}
		}
	}
	else if (element)
	{
		// Here, we are specifically canceling
		// any drag on a specific element

		if (m_dragCapturedEntry
			&& (m_dragCapturedEntry->element == element)
			&& (m_mouseState == EMouseDragging)
			&& m_dragCapturedEntry->eventManager
			&& m_dragCapturedEntry->ignoreEntry == false
			&& m_dragCapturedEntry->eventManager->isGroupUnloaded() == false)
		{
			m_dragCapturedEntry->element->onMouseCancel();

			m_mouseState = EMouseUp;
			m_dragCapturedEntry = NULL;
			m_pageWidgetElemList.clear();
		}
		else if (m_mouseState == EMouseDown)
		{
			// Not captured yet, but remove interest
			// for the specified element because it
			// may now be an invalid pointer

			TmAPageWidgetElem::iterator it;

			// Iterate through the list of elements to find the one we are canceling
			for (it = m_pageWidgetElemList.begin(); it != m_pageWidgetElemList.end(); it++)
			{
				TPageWidgetElem* entry = &(*it);
				if (entry && (entry->element == element))
				{
					if (entry->mouseDownSent
						&& entry->ignoreEntry == false
						&& entry->eventManager
						&& entry->eventManager->isGroupUnloaded() == false)
					{
						// Must cancel if it had received a mouse down
						entry->element->onMouseCancel();
					}

					entry->mouseDownSent = false;
					entry->element = NULL;
					entry->eventManager = NULL;
					break;
				}
			}
		}
	}
	else if (page)
	{
		// Here, we are specifically canceling
		// any drag on a specific page

		if (m_dragCapturedEntry
			&& (m_dragCapturedEntry->eventManager == page)
			&& (m_mouseState == EMouseDragging))
		{
			if (m_dragCapturedEntry->element
				&& m_dragCapturedEntry->ignoreEntry == false
				&& m_dragCapturedEntry->eventManager
				&& m_dragCapturedEntry->eventManager->isGroupUnloaded() == false)
			{
				m_dragCapturedEntry->element->onMouseCancel();
			}

			m_mouseState = EMouseUp;
			m_dragCapturedEntry = NULL;
			m_pageWidgetElemList.clear();
		}
		else if (m_mouseState == EMouseDown)
		{
			// Not captured yet, but remove interest
			// for the specified page because it
			// may now be an invalid pointer

			TmAPageWidgetElem::iterator it;

			// Iterate through the list of elements calling the page model mouse down method on each.
			for (it = m_pageWidgetElemList.begin(); it != m_pageWidgetElemList.end(); it++)
			{
				TPageWidgetElem* entry = &(*it);
				if (entry && (entry->eventManager == page))
				{
					if (entry->element && entry->mouseDownSent
						&& entry->ignoreEntry == false
						&& entry->eventManager
						&& entry->eventManager->isGroupUnloaded() == false)
					{
						// Must cancel if it had received a mouse down
						entry->element->onMouseCancel();
					}

					entry->mouseDownSent = false;
					entry->element = NULL;
					entry->eventManager = NULL;
				}
			}
		}
	}
	else
	{
		// The general case.  Just cancel the drag operation

		if (m_dragCapturedEntry && (m_mouseState == EMouseDragging))
		{
			if (m_dragCapturedEntry->element
				&& m_dragCapturedEntry->ignoreEntry == false
				&& m_dragCapturedEntry->eventManager
				&& m_dragCapturedEntry->eventManager->isGroupUnloaded() == false)
			{
				m_dragCapturedEntry->element->onMouseCancel();
			}
		}
		else
		{
			// Capture had not been granted, so we may
			// need to send cancel events to elements
			TmAPageWidgetElem::iterator it;

			// Iterate through the list of elements calling the page model mouse down method on each.
			for (it = m_pageWidgetElemList.begin(); it != m_pageWidgetElemList.end(); it++)
			{
				TPageWidgetElem* entry = &(*it);
				if (entry && entry->mouseDownSent
					&& entry->ignoreEntry == false
					&& entry->element && entry->eventManager
					&& entry->eventManager->isGroupUnloaded() == false)
				{
					// Must cancel if it had received a mouse down
					entry->element->onMouseCancel();
				}
			}
		}

		m_mouseState = EMouseUp;
		m_dragCapturedEntry = NULL;
		m_pageWidgetElemList.clear();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::ignoreEntry(NdhsCElementGroup* group)
{
	TmAPageWidgetElem::iterator it;
	// Iterate through the list of elements, and see which we need to ignore
	for (it = m_pageWidgetElemList.begin(); it != m_pageWidgetElemList.end(); it++)
	{
		TPageWidgetElem* entry = &(*it);
		if (entry && entry->eventManager
			&& entry->eventManager == group)
		{
			entry->ignoreEntry = true;
		}
	}
}

#endif

#ifdef LC_USE_MOUSEOVER
/*-------------------------------------------------------------------------*//**
																			 */
void NdhsCPageManager::resetMouseOverDisplacement(const LcTPixelPoint& pt, bool force)
{
	if (getSpace() && m_currentMouseOverTestElement)
	{
		if (force || m_currentMouseOverTestElement->getWidget()->contains(getSpace()->mapCanvasToLocal(pt, *(m_currentMouseOverTestElement->getWidget())), 0) == false)
		{
			m_currentMouseOverTestElement->onMouseCancel();
			m_currentMouseOverTestElement = NULL;
		}
	}
}

/*-------------------------------------------------------------------------*//**
																			 */
void NdhsCPageManager::resetMouseOverElement(NdhsCElement* mouseOverElement)
{
	if (mouseOverElement == m_currentMouseOverTestElement || mouseOverElement == m_currentMouseOverElement)
	{
		m_currentMouseOverTestElement = NULL;
		m_currentMouseOverElement = NULL;
	}
}

/*-------------------------------------------------------------------------*//**
																			 */
void NdhsCPageManager::displacementApplied()
{
	if (m_currentMouseOverTestElement && m_mouseState == EMouseOver)
	{
		m_currentMouseOverElement = m_currentMouseOverTestElement;
	}
}

/*-------------------------------------------------------------------------*//**
																			 */
bool NdhsCPageManager::scheduleMouseOverHitTest(const LcTPixelPoint& pt)
{
	if (m_activePage >= 0)
	{
		if (m_mouseOverDelay > 0)
		{
			// Store current mouse over location
			m_mouseOverCurrentPt = pt;

			LcTVector vPoint;

			if (getSpace() && m_currentMouseOverElement)
			{
				vPoint = getSpace()->mapCanvasToLocal(pt, *(m_currentMouseOverElement->getWidget()));
			}

			if (m_currentMouseOverElement == NULL || m_currentMouseOverElement->getWidget()->contains(vPoint, 0) == false)
			{
				if (m_doMouseOverHitTestMessage.isScheduled())
				{
					m_doMouseOverHitTestMessage.cancel();
				}

				m_doMouseOverHitTestMessage.schedule(getSpace()->getTimer(), 0, m_mouseOverDelay);

				if (m_mouseState == EMouseOver && m_currentMouseOverElement)
				{
					m_currentMouseOverElement->onMouseCancel();
					m_mouseState = EMouseUp;
				}
				m_currentMouseOverElement = NULL;
			}
		}
	}
	return false;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::savePersistentData()
{
	m_tokenStack->saveIniTokens(m_dataPath, m_iniTokenFile);
}

#ifdef IFX_USE_PLUGIN_ELEMENTS
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::makeFullScreen(IFX_HELEMENT hElement, bool bStartFullScreen)
{
	bool bSuccess = false;

	// Notify the plugin element
	NdhsCPlugin::NdhsCPluginHElement* element = findOwnerPluginHandler(hElement);

	if (element != NULL)
		bSuccess = element->makeFullScreen(bStartFullScreen);


	if(bSuccess == true)
	{
		if(bStartFullScreen == true)
		{
			getSpace()->setFullScreenMode(true);
		}
		else
		{
			getSpace()->setFullScreenMode(false);

			// We need to regenerate the whole screen
			getSpace()->revalidateAll();
		}
	}

	return bSuccess;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPlugin::NdhsCPluginHElement* NdhsCPageManager::findOwnerPluginHandler(IFX_HELEMENT hElement)
{
	NdhsCPlugin::NdhsCPluginHElement* retVal = NULL;

	TmMPluginElementCache::iterator it = m_pluginElements.begin();
	for (; it != m_pluginElements.end(); it++)
	{
		CPluginElementItem* ptItem = it->second;

		if (ptItem->pluginElement->isPluginElement(hElement))
		{
			retVal = ptItem->pluginElement.ptr();
		}
	}

	return retVal;
}
#endif

/*-------------------------------------------------------------------------*//**
	Iterate through the refresh queue to action as many refreshes as possible
	before finding one that requires an intermediate screen refresh (i.e. a menu
	refresh).
*/
bool NdhsCPageManager::doPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame)
{
	bool reschedule = false;
	bool stateChangesComplete = true;

	volatile int memError = 0;

	// Prepare cleanup frame
	LC_CLEANUP_PUSH_FRAME(memError);

	if (memError == 0)
	{
		NdhsCLaundry* laundry = getCon()->getLaundry();

		if (laundry && !laundry->isEmpty())
		{
			// check return value for the case of animating variables etc
			reschedule |= laundry->cleanAll();
		}

		// Update the fields in the cache
		if (m_plugin)
		{
			reschedule |= m_plugin->updateGlobalFields(timestamp);
		}

		// Do we have any pending events?
		if( m_eventMessage.isScheduled())
		{
			m_eventMessage.cancel();

			// Iterate through refresh queue
			if(processRefreshEvents(false))
			{
				bool instantEvent = false;
				// There are events left to handle...check for the refresh page completion
				// case.  Note that in this context, the normal schedules are fine.

				// Acquire the refresh event queue mutex
				if (m_eventQueueMutex.ptr() && m_eventQueueMutex->lock(LcCMutex::EInfinite) == true)
				{
					if (m_eventListHead->eventType == EEventRefreshPageCompletion)
						instantEvent = true;

					// Release the mutex
					m_eventQueueMutex->unlock();
				}

				if (instantEvent)
					m_eventMessage.scheduleImmediate(getSpace()->getTimer(), 0);
				else
					m_eventMessage.schedule(getSpace()->getTimer(), 0, 0);
			}
		}

		// Now do any animation actions that are pending
		// Note that we must animate all remaining pages
		// not just the active page and up.
		bool pageFinalFrame;
		finalFrame = true;
		for (int i = m_pageStack.size() - 1; i >= 0; i--)
		{
			reschedule |= m_pageStack[i]->doPrepareForFrameUpdate(timestamp, pageFinalFrame);
			reschedule |= m_pageStack[i]->componentDoPrepareForFrameUpdate(timestamp, pageFinalFrame);

			stateChangesComplete &= m_pageStack[i]->getStateChangeAnimComplete();
			finalFrame &= pageFinalFrame;
		}

		if (m_primaryLightOverride)
			doPrimaryLightOverride(timestamp);

		// OK...all pages have finished, lets tidy up.
		if (m_animatingStateChange && stateChangesComplete)
			onTransitionComplete(false);
	}
	else
	{
		// Make sure the mutex is unlocked
		m_eventQueueMutex->unlock();

		// If event queue has been corrupted, this is serious enough to restart
		LC_CLEANUP_THROW(IFX_ERROR_RESTART);
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memError);

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::doPreFrameUpdate()
{
#ifdef IFX_USE_PLUGIN_ELEMENTS
	// Notify all cached plugin elements that we're about to go through a rendering cycle
	TmMPluginElementCache::iterator it = m_pluginElements.begin();
	for (; it != m_pluginElements.end(); it++)
		it->second->pluginElement->frameStart();
#endif
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::doPostFrameUpdate()
{
#ifdef IFX_USE_PLUGIN_ELEMENTS
	// Notify all cached plugin elements that we've completed a rendering cycle
	TmMPluginElementCache::iterator it = m_pluginElements.begin();
	for (; it != m_pluginElements.end(); it++)
		it->second->pluginElement->frameEnd();
#endif
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::updateFields(LcTTime timestamp)
{
	bool reschedule = false;

	// Update all menus fields
	TmMMenuCache::iterator it = m_menus.begin();
	while (it != m_menus.end())
	{
		CMenuItem* menuItem = it->second;

		if (menuItem && menuItem->menu.ptr())
			reschedule |= menuItem->menu->updateFields(timestamp);
	}
	return reschedule;
}

/*-------------------------------------------------------------------------*//**
	Traverses refresh event list, attempting to perform each refresh request
	in turn.  Terminates on finding any refresh request that will require
	a repaint before it can complete.
	This method returns 'true' if there are more events to handle in the list.
*/
bool NdhsCPageManager::processRefreshEvents(bool refreshTimeout)
{
	bool bContinue = true;

	// Nothing to do!
	if (!m_eventListHead)
		return false;

	TEventInfo* eventListHead = NULL;
	TEventInfo* eventListTail = m_eventListTail;

	// Now do each refresh request in turn
	while(bContinue)
	{
		// If we can't acquire the lock on the mutex, return true if there are events
		// to process, false otherwise
		if (m_eventQueueMutex.ptr() == NULL || m_eventQueueMutex->lock(LcCMutex::EInfinite) == false)
		{
			return (m_eventListHead != NULL);
		}

		eventListHead = m_eventListHead;

		switch(eventListHead->eventType)
		{
			case EEventRefreshPage:
			{
				// Stop any transitions first
				jumpTransitionToEnd();

				// Change type of event
				eventListHead->eventType = EEventRefreshPageCompletion;

				// Note that we shouldn't process any more events
				bContinue = false;

				break;
			}

			case EEventRefreshPageCompletion:
			{
				if(refreshTimeout == false)
					bContinue = false;

				break;
			}

			case EEventTrigger:
			{
				// If we're looking at events as part of a pre-frame update check, do not
				// action triggers - they must wait until the refresh message times out, as
				// they may take too long to complete
				// Note that this trigger is from an integrated module
				if(refreshTimeout == false)
					bContinue = false;

				break;
			}

			case EEventLink:
			{
				// If we're looking at events as part of a pre-frame update check, do not
				// action links - they must wait until the refresh message times out, as
				// they may take too long to complete
				if(refreshTimeout == false)
					bContinue = false;

				break;
			}

			case EEventSetActiveItem:
			{
				if(refreshTimeout == false)
					bContinue = false;

				break;
			}

			default:
			{
				break;
			}
		}

		// If we're processing another event, clean up the memory associated with
		// the current one.
		if(bContinue)
		{
			m_eventListHead = m_eventListHead->next;
		}

		// We got the head so release the lock
		m_eventQueueMutex->unlock();

		if(bContinue)
		{
			switch(eventListHead->eventType)
			{
#ifdef IFX_USE_PLUGIN_ELEMENTS
				case EEventRefreshBufferedElement:
				{
					// Find the plugin element that owns this handle
					NdhsCPlugin::NdhsCPluginHElement* element = findOwnerPluginHandler(eventListHead->element);

					if (element != NULL)
						element->refreshBufferedElement();

					break;
				}
#endif
				case EEventRefreshPageCompletion:
				{
					// Note that refreshing one menu may cause others to be added or 
					// removed, so expect any STL iterator to  be invalidated during the
					// calls to notifyMenuReload.  The 'break' below is critical!
					TmMMenuCache::iterator it = m_menus.begin();
					for (; it != m_menus.end(); it++)
					{
						CMenuItem* ptItem = it->second;

						NdhsCPlugin::NdhsCPluginMenu* pluginMenu = ptItem->menu->getMenuPlugin();
						if (pluginMenu != NULL && pluginMenu->getMenuSession() == eventListHead->menu)
						{
							ptItem->notifyMenuReload(true);

							ptItem->menu->reloadMenu();

							ptItem->notifyMenuReload(false);
							break;
						}
					}

					onTransitionComplete(false);

					break;
				}

				case EEventRefreshField:
				{
					LcTaString field;
					if (eventListHead->fieldOrUri)
						field.fromBufWChar(eventListHead->fieldOrUri, (int)lc_wcslen((const IFX_WCHAR*)eventListHead->fieldOrUri));

					doRefreshField(eventListHead->menu, eventListHead->item, field);

					break;
				}

				case EEventTrigger:
				{
					// If we're looking at events as part of a pre-frame update check, do not
					// action triggers - they must wait until the refresh message times out, as
					// they may take too long to complete
					// Note that this trigger is from an integrated module
					onKeyDown(eventListHead->key, true);

					break;
				}

				case EEventLink:
				{
					// If we're looking at events as part of a pre-frame update check, do not
					// action links - they must wait until the refresh message times out, as
					// they may take too long to complete

					// This needs to be a TmArray, otherwise it causes compilation problems in ADS.
					LcTmOwnerArray<NdhsCTemplate::CAction::CAttempt> linkAttempts;

					LcTaOwner<NdhsCTemplate::CAction::CLink> linkItem = NdhsCTemplate::CAction::CLink::create();

					linkItem->attemptType = NdhsCTemplate::CAction::CAttempt::ELink;

					LcTaString uri;

					if (eventListHead->fieldOrUri)
						uri.fromBufWChar(eventListHead->fieldOrUri, (int)lc_wcslen((const IFX_WCHAR*)eventListHead->fieldOrUri));

					linkItem->uriString = uri;

					LcTaOwner<NdhsCTemplate::CAction::CAttempt> linkItemTemp(linkItem);
					linkAttempts.push_back(linkItemTemp);

					onSimulateLinkTrigger(&linkAttempts);

					break;
				}

				case EEventSetActiveItem:
				{
					TmMMenuCache::iterator it = m_menus.begin();
					for (; it != m_menus.end(); it++)
					{
						CMenuItem* ptItem = it->second;

						NdhsCPlugin::NdhsCPluginMenu* pluginMenu = ptItem->menu->getMenuPlugin();
						if (pluginMenu != NULL && pluginMenu->getMenuSession() == eventListHead->menu)
						{
							ptItem->notifyActiveItemChange(eventListHead->item);
							break;
						}
					}

					break;
				}

				default:
				{
					// Unhandled event
					LC_ASSERT(false);

					break;
				}
			}

			if(eventListHead == eventListTail)
				bContinue = false;

			TEventInfo::destroy(eventListHead);

			if(!m_eventListHead)
				bContinue = false;
		}
	}

	return (m_eventListHead != NULL);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::setupChangeBackgroundMessage(int subPageIndex)
{
	// Cancel any in progress
	if(m_changeBackgroundMessage.isScheduled())
	{
		m_changeBackgroundMessage.cancel();
	}

	// Set up new delay
	int subBackgroundDelay = m_pageStack[subPageIndex]->getBackgroundDelay();
	if (subBackgroundDelay > -1)
	{
		m_changeBackgroundMessage.schedule( getSpace()->getTimer(), 0, subBackgroundDelay);
	}
}

/*-------------------------------------------------------------------------*//**
	Returns the full path to the specified theme, or an empty string if
	the theme could not be found on any drive.
*/
LcTaString NdhsCPageManager::getFullPackagePath(const LcTmString& path)
{
	LcTaString fullPath = "";

	if (!path.isEmpty())
	{
		LcTaString driveLetter;

		// Work out which drive this file is on
		driveLetter = m_con->getDriveLetter(NDHS_PACKAGE_FILENAME, path + NDHS_DIR_SEP);

		if (!driveLetter.isEmpty())
		{
			// Add the drive and path
			fullPath = driveLetter + path + NDHS_DIR_SEP;
		}
	}

	return fullPath;
}

/*-------------------------------------------------------------------------*//**
	Returns the full path to the specified palette, or an empty string if
	the palette could not be found on any drive.
*/
LcTaString NdhsCPageManager::getFullPalettePath(const LcTmString& path)
{
	LcTaString fullPath = "";

	if (!path.isEmpty())
	{
		LcTaString driveLetter;

		// Work out which drive this file is on
		driveLetter = m_con->getDriveLetter(NDHS_MANIFEST_FILENAME, path + NDHS_DIR_SEP);

		if (!driveLetter.isEmpty())
		{
			// Add the drive and path
			fullPath = driveLetter + path + NDHS_DIR_SEP;
		}
	}

	return fullPath;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::doRefreshField(IFX_HMENU hMenu, int item, const LcTmString& field)
{
	if (m_plugin)
	{
		if (hMenu != NULL)
		{
			// Update just the given menu
			NdhsCPlugin::NdhsCPluginMenu* menu = getPluginMenu(hMenu);

			if (menu)
			{
				menu->refreshField(field, item);
			}
		}
		else
		{
			// Update all menus
			TmAPageStack::iterator it = m_pageStack.begin();
			while (it != m_pageStack.end())
			{
				(*it)->componentRefreshField(field, item);
				it++;
			}

			// Refresh global
			m_plugin->refreshGlobalField(field);
		}

		getCon()->getLaundry()->cleanAll();
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPlugin::NdhsCPluginMenu* NdhsCPageManager::getPluginMenu(IFX_HMENU hMenu)
{
	TmMMenuCache::iterator it = m_menus.begin();
	for (; it != m_menus.end(); it++)
	{
		CMenuItem* ptItem = it->second;

		if (ptItem && ptItem->menu.ptr() && ptItem->menu.ptr()->getMenuSource() == NdhsCMenu::EPlugin)
		{
			if (ptItem->menu.ptr()->getMenuPlugin()->getMenuSession() == hMenu)
			{
				// We found it!
				return ptItem->menu.ptr()->getMenuPlugin();
			}
		}
	}
	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::setFieldCacheDirty()
{
	// Dirty all menus
	TmMMenuCache::iterator it = m_menus.begin();
	while (it != m_menus.end())
	{
		CMenuItem* menuItem = it->second;
		if (menuItem && menuItem->menu.ptr())
			menuItem->menu->setPluginFieldsDirty(false);
		it++;
	}

	// Dirty globals
	if (m_plugin)
		m_plugin->setGlobalFieldsDirty(false);

	getCon()->getLaundry()->cleanAll();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::stopAllFields()
{
	// Dirty all menus
	TmMMenuCache::iterator it = m_menus.begin();
	while (it != m_menus.end())
	{
		CMenuItem* menuItem = it->second;
		if (menuItem && menuItem->menu.ptr())
			menuItem->menu->stopFields();
		it++;
	}

	// Dirty globals
	if (m_plugin)
		m_plugin->stopGlobalFields();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::setPrimaryLightPlacement(const LcTPlacement& pl, int mask)
{
	if (!m_primaryLightOverride)
		getSpace()->setPrimaryLightPlacement(pl, mask);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::setupPrimaryLightTransitionOverrideFlag(int newActivePage, int oldActivePage)
{
	if (oldActivePage != newActivePage && newActivePage >=0 && oldActivePage >= 0)
	{
		m_primaryLightOverride = true;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::setupPrimaryLightTransitionOverrideTiming(int newActivePage, int oldActivePage)
{
	if (oldActivePage != newActivePage && newActivePage >=0 && oldActivePage >= 0)
	{
		LcTPlacement startPlacement;
		LcTPlacement endPlacement;
		int startMask = 0;
		int endMask = 0;

		m_pageStack[oldActivePage]->getActivePrimaryLightConfig(startPlacement, startMask);
		m_pageStack[newActivePage]->getActivePrimaryLightConfig(endPlacement, endMask);

		if ((startMask | endMask) & LcTPlacement::EOrientation)
			endPlacement.orientation.shortestPath(startPlacement.orientation);

		m_primaryLightPath->setPathData(startPlacement, endPlacement, startMask | endMask, ENdhsDecoratorTypeLinear);

		int subPage = max(newActivePage, oldActivePage);
		int parentPage = min(newActivePage, oldActivePage);

		int subPagePrimaryLightDelay = m_pageStack[subPage]->getPrimaryLightDelay();
		int subPagePrimaryLightDuration = m_pageStack[subPage]->getPrimaryLightDuration();

		int subPageTransitionDuration = m_pageStack[subPage]->getTransitionDelay() + m_pageStack[subPage]->getTransitionDuration();
		int parentPageTransitionDuration = m_pageStack[parentPage]->getTransitionDelay() + m_pageStack[parentPage]->getTransitionDuration();
		int overallTransitionTime = max(subPageTransitionDuration, parentPageTransitionDuration);

		LcTTime timestamp = getSpace()->getTimestamp();

		// Set up delay & duration, choosing defaults where necessary
		int primaryLightDelay;

		if (subPagePrimaryLightDelay == -1)
		{
			primaryLightDelay = 0;
		}
		else
		{
			primaryLightDelay = subPagePrimaryLightDelay;
		}

		if (subPagePrimaryLightDuration == -1)
		{
			primaryLightDelay = 0;
			m_primaryLightDuration = overallTransitionTime;
		}
		else
		{
			m_primaryLightDuration = subPagePrimaryLightDuration;
		}

		// Validate that it will complete within the transition
		if ((primaryLightDelay + m_primaryLightDuration) > overallTransitionTime)
		{
			primaryLightDelay = min(primaryLightDelay, overallTransitionTime);
			m_primaryLightDuration = overallTransitionTime - primaryLightDelay;
		}

		m_primaryLightOverrideTransitonBeginTimestamp = timestamp + primaryLightDelay;

		m_primaryLightOverride = true;
		m_primaryLightOverrideTransitonStarted = false;
		m_primaryLightOverrideTransitonEnded = false;

		getSpace()->setPrimaryLightPlacement(startPlacement, startMask | endMask);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::doPrimaryLightOverride(LcTTime timestamp)
{
	bool updatePlacement = false;
	LcTScalar animDelta = 0;

	if (!m_primaryLightOverrideTransitonStarted)
	{
		// Check whether to set up the transition
		if (timestamp >= m_primaryLightOverrideTransitonBeginTimestamp)
		{
			int duration = m_primaryLightDuration - (int)(timestamp - m_primaryLightOverrideTransitonBeginTimestamp);

			m_primaryLightOverrideTransitonStarted = true;

			if (duration > 0)
			{
				m_primaryLightPos->setValue(0.0, false);
				m_primaryLightPos->setTargetValue(1.0, ENdhsFieldDirectionIncreasing, false, duration, ENdhsScrollFieldModeNormal, ENdhsVelocityProfileLinear);
			}
			else
			{
				animDelta = 1;
				updatePlacement = true;
				m_primaryLightOverrideTransitonEnded = true;
			}
		}
	}

	if (m_primaryLightOverrideTransitonStarted && !m_primaryLightOverrideTransitonEnded)
	{
		m_primaryLightPos->updateValue(timestamp, m_primaryLightOverrideTransitonEnded);
		animDelta = m_primaryLightPos->getRawFieldData(NULL);
		updatePlacement = true;
	}

	if (updatePlacement)
	{
		LcTPlacement pl;
		m_primaryLightPath->getPlacement(animDelta, pl);
		int mask = m_primaryLightPath->getMask();
		getSpace()->setPrimaryLightPlacement(pl, mask);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::onSuspend()
{
	m_suspendPrimaryLightPlacement = getSpace()->getPrimaryLightPlacement();


	// Cancel m_asynchronousLaunchCompleteMessage when going to suspend. This message will be rescheduled
	// on resume.
	if (m_asynchronousLaunchCompleteMessage.isScheduled())
	{
		m_asynchronousLaunchCompleteMessage.cancel();
	}

	for (int i = m_pageStack.size() - 1; i >= 0; i--)
	{
		m_pageStack[i]->onSuspend();
	}

	m_lastRealizedElement = NULL;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::onResume()
{
	setPrimaryLightPlacement(m_suspendPrimaryLightPlacement, LcTPlacement::EAll);
	if (m_bIsAsynchronousLaunchCompleteMessagePending == true)
	{
		scheduleAsynchronousLaunchComplete();
	}

	for (int i = m_pageStack.size() - 1; i >= 0; i--)
	{
		m_pageStack[i]->onResume();
	}

	if(getSpace())
		getSpace()->revalidateAll();
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCPageManager::getDefaultFontFace()
{
	LcTaString fontName = "";
	LcTaString fontNameWithTokens;
	int stackLevel = 0;

	if (m_manifestStack)
	{
		stackLevel = m_manifestStack->getStackHeight() - 1;
	}

	if (m_defaultFontFace)
	{
		m_fontExpr = m_defaultFontFace->createExpression(NULL, -1, NULL);
		if (m_fontExpr)
		{
			m_fontExpr->evaluate(NULL, -1, NULL);

			if (m_fontExpr->isError())
			{
				m_fontExpr->errorDiagnostics("Font face", true);
			}
			else
			{
				fontNameWithTokens = m_fontExpr->getValueString();
			}
		}

		getTokenStack()->replaceTokens(fontNameWithTokens, fontName, NULL, NULL, NULL, NULL, stackLevel);
	}
	return fontName;
}
#if defined(NDHS_JNI_INTERFACE)

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::cacheCurrentStateSnapShot(LcTmOwner<CStaticPreviewCache> info)
{
	// Check we have a valid information
	if (info)
	{
		m_currentStateSnapshot.add_element(info->identifier, info);
		return true;
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageManager::CStaticPreviewCache* NdhsCPageManager::getCachedLayoutInfo(CStaticPreviewCache* info)
{
	// Try to get cached information through identifier

	if (info && m_currentStateSnapshot.size() > 0)
	{
		TmAPreviewCache::iterator it = m_currentStateSnapshot.find(info->identifier);
		if (it != m_currentStateSnapshot.end())
		{
			return it->second;
		}
	}
	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::setInPlacePreviewCacheEntry(NdhsCPageManager::CStaticPreviewCache* layoutCacheEntry)
{
	// Verify we got valid data
	if (layoutCacheEntry)
	{
		LcTaOwner<NdhsCPageManager::CStaticPreviewCache> cacheEntry = NdhsCPageManager::CStaticPreviewCache::create();

		if (cacheEntry)
		{
			cacheEntry->slotNumber = layoutCacheEntry->slotNumber;
			cacheEntry->componentClassName = layoutCacheEntry->componentClassName;
			cacheEntry->componentPath = layoutCacheEntry->componentPath;
			cacheEntry->layoutName = layoutCacheEntry->layoutName;
			cacheEntry->identifier = layoutCacheEntry->identifier;

			cacheCurrentStateSnapShot(cacheEntry);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCPageManager::getLayoutToConfigure(CStaticPreviewCache* info)
{
	// Check we have a valid information
	if (info)
	{
		CStaticPreviewCache* cachedEntry = NULL;
		TmAPreviewCache::iterator it = m_currentStateSnapshot.begin();

		while (it != m_currentStateSnapshot.end())
		{
			cachedEntry = it->second;
			if (cachedEntry)
			{
				if ((cachedEntry->componentClassName.compareNoCase(info->componentClassName) == 0)
					&& (cachedEntry->componentPath.compareNoCase(info->componentPath) == 0)
					&& (cachedEntry->slotNumber == info->slotNumber))
				{
					// We found a match
					return cachedEntry->layoutName;
				}
			}
			it++;
		}
	}
	return LcTaString("");
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageManager::CStaticPreviewCache* NdhsCPageManager::getCachedEntry(CStaticPreviewCache* entry)
{
	CStaticPreviewCache* cachedEntry = NULL;
	// Check we have a valid information
	if (entry)
	{
		TmAPreviewCache::iterator it = m_currentStateSnapshot.find(entry->identifier);

		if (it != m_currentStateSnapshot.end())
		{
			cachedEntry = it->second;

			if (cachedEntry->componentClassName.compareNoCase(entry->componentClassName) != 0
				|| cachedEntry->slotNumber != entry->slotNumber
				|| cachedEntry->componentPath.compareNoCase(entry->componentPath) != 0)
			{
				cachedEntry = NULL;
			}
		}
	}
	return cachedEntry;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageManager::CStaticPreviewCache* NdhsCPageManager::getCachedEntry(unsigned int identifier)
{
	// Check we have a valid information
	if (identifier > 0)
	{
		TmAPreviewCache::iterator it = m_currentStateSnapshot.find(identifier);

		if (it != m_currentStateSnapshot.end())
		{
			return it->second;
		}
	}
	return NULL;
}

#endif

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageManager::isAsyncBlockingMessageScheduled()
{
	if (m_asynchronousLaunchCompleteMessage.isScheduled())
	{
		return true;
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageManager::doAsyncLaunchComplete()
{
	if (m_asynchronousLaunchCompleteMessage.isScheduled())
		m_asynchronousLaunchCompleteMessage.cancel();

	m_bIsAsynchronousLaunchCompleteMessagePending = false;

	postLaunchItemAction();

	// Reset async blocking flag
	getCon()->resetAsyncLinkBlocking();

	// Repaint the screen.
	if (getSpace())
		getSpace()->revalidateAll();
}
