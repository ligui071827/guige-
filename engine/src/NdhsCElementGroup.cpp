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
#include "inflexionui/engine/inc/NdhsCScrollPosField.h"
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif

#define GROUP_MASK (LcTPlacement::EOffset | LcTPlacement::ELocation | LcTPlacement::EScale | LcTPlacement::EOrientation | LcTPlacement::EOpacity | LcTPlacement::EExtent)

/*-------------------------------------------------------------------------*//**
*/
NdhsCElementGroup::NdhsCElementGroup()
{
	// For groups, we create them as 'loaded' for efficiency
	m_isGroupUnloaded = false;

#ifdef LC_USE_LIGHTS
	m_lightModel	= ENdhsLightModelNormal;
#endif	
}

/*-------------------------------------------------------------------------*//**
Static factory method
*/
LC_EXPORT LcTaOwner<NdhsCElementGroup> NdhsCElementGroup::create(const LcTmString& name,
																	NdhsCElementGroup* page,
																	NdhsCMenu*		menu,
																	NdhsCMenuItem*	menuItem,
																	int				drawLayerIndex)
{
	LcTaOwner<NdhsCElementGroup> ref;
	ref.set(new NdhsCElementGroup(name, page, menu, menuItem, drawLayerIndex));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Constructor
*/
LC_EXPORT NdhsCElementGroup::NdhsCElementGroup(const LcTmString& name,
																	NdhsCElementGroup* page,
																	NdhsCMenu*		menu,
																	NdhsCMenuItem*	menuItem,
																	int				drawLayerIndex,
																	bool			teleportStateChange,
																	bool			teleportScroll)
{
	m_oldSlot   = -1;
	m_slot 	    = -1;

	m_menu = menu;
	m_item = menuItem;
	m_groupName = name;

	m_bTeleportStateChange	= teleportStateChange;
	m_bTeleportScroll		= teleportScroll;
	m_transitioning			= false;
	
	if (page)
		m_pageManager = page->getPageManager();
	
	// For groups, we create them as 'loaded' for efficiency
	m_isGroupUnloaded		= false;

	m_scheduledForDeletion = false;
	m_page = page;
	m_animate = false;
	m_drawLayerIndex = drawLayerIndex;

#ifdef LC_USE_LIGHTS
	m_lightModel			= ENdhsLightModelNormal;
#endif	

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	m_openGLRenderQualitySetting = "normal";
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCElementGroup::construct()
{
}

#ifdef IFX_SERIALIZATION

ISerializeable * NdhsCElementGroup::getSerializeAble(int &type)
{
	if(getMenuItem()!=NULL)
	{
		type=-1;
		return NULL;
	}
	type=1;
	return this;
}

NdhsCElementGroup* NdhsCElementGroup::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * ptr=serializeMaster->getOffset(handle);

	ENdhsCElementGroupType type=*((ENdhsCElementGroupType*)ptr);
	switch(type)
	{
		case ENdhsCElementGroupTypeBase:
		{
			NdhsCElementGroup * group=new NdhsCElementGroup();
			group->deSerialize(handle,serializeMaster);
			serializeMaster->setPointer(handle,group);
			return group;
		}

		case ENdhsCElementGroupTypePage:
		{
			return NdhsCPageModel::loadState(handle,serializeMaster);
		}

		case ENdhsCElementGroupTypeComponent:
		{
			return NdhsCComponent::loadState(handle,serializeMaster);
		}

		case ENdhsCElementGroupTypeMenuComponent:
		{
			return NdhsCMenuComponent::loadState(handle,serializeMaster);
		}
	}
	return NULL;
}
SerializeHandle	NdhsCElementGroup::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCElementGroup)+sizeof(IFX_INT32);
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;

	ENdhsCElementGroupType ty=ENdhsCElementGroupTypeBase;
	SERIALIZE(ty,serializeMaster,cPtr)
	SERIALIZE(m_bTeleportScroll,serializeMaster,cPtr)
	SERIALIZE(m_bTeleportStateChange,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_pageManager,serializeMaster,cPtr)
	SERIALIZE(m_menuIndex,serializeMaster,cPtr)
	SERIALIZE(m_oldSlot,serializeMaster,cPtr)
	SERIALIZE(m_slot,serializeMaster,cPtr)
	SERIALIZE(m_componentGroupType,serializeMaster,cPtr)
	SERIALIZE(m_groupType,serializeMaster,cPtr)
	SERIALIZE(m_scheduledForDeletion,serializeMaster,cPtr)
	SERIALIZE_Placement(m_baseEndBackup,serializeMaster,cPtr)
	SERIALIZE_Placement(m_baseStartBackup,serializeMaster,cPtr)
	SERIALIZE(m_animate,serializeMaster,cPtr)
	SERIALIZE(m_baseEndMask,serializeMaster,cPtr)
	SERIALIZE_Placement(m_baseEnd,serializeMaster,cPtr)
	SERIALIZE(m_baseStartMask,serializeMaster,cPtr)
	SERIALIZE_Placement(m_baseStart,serializeMaster,cPtr)
	SERIALIZE(m_atEnd,serializeMaster,cPtr)
	SERIALIZE(m_atStart,serializeMaster,cPtr)
	SERIALIZE_Placement(m_currentPlacement,serializeMaster,cPtr)
	SERIALIZE(m_baseEndMaskBackup,serializeMaster,cPtr)
	SERIALIZE(m_baseStartMaskBackup,serializeMaster,cPtr)
	SERIALIZE(m_drawLayerIndex,serializeMaster,cPtr)
	SERIALIZE(m_isGroupUnloaded,serializeMaster,cPtr)
#ifdef LC_USE_LIGHTS
	SERIALIZE(m_lightModel,serializeMaster,cPtr)
#endif
	SERIALIZE(m_hideGroup,serializeMaster,cPtr)
	SERIALIZE(m_makeVisible,serializeMaster,cPtr)
	SERIALIZE(m_bCurveForwards,serializeMaster,cPtr)
	SERIALIZE_String(m_groupName,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_page,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_owner,serializeMaster,cPtr)
	SERIALIZE_MapString(m_elements,serializeMaster,cPtr)
	SERIALIZE_MapString(m_subgroups,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_menu,serializeMaster,cPtr)

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	SERIALIZE_String(m_openGLRenderQualitySetting,serializeMaster,cPtr)
#endif

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void NdhsCElementGroup::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	ENdhsCElementGroupType ty=ENdhsCElementGroupTypePage;
	DESERIALIZE(ty,serializeMaster,cPtr)
	DESERIALIZE(m_bTeleportScroll,serializeMaster,cPtr)
	DESERIALIZE(m_bTeleportStateChange,serializeMaster,cPtr)
	DESERIALIZE_Reserve(m_pageManager,serializeMaster,cPtr,NdhsCPageManager)
	DESERIALIZE(m_menuIndex,serializeMaster,cPtr)
	DESERIALIZE(m_oldSlot,serializeMaster,cPtr)
	DESERIALIZE(m_slot,serializeMaster,cPtr)
	DESERIALIZE(m_componentGroupType,serializeMaster,cPtr)
	DESERIALIZE(m_groupType,serializeMaster,cPtr)
	DESERIALIZE(m_scheduledForDeletion,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_baseEndBackup,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_baseStartBackup,serializeMaster,cPtr)
	DESERIALIZE(m_animate,serializeMaster,cPtr)
	DESERIALIZE(m_baseEndMask,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_baseEnd,serializeMaster,cPtr)
	DESERIALIZE(m_baseStartMask,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_baseStart,serializeMaster,cPtr)
	DESERIALIZE(m_atEnd,serializeMaster,cPtr)
	DESERIALIZE(m_atStart,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_currentPlacement,serializeMaster,cPtr)
	DESERIALIZE(m_baseEndMaskBackup,serializeMaster,cPtr)
	DESERIALIZE(m_baseStartMaskBackup,serializeMaster,cPtr)
	DESERIALIZE(m_drawLayerIndex,serializeMaster,cPtr)
	DESERIALIZE(m_isGroupUnloaded,serializeMaster,cPtr)
#ifdef LC_USE_LIGHTS
	DESERIALIZE(m_lightModel,serializeMaster,cPtr)
#endif
	DESERIALIZE(m_hideGroup,serializeMaster,cPtr)
	DESERIALIZE(m_makeVisible,serializeMaster,cPtr)
	DESERIALIZE(m_bCurveForwards,serializeMaster,cPtr)
	DESERIALIZE_String(m_groupName,serializeMaster,cPtr)
	DESERIALIZE_Ptr(m_page,serializeMaster,cPtr,NdhsCElementGroup)
	DESERIALIZE_Ptr(m_owner,serializeMaster,cPtr,NdhsCElementGroup)

	LcTaString dummString="";
	DESERIALIZE_MapString(m_elements,serializeMaster,cPtr)
	DESERIALIZE_MapString(m_subgroups,serializeMaster,cPtr)
	DESERIALIZE_Reserve(m_menu,serializeMaster,cPtr,NdhsCMenu)
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	DESERIALIZE_String(m_openGLRenderQualitySetting,serializeMaster,cPtr)
#endif
}
#endif /* IFX_SERIALIZATION */

/*-------------------------------------------------------------------------*//**
	Destructor
*/
LC_EXPORT NdhsCElementGroup::~NdhsCElementGroup()
{
	if(m_pStaticAnimField)
	{
		m_pStaticAnimField->removeObserver(this);
	}

#ifdef IFX_USE_STYLUS
	if (m_pageManager)
	{
		m_pageManager->ignoreEntry(this);
	}
#endif

	// Destroy all the elements in the group.
	m_elements.clear();

	// Destroy all the subgroups
	m_subgroups.clear();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCElementGroup::setSlot(int newSlot)
{
	// Record the new slot
	m_oldSlot = m_slot;
	m_slot = newSlot;

	// Let child groups know
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->setSlot(newSlot);
	}
}

/*-------------------------------------------------------------------------*//**
 Realize event handler - realize all the members of the group
*/
LC_EXPORT void NdhsCElementGroup::onRealize()
{
	// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	if (isGroupUnloaded())
	{
		// Undo the realize on this group
		retire();
	}
	else
	{
		// Realize all the element widgets
		TmMElements::iterator it = m_elements.begin();

		for (; it != m_elements.end(); it++)
		{
			NdhsCElement* element = it->second;
			element->realize();
		}

		// Realize all the subgroups
		TmMGroups::iterator itGroups = m_subgroups.begin();

		for (; itGroups != m_subgroups.end(); itGroups++)
		{
			itGroups->second->realize(this);
		}
	}
}

/*-------------------------------------------------------------------------*//**
 Retire event handler - retire all the members of the group
*/
LC_EXPORT void NdhsCElementGroup::onRetire()
{
	// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	resetTransitionCache();
	stopStaticAnimations();

	// Retire all the elements
	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;
		element->retire();
	}

	// Retire all the subgroups
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->retire();
	}
}

/*-------------------------------------------------------------------------*//**
	Get combined animation mask for group
*/
int NdhsCElementGroup::getAnimateMask()
{
	int mask = 0;

	if (m_animate)
	{
		mask |= m_basePath.getMask();

		// Check if we've a curve to follow, or a decoration to perform
		if (m_pCurve)
			mask |= m_pCurve->getMask();

		if (m_pDecorator)
			mask |= m_pDecorator->getMask();
	}

	if (m_owner)
		mask |= m_owner->getAnimateMask();

	return mask;
}

/*-------------------------------------------------------------------------*//**
	Work out the combined static animation mask
*/
int NdhsCElementGroup::getStaticAnimateMask()
{
	int mask = 0;

	if (m_pStaticAnim)
		mask |= m_pStaticAnim->getMask();

	if (m_owner)
		mask |= m_owner->getStaticAnimateMask();

	return mask;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::updatePosition(ENdhsAnimationType animType,
										LcTScalar position,
										bool positionIncreasing,
										bool updateCache,
										bool forceNonSketchy,
										bool aggregateAnimating,
										bool finalFrame)
{
	int mask = 0;

	// If no children, don't bother to continue
	// Exception for components: the children aren't owned
	if ((m_groupType != ENdhsObjectTypeFurnitureComponent && m_groupType != ENdhsObjectTypeItemComponent) && (m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

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
		m_page->getTransitionAgent()->updateAnimationCache(this, animType, positionIncreasing);

		// Check to see if we should be visible, etc
		m_animate = updateGroupStatus(animType, finalFrame);

		if (!m_animate && m_pDecorator)
			m_animate = true;

		// For base tweens, always set up the shortest rotation path
		m_baseEnd.orientation.shortestPath(m_baseStart.orientation);

		// Now work out the 'delta' placement between start and end points
		m_basePath.setPathData(m_baseStart, m_baseEnd, GROUP_MASK, ENdhsDecoratorTypeLinear);

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
		}

		// When we're flick scrolling or dragging quickly, there's a chance that some effect
		// of the previous animation segment is left on the widget placement, so we 'initialize'
		// the placement each time we update the cache with the nearest base placement.
		if ((position <= .5) && (m_baseStartMask != 0))
			m_currentPlacement = m_baseStart;
		else if ((position > .5) && (m_baseEndMask != 0))
			m_currentPlacement = m_baseEnd;

		LcTPlacement currentPlacement = m_currentPlacement;

		(void)applyStaticDisplacement(currentPlacement);

		setPlacement(currentPlacement, GROUP_MASK);
	}

	if (finalFrame || m_animate)
	{
		// Check to see if we should be visible, etc
		updateGroupStatus(animType, finalFrame);

		// Reset cached value
		m_currentPlacement = m_baseStart;

		mask = calculatePosition(position, m_currentPlacement);

		if (finalFrame)
		{
			if (m_animate)
			{
				// Element groups that have animated need to be revalidated, as the set
				// placement will not 'dirty' the agg.
				revalidate();
			}
			else
			{
				// Groups that have teleported need to check that the final position
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

						// Check visibility
						if (isVisible() == false)
							setVisible(true);
					}
					else
					{
						// If we are here then start mask backup is '0' if we have teleported 
						// we must have end mask backup not '0'
						if (isVisible() && m_baseEndMaskBackup != 0)
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

						// Check visibility
						if (isVisible() == false)
							setVisible(true);
					}
				}

				// Cater for the case where the group is being statically animated
				// (i.e. it's a furniture group, and the animation is a scroll
				mask |= applyStaticDisplacement(m_currentPlacement);
			}

			// Clear any stored animation data, otherwise we'll claim that we're animating when we may not
			// be, and make children go sketchy when they shouldn't
			m_basePath.clearPath();
			m_pCurve = NULL;
			m_pDecorator = NULL;
		}

		// Now update the group position
		setPlacement(m_currentPlacement, mask);
	}

	// Let the elements respond,
	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;
		element->updatePosition(animType, position, positionIncreasing, updateCache,
								forceNonSketchy, m_animate || aggregateAnimating, finalFrame);
	}

	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->updatePosition(animType, position, positionIncreasing, updateCache,
								forceNonSketchy, m_animate || aggregateAnimating, finalFrame);
	}
}

/*-------------------------------------------------------------------------*//**
	Calculate current placement.
*/
int NdhsCElementGroup::calculatePosition(LcTScalar position, LcTPlacement& placement)
{
	int mask = 0;

	LcTColor white(255, 255, 255, 0);
	LcTColor gray20(LcTColor::GRAY20);

	// Now position the group
	// Interpolate
	m_basePath.getPlacement(position, placement);
	mask = m_basePath.getMask();

	// Is there a curve?
	if (m_pCurve)
	{
		LcTPlacement curveOffset;
		int curveMask;
		if (m_bCurveForwards)
			curveMask = m_pCurve->getPlacement(position, curveOffset, white, white, gray20, gray20);
		else
			curveMask = m_pCurve->getPlacement((1-position), curveOffset, white, white, gray20, gray20);

		// Limit the curve to permissible mask values
		curveMask &= GROUP_MASK;

		placement.offset(curveOffset, curveMask);
		mask |= curveMask;
	}

	// Apply a decoration?
	if (m_pDecorator)
	{
		LcTPlacement decoration;
		int decMask = m_pDecorator->getPlacement(position, decoration, white, white, gray20, gray20);

		// Limit the decoration to permissible mask values
		decMask &= GROUP_MASK;

		placement.offset(decoration, decMask);
		mask |= decMask;
	}

	return mask;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::applyDisplacements(const LcTmArray<NdhsCTemplate::CElementDisplacement*>& pDisplacements,
												LcTPlacement& placement,
												int& placementMask,NdhsCTemplate::CElementDisplacement* currentDisplacement)
{
	typedef NdhsCTemplate::CDisplacement CDisplacement;
	typedef NdhsCTemplate::CElementDisplacement CElementDisplacement;
	LcTColor origColor = placement.color;
	LcTColor origColor2 = placement.color2;

	if (pDisplacements.size() != 0)
	{
		CElementDisplacement * disp=NULL;
		LcTmArray<CElementDisplacement*>::const_iterator it = pDisplacements.begin();
		for (; it !=  pDisplacements.end(); it++)
		{
			disp = *it;
			if (disp->displacement->value.isEmpty())
				continue;

			// Get the named displacement
			const NdhsCKeyFrameList* pDisplacementKFL = NULL;

			if(currentDisplacement == disp)
				pDisplacementKFL = m_page->getTemplate()->getAnimation(disp->displacement->animation);

			if (pDisplacementKFL)
			{
				LcTScalar frac = disp->displacement->getNormalizedValue(disp->expression.ptr());

				// Get placement
				LcTPlacement displacement;
				int displacementMask = pDisplacementKFL->getPlacement(frac, displacement, origColor, origColor, origColor2, origColor2);

				// Limit the animation to permissible mask values
				displacementMask &= GROUP_MASK;

				// Apply to base placement
				disp->cachedPlacement=displacement;
				placement.offset(displacement, displacementMask);

				// Note the displacement mask
				placementMask |= displacementMask;

				disp->cachedMask=displacementMask;

#ifdef LC_USE_MOUSEOVER
				if (frac == 1)
					getPageManager()->displacementApplied();
#endif
			}
			else if(currentDisplacement!=NULL)
			{
				// Apply to base placement
				placement.offset(disp->cachedPlacement, disp->cachedMask);
				placementMask |= disp->cachedMask;
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Apply displacements.
*/
void NdhsCElementGroup::applyDisplacements(const LcTmArray<NdhsCTemplate::CElementDisplacement*>& pDisplacements,
												LcTPlacement& placement,
												int& placementMask)
{
	typedef NdhsCTemplate::CDisplacement CDisplacement;
	typedef NdhsCTemplate::CElementDisplacement CElementDisplacement;
	LcTColor origColor = placement.color;
	LcTColor origColor2 = placement.color2;

	if (pDisplacements.size() != 0)
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
				int displacementMask = pDisplacementKFL->getPlacement(frac, displacement, origColor, origColor, origColor2, origColor2);

				// Limit the animation to permissible mask values
				displacementMask &= GROUP_MASK;

				// Apply to base placement
				disp->cachedPlacement=displacement;
				placement.offset(displacement, displacementMask);

				// Note the displacement mask
				placementMask |= displacementMask;

				disp->cachedMask=displacementMask;

			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Check whether we need to change the visibility status of the group
*/
bool NdhsCElementGroup::updateGroupStatus(ENdhsAnimationType animType, bool finalFrame)
{
	bool canAnimate = false;
	bool forceHide = false;

	// If true, group is set to hide in theme.
	if (getHideGroupFlag() == true)
	{
		setVisible(false);
		return canAnimate;
	}

	// If we're in a menu slot and are unplaced, it's an easy decision
	if (m_groupType == ENdhsObjectTypeItem && (getOldSlot() == -1 && getSlot() == -1))
	{
		setVisible(false);
		m_baseStartMaskBackup = m_baseEndMaskBackup = 0;
		m_baseStartMask = m_baseEndMask = 0;
		return canAnimate;
	}

	// Now check whether we should be teleporting, etc
	switch(animType)
	{
		case ENdhsAnimationTypeInteractiveState:
		case ENdhsAnimationTypeTerminalState:
		// State change - we are moving elements between the old and new layouts
		{
			m_makeVisible = false;

			if(m_baseStartMask != 0 && m_baseEndMask != 0)
			{
				// Both start and end placements for this element exist
				if(!m_bTeleportStateChange)
					canAnimate = true;
			}
			else
			{
				// Teleporting
				forceHide = true;
			}

			break;
		}

		case ENdhsAnimationTypeScroll:
		// Moving menu items from slot to slot.
		{
			if (m_groupType == ENdhsObjectTypeItem || m_groupType == ENdhsObjectTypeItemComponent)
			{
				m_makeVisible = false;

				// Get start and end slot values
				int startSlot = getOldSlot();
				int endSlot = getSlot();

				// We have 4 cases...
				if (startSlot == -1 && endSlot == -1)
				{
					// make sure we're hidden
					forceHide = true;
				}
				else if (startSlot == -1 && endSlot != -1)
				{
					// Teleporting
					forceHide = true;
				}
				else if(startSlot != -1 && endSlot == -1)
				{
					// Teleporting
					forceHide = true;
				}
				else if(startSlot != -1 && endSlot != -1)
				{
					if((m_groupType!=ENdhsObjectTypeFurnitureComponent && m_groupType!=ENdhsObjectTypeItemComponent) ||(m_baseStartMask!=0 && m_baseEndMask!=0))
					{
						if(!m_bTeleportScroll)
							canAnimate = true;
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

					if (canAnimate == false)
					{
						forceHide = true;
					}
				}
			}
			else
			{
				// Non-item groups will not animate.  Set the start placements to the
				// end placements so any prior state transition is forgotten about
				m_baseStartBackup = m_baseEndBackup;
				m_baseStartMaskBackup = m_baseEndMaskBackup;
				m_baseStart = m_baseEnd;
				m_baseStartMask = m_baseEndMask;
			}

			break;
		}

		default:
			break;
	}

	// Set visibility if needed
	// If we're at the end of this part, there's an end placement
	// and we're not visible, show ourselves
	if (finalFrame)
	{
		if (m_makeVisible)
			setVisible(true);
	}

	if (canAnimate)
	{
		setVisible(true);
		m_makeVisible = true;
	}
	else
	{
		// Teleporting, so make end and start placements the same
		if (m_baseEndMask != 0)
		{
			m_baseStart = m_baseEnd;
			m_baseStartMask = m_baseEndMask;

			// Set 'to be made visible' true
			m_makeVisible = true;
		}

		if (forceHide && !finalFrame)
		{
			setVisible(false);
		}
	}

	return canAnimate;
}

/*-------------------------------------------------------------------------*//**
	Reset cached data pointing into a transition agent layout
*/
void NdhsCElementGroup::resetTransitionCache()
{
	// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	unsubscribeToFields(NULL,false);
	unsubscribeToFields(NULL,true);
	m_pCurve = NULL;

	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;
		element->resetTransitionCache();
	}

	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->resetTransitionCache();
	}
}

/*-------------------------------------------------------------------------*//**
	Identifies class name of element associated with a particular widget
*/
LC_EXPORT LcTaString NdhsCElementGroup::getClassFromWidget(LcCWidget* widget)
{
	LcTaString retVal = "";

	// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return retVal;

	// Iterate through our map of elements
	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;

		switch (element->getElementType())
		{
			case ENdhsElementTypeGraphic:
			{
				NdhsCGraphicElement* pGE = reinterpret_cast<NdhsCGraphicElement*> (element);
				LC_ASSERT(pGE);

				if(pGE->getWidget() == widget)
					return pGE->getElementClassName();

				break;
			}

#ifdef LC_USE_STYLUS
			case ENdhsElementTypeDragRegion:
			{
				NdhsCDragRegionElement* pGE = reinterpret_cast<NdhsCDragRegionElement*> (element);
				LC_ASSERT(pGE);

				if(pGE->getWidget() == widget)
					return pGE->getElementClassName();

				break;
			}
#endif

			case ENdhsElementTypeText:
			{
				NdhsCTextElement* pTE = reinterpret_cast<NdhsCTextElement*> (element);
				LC_ASSERT(pTE);

				if(pTE->getWidget() == widget)
					return pTE->getElementClassName();

				break;
			}
#ifdef IFX_USE_PLUGIN_ELEMENTS
			case ENdhsElementTypePlugin:
			{
				NdhsCPluginElement* pTE = reinterpret_cast<NdhsCPluginElement*> (element);
				LC_ASSERT(pTE);

				if(pTE->getWidget() == widget)
					return pTE->getElementClassName();

				break;
			}
#endif
			default:
				break;
		}
	}

	// Check subgroups
	TmMGroups::iterator itGroup = m_subgroups.begin();

	for (; itGroup != m_subgroups.end() && retVal.isEmpty(); itGroup++)
	{
		retVal = itGroup->second->getClassFromWidget(widget);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT NdhsCElementGroup* NdhsCElementGroup::getGroup(const LcTmString& groupName)
{
	NdhsCElementGroup* retVal = NULL;

	// If no children, don't bother to continue
	if (m_subgroups.size() == 0)
		return retVal;

	// Check Groups

	TmMGroups::iterator it = m_subgroups.find(groupName);
	if(it != m_subgroups.end())
	{
		retVal = it->second;
	}

	it = m_subgroups.begin();

	for(; it != m_subgroups.end() && retVal == NULL; it++)
	{
		retVal = it->second->getGroup(groupName);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Gets the element given the element class name
*/
LC_EXPORT NdhsCElement* NdhsCElementGroup::getItem(const LcTmString& elementClass)
{
	NdhsCElement* retVal = NULL;

	// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return retVal;

	// Check elements
	TmMElements::iterator it = m_elements.find(elementClass);
	retVal = (it == m_elements.end()) ? NULL : it->second;

	// Check subgroups
	TmMGroups::iterator itGroup = m_subgroups.begin();

	for (; itGroup != m_subgroups.end() && (retVal == NULL); itGroup++)
	{
		retVal = itGroup->second->getItem(elementClass);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Insert an element into the group, using the class name as a unique key
*/
LC_EXPORT bool NdhsCElementGroup::addItem(const LcTmString& elementClass, LcTmOwner<NdhsCElement>& element)
{
	bool retVal = false;

	if( element.ptr() != NULL && (m_elements.find(elementClass) == m_elements.end()) )
	{
		//doesn't already exist, insert
		element->setElementGroup(this);
		m_elements.add_element(elementClass, element);
		retVal = true;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Remove and destroy an element from the group using the class name
*/
LC_EXPORT void NdhsCElementGroup::removeItem(LcTmString& elementClass)
{
	TmMElements::iterator it = m_elements.find(elementClass);
	if (it != m_elements.end())
	{
		NdhsCElement* element = it->second;
		delete element;
		m_elements.erase(it);
	}
}

/*-------------------------------------------------------------------------*//**
	Insert a sub-group into the group, using the group name as a unique key
*/
LC_EXPORT bool NdhsCElementGroup::addGroup(const LcTmString& groupName, LcTmOwner<NdhsCElementGroup>& group)
{
	bool retVal = false;

	if( group.ptr() != NULL && (m_subgroups.find(groupName) == m_subgroups.end()) )
	{
		// Doesn't already exist, insert
		group->setElementGroup(this);
		m_subgroups.add_element(groupName, group);
		retVal = true;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Removes a sub-group from the group, using the group name as a unique key
*/
LC_EXPORT bool NdhsCElementGroup::removeGroup(const LcTmString& groupName)
{
	bool retVal = false;

	TmMGroups::iterator it = m_subgroups.find(groupName);
	if (it != m_subgroups.end())
	{
		m_subgroups.erase(it);
		retVal = true;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
forward aggregate events to the element widgets
*/
LC_EXPORT void NdhsCElementGroup::onAggregateEvent(LcCWidget::TAggregateEvent* e)
{
	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;

		if(element->getElementType() == ENdhsElementTypeGraphic)
		{
			NdhsCGraphicElement* pGE = reinterpret_cast<NdhsCGraphicElement*> (element);
			LC_ASSERT(pGE);

			fireAggregateEvent(pGE->getWidget(), e);
		}
		else if(element->getElementType() == ENdhsElementTypeText)
		{
			NdhsCTextElement* pTE = reinterpret_cast<NdhsCTextElement*> (element);
			LC_ASSERT(pTE);

			fireAggregateEvent(pTE->getWidget(), e);
		}
	}
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCElementGroup::populateElementList(	NdhsCPageManager::TmAWidgets& widgets,
														NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList)
{
	// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;
		if (element)
		{
			element->populateElementList(widgets, pageWidgetElemList);
		}
	}

	// Now the subgroups
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->populateElementList(widgets, pageWidgetElemList);
	}
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::startStaticAnimations()
{
	// If no children, don't bother to continue
	if ((m_groupType != ENdhsObjectTypeFurnitureComponent && m_groupType != ENdhsObjectTypeItemComponent) && (m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	// Query transition agent for any static animations
	m_page->getTransitionAgent()->getStaticAnimation(this);

	// Now check the elements
	TmMElements::iterator it = m_elements.begin();
	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;

		element->startStaticAnimations();
	}

	// Now the subgroups
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->startStaticAnimations();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::stopStaticAnimations()
{
	// If no children, don't bother to continue
	if ((m_groupType != ENdhsObjectTypeFurnitureComponent && m_groupType != ENdhsObjectTypeItemComponent) && (m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	if (m_pStaticAnim)
	{
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

		setPlacement(m_currentPlacement, GROUP_MASK);

#ifdef IFX_GENERATE_SCRIPTS
	if (NdhsCScriptGenerator::getInstance())
		NdhsCScriptGenerator::getInstance()->setAnimationStops();
#endif //IFX_GENERATE_SCRIPTS

	}

	// Now check the elements
	TmMElements::iterator it = m_elements.begin();
	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;

		if(element)
		{
			element->stopStaticAnimations();
		}
	}

	// Now the subgroups
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->stopStaticAnimations();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::setStaticAnimation(NdhsCKeyFrameList* pDec,
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
int NdhsCElementGroup::applyStaticDisplacement(LcTPlacement& placement)
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
	LcTPlacement delta;
	int mask = m_pStaticAnim->getPlacement(val, delta, m_baseStart.color, m_baseEnd.color, m_baseStart.color2, m_baseEnd.color2);

	if ((val >= 0) && (val <= 1))
	{
		placement.offset(delta, mask & GROUP_MASK);
	}

	return mask;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::onWidgetEvent(LcTWidgetEvent* e)
{
	// Let the elements respond,
	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;
		if (element->onWidgetEvent(e))
			return;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::fieldValueUpdated(NdhsCField* field)
{
	if (field == m_pStaticAnimField && m_pStaticAnim)
	{
		LcTScalar val = m_pStaticAnimField->getRawFieldData(NULL);
		LcTPlacement placement;
		LcTPlacement offset;

		// Check for wrapping
		val = val - (int)val;

		// Check for endpoint of static animation...
		if (m_pStaticAnimField->atRest())
			val = 1.0;

		int mask = m_pStaticAnim->getPlacement(val, offset, 0, 0, 0, 0);

		// Limit the animation to permissible mask values
		mask &= GROUP_MASK;

		// If we're not animating, then we need to reset currentPlacement
		// each time or else the static animation would have a cumulative
		// effect
		if (m_atStart)
			placement = m_baseStart;
		else if (m_atEnd)
			placement = m_baseEnd;
		else
			placement = m_currentPlacement;

		if ((val > 0) && (val < 1))
			placement.offset(offset, mask);

		setPlacement(placement, mask);

		if (m_pStaticAnimField->atRest())
			m_pStaticAnim = NULL;
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

				setPlacement(m_currentPlacement, mask);
			}
			else
			{
				// Ensure that teleported (non-animated) items start in the
				// correct place.
				// Especially important for field updates.
				int mask = m_baseEndMask;
				m_currentPlacement = m_baseEnd;

				mask |= applyStaticDisplacement(m_currentPlacement);

				setPlacement(m_currentPlacement, mask);
			}
		}
		else if (m_atEnd || !m_animate)
		{
			int mask = m_baseEndMask;
			m_currentPlacement = m_baseEnd;

			mask |= applyStaticDisplacement(m_currentPlacement);

			setPlacement(m_currentPlacement, mask);
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
void NdhsCElementGroup::fieldDestroyed(NdhsCField* field)
{
	if (field == m_pStaticAnimField)
	{
		m_pStaticAnimField = NULL;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::applyDisplacement(NdhsCTemplate::CElementDisplacement *displacement)
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

			setPlacement(m_currentPlacement, mask);
		}
		else
		{
			// Ensure that teleported (non-animated) items start in the
			// correct place.
			// Especially important for field updates.
			int mask = m_baseEndMask;
			m_currentPlacement = m_baseEnd;

			mask |= applyStaticDisplacement(m_currentPlacement);

			setPlacement(m_currentPlacement, mask);
		}
	}
	else if (m_atEnd || (m_transitioning && !m_animate))
	{
		int mask = m_baseEndMask;
		m_currentPlacement = m_baseEnd;

		mask |= applyStaticDisplacement(m_currentPlacement);

		setPlacement(m_currentPlacement, mask);
	}

	if (!m_atEnd && m_animate)
	{
		// We need to regenerate the animation path
		m_basePath.setPathData(m_baseStart, m_baseEnd, m_baseStartMask | m_baseEndMask, ENdhsDecoratorTypeLinear);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::setTransitionBaseDisplacements(const LcTmArray<NdhsCTemplate::CDisplacement*>* pStart,
												const LcTmArray<NdhsCTemplate::CDisplacement*>* pEnd)
{


	unsubscribeToFields(NULL,false);
	unsubscribeToFields(NULL,true);
	subscribeToFields(pStart,false);
	subscribeToFields(pEnd,true);

	// Check for initial displacement
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
void NdhsCElementGroup::subscribeToFields(const LcTmArray<NdhsCTemplate::CDisplacement*>* pDisplacements, bool isBaseEnd)
{
	LcTmOwnerArray<NdhsCTemplate::CElementDisplacement> *placementArray = (isBaseEnd) ? (&m_BaseEndDisplacementArray) : (&m_BaseStartDisplacementArray);
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

				LcTaOwner<NdhsCExpression> displacementExpression = des->createExpression(m_page, getSlot(), m_item);
				if(!displacementExpression)
					continue;

				LcTmOwner<NdhsCTemplate::CElementDisplacement> elementDisplacement = NdhsCTemplate::CElementDisplacement::create();
				displacementExpression->setObserver(elementDisplacement.ptr());
				elementDisplacement->expression = displacementExpression;
				elementDisplacement->elementRef = this;
				elementDisplacement->displacement = d;
				placementArray->push_back(elementDisplacement);
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::unsubscribeToFields(const LcTmArray<NdhsCTemplate::CDisplacement*>* pDisplacements, bool isBaseEnd)
{
	LcTmOwnerArray<NdhsCTemplate::CElementDisplacement> * placementArray = (isBaseEnd) ? (&m_BaseEndDisplacementArray) : (&m_BaseStartDisplacementArray);
	LcTmOwnerArray<NdhsCTemplate::CElementDisplacement>::iterator it = placementArray->begin();

	for(;it != placementArray->end(); it++)
	{
		(*it)->expression->removeObserver();
	}

	placementArray->clear();
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCElementGroup::getField(const LcTmString& field, int slotNum, NdhsCMenuItem* item)
{
	NdhsCField* retVal = NULL;

	if (m_owner)
	{
		retVal = m_owner->getField(field, slotNum, item);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCElementGroup::getPageParamValue(const LcTmString& key)
{
	NdhsCField* retVal = NULL;

	if (m_owner)
	{
		retVal = m_owner->getPageParamValue(key);
	}

	return retVal;
}

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

LC_EXPORT LcTmString NdhsCElementGroup::getOpenGLRenderQualitySetting()
{
	// If the "high" setting is enabled for this group simply return
	// Or the parent group is not there, return as well
	if ( (m_openGLRenderQualitySetting.compare ("high") == 0) || (!m_owner) )
	{
		return m_openGLRenderQualitySetting;
	}
	// Retreive setting from this group's parent
	else
	{
		return m_owner->getOpenGLRenderQualitySetting();
	}
}

#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

/*-------------------------------------------------------------------------*//**
*/
int NdhsCElementGroup::getDrawLayerIndex()
{
	if (m_drawLayerIndex == NDHS_LOWER_INVALID_DRAW_LAYER_INDEX)
	{
		if (m_owner)
		{
			return	m_owner->getDrawLayerIndex();
		}
	}

	return  m_drawLayerIndex;
}
/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::onSuspend()
{
		// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	// apply on all the elements
	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;
		element->onSuspend();
	}

	// Let child groups know
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->onSuspend();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::onResume()
{
	// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	// Let the elements respond,
	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;
		element->onResume();
	}

	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->onResume();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::loadResources()
{
	// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	// Let the elements respond,
	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;
		element->setUnloaded(false);
	}

	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->loadResources();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::UnloadResources()
{
	// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	// Let the elements respond,
	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;
		element->setUnloaded(true);
	}

	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->UnloadResources();
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCComponent* NdhsCElementGroup::findParentComponent()
{
	if (m_page)
	{
		return m_page->findParentComponent();
	}
	else
	{
		return NULL;
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageModel* NdhsCElementGroup::findParentPageModel()
{
	if (m_page)
	{
		return m_page->findParentPageModel();
	}
	else
	{
		return NULL;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::focusSetOnChild(NdhsCComponent* childComponent, NdhsCComponent* focusComponent)
{
	if (m_page)
	{
		m_page->focusSetOnChild(childComponent, focusComponent);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::unsetFocus()
{
	if (m_page)
	{
		m_page->unsetFocus();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::globalUnsetFocus()
{
	NdhsCPageModel* pageModel = findParentPageModel();

	if (pageModel)
	{
		pageModel->pageWideUnsetFocus();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::setVisible(bool b)
{
	LcCAggregate::setVisible(b);

	if (!b)
	{
		doCheckVisibilityChanges();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::doCheckVisibilityChanges()
{
	if (!m_makeVisible)
	{
		// Notify component or page model of visibility changes
		NdhsCComponent* parentComponent = findParentComponent();

		if (parentComponent)
		{
			parentComponent->checkVisibilityChanges();
		}
		else
		{
			NdhsCPageModel* parentPageModel = findParentPageModel();

			if (parentPageModel)
			{
				parentPageModel->checkVisibilityChanges();
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCElementGroup::componentDoPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame)
{
	bool reschedule = false;

	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		if (itGroups->second && itGroups->second->componentDoPrepareForFrameUpdate(timestamp, finalFrame))
			reschedule = true;
	}

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::componentsJumpTransitionToEnd(bool setIdle)
{
	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->componentsJumpTransitionToEnd(setIdle);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::resumeStaticAnimations()
{
	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->resumeStaticAnimations();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::componentRefreshField(const LcTmString& field, int item)
{
	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->componentRefreshField(field, item);
	}
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCElementGroup::getHMenuActiveItemIndex(IFX_HMENU hMenu)
{
	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		int index = itGroups->second->getHMenuActiveItemIndex(hMenu);
		if (index != -1)
			return index;
	}
	return -1;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCElementGroup::getFullFilePath(	IFX_HMENU hMenu,
											const LcTmString& searchFile,
											LcTmString& returnFilePath,
											int menuIndex)
{
	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		if (itGroups->second->getFullFilePath(hMenu, searchFile, returnFilePath, menuIndex))
			return true;
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCElementGroup::isComponentTransitioning()
{
	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		if (itGroups->second->isComponentTransitioning())
			return true;
	}
	return false;
}

#if defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::componentOnTransitionComplete(bool setIdle)
{
	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->componentOnTransitionComplete(setIdle);
	}
}

/*-------------------------------------------------------------------------*//**
*/
unsigned int NdhsCElementGroup::distanceFromPage()
{
	if (m_page)
	{
		return m_page->distanceFromPage();
	}
	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
unsigned int NdhsCElementGroup::getIdentifier()
{
	if (m_page)
	{
		return m_page->getIdentifier();
	}
	return 0;
}

#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::loadGroup()
{
	// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	// Let the elements respond,
	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;
		element->setUnloaded(false);
	}

	// Reset group unload flag before loading group
	m_isGroupUnloaded = false;

	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->loadGroup();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::unLoadGroup()
{
	// If no children, don't bother to continue
	if ((m_subgroups.size() == 0) && (m_elements.size() == 0))
		return;

	// Let the elements respond,
	TmMElements::iterator it = m_elements.begin();

	for (; it != m_elements.end(); it++)
	{
		NdhsCElement* element = it->second;
		element->setUnloaded(true);
	}

	// Let the subgroups respond,
	TmMGroups::iterator itGroups = m_subgroups.begin();

	for (; itGroups != m_subgroups.end(); itGroups++)
	{
		itGroups->second->unLoadGroup();
	}

	// Set group unload flag after unloading group
	m_isGroupUnloaded = true;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageModel*	NdhsCElementGroup::getParentPageRef()
{
	if (m_page)
		return m_page->getParentPageRef();
	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCElementGroup::getTouchDownElement()
{ 
	if (m_page)
		return m_page->getTouchDownElement();
	return ""; 
}

#ifdef LC_USE_MOUSEOVER
/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::updateMouseOverIntrinsicField(const LcTmString& componentName, int slot)
{
	if (m_page)
		m_page->updateMouseOverIntrinsicField(componentName, slot);
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCElementGroup::setTransitionBasePlacements(LcTPlacement start, int startMask, LcTPlacement end, int endMask)
{
	m_baseStart = start;
	m_baseStartBackup = start;
	m_baseStartMask = m_baseStartMaskBackup = startMask;
	m_baseEnd = end;
	m_baseEndBackup = end;
	m_baseEndMask = m_baseEndMaskBackup = endMask;

	// Reset current placement to prevent any placement 'echo' in scrolling
	// groups
	if (startMask == 0 && endMask == 0 && getGroupType() == ENdhsObjectTypeItem)
	{
		m_currentPlacement = LcTPlacement();
	}
}
