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
#include "inflexionui/engine/inc/NdhsCField.h"
#include "inflexionui/engine/inc/NdhsCScrollPosField.h"

#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif



/*-------------------------------------------------------------------------*//**
*/
NdhsCElement::NdhsCElement()
{
#ifdef LC_USE_LIGHTS
	m_lightModel			= ENdhsLightModelNormal;
#endif	
}

/*-------------------------------------------------------------------------*//**
 Basic construction.
*/
void NdhsCElement::construct(	ENdhsObjectType		use,
								ENdhsElementType	type,
								const LcTmString&	parent,
								const LcTmString&	className,
								NdhsCExpression::CExprSkeleton*	resourceData,
								NdhsCExpression::CExprSkeleton*	enableFocusSkel,
								NdhsCMenu*			menu,
								NdhsCMenuItem*		menuItem,
								NdhsCPageManager*	pPageManager,
								NdhsCElementGroup*	pPage,
								int					stackLevel,
								int					drawLayerIndex,
								bool				teleportStateChange,
								bool				teleportScroll,
								bool				isDetail)
{
	m_elementType			= type;
	m_elementParent			= parent;
	m_elementUsage			= use;
	m_elementClassName		= className;
	m_menu					= menu;
	m_menuItem				= menuItem;
	m_pageManager			= pPageManager;
	m_page					= pPage;
	m_stackLevel			= stackLevel;
	m_bTeleportStateChange	= teleportStateChange;
	m_bTeleportScroll		= teleportScroll;
	m_isDetail 				= isDetail;

	m_defaultExtent			= LcTVector(0,0,0);

	m_drawLayerIndex = drawLayerIndex;
	m_isRealized 			= false;
	m_needsReload			= false;
	m_needsFocusCheck		= false;
	m_focusEnabled			= true;
	m_transitioning			= false;
	m_unloaded				= false;

	if (resourceData)
	{
		m_resourceData		= resourceData->createExpression(pPage, -1, menuItem);

		if (m_resourceData)
		{
			m_resourceData->setObserver(this);
		}
	}

	if (enableFocusSkel)
	{
		if (!enableFocusSkel->isEmpty())
		{
			m_enableFocus		= enableFocusSkel->createExpression(pPage, -1, menuItem);

			if(m_enableFocus)
			{
				m_enableFocus->setObserver(this);

				m_enableFocus->evaluate();

				if (m_enableFocus->isError() || !m_enableFocus->isBool())
				{
					m_enableFocus->errorDiagnostics("enableFocus condition", true);
				}
				else
				{
					m_focusEnabled = m_enableFocus->getValueBool();
				}
			}
		}
	}

	if (m_menuItem && m_menuItem->isMenuAddition())
		m_packagePath = m_menuItem->getPackagePath();

	if (m_pageManager)
	{
		m_laundry = m_pageManager->getCon()->getLaundry();
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCElement::~NdhsCElement()
{
	if(m_pStaticAnimField)
	{
		m_pStaticAnimField->removeObserver(this);
	}
	if (m_laundry)
	{
		m_laundry->removeItem(this);
	}
#ifdef LC_USE_MOUSEOVER
	m_pageManager->resetMouseOverElement(this);
#endif

	clearFieldTokens();
	destroyWidget();
}

#ifdef IFX_SERIALIZATION

ISerializeable * NdhsCElement::getSerializeAble(int &type)
{
	if(getMenuItem()!=NULL)
	{
		type=-1;
		return NULL;
	}
	type=0;
	return this;
}

SerializeHandle NdhsCElement::serialize(LcCSerializeMaster *serializeMaster,bool force)
{
	SerializeHandle thisHandle=-1;
	if(!force)
	{
		thisHandle=serializeMaster->getHandle(this);
		if(thisHandle!=-1 && serializeMaster->isSerialized(thisHandle))
		{
			return thisHandle;
		}
	}
	else
	{
		thisHandle=serializeMaster->newHandle(this);
	}

	void * output;
	int outputSize=sizeof(NdhsCElement)+sizeof(IFX_INT32)*2;
	IFXP_Mem_Allocate(IFXP_MEM_ENGINE, outputSize, &output);

	void * cPtr=output;

	ENdhsElementType ty=ENdhsElementTypeAggregate;
	SERIALIZE(ty,serializeMaster,cPtr)
	SERIALIZE(m_elementType,serializeMaster,cPtr)
	SERIALIZE(m_elementUsage,serializeMaster,cPtr)
	SERIALIZE_Placement(m_baseStart,serializeMaster,cPtr)
	SERIALIZE(m_touchdownActive,serializeMaster,cPtr)
	SERIALIZE(m_bKeepNonSketchy,serializeMaster,cPtr)
	SERIALIZE(m_bTeleportScroll,serializeMaster,cPtr)
	SERIALIZE(m_bTeleportStateChange,serializeMaster,cPtr)
	SERIALIZE(m_stackLevel,serializeMaster,cPtr)
	SERIALIZE(m_isDetail,serializeMaster,cPtr)
	SERIALIZE(m_unloaded,serializeMaster,cPtr)
	SERIALIZE(m_atEnd,serializeMaster,cPtr)
	SERIALIZE(m_atStart,serializeMaster,cPtr)
	SERIALIZE_Placement(m_currentPlacement,serializeMaster,cPtr)
	SERIALIZE(m_baseEndMaskBackup,serializeMaster,cPtr)
	SERIALIZE(m_baseStartMaskBackup,serializeMaster,cPtr)
	SERIALIZE_Placement(m_baseEndBackup,serializeMaster,cPtr)
	SERIALIZE_Placement(m_baseStartBackup,serializeMaster,cPtr)
	SERIALIZE(m_baseEndUsingLayoutExtent,serializeMaster,cPtr)
	SERIALIZE(m_baseEndMask,serializeMaster,cPtr)
	SERIALIZE_Placement(m_baseEnd,serializeMaster,cPtr)
	SERIALIZE(m_baseStartUsingLayoutExtent,serializeMaster,cPtr)
	SERIALIZE(m_baseStartMask,serializeMaster,cPtr)
	SERIALIZE(m_focusEnabled,serializeMaster,cPtr)
	SERIALIZE(m_needsFocusCheck,serializeMaster,cPtr)
	SERIALIZE(m_needsReload,serializeMaster,cPtr)
	SERIALIZE(m_hasFocus,serializeMaster,cPtr)
	SERIALIZE(m_isRealized,serializeMaster,cPtr)
#ifdef LC_USE_LIGHTS
	SERIALIZE(m_lightModel,serializeMaster,cPtr)
#endif
	SERIALIZE(m_tapTolerance,serializeMaster,cPtr)
	SERIALIZE(m_tappable,serializeMaster,cPtr)
	SERIALIZE(m_defaultExtent,serializeMaster,cPtr)
	SERIALIZE(m_endExtentFromWidget,serializeMaster,cPtr)
	SERIALIZE(m_startExtentFromWidget,serializeMaster,cPtr)
	SERIALIZE(m_animate,serializeMaster,cPtr)
	SERIALIZE(m_drawLayerIndex,serializeMaster,cPtr)
	SERIALIZE(m_isVisible,serializeMaster,cPtr)
	SERIALIZE(m_makeVisible,serializeMaster,cPtr)
	SERIALIZE(m_bCurveForwards,serializeMaster,cPtr)
	SERIALIZE(m_hideElement,serializeMaster,cPtr)
	SERIALIZE(m_suspendVisible,serializeMaster,cPtr)
	SERIALIZE_Placement(m_suspendPlacement,serializeMaster,cPtr)
	SERIALIZE(m_suspendWidgetRealized,serializeMaster,cPtr)
	SERIALIZE(m_suspendWidgetExists,serializeMaster,cPtr)
	SERIALIZE_String(m_elementParent,serializeMaster,cPtr)
	SERIALIZE_String(m_packagePath,serializeMaster,cPtr)
	SERIALIZE_String(m_elementClassName,serializeMaster,cPtr)
	SERIALIZE_Array(m_fieldTokens,serializeMaster,cPtr)
	SERIALIZE_Ptr(m_pStaticAnimField,serializeMaster,cPtr)
	SERIALIZE_Owner(m_resourceData,serializeMaster,cPtr)
	SERIALIZE_Owner(m_enableFocus,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_pageManager,serializeMaster,cPtr)
	SERIALIZE_Ptr(m_page,serializeMaster,cPtr)
	SERIALIZE_Ptr(m_owningGroup,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_menu,serializeMaster,cPtr)

	serializeMaster->setData(thisHandle,outputSize,(LcTByte*)output);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,output);
	return thisHandle;
}

void NdhsCElement::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummySize=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	DESERIALIZE(dummySize,serializeMaster,cPtr)
	DESERIALIZE(m_elementType,serializeMaster,cPtr)
	DESERIALIZE(m_elementUsage,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_baseStart,serializeMaster,cPtr)
	DESERIALIZE(m_touchdownActive,serializeMaster,cPtr)
	DESERIALIZE(m_bKeepNonSketchy,serializeMaster,cPtr)
	DESERIALIZE(m_bTeleportScroll,serializeMaster,cPtr)
	DESERIALIZE(m_bTeleportStateChange,serializeMaster,cPtr)
	DESERIALIZE(m_stackLevel,serializeMaster,cPtr)
	DESERIALIZE(m_isDetail,serializeMaster,cPtr)
	DESERIALIZE(m_unloaded,serializeMaster,cPtr)
	DESERIALIZE(m_atEnd,serializeMaster,cPtr)
	DESERIALIZE(m_atStart,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_currentPlacement,serializeMaster,cPtr)
	DESERIALIZE(m_baseEndMaskBackup,serializeMaster,cPtr)
	DESERIALIZE(m_baseStartMaskBackup,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_baseEndBackup,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_baseStartBackup,serializeMaster,cPtr)
	DESERIALIZE(m_baseEndUsingLayoutExtent,serializeMaster,cPtr)
	DESERIALIZE(m_baseEndMask,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_baseEnd,serializeMaster,cPtr)
	DESERIALIZE(m_baseStartUsingLayoutExtent,serializeMaster,cPtr)
	DESERIALIZE(m_baseStartMask,serializeMaster,cPtr)
	DESERIALIZE(m_focusEnabled,serializeMaster,cPtr)
	DESERIALIZE(m_needsFocusCheck,serializeMaster,cPtr)
	DESERIALIZE(m_needsReload,serializeMaster,cPtr)
	DESERIALIZE(m_hasFocus,serializeMaster,cPtr)
	DESERIALIZE(m_isRealized,serializeMaster,cPtr)
#ifdef LC_USE_LIGHTS
	DESERIALIZE(m_lightModel,serializeMaster,cPtr)
#endif
	m_isRealized=false;
	DESERIALIZE(m_tapTolerance,serializeMaster,cPtr)
	DESERIALIZE(m_tappable,serializeMaster,cPtr)
	DESERIALIZE(m_defaultExtent,serializeMaster,cPtr)
	DESERIALIZE(m_endExtentFromWidget,serializeMaster,cPtr)
	DESERIALIZE(m_startExtentFromWidget,serializeMaster,cPtr)
	DESERIALIZE(m_animate,serializeMaster,cPtr)
	DESERIALIZE(m_drawLayerIndex,serializeMaster,cPtr)
	DESERIALIZE(m_isVisible,serializeMaster,cPtr)
	DESERIALIZE(m_makeVisible,serializeMaster,cPtr)
	DESERIALIZE(m_bCurveForwards,serializeMaster,cPtr)
	DESERIALIZE(m_hideElement,serializeMaster,cPtr)
	DESERIALIZE(m_suspendVisible,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_suspendPlacement,serializeMaster,cPtr)
	DESERIALIZE(m_suspendWidgetRealized,serializeMaster,cPtr)
	DESERIALIZE(m_suspendWidgetExists,serializeMaster,cPtr)
	DESERIALIZE_String(m_elementParent,serializeMaster,cPtr)
	DESERIALIZE_String(m_packagePath,serializeMaster,cPtr)
	DESERIALIZE_String(m_elementClassName,serializeMaster,cPtr)
	DESERIALIZE_Array(m_fieldTokens,serializeMaster,cPtr)
	DESERIALIZE_Ptr(m_pStaticAnimField,serializeMaster,cPtr,NdhsCScrollPosField)
	DESERIALIZE_Owner(m_resourceData,serializeMaster,cPtr,NdhsCExpression)
	DESERIALIZE_Owner(m_enableFocus,serializeMaster,cPtr,NdhsCExpression)
	DESERIALIZE_Reserve(m_pageManager,serializeMaster,cPtr,NdhsCPageManager)
	DESERIALIZE_Ptr(m_page,serializeMaster,cPtr,NdhsCElementGroup)
	DESERIALIZE_Ptr(m_owningGroup,serializeMaster,cPtr,NdhsCElementGroup)
	DESERIALIZE_Reserve(m_menu,serializeMaster,cPtr,NdhsCMenu)

	if (m_pageManager)
	{
		m_laundry = m_pageManager->getCon()->getLaundry();
	}
}

/*-------------------------------------------------------------------------*//**
*/

void NdhsCElement::deflate()
{
	m_laundry->addItem(this);
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCElement * NdhsCElement::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * ptr=serializeMaster->getOffset(handle);

	int type=*((int*)ptr);

	switch(type)
	{
		case ENdhsElementTypeText:
		{
			return NdhsCTextElement::loadState(handle,serializeMaster);
		}
		case ENdhsElementTypeGraphic:
		{
			return NdhsCGraphicElement::loadState(handle,serializeMaster);
		}
#ifdef LC_USE_STYLUS
		case ENdhsElementTypeDragRegion:
		{
			return NdhsCDragRegionElement::loadState(handle,serializeMaster);
		}
#endif
#ifdef IFX_USE_PLUGIN_ELEMENTS
		case ENdhsElementTypePlugin:
		{
			return NdhsCPluginElement::loadState(handle,serializeMaster);
		}
#endif
#ifdef LC_USE_LIGHTS
		case ENdhsElementTypeLight:
		{
			return NdhsCLightElement::loadState(handle,serializeMaster);
		}
#endif
	}
	return NULL;
}
#endif /* IFX_SERIALIZATION */

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCElement::getElementData()
{
	LcTaString elementData = "";
	bool elementDataFound = false;
	LcTaString data;

	switch (m_elementUsage)
	{
		case ENdhsObjectTypeFurniture:
		{
			if (m_isDetail)
			{
				// Detail uses menu item data
				if (m_menuItem)
				{
					if (m_menuItem->getFieldData(this, m_elementClassName, data))
					{
						elementData = data;
						elementDataFound = true;
						m_menuItem->isMenuAddition();
					}
				}
			}
			else
			{
				// Furniture items cannot get data from menu items
				if (m_menu)
				{
					if (m_menu->getFieldData(this, m_elementClassName, data))
					{
						elementData = data;
						elementDataFound = true;
					}
				}
			}
			break;
		}

		case ENdhsObjectTypeItem:
		{
			// Item uses menu item data
			if (m_menuItem)
			{
				if (m_menuItem->getFieldData(this, m_elementClassName, data))
				{
					elementData = data;
					elementDataFound = true;
					m_menuItem->isMenuAddition();
				}
			}
			break;
		}

		// Do Nothing
		default:
			break;
	}

	// If the element data is still missing, look in the template default
	if (!elementDataFound)
	{
		if (m_resourceData)
		{
			m_resourceData->evaluate(m_page, -1, m_menuItem);

			if (m_resourceData->isError())
			{
				m_resourceData->errorDiagnostics("Resource data", true);
			}
			else
			{
				elementData = m_resourceData->getValueString();
			}
		}
	}
	else
	{
		if (m_menu)
		{
			// If the class name is used, we must check to see
			// if a cached field exists and register as an observer
			// for field refreshes
			int index = -1;

			if (m_menuItem && (m_isDetail || m_elementUsage == ENdhsObjectTypeItem))
			{
				index = m_menuItem->getIndex();
			}

			NdhsCField* field = m_menu->getField(m_elementClassName, index, this);

			if (field)
			{
				registerFieldToken(field);
			}
		}
	}

	// now run the token replacer over the element data
	if (m_isDetail || m_elementUsage == ENdhsObjectTypeItem)
	{
		m_pageManager->getTokenStack()->replaceTokens(elementData, data, this, NULL, m_menuItem, m_owningGroup, m_stackLevel);
	}
	else
	{
		m_pageManager->getTokenStack()->replaceTokens(elementData, data, this, m_menu, NULL, m_owningGroup, m_stackLevel);
	}

	elementData = data;

	return elementData;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::refreshField(NdhsCField* field)
{
	bool found = false;

	if(getUnloaded())
		return;

	// Find the field
	if (!m_fieldTokens.empty())
	{
		TmMFieldTokens::iterator it = m_fieldTokens.begin();
		while(!found && it != m_fieldTokens.end())
		{
			if (field == *it)
			{
				found = true;
			}

			it++;
		}
	}

	// The field passed in must be lower case, it would be
	// inefficient to convert in here.
	// Only refresh if we have field tokens AND we are using the passed field token
	// OR the class name matches the field
	if (found
		|| (field && field->getFieldName().compareNoCase(m_elementClassName) == 0))
	{
		reloadElement();

		if (m_isRealized)
		{
			realize();
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::registerFieldToken(NdhsCField* field)
{
	if (field)
	{
		field->addObserver(this);
		m_fieldTokens.push_back(field);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::clearFieldTokens()
{
	if (!m_fieldTokens.empty())
	{
		TmMFieldTokens::iterator it = m_fieldTokens.begin();
		while(it != m_fieldTokens.end())
		{
			(*it)->removeObserver(this);
			it++;
		}
	}

	m_fieldTokens.clear();
}

/*-------------------------------------------------------------------------*//**
 Menu Slot index accessors.
 The slots are properties of the element group that this element belongs to.
*/
int NdhsCElement::getSlot()
{
	return (m_owningGroup == NULL)? -1 : m_owningGroup->getSlot();
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCElement::getOldSlot()
{
	return (m_owningGroup == NULL)? -1 : m_owningGroup->getOldSlot();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::updatePosition(ENdhsAnimationType animType,
										LcTScalar position,
										bool positionIncreasing,
										bool updateCache,
										bool forceNonSketchy,
										bool aggregateAnimating,
										bool finalFrame)
{
	int mask = 0;

	m_atStart = (position == 0);
	m_atEnd = (position == 1);
	m_transitioning = !finalFrame;

	if (updateCache)
	{
		// If we've passed through a slot and we were teleporting, we need
		// to apply the pending visibility request
		if (m_makeVisible && !finalFrame)
		{
			setVisible(true);
			m_makeVisible = false;
		}

		// Query for base placements, displacement list, curve and decoration
		m_page->getTransitionAgent()->updateAnimationCache(this, m_isDetail, animType, positionIncreasing);

		// Inspect the start and end points, and decide whether we animate this 'segment'
		m_animate = updateElementStatus(animType, finalFrame);

		// For base tweens, always set up the shortest rotation path
		if ((m_baseStartMask | m_baseEndMask) & LcTPlacement::EOrientation)
			m_baseEnd.orientation.shortestPath(m_baseStart.orientation);

		// Now work out the 'delta' placement between start and end points
		m_basePath.setPathData(m_baseStart, m_baseEnd, m_baseStartMask | m_baseEndMask, ENdhsDecoratorTypeLinear);

		// Assume high quality mode, unless our aggregate is animating
		m_bKeepNonSketchy = !aggregateAnimating;

		if (m_animate)
		{
			// Check what parts of the placement will actually change
			mask = m_basePath.getMask();

			// Check if we've a curve to follow, or a decoration to perform
			if (m_pCurve)
				mask |= m_pCurve->getMask();

			if (m_pDecorator)
				mask |= m_pDecorator->getMask();

			// If the placement isn't actually going to change, don't animate
			if (mask == 0)
				m_animate = false;

			// If element will move on screen, use sketchy mode if possible
			if ( (mask & LcTPlacement::ELocation)
				|| (mask & LcTPlacement::EExtent)
				|| (mask & LcTPlacement::EScale)
				|| (mask & LcTPlacement::EOrientation) )
			{
				m_bKeepNonSketchy = false;
			}
		}

		// When we're flick scrolling or dragging quickly, there's a chance that some effect
		// of the previous animation segment is left on the widget placement, so we 'initialize'
		// the placement each time we update the cache with the nearest base placement.
		if (getWidget())
		{
			if ((position <= .5) && (m_baseStartMask != 0))
			{
				m_currentPlacement = m_baseStart;
				getWidget()->setUsingLayoutExtent(m_baseStartUsingLayoutExtent);
			}
			else if ((position > .5) && (m_baseEndMask != 0))
			{
				m_currentPlacement = m_baseEnd;
				getWidget()->setUsingLayoutExtent(m_baseEndUsingLayoutExtent);
			}

			LcTPlacement currentPlacement = m_currentPlacement;

			(void)applyStaticDisplacement(currentPlacement);

			getWidget()->setPlacement(currentPlacement, LcTPlacement::EAll);
		}
	}

	if (getWidget() && !forceNonSketchy && !m_bKeepNonSketchy)
		getWidget()->setSketchyAnimation(true);

	// Note that we always position the element at the start or end of the animation,
	// whether it was tweening or not
	if (!finalFrame && !m_animate)
		return;

	// Check to see if we should be visible, etc - ignore return flag as once we've started
	// an animation we'll continue to animate evenly until complete
	if (!updateCache)
		updateElementStatus(animType, finalFrame);

	if (m_animate)
	{
		// Reset current placement
		m_currentPlacement = m_baseStart;

		// Now position the element
		mask = calculatePosition(position, m_currentPlacement);
	}

	// If this is the last frame, draw in high quality.
	if (finalFrame)
	{
		if (getWidget())
		{
			getWidget()->setSketchyAnimation(false);
			getWidget()->setUsingLayoutExtent(m_baseEndUsingLayoutExtent);
		}

		// Set visibility if needed
		// If we're at the end of this part, there's an end placement
		// and we're not visible, show ourselves
		if (m_makeVisible)
			setVisible(true);

		if (m_animate)
		{
			// Elements that have animated need to be revalidated, as the set
			// placement will not 'dirty' the widget.
			if (getWidget())
				getWidget()->revalidate();
		}
		else
		{
			// Elements that have teleported need to check that the final position
			// is not '0' (which it might be when dragging is used).  If it is, then
			// the placement and visibility will be wrong!
			if (position == 0)
			{
				if (m_baseStartMaskBackup != 0)
				{
					// Regenerate current placement
					m_currentPlacement = m_baseStartBackup;
					mask = m_baseStartMaskBackup;
					applyDisplacements(m_BaseStartDisplacementArray, m_currentPlacement, mask);

					// In case the base tween modified anything not covered by m_baseStartMaskBackup
					mask |= m_baseEndMask;

					setVisible(true);
				}
				else
				{
					// If we are here then start mask backup is '0' if we have teleported 
					// we must have end mask backup not '0'				
					if (m_baseEndMaskBackup != 0)
						setVisible(false);
				}
			}
			else
			{
				// If we are teleporting then set the position to the end.
				if (m_baseEndMaskBackup != 0)
				{
					// Regenerate current placement
					m_currentPlacement = m_baseEndBackup;
					mask = m_baseEndMaskBackup;
					applyDisplacements(m_BaseEndDisplacementArray, m_currentPlacement, mask);

					// In case the base tween modified anything not covered by m_baseEndMaskBackupS
					mask |= m_baseStartMask;

					setVisible(true);
				}
			}

			// Cater for the case where the element is being statically animated
			// (i.e. it's a furniture element, and the animation is a scroll
			mask |= applyStaticDisplacement(m_currentPlacement);
		}
	}

	// Now update the widget position
	if(getWidget())
		getWidget()->setPlacement(m_currentPlacement, mask);
}

/*-------------------------------------------------------------------------*//**
	Calculate current placement.
*/
int NdhsCElement::calculatePosition(LcTScalar position, LcTPlacement& placement)
{
	int mask = 0;

	// Interpolate
	m_basePath.getPlacement(position, placement);
	mask = m_basePath.getMask();

	// Is there a curve?
	if (m_pCurve)
	{
		LcTPlacement curveOffset;
		int curveMask;
		if (m_bCurveForwards)
			curveMask = m_pCurve->getPlacement(position, curveOffset, LcTColor::WHITE, LcTColor::WHITE, LcTColor::GRAY20, LcTColor::GRAY20);
		else
			curveMask = m_pCurve->getPlacement((1-position), curveOffset, LcTColor::WHITE, LcTColor::WHITE, LcTColor::GRAY20, LcTColor::GRAY20);
		placement.offset(curveOffset, curveMask);
		mask |= curveMask;
	}

	// Apply a decoration?
	if (m_pDecorator)
	{
		LcTPlacement decoration;
		int decMask = m_pDecorator->getPlacement(position, decoration, m_baseStart.color, m_baseEnd.color, m_baseStart.color2, m_baseEnd.color2);
		placement.offset(decoration, decMask & (~LcTPlacement::EColor) & (~LcTPlacement::EColor2));

		if (decMask & LcTPlacement::EColor)
		{
			placement.color = decoration.color;
		}

		if (decMask & LcTPlacement::EColor2)
		{
			placement.color2 = decoration.color2;
		}

		mask |= decMask;
	}

	return mask;
}

/*-------------------------------------------------------------------------*//**
	Apply displacements.
*/
void NdhsCElement::applyDisplacements(const LcTmArray<NdhsCTemplate::CElementDisplacement*> & pDisplacements,
										LcTPlacement& placement,
										int& placementMask)
{
	typedef NdhsCTemplate::CDisplacement CDisplacement;
	typedef NdhsCTemplate::CElementDisplacement CElementDisplacement;
	typedef LcTmOwnerMap<LcTmString, NdhsCExpression> TmExpressionMap;
	int displacementMask = 0;
	LcTColor baseColor(LcTColor::WHITE);
	LcTColor originalPlacementColor = placement.color;
	LcTColor baseColor2(LcTColor::GRAY20);
	LcTColor originalPlacementColor2 = placement.color2;

	if (placementMask & LcTPlacement::EColor)
	{
		baseColor = placement.color;
	}

	if (placementMask & LcTPlacement::EColor2)
	{
		baseColor2 = placement.color2;
	}

	// If any displacement modifies the color, we'll be overwriting the base color
	placement.color = LcTColor::WHITE;

	if (pDisplacements.size()!=0)
	{
		CElementDisplacement * disp=NULL;
		LcTmArray<CElementDisplacement*>::const_iterator it = pDisplacements.begin();
		for (; it !=  pDisplacements.end(); it++)
		{
			disp = *it;
			if (disp->displacement->value.isEmpty())
				continue;

			// Get the named displacement
			const NdhsCKeyFrameList* pDisplacementKFL = m_page->getTemplate()->getAnimation(disp->displacement->animation);

			if (pDisplacementKFL)
			{
				LcTScalar frac = disp->displacement->getNormalizedValue(disp->expression.ptr());

				// Get placement
				LcTPlacement displacement;
				int mask = pDisplacementKFL->getPlacement(frac, displacement, baseColor, baseColor, baseColor2, baseColor2);
				disp->cachedPlacement=displacement;
				disp->cachedMask=mask;

				// Apply to base placement
				placement.offset(displacement, mask);
				displacementMask |= mask;
				placementMask |= mask;
			}
		}
	}

	// If nothing modified the placement color, we need to reset it
	if (!(displacementMask & LcTPlacement::EColor))
	{
		placement.color = originalPlacementColor;
	}

	if (!(displacementMask & LcTPlacement::EColor2))
	{
		placement.color2 = originalPlacementColor2;
	}
}

/*-------------------------------------------------------------------------*//**
	Apply displacements.
*/
void NdhsCElement::applyDisplacements(const LcTmArray<NdhsCTemplate::CElementDisplacement*> & pDisplacements,
										LcTPlacement& placement,
										int& placementMask,NdhsCTemplate::CElementDisplacement* currentDisplacement)
{
	typedef NdhsCTemplate::CDisplacement CDisplacement;
	typedef NdhsCTemplate::CElementDisplacement CElementDisplacement;
	typedef LcTmOwnerMap<LcTmString, NdhsCExpression> TmExpressionMap;
	int displacementMask = 0;
	LcTColor baseColor(LcTColor::WHITE);
	LcTColor originalPlacementColor = placement.color;
	LcTColor baseColor2(LcTColor::GRAY20);
	LcTColor originalPlacementColor2 = placement.color2;

	if (placementMask & LcTPlacement::EColor)
	{
		baseColor = placement.color;
	}

	if (placementMask & LcTPlacement::EColor2)
	{
		baseColor2 = placement.color2;
	}

	// If any displacement modifies the color, we'll be overwriting the base color
	placement.color = LcTColor::WHITE;

	if (pDisplacements.size()!=0)
	{
		CElementDisplacement * disp=NULL;
		LcTmArray<CElementDisplacement*>::const_iterator it = pDisplacements.begin();
		for (; it !=  pDisplacements.end(); it++)
		{
			disp=*it;
			if (disp->displacement->value.isEmpty())
				continue;

			// Get the named displacement
			const NdhsCKeyFrameList* pDisplacementKFL = NULL;

			if(currentDisplacement==disp)
				pDisplacementKFL = m_page->getTemplate()->getAnimation(disp->displacement->animation);

			if (pDisplacementKFL)
			{
				LcTScalar frac = disp->displacement->getNormalizedValue(disp->expression.ptr());

				// Get placement
				LcTPlacement displacement;
				int mask = pDisplacementKFL->getPlacement(frac, displacement, baseColor, baseColor, baseColor2, baseColor2); // cache here
				disp->cachedPlacement = displacement;
				disp->cachedMask = mask;
#ifdef LC_USE_MOUSEOVER
				if (frac == 1)
					getPageManager()->displacementApplied();
#endif
				// Apply to base placement
				placement.offset(displacement, mask);
				displacementMask |= mask;
				placementMask |= mask;
			}
			else if(currentDisplacement!=NULL)
			{
				// Apply to base placement
				placement.offset(disp->cachedPlacement, disp->cachedMask);
				displacementMask |= disp->cachedMask;
				placementMask |= disp->cachedMask;
			}
		}
	}

	// If nothing modified the placement color, we need to reset it
	if (!(displacementMask & LcTPlacement::EColor))
	{
		placement.color = originalPlacementColor;
	}

	if (!(displacementMask & LcTPlacement::EColor2))
	{
		placement.color2 = originalPlacementColor2;
	}
}

/*-------------------------------------------------------------------------*//**
	Realize the element (i.e. its widget) on an aggregate
*/
void NdhsCElement::realize()
{
	m_isRealized = true;

	// Tell the page manager this element is being realized.
	m_pageManager->setLastRealizedElement(this);

	if (m_widget.ptr() != NULL)
		m_widget->realize(m_owningGroup);
}

/*-------------------------------------------------------------------------*//**
	Retire the element (i.e. its widget)
*/
void NdhsCElement::retire()
{
	resetTransitionCache();
	stopStaticAnimations();

	m_pageManager->setLastRealizedElement(NULL);

	if (m_widget.ptr() != NULL)
		m_widget->retire();

	m_isRealized = false;
}

/*-------------------------------------------------------------------------*//**
	Work out the combined animation mask
*/
int NdhsCElement::getAnimateMask()
{
	int mask = 0;

	if (m_animate)
	{
		mask |= m_basePath.getMask();

		if (m_pCurve)
			mask |= m_pCurve->getMask();

		if (m_pDecorator)
			mask |= m_pDecorator->getMask();
	}

	if (m_owningGroup)
		mask |= m_owningGroup->getAnimateMask();

	return mask;
}

/*-------------------------------------------------------------------------*//**
	Work out the combined static animation mask
*/
int NdhsCElement::getStaticAnimateMask()
{
	int mask = 0;

	if (m_pStaticAnim)
		mask |= m_pStaticAnim->getMask();

	mask |= m_owningGroup->getStaticAnimateMask();

	return mask;
}

/*-------------------------------------------------------------------------*//**
	Check whether we need to change the visibility status of the element
*/
bool NdhsCElement::updateElementStatus(ENdhsAnimationType animType, bool finalFrame)
{
	bool canAnimate = false;
	bool forceHide = false;

	// If element is set to hide in the theme
	if (getHideElementFlag() == true)
	{
		setVisible(false);
		return canAnimate;
	}

	// Now check whether we should be teleporting, etc
	switch(animType)
	{
		case ENdhsAnimationTypeInteractiveState:
		case ENdhsAnimationTypeTerminalState:
		// State change - we are moving elements between the old and new layouts
		{
			if(m_baseStartMask != 0 && m_baseEndMask != 0)
			{
				// Both start and end placements for this element exist
				// Terminal animations don't teleport (unless it's a drag
				// region, which always teleports)
				if ((animType == ENdhsAnimationTypeTerminalState)
#ifdef LC_USE_STYLUS
					 && (m_elementType != ENdhsElementTypeDragRegion)
#endif
					 )
				{
					canAnimate = true;
				}

				if(m_isDetail)
				{
					// Detail elements don't teleport if they've a decoration
					if (m_pDecorator != NULL)
						canAnimate = true;

					// Detail elements must teleport if the start state is 'open'
					// or if the end state is 'close'
					if ((m_page->getPageState() == ENdhsPageStateClose)
								|| (m_page->getPrevPageState() == ENdhsPageStateOpen))
					{
						canAnimate = false;
					}
				}
				else
				{
					// For other elements, check the configured behavior
					if(!m_bTeleportStateChange)
						canAnimate = true;
				}
			}

			if(canAnimate != true)
			{
				forceHide = true;
			}

			break;
		}

		case ENdhsAnimationTypeScroll:
		// Moving menu items from slot to slot.
		{
			// Get start and end slot values
			int startSlot = getOldSlot();
			int endSlot = getSlot();

			// We have 4 cases...
			if ((startSlot == -1) && (endSlot == -1))
			{
				if (m_isDetail || m_elementUsage == ENdhsObjectTypeItem)
				{
					// Detail and Item elements should always be invisible in
					// this scenario
					forceHide = true;
				}
				else
				{
					// During scroll furniture element can have scroll-teleport effect 
					if (m_bTeleportScroll)
					{
						forceHide = true;
					}
					else
					{
						canAnimate = true;
					}
				}
			}
			else if(startSlot != -1 && endSlot == -1)
			{
				// Teleport element out of slot
				forceHide = true;
			}
			else if(startSlot == -1 && endSlot != -1)
			{
				// Teleport element into slot
				forceHide = true;
			}
			else if(startSlot != -1 && endSlot != -1)
			{
				// Moving element between slots
				if(m_baseStartMask != 0 && m_baseEndMask != 0)
				{
					// Start and end placements exist
					if(m_isDetail)
					{
						// Detail elements only animate if a decoration is defined
						if (m_pDecorator != NULL)
							canAnimate = true;
					}
					else
					{
						if(!m_bTeleportScroll)
							canAnimate = true;
					}
				}

				// Don't allow items to animate between non-selectable
				// slots when they are wrapping round the end
				if ( (((NdhsCMenuComponent*)m_page)->isScrollDirectionPositive() && startSlot < ((NdhsCMenuComponent*)m_page)->getMenuTemplate()->getFirstSelectableSlot()
						&& endSlot > ((NdhsCMenuComponent*)m_page)->getMenuTemplate()->getLastSelectableSlot())
					|| (!((NdhsCMenuComponent*)m_page)->isScrollDirectionPositive() && startSlot > ((NdhsCMenuComponent*)m_page)->getMenuTemplate()->getLastSelectableSlot()
						&& endSlot < ((NdhsCMenuComponent*)m_page)->getMenuTemplate()->getFirstSelectableSlot()))
				{
					canAnimate = false;
				}

				if(canAnimate == false)
				{
					forceHide = true;
				}
			}

			break;
		}

		default:
			break;
	}

	if (forceHide && !finalFrame && getWidget())
	{
		setVisible(false);
	}

	if (canAnimate)
	{
		setVisible(true);
	}
	else
	{
		// Teleporting, so make end and start placements the same
		teleportElement();
	}

	return canAnimate;
}

/*-------------------------------------------------------------------------*//**
	Set up the element to teleport
*/
void NdhsCElement::teleportElement()
{
	if (m_baseEndMask != 0)
	{
		// Set 'to be made visible' true
		m_makeVisible = true;
	}
}

/*-------------------------------------------------------------------------*//**
	Reset cached data pointing into a transition agent layout
*/
void NdhsCElement::resetTransitionCache()
{
	unsubscribeToFields(NULL,false);
	unsubscribeToFields(NULL,true);
	m_pCurve = NULL;
}

/*-------------------------------------------------------------------------*//**
	Set visibility flag on owned widget
*/
void NdhsCElement::setVisible(bool bVisible)
{
	// Store visiblility status of widget
	m_isVisible = bVisible;

	if (m_widget.ptr() != NULL)
	{
		if (m_widget->getVisible() != bVisible)
			m_widget->setVisible(bVisible);
	}

	if (m_page && !bVisible)
	{
		m_page->doCheckVisibilityChanges();
	}
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::populateElementList(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList)
{
	NdhsCPageManager::TmAWidgets::iterator it = widgets.begin();
	int i = 0;

	for (; it != widgets.end(); ++it, ++i)
	{
		LcCWidget* widget = *it;

		if (widget == m_widget.ptr())
		{
			pageWidgetElemList[i].element = this;
			pageWidgetElemList[i].eventManager = m_page;
			return;
		}
	}
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::startStaticAnimations()
{
	// Query transition agent for any static animations (unless one is
	// currently active)
	if (m_pStaticAnim == NULL)
		m_page->getTransitionAgent()->getStaticAnimation(this, m_isDetail);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::stopStaticAnimations()
{
	if (!m_pStaticAnim)
		return;

	// Remove animation field from page list
	m_page->releaseStaticAnimationField(m_pStaticAnimField);
	m_pStaticAnimField = NULL;

	// Forget about animation
	m_pStaticAnim = NULL;

	// Reset placement
	if (m_atStart)
		m_currentPlacement = m_baseStart;
	else if (m_atEnd)
		m_currentPlacement = m_baseEnd;

	if (getWidget())
		getWidget()->setPlacement(m_currentPlacement, LcTPlacement::EAll);

#ifdef IFX_GENERATE_SCRIPTS
	if (NdhsCScriptGenerator::getInstance())
		NdhsCScriptGenerator::getInstance()->setAnimationStops();
#endif //IFX_GENERATE_SCRIPTS

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::setStaticAnimation(NdhsCKeyFrameList* pDec,
											unsigned int staticTime,
											int loopCount,
											ENdhsVelocityProfile velocityProfile)
{
	// Store the details of the animation
	m_pStaticAnim = pDec;

	if (pDec)
	{
		if (m_pStaticAnimField)
			m_page->releaseStaticAnimationField(m_pStaticAnimField);

		// Get the animation field reference
		m_pStaticAnimField = m_page->getStaticAnimationField(this);

		// Configure the field for the animation
		if (m_pStaticAnimField)
		{
			bool retVal;

			retVal = m_pStaticAnimField->setTargetValue(1.0, ENdhsFieldDirectionIncreasing, false, staticTime,
											ENdhsScrollFieldModeNormal, velocityProfile);
			m_pStaticAnimField->setLoop(loopCount);

			if (!retVal)
			{
				m_page->releaseStaticAnimationField(m_pStaticAnimField);
				m_pStaticAnim = NULL;
			}

#ifdef IFX_GENERATE_SCRIPTS
	if(retVal)
	{
		if (NdhsCScriptGenerator::getInstance())
			NdhsCScriptGenerator::getInstance()->setAnimationStarts(staticTime);
	}
#endif //IFX_GENERATE_SCRIPTS

		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCElement::applyStaticDisplacement(LcTPlacement& placement)
{
	if (!m_pStaticAnimField || !m_pStaticAnim)
		return 0;

	LcTScalar val = m_pStaticAnimField->getRawFieldData(NULL);

	// Check for wrapping
	val = val - (int)val;

	// Check for endpoint of static animation...
	if (m_pStaticAnimField->atRest())
		val = 1.0;

	// Get the placement data.
	// NOTE: when the element is in a static animation (m_atEnd == true) we
	// are only interested in the animating the final end color.
	LcTPlacement delta;
	int mask;

	if (m_atEnd)
		mask = m_pStaticAnim->getPlacement(val, delta, m_baseEnd.color, m_baseEnd.color, m_baseEnd.color2, m_baseEnd.color2);
	else
		mask = m_pStaticAnim->getPlacement(val, delta, m_baseStart.color, m_baseEnd.color, m_baseStart.color2, m_baseEnd.color2);

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	// Prepare static animation if there are effect uniforms in there
	if(getWidget())
		getWidget()->prepareStaticDisplacementFromEffect (placement, delta);
#endif

	// Note that if a static animation has been interrupted, color should no longer
	// be modified
	if (m_page->areStaticAnimationsStopping())
		mask &= ~(LcTPlacement::EColor | LcTPlacement::EColor2);

	if ((val >= 0) && (val <= 1))
	{
		placement.offset(delta, mask & ~(LcTPlacement::EColor | LcTPlacement::EColor2));

		if (mask & LcTPlacement::EColor)
		{
			placement.color = delta.color;
		}

		if (mask & LcTPlacement::EColor2)
		{
			placement.color2 = delta.color2;
		}
	}

	return mask;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::fieldValueUpdated(NdhsCField* field)
{
	refreshField(field);

	if (field == m_pStaticAnimField && m_pStaticAnim)
	{
		LcTPlacement placement;

		// If we're not animating, then we need to reset currentPlacement
		// each time or else the static animation would have a cumulative
		// effect
		if (m_atStart)
			placement = m_baseStart;
		else if (m_atEnd)
			placement = m_baseEnd;
		else
			placement = m_currentPlacement;

		int mask = applyStaticDisplacement(placement);

		if (getWidget())
			getWidget()->setPlacement(placement, mask);
	}
	else
	{
		// There are 3 cases here - either we're animating, in which case
		// the element position will be calculated shortly, or we're at the
		// start or end point of the last transition.  In the latter two cases
		// we need to update the widget placement otherwise the displacement will
		// not be seen.
		m_baseStart = m_baseStartBackup;
		m_baseEnd = m_baseEndBackup;
		m_baseStartMask = m_baseStartMaskBackup;
		m_baseEndMask = m_baseEndMaskBackup;

		applyDisplacements(m_BaseStartDisplacementArray, m_baseStart, m_baseStartMask);
		applyDisplacements(m_BaseEndDisplacementArray, m_baseEnd, m_baseEndMask);

		if (m_animate && ((m_baseStartMask | m_baseEndMask) & LcTPlacement::EOrientation))
		{
			// For base tweens, always set up the shortest rotation path
			m_baseEnd.orientation.shortestPath(m_baseStart.orientation);
		}

		if (m_atStart)
		{
			if (m_animate)
			{
				int mask = m_baseStartMask;
				m_currentPlacement = m_baseStart;

				mask |= applyStaticDisplacement(m_currentPlacement);

				if (getWidget())
					getWidget()->setPlacement(m_currentPlacement, mask);
			}
			else
			{
				// Ensure that teleported (non-animated) items start in the
				// correct place.
				// Especially important for field updates.
				int mask = m_baseEndMask;
				m_currentPlacement = m_baseEnd;

				mask |= applyStaticDisplacement(m_currentPlacement);

				if (getWidget())
					getWidget()->setPlacement(m_currentPlacement, mask);
			}
		}
		else if (m_atEnd || !m_animate)
		{
			int mask = m_baseEndMask;
			m_currentPlacement = m_baseEnd;

			mask |= applyStaticDisplacement(m_currentPlacement);

			if (getWidget())
				getWidget()->setPlacement(m_currentPlacement, mask);
		}

		if (!m_atEnd && m_animate)
		{
			// We need to regenerate the animation path
			m_basePath.setPathData(m_baseStart, m_baseEnd, m_baseStartMask | m_baseEndMask, ENdhsDecoratorTypeLinear);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::fieldDestroyed(NdhsCField* field)
{
	if (field == m_pStaticAnimField)
	{
		m_pStaticAnimField = NULL;
	}
	else
	{
		TmMFieldTokens::iterator fieldIt = m_fieldTokens.begin();

		while (fieldIt != m_fieldTokens.end())
		{
			if (field == *fieldIt)
			{
				fieldIt = m_fieldTokens.erase(fieldIt);
			}
			else
			{
				fieldIt++;
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::setTransitionBaseDisplacements(const LcTmArray<NdhsCTemplate::CDisplacement*>* pStart,
												const LcTmArray<NdhsCTemplate::CDisplacement*>* pEnd)
{
	unsubscribeToFields(NULL,false);
	unsubscribeToFields(NULL,true);
	subscribeToFields(pStart,false);
	subscribeToFields(pEnd,true);

	// Now we need to check for any initial displacement
	if (m_BaseStartDisplacementArray.size()!=0)
	{
		// Apply the displacement...note that this also adds in the displacement
		// mask to the base mask
		applyDisplacements(m_BaseStartDisplacementArray, m_baseStart, m_baseStartMask);
	}

	if (m_BaseEndDisplacementArray.size()!=0)
	{
		// Apply the displacement...note that this also adds in the displacement
		// mask to the base mask
		applyDisplacements(m_BaseEndDisplacementArray, m_baseEnd, m_baseEndMask);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::subscribeToFields(const LcTmArray<NdhsCTemplate::CDisplacement*>* pDisplacements, bool isBaseEnd)
{
	LcTmOwnerArray<NdhsCTemplate::CElementDisplacement> * placementArray = (isBaseEnd) ? (&m_BaseEndDisplacementArray) : (&m_BaseStartDisplacementArray);
	typedef LcTmOwnerArray<NdhsCTemplate::CElementDisplacement> TmExpressionMap;

	if (pDisplacements && !pDisplacements->empty())
	{
		LcTmArray<NdhsCTemplate::CDisplacement*>::const_iterator it = pDisplacements->begin();
		for (; it !=  pDisplacements->end(); it++)
		{
			NdhsCTemplate::CDisplacement *d = *it;
			if(d->value.length() > 0)
			{
				NdhsCExpression::CExprSkeleton *des = d->exprSkeleton.ptr();

				LcTaOwner<NdhsCExpression> displacementExpression = des->createExpression(m_page, getSlot(), m_menuItem);

				if(!displacementExpression)
					continue;

				LcTmOwner<NdhsCTemplate::CElementDisplacement> elementDisplacement = NdhsCTemplate::CElementDisplacement::create();
				displacementExpression->setObserver(elementDisplacement.ptr());
				elementDisplacement->expression=displacementExpression;
				elementDisplacement->elementRef = this;
				elementDisplacement->displacement = d;
				placementArray->push_back(elementDisplacement);
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::applyDisplacement(NdhsCTemplate::CElementDisplacement *displacement)
{
	// There are 3 cases here - either we're animating, in which case
	// the element position will be calculated shortly, or we're at the
	// start or end point of the last transition.  In the latter two cases
	// we need to update the widget placement otherwise the displacement will
	// not be seen.
	m_baseStart = m_baseStartBackup;
	m_baseEnd = m_baseEndBackup;
	m_baseStartMask = m_baseStartMaskBackup;
	m_baseEndMask = m_baseEndMaskBackup;

	applyDisplacements(m_BaseStartDisplacementArray, m_baseStart, m_baseStartMask,displacement);
	applyDisplacements(m_BaseEndDisplacementArray, m_baseEnd, m_baseEndMask,displacement);

	if (m_animate && ((m_baseStartMask | m_baseEndMask) & LcTPlacement::EOrientation))
	{
		// For base tweens, always set up the shortest rotation path
		m_baseEnd.orientation.shortestPath(m_baseStart.orientation);
	}

	if (m_atStart)
	{
		if (!m_transitioning || m_animate)
		{
			int mask = m_baseStartMask;
			m_currentPlacement = m_baseStart;

			mask |= applyStaticDisplacement(m_currentPlacement);

			if (getWidget())
				getWidget()->setPlacement(m_currentPlacement, mask);
		}
		else
		{
			// Ensure that teleported (non-animated) items start in the
			// correct place.
			// Especially important for field updates.
			int mask = m_baseEndMask;
			m_currentPlacement = m_baseEnd;

			mask |= applyStaticDisplacement(m_currentPlacement);

			if (getWidget())
				getWidget()->setPlacement(m_currentPlacement, mask);
		}
	}
	else if (m_atEnd || (m_transitioning && !m_animate))
	{
		int mask = m_baseEndMask;
		m_currentPlacement = m_baseEnd;

		mask |= applyStaticDisplacement(m_currentPlacement);

		if (getWidget())
			getWidget()->setPlacement(m_currentPlacement, mask);
	}

	if (!m_atEnd && m_animate)
	{
		// We need to regenerate the animation path
		m_basePath.setPathData(m_baseStart, m_baseEnd, m_baseStartMask | m_baseEndMask, ENdhsDecoratorTypeLinear);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::unsubscribeToFields(const LcTmArray<NdhsCTemplate::CDisplacement*>* pDisplacements, bool isBaseEnd)
{

	LcTmOwnerArray<NdhsCTemplate::CElementDisplacement> *placementArray = (isBaseEnd) ? (&m_BaseEndDisplacementArray) : (&m_BaseStartDisplacementArray);
	LcTmOwnerArray<NdhsCTemplate::CElementDisplacement>::iterator it = placementArray->begin();

	for(;it != placementArray->end(); it++)
	{
		(*it)->expression->removeObserver();
	}
	placementArray->clear();
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCElement::getField(const LcTmString& fieldName)
{
	NdhsCField* retVal = NULL;

	retVal = m_page->getFieldValue(fieldName,
									getMenu(),
									getLocalMenuItem() ? getLocalMenuItem()->getIndex() : -1,
#ifdef IFX_USE_PLUGIN_ELEMENTS
									this);
#else
									NULL);
#endif

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::setElementGroup(NdhsCElementGroup* pGroup)
{
	m_owningGroup = pGroup;

	int drawLayerIndex = getDrawLayerIndex();

	if ((getDrawLayerIndex() >= NDHS_UPPER_INVALID_DRAW_LAYER_INDEX) ||
		(getDrawLayerIndex() <= NDHS_LOWER_INVALID_DRAW_LAYER_INDEX))
	{
		if (getElementGroup())
		{
			drawLayerIndex = (getElementGroup())->getDrawLayerIndex();

			if ((getElementGroup()->getDrawLayerIndex() >= NDHS_UPPER_INVALID_DRAW_LAYER_INDEX) ||
				(getElementGroup()->getDrawLayerIndex() <= NDHS_LOWER_INVALID_DRAW_LAYER_INDEX))
			{
				drawLayerIndex = getPage()->getDrawLayerIndex();
				if ((getPage()->getDrawLayerIndex() >= NDHS_UPPER_INVALID_DRAW_LAYER_INDEX) ||
					(getPage()->getDrawLayerIndex() <= NDHS_LOWER_INVALID_DRAW_LAYER_INDEX))
				{
					drawLayerIndex = 0;
				}
			}
		}
	}

	if ((drawLayerIndex < NDHS_UPPER_INVALID_DRAW_LAYER_INDEX) &&
		(drawLayerIndex > NDHS_LOWER_INVALID_DRAW_LAYER_INDEX))
	{
		m_drawLayerIndex = drawLayerIndex;
	}

}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCElement::onMouseDown(const LcTPixelPoint& pt)
{
	setTouchdownStatus(true, true);
#ifdef LC_USE_MOUSEOVER
	setMouseoverStatus(false, true);
#endif
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCElement::onMouseMove(const LcTPixelPoint& pt)
{
	LcTVector vPoint = m_pageManager->getSpace()->mapCanvasToLocal(pt, *getWidget());

	if (m_pageManager->getMouseState() != NdhsCPageManager::EMouseUp &&  getPageManager()->getMouseState() != NdhsCPageManager::EMouseOver)
		setTouchdownStatus(getWidget()->contains(vPoint, getTapTolerance()), true);

#ifdef LC_USE_MOUSEOVER
	if (m_pageManager->getMouseState() == NdhsCPageManager::EMouseOver)
		return setMouseoverStatus(getWidget()->contains(vPoint, getTapTolerance()), true);
#endif
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCElement::onMouseUp(const LcTPixelPoint& pt)
{
	setTouchdownStatus(false, false);
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::onMouseCancel()
{
	setTouchdownStatus(false, true);
#ifdef LC_USE_MOUSEOVER
	setMouseoverStatus(false, true);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::setTouchdownStatus(bool touchdown, bool updateField)
{
	m_touchdownActive = touchdown;

	if (updateField)
	{
		if (touchdown)
		{
			m_page->updateTouchDownIntrinsicField(m_elementClassName, getSlot());
		}
		else
		{
			// NOTE Touchdown fields are NOT generally updated on release, as they
			// need to keep their value while the relevant onTap event is
			// processed. But some situations, like moving the mouse away, requires
			// the fields to be updated

			// If we were set as touch down element then we can clear _touchElement intrinsic, other
			// wise let the correct element try

			LcTaString touchDownElement = m_page->getTouchDownElement();
			if (touchDownElement.isEmpty() || touchDownElement.compareNoCase(m_elementClassName) == 0)
				m_page->updateTouchDownIntrinsicField("", -1);
		}
	}
}

#ifdef LC_USE_MOUSEOVER
/*-------------------------------------------------------------------------*//**
																			 */
bool NdhsCElement::setMouseoverStatus(bool mouseOver, bool updateField)
{
	if (updateField)
	{
		if (mouseOver)
		{
			getPageManager()->setMouseOverElement(this);
			m_page->updateMouseOverIntrinsicField(m_elementClassName, getSlot());
		}
		else
		{
			// Update field in case of moving mouse away
			m_page->updateMouseOverIntrinsicField("", -1);
		}
	}
	return false;
}
#endif
#endif //LC_USE_STYLUS

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::updateWidgetPlacement()
{
	if (m_startExtentFromWidget)
	{
		m_baseStart.extent = getDefaultExtent();
		m_baseStartBackup.extent = getDefaultExtent();
	}

	if (m_endExtentFromWidget)
	{
		m_baseEnd.extent = getDefaultExtent();
		m_baseEndBackup.extent = getDefaultExtent();
	}

	if (m_atStart)
	{
		if (getWidget())
			getWidget()->setPlacement(m_baseStart, m_baseStartMask);
		m_currentPlacement = m_baseStart;
	}
	else if (m_atEnd || !m_animate)
	{
		if (getWidget())
			getWidget()->setPlacement(m_baseEnd, m_baseEndMask);
		m_currentPlacement = m_baseEnd;
	}
	else
	{
		if (getWidget())
			getWidget()->setPlacement(m_currentPlacement, LcTPlacement::EAll & ~LcTPlacement::EExtent);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::setTransitionBasePlacements(LcTPlacement start,
										int startMask,
										bool startUsingLayoutExtent,
										LcTPlacement end,
										int endMask,
										bool endUsingLayoutExtent)
{
	m_baseStartBackup = start;
	m_baseStart = m_baseStartBackup;
	m_baseStartMask = m_baseStartMaskBackup = startMask;
	m_baseStartUsingLayoutExtent = startUsingLayoutExtent;

	m_baseEndBackup = end;
	m_baseEnd = m_baseEndBackup;

	m_baseEndMask = m_baseEndMaskBackup = endMask;
	m_baseEndUsingLayoutExtent = endUsingLayoutExtent;

	// Reset current placement to prevent any placement 'echo' in scrolling
	// elements
	if (startMask == 0 && endMask == 0 && getElementUse() == ENdhsObjectTypeItem)
	{
		m_currentPlacement = LcTPlacement();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::expressionDirty(NdhsCExpression* expr)
{
	m_needsReload     |= (expr == m_resourceData.ptr());
	m_needsFocusCheck |= (expr == m_enableFocus.ptr());

	if ((expr == m_resourceData.ptr() || expr == m_enableFocus.ptr()) && m_laundry)
	{
		m_laundry->addItem(this);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::expressionDestroyed(NdhsCExpression* expr)
{
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCElement::cleanLaundryItem(LcTTime timestamp)
{
	if (m_needsReload && !getUnloaded())
	{
		reloadElement();

		if (m_isRealized)
		{
			realize();
		}
	}

	if (m_needsFocusCheck && m_enableFocus)
	{
		m_enableFocus->evaluate();

		if (m_enableFocus->isError() || !m_enableFocus->isBool())
		{
			m_enableFocus->errorDiagnostics("enableFocus condition", true);
		}
		else
		{
			m_focusEnabled = m_enableFocus->getValueBool();
		}
	}

	m_needsReload     = false;
	m_needsFocusCheck = false;

	return false;
}

/*-------------------------------------------------------------------------*//**
*/
void  NdhsCElement::onSuspend()
{
	if (m_widget)
	{
		m_suspendWidgetExists = true;
		m_suspendWidgetRealized = m_widget->getAggregate()!=NULL;
		m_suspendPlacement = m_widget->getPlacement();
		m_suspendVisible = m_widget->getVisible();
		if(forceDestroyOnSuspend())
			destroyWidget();
		else
		{
			m_widget->releaseResources();
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
			m_widget->resetEffectMakesTranslucent();
#endif
		}
	}
}

void NdhsCElement::setUnloaded(bool value)
{
	if(m_unloaded!=value)
	{
		m_unloaded=value;
		if(!m_unloaded)
		{
			reloadElement();
			if(m_isRealized)
				realize();
		}
		else
		{
			if(m_widget)
				destroyWidget();
		}
	}

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::onResume()
{
	bool dontReloadElement=getUnloaded() || (m_suspendWidgetExists && m_widget);
	if(!dontReloadElement)
	{
		reloadElement();
	}

	if (m_suspendWidgetExists)
	{
		m_suspendWidgetExists = false;

		if(!m_widget)
			return;

		if(dontReloadElement)
		{
			m_widget->reloadResources();
		}
		else
		{
			m_widget->setPlacement(m_suspendPlacement, LcTPlacement::EAll);

			if (!m_suspendVisible)
			{
				m_widget->setVisible(false);
			}

			if (m_suspendWidgetRealized && !m_widget->getAggregate())
			{
				realize();
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCElement::trySetFocus()
{
	if (m_focusEnabled)
	{
		m_hasFocus = setElementFocus(true);
	}
	else
	{
		m_hasFocus = false;
	}

#ifdef NDHS_JNI_INTERFACE
	if (m_hasFocus)
		NDHS_TRACE(ENdhsTraceLevelInfo, ENdhsTraceModuleGeneral, "Focus change: focus now " + getFocusChain());
#endif

	return m_hasFocus;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElement::unsetFocus()
{
	m_hasFocus = false;
	setElementFocus(false);
}
