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
LcTaOwner<NdhsCPlugin::NdhsCPluginMenu>
NdhsCPlugin::NdhsCPluginMenu::create(NdhsCPlugin*		pluginParent,
									 IFX_HMENU			menuSession,
									 const LcTmString&	defaultMenuName)
{
	LcTaOwner<NdhsCPluginMenu> ref;
	ref.set(new NdhsCPluginMenu());
	ref->construct(pluginParent, menuSession, defaultMenuName);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Construct
*/
void NdhsCPlugin::NdhsCPluginMenu::construct(NdhsCPlugin*		pluginParent,
											 IFX_HMENU			menuSession,
											 const LcTmString&	defaultMenuName)
{
	m_pluginParent	= pluginParent;
	m_menuSession	= menuSession;

	// Ask the plugin if it wishes to override the default menu name
	if (!m_pluginParent->getElementData(-1, "", IFX_MENU_NAME, NULL, this, m_menuName))
		m_menuName = defaultMenuName;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPlugin::NdhsCPluginMenu::~NdhsCPluginMenu()
{
	if (m_pluginParent->m_pPluginCloseMenu &&
		getMenuSession())
	{
		// Close the menu
		m_pluginParent->m_pPluginCloseMenu(m_pluginParent->getPluginSession(),
										   getMenuSession());
	}
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCPlugin::NdhsCPluginMenu::getItemCount()
{
	if (m_pluginParent->m_pPluginGetItemCount &&
		getMenuSession())
	{
		// Extract item count.
		IFX_INT32 itemCount = 0;
		if (IFX_SUCCESS == m_pluginParent->m_pPluginGetItemCount(m_pluginParent->getPluginSession(), getMenuSession(), &itemCount))
		{
			if (itemCount > 0)
				return (int)itemCount;
			else
				return 0;
		}
		else
			return NDHS_COUNT_FAILED;
	}
	return NDHS_COUNT_NOT_IMPLEMENTED;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginMenu::getMenuLOD()
{
	LcTmString	menuLOD;

	m_pluginParent->getElementData(-1, "", IFX_MENU_LOAD_ON_DEMAND, NULL, this, menuLOD);

	if (menuLOD.compareNoCase("false") == 0)
		return false;
	else
		return true;
}

#if 0 /*ligui added here*/

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCPlugin::NdhsCPluginMenu::getMenuSortClass()
{
#if 1 /*ligui added here*/
	LcTaString retVal("");
#else
	LcTaString retVal("");
	m_pluginParent->getElementData(-1, "", IFX_MENU_SORT_FIELD, NULL, this, retVal);
#endif
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCPlugin::NdhsCPluginMenu::getMenuSortType()
{
#if 1 /*ligui added here*/
	LcTaString retVal("caseSensitive");
#else
	LcTaString retVal("");
	m_pluginParent->getElementData(-1, "", IFX_MENU_SORT_TYPE, NULL, this, retVal);
#endif
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCPlugin::NdhsCPluginMenu::getMenuSortDirection()
{
#if 1 /*ligui added here*/
	LcTaString retVal("ascending");
#else
	LcTaString retVal("");
	m_pluginParent->getElementData(-1, "", IFX_MENU_SORT_DIRECTION, NULL, this, retVal);
#endif
	return retVal;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
int NdhsCPlugin::NdhsCPluginMenu::getMenuFirstActiveItem()
{
	int retVal = -1;

	if (m_pluginParent->m_pPluginGetFirstActiveItem && getMenuSession())
	{
		IFX_INT32 activeItem = 0;
		if (IFX_SUCCESS == m_pluginParent->m_pPluginGetFirstActiveItem(m_pluginParent->getPluginSession(), getMenuSession(), &activeItem))
		{
			retVal = activeItem;
		}
	}

	return retVal;
}


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<NdhsCPlugin> NdhsCPlugin::create(NdhsCMenuCon* con)
{
	LcTaOwner<NdhsCPlugin> ref;
	ref.set(new NdhsCPlugin(con));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPlugin::NdhsCPlugin(NdhsCMenuCon* con)
{
	// All members are NULL'ed because this is an LcCBase-derived class

	m_con = con;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::construct()
{
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPlugin::~NdhsCPlugin()
{
	// Call the plug-in shutdown method.
	shutdown();

	m_exclusivity.destroy();
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::loadPlugin()
{
	m_pPluginInitialize			= IFXI_Initialize;
	m_pPluginShutDown			= IFXI_ShutDown;
	m_pPluginValidateGUID		= IFXI_ValidateGUID;
	m_pPluginGetLinkTypeCount	= IFXI_GetLinkTypeCount;
	m_pPluginGetLinkTypeData	= IFXI_GetLinkTypeData;
	m_pPluginGetExclusivityCount = IFXI_GetExclusivityCount;
	m_pPluginOpenMenu			= IFXI_OpenMenu;
	m_pPluginCloseMenu			= IFXI_CloseMenu;
	m_pPluginGetItemCount		= IFXI_GetItemCount;
	m_pPluginGetFirstActiveItem	= IFXI_GetFirstActiveItem;
	m_pPluginSetActiveItem      = IFXI_SetActiveItem;
	m_pPluginGetFieldInfo		= IFXI_GetFieldInfo;
	m_pPluginGetFieldSize		= IFXI_GetFieldSize;
	m_pPluginGetFieldData		= IFXI_GetFieldData;
	m_pPluginGetFieldRaw		= IFXI_GetFieldRaw;
	m_pPluginSetFieldRaw		= IFXI_SetFieldRaw;
	m_pPluginGetFieldSizeFromRaw	= IFXI_GetFieldSizeFromRaw;
	m_pPluginExecuteLink		= IFXI_ExecuteLink;
	m_pPluginExclusivityStatusChange = IFXI_ExclusivityStatusChange;
#ifdef IFX_USE_PLUGIN_ELEMENTS
	m_pPluginCreateElement		= IFXI_CreateElement;
	m_pPluginDestroyElement		= IFXI_DestroyElement;
	m_pPluginActivateElement	= IFXI_ActivateElement;
	m_pPluginDeactivateElement	= IFXI_DeactivateElement;
	m_pPluginSetElementFocus	= IFXI_SetElementFocus;
	m_pPluginUnsetElementFocus	= IFXI_UnsetElementFocus;
	m_pProcessElementKeyDownEvent	= IFXI_ProcessElementKeyDownEvent;
	m_pProcessElementKeyUpEvent	= IFXI_ProcessElementKeyUpEvent;
	m_pGetElementCaretPosition	= IFXI_GetElementCaretPosition;
	m_pPluginPositionElement	= IFXI_PositionElement;
	m_pProcessElementStylusDownEvent = IFXI_ProcessElementStylusDownEvent;
	m_pProcessElementStylusUpEvent	= IFXI_ProcessElementStylusUpEvent;
	m_pProcessElementStylusDragEvent = IFXI_ProcessElementStylusDragEvent;
	m_pProcessElementStylusCancelEvent = IFXI_ProcessElementStylusCancelEvent;
	m_pPluginPaintElement		= IFXI_PaintElement;
	m_pPluginChangeElementMode	= IFXI_ChangeElementMode;
#endif //IFX_USE_PLUGIN_ELEMENTS

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::initialize()
{
	setPluginSession(NULL);

	// Load the plugin and entry points (platform-specific)
	if (!loadPlugin())
		return false;

	// Initialize the plugin, if it does not have an initialization method
	// return true.
	bool retVal = true;
	if (m_pPluginInitialize)
	{
		IFX_HIL			pluginSession;

#if defined(IFX_WIN_PLAYER)
		if(m_moduleIntegration == NULL)
		{
			m_moduleIntegration = NdhsCModuleIntegration::create();
			m_moduleIntegration->parseApplicationInfoFromXml();
		}

#endif	//defined(IFX_WIN_PLAYER)


		retVal = (IFX_SUCCESS == m_pPluginInitialize((IFX_HMENU)this, &pluginSession));
		if (retVal)
			setPluginSession(pluginSession);

		if (retVal)
		{
			IFX_INT32 moduleCount = getExclusivityCount();
			if (moduleCount > 0)
			{
				LcTaOwner<NdhsCExclusivity> newExclusivity = NdhsCExclusivity::create(this, m_con->getSpace(), (int)moduleCount);
				m_exclusivity = newExclusivity;
			}
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	This will call the plug-in shut down function.
	It should be called the derived plugin destructor.
*/
void NdhsCPlugin::shutdown()
{
	// Shutdown the plugin
	if (m_pPluginShutDown)
	{
		m_pPluginShutDown(m_pluginSession);
		m_pPluginShutDown = NULL;
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::validateGUID(const LcTmString& guid)
{
	bool retVal = false;

	if (m_pPluginValidateGUID)
	{
		IFX_INT32 valid = 0;
		if (m_pPluginValidateGUID(getPluginSession(), guid.bufWChar(), &valid) == IFX_SUCCESS)
		{
			retVal = (valid == 0) ? false : true;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCPlugin::getLinkTypeCount()
{
	if (m_pPluginGetLinkTypeCount)
	{
		// Extract link count.
		IFX_INT32 linkTypeCount;
		if (IFX_SUCCESS == m_pPluginGetLinkTypeCount(getPluginSession(), &linkTypeCount))
			return (int)linkTypeCount;
		else
			return NDHS_COUNT_FAILED;
	}
	return NDHS_COUNT_NOT_IMPLEMENTED;
}

/*-------------------------------------------------------------------------*//**
*/
ENdhsLinkType NdhsCPlugin::getLinkTypeData(int linkTypeIndex, LcTmString& linkTypePrefix)
{
	ENdhsLinkType retVal = ENdhsLinkTypeUnknown;
	if (m_pPluginGetLinkTypeData)
	{
		// Extract the link type data.
		IFX_WCHAR linkTypeName[IFX_MAX_LINK_TYPE_NAME_LENGTH + 1];
		IFX_LINK_MODE linkType = IFX_MODE_MENU;
		if (IFX_SUCCESS == m_pPluginGetLinkTypeData(getPluginSession(), (IFX_INT32)linkTypeIndex, &linkTypeName[0], &linkType))
		{
			linkTypePrefix.fromBufWChar(linkTypeName, IFX_MAX_LINK_TYPE_NAME_LENGTH + 1);
			switch (linkType)
			{
				case IFX_MODE_MENU:				retVal = ENdhsLinkTypeMenuPlugin;					break;
				case IFX_MODE_FUNCTION_SYNC:	retVal = ENdhsLinkTypeSyncLinkPlugin;				break;
				case IFX_MODE_FUNCTION_ASYNC:	retVal = ENdhsLinkTypeAsyncLinkPlugin;				break;
#ifdef IFX_USE_PLUGIN_ELEMENTS
				case IFX_MODE_ELEMENT:			retVal = ENdhsLinkTypeCreatePluginElement;			break;
				case IFX_MODE_EVENT_HANDLER:	retVal = ENdhsLinkTypeCreateEventHandler;			break;
#endif
				default:																			break;
			}
		}
	}
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCPlugin::getExclusivityCount()
{
	if (m_pPluginGetExclusivityCount)
	{
		// Extract link count.
		IFX_INT32 exclusivityCount;
		if (IFX_SUCCESS == m_pPluginGetExclusivityCount(getPluginSession(), &exclusivityCount))
			return (int)exclusivityCount;
		else
			return NDHS_COUNT_FAILED;
	}
	return NDHS_COUNT_NOT_IMPLEMENTED;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::getElementData(	int					menuItemIndex,
									const LcTmString&	elementClass,
									IFX_FIELD_TYPE		dataType,
									NdhsCElement*		element,
									NdhsCPluginMenu*	menu,
									LcTmString&			returnValue)
{
	bool retVal = false;

	if (IFX_FIELD == dataType)
	{
		NdhsCField* field = getField(elementClass, menu, menuItemIndex, element);

		if (field)
		{
			returnValue = field->getFieldData(element);
			retVal = true;
		}
	}
	else
	{
		retVal = requestLiveElementData(menuItemIndex, elementClass, dataType, element, menu, returnValue, NULL);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::requestLiveElementData(
									int					menuItemIndex,
									const LcTmString&	elementClass,
									IFX_FIELD_TYPE		dataType,
									NdhsCElement*		element,
									NdhsCPluginMenu*	menu,
									LcTmString&			returnValue,
									IFXI_FIELD_SCOPE*   pScope)
{
	bool retVal = false;

	// If pScope is NULL, create a pointer to a temporary scope variable
	IFXI_FIELD_SCOPE tempScope;
	IFXI_FIELD_SCOPE* pOutputScope = pScope;

	if (!pOutputScope)
	{
		pOutputScope = &tempScope;
	}

	if (m_pPluginGetFieldSize && m_pPluginGetFieldData)
	{
		IFX_HMENU hMenu = NULL;
		if (menu)
			hMenu = menu->getMenuSession();

		IFX_HELEMENT hElement = NULL;

#ifdef IFX_USE_PLUGIN_ELEMENTS
		if (element)
		{
			NdhsCPluginHElement* pluginElement = element->getPluginElement();
			if (pluginElement)
				hElement = pluginElement->getPluginElement();
		}
#endif

		// First get the size of the element string.
		IFX_INT32 stringSize = -1;

		if (IFX_SUCCESS == m_pPluginGetFieldSize(m_pluginSession,
													dataType,
													hElement,
													hMenu,
													(IFX_INT32)menuItemIndex,
													elementClass.bufWChar(),
													pOutputScope,
													&stringSize))
		{
			if (stringSize <= 0)
			{
				returnValue = "";
				retVal = true;
			}
			else
			{
				// Get the actual element data string.
				int stringLength = stringSize / sizeof(IFX_WCHAR);
				LcTaAlloc<IFX_WCHAR> retElementData(stringLength + 1);
				
				if (IFX_SUCCESS == m_pPluginGetFieldData(m_pluginSession,
															dataType,
															hElement,
															hMenu,
															(IFX_INT32)menuItemIndex,
															elementClass.bufWChar(),
															retElementData))
				{
					returnValue.fromBufWChar(retElementData, lc_wcslen(retElementData));
					retVal = true;
				}
			}
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPlugin::NdhsCPluginMenu> NdhsCPlugin::openMenu(const LcTmString& menuLink)
{
	LcTaOwner<NdhsCPlugin::NdhsCPluginMenu> menuPlugin;

	if (m_pPluginOpenMenu)
	{
		// Open the menu and return the menu handle if it opens correctly.
		IFX_HMENU retVal = NULL;
		if (IFX_SUCCESS == m_pPluginOpenMenu(	getPluginSession(),
												&retVal,
												menuLink.bufWChar()))
		{
			menuPlugin = NdhsCPluginMenu::create(this,
												 retVal,
												 menuLink);
		}
	}

	return menuPlugin;
}

#ifdef IFX_USE_PLUGIN_ELEMENTS
/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPlugin::NdhsCPluginHElement>NdhsCPlugin::createElement(	NdhsCPluginMenu*	menu,
																		NdhsCPageManager*	pageManager,
																		int					item,
																		const LcTmString&	link,
																		IFX_ELEMENT_PROPERTY *pProperty)
{
	LcTaOwner<NdhsCPlugin::NdhsCPluginHElement> elementPlugin;

	if (m_pPluginCreateElement)
	{
		// Open the element and return the element handle if it opens correctly.
		IFX_HMENU hMenu = NULL;
		if (menu)
			hMenu = menu->getMenuSession();

		IFX_HELEMENT hElement = NULL;
		if (IFX_SUCCESS == m_pPluginCreateElement(	getPluginSession(),
													&hElement,
													hMenu,
													(IFX_INT32)item,
													link.bufWChar(),
													pProperty))
		{
			elementPlugin = NdhsCPluginHElement::create(this, pageManager, hElement, pProperty);
		}
	}
	return elementPlugin;
}
#endif


/*-------------------------------------------------------------------------*//**
	Launch the specified link.
*/
bool NdhsCPlugin::launchLink(const LcTmString& link)
{
	bool retVal = false;
	if (m_pPluginExecuteLink)
	{
		if (IFX_SUCCESS == m_pPluginExecuteLink(getPluginSession(),
												 link.bufWChar()))
		{
			retVal = true;
		}
	}
	return retVal;
}

#ifdef IFX_USE_PLUGIN_ELEMENTS
/*-------------------------------------------------------------------------*//**
	Create
*/
LcTaOwner<NdhsCPlugin::NdhsCPluginHElement>
NdhsCPlugin::NdhsCPluginHElement::create(NdhsCPlugin*	pluginParent,
										 NdhsCPageManager* pageManager,
										IFX_HELEMENT	pluginElement,
										IFX_ELEMENT_PROPERTY *pProperty)
{
	LcTaOwner<NdhsCPluginHElement> ref;
	ref.set(new NdhsCPluginHElement());
	ref->construct(pluginParent, pageManager, pluginElement, pProperty);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Construct
*/
void NdhsCPlugin::NdhsCPluginHElement::construct(NdhsCPlugin*	pluginParent,
												 NdhsCPageManager* pageManager,
												IFX_HELEMENT	pluginElement,
												IFX_ELEMENT_PROPERTY *pProperty)
{
	m_pluginParent	= pluginParent;
	m_pageManager	= pageManager;
	m_pluginElement	= pluginElement;
	
	// pProperty can be NULL
	if (pProperty)
		m_cachedProperties = *pProperty;
		
	m_realizeCount = 0;
	m_activateCount = 0;
	m_space = 0;
	m_paintedThisFrame = false;
	m_resourcesHeld = false;
	m_currentMode = IFX_MODE_ELEMENT_INVALID;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPlugin::NdhsCPluginHElement::~NdhsCPluginHElement()
{
	destroyView();

	// Now clean up module-side resources
	if (m_pluginParent->m_pPluginDestroyElement
		&& m_pluginParent->getPluginSession())
	{
		m_pluginParent->m_pPluginDestroyElement(m_pluginParent->getPluginSession(),
												m_pluginElement);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginHElement::realize(LcCSpace* pSpace)
{
	m_realizeCount++;

	// Only realize once
	if (!pSpace || m_space)
		return;

	// Cache the space pointer
	m_space = pSpace;

	// switch mode now we have a space pointer - we prefer direct mode in non-OGL platforms for speed,
	// and texture id mode in OGL platforms for the same reason
#if !defined(LC_OGL_DIRECT)
	if ((m_cachedProperties.mode & IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT) == IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT)
	{
		switchMode(IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT);
	}
	else
#else
	if ((m_cachedProperties.mode & IFX_MODE_ELEMENT_OPENGL_TEXTURE) == IFX_MODE_ELEMENT_OPENGL_TEXTURE)
	{
		switchMode(IFX_MODE_ELEMENT_OPENGL_TEXTURE);
	}
	else 
#endif
	if ((m_cachedProperties.mode & IFX_MODE_ELEMENT_BUFFERED) == IFX_MODE_ELEMENT_BUFFERED)
	{
		switchMode(IFX_MODE_ELEMENT_BUFFERED);
	}
	else if ((m_cachedProperties.mode & IFX_MODE_ELEMENT_BUFFERED_NORMAL) == IFX_MODE_ELEMENT_BUFFERED_NORMAL)
	{
		switchMode(IFX_MODE_ELEMENT_BUFFERED_NORMAL);
		NDHS_TRACE(ENdhsTraceLevelInfo, ENdhsTraceModuleGeneral, "Plugin elements: IFX_MODE_ELEMENT_BUFFERED_NORMAL is deprecated - please use IFX_MODE_ELEMENT_BUFFERED instead");
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginHElement::retire()
{
	m_realizeCount--;

	if (m_realizeCount == 0)
	{
		// Clean up view and set plugin element into a safe state
		if (m_view)
			destroyView();

		m_space = NULL;
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::activateElement()
{
	// Only return false on a catastrophe
	bool retVal = true;

	// Only visible (i.e. non-event handler) elements can be activated
	if (m_currentMode == IFX_MODE_ELEMENT_INVALID)
		return false;

	if ((m_activateCount == 0) && m_pluginParent->m_pPluginActivateElement)
	{
		retVal = (m_pluginParent->m_pPluginActivateElement(	m_pluginParent->getPluginSession(),
															m_pluginElement) == IFX_SUCCESS);
	}

	m_activateCount++;

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::deactivateElement()
{
	if (m_activateCount == 0)
		return true;

	// Only visible (i.e. non-event handler) elements can be deactivated
	if (m_currentMode == IFX_MODE_ELEMENT_INVALID)
		return false;

	// Only return false on a catastrophe
	bool retVal = true;
	m_activateCount--;

	if ((m_activateCount == 0) && m_pluginParent->m_pPluginDeactivateElement)
	{
		retVal = (m_pluginParent->m_pPluginDeactivateElement(	m_pluginParent->getPluginSession(),
																m_pluginElement) == IFX_SUCCESS);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	When we suspend, we need to release resources when possible
*/
void NdhsCPlugin::NdhsCPluginHElement::releaseResources()
{
	if (m_view && m_resourcesHeld)
	{
		LcTmArray<IObserver*>::iterator it = m_observers.begin();

		for (; it < m_observers.end(); it++)
		{
			m_view->detachWidget((*it)->getWidget());
		}

		m_view->releaseResources();
		m_resourcesHeld = false;
	}
}

/*-------------------------------------------------------------------------*//**
	On resume - reload the resources
*/
void NdhsCPlugin::NdhsCPluginHElement::reloadResources()
{
	if (m_view && (m_resourcesHeld == false))
	{
		m_view->loadResources();
		m_resourcesHeld = true;

		LcTmArray<IObserver*>::iterator it = m_observers.begin();

		for (; it < m_observers.end(); it++)
		{
			m_view->attachWidget((*it)->getWidget());
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::setElementFocus(bool enableFocus)
{
	bool retVal = false;

	if (enableFocus)
	{
		if (m_pluginParent->m_pPluginSetElementFocus)
			retVal = (IFX_SUCCESS == m_pluginParent->m_pPluginSetElementFocus(	m_pluginParent->getPluginSession(),
														m_pluginElement));
	}
	else
	{
		if (m_pluginParent->m_pPluginUnsetElementFocus)
			retVal = (IFX_SUCCESS == m_pluginParent->m_pPluginUnsetElementFocus(	m_pluginParent->getPluginSession(),
														m_pluginElement));
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::processElementKeyDownEvent(int key)
{
	bool retVal = false;
	if (m_pluginParent->m_pProcessElementKeyDownEvent)
	{
		IFX_INT32 consumed = 0;
		if (m_pluginParent->m_pProcessElementKeyDownEvent(	m_pluginParent->getPluginSession(),
														m_pluginElement,
														(IFX_INT32)key,
														&consumed) == IFX_SUCCESS)
		{
			retVal = (consumed == 0) ? false : true;
		}
	}
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::processElementKeyUpEvent(int key)
{
	bool retVal = false;
	if (m_pluginParent->m_pProcessElementKeyUpEvent)
	{
		if (m_pluginParent->m_pProcessElementKeyUpEvent(m_pluginParent->getPluginSession(),
														m_pluginElement,
														(IFX_INT32)key) == IFX_SUCCESS)
		{
			retVal = true;
		}
	}
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCPlugin::NdhsCPluginHElement::getElementCaretPosition()
{
	IFX_INT32 pos = -1;
	if (m_pluginParent->m_pGetElementCaretPosition)
	{

		if (m_pluginParent->m_pGetElementCaretPosition(	m_pluginParent->getPluginSession(),
														m_pluginElement,
														&pos) != IFX_SUCCESS)
			pos = -1;
	}
	return (int)pos;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::positionElement(int x, int y, int w, int h)
{
	bool retVal = false;

	// Only visible (i.e. non-event handler) elements can be positioned
	if (m_currentMode == IFX_MODE_ELEMENT_INVALID)
		return false;

	if (m_pluginParent->m_pPluginPositionElement)
	{
		IFX_POSITION pos;

		pos.left 	= x;
		pos.top 	= y;
		pos.width 	= w;
		pos.height 	= h;

		retVal = (m_pluginParent->m_pPluginPositionElement(	m_pluginParent->getPluginSession(),
															m_pluginElement,
															&pos) == IFX_SUCCESS);
	}
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginHElement::refreshBufferedElement()
{
	if (m_view)
	{
		m_view->refreshBuffer();

		LcTmArray<IObserver*>::iterator it = m_observers.begin();
		for (; it != m_observers.end(); it++)
			(*it)->contentsUpdated();
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::resizeBufferedElement(int w, int h)
{
	bool retVal = false;

	if (m_view && m_view->setBufferSize(w, h))
	{
		m_cachedProperties.requiredBufferWidth = w;
		m_cachedProperties.requiredBufferHeight = h;
		retVal = true;
	}

	// Notify observers
	LcTmArray<IObserver*>::iterator it = m_observers.begin();
	for (; it != m_observers.end(); it++)
		(*it)->contentsUpdated();

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::paintElement()
{
	bool retVal = false;

	// Only visible (i.e. non-event handler) elements can be painted
	if (m_currentMode == IFX_MODE_ELEMENT_INVALID)
		return false;

	if (m_pluginParent->m_pPluginPaintElement)
	{
		retVal = (m_pluginParent->m_pPluginPaintElement(m_pluginParent->getPluginSession(),
															m_pluginElement) == IFX_SUCCESS);		
	}
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::processElementStylusDownEvent(int x, int y)
{
	bool retVal = false;
	if (m_pluginParent->m_pProcessElementStylusDownEvent)
	{
		IFX_INT32 consumed = 0;
		if (m_pluginParent->m_pProcessElementStylusDownEvent(	m_pluginParent->getPluginSession(),
																m_pluginElement,
																x,
																y,
																&consumed) == IFX_SUCCESS)
		{
			retVal = (consumed == 0) ? false : true;
		}
	}
	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::processElementStylusUpEvent(int x, int y)
{
	bool consumed = false;

	if (m_pluginParent->m_pProcessElementStylusUpEvent)
	{
		if(m_pluginParent->m_pProcessElementStylusUpEvent(	m_pluginParent->getPluginSession(),
																m_pluginElement,
																x,
																y) == IFX_SUCCESS)
		{
			consumed = true;
		}
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::processElementStylusDragEvent(int x, int y)
{
	bool consumed = false;

	if (m_pluginParent->m_pProcessElementStylusDragEvent)
	{
		if (m_pluginParent->m_pProcessElementStylusDragEvent(	m_pluginParent->getPluginSession(),
																m_pluginElement,
																x,
																y) == IFX_SUCCESS)
		{
			consumed = true;
		}
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginHElement::processElementStylusCancelEvent()
{
	if (m_pluginParent->m_pProcessElementStylusCancelEvent)
	{
		m_pluginParent->m_pProcessElementStylusCancelEvent(	m_pluginParent->getPluginSession(),
																m_pluginElement);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginHElement::switchMode(IFX_ELEMENT_MODE newMode)
{
	LcTaOwner<NdhsCPluginElementView> newPluginView;

	// Do not allow switching mode when ref is more than 1;
	if (getRefCount() > 1 && m_currentMode != IFX_MODE_ELEMENT_INVALID)
		return;

	if (m_currentMode == newMode)
		return;

	// We must insist on a space pointer
	if (!m_space)
		return;

	// Check that requested mode is supported
	if ((m_cachedProperties.mode & newMode) != newMode)
		return;

	// OK - clean up any existing mode
	if (m_view)
		destroyView();

	switch(newMode)
	{
#if !defined(LC_OGL_DIRECT)
		case IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT:
		{
			LcTaOwner<NdhsCPluginElementDirectView>tmpPluginView
								= NdhsCPluginElementDirectView::create(this);
			newPluginView = tmpPluginView;
			break;
		}
#else
		case IFX_MODE_ELEMENT_OPENGL_TEXTURE:
		{
			LcTaOwner<NdhsCPluginElementOGLTextureView>tmpPluginView
							= NdhsCPluginElementOGLTextureView::create(this);

			newPluginView = tmpPluginView;
			break;
		}
#endif

		case IFX_MODE_ELEMENT_BUFFERED:
		case IFX_MODE_ELEMENT_BUFFERED_NORMAL:
		{
			LcTaOwner<NdhsCPluginElementBufferedView>tmpPluginView
								= NdhsCPluginElementBufferedView::create(this, newMode);

			newPluginView = tmpPluginView;
			break;
		}

		default:
			break;
	}

	if (newPluginView)
	{
		bool viewOK = false;
		
		// Set the PMA value.
		newPluginView->setPreMultipliedAlpha(m_cachedProperties.hasPreMultipliedAlpha == IFX_TRUE);

		// Set up rendering buffer
		if (newPluginView->setBufferFormat(m_cachedProperties.requiredBufferFormat)
			&& newPluginView->setBufferSize(m_cachedProperties.requiredBufferWidth, m_cachedProperties.requiredBufferHeight) )
		{
			viewOK = true;
			m_resourcesHeld = true;

			// Set the translucency value
			newPluginView->setTranslucency(m_cachedProperties.translucency == IFX_TRANSLUCENCY_ELEMENT_NONOPAQUE);

			// Update placement as necessary - first widget in the observer list only, however
			// This is for direct mode plugins, and the 'oldest' observer is the only one supported
			newPluginView->onWidgetPlacementChanged((*(m_observers.begin()))->getWidget());
		}

		if (viewOK && m_pluginParent->m_pPluginChangeElementMode)
		{
			if (m_pluginParent->m_pPluginChangeElementMode(m_pluginParent->getPluginSession(),
															m_pluginElement,
															newMode,
															newPluginView->getRenderContext()) == IFX_SUCCESS)
			{
				// Now let the views set the appropriate image on the widgets
#if !defined(LC_OGL_DIRECT)
				if (newMode == IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT)
				{
					newPluginView->attachWidget((*(m_observers.begin()))->getWidget());
				}
				else
#endif
				{
					LcTmArray<IObserver*>::iterator it = m_observers.begin();

					for (; it < m_observers.end(); it++)
					{
						newPluginView->attachWidget((*it)->getWidget());
						(*it)->switchView(newPluginView.ptr());
					}
				}
			
				m_view = newPluginView;
				m_currentMode = newMode;
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Clean up view - don't want any dangling pointers from
	widgets (which shouldn't happen anyway, of course!)
*/
void NdhsCPlugin::NdhsCPluginHElement::destroyView()
{
	if (m_view)
	{
		LcTmArray<IObserver*>::iterator it = m_observers.begin();

		for (; it < m_observers.end(); it++)
			m_view->detachWidget((*it)->getWidget());

		m_view.destroy();

		m_currentMode = IFX_MODE_ELEMENT_INVALID;
	}
}

/*-------------------------------------------------------------------------*//**
	To support efficient plugin rendering in internal rendering mode, we let
	plugin elements switch between direct and buffered mode when animating.
	This method facilitates the switch - the NdhsCPluginElement uses this
	at the start and end of an animation, and we can decide whether to 
	change mode or not.
*/
void NdhsCPlugin::NdhsCPluginHElement::modeChangeOnAnimation(bool atAnimationStart)
{
#ifndef LC_OGL_DIRECT
	if (atAnimationStart)
	{
		if (getMode() == IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT)
		{
			// Change to buffered mode if possible
			switchMode(IFX_MODE_ELEMENT_BUFFERED);
		}
	}
	else
	{
		if (getMode() == IFX_MODE_ELEMENT_BUFFERED)
		{
			// If placement of first observer in list is rotated, scaled
			//  or semi-transparent, leave as buffered mode
			LcTPlacement placement = (*(m_observers.begin()))->getWidget()->getPlacement();
			if (placement.scale.equals(LcTVector(1.0,1.0,1.0))
				&& placement.orientation.isZero()
				&& (placement.opacity == 1.0))
				switchMode(IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT);
		}
	}
#endif
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginHElement::addObserver(IObserver* obs)
{
	// Check existing observer list
	LcTmArray<IObserver*>::iterator it = m_observers.begin();
	for (; it != m_observers.end() && *it != obs; it++);

	// Add to list if found
	if (it == m_observers.end())
	{
		m_observers.push_back(obs);

		// Let view know if necessary
		if (m_view)
			m_view->attachWidget(obs->getWidget());
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginHElement::removeObserver(IObserver* obs)
{
	// check existing observer list
	LcTmArray<IObserver*>::iterator it = m_observers.begin();
	for (; it != m_observers.end() && *it != obs; it++);

	// remove from list if found
	if (it != m_observers.end())
	{
		if (m_view)
			m_view->detachWidget(obs->getWidget());

		m_observers.erase(it);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::makeFullScreen(bool bStartFullScreen)
{
	bool retVal = false;

	// Only visible (i.e. non-event handler) elements can be full screen
	if (m_currentMode == IFX_MODE_ELEMENT_INVALID)
		return false;

	// We give priority to the first widget in the observer list
	if (m_observers.empty() == false)
		retVal = m_observers.front()->makeFullScreen(bStartFullScreen);

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::onWidgetPlacementChanged(LcwCPlugin* pWidget)
{
#ifndef LC_OGL_DIRECT
	// Only listen to the oldest observer in direct mode
	if (getMode() == IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT)
	{
		if (pWidget != m_observers.front()->getWidget())
			return true;
	}
#endif

	return m_view ? m_view->onWidgetPlacementChanged(pWidget) : true;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginHElement::prepareForPaint(LcwCPlugin* pWidget)
{
#ifndef LC_OGL_DIRECT
	// Only listen to the oldest observer in direct mode
	if (getMode() == IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT)
	{
		if (pWidget != m_observers.front()->getWidget())
			return;
	}
#endif
	if (m_view)
		m_view->prepareForPaint(pWidget);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginHElement::checkPaintElement()
{
	bool retVal = true;

	// If there's no view, something bad must have happened
	if (!m_view)
		return false;

	// Only want to call paintElement once per render cycle
	if (m_paintedThisFrame == false)
	{
		retVal = m_view->paintElement();
		m_paintedThisFrame = true;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginHElement::frameEnd()
{
	// Reset paint call
	m_paintedThisFrame = false;

	// Notify module that render cycle is complete
}

#endif //IFX_USE_PLUGIN_ELEMENTS

/*-------------------------------------------------------------------------*//**
	Callback routine.

	NOTE:	This function cannot use LcTaOwner objects EXCEPT within switch
			blocks that are documented as only being available within the
			same thread.
*/
IFX_RETURN_STATUS NdhsCPlugin::pluginCallback(	IFX_HUI					hIfx,
												IFX_CALLBACK_CODE		nCode,
												IFX_HELEMENT			hElement,
												IFX_HMENU				hMenu,
												int						item,
												const void*				pInput,
												void*					pOutput)
{

#ifndef IFX_USE_PLUGIN_ELEMENTS
	LC_UNUSED(hElement)
#endif

	// Error if no valid handle supplied
	if (hIfx == NULL)
		return IFX_ERROR;

	// If engine is not ready to handle a call back back off!
	if (((NdhsCPlugin*)hIfx)->getCon()->callbacksEnabled() == false)
		return IFX_ERROR_ENGINE_NOT_READY;

	// Return an error if an asynchronous function is in progress.
	if (((NdhsCPlugin*)hIfx)->getCon()->isAsyncLinkBlocking())
		return IFX_ERROR_ASYNC_BLOCKING;

	IFX_RETURN_STATUS retVal = IFX_ERROR;

	switch (nCode)
	{
		// Synchronous
		// Same Thread
		case IFX_QUERY_ACTIVE_ITEM:
		{
			if (pOutput != NULL)
			{
				int itemIndex = ((NdhsCPlugin*)hIfx)->getCon()->getPageManager()->getHMenuActiveItemIndex(hMenu);
				if (-1 != itemIndex)
				{
					// Set data to index.
					*(int*)pOutput = itemIndex;
					retVal = IFX_SUCCESS;
				}
			}
		}
		break;

		// Asynchronous
		// Any Thread
		case IFX_REFRESH_MENU:
		{
			((NdhsCPlugin*)hIfx)->getCon()->getPageManager()->refreshPage(hMenu, false);
			retVal = IFX_SUCCESS;
		}
		break;

		// Asynchronous
		// Any Thread
		case IFX_REFRESH_FIELD:
		{
			((NdhsCPlugin*)hIfx)->getCon()->getPageManager()->refreshField(hMenu, item, (const IFX_WCHAR*)pInput);
			retVal = IFX_SUCCESS;
		}
		break;

		// Synchronous
		// Same Thread
		case IFX_RESOURCE_SEARCH:
		{
			if (pOutput != NULL)
			{
				if (((NdhsCPlugin*)hIfx)->getCon()->getPageManager()->getFullFilePath(hMenu, (const IFX_WCHAR*)pInput, (IFX_WCHAR*)pOutput))
				{
					retVal = IFX_SUCCESS;
				}
			}
		}
		break;

#ifdef IFX_USE_PLUGIN_ELEMENTS
		// Synchronous
		// Same Thread
		case IFX_FULL_SCREEN_START:
		case IFX_FULL_SCREEN_STOP:
		{
			// Tell the page manager to start or stop full screen mode
			if ( ((NdhsCPlugin*)hIfx)->getCon()->getPageManager()->makeFullScreen(hElement, nCode == IFX_FULL_SCREEN_START) == true)
				retVal = IFX_SUCCESS;
			else
				retVal = IFX_ERROR;
		}
		break;

		// Asynchronous
		// Any Thread
		case IFX_REFRESH_BUFFERED_ELEMENT:
		{
			// Ask for the element to be refreshed
			((NdhsCPlugin*)hIfx)->getCon()->getPageManager()->refreshBufferedElement(hElement);

			// Always report success
			retVal = IFX_SUCCESS;
		}
		break;

		// Synchronous
		// Same Thread
		case IFX_RESIZE_BUFFERED_ELEMENT_BUFFER:
		{
			IFX_BUFFER_SIZE* bufferSize = (IFX_BUFFER_SIZE*)pInput;

			// Request buffer resize
			if ( ((NdhsCPlugin*)hIfx)->getCon()->getPageManager()->resizeBufferedElement(hElement, bufferSize) == true)
				retVal = IFX_SUCCESS;
			else
				retVal = IFX_ERROR;
		}
		break;
#endif

		case IFX_COMMAND_LINK:
		{
			if (pInput)
			{
				((NdhsCPlugin*)hIfx)->getCon()->getPageManager()->queueLinkEvent((const IFX_WCHAR*)pInput);
				retVal = IFX_SUCCESS;
			}
		}
		break;

		case IFX_TRIGGER_KEY:
		{
			((NdhsCPlugin*)hIfx)->getCon()->getPageManager()->queueKeyDownEvent((int)pInput);
			retVal = IFX_SUCCESS;
		}
		break;

		// Not yet enabled
		case IFX_CLOSE_MENU:
		{
			retVal = IFX_ERROR;
		}
		break;

		case IFX_SET_ACTVE_ITEM:
		{
			((NdhsCPlugin*)hIfx)->getCon()->getPageManager()->queueSetActiveItemEvent(hMenu, item);
			retVal = IFX_SUCCESS;
		}
		break;

		default:
			break;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS NdhsCPlugin::requestExclusivity(
		IFX_HUI				hIfx,
		IFX_HEXCLUSIVITY 	hExclusivity,
		IFX_UINT32			priority,
		IFX_UINT32			timeout)
{
	IFX_RETURN_STATUS retVal = IFX_ERROR;

	if(((NdhsCPlugin*)hIfx)->getExclusivityPtr() != NULL)
		retVal = ((NdhsCPlugin*)hIfx)->getExclusivityPtr()->requestExclusivity(hExclusivity, priority, timeout);

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS NdhsCPlugin::releaseExclusivity(
		IFX_HUI				hIfx,
		IFX_HEXCLUSIVITY 	hExclusivity)
{
	IFX_RETURN_STATUS retVal = IFX_ERROR;

	if(((NdhsCPlugin*)hIfx)->getExclusivityPtr() != NULL)
		retVal = ((NdhsCPlugin*)hIfx)->getExclusivityPtr()->releaseExclusivity(hExclusivity);

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS NdhsCPlugin::exclusivityPermitted(
		IFX_HUI				hIfx,
		IFX_HEXCLUSIVITY 	hExclusivity,
		IFX_INT32*			pResult)
{
	IFX_RETURN_STATUS retVal = IFX_SUCCESS;

	if(((NdhsCPlugin*)hIfx)->getExclusivityPtr() != NULL)
	{
		retVal = ((NdhsCPlugin*)hIfx)->getExclusivityPtr()->exclusivityPermitted(hExclusivity, pResult);
	}
	else
	{
		*pResult = 1;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS NdhsCPlugin::exclusivityStatusChange(
		IFX_HEXCLUSIVITY 		hExclusivity,
		IFX_EXCLUSIVITY_STATUS 	status)
{
	IFX_RETURN_STATUS retVal = IFX_ERROR;

	if (m_pPluginExclusivityStatusChange)
	{
		retVal = m_pPluginExclusivityStatusChange(getPluginSession(), hExclusivity, status);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCPlugin::getField(		const LcTmString& fieldName,
										NdhsCPlugin::NdhsCPluginMenu* menu,
										int menuItemIndex,
										NdhsCElement* element)
{
	NdhsCField* retVal = NULL;

	if (menu)
	{
		retVal = menu->findField(fieldName, menuItemIndex);

		if (!retVal)
		{
			IFXI_FIELD_MODE mode;
			IFXI_FIELD_SCOPE scope;
			IFXI_FIELDDATA_TYPE variant;

			if (getFieldInfo(fieldName, menu, menuItemIndex, mode, scope, variant))
			{
				if (IFXI_FIELD_SCOPE_GLOBAL == scope)
				{
					retVal = findField(fieldName);

					if (!retVal)
					{
						retVal = createField(fieldName, element, mode, variant);
					}
				}
				else
				{
					retVal = menu->createField(fieldName, menuItemIndex, element, mode, scope, variant);
				}
			}
		}
	}
	else
	{
		retVal = findField(fieldName);

		if (!retVal)
		{
			IFXI_FIELD_MODE mode;
			IFXI_FIELD_SCOPE scope;
			IFXI_FIELDDATA_TYPE variant;

			if (getFieldInfo(fieldName, menu, menuItemIndex, mode, scope, variant))
			{
				if (IFXI_FIELD_SCOPE_GLOBAL == scope)
				{
					retVal = createField(fieldName, element, mode, variant);
				}
			}
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::getFieldInfo(const LcTmString& fieldName, NdhsCPlugin::NdhsCPluginMenu* menu, int menuItemIndex,
		IFXI_FIELD_MODE& mode, IFXI_FIELD_SCOPE& scope, IFXI_FIELDDATA_TYPE& variant)
{
	bool retVal = false;

	if (m_pPluginGetFieldInfo)
	{
		IFX_HMENU hMenu = NULL;

		if (menu)
		{
			hMenu = menu->getMenuSession();
		}

		retVal = (IFX_SUCCESS ==
			m_pPluginGetFieldInfo(m_pluginSession, hMenu, menuItemIndex, fieldName.bufWChar(), &mode, &scope, &variant));
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::getRawElementData(
			int					menuItemIndex,
			const LcTmString&	elementClass,
			NdhsCElement*		element,
			NdhsCPluginMenu*	menu,
			IFXI_VARIANT_TYPE&  returnValue,
			IFXI_FIELD_SCOPE*   pScope)
{
	bool retVal = false;

	// If pScope is NULL, create a pointer to a temporary scope variable
	IFXI_FIELD_SCOPE tempScope;
	IFXI_FIELD_SCOPE* pOutputScope = pScope;

	if (!pOutputScope)
	{
		pOutputScope = &tempScope;
	}

	if (m_pPluginGetFieldRaw)
	{
		IFX_HMENU hMenu = NULL;
		IFX_HELEMENT hElement = NULL;

		if (menu)
		{
			hMenu = menu->getMenuSession();
		}

#ifdef IFX_USE_PLUGIN_ELEMENTS
		if (element)
		{
			NdhsCPluginHElement* pluginElement = element->getPluginElement();
			if (pluginElement)
				hElement = pluginElement->getPluginElement();
		}
#endif

		retVal = (IFX_SUCCESS ==
			m_pPluginGetFieldRaw(m_pluginSession, IFX_FIELD, hElement, hMenu, menuItemIndex, elementClass.bufWChar(), pOutputScope, &returnValue));
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::translateRawElementData(
			const LcTmString&	elementClass,
			NdhsCPluginMenu*	menu,
			IFXI_VARIANT_TYPE	dataValue,
			LcTmString&			returnValue)
{
	bool retVal = false;

	if (m_pPluginGetFieldSizeFromRaw && m_pPluginGetFieldData)
	{
		IFX_HMENU hMenu = NULL;
		if (menu)
			hMenu = menu->getMenuSession();

		IFX_HELEMENT hElement = NULL;

			// First get the size of the element string.
		IFX_INT32 stringSize = -1;

		if (IFX_SUCCESS == m_pPluginGetFieldSizeFromRaw(m_pluginSession,
													hMenu,
													(IFX_INT32)-1,
													elementClass.bufWChar(),
													dataValue,
													&stringSize))
		{
			if (stringSize <= 0)
			{
				returnValue = "";
				retVal = true;
			}
			else
			{
				// Get the actual element data string.
				int stringLength = stringSize / sizeof(IFX_WCHAR);
				LcTaAlloc<IFX_WCHAR> retElementData(stringLength + 1);

				if (IFX_SUCCESS == m_pPluginGetFieldData(m_pluginSession,
															IFX_FIELD,
															hElement,
															hMenu,
															(IFX_INT32)-1,
															elementClass.bufWChar(),
															retElementData))
				{
					returnValue.fromBufWChar(retElementData, lc_wcslen(retElementData));
					retVal = true;
				}
			}
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::notifyFieldValueRequest(
			int					menuItemIndex,
			const LcTmString&	elementClass,
			NdhsCPluginMenu*	menu,
			IFXI_VARIANT_TYPE	newValue,
			bool				finalValue)
{
	bool retVal = false;

	if (m_pPluginSetFieldRaw)
	{
		IFX_HMENU hMenu = NULL;

		if (menu)
		{
			hMenu = menu->getMenuSession();
		}

		retVal = (IFX_SUCCESS ==
			m_pPluginSetFieldRaw(m_pluginSession, IFX_FIELD, NULL, hMenu, menuItemIndex, elementClass.bufWChar(), newValue, finalValue ? 1 : 0));
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCPlugin::createField(	const LcTmString& fieldName,
										NdhsCElement* element,
										IFXI_FIELD_MODE& mode,
										IFXI_FIELDDATA_TYPE& variant)
{
	LcTaString lowercaseFieldName = fieldName.toLower();
	LcTaOwner<NdhsCField> field = NdhsCField::create(m_con, lowercaseFieldName, true, NULL, -1, element, mode, IFXI_FIELD_SCOPE_GLOBAL, variant);
	NdhsCField* retVal = field.ptr();

	m_globalFieldCache[lowercaseFieldName] = field;

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCPlugin::findField(	const LcTmString& fieldName)
{
	LcTaString lowercaseFieldName = fieldName.toLower();
	NdhsCField* retVal = NULL;

	TmMNameCache::iterator it = m_globalFieldCache.find(lowercaseFieldName);

	if (it != m_globalFieldCache.end())
	{
		retVal = it->second.ptr();
	}

	return retVal;
}


/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCPlugin::NdhsCPluginMenu::createField(	const LcTmString& fieldName,
										int menuItemIndex,
										NdhsCElement* element,
										IFXI_FIELD_MODE& mode,
										IFXI_FIELD_SCOPE& scope,
										IFXI_FIELDDATA_TYPE& variant)
{
	NdhsCField* retVal = NULL;

	// Check the parameters are valid
	if ((IFXI_FIELD_SCOPE_ITEM == scope && menuItemIndex >= 0) || IFXI_FIELD_SCOPE_MENU == scope)
	{
		LcTaString lowercaseFieldName = fieldName.toLower();
		LcTaOwner<NdhsCField> field = NdhsCField::create(m_pluginParent->m_con, lowercaseFieldName, true, this, menuItemIndex, element, mode, scope, variant, "");
		retVal = field.ptr();

		m_menuFieldCache[menuItemIndex][lowercaseFieldName] = field;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCPlugin::NdhsCPluginMenu::findField(	const LcTmString& fieldName,
										int menuItemIndex)
{
	LcTaString lowercaseFieldName = fieldName.toLower();
	NdhsCField* retVal = NULL;

	TmMIndexCache::iterator idxIt = m_menuFieldCache.find(menuItemIndex);

	if (idxIt != m_menuFieldCache.end())
	{
		TmMNameCache::iterator nameIt = idxIt->second.find(lowercaseFieldName);

		if (nameIt!= idxIt->second.end())
		{
			retVal = nameIt->second.ptr();
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::updateGlobalFields(LcTTime timestamp)
{
	bool reschedule = false;

	TmMNameCache::iterator it = m_globalFieldCache.begin();

	while (it != m_globalFieldCache.end())
	{
		bool finalUpdate;
		if (it->second->atRest() == false)
			reschedule |= it->second->updateValue(timestamp, finalUpdate);
		it++;
	}

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::refreshGlobalField(const LcTmString& fieldName)
{
	if (fieldName.isEmpty())
	{
		// Refresh all
		TmMNameCache::iterator it = m_globalFieldCache.begin();
		while (it != m_globalFieldCache.end())
		{
			it->second->refresh();
			it++;
		}
	}
	else
	{
		// Find field
		LcTaString lowercaseFieldName = fieldName.toLower();
		TmMNameCache::iterator it = m_globalFieldCache.find(lowercaseFieldName);
		if (it != m_globalFieldCache.end())
		{
			it->second->refresh();
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::setGlobalFieldsDirty(bool requiresLaundry)
{
	TmMNameCache::iterator it = m_globalFieldCache.begin();
	while (it != m_globalFieldCache.end())
	{
		it->second->setDirtyByPlugin(requiresLaundry);
		it++;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::stopGlobalFields()
{
	TmMNameCache::iterator it = m_globalFieldCache.begin();
	while (it != m_globalFieldCache.end())
	{
		it->second->setVelocity(0);
		it++;
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPlugin::NdhsCPluginMenu::updateFields(LcTTime timestamp)
{
	bool reschedule = false;

	TmMIndexCache::iterator idxIt = m_menuFieldCache.begin();

	while (idxIt != m_menuFieldCache.end())
	{
		TmMNameCache::iterator nameIt = idxIt->second.begin();

		while (nameIt != idxIt->second.end())
		{
			bool finalUpdate;
			reschedule |= nameIt->second->updateValue(timestamp, finalUpdate);
			nameIt++;
		}

		idxIt++;
	}

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginMenu::refreshField(const LcTmString& fieldName,
					int menuItemIndex)
{
	if (menuItemIndex == -1)
	{
		// Refresh all items
		TmMIndexCache::iterator idxIt = m_menuFieldCache.begin();

		while (idxIt != m_menuFieldCache.end())
		{

			if (fieldName.isEmpty())
			{
				// Refresh all field names
				TmMNameCache::iterator nameIt = idxIt->second.begin();

				while (nameIt != idxIt->second.end())
				{
					nameIt->second->refresh();
					nameIt++;
				}
			}
			else
			{
				// Find given name
				TmMNameCache::iterator nameIt = idxIt->second.find(fieldName.toLower());

				if (nameIt != idxIt->second.end())
				{
					nameIt->second->refresh();
				}
			}

			idxIt++;
		}
	}
	else
	{
		// Refresh given item
		TmMIndexCache::iterator idxIt = m_menuFieldCache.find(menuItemIndex);

		if (idxIt != m_menuFieldCache.end())
		{
			if (fieldName.isEmpty())
			{
				// Refresh all field names
				TmMNameCache::iterator nameIt = idxIt->second.begin();

				while (nameIt != idxIt->second.end())
				{
					nameIt->second->refresh();
					nameIt++;
				}
			}
			else
			{
				// Find given name
				TmMNameCache::iterator nameIt = idxIt->second.find(fieldName.toLower());

				if (nameIt != idxIt->second.end())
				{
					nameIt->second->refresh();
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginMenu::setPluginFieldsDirty(bool requiresLaundry)
{
	TmMIndexCache::iterator idxIt = m_menuFieldCache.begin();

	while (idxIt != m_menuFieldCache.end())
	{
		TmMNameCache::iterator nameIt = idxIt->second.begin();

		while (nameIt != idxIt->second.end())
		{
			nameIt->second->setDirtyByPlugin(requiresLaundry);
			nameIt++;
		}

		idxIt++;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginMenu::stopFields()
{
	TmMIndexCache::iterator idxIt = m_menuFieldCache.begin();

	while (idxIt != m_menuFieldCache.end())
	{
		TmMNameCache::iterator nameIt = idxIt->second.begin();

		while (nameIt != idxIt->second.end())
		{
			nameIt->second->setVelocity(0.0);
			nameIt++;
		}

		idxIt++;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginMenu::clearFieldCache()
{
	m_menuFieldCache.clear();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPlugin::NdhsCPluginMenu::activeItemUpdated(int itemIndex)
{
	if (m_pluginParent && m_pluginParent->m_pPluginSetActiveItem)
		m_pluginParent->m_pPluginSetActiveItem(m_pluginParent->getPluginSession(), getMenuSession(), itemIndex);
}
