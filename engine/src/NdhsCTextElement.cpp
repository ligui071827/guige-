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
LcTaOwner<NdhsCTextElement>
NdhsCTextElement::create(	ENdhsObjectType						use,
							const LcTmString&					className,
							NdhsCMenu*							menu,
							NdhsCMenuItem*						menuItem,
							NdhsCPageTemplate::CTextElement*	pTE,
							NdhsCPageManager*					pPageManager,
							NdhsCElementGroup*					pPage,
							int									stackLevel,
							NdhsCManifest*						parentPaletteMan,
							NdhsCElementGroup*					parentGroup)
{
	LcTaOwner<NdhsCTextElement> ref;
	ref.set(new NdhsCTextElement());
	ref->construct(use, className, menu, menuItem, pTE, pPageManager, pPage, stackLevel, parentPaletteMan, parentGroup);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Constructor
*/
NdhsCTextElement::NdhsCTextElement()
{
	m_hasFocus = false;
	m_fontExpressionDirty = false;
	m_fontExpressionDirtyReload = false;
	m_marqueeFieldPending = false;
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	m_customEffectDirty = false;
#endif
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTextElement::startStaticAnimations()
{
	if (m_marqueeFieldPending
			&& m_marqueeField == NULL
			&& getWidget())
	{
		m_marqueeFieldPending = false;

		startMarqueeAnimation();
	}

	NdhsCElement::startStaticAnimations();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTextElement::stopMarqueeAnimations()
{
	if (m_marqueeField)
	{
		getPage()->releaseStaticAnimationField(m_marqueeField);
		m_marqueeField = NULL;
		((LcwCLabel*)getWidget())->resetDelta();
		getPageManager()->getSpace()->revalidate();
	}

	m_marqueeFieldPending = false;
	m_currentDelta = -1;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTextElement::stopStaticAnimations()
{
	if (m_marqueeField
			&& (m_verticalMarquee ||m_horizontalMarquee))
	{
		getPage()->releaseStaticAnimationField(m_marqueeField);
		m_marqueeField = NULL;
		m_marqueeFieldPending = true;
		m_currentDelta = 0;
		((LcwCLabel*)getWidget())->resetDelta();
		getPageManager()->getSpace()->revalidate();
	}

	NdhsCElement::stopStaticAnimations();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTextElement::fieldValueUpdated(NdhsCField* field)
{
	if (m_marqueeField && m_marqueeField == field)
	{
		if (getWidget())
		{
			int delta = (int) m_marqueeField->getRawFieldData(NULL);
			if (delta !=  m_currentDelta)
			{
				m_currentDelta = delta;
				((LcwCLabel*)getWidget())->setDelta(m_currentDelta);
				getPageManager()->getSpace()->revalidate();
			}
		}
	}
	else
	{
		NdhsCElement::fieldValueUpdated(field);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTextElement::startMarqueeAnimation()
{
	LcwCLabel *widget = (LcwCLabel*) getWidget();

	bool isHorizontal = widget->isHorizontalMarqueeEnabled();
	bool isVertical = widget->isVerticalMarqueeEnabled();

	bool textRTL = widget->isTextRTL();

	if(isHorizontal)
	{
		LcTScalar width =  widget->getTextWidth();
		startHorizontalMarqueeAnimation(width - widget->getExtent().x, textRTL);
	}
	else if (isVertical)
	{
		LcTScalar height =  widget->getTextHeight();
		startVerticalMarqueeAnimation(height - widget->getExtent().y);
	}
	else
	{
		stopMarqueeAnimations();
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTextElement::onWidgetEvent(LcTWidgetEvent* e)
{
	if (getWidget() == e->getWidget())
	{
		startMarqueeAnimation();
		return true;
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTextElement::startHorizontalMarqueeAnimation(LcTScalar width, bool textRTL)
{
	if (m_marqueeField)
	{
		getPage()->releaseStaticAnimationField(m_marqueeField);
		m_marqueeField = NULL;
	}

	m_currentDelta = -1;

	// Create marquee field only if required
	if (m_scrollVelocity > 0)
	{
		// Get the animation field reference
		m_marqueeField = getPage()->getStaticAnimationField(this);
	}

	if (m_marqueeField)
	{
		LcTScalar duration = 1000 + (width * 1000) / m_scrollVelocity;
		m_marqueeField->updateBounds( -width, 0, false, 0, 1);
		if (textRTL)
		{
			m_marqueeField->jumpValueTo((int)-width);
			m_marqueeField->setTargetValue(0, ENdhsFieldDirectionIncreasing, false, (int)duration, ENdhsScrollFieldModeNormal, ENdhsVelocityProfilePauseStartFinish);
		}
		else
		{
			m_marqueeField->setTargetValue(-width, ENdhsFieldDirectionDecreasing, false, (int)duration, ENdhsScrollFieldModeNormal, ENdhsVelocityProfilePauseStartFinish);
		}
		m_marqueeField->setLoop(-1);
		getPageManager()->startAnimation();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTextElement::startVerticalMarqueeAnimation(LcTScalar height)
{
	if (m_marqueeField)
	{
		getPage()->releaseStaticAnimationField(m_marqueeField);
		m_marqueeField = NULL;
	}

	m_currentDelta = -1;

	// Create marquee field only if required
	if (m_scrollVelocity > 0)
	{
		// Get the animation field reference
		m_marqueeField = getPage()->getStaticAnimationField(this);
	}

	if (m_marqueeField)
	{
		LcTScalar duration = (height * 1000) / m_scrollVelocity;
		m_marqueeField->updateBounds( -height, 0, false, 0, 1);
		m_marqueeField->setTargetValue(-height, ENdhsFieldDirectionDecreasing, false, (unsigned int)duration, ENdhsScrollFieldModeNormal, ENdhsVelocityProfilePauseStartFinish);

		m_marqueeField->setLoop(-1);
		getPageManager()->startAnimation();
	}
}

/*-------------------------------------------------------------------------*//**
	Construction method - creates the widget and animator
*/
void NdhsCTextElement::construct(	ENdhsObjectType						use,
									const LcTmString&					className,
									NdhsCMenu*							menu,
									NdhsCMenuItem*						menuItem,
									NdhsCPageTemplate::CTextElement*	pTE,
									NdhsCPageManager*					pPageManager,
									NdhsCElementGroup*					pPage,
									int									stackLevel,
									NdhsCManifest*						parentPaletteMan,
									NdhsCElementGroup*					parentGroup)
{
	LcTaOwner<NdhsCMenuWidgetFactory::CSettings> temp = NdhsCMenuWidgetFactory::CSettings::create();
	m_settings = temp;
	m_settings->fontAbbrevSuffix	= pTE->font->abbrevSuffix;
	m_settings->fontHeight			= pTE->font->height;
	m_settings->fontStyle			= pTE->font->style;
	m_settings->parentPaletteMan	= parentPaletteMan;
	m_settings->sketchyMode			= pTE->sketchyMode;
	m_settings->antiAlias			= pTE->antiAlias;
	m_settings->meshGridX			= pTE->meshGridX;
	m_settings->meshGridY			= pTE->meshGridY;

	m_settings->marqueeSpeed		= pTE->marqueeSpeed;
	m_TE							= pTE;
	// Set tappable status
	setTappable(pTE->tappable);
	// Store tap tolerance
	setTapTolerance(pTE->tapTolerance);

#ifdef LC_USE_LIGHTS
	setLightModel(pTE->lightModel);
	if (parentGroup != NULL)
		applyLightModel(parentGroup->getLightModel());
#endif
	LcTaString fontName;

	if (m_TE->font->face)
	{
		m_fontExpr = m_TE->font->face->createExpression(pPage, -1, menuItem);
		if (m_fontExpr)
		{
			m_fontExpr->setObserver(this);

			m_fontExpr->evaluate(pPage, -1, menuItem);

			if (m_fontExpr->isError())
			{
				m_fontExpr->errorDiagnostics("Text element font face", true);
			}
			else
			{
				if (m_fontExpr->getValueString().isEmpty() == false)
				{
					pPageManager->getTokenStack()->replaceTokens(m_fontExpr->getValueString(), fontName, this, menu, menuItem, pPage, stackLevel);
					m_settings->fontName = fontName;
				}
			}
		}
	}

	if (m_settings->fontName.isEmpty() && pPageManager)
		m_settings->fontName = pPageManager->getDefaultFontFace();

#ifdef IFX_USE_PLUGIN_ELEMENTS
	LcTaString eventHandler;
	LcTaString eventHandlerWithTokens;

	if (pTE->eventHandler)
	{
		NdhsCExpression* expr = pTE->eventHandler->getContextFreeExpression();
		if (expr)
		{
			expr->evaluate(pPage, -1, menuItem);
			eventHandlerWithTokens = expr->getValueString();
		}
	}

	pPageManager->getTokenStack()->replaceTokens(eventHandlerWithTokens, eventHandler, this, menu, menuItem, pPage, stackLevel);

	m_settings->eventHandler = eventHandler;
	m_settings->eventHandlerLink = eventHandler.subString(0, eventHandler.find(":"));
#endif

	if(pTE->font->hAlign.compareNoCase("LEFT") == 0)
		m_settings->fontHAlign = LcwCLabel::HALIGN_LEFT;
	else if(pTE->font->hAlign.compareNoCase("RIGHT") == 0)
		m_settings->fontHAlign = LcwCLabel::HALIGN_RIGHT;
	else
		m_settings->fontHAlign = LcwCLabel::HALIGN_CENTER;

	if(pTE->font->vAlign.compareNoCase("TOP") == 0)
		m_settings->fontVAlign = LcwCLabel::VALIGN_TOP;
	else if(pTE->font->vAlign.compareNoCase("BOTTOM") == 0)
		m_settings->fontVAlign = LcwCLabel::VALIGN_BOTTOM;
	else
		m_settings->fontVAlign = LcwCLabel::VALIGN_CENTER;

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)

	m_bUseCustomEffect = false;

	if (m_TE && m_TE->useCustomEffect)
	{
		m_useCustomEffect = m_TE->useCustomEffect->createExpression(pPage, -1, menuItem);

		if (m_useCustomEffect)
		{
			m_useCustomEffect->setObserver ((NdhsIExpressionObserver *)this);

			// Evaluate just after creation
			m_useCustomEffect->evaluate(pPage, -1, menuItem);

			if (m_useCustomEffect->isError() || !m_useCustomEffect->isBool())
			{
				m_useCustomEffect->errorDiagnostics("Text element useCustomEffect condition", true);
			}
			else
			{
				m_bUseCustomEffect = m_useCustomEffect->getValueBool();
			}
		}
	}
#endif

	if (m_TE && m_TE->verticalMarquee)
	{
		m_verticalMarqueeExpr = m_TE->verticalMarquee->createExpression(pPage, -1, menuItem);

		if (m_verticalMarqueeExpr)
		{
			m_verticalMarqueeExpr->setObserver ((NdhsIExpressionObserver *)this);

			// Evaluate just after creation
			m_verticalMarqueeExpr->evaluate(pPage, -1, menuItem);

			if (m_verticalMarqueeExpr->isError() || !m_verticalMarqueeExpr->isBool())
			{
				m_verticalMarqueeExpr->errorDiagnostics("Text element verticalMarquee condition", true);
			}
			else
			{
				m_verticalMarquee = m_verticalMarqueeExpr->getValueBool();
			}
		}
	}

	if (m_TE && m_TE->horizontalMarquee)
	{
		m_horizontalMarqueeExpr = m_TE->horizontalMarquee->createExpression(pPage, -1, menuItem);

		if (m_horizontalMarqueeExpr)
		{
			m_horizontalMarqueeExpr->setObserver ((NdhsIExpressionObserver *)this);

			// Evaluate just after creation
			m_horizontalMarqueeExpr->evaluate(pPage, -1, menuItem);

			if (m_horizontalMarqueeExpr->isError() || !m_horizontalMarqueeExpr->isBool())
			{
				m_horizontalMarqueeExpr->errorDiagnostics("Text element horizontalMarquee condition", true);
			}
			else
			{
				m_horizontalMarquee = m_horizontalMarqueeExpr->getValueBool();
			}
		}
	}

	if (m_verticalMarquee && m_horizontalMarquee)
	{
		m_verticalMarquee = false;
		m_horizontalMarquee = false;

		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleGeneral, LcTaString("Text element vertical and horizontal Marquee both cannot be enabled"));
	}

	// Let the base class initialize
	NdhsCElement::construct(use,
							ENdhsElementTypeText,
							pTE->elementParent,
							className,
							pTE->resourceData.ptr(),
							pTE->enableFocusExprSkel.ptr(),
							menu,
							menuItem,
							pPageManager,
							pPage,
							stackLevel,
							pTE->m_drawLayerIndex,
							pTE->layoutTeleport,
							pTE->scrollTeleport,
							pTE->isDetail);

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
			LcTaString warningStr = "Text element has an invalid event handler link. Element name = " + className + ", link = " + m_settings->eventHandlerLink;
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleGeneral, warningStr);
		}
	}
#endif
}

#ifdef IFX_SERIALIZATION
NdhsCTextElement* NdhsCTextElement::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCTextElement *obj=new NdhsCTextElement();
	if(handle==-1)
		return obj;
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}
SerializeHandle	NdhsCTextElement::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCTextElement) - sizeof(NdhsCElement)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCElement::serialize(serializeMaster,true);
	ENdhsElementType dataType=ENdhsElementTypeText;
	SERIALIZE(dataType,serializeMaster,cPtr);
	SERIALIZE(parentHandle,serializeMaster,cPtr);
	SERIALIZE(m_hasFocus,serializeMaster,cPtr);
	SERIALIZE(m_fontExpressionDirty,serializeMaster,cPtr);
	SERIALIZE(m_fontExpressionDirtyReload,serializeMaster,cPtr);
	SERIALIZE_Owner(m_settings,serializeMaster,cPtr);
	SERIALIZE_Owner(m_fontExpr,serializeMaster,cPtr);
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
void	NdhsCTextElement::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummy=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;
	DESERIALIZE(dummy,serializeMaster,cPtr);
	DESERIALIZE(parentHandle,serializeMaster,cPtr);
	NdhsCElement::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE(m_hasFocus,serializeMaster,cPtr);
	DESERIALIZE(m_fontExpressionDirty,serializeMaster,cPtr);
	DESERIALIZE(m_fontExpressionDirtyReload,serializeMaster,cPtr);
	DESERIALIZE_Owner(m_settings,serializeMaster,cPtr,NdhsCMenuWidgetFactory::CSettings)
	DESERIALIZE_Owner(m_fontExpr,serializeMaster,cPtr,NdhsCExpression)
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	DESERIALIZE_Owner(m_useCustomEffect,serializeMaster,cPtr,NdhsCExpression)
	DESERIALIZE(m_bUseCustomEffect,serializeMaster,cPtr);
	DESERIALIZE(m_customEffectDirty,serializeMaster,cPtr);
#endif
#ifdef IFX_USE_PLUGIN_ELEMENTS
	DESERIALIZE_Reserve(m_pPlugin,serializeMaster,cPtr,NdhsCPlugin)
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
/*-------------------------------------------------------------------------*//**
	Destructor
*/
NdhsCTextElement::~NdhsCTextElement()
{
#ifdef IFX_USE_PLUGIN_ELEMENTS
	getPageManager()->releasePluginElement(m_hPluginElement);
#endif
}

/*-------------------------------------------------------------------------*//**
	Load the widget
*/
void NdhsCTextElement::reloadElement()
{
	if(getWidget() != NULL && m_fontExpressionDirtyReload)
		destroyWidget();

	clearFieldTokens();

	LcTaString elementData = getElementData();

	m_marqueeFieldPending = false;

	if (getWidget() == NULL)
	{
		ENdhsWidgetType iconType = ENdhsWidgetTypeText;

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
			setWidget(newWidget);

#ifdef LC_USE_LIGHTS
			getWidget()->setLightModelSimple(m_lightModel == ENdhsLightModelSimple);
#endif
			getWidget()->setVisible(widgetVisible());

			// Set the text specific default color.
			LcTPlacement colourPlacement;
			colourPlacement.color = LcTColor::RED;
			getWidget()->setPlacement(colourPlacement, LcTPlacement::EColor);

			// Text element is tappable or not
			getWidget()->setTappable(getTappable());
			getWidget()->setTapTolerance(getTapTolerance());

			if(m_TE==NULL)
			{
				m_TE=(NdhsCTemplate::CTextElement*)(getPage()->getTemplate()->getElementClass(getElementClassName(),getElementUse()));
			}

			if(m_TE)
			{
				// Update widget material properties
				getWidget()->setMaterialSpecularColor (m_TE->materialSpecularColor);
				getWidget()->setMaterialEmissiveColor (m_TE->materialEmissiveColor);
				getWidget()->setMaterialSpecularShininess (m_TE->materialSpecularShininess);

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

				// Set flag to use Custom Effect
				getWidget()->setUseCustomEffect (m_bUseCustomEffect);

				// Associate effect with this widget
				getWidget()->setVisualEffectName(m_TE->visualEffect);

				// If element does not specify "high" setting, then look for that setting in its parent
				if (m_TE->openGLRenderQuality.compare("normal") == 0)
				{
					if (getElementGroup())
					{
						// Parent has specified "high" setting, so update element's setting from there
						if (getElementGroup()->getOpenGLRenderQualitySetting().compare("high") == 0)
							m_TE->openGLRenderQuality = "high";
					}
				}

				// Set OpenGL rendering quality with which this widget will be rendered
				getWidget()->setOpenGLRenderQualitySetting (m_TE->openGLRenderQuality);

				// Populate widget uniform map with non-interpolatable config/effect uniforms
				getWidget()->populateEffectUniformsFromEffectFile (getPageManager()->getSpace());

				LcTmOwnerMap<LcTmString, LcOglCSLType>::iterator it;

				// Fill in widget effect uniform map.
				for(it = m_TE->effectUniMap.begin(); it != m_TE->effectUniMap.end(); it++)
				{
					getWidget()->addEffectUniform((*it).first, (*it).second, getPageManager()->getSpace());
				}

				// Configure viusal effect related effect uniforms.
				getWidget()->configEffectUniforms(getPageManager()->getSpace(), getPageManager()->getCon()->getAppPath());

#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */
			}
			((LcwCLabel*)getWidget())->setMarquee(m_verticalMarquee, m_horizontalMarquee);
		}
	}
	else
	{
		// Already have a widget, so just modify the text
		((LcwCLabel*)getWidget())->setCaption(elementData);
	}

	m_scrollVelocity = m_settings->marqueeSpeed;

	getWidget()->setDrawLayerIndex(getDrawLayerIndex());

#ifdef IFX_USE_PLUGIN_ELEMENTS
	// Set the caret position
	if ((m_pPlugin != NULL) && (m_hPluginElement != NULL) && (getWidget() != NULL))
	{
		if (m_hasFocus == true)
			((LcwCLabel*)getWidget())->setCaretPosition(m_hPluginElement->getElementCaretPosition());
		else
			((LcwCLabel*)getWidget())->setCaretPosition(-1);
	}
#endif

	if (m_fontExpressionDirtyReload)
		updateWidgetPlacement();
	m_fontExpressionDirtyReload = false;
}

#ifdef IFX_USE_PLUGIN_ELEMENTS
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTextElement::setElementFocus(bool enableFocus)
{
	bool retVal = false;

	m_hasFocus = enableFocus;

	if(m_pPlugin != NULL && m_hPluginElement != NULL)
	{
		retVal = m_hPluginElement->setElementFocus(enableFocus);

		if (retVal)
		{
			// Set the caret position
			if (getWidget() != NULL)
			{
				if (m_hasFocus == true)
					((LcwCLabel*)getWidget())->setCaretPosition(m_hPluginElement->getElementCaretPosition());
				else
					((LcwCLabel*)getWidget())->setCaretPosition(-1);
			}
		}
	}
	else
	{
		retVal = true;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTextElement::processKeyDownEvent(int key)
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
bool NdhsCTextElement::processKeyUpEvent(int key)
{
	bool consumed = false;

	if(m_pPlugin != NULL && m_hPluginElement != NULL)
	{
		consumed = m_hPluginElement->processElementKeyUpEvent(key);
	}

	return consumed;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTextElement::expressionDirty(NdhsCExpression* expr)
{
	NdhsCLaundry* laundry = getLaundry();

	m_fontExpressionDirty	|= (expr == m_fontExpr.ptr());

	m_verticalMarqDirty		|= (expr == m_verticalMarqueeExpr.ptr());

	m_horizontalMarqDirty	|= (expr == m_horizontalMarqueeExpr.ptr());

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	m_customEffectDirty		|= (expr == m_useCustomEffect.ptr());
#endif

	if ((
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	expr == m_useCustomEffect.ptr() ||
#endif
	expr == m_fontExpr.ptr() ||
	expr == m_verticalMarqueeExpr.ptr() ||
	expr == m_horizontalMarqueeExpr.ptr()) && laundry)
	{
		laundry->addItem(this);
	}

	NdhsCElement::expressionDirty (expr);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTextElement::expressionDestroyed(NdhsCExpression* expr)
{
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTextElement::cleanLaundryItem(LcTTime timestamp)
{
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	if (m_customEffectDirty)
	{
		m_useCustomEffect->evaluate(getPage(), -1, getLocalMenuItem());

		if (m_useCustomEffect->isError() || !m_useCustomEffect->isBool())
		{
			m_useCustomEffect->errorDiagnostics("Text element useCustomEffect condition", true);
		}
		else
		{
			m_bUseCustomEffect = m_useCustomEffect->getValueBool();
		}
	}
#endif

	bool marqDirty = false;
	if(m_verticalMarqDirty)
	{
		m_verticalMarqueeExpr->evaluate(getPage(), -1, getMenuItem());
		if (m_verticalMarqueeExpr->isError() || !m_verticalMarqueeExpr->isBool())
		{
			m_verticalMarqueeExpr->errorDiagnostics("Text element verticalMarquee condition", true);
		}
		else
		{
			m_verticalMarquee = m_verticalMarqueeExpr->getValueBool();
		}
		marqDirty = true;
		m_verticalMarqDirty = false;
	}

	if(m_horizontalMarqDirty)
	{
		m_horizontalMarqueeExpr->evaluate(getPage(), -1, getMenuItem());
		if (m_horizontalMarqueeExpr->isError() || !m_horizontalMarqueeExpr->isBool())
		{
			m_horizontalMarqueeExpr->errorDiagnostics("Text element horizontalMarquee condition", true);
		}
		else
		{
			m_horizontalMarquee = m_horizontalMarqueeExpr->getValueBool();
		}
		marqDirty = true;
		m_horizontalMarqDirty = false;
	}

	if(m_verticalMarquee && m_horizontalMarquee)
	{
		m_verticalMarquee = false;
		m_horizontalMarquee = false;
		marqDirty = true;

		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleGeneral, LcTaString("Text element vertical and horizontal Marquee both cannot be enabled"));
	}

	if (m_fontExpressionDirty)
	{
		LcTaString fontName;
		if (m_fontExpr)
		{
			m_fontExpr->evaluate(getPage(), -1, getMenuItem());

			if (m_fontExpr->isError())
			{
				m_fontExpr->errorDiagnostics("Font face", true);
			}
			else
			{
				if (m_fontExpr->getValueString().isEmpty() == false)
				{
					getPageManager()->getTokenStack()->replaceTokens(m_fontExpr->getValueString(), fontName, this, getMenu(), getMenuItem(), getPage(), getStackLevel());
					m_settings->fontName			= fontName;
				}
			}
		}

		if (m_settings->fontName.isEmpty())
			m_settings->fontName = getPageManager()->getDefaultFontFace();

		m_fontExpressionDirtyReload = true;

		NdhsCElement::needsReload(true);
	}

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	// Pass the value to widget
	if (getWidget())
		getWidget()->setUseCustomEffect (m_bUseCustomEffect);
#endif

	if(getWidget() && marqDirty)
	{
		// Stop any current marquee
		stopMarqueeAnimations();

		// Inform widget of new marquee status
		((LcwCLabel*)getWidget())->setMarquee(m_verticalMarquee, m_horizontalMarquee);
	}

	return NdhsCElement::cleanLaundryItem(timestamp);
}

