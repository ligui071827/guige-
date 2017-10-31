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
LcTaOwner<NdhsCTransitionAgent> NdhsCTransitionAgent::create(NdhsCTemplate* pTemplate,
															NdhsCElementGroup*	pModel)
{
	LcTaOwner<NdhsCTransitionAgent> ref;
	ref.set(new NdhsCTransitionAgent(pTemplate, pModel));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Constructor
*/
NdhsCTransitionAgent::NdhsCTransitionAgent(NdhsCTemplate* pTemplate,
															NdhsCElementGroup*	pModel)
{
	m_template = pTemplate;
	m_model = pModel;
	m_ovveridenVelocityProfile=ENdhsVelocityProfileUnknown;
	m_ovveridenDuration=-1;
	m_ovveridenDelay=-1;
}

/*-------------------------------------------------------------------------*//**
	2nd phase construction
*/
void NdhsCTransitionAgent::construct()
{
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
NdhsCTransitionAgent::~NdhsCTransitionAgent()
{
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTransitionAgent::prepareTransition(NdhsCTemplate::CLayout* newLayout,
												int initialSlotActive,
												int finalSlotActive)
{
	// Back up old layout
	m_oldLayout.destroy();
	m_oldLayout = m_layout;

	// Store new layout
	m_layout.destroy();
	m_layout = m_template->getMergedLayout(newLayout);

	m_initialSlotActive = initialSlotActive;
	m_finalSlotActive = finalSlotActive;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTransitionAgent::ovverideTransitionDetails(ENdhsVelocityProfile velocityProfile,
													int duration,
													int	delay)
{
	if(velocityProfile != ENdhsVelocityProfileUnknown)
		m_ovveridenVelocityProfile = velocityProfile;

	if(duration != -1)
		m_ovveridenDuration = duration;

	if(delay != -1)
		m_ovveridenDelay = delay;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTransitionAgent::resetOvveridenTransitionDetails()
{
	m_ovveridenVelocityProfile = ENdhsVelocityProfileUnknown;
	m_ovveridenDuration = -1;
	m_ovveridenDelay = -1;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTransitionAgent::getPageAnimationDetails(NdhsCElementGroup* pageToPosition,
													ENdhsAnimationType animType,
													int& duration,
													int& delay,
													ENdhsVelocityProfile& profile,
													int& backgroundDelay,
													int& primaryLightDelay,
													int& primaryLightDuration)
{
	if (!pageToPosition)
		return;

	//initialize the transition timing with ovveriden values

	duration = m_ovveridenDuration;
	profile = m_ovveridenVelocityProfile;
	delay = m_ovveridenDelay;

	// Get the trigger list
	NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* pTriggerList = NULL;
	switch (animType)
	{
		case ENdhsAnimationTypeTerminalState:
		{
			m_template->getTerminalDecorationInfo( delay, duration, profile, backgroundDelay, &pTriggerList, primaryLightDelay, primaryLightDuration,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

			break;
		}

		case ENdhsAnimationTypeLayoutChange:
		{
			m_template->getLayoutDecorationInfo(delay, duration, profile, backgroundDelay, &pTriggerList, primaryLightDelay, primaryLightDuration,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));
			break;
		}

		case ENdhsAnimationTypeInteractiveState:
		{
			m_template->getStateDecorationInfo(&pTriggerList,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

			break;
		}

		case ENdhsAnimationTypeScroll:
		{
			((NdhsCMenuComponentTemplate*)m_template)->getScrollDecorationInfo(m_finalSlotActive, &pTriggerList,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

			break;
		}

		default:
			break;
	}

	pageToPosition->setPageTransitionDecoration(pTriggerList);

	// This is the superset of placement attributes that will be considered for the primary light:
	// this selection may be narrowed below.
	LcTPlacement baseStart;
	int baseStartMask = 0;
	LcTPlacement baseEnd;
	int baseEndMask = 0;

	baseStartMask = baseEndMask = LcTPlacement::EColor | LcTPlacement::EColor2 | LcTPlacement::EOrientation;

	// Primary light
	if ((animType == ENdhsAnimationTypeTerminalState) || (animType == ENdhsAnimationTypeInteractiveState) || (animType == ENdhsAnimationTypeLayoutChange))
	{
		if (m_oldLayout)
		{
			baseStart = m_oldLayout->primaryLightLayout.layout;
			baseStartMask &= m_oldLayout->primaryLightLayout.mask;
		}
		else
		{
			baseStartMask = LcTPlacement::ENone;
		}
	}
	else
	{
		if (m_layout)
		{
			baseStart = m_layout->primaryLightLayout.layout;
			baseStartMask &= m_layout->primaryLightLayout.mask;
		}
		else
		{
			baseStartMask = LcTPlacement::ENone;
		}
	}

	if (m_layout)
	{
		baseEnd = m_layout->primaryLightLayout.layout;
		baseEndMask &= m_layout->primaryLightLayout.mask;
	}
	else
	{
		baseEndMask = LcTPlacement::ENone;
	}

	pageToPosition->setPrimaryLightTransitionBasePlacements(baseStart, baseStartMask, baseEnd, baseEndMask);
}

#define AGG_MASK (LcTPlacement::ELocation | LcTPlacement::EScale | LcTPlacement::EOrientation | LcTPlacement::EOpacity | LcTPlacement::EOffset)
/*-------------------------------------------------------------------------*//**
	Get the animation details for the specified transition.
*/
void NdhsCTransitionAgent::updateAnimationCache(NdhsCElementGroup* groupToPosition, ENdhsAnimationType animType,
											bool positionIncreasing)
{
	if (!groupToPosition)
		return;

	LcTPlacement *pBaseStart = NULL, *pBaseEnd = NULL;
	LcTPlacement baseStart, baseEnd;
	int baseStartMask = 0;
	int baseEndMask = 0;
	bool unLoadGroup = false;
	bool isHideStart = false;
	bool isHideEnd = false;
	LcTaString groupName = groupToPosition->getGroupName();

	// Detail Elements - both menu and page detail groups are treated as one
	if ((groupName.compare("detail_page") == 0)
			|| (groupName.compare("detail_menu") == 0))
		groupName = "detail";

	switch (animType)
	{
		case ENdhsAnimationTypeInteractiveState:
		case ENdhsAnimationTypeTerminalState:
		// State change - we are moving elements between the old and new layouts
		{
			// Reset group placement, we don't need to worry about other cached properties,
			// because they will be set in the steps below
			groupToPosition->resetPlacement();

			// Get starting and ending base placements
			if (m_oldLayout)
				pBaseStart = m_oldLayout->getPlacementForClass(groupToPosition->getGroupType(),
																		groupToPosition->getSlot(),
																		groupName,
																		baseStartMask,
																		isHideStart,
																		unLoadGroup);
			if (m_layout)
				pBaseEnd = m_layout->getPlacementForClass(groupToPosition->getGroupType(),
																		groupToPosition->getSlot(),
																		groupName,
																		baseEndMask,
																		isHideEnd,
																		unLoadGroup);

			//Set hide group flag for group
			groupToPosition->setHideGroupFlag(isHideEnd);

			// Check if we need to unload group or not
			if (unLoadGroup == true)
			{
				// Only try unloading a group if it is loaded
				if (groupToPosition->isGroupUnloaded() == false)
				{
					groupToPosition->unLoadGroup();
				}
			}
			else
			{
				// Only try loading a group if it is unloaded
				if (groupToPosition->isGroupUnloaded())
				{
					groupToPosition->loadGroup();
				}
			}

			if (pBaseStart != NULL)
			{
				baseStart = *pBaseStart;
				baseStartMask = AGG_MASK;
			}

			if (pBaseEnd != NULL)
			{
				baseEnd = *pBaseEnd;
				baseEndMask = AGG_MASK;
			}

			groupToPosition->setTransitionBasePlacements(	baseStart,
															baseStartMask,
															baseEnd,
															baseEndMask);

			// Now query the layouts for any displacements to the base placements
			const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseStartDisplacements = NULL;
			const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseEndDisplacements = NULL;
			if (m_oldLayout && (isHideStart == false))
				pBaseStartDisplacements = m_oldLayout->getDisplacements(groupToPosition->getGroupType(),
																		groupToPosition->getSlot(),
																		groupName);
			if (m_layout && (isHideEnd == false))
				pBaseEndDisplacements = m_layout->getDisplacements(groupToPosition->getGroupType(),
																		groupToPosition->getSlot(),
																		groupName);

			groupToPosition->setTransitionBaseDisplacements(pBaseStartDisplacements, pBaseEndDisplacements);

			// No curves on state change transitions
			groupToPosition->setTransitionScrollCurve(NULL, false);

			// Apply a decoration?
			// Get the decorator
			NdhsCKeyFrameList* pDecorator = m_template->getDecorationAnimation(m_initialSlotActive,
										animType, m_finalSlotActive,
										groupToPosition->getGroupType(), groupName,
										groupToPosition->getSlot(),
										groupToPosition->getSlot(),
										NULL, NULL, NULL,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

			groupToPosition->setTransitionDecoration(pDecorator);
			break;
		}

		case ENdhsAnimationTypeScroll:
		// We are moving aggregates between slots
		{
			// Get start and end slot values from element
			int startSlot = groupToPosition->getOldSlot();
			int endSlot = groupToPosition->getSlot();

			// We have 4 cases...
			if (startSlot == -1 && endSlot == -1)
			{
				// Must be furniture / detail aggregate...base placement doesn't
				// animate, but there may be a decoration to apply
				LcTPlacement placement = groupToPosition->getBasePlacement();
				if (groupToPosition->isVisible())
				{
					groupToPosition->setTransitionBasePlacements(	placement,
																	AGG_MASK,
																	placement,
																	AGG_MASK);
				}
				else
				{
					// Let hidden object remain hidden
					groupToPosition->setTransitionBasePlacements(	placement,
																	0,
																	placement,
																	0);
				}

				// Now query the layouts for any displacements to the base placement
				const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseDisplacements= NULL;
				if (m_layout)
					pBaseDisplacements = m_layout->getDisplacements(groupToPosition->getGroupType(),
																		-1,
																		groupName);

				groupToPosition->setTransitionBaseDisplacements(pBaseDisplacements, pBaseDisplacements);

				// No curve
				groupToPosition->setTransitionScrollCurve(NULL, false);

				// Get the decorator
				NdhsCKeyFrameList* pDecorator = m_template->getDecorationAnimation(m_finalSlotActive,
																			animType, m_finalSlotActive,
																			groupToPosition->getGroupType(),
																			groupName,
																			-1,
																			-1,
																			NULL, NULL, NULL,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

				groupToPosition->setTransitionDecoration(pDecorator);
			}
			else if (startSlot != -1 && endSlot == -1)
			{
				// Teleport aggregate out of slot
				if (m_layout)
					pBaseStart = m_layout->getPlacementForClass(groupToPosition->getGroupType(),
																		startSlot,
																		groupName,
																		baseStartMask,
																		isHideEnd,
																		unLoadGroup);

				//Set hide group flag for group - always hide as we are teleporting
				groupToPosition->setHideGroupFlag(true);

				if (pBaseStart)
				{
					baseStart = *pBaseStart;
					baseStartMask = AGG_MASK;

					groupToPosition->setTransitionBasePlacements(	baseStart,
																	baseStartMask,
																	LcTPlacement(),
																	0);

					// Now query the layout for any displacements to the base placement
					const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseStartDisplacements;
					pBaseStartDisplacements = m_layout->getDisplacements(groupToPosition->getGroupType(),
																		startSlot,
																		groupName);

					groupToPosition->setTransitionBaseDisplacements(pBaseStartDisplacements, NULL);

					// No curve
					groupToPosition->setTransitionScrollCurve(NULL, false);

					// No decoration
					groupToPosition->setTransitionDecoration(NULL);
				}
				else
				{
					// Clear out any existing transition information
					groupToPosition->setTransitionBasePlacements(	LcTPlacement(),
																	0,
																	LcTPlacement(),
																	0);
				}
			}
			else if (startSlot == -1 && endSlot != -1)
			{
				// Teleport element into slot
				// Get end placements for element from layout
				if (m_layout)
					pBaseEnd = m_layout->getPlacementForClass(groupToPosition->getGroupType(),
																		endSlot,
																		groupName,
																		baseEndMask,
																		isHideEnd,
																		unLoadGroup);

				//Set hide group flag for group - always hide as we are teleporting
				groupToPosition->setHideGroupFlag(true);

				if (pBaseEnd)
				{
					baseEnd = *pBaseEnd;
					baseEndMask = AGG_MASK;

					groupToPosition->setTransitionBasePlacements(	LcTPlacement(),
																	0,
																	baseEnd,
																	baseEndMask);

					// Now query the layout for any displacements to the base placement
					const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseEndDisplacements;
					pBaseEndDisplacements = m_layout->getDisplacements(groupToPosition->getGroupType(),
																		endSlot,
																		groupName);

					groupToPosition->setTransitionBaseDisplacements(NULL, pBaseEndDisplacements);

					// No curve
					groupToPosition->setTransitionScrollCurve(NULL, false);

					// No decoration
					groupToPosition->setTransitionDecoration(NULL);
				}
				else
				{
					// Clear out any existing transition information
					groupToPosition->setTransitionBasePlacements(	LcTPlacement(),
																	0,
																	LcTPlacement(),
																	0);
				}
			}
			else if (startSlot != -1 && endSlot != -1)
			{
				// Get endpoints
				if (m_layout)
				{
					bool startHide = false;
					bool endHide = false;

					pBaseStart = m_layout->getPlacementForClass(groupToPosition->getGroupType(),
																			startSlot,
																			groupName,
																			baseStartMask,
																			startHide,
																			unLoadGroup);

					pBaseEnd = m_layout->getPlacementForClass(groupToPosition->getGroupType(),
																			endSlot,
																			groupName,
																			baseEndMask,
																			endHide,
																			unLoadGroup);

					// If either the start or end is hidden, we are teleporting
					groupToPosition->setHideGroupFlag(startHide || endHide);
				}

				if (pBaseStart)
				{
					baseStart = *pBaseStart;
					baseStartMask = AGG_MASK;
				}

				if (pBaseEnd)
				{
					baseEnd = *pBaseEnd;
					baseEndMask = AGG_MASK;
				}

				groupToPosition->setTransitionBasePlacements(	baseStart,
																baseStartMask,
																baseEnd,
																baseEndMask);

				// Now query the layouts for any displacements to the base placements
				const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseStartDisplacements = NULL;
				const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseEndDisplacements = NULL;

				if (m_layout)
				{
					pBaseStartDisplacements = m_layout->getDisplacements(groupToPosition->getGroupType(),
													startSlot,
													groupName);
					pBaseEndDisplacements = m_layout->getDisplacements(groupToPosition->getGroupType(),
													endSlot,
													groupName);
				}

				groupToPosition->setTransitionBaseDisplacements(pBaseStartDisplacements, pBaseEndDisplacements);

				// Is there a curve?
				NdhsCKeyFrameList* pCurve = NULL;
				if (m_layout)
					pCurve = m_layout->getItemCurve(groupName,
													(startSlot < endSlot) ? startSlot : endSlot,
													(startSlot < endSlot) ? endSlot : startSlot);

				groupToPosition->setTransitionScrollCurve(pCurve, (startSlot < endSlot));

				// Get the decorator
				NdhsCKeyFrameList* pDecorator = m_template->getDecorationAnimation(m_initialSlotActive,
											animType, m_finalSlotActive,
											groupToPosition->getGroupType(), groupName,
											startSlot,
											endSlot,
											NULL, NULL, NULL,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

				groupToPosition->setTransitionDecoration(pDecorator);
			}

			break;
		}

		default:
			break;
	}
}

/*-------------------------------------------------------------------------*//**
	Get the animation details for the specified transition.
*/
void NdhsCTransitionAgent::updateAnimationCache(NdhsCElement* elementToPosition,
												bool isDetail,
												ENdhsAnimationType animType,
												bool positionIncreasing)
{
	LcTPlacement *pBaseStart = NULL, *pBaseEnd = NULL;
	LcTPlacement baseStart, baseEnd;
	int baseStartMask = 0;
	int baseEndMask = 0;
	bool unLoadElement = false;
	bool isHideStart = false;
	bool isHideEnd = false;
	bool baseStartUsingLayoutExtent = false;
	bool baseEndUsingLayoutExtent = false;


	if (!elementToPosition)
		return;

	// What we actually do with the element depends on what the page model requires
	switch (animType)
	{
		case ENdhsAnimationTypeInteractiveState:
		case ENdhsAnimationTypeTerminalState:
		// State change - we are moving elements between the old and new layouts
		{
			// Get starting and ending base placements
			baseStartUsingLayoutExtent = false;
			baseEndUsingLayoutExtent = false;

			// Reset element placement, we don't need to worry about other cached properties,
			// because they will be set in the steps below
			elementToPosition->resetPlacement();

			if (m_oldLayout)
				pBaseStart = m_oldLayout->getPlacementForClass(elementToPosition->getElementUse(),
																		elementToPosition->getElementGroup()->getSlot(),
																		elementToPosition->getElementClassName(),
																		baseStartMask,
																		isHideStart,
																		unLoadElement);
			if (m_layout)
				pBaseEnd = m_layout->getPlacementForClass(elementToPosition->getElementUse(),
																		elementToPosition->getElementGroup()->getSlot(),
																		elementToPosition->getElementClassName(),
																		baseEndMask,
																		isHideEnd,
																		unLoadElement);
			//Set the hide flag for the element
			elementToPosition->setHideElementFlag(isHideEnd);

			// Check if we need to unload group or not
			if (unLoadElement == true)
			{
				// Only try unloading an element if it is loaded
				if (elementToPosition->isElementSuspended() == false)
				{
					elementToPosition->onSuspend();
				}
			}
			else
			{
				// Only try loading an element if it is unloaded
				if (elementToPosition->isElementSuspended())
				{
					elementToPosition->onResume();
				}
			}

			if (pBaseStart)
			{
				baseStartUsingLayoutExtent = true;
				baseStart = *pBaseStart;

				// Extent should use the extent hint if not in the layout
				if ((baseStartMask & LcTPlacement::EExtent) == 0)
				{
					baseStart.extent = elementToPosition->getDefaultExtent();
					baseStartMask |= LcTPlacement::EExtent;
					baseStartUsingLayoutExtent = false;
					elementToPosition->setStartExtentIsFromWidget(true);
				}
				else
				{
					elementToPosition->setStartExtentIsFromWidget(false);
				}
			}

			if (pBaseEnd != NULL)
			{
				baseEndUsingLayoutExtent = true;
				baseEnd = *pBaseEnd;

				// Extent should use the extent hint if not in the layout
				if ((baseEndMask & LcTPlacement::EExtent) == 0)
				{
					baseEnd.extent = elementToPosition->getDefaultExtent();
					baseEndMask |= LcTPlacement::EExtent;
					baseEndUsingLayoutExtent = false;
					elementToPosition->setEndExtentIsFromWidget(true);
				}
				else
				{
					elementToPosition->setEndExtentIsFromWidget(false);
				}
			}

			elementToPosition->setTransitionBasePlacements(	baseStart,
															baseStartMask,
															baseStartUsingLayoutExtent,
															baseEnd,
															baseEndMask,
															baseEndUsingLayoutExtent);

			// Now query the layouts for any displacements to the base placements
			const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseStartDisplacements = NULL;
			const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseEndDisplacements = NULL;
			if (m_oldLayout && (isHideStart == false))
				pBaseStartDisplacements = m_oldLayout->getDisplacements(elementToPosition->getElementUse(),
												elementToPosition->getElementGroup()->getSlot(),
												elementToPosition->getElementClassName());
			if (m_layout && (isHideEnd == false))
				pBaseEndDisplacements = m_layout->getDisplacements(elementToPosition->getElementUse(),
												elementToPosition->getElementGroup()->getSlot(),
												elementToPosition->getElementClassName());

			elementToPosition->setTransitionBaseDisplacements(pBaseStartDisplacements, pBaseEndDisplacements);

			// No curves on state change transitions
			elementToPosition->setTransitionScrollCurve(NULL, false);

			// Apply a decoration?
			// Get the decorator
			NdhsCKeyFrameList* pDecorator = m_template->getDecorationAnimation(m_initialSlotActive,
										animType, m_finalSlotActive,
										elementToPosition->getElementUse(), elementToPosition->getElementClassName(),
										elementToPosition->getElementGroup()->getSlot(),
										elementToPosition->getElementGroup()->getSlot(),
										NULL, NULL, NULL,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

			elementToPosition->setTransitionDecoration(pDecorator);
			break;
		}

		case ENdhsAnimationTypeScroll:
		// We are moving item elements between slots
		{
			// Get start and end slot values from element
			int startSlot = elementToPosition->getOldSlot();
			int endSlot = elementToPosition->getSlot();

			// We have 4 cases...
			if (startSlot == -1 && endSlot == -1)
			{
				// Must be furniture / detail element...base placement doesn't
				// animate, but there may be a decoration to apply
				LcTPlacement placement = elementToPosition->getBasePlacement();
				bool placementUsingLayoutExtent = elementToPosition->isUsingLayoutExtent();
				if (elementToPosition->getWidget() && elementToPosition->getWidget()->isVisible())
				{
					elementToPosition->setTransitionBasePlacements(	placement,
																	LcTPlacement::EAll,
																	placementUsingLayoutExtent,
																	placement,
																	LcTPlacement::EAll,
																	placementUsingLayoutExtent);
				}
				else
				{
					// Let hidden object remain hidden
					elementToPosition->setTransitionBasePlacements(	placement,
																	0,
																	placementUsingLayoutExtent,
																	placement,
																	0,
																	placementUsingLayoutExtent);
				}

				// Now query the layouts for any displacements to the base placement
				const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseDisplacements = NULL;
				if (m_layout)
					pBaseDisplacements = m_layout->getDisplacements(elementToPosition->getElementUse(),
													-1,
													elementToPosition->getElementClassName());

				elementToPosition->setTransitionBaseDisplacements(pBaseDisplacements, pBaseDisplacements);

				// No curve
				elementToPosition->setTransitionScrollCurve(NULL, false);

				// Get the decorator
				NdhsCKeyFrameList* pDecorator = m_template->getDecorationAnimation(m_finalSlotActive,
																			animType, m_finalSlotActive,
																			elementToPosition->getElementUse(),
																			elementToPosition->getElementClassName(),
																			-1,
																			-1,
																			NULL, NULL, NULL,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

				elementToPosition->setTransitionDecoration(pDecorator);
			}
			else if (startSlot != -1 && endSlot == -1)
			{
				// Teleport element out of slot
				baseStartUsingLayoutExtent = false;
				if (m_layout)
					pBaseStart = m_layout->getPlacementForClass(elementToPosition->getElementUse(),
																		startSlot,
																		elementToPosition->getElementClassName(),
																		baseStartMask,
																		isHideEnd,
																		unLoadElement);

				// Set hide flag for element - always hide as we are teleporting
				elementToPosition->setHideElementFlag(true);

				if (pBaseStart)
				{
					baseStartUsingLayoutExtent = true;
					baseStart = *pBaseStart;

					// Extent should use the extent hint if not in the layout
					if ((baseStartMask & LcTPlacement::EExtent) == 0)
					{
						baseStart.extent = elementToPosition->getDefaultExtent();
						baseStartMask |= LcTPlacement::EExtent;
						baseStartUsingLayoutExtent = false;
						elementToPosition->setStartExtentIsFromWidget(true);
					}
					else
					{
						elementToPosition->setStartExtentIsFromWidget(false);
					}

					bool elementUsingLayoutExtent = elementToPosition->isUsingLayoutExtent();
					elementToPosition->setTransitionBasePlacements(	baseStart,
																	baseStartMask,
																	baseStartUsingLayoutExtent,
																	LcTPlacement(),
																	0,
																	elementUsingLayoutExtent);

					// Now query the layout for any displacements to the base placement
					const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseStartDisplacements;
					pBaseStartDisplacements = m_layout->getDisplacements(elementToPosition->getElementUse(),
														startSlot,
														elementToPosition->getElementClassName());

					elementToPosition->setTransitionBaseDisplacements(pBaseStartDisplacements, NULL);

					// No curve
					elementToPosition->setTransitionScrollCurve(NULL, false);

					// No decoration
					elementToPosition->setTransitionDecoration(NULL);
				}
				else
				{
					// Clear out any existing transition information
					elementToPosition->setTransitionBasePlacements(	LcTPlacement(),
																	0,
																	false,
																	LcTPlacement(),
																	0,
																	false);
				}
			}
			else if (startSlot == -1 && endSlot != -1)
			{
				// Teleport element into slot
				// Get end placements for element from layout
				baseStartUsingLayoutExtent = false;
				baseEndUsingLayoutExtent = false;
				if (m_layout)
					pBaseEnd = m_layout->getPlacementForClass(elementToPosition->getElementUse(),
																		endSlot,
																		elementToPosition->getElementClassName(),
																		baseEndMask,
																		isHideEnd,
																		unLoadElement);

				// Set hide flag for element - always hide as we are teleporting
				elementToPosition->setHideElementFlag(true);

				if (pBaseEnd)
				{
					baseEndUsingLayoutExtent = true;
					baseEnd = *pBaseEnd;

					// Extent should use the extent hint if not in the layout
					if ((baseEndMask & LcTPlacement::EExtent) == 0)
					{
						baseEnd.extent = elementToPosition->getDefaultExtent();
						baseEndMask |= LcTPlacement::EExtent;
						baseEndUsingLayoutExtent = false;
						elementToPosition->setEndExtentIsFromWidget(true);
					}
					else
					{
						elementToPosition->setEndExtentIsFromWidget(false);
					}

					elementToPosition->setTransitionBasePlacements(	LcTPlacement(),
																	0,
																	elementToPosition->isUsingLayoutExtent(),
																	baseEnd,
																	baseEndMask,
																	baseEndUsingLayoutExtent);

					// Now query the layout for any displacements to the base placement
					const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseEndDisplacements;
					pBaseEndDisplacements = m_layout->getDisplacements(elementToPosition->getElementUse(),
														endSlot,
														elementToPosition->getElementClassName());

					elementToPosition->setTransitionBaseDisplacements(NULL, pBaseEndDisplacements);

					// No curve
					elementToPosition->setTransitionScrollCurve(NULL, false);

					// No decoration
					elementToPosition->setTransitionDecoration(NULL);
				}
				else
				{
					// Clear out any existing transition information
					elementToPosition->setTransitionBasePlacements(	LcTPlacement(),
																	0,
																	false,
																	LcTPlacement(),
																	0,
																	false);
				}
			}
			else if (startSlot != -1 && endSlot != -1)
			{
				// Get endpoints
				baseStartUsingLayoutExtent = false;
				baseEndUsingLayoutExtent = false;
				if (m_layout)
				{
					bool startHide = false;
					bool endHide = false;

					pBaseStart = m_layout->getPlacementForClass(elementToPosition->getElementUse(),
																			startSlot,
																			elementToPosition->getElementClassName(),
																			baseStartMask,
																			startHide,
																			unLoadElement);

					pBaseEnd = m_layout->getPlacementForClass(elementToPosition->getElementUse(),
																			endSlot,
																			elementToPosition->getElementClassName(),
																			baseEndMask,
																			endHide,
																			unLoadElement);

					// If either the start or end is hidden, we are teleporting
					elementToPosition->setHideElementFlag(startHide || endHide);
				}

				if (pBaseStart)
				{
					baseStartUsingLayoutExtent = true;
					baseStart = *pBaseStart;

					// Extent should use the extent hint if not in the layout
					if ((baseStartMask & LcTPlacement::EExtent) == 0)
					{
						baseStart.extent = elementToPosition->getDefaultExtent();
						baseStartMask |= LcTPlacement::EExtent;
						baseStartUsingLayoutExtent = false;
						elementToPosition->setStartExtentIsFromWidget(true);
					}
					else
					{
						elementToPosition->setStartExtentIsFromWidget(false);
					}
				}

				if (pBaseEnd)
				{
					baseEndUsingLayoutExtent = true;
					baseEnd = *pBaseEnd;

					// Extent should use the extent hint if not in the layout
					if ((baseEndMask & LcTPlacement::EExtent) == 0)
					{
						baseEnd.extent = elementToPosition->getDefaultExtent();
						baseEndMask |= LcTPlacement::EExtent;
						baseEndUsingLayoutExtent = false;
						elementToPosition->setEndExtentIsFromWidget(true);
					}
					else
					{
						elementToPosition->setEndExtentIsFromWidget(false);
					}
				}

				elementToPosition->setTransitionBasePlacements(	baseStart,
																baseStartMask,
																baseStartUsingLayoutExtent,
																baseEnd,
																baseEndMask,
																baseEndUsingLayoutExtent);

				// Now query the layouts for any displacements to the base placements
				const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseStartDisplacements = NULL;
				const LcTmArray<NdhsCTemplate::CDisplacement*>* pBaseEndDisplacements = NULL;

				if (m_layout)
				{
					pBaseStartDisplacements = m_layout->getDisplacements(elementToPosition->getElementUse(),
													startSlot,
													elementToPosition->getElementClassName());
					pBaseEndDisplacements = m_layout->getDisplacements(elementToPosition->getElementUse(),
													endSlot,
													elementToPosition->getElementClassName());
				}

				elementToPosition->setTransitionBaseDisplacements(pBaseStartDisplacements, pBaseEndDisplacements);

				// Is there a curve?
				NdhsCKeyFrameList* pCurve = NULL;
				if (m_layout)
					pCurve = m_layout->getItemCurve(elementToPosition->getElementClassName(),
																	(startSlot < endSlot) ? startSlot : endSlot,
																	(startSlot < endSlot) ? endSlot : startSlot);

				elementToPosition->setTransitionScrollCurve(pCurve, (startSlot < endSlot));

				// Get the decorator
				NdhsCKeyFrameList* pDecorator = m_template->getDecorationAnimation(m_initialSlotActive,
											animType, m_finalSlotActive,
											elementToPosition->getElementUse(), elementToPosition->getElementClassName(),
											startSlot,
											endSlot,
											NULL, NULL, NULL,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

				elementToPosition->setTransitionDecoration(pDecorator);
			}

			break;
		}

		default:
			break;
	}
}

/*-------------------------------------------------------------------------*//**
	Get the static animation details for a page and menu
*/
void NdhsCTransitionAgent::getStaticTriggers(NdhsCElementGroup* group)
{
	NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* pTriggerList = NULL;

	// Get the trigger list
	m_template->getStaticDecorationInfo(m_finalSlotActive, &pTriggerList,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

	if (pTriggerList != NULL && pTriggerList->transitionTime > 0)
	{
		// Set the static trigger, if it exists
		group->setStaticAnimationTrigger(pTriggerList);
	}
}

/*-------------------------------------------------------------------------*//**
	Get the static animation details for an element group
*/
void NdhsCTransitionAgent::getStaticAnimation(NdhsCElementGroup* group)
{
	int staticTime = 0;
	int loopCount = 1;
	ENdhsVelocityProfile velocityProfile = ENdhsVelocityProfileUnknown;
	LcTaString groupName = group->getGroupName();

	// Detail Elements - both menu and page detail groups are treated as one
	if ((groupName.compare("detail_page") == 0)
			|| (groupName.compare("detail_menu") == 0))
		groupName = "detail";


	NdhsCKeyFrameList* pDecorator = m_template->getDecorationAnimation(m_finalSlotActive,
												ENdhsAnimationTypeStatic, m_finalSlotActive,
												group->getGroupType(),
												groupName,
												group->getSlot(),
												group->getSlot(),
												&staticTime, &loopCount, &velocityProfile,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

	if (pDecorator != NULL && staticTime > 0)
	{
		// Set the static animation, if it exists
		group->setStaticAnimation(pDecorator, staticTime, loopCount, velocityProfile);
	}
}

/*-------------------------------------------------------------------------*//**
	Get the static animation details for an element
*/
void NdhsCTransitionAgent::getStaticAnimation(NdhsCElement* element, bool isDetail)
{
	int staticTime = 0;
	int loopCount = 1;
	ENdhsVelocityProfile velocityProfile = ENdhsVelocityProfileUnknown;

	NdhsCKeyFrameList* pDecorator = m_template->getDecorationAnimation(m_finalSlotActive,
												ENdhsAnimationTypeStatic, m_finalSlotActive,
												element->getElementUse(),
												element->getElementClassName(),
												element->getElementGroup()->getSlot(),
												element->getElementGroup()->getSlot(),
												&staticTime, &loopCount, &velocityProfile,((m_oldLayout.ptr()==NULL)?"":m_oldLayout->name),((m_layout.ptr()==NULL)?"":m_layout->name));

	if (pDecorator != NULL && staticTime > 0)
	{
		// Set the static animation, if it exists
		element->setStaticAnimation(pDecorator, staticTime, loopCount, velocityProfile);
	}
}
