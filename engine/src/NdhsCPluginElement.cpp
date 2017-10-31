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


#ifdef IFX_USE_PLUGIN_ELEMENTS

#include "inflexionui/engine/inc/NdhsCPluginElement.h"

/*-------------------------------------------------------------------------*//**
	static factory method
*/
LcTaOwner<NdhsCPluginElement>
NdhsCPluginElement::create(	ENdhsObjectType						use,
							const LcTmString&					className,
							NdhsCMenu*							menu,
							NdhsCMenuItem*						menuItem,
							NdhsCTemplate::CPluginElement*		pPE,
							NdhsCPageManager*					pPageManager,
							NdhsCElementGroup*					pPage,
							int									stackLevel,
							NdhsCElementGroup*					parentGroup)
{
	LcTaOwner<NdhsCPluginElement> ref;
	ref.set(new NdhsCPluginElement());
	ref->construct(use, className, menu, menuItem, pPE, pPageManager, pPage, stackLevel, parentGroup);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	constructor
*/
NdhsCPluginElement::NdhsCPluginElement()
{
	m_widgetType = ENdhsWidgetTypeUnknown;
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	m_customEffectDirty = false;
#endif
}

/*-------------------------------------------------------------------------*//**
	construction method - creates the widget and animator
*/
void NdhsCPluginElement::construct(ENdhsObjectType						use,
								   const LcTmString&					className,
								   NdhsCMenu*							menu,
								   NdhsCMenuItem*						menuItem,
								   NdhsCTemplate::CPluginElement*		pPE,
								   NdhsCPageManager*					pPageManager,
								   NdhsCElementGroup*					pPage,
								   int									stackLevel,
								   NdhsCElementGroup*					parentGroup)
{
	LcTaOwner<NdhsCMenuWidgetFactory::CSettings> temp = NdhsCMenuWidgetFactory::CSettings::create();
	m_settings				= temp;
	m_settings->forceExtent	= pPE->extentHint;
	m_settings->antiAlias	= pPE->antiAlias;

	m_settings->meshGridX	= pPE->meshGridX;
	m_settings->meshGridY	= pPE->meshGridY;

	// Set tappable status
	setTappable(pPE->tappable);
	// Store tap tolerance
	setTapTolerance(pPE->tapTolerance);

	m_eventHandlerExprDirty = false;
	m_customEffectExprDirty = false;
	m_reloadRequired		= false;
	m_eventHandlerStr		= "";

	m_PE					= pPE;

#ifdef LC_USE_LIGHTS
	setLightModel(pPE->lightModel);
	if (parentGroup != NULL)
		applyLightModel(parentGroup->getLightModel());
#endif

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)

	m_bUseCustomEffect = false;

	if (pPE && pPE->useCustomEffect)
	{
		m_useCustomEffect = pPE->useCustomEffect->createExpression(pPage, -1, menuItem);

		if(m_useCustomEffect)
		{
			m_useCustomEffect->setObserver ((NdhsIExpressionObserver *)this);

			// Evaluate just after creation
			m_useCustomEffect->evaluate(pPage, -1, menuItem);

			if (m_useCustomEffect->isError() || !m_useCustomEffect->isBool())
			{
				m_useCustomEffect->errorDiagnostics("Plugin element useCustomEffect condition", true);
			}
			else
			{
				m_bUseCustomEffect = m_useCustomEffect->getValueBool();
			}
		}
	}
#endif

	//let the base class initialize
	NdhsCElement::construct(use,
							ENdhsElementTypePlugin,
							pPE->elementParent,
							className,
							NULL,
							pPE->enableFocusExprSkel.ptr(),
							menu,
							menuItem,
							pPageManager,
							pPage,
							stackLevel,
							pPE->m_drawLayerIndex,
							pPE->layoutTeleport,
							pPE->scrollTeleport,
							pPE->isDetail);

	clearFieldTokens();

	if (pPE->eventHandler)
	{
		m_eventHandler = pPE->eventHandler->createExpression(pPage, -1, menuItem);

		if (m_eventHandler)
		{
			m_eventHandler->setObserver(this);

			m_eventHandler->evaluate(getPage(), -1, getMenuItem());

			if (m_eventHandler->isError())
			{
				m_eventHandler->errorDiagnostics("Plugin element event handler", true);
			}
			else
			{
				m_eventHandlerStr = m_eventHandler->getValueString();
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
NdhsCPluginElement::~NdhsCPluginElement()
{
}

/*-------------------------------------------------------------------------*//**
	Reload the widget - in the plugin element case we do not want to tear down the
	widget (it would mean destroying the underlying plugin element instance).  We do,
	however, need a mechanism to forward on changes in field items to the plugin element
	to allow indirect communication between plugin elements.
*/
void NdhsCPluginElement::reloadElement()
{
	if(m_reloadRequired && getWidget() != NULL) {
		destroyWidget();
	}

	if (!getWidget())
	{
		LcTaString eventHandler;
		LcTaString eventHandlerWithTokens;

		if (m_eventHandler)
		{
			eventHandlerWithTokens = m_eventHandlerStr;
		}

		getPageManager()->getTokenStack()->replaceTokens(eventHandlerWithTokens, eventHandler, this, getMenu(), getMenuItem(), getPage(), getStackLevel());
		m_settings->eventHandler = eventHandler;
		LcTaString linkName = eventHandler.subString(0, eventHandler.find(":"));

		//identify the widget we need from the creation link type
		if(getPageManager()->getCon()->getTRLinkType(linkName) == ENdhsLinkTypeCreatePluginElement)
		m_widgetType = ENdhsWidgetTypePluginElement;

		// Attempt to create widget
		LcTaOwner<LcCWidget> newWidget = NdhsCMenuWidgetFactory::createWidget(	m_settings.ptr(),
																				m_widgetType,
																				getPackagePath(),
																				m_settings->eventHandler,
																				getPageManager(),
																				this,
																				getMenu(),
																				getMenuItem(),
																				getPage(),
																				getStackLevel());

		if(m_PE==NULL)
		{
			m_PE=(NdhsCTemplate::CPluginElement*) (getPage()->getTemplate()->getElementClass(getElementClassName(),getElementUse()));
		}

		if (newWidget)
		{
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
			int imageWidth = newWidget->getImageWidth();
			int imageHeight = newWidget->getImageHeight();
#endif
			// Attach widget to element
			setWidget(newWidget);

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
			getWidget()->setImageSize(imageWidth, imageHeight);
#endif

#ifdef LC_USE_LIGHTS
			getWidget()->setLightModelSimple(m_lightModel == ENdhsLightModelSimple);
#endif

			getWidget()->setTappable(getTappable());
			getWidget()->setTapTolerance(getTapTolerance());

			// Update widget material properties
			getWidget()->setMaterialSpecularColor (m_PE->materialSpecularColor);
			getWidget()->setMaterialEmissiveColor (m_PE->materialEmissiveColor);
			getWidget()->setMaterialSpecularShininess (m_PE->materialSpecularShininess);
			getWidget()->setDrawLayerIndex(getDrawLayerIndex());

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

			// Set flag to use Custom Effect
			getWidget()->setUseCustomEffect (m_bUseCustomEffect);

			// Associate effect with this widget
			getWidget()->setVisualEffectName(m_PE->visualEffect);

			// If element does not specify "high" setting, then look for that setting in its parent
			if (m_PE->openGLRenderQuality.compare("normal") == 0)
			{
				if (getElementGroup())
				{
					// Parent has specified "high" setting, so update element's setting from there
					if (getElementGroup()->getOpenGLRenderQualitySetting().compare("high") == 0)
						m_PE->openGLRenderQuality = "high";
				}
			}

			// Set OpenGL rendering quality with which this widget will be rendered
			getWidget()->setOpenGLRenderQualitySetting (m_PE->openGLRenderQuality);

			// Populate widget uniform map with non-interpolatable config/effect uniforms
			getWidget()->populateEffectUniformsFromEffectFile (getPageManager()->getSpace());

			LcTmMap<LcTmString, LcOglCSLType *>::iterator it;

			// Fill in widget effect uniform map.
			for(it = m_PE->effectUniMap.begin(); it != m_PE->effectUniMap.end(); it++)
			{
				getWidget()->addEffectUniform((*it).first, (*it).second, getPageManager()->getSpace());
			}

			// Configure viusal effect related effect uniforms.
			getWidget()->configEffectUniforms(getPageManager()->getSpace(), getPageManager()->getCon()->getAppPath());

#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

			if(m_reloadRequired)
			{
				updateWidgetPlacement();
			}
		}
	}
}

#define NDHS_PLUGIN_ELEMENT_ANIM_MASK (LcTPlacement::ELocation | LcTPlacement::EExtent | LcTPlacement::EScale \
								| LcTPlacement::EOrientation | LcTPlacement::EOpacity | LcTPlacement::EOffset)

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElement::updatePosition(ENdhsAnimationType animType,
										LcTScalar position,
										bool positionIncreasing,
										bool updateCache,
										bool forceNonSketchy,
										bool aggregateAnimating,
										bool finalFrame)
{
	// Do common actions
	NdhsCElement::updatePosition(animType, position, positionIncreasing, updateCache,
								forceNonSketchy, aggregateAnimating, finalFrame);

	if (getWidget())
	{
		// Check to see if we need to change plugin modes
		if (getWidget()->isVisible())
		{
#ifndef LC_OGL_DIRECT
			NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt
						= LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, getWidget());

			if (finalFrame)
			{
				// If element is on active page, check to see if we want to switch to a more efficient mode
				if ((getPage() == getPageManager()->getActivePage()) && pPluginWgt->getPluginElement())
					pPluginWgt->getPluginElement()->modeChangeOnAnimation(false);
			}
			else if (position == 0)
			{
				// Must be the start of a transition
				// If the animation affects anything other than frame, color or intensity
				// then we need to either switch to buffered mode or teleport, likewise
				// if we're no longer on the active page
				if ((getAnimateMask() & NDHS_PLUGIN_ELEMENT_ANIM_MASK)
						|| (getPage() != getPageManager()->getActivePage()))
				{
					if (pPluginWgt->getPluginElement())
					{
						pPluginWgt->getPluginElement()->modeChangeOnAnimation(true);

						if (pPluginWgt->getPluginElement()->getMode() == IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT)
						{
							// Still direct mode - in this case, hide the element and teleport
							teleportElement();
							getWidget()->setVisible(false);
						}
					}
				}
			}
#endif //LC_OGL_DIRECT
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPluginElement::setElementFocus(bool enableFocus)
{
	bool retVal = false;

	LcCWidget* pWgt = getWidget();
	NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt = LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, pWgt);
	if(pPluginWgt)
		retVal = pPluginWgt->setElementFocus(enableFocus);

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPluginElement::processKeyDownEvent(int key)
{
	bool consumed = false;

	NdhsCPlugin::NdhsCPluginHElement* pluginHElement = NULL;
	LcCWidget* pWgt = getWidget();
	NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt = LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, pWgt);
	if(pPluginWgt)
		pluginHElement = pPluginWgt->getPluginElement();

	if(pluginHElement)
	{
		consumed = pluginHElement->processElementKeyDownEvent(key);
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPluginElement::processKeyUpEvent(int key)
{
	bool consumed = false;

	NdhsCPlugin::NdhsCPluginHElement* pluginHElement = NULL;
	LcCWidget* pWgt = getWidget();
	NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt = LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, pWgt);
	if(pPluginWgt)
		pluginHElement = pPluginWgt->getPluginElement();

	if(pluginHElement)
	{
		consumed = pluginHElement->processElementKeyUpEvent(key);
	}

	return consumed;
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPluginElement::onMouseDown(const LcTPixelPoint& pt)
{
	bool consumed = false;

	NdhsCPlugin::NdhsCPluginHElement* pluginHElement = NULL;
	LcCWidget* pWgt = getWidget();
	NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt = LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, pWgt);
	if(pPluginWgt)
		pluginHElement = pPluginWgt->getPluginElement();

	if(pluginHElement && pWgt)
	{
		LcTVector loc;
		LcTVector centre = LcTVector::add(pWgt->getPlacement().location, pWgt->getPlacement().centerOffset);

		if (pWgt->getSpace())
		{
			// Get the local co-ordinates.
			bool planeHitTest = pWgt->getSpace()->mapCanvasToLocal(pt, pWgt, centre, loc);

			if (planeHitTest)
			{
				LcTScalar width		= pWgt->getExtent().x;
				LcTScalar height 	= pWgt->getExtent().y;

				// Negative is down for y, so reverse it here
				LcTScalar themeX	= (width * 0.5f) + loc.x;
				LcTScalar themeY 	= (height * 0.5f) - loc.y;

				// Convert into plugin coordinates
				IFX_ELEMENT_PROPERTY* elementProperties = pluginHElement->getCachedProperties();
				int pluginX = (int)themeX;
				int pluginY = (int)themeY;

				if (pluginHElement->getMode() != IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT)
				{
					// If we're not in direct mode, we should scale the theme location
					// into the buffer coordinates
					pluginX = (int)(themeX * elementProperties->requiredBufferWidth / width);
					pluginY = (int)(themeY * elementProperties->requiredBufferHeight / height);
				}

				consumed = pluginHElement->processElementStylusDownEvent(pluginX, pluginY);
			}
		}
	}

	NdhsCElement::onMouseDown(pt);

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPluginElement::onMouseMove(const LcTPixelPoint& pt)
{
	bool consumed = false;

	NdhsCPlugin::NdhsCPluginHElement* pluginHElement = NULL;
	LcCWidget* pWgt = getWidget();
	NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt = LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, pWgt);
	if(pPluginWgt)
		pluginHElement = pPluginWgt->getPluginElement();

	if(pluginHElement && pWgt)
	{
		LcTVector loc;
		LcTVector centre = LcTVector::add(pWgt->getPlacement().location, pWgt->getPlacement().centerOffset);

		if (pWgt->getSpace())
		{
			// Get the local co-ordinates.
			bool planeHitTest = pWgt->getSpace()->mapCanvasToLocal(pt, pWgt, centre, loc);

			if (planeHitTest)
			{
				LcTScalar width		= pWgt->getExtent().x;
				LcTScalar height 	= pWgt->getExtent().y;

				// Negative is down for y, so reverse it here
				LcTScalar themeX	= (width * 0.5f) + loc.x;
				LcTScalar themeY 	= (height * 0.5f) - loc.y;

				// Convert into plugin coordinates
				IFX_ELEMENT_PROPERTY* elementProperties = pluginHElement->getCachedProperties();
				int pluginX = (int)themeX;
				int pluginY = (int)themeY;

				if (pluginHElement->getMode() != IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT)
				{
					// If we're not in direct mode, we should scale the theme location
					// into the buffer coordinates
					pluginX = (int)(themeX * elementProperties->requiredBufferWidth / width);
					pluginY = (int)(themeY * elementProperties->requiredBufferHeight / height);
				}

				consumed = pluginHElement->processElementStylusDragEvent(pluginX, pluginY);
			}
		}
	}

	consumed = consumed || NdhsCElement::onMouseMove(pt);

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPluginElement::onMouseUp(const LcTPixelPoint& pt)
{
	bool consumed = false;

	NdhsCPlugin::NdhsCPluginHElement* pluginHElement = NULL;
	LcCWidget* pWgt = getWidget();
	NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt = LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, pWgt);
	if(pPluginWgt)
		pluginHElement = pPluginWgt->getPluginElement();

	if(pluginHElement && pWgt)
	{
		LcTVector loc;
		LcTVector centre = LcTVector::add(pWgt->getPlacement().location, pWgt->getPlacement().centerOffset);

		if (pWgt->getSpace())
		{
			// Get the local co-ordinates.
			bool planeHitTest = pWgt->getSpace()->mapCanvasToLocal(pt, pWgt, centre, loc);

			if (planeHitTest)
			{
				LcTScalar width		= pWgt->getExtent().x;
				LcTScalar height 	= pWgt->getExtent().y;

				// Negative is down for y, so reverse it here
				LcTScalar themeX	= (width * 0.5f) + loc.x;
				LcTScalar themeY 	= (height * 0.5f) - loc.y;

				// Convert into plugin coordinates
				IFX_ELEMENT_PROPERTY* elementProperties = pluginHElement->getCachedProperties();
				int pluginX = (int)themeX;
				int pluginY = (int)themeY;

				if (pluginHElement->getMode() != IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT)
				{
					// If we're not in direct mode, we should scale the theme location
					// into the buffer coordinates
					pluginX = (int)(themeX * elementProperties->requiredBufferWidth / width);
					pluginY = (int)(themeY * elementProperties->requiredBufferHeight / height);
				}

				consumed = pluginHElement->processElementStylusUpEvent(pluginX, pluginY);
			}
		}
	}

	NdhsCElement::onMouseUp(pt);

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElement::onMouseCancel()
{
	NdhsCPlugin::NdhsCPluginHElement* pluginHElement = NULL;
	LcCWidget* pWgt = getWidget();
	NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt = LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, pWgt);
	if(pPluginWgt)
		pluginHElement = pPluginWgt->getPluginElement();

	if(pluginHElement)
	{
		pluginHElement->processElementStylusCancelEvent();
	}
	NdhsCElement::onMouseCancel();
}

#endif // LC_USE_STYLUS

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElement::startStaticAnimations()
{
	NdhsCElement::startStaticAnimations();

#ifndef LC_OGL_DIRECT
	if (getWidget())
	{
		// Check to see if we need to change plugin modes
		if (getWidget()->isVisible())
		{
			NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt
				= LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, getWidget());

			// If the animation affects anything other than frame, color or intensity
			// then we need to either switch to buffered mode or teleport
			if ((getStaticAnimateMask() & NDHS_PLUGIN_ELEMENT_ANIM_MASK) && pPluginWgt->getPluginElement())
			{
				pPluginWgt->getPluginElement()->modeChangeOnAnimation(true);

				if (pPluginWgt->getPluginElement()->getMode() == IFX_MODE_ELEMENT_BUFFERED_AND_DIRECT)
				{
					// Still direct mode - in this case, hide the element for the duration
					m_makeVisibleOnStaticCompletion = true;
					getWidget()->setVisible(false);
				}
			}
		}
	}
#endif // LC_OGL_DIRECT
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElement::stopStaticAnimations()
{
	NdhsCElement::stopStaticAnimations();

	if (getWidget())
	{
		if (getWidget()->isVisible())
		{
#ifndef LC_OGL_DIRECT
			NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt
						= LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, getWidget());

			// If element isn't on active menu, leave as buffered
			if ((getPage() == getPageManager()->getActivePage()) && pPluginWgt->getPluginElement())
				pPluginWgt->getPluginElement()->modeChangeOnAnimation(false);
#endif // LC_OGL_DIRECT
		}
		else if(m_makeVisibleOnStaticCompletion)
		{
			getWidget()->setVisible(true);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPluginElement::forceDestroyOnSuspend()
{
	NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt
				= LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, getWidget());
	if (pPluginWgt && pPluginWgt->getPluginElement() 
		&& pPluginWgt->getPluginElement()->getMode()==IFX_MODE_ELEMENT_OPENGL_TEXTURE)
		return true;
	else
		return false;
}


/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElement::onSuspend()
{
	NdhsCMenuWidgetFactory::CPluginItem* pPluginWgt
				= LC_DYNAMIC_CAST(NdhsCMenuWidgetFactory::CPluginItem, getWidget());
	if(pPluginWgt)
	{
		pPluginWgt->deactivate();
	}
	NdhsCElement::onSuspend();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElement::onResume()
{
	NdhsCElement::onResume();
}


/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElement::expressionDirty(NdhsCExpression* expr)
{
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	if (expr == m_useCustomEffect.ptr())
	{
		getPageManager()->getCon()->getLaundry()->addItem(this);
		m_customEffectExprDirty = true;
	}
	else
#endif
	if(expr == m_eventHandler.ptr())
	{
		getPageManager()->getCon()->getLaundry()->addItem(this);
		m_eventHandlerExprDirty = true;
	}

	NdhsCElement::expressionDirty (expr);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPluginElement::expressionDestroyed(NdhsCExpression* expr)
{
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPluginElement::cleanLaundryItem(LcTTime timestamp)
{
	if(m_customEffectExprDirty)
	{
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
		m_useCustomEffect->evaluate(getPage(), -1, getLocalMenuItem());

		if (m_useCustomEffect->isError() || !m_useCustomEffect->isBool())
		{
			m_useCustomEffect->errorDiagnostics("Plugin element useCustomEffect condition", true);
		}
		else
		{
			m_bUseCustomEffect = m_useCustomEffect->getValueBool();
		}
		m_customEffectExprDirty = false;
#endif
	}

	if(m_eventHandlerExprDirty && !getUnloaded())
	{
		LcTaString eventHandlerStr = "";

		m_eventHandlerExprDirty = false;
		m_eventHandler->evaluate(getPage(), -1, getMenuItem());

		if (m_eventHandler->isError())
		{
			m_eventHandler->errorDiagnostics("Plugin element event handler", true);
		}
		else
		{
			eventHandlerStr = m_eventHandler->getValueString();
		}

		if(eventHandlerStr.compareNoCase(m_eventHandlerStr) != 0)
		{
			m_reloadRequired = true;
			m_eventHandlerStr = eventHandlerStr;
			reloadElement();

			if(m_isRealized)
			{
				realize();
			}
			m_reloadRequired = false;
		}
	}

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	// Pass the value to widget
	if (getWidget())
		getWidget()->setUseCustomEffect (m_bUseCustomEffect);
#endif

	return NdhsCElement::cleanLaundryItem(timestamp);
}

#ifdef IFX_SERIALIZATION
NdhsCPluginElement* NdhsCPluginElement::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCPluginElement *obj=new NdhsCPluginElement();
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCPluginElement::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCPluginElement) - sizeof(NdhsCElement)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCElement::serialize(serializeMaster,true);
	ENdhsElementType dataType=ENdhsElementTypePlugin;
	SERIALIZE(dataType,serializeMaster,cPtr)
	SERIALIZE(parentHandle,serializeMaster,cPtr)
	SERIALIZE_Owner(m_settings,serializeMaster,cPtr)
	SERIALIZE_Placement(m_widgetPlacement,serializeMaster,cPtr)
	SERIALIZE(m_widgetType,serializeMaster,cPtr)
	SERIALIZE(m_makeVisibleOnStaticCompletion,serializeMaster,cPtr)
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	SERIALIZE_Owner(m_useCustomEffect,serializeMaster,cPtr)
	SERIALIZE(m_bUseCustomEffect,serializeMaster,cPtr)
	SERIALIZE(m_customEffectDirty,serializeMaster,cPtr)
#endif
	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCPluginElement::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummy=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;
	DESERIALIZE(dummy,serializeMaster,cPtr);
	DESERIALIZE(parentHandle,serializeMaster,cPtr);
	NdhsCElement::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE_Owner(m_settings,serializeMaster,cPtr,NdhsCMenuWidgetFactory::CSettings)
	DESERIALIZE_Placement(m_widgetPlacement,serializeMaster,cPtr)
	DESERIALIZE(m_widgetType,serializeMaster,cPtr)
	DESERIALIZE(m_makeVisibleOnStaticCompletion,serializeMaster,cPtr)
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	DESERIALIZE_Owner(m_useCustomEffect,serializeMaster,cPtr,NdhsCExpression)
	DESERIALIZE(m_bUseCustomEffect,serializeMaster,cPtr)
	DESERIALIZE(m_customEffectDirty,serializeMaster,cPtr)
#endif
}
#endif /* IFX_SERIALIZATION */

#endif // IFX_USE_PLUGIN_ELEMENTS
