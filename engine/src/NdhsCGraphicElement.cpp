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
	Static factory method
*/
LcTaOwner<NdhsCGraphicElement>
NdhsCGraphicElement::create(ENdhsObjectType						use,
							const LcTmString&					className,
							NdhsCMenu*							menu,
							NdhsCMenuItem*						menuItem,
							NdhsCTemplate::CGraphicElement*		pGE,
							NdhsCPageManager*					pPageManager,
							NdhsCElementGroup*					pPage,
							int									stackLevel,
							NdhsCManifest*						parentPaletteMan,
							NdhsCElementGroup*					parentGroup)
{
	LcTaOwner<NdhsCGraphicElement> ref;
	ref.set(new NdhsCGraphicElement());
	ref->construct(use, className, menu, menuItem, pGE, pPageManager, pPage, stackLevel, parentPaletteMan, parentGroup);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Constructor
*/
NdhsCGraphicElement::NdhsCGraphicElement()
{
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	m_customEffectDirty = false;
#endif
}

/*-------------------------------------------------------------------------*//**
	Construction method - creates the widget and animator
*/
void NdhsCGraphicElement::construct(ENdhsObjectType						use,
									const LcTmString&					className,
									NdhsCMenu*							menu,
									NdhsCMenuItem*						menuItem,
									NdhsCTemplate::CGraphicElement*		pGE,
									NdhsCPageManager*					pPageManager,
									NdhsCElementGroup*					pPage,
									int									stackLevel,
									NdhsCManifest*						parentPaletteMan,
									NdhsCElementGroup*					parentGroup)
{
	// Initialize settings structure valid for a graphical widget
	LcTaOwner<NdhsCMenuWidgetFactory::CSettings> tempSettings = NdhsCMenuWidgetFactory::CSettings::create();
	m_settings = tempSettings;
	m_settings->forceExtent			= pGE->extentHint;
	m_settings->frameCount			= pGE->frameCount;
	m_settings->sketchyMode			= pGE->sketchyMode;
	m_settings->antiAlias			= pGE->antiAlias;
	m_settings->parentPaletteMan	= parentPaletteMan;
	m_settings->meshGridX			= pGE->meshGridX;
	m_settings->meshGridY			= pGE->meshGridY;
	m_GE							= pGE;

	// Set tappable status
	setTappable(pGE->tappable);
	// Store tap tolerance
	setTapTolerance(pGE->tapTolerance);

#ifdef LC_USE_LIGHTS
	setLightModel(pGE->lightModel);
	if(parentGroup != NULL)
		applyLightModel(parentGroup->getLightModel());
#endif

#ifdef IFX_USE_PLUGIN_ELEMENTS
	LcTaString eventHandler;
	LcTaString eventHandlerWithTokens;

	if (pGE->eventHandler)
	{
		NdhsCExpression* expr = pGE->eventHandler->getContextFreeExpression();
		if (expr)
		{
			expr->evaluate(pPage, -1, menuItem);

			if (expr->isError())
			{
				expr->errorDiagnostics("Graphic element event handler", true);
			}
			else
			{
				eventHandlerWithTokens = expr->getValueString();
			}
		}
	}

	pPageManager->getTokenStack()->replaceTokens(eventHandlerWithTokens, eventHandler, this, menu, menuItem, pPage, stackLevel);

	m_settings->eventHandler = eventHandler;
	m_settings->eventHandlerLink = eventHandler.subString(0, eventHandler.find(":"));
#endif

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)

	m_bUseCustomEffect = false;

	if (m_GE && m_GE->useCustomEffect)
	{
		m_useCustomEffect = m_GE->useCustomEffect->createExpression(pPage, -1, menuItem);

		if(m_useCustomEffect)
		{
			m_useCustomEffect->setObserver ((NdhsIExpressionObserver *)this);

			// Evaluate just after creation
			m_useCustomEffect->evaluate(pPage, -1, menuItem);

			if (m_useCustomEffect->isError() || !m_useCustomEffect->isBool())
			{
				m_useCustomEffect->errorDiagnostics("Graphic element useCustomEffect condition", true);
			}
			else
			{
				m_bUseCustomEffect = m_useCustomEffect->getValueBool();
			}
		}
	}
#endif

	// Let the base class Initialize
	NdhsCElement::construct(use,
							ENdhsElementTypeGraphic,
							pGE->elementParent,
							className,
							pGE->resourceData.ptr(),
							pGE->enableFocusExprSkel.ptr(),
							menu,
							menuItem,
							pPageManager,
							pPage,
							stackLevel,
							pGE->m_drawLayerIndex,
							pGE->layoutTeleport,
							pGE->scrollTeleport,
							pGE->isDetail);


#ifdef IFX_USE_PLUGIN_ELEMENTS
	// Get a pointer to the underlying plugin element
	m_pPlugin = pPageManager->getCon()->getTRLinkTypePlugin(m_settings->eventHandlerLink);

	// Now we can create the plugin element instance
	if (m_pPlugin != NULL)
	{
		if (pPageManager->getCon()->getTRLinkType(m_settings->eventHandlerLink) == ENdhsLinkTypeCreateEventHandler)
		{
			NdhsCPlugin::NdhsCPluginMenu* menuPlugin = NULL;
			if (menu)
				menuPlugin = menu->getMenuPlugin();

			int item = -1;
			if (menuItem)
				item = menuItem->getIndex();

			m_hPluginElement = pPageManager->getPluginElement(menuPlugin,
																item,
																m_settings->eventHandler,
																NULL);
			
			if (m_hPluginElement == NULL)
				m_pPlugin = NULL;
		}
		else
		{
			LcTaString warningStr = "Graphic element has an invalid event handler link. Element name = " + className + ", link = " + m_settings->eventHandlerLink;
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleGeneral, warningStr);
		}
	}
#endif

}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
NdhsCGraphicElement::~NdhsCGraphicElement()
{
#ifdef IFX_USE_PLUGIN_ELEMENTS
	getPageManager()->releasePluginElement(m_hPluginElement);
#endif
}

/*-------------------------------------------------------------------------*//**
	Load the widget
*/
void NdhsCGraphicElement::reloadElement()
{
	LcTPlacement pl;

	if(getWidget() != NULL)
		destroyWidget();

	clearFieldTokens();

	LcTaString elementData;

	elementData = getElementData();

	// Identify exact type of widget needed from elementData
	ENdhsWidgetType iconType = getPageManager()->getCon()->getWidgetType(elementData);

	// Now attempt to create widget
	LcTaOwner<LcCWidget> newWidget = NdhsCMenuWidgetFactory::createWidget(	m_settings.ptr(),
																			iconType,
																			getPackagePath(),
																			elementData,
																			getPageManager(),
																			this,
																			getMenu(),
																			getLocalMenuItem(),
																			getPage(),
																			getStackLevel());

	if (newWidget)
	{
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	int imageWidth = newWidget->getImageWidth();
	int imageHeight = newWidget->getImageHeight();
#endif
		setWidget(newWidget);
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
		getWidget()->setImageSize(imageWidth, imageHeight);
#endif

#ifdef LC_USE_LIGHTS
		getWidget()->setLightModelSimple(m_lightModel == ENdhsLightModelSimple);
#endif
		// Set the graphic specific default color.
		LcTPlacement colourPlacement;
		colourPlacement.color = LcTColor::WHITE;
		getWidget()->setPlacement(colourPlacement, LcTPlacement::EColor);

		// Set element's tappable property
		getWidget()->setTappable(getTappable());
		getWidget()->setTapTolerance(getTapTolerance());

		getWidget()->setVisible(widgetVisible());

		getWidget()->setDrawLayerIndex(getDrawLayerIndex());

		if(m_GE==NULL)
		{
			m_GE=(NdhsCTemplate::CGraphicElement*) (getPage()->getTemplate()->getElementClass(getElementClassName(),getElementUse()));
		}


		if(m_GE)
		{
			// Update widget material properties
			getWidget()->setMaterialSpecularColor (m_GE->materialSpecularColor);
			getWidget()->setMaterialEmissiveColor (m_GE->materialEmissiveColor);
			getWidget()->setMaterialSpecularShininess (m_GE->materialSpecularShininess);

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

			// Set flag to use Custom Effect
			getWidget()->setUseCustomEffect (m_bUseCustomEffect);

			// Associate effect with this widget
			getWidget()->setVisualEffectName(m_GE->visualEffect);

			// If element does not specify "high" setting, then look for that setting in its parent
			if (m_GE->openGLRenderQuality.compare("normal") == 0)
			{
				if (getElementGroup())
				{
					// Parent has specified "high" setting, so update element's setting from there
					if (getElementGroup()->getOpenGLRenderQualitySetting().compare("high") == 0)
						m_GE->openGLRenderQuality = "high";
				}
			}

			// Set OpenGL rendering quality with which this widget will be rendered
			getWidget()->setOpenGLRenderQualitySetting (m_GE->openGLRenderQuality);

			// Populate widget uniform map with non-interpolatable config/effect uniforms
			getWidget()->populateEffectUniformsFromEffectFile (getPageManager()->getSpace());

			LcTmMap<LcTmString, LcOglCSLType *>::iterator it;

			// Fill in widget effect uniform map.
			for(it = m_GE->effectUniMap.begin(); it != m_GE->effectUniMap.end(); it++)
			{
				getWidget()->addEffectUniform((*it).first, (*it).second, getPageManager()->getSpace());
			}

			// Configure viusal effect related effect uniforms.
			getWidget()->configEffectUniforms(getPageManager()->getSpace(), getPageManager()->getCon()->getAppPath());
#endif
		}

		updateWidgetPlacement();
	}
}

#ifdef IFX_USE_PLUGIN_ELEMENTS
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCGraphicElement::setElementFocus(bool enableFocus)
{
	bool retVal = true;

	if(m_pPlugin != NULL && m_hPluginElement != NULL)
	{
		retVal = m_hPluginElement->setElementFocus(enableFocus);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCGraphicElement::processKeyDownEvent(int key)
{
	bool consumed = false;

	if(m_pPlugin != NULL && m_hPluginElement != NULL)
	{
		consumed = m_hPluginElement->processElementKeyDownEvent(key);
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCGraphicElement::processKeyUpEvent(int key)
{
	bool consumed = false;

	if(m_pPlugin != NULL && m_hPluginElement != NULL)
	{
		consumed = m_hPluginElement->processElementKeyUpEvent(key);
	}

	return consumed;
}
#endif /* #ifdef IFX_USE_PLUGIN_ELEMENTS */

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
/*-------------------------------------------------------------------------*//**
*/
void NdhsCGraphicElement::expressionDirty(NdhsCExpression* expr)
{
	NdhsCLaundry* laundry = getLaundry();
	m_customEffectDirty |= (expr == m_useCustomEffect.ptr());

	if (expr == m_useCustomEffect.ptr() && laundry)
	{
		laundry->addItem(this);
	}

	NdhsCElement::expressionDirty (expr);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCGraphicElement::expressionDestroyed(NdhsCExpression* expr)
{
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCGraphicElement::cleanLaundryItem(LcTTime timestamp)
{
	if (m_customEffectDirty)
	{
		m_useCustomEffect->evaluate(getPage(), -1, getLocalMenuItem());

		if (m_useCustomEffect->isError() || !m_useCustomEffect->isBool())
		{
			m_useCustomEffect->errorDiagnostics("Graphic element useCustomEffect condition", true);
		}
		else
		{
			m_bUseCustomEffect = m_useCustomEffect->getValueBool();
		}
		// Pass the value to widget
		if (getWidget())
			getWidget()->setUseCustomEffect (m_bUseCustomEffect);

		m_customEffectDirty = false;
	}

	return NdhsCElement::cleanLaundryItem(timestamp);
}
#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

#ifdef IFX_SERIALIZATION
NdhsCGraphicElement* NdhsCGraphicElement::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCGraphicElement *obj=new NdhsCGraphicElement();
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCGraphicElement::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCGraphicElement) - sizeof(NdhsCElement)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCElement::serialize(serializeMaster,true);
	ENdhsElementType dataType=ENdhsElementTypeGraphic;
	SERIALIZE(dataType,serializeMaster,cPtr);
	SERIALIZE(parentHandle,serializeMaster,cPtr);
	SERIALIZE_Owner(m_settings,serializeMaster,cPtr);
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	SERIALIZE_Owner(m_useCustomEffect,serializeMaster,cPtr);
	SERIALIZE(m_bUseCustomEffect,serializeMaster,cPtr);
	SERIALIZE(m_customEffectDirty,serializeMaster,cPtr);
#endif
#ifdef IFX_USE_PLUGIN_ELEMENTS
	SERIALIZE_Reserve(m_pPlugin,serializeMaster,cPtr);
	getPageManager()->releasePluginElement(m_hPluginElement);
#endif
	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCGraphicElement::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummy=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;
	DESERIALIZE(dummy,serializeMaster,cPtr);
	DESERIALIZE(parentHandle,serializeMaster,cPtr);
	NdhsCElement::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE_Owner(m_settings,serializeMaster,cPtr,NdhsCMenuWidgetFactory::CSettings)
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	DESERIALIZE_Owner(m_useCustomEffect,serializeMaster,cPtr,NdhsCExpression)
	DESERIALIZE(m_bUseCustomEffect,serializeMaster,cPtr);
	DESERIALIZE(m_customEffectDirty,serializeMaster,cPtr);
#endif
#ifdef IFX_USE_PLUGIN_ELEMENTS
	DESERIALIZE_Reserve(m_pPlugin,serializeMaster,cPtr,NdhsCPlugin);
	NdhsCMenu *menu=getMenu();
	NdhsCMenuItem *menuItem=getMenuItem();

	// Now we can create the plugin element instance
	if(m_pPlugin != NULL)
	{
		NdhsCPlugin::NdhsCPluginMenu* menuPlugin = NULL;
		if (menu)
			menuPlugin = menu->getMenuPlugin();

		int item = -1;
		if (menuItem)
			item = menuItem->getIndex();

		m_hPluginElement = getPageManager()->getPluginElement(menuPlugin,
																item,
																m_settings->eventHandler,
																NULL);
		if (m_hPluginElement == NULL)
			m_pPlugin = NULL;
	}
#endif /* IFX_USE_PLUGIN_ELEMENTS */
}
#endif /* IFX_SERIALIZATION */

