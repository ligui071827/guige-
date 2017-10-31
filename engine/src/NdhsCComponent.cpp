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

#include <math.h>

#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


#define ANIM_NON_SKETCHY_THRESHOLD .001
#define AGG_MASK (LcTPlacement::ELocation | LcTPlacement::EScale | LcTPlacement::EOrientation | LcTPlacement::EOpacity)

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCComponent> NdhsCComponent::create(	NdhsCPageManager*					pageManager,
													NdhsCTemplate::CComponentElement*	component,
													NdhsCElementGroup*					parent,
													int									stackLevel,
													NdhsCMenu*							menu,
													NdhsCMenuItem*						menuItem,
													NdhsCElementGroup*					parentGroup)
{
	LcTaOwner<NdhsCComponent> ref;
	ref.set(new NdhsCComponent(pageManager, component, parent, stackLevel, menu, menuItem, parentGroup));
//	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCComponent::NdhsCComponent(	NdhsCPageManager*					pageManager,
								NdhsCTemplate::CComponentElement*	component,
								NdhsCElementGroup*					parent,
								int									stackLevel,
								NdhsCMenu*							menu,
								NdhsCMenuItem*						menuItem,
								NdhsCElementGroup*					parentGroup)
: NdhsCElementGroup(component->className, parent, menu, menuItem, component->m_drawLayerIndex, component->layoutTeleport, component->scrollTeleport),
m_postTransitionCompleteMessage(this, EPostTransitionCompleteMsg),
m_simulateKeyMessage(this,ESimulateKeyMsg)
{
	LC_ASSERT(pageManager != NULL);
	LC_ASSERT(component != NULL);
	LC_ASSERT(component->componentFile != NULL);

	m_path = component->path;

	m_parent = parent;
	m_groupUnloaded = true;

	// by default mark it true
	m_componentRetire = true;

	m_componentName = component->className;

	setGroupType((menuItem!=NULL && getPage()->isImmediateMenu(menuItem))?ENdhsObjectTypeItemComponent:ENdhsObjectTypeFurnitureComponent);

	m_pageManager  = pageManager;
	m_isCreated  = false;
	m_isEnabled = false;


	// Set the component file
	m_template = component->componentFile;

	// Set the param array
	if (component->bindingParameters.empty() == false)
		m_parameters = &(component->bindingParameters);

	m_stackLevel			= stackLevel;

	m_refreshNeeded = false;
	m_jumpingToEnd = false;

	setDrawLayerIndex(component->m_drawLayerIndex);

#if defined(NDHS_JNI_INTERFACE)
	m_pageManager->resetIdentifier();
	m_identifier = 0;
	if (m_parent->getIdentifier() >= 1)
	{
		m_identifier = m_parent->distanceFromPage();
		NdhsCPageManager::CStaticPreviewCache* entry = m_pageManager->getCachedEntry(m_identifier);
		if (entry)
		{
			if(entry->componentClassName.compareNoCase(m_componentName) != 0)
			{
				m_identifier = 0;
			}
		}
	}
#endif

#ifdef LC_USE_LIGHTS
	setLightModel(component->componentFile->getLightModel());
	applyLightModel(component->lightModel);

	if (parentGroup)
		applyLightModel(parentGroup->getLightModel());
#endif

	m_isEligible = false;
	m_trySetFocus =  false;
	m_pendingLayout = NULL;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::construct()
{

	LcCAggregate::construct();

	// Create the layout change field
	LcTaOwner<NdhsCScrollPosField> tempPos = NdhsCScrollPosField::create(m_pageManager->getCon(), NULL,
													NDHS_VELOCITY_PROFILE_CATAPULT_MAX_DRAWBACK,
													NDHS_VELOCITY_PROFILE_BOUNCE_MAX_EXTENSION,
													false,
													0.0f);
	m_layoutPos = tempPos;
	m_layoutPos->setIgnoreLaundry(true);
	// Note that we don't observe the layoutPos field, as we're the only entity that can modify it.

	// Create field cache
	LcTaOwner<NdhsCFieldCache> tempFC = NdhsCFieldCache::create(m_pageManager->getCon());
	m_fieldCache = tempFC;

	// Create intrinsic fields
	createIntrinsicFields();

	// First create field cache for component
	createFieldCache();

	// Bind parameter now
	bindParameters();

	m_focusEnabledField = m_fieldCache->getField("_focusenabled");
	if (m_focusEnabledField)
	{
		if (m_focusEnabledField->isError())
		{
			NdhsCExpression* expr = m_focusEnabledField->getBoundExpression();

			if (expr)
			{
				expr->errorDiagnostics("\"Enable focus\"", true);
			}
		}
		else
		{
			m_focusEnabledField->addObserver(this);
		}
	}

	NdhsCComponent* parentComponent = findParentComponent();
	if (parentComponent)
	{
		m_parentFocusEnabledField = parentComponent->getFocusEnabledField();

		if (m_parentFocusEnabledField)
		{
			m_parentFocusEnabledField->addObserver(this);
		}
	}

	LcTaOwner<NdhsCStateManager> tempSM = NdhsCStateManager::create(m_pageManager->getCon()->getLaundry(), m_template, this, m_pageManager);
	m_stateManager = tempSM;

	// Create the transition agent
	LcTaOwner<NdhsCTransitionAgent> temp = NdhsCTransitionAgent::create(m_template, this);
	m_transitionAgent = temp;

	// Update component extent intrinsic varaible
	if (m_template)
		updateExtentIntrinsicVaraibles(m_template->getComponentBoundingBox(), LcTPlacement::EExtent);

	// component should begin invisible and become visible
	// after the opening delay
	setVisible(false);

	doPostConstruct();

}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCComponent::onRealize()
{
	if (isGroupUnloaded())
	{
		// Undo the realize on this group
		retire();
	}
	else
	{
		NdhsCElementGroup::onRealize();

		if (m_localScreenAggregate)
		{
			m_localScreenAggregate->realize(m_pageManager);
		}

		m_componentRetire = false;

		if (m_stateManager)
		{
#if defined(NDHS_JNI_INTERFACE)
			m_stateManager->setAggregateSnapShotInfo(takeSnapShot());
#endif
			m_stateManager->addObserver(this);
			m_stateManager->testLayoutConditions();
			onTransitionComplete(false);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::onRetire()
{
	m_componentRetire = true;

	NdhsCElementGroup::onRetire();

	if (m_stateManager)
		m_stateManager->removeObserver(this);

	resetTransitionCache();

	// Terminate any static animations
	stopStaticAnimations(true, EAnimationAll);

	jumpTransitionToEnd(false);

	m_bRequiresPostTransitionComplete = false;

	if (m_localScreenAggregate)
		m_localScreenAggregate->retire();

#ifdef IFX_USE_STYLUS
	// In case the page is referencing this component....
	m_pageManager->onMouseCancel(this, NULL, false);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCComponent::componentDoPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame)
{
	bool reschedule = false;

	// If group unloaded no need to go further
	if (isGroupUnloaded() || !m_outerGroup)
		return reschedule;

	if (isComponentReady() && visibleOnTransitionComplete() && doPrepareForFrameUpdate(timestamp, finalFrame))
		reschedule = true;

	// Call on our child also
	if (m_outerGroup->componentDoPrepareForFrameUpdate(timestamp, finalFrame))
		reschedule = true;

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::componentsJumpTransitionToEnd(bool setIdle)
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup || isGroupUnloaded())
		return;

	if (isComponentReady())
		jumpTransitionToEnd(setIdle);

	// Call on our child also
	m_outerGroup->componentsJumpTransitionToEnd(setIdle);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCComponent::getFullFilePath(IFX_HMENU hMenu,
								const LcTmString& searchFile,
								LcTmString& returnFilePath,
								int menuIndex)
{
	if(isGroupUnloaded() || hMenu==NULL)
		return false;

	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return false;

	// Call on child also
	return m_outerGroup->getFullFilePath(hMenu, searchFile, returnFilePath, menuIndex);
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCComponent::getHMenuActiveItemIndex(IFX_HMENU hMenu)
{
	if(isGroupUnloaded())
		return -1;

	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return -1;

	// Call on child also
	return m_outerGroup->getHMenuActiveItemIndex(hMenu);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCComponent::resumeStaticAnimations()
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup || isGroupUnloaded())
		return ;

	// If component got any static animation try to start it
	if (startStaticAnimations(EAnimationAll))
		m_animType = ENdhsAnimationTypeStatic;

	// Call on child also
	m_outerGroup->resumeStaticAnimations();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCComponent::componentRefreshField(const LcTmString& field, int item)
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup || isGroupUnloaded())
		return;

	// Refresh current component
	refreshField(field, item);

	// Call on child also
	m_outerGroup->componentRefreshField(field, item);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCComponent::isComponentTransitioning()
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup || isGroupUnloaded())
		return false;

	// Current component transitioning
	if (isTransitioning())
		return true;

	// Call on child also
	return m_outerGroup->isComponentTransitioning();
}

#if defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::componentOnTransitionComplete(bool setIdle)
{
	onTransitionComplete(setIdle);

	// Go through child components of current component
	if (m_outerGroup)
	{
		m_outerGroup->componentOnTransitionComplete(setIdle);
	}
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::schedulePostTransitionComplete()
{
	// If we're not scheduled and we want to, schedule with 0 delay
	if (m_postTransitionCompleteMessage.isScheduled() == false)
	{
		m_postTransitionCompleteMessage.schedule(getSpace()->getTimer(), 0, 0);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::doPostConstruct()
{
	reloadElement();

	m_isCreated = true;
}

/*-------------------------------------------------------------------------*//**
																			 */
void NdhsCComponent::createIntrinsicFields()
{
	// Create component intirnsic fields, need to create before transition manager, so placed here
	if (m_fieldCache)
		m_fieldCache->createComponentIntrinsicFields();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::createFieldCache()
{
	// Add input parameters
	if (m_fieldCache)
	{
		m_fieldCache->addInputParameters(m_template, this, this);

		// Add input parameters
		if (m_parameters && m_parameters->empty() == false)
		{
			LcTmOwnerMap<LcTmString, NdhsCExpression::CExprSkeleton>::iterator it = m_parameters->begin();
			NdhsCTemplate::TmMParameterMap& paramsMap = m_template->getParametersMap();

			for (; it != m_parameters->end(); it++)
			{
				NdhsCTemplate::TmMParameterMap::iterator paramIt = paramsMap.find(it->first);

				if (paramIt != paramsMap.end() && paramIt->second->mode == IFXI_FIELD_MODE_INPUT_OUTPUT)
				{
					m_fieldCache->bindOutputParameter(it->first, it->second, m_parent, getSlot(), getMenuItem());
				}
			}
		}

		// Add local varaibles
		m_fieldCache->addLocalVariables(m_template, this);
		#ifdef IFX_SERIALIZATION
			m_fieldCache->setMenuItemChild(getMenuItem()!=NULL);
		#endif
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::bindParameters()
{
	if (m_parameters && m_parameters->empty() == false)
	{
		LcTmOwnerMap<LcTmString, NdhsCExpression::CExprSkeleton>::iterator it = m_parameters->begin();
		NdhsCTemplate::TmMParameterMap& paramsMap = m_template->getParametersMap();

		for (; it != m_parameters->end(); it++)
		{
			NdhsCTemplate::TmMParameterMap::iterator paramIt = paramsMap.find(it->first);

			if (paramIt == paramsMap.end())
			{
				if (it->first.compareNoCase("_focusEnabled") == 0)
				{
					m_fieldCache->bindInputParameter(it->first, it->second, m_parent, getSlot(), getMenuItem());
				}
				else if (it->first.compareNoCase("_pagestate") == 0)
				{
					m_fieldCache->bindInputParameter(it->first, it->second, m_parent, getSlot(), getMenuItem());
				}
			}
			else if (paramIt->second->mode == IFXI_FIELD_MODE_OUTPUT)
			{
				m_fieldCache->bindInputParameter(it->first, it->second, m_parent, getSlot(), getMenuItem());
			}
		}
	}

	// Clean all of the expressions
	m_pageManager->getCon()->getLaundry()->cleanAll();
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCComponent::~NdhsCComponent()
{
	doCleanup();
}

/*-------------------------------------------------------------------------*//**
	Creating it such a way so that it can be used in component
	re-loading/unloading i.e. destroying aggregates explicitely
*/
void NdhsCComponent::doCleanup()
{
	retire();

#ifdef IFX_USE_STYLUS
	if (m_pageManager)
	{
		m_pageManager->ignoreEntry(this);
	}
#endif

	if(m_pStaticAnimField)
	{
		m_pStaticAnimField->removeObserver(this);
	}

	if (m_staticAnimFields.size() > 0)
	{
		m_staticAnimFields.clear();
	}

	if (m_postTransitionCompleteMessage.isScheduled())
	{
		m_postTransitionCompleteMessage.cancel();
	}

	if(m_simulateKeyMessage.isScheduled())
	{
		m_simulateKeyMessage.cancel();
	}

	if (m_fieldCache)
	{
		m_fieldCache.destroy();
	}

	if (m_transitionAgent)
	{
		m_transitionAgent.destroy();
	}

	if (m_layoutPos)
	{
		m_layoutPos.destroy();
	}

	if (m_stateManager)
	{
		m_stateManager.destroy();
	}

	if (m_localScreenAggregate)
	{
		m_localScreenAggregate->retire();
		m_localScreenAggregate.destroy();
	}

	if (m_outerGroup)
	{
		m_outerGroup->retire();
		m_outerGroup.destroy();
	}
#ifdef IFX_USE_STYLUS
	m_mouseFocusElement = NULL;
#endif
	m_focusedElement = NULL;
	m_focusedChildComponent = NULL;
	m_inFocus = false;
	m_focusEnabledField = NULL;
	m_parentFocusEnabledField = NULL;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::destroyUnwantedElements()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT NdhsCField* NdhsCComponent::getFieldValue(const LcTmString& fieldName,
													NdhsCMenu* menu,
													int item,
													NdhsCElement* element)
{
	return getField(fieldName, -1, NULL);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCComponent::startTransition(ENdhsAnimationType animType,
									bool animate)
{
	bool willAnimate = true;

	if (isComponentRetired() || !m_outerGroup)
		return false;

	if (m_transitionAgent)
	{
		switch(animType)
		{
			case ENdhsAnimationTypeLayoutChange:
			case ENdhsAnimationTypeInteractiveState:
			case ENdhsAnimationTypeTerminalState:
			{
				// Tell any ongoing statics to terminate at the end
				// of their current animation
				if (m_animType == ENdhsAnimationTypeStatic)
					stopStaticAnimations(true, EAnimationAll);

				// We're about to make a whole bunch of pointers stale, so we need to clear them up first
				resetTransitionCache();

				// Update the starting and ending layout objects.
				m_transitionAgent->prepareTransition(m_stateManager->getCurrentLayout(), -1, -1);

				// Record transition type
				m_previousAnimType = m_animType;
				m_animType = animType;

				{
					int tempBackgroundDelay;
					int tempPrimaryLightDelay;
					int tempPrimaryLightDuration;

					// Get info for the transition to come - note that this may replace duration/delay/profile
					// and bg delay for terminal state transitions if decorations are supported.
					m_transitionAgent->getPageAnimationDetails(this,
															getAnimType(),
															m_transitionDuration,
															m_transitionDelay,
															m_transitionVelProfile,
															tempBackgroundDelay,
															tempPrimaryLightDelay,
															tempPrimaryLightDuration);
				}

				if(animType == ENdhsAnimationTypeLayoutChange)
				{
					animType = ENdhsAnimationTypeInteractiveState;
					m_animType = animType;
				}

				LcTTime timestamp = getSpace()->getTimestamp();

				// we have got the timing details, we need to set up the relevant field to control animation
				bool finalUpdate;

				if (animType == ENdhsAnimationTypeInteractiveState)
				{
					m_layoutPos->setValue(0.0, false);
					m_layoutPos->setTargetValue(1.0, ENdhsFieldDirectionIncreasing, false, m_transitionDuration,
												ENdhsScrollFieldModeNormal, m_transitionVelProfile);

					// We need to cache the timestamp locally...Note that we don't
					// call 'updateValue' here, as we don't want the animator to start
					// until the delay has finished, unless delay is 0.
					if (m_transitionDelay == 0)
					{
						m_layoutPos->updateValue(timestamp, finalUpdate);
					}

					m_decorationDelayTimestamp = timestamp + m_transitionDelay;
					m_stateChangeAnimComplete = false;
				}

				updatePosition(0.0, true, true);

#ifdef IFX_GENERATE_SCRIPTS

		// Inform script generator that we need screen capture events in transition.
		if (NdhsCScriptGenerator::getInstance())
			NdhsCScriptGenerator::getInstance()->setTransitionCaptureEvents(timestamp, m_transitionDelay,
																			m_transitionDuration);
#endif //IFX_GENERATE_SCRIPTS

				break;
			}

			case ENdhsAnimationTypeStatic:
			{
				// Set up the static animations...note that if we've recently
				// finished a scroll, it's only the slots/items/detail that will
				// start a static animation (although the furniture/page/menu might
				// still have an active static ongoing...)
				if (startStaticAnimations(m_staticAnimGroup))
					m_animType = ENdhsAnimationTypeStatic;

				break;
			}

			default:
				break;
		}

	}

	if ((m_animType != ENdhsAnimationTypeStatic) && (m_transitionDuration == 0))
		willAnimate = false;

	if (willAnimate)
		m_pageManager->startAnimation();

	return willAnimate;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCComponent::startStaticAnimations(EAnimationGroup groupToStart)
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return false;

	// Initialize
	m_stopStaticAnimationAll = false;
	m_stopStaticAnimationItem = false;

	// Clear out any existing statics that are still going
	stopStaticAnimations(true, groupToStart);

	// Query transition agent for any static animations
	if (!m_pStaticTriggerList)
		m_transitionAgent->getStaticTriggers(this);

	if (groupToStart == EAnimationAll)
	{
		// Set up the statics on the page group (this will propagate
		// down through the other furniture groups)
		m_outerGroup->startStaticAnimations();
	}

	// If there are any static fields, then we have some static animations to do
	return (m_staticAnimFields.size() != 0);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::stopStaticAnimations(bool immediateStop, EAnimationGroup groupToStop)
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)	
		return;

	// If there are no static animation fields, there's nothing to do.
	if (m_staticAnimFields.size() == 0 && !m_pStaticAnimFieldTrigger && !m_pStaticTriggerList)
		return;

	if (immediateStop)
	{
		if (groupToStop == EAnimationAll)
		{
			// Stop the furniture static animation
			m_outerGroup->stopStaticAnimations();

			// Update animation states
			if (m_animType == ENdhsAnimationTypeStatic)
				m_animType = ENdhsAnimationTypeNone;
			m_previousAnimType = ENdhsAnimationTypeNone;
		}

		// Stop the static triggers - always done
		if (m_pStaticAnimFieldTrigger)
		{
			// Call cleanup trigger if required
			if (m_pStaticAnimFieldTrigger->atRest() == false)
			{
				if (m_pStaticTriggerList && m_pStaticTriggerList->hasCleanupTrigger)
					simulateKeyDown(m_pStaticTriggerList->cleanupTrigger);
			}

			releaseStaticAnimationField(m_pStaticAnimFieldTrigger);
			m_pStaticAnimFieldTrigger = NULL;
		}
		m_pStaticTriggerList = NULL;
	}
	else
	{
		if (groupToStop == EAnimationAll)
		{
			m_stopStaticAnimationAll = true;
		}
		else
		{
			// Note that next time animation field wraps, we should terminate
			m_stopStaticAnimationItem = true;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::setStaticAnimationTrigger(NdhsCTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* pTriggerList)
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return;

	m_pStaticTriggerList = pTriggerList;

	if (pTriggerList)
	{
		if (m_pStaticAnimFieldTrigger != NULL)
			releaseStaticAnimationField(m_pStaticAnimFieldTrigger);

		// Get the animation field reference
		m_pStaticAnimFieldTrigger = getStaticAnimationField(this);

		// Configure the field for the animation
		if (m_pStaticAnimFieldTrigger)
		{
			bool retVal;
			int loopCount = m_pStaticTriggerList->loopCount;
			int staticTime = m_pStaticTriggerList->transitionTime;

			retVal = m_pStaticAnimFieldTrigger->setTargetValue(1.0, ENdhsFieldDirectionIncreasing, false, staticTime,
											ENdhsScrollFieldModeNormal, ENdhsVelocityProfileUnknown);
			m_pStaticAnimFieldTrigger->setLoop(loopCount);
			m_lastStaticTriggerPos = -1;

			if (!retVal)
			{
				releaseStaticAnimationField(m_pStaticAnimFieldTrigger);
				m_pStaticTriggerList = NULL;
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCScrollPosField* NdhsCComponent::getStaticAnimationField(NdhsCField::IObserver* observer)
{
	NdhsCScrollPosField* retVal = NULL;

	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return NULL;

	// Create the field
	LcTaOwner<NdhsCScrollPosField> tempPos = NdhsCScrollPosField::create(m_pageManager->getCon(), NULL,
													NDHS_VELOCITY_PROFILE_CATAPULT_MAX_DRAWBACK,
													NDHS_VELOCITY_PROFILE_BOUNCE_MAX_EXTENSION,
													true, 0);

	if (tempPos)
	{
		tempPos->addObserver(observer);
		retVal = tempPos.ptr();

		// Add it to our list of static animation fields
		m_staticAnimFields.push_back(tempPos);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::releaseStaticAnimationField(NdhsCScrollPosField* field)
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)	
		return;

	// Find the field in our array & remove it.
	LcTmOwnerArray<NdhsCScrollPosField>::iterator it = m_staticAnimFields.begin();

	for (; it != m_staticAnimFields.end(); it++)
	{
		if ((*it) == field)
		{
			m_staticAnimFields.erase(it);
			break;
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Update the fields managed by this component
*/
bool NdhsCComponent::updateFields(LcTTime timestamp)
{
	bool reschedule = false;

	// Now check the static animation fields
	if (m_outerGroup && m_staticAnimFields.size())
	{
		bool finalUpdate = false;
		LcTScalar oldVal = 0;
		LcTScalar newVal;

		LcTmOwnerArray<NdhsCScrollPosField>::iterator it = m_staticAnimFields.begin();

		for (; it != m_staticAnimFields.end(); it++)
		{
			if (m_stopStaticAnimationAll || m_stopStaticAnimationItem)
			{
				oldVal = (*it)->getRawFieldData(NULL);
				oldVal = oldVal - (int)oldVal;
			}

			(*it)->updateValue(timestamp, finalUpdate);

			if (m_stopStaticAnimationAll ||
				(m_stopStaticAnimationItem && (*it != m_pStaticAnimFieldTrigger)))
			{
				newVal = (*it)->getRawFieldData(NULL);
				newVal = newVal - (int)newVal;

				// this animator should be stopped on a 'wrap'.
				if (newVal < oldVal)
					(*it)->setValue(0, true);
				else
					reschedule |= !finalUpdate;
			}
			else
			{
				// We want another timestep if any of the static animators are still going;
				reschedule |= !finalUpdate;
			}
		}

		// If all animations have completed, clear them up.
		if (!reschedule)
			stopStaticAnimations(true, EAnimationAll);
	}

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
	Reset cached data pointing into a transition agent layout
*/
void NdhsCComponent::resetTransitionCache()
{
	if (m_outerGroup)
	{
		// outer group
		m_outerGroup->resetTransitionCache();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCComponent::jumpTransitionToEnd(bool setIdle)
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return;

	m_jumpingToEnd = true;

#ifdef LC_USE_STYLUS
	if (setIdle)
	{
		// Cancel any current drag operation
		m_pageManager->onMouseCancel(this, NULL, false);
		m_mouseFocusElement = NULL;
	}
#endif

	// Execute cleanup trigger if necessary
	if ((m_animType != ENdhsAnimationTypeNone) && (m_animType != ENdhsAnimationTypeDrag))
	{
		if (m_pAnimTriggerList && m_pAnimTriggerList->hasCleanupTrigger)
		{
			// Let this happen even when setIdle is true.  It could start
			// a transition, but the cleanup trigger was intended for
			// sending a sync link to a plugin to tell it to cleanup

			// Do this trigger
			simulateKeyDown(m_pAnimTriggerList->cleanupTrigger);
		}

		m_pAnimTriggerList = NULL;
	}

	// Interrupt - Jump any previous transition to the end
	switch (m_animType)
	{
		case ENdhsAnimationTypeInteractiveState:
		{
			// If we get here, we are interrupting the transition
			// so the chained action will not execute
			m_chainedAction = "";

			// Set the layout field to 'end'
			m_layoutPos->setValue(1.0, true);

			// Now do the 'on transition complete' step.
			onTransitionComplete(setIdle);

			break;
		}

		case ENdhsAnimationTypeStatic:
		{
			if (setIdle)
			{
				// Need to stop any static decorations
				stopStaticAnimations(true, EAnimationAll);
			}

			break;
		}

		default:
			break;
	}

	m_jumpingToEnd = false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCComponent::changeState(ENdhsPageState state, int activeSlot, const NdhsCMenuComponentTemplate::CAction::CAttempt* attempt, bool animate)
{
	ENdhsAnimationType animType = ENdhsAnimationTypeInteractiveState;
	ENdhsVelocityProfile velocityProfile = ENdhsVelocityProfileUnknown;
	int duration = 0;
	int delay = 0;
	bool willAnimate = false;

	// If we are not ready no need to go further
	if (isComponentRetired() || !m_outerGroup)
		return false;

	// Do not do any post-transition complete processing
	if(m_bRequiresPostTransitionComplete)
		m_bRequiresPostTransitionComplete = false;

	// Do we need to interrupt a transition?
	switch (m_animType)
	{
		// No animation, or scrolling - in the latter case, a decision
		// to interrupt transitions is taken in processTrigger
		case ENdhsAnimationTypeScroll:
		case ENdhsAnimationTypeDrag:
		case ENdhsAnimationTypeScrollKick:
		case ENdhsAnimationTypeStatic:
		case ENdhsAnimationTypeNone:
			break;

		// All other transitions
		default:
			jumpTransitionToEnd(false);
			break;
	}

	if (state == ENdhsPageStateOpen)
	{
		animType = ENdhsAnimationTypeTerminalState;
	}

	// Reset the static animation flag
	m_staticTransitionDone = false;

	// Check if we are doing the static animations
	if (state == ENdhsPageStateNone
		&& (animType != ENdhsAnimationTypeScroll)
		&& (animType != ENdhsAnimationTypeDrag)
		&& (animType != ENdhsAnimationTypeScrollKick))
	{
		animType = ENdhsAnimationTypeStatic;

		if (animate == true)
			m_staticTransitionDone = true;
	}

	// Note that we should postpone any layout changes until after we've completed the start transition call
	m_preparingTransition = true;

	m_transitionDuration = duration;
	m_transitionDelay = delay;
	m_transitionVelProfile = velocityProfile;

	// Note the special case where we're kicking again...in that case, we're
	// already transitioning, and should not call startTransition again.
	if ((animType != ENdhsAnimationTypeScrollKick) || (m_animType != ENdhsAnimationTypeScrollKick))
		willAnimate = startTransition(animType, animate);
	else
		willAnimate = true;

	// If no animators have started, the transition
	// must be complete
	if (!willAnimate)
	{
		onTransitionComplete(false, true);
	}
#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
	else
	{
		if (m_isEligible == false)
		{
			m_isEligible = true;
			getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_STARTED);
		}
	}
#endif

	// Now check for any pending layout change
	m_preparingTransition = false;
	if (m_pendingLayout)
	{
		// This should set up the new transition, cancelling the current if necessary
		layoutChanged(m_pendingLayout, m_pendingLayoutAnimateFlag);
		m_pendingLayout = NULL;
	}

	// We tell the caller if we're in a terminal state transition or not
	return (m_animType == ENdhsAnimationTypeTerminalState);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCComponent::onTransitionComplete(bool setIdle, bool generateSignal, bool forceSlotUpdate)
{

	LC_UNUSED(forceSlotUpdate);

#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
	if (m_isEligible == true && generateSignal)
	{
		m_isEligible = false;
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_COMPLETE);
	}
#endif

#ifdef IFX_GENERATE_SCRIPTS
	if (!m_bRequiresPostTransitionComplete)
		if (NdhsCScriptGenerator::getInstance())
				NdhsCScriptGenerator::getInstance()->cancelCaptureMove();
#endif

	// If outer group not created not need to go further
	if (!m_outerGroup)
		return;

	// Stop static animation...the group affected depends on
	// whether we're stopping a scroll or a state transition
	// If setting idle, just stop the lot
	if (!setIdle && (m_animType == ENdhsAnimationTypeScroll || m_animType == ENdhsAnimationTypeDrag
			|| m_animType == ENdhsAnimationTypeScrollKick))
		m_staticAnimGroup = EAnimationItemOnly;
	else
		m_staticAnimGroup = EAnimationAll;

	stopStaticAnimations(true, m_staticAnimGroup);

	m_stateChangeAnimComplete = true;
	m_lastFrame = false;

	// Position the page and menu in the final location
	switch (m_animType)
	{
		case ENdhsAnimationTypeTerminalState:
		case ENdhsAnimationTypeInteractiveState:
		{
			// Final frame!
			updatePosition(1.0, true, false, true);
			m_animType = ENdhsAnimationTypeNone;

			break;
		}

		default:
		{
			m_animType = ENdhsAnimationTypeNone;

			break;
		}
	}

	// Check to see if there is a chained action, or a static animation that might need
	// to be started, or a focus element that needs updating.
	if (!setIdle
		&& ((!m_chainedAction.isEmpty())
		|| ((m_staticTransitionDone == false) && (m_jumpingToEnd == false))
		))
	{
		// Ok...we need to do some more actions
		m_bRequiresPostTransitionComplete = true;

		// Request that the Page Manager schedule the post-TC event
		schedulePostTransitionComplete();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::onMessage(int iID, int iParam)
{
	LcTaString emptyString = "";
	switch (iID)
	{
		case EPostTransitionCompleteMsg:
		{
			postTransitionComplete();
			break;
		}
		case ESimulateKeyMsg:
		{
			getPageManager()->translateKeyCode(iParam);
			processTrigger(iParam,-1,emptyString,NULL,false);
			break;
		}
		default:
			break;
	}
}

/*-------------------------------------------------------------------------*//**
	Simulate a key trigger
*/
void NdhsCComponent::simulateKeyDown(int c)
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
*/
LC_EXPORT void NdhsCComponent::postTransitionComplete()
{
	// Only do anything if we've recorded that we need to...
	// And we have valid field cache (can be invalid in case
	// of component relaoding/unloading)
	if (!m_bRequiresPostTransitionComplete || !m_fieldCache || !m_outerGroup)
		return;

	volatile int memError = 0;

	m_isLayoutChange = false;

	// check we are tweening between layouts?
	NdhsCField* field = m_fieldCache->getField("_transitioning");
	if (field)
	{
		field->setValueBool(false, true);
	}

	// Note we no longer need to perform any actions.
	m_bRequiresPostTransitionComplete = false;

	// Prepare cleanup frame
	LC_CLEANUP_PUSH_FRAME(memError);

	if (memError == 0)
	{
		// Now do the chained action
		if (!m_chainedAction.isEmpty())
		{
			executeChainedAction(m_chainedAction, m_chainedActionSlot);
		}

		// Only carry on if a chained action didn't start a transition, or schedule another post transition complete
		// message (as it would if the chained action duration was 0ms)
		if (!m_bRequiresPostTransitionComplete && ((m_animType == ENdhsAnimationTypeNone) || (m_animType == ENdhsAnimationTypeStatic)))
		{
			if ((m_staticTransitionDone == false) && (m_jumpingToEnd == false))
			{
				// Start the static transition, but never when the transition completes
				// because we jumped to the end of a transition
				(void)changeState(ENdhsPageStateNone, -1, NULL, true);
			}
		}
	}
	else
	{
		// This should not be reached, as the memory needed should have been factored in
		// to the memory headroom calculation
		LC_ASSERT(false);

		// State may be partially changed, so dangerous to continue
		LC_CLEANUP_THROW(IFX_ERROR_RESTART);
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memError);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCComponent::processKeyUp(int code)
{
	if (m_focusedChildComponent)
	{
		m_focusedChildComponent->processKeyUp(code);
	}
	else if (m_focusedElement)
	{
#ifdef IFX_USE_PLUGIN_ELEMENTS
		m_focusedElement->processKeyUpEvent(code);
#endif
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCComponent::processTrigger(	int code,
												int slot,
												LcTmString& elementClass,
												LcTmArray<NdhsCTemplate::CAction::CAttempt*>* optionsAttempts,
												bool fromModule)
{
	volatile int memError = 0;
	bool bSkipToEnd = false;
	bool bConsumed = false;

	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return false;
	
	// Prepare cleanup frame
	LC_CLEANUP_PUSH_FRAME(memError);

	if (memError == 0)
	{
		int localCode = code;

		if ((code != ENdhsNavigationStylusTap)
			&& (code != ENdhsNavigationOptionsMenu)
			&& (code != ENdhsNavigationOpenLink)
			&& (code != ENdhsNavigationChainAction))
		{
#ifdef IFX_USE_PLUGIN_ELEMENTS
			if (m_focusedElement)
			{
				if (m_focusedElement->processKeyDownEvent(localCode))
				{
					bConsumed = true;
					bSkipToEnd = true;
				}
			}
#endif

			if (!bConsumed && m_focusedChildComponent)
			{
				if (m_focusedChildComponent->processTrigger(code, slot, elementClass, optionsAttempts, fromModule))
				{
					bConsumed = true;
					bSkipToEnd = true;
				}
			}
		}

		LcTmArray<NdhsCTemplate::CAction::CAttempt*>* attempts = NULL;

		if(bSkipToEnd == false)
		{
			if (localCode == ENdhsNavigationOptionsMenu
				|| localCode == ENdhsNavigationOpenLink
				|| localCode == ENdhsNavigationChainAction)
			{
				attempts = optionsAttempts;
			}
			else if (localCode == ENdhsNavigationStylusTap)
			{
				if (slot == -1)
				{
					// furniture/detail are identified by class, so slot will be -1
					bConsumed = m_stateManager->processClassTap(elementClass);
				}
				else
				{
					// item tap, identified by the slot number
					bConsumed = m_stateManager->processSlotTap(slot,elementClass);
				}
			}
			else if (localCode == ENdhsNavigationOnSignal)
			{
				// Check for trigger on slot and className
				if (slot >= 0 && elementClass.length() > 0)
				{
					bConsumed = m_stateManager->processSlotClassNameSignal(slot, elementClass);
				}

				if (bConsumed == false)
				{
					// In case of deep hierarchies we get slot number, but we might not have onsginal against that
					// Slot so signal will be missed, therefor check that of class also.
					if (slot != -1)
					{
						bConsumed = m_stateManager->processSlotSignal(slot);
					}
					if (bConsumed == false)
					{
						bConsumed = m_stateManager->processClassSignal(elementClass);
					}
				}
			}
			else
			{
				// Must be a key press, identified by an internal code for
				// special predefined keys or a scan code
				bConsumed = m_stateManager->processKeypress(localCode);
			}

			if (bConsumed == false && attempts!= NULL)
			{
				// Hanlde link executed from Modules by callbacks
				if (localCode == ENdhsNavigationOpenLink)
				{
					typedef LcTmArray<NdhsCTemplate::CAction::CAttempt*> TmAAttempts;
					TmAAttempts::iterator itA = attempts->begin();

					for (; itA != attempts->end(); itA++)
					{
						doAttempt(*itA, slot);
					}
				}
			}
		}
	}
	else
	{
		// This should not be reached, as the memory needed should have been factored in
		// to the memory headroom calculation
		LC_ASSERT(false);

		// State may be partially changed, so dangerous to continue
		LC_CLEANUP_THROW(IFX_ERROR_RESTART);
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memError);

	// NOTE: Nothing should be done down here

	return bConsumed;

}

/*-------------------------------------------------------------------------*//**
	Executes a chained action
*/
bool NdhsCComponent::executeChainedAction(const LcTmString& action, int slotNum)
{
	bool retVal = false;

	if (!action.isEmpty() && m_outerGroup)
	{
		NdhsCTemplate::CAction * pActionObj= m_template->getAction(action);

		if(pActionObj)
		{
			doAction(pActionObj, slotNum);
		}
		else
		{
			m_chainedAction = "";
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::fieldValueUpdated(NdhsCField* field)
{
	// if outer group not created for some reason no need to go further
	if (m_isCreated == false || !m_outerGroup) 
		return;

	// Check static trigger
	if (field == m_pStaticAnimFieldTrigger && m_pStaticTriggerList)
	{
		LcTScalar val = m_pStaticAnimFieldTrigger->getRawFieldData(NULL);
		LcTScalar frac = val - (int)val;

		// Check for endpoint of static animation...
		if (m_pStaticAnimFieldTrigger->atRest())
			frac = 1.0;

		// Check for looping
		if (frac < m_lastStaticTriggerPos)
		{
			m_lastStaticTriggerPos = -1;
		}

		if (m_stopStaticAnimationAll && (m_pStaticAnimFieldTrigger->atRest() == false))
		{
			// Execute cleanup trigger
			if (m_pStaticTriggerList && m_pStaticTriggerList->hasCleanupTrigger)
				simulateKeyDown(m_pStaticTriggerList->cleanupTrigger);

			// Stop us receiving any further updates - note that we can't destroy the field, as
			// we're being called by it a the moment!
			m_pStaticAnimFieldTrigger->setTargetValue(val, 0);
		}
		else
		{
			int listSize = m_pStaticTriggerList->triggerList.size();
			int i = 0;

			if (listSize > 0)
			{
				while ((i < listSize) &&
						(m_pStaticTriggerList->triggerList[i]->position <= m_lastStaticTriggerPos))
				{
					i++;
				}

				if ((i < listSize) && (m_pStaticTriggerList->triggerList[i]->position <= frac))
				{
					// Do this trigger
					simulateKeyDown(m_pStaticTriggerList->triggerList[i]->key);
					m_lastStaticTriggerPos = m_pStaticTriggerList->triggerList[i]->position;
				}
				else
				{
					m_lastStaticTriggerPos = frac;
				}
			}
		}
	}
	else if (field && field == m_focusEnabledField)
	{
		if (m_inFocus && field->getRawFieldDataBool(NULL) == false)
		{
			globalUnsetFocus();
		}
	}
	else
	{
		NdhsCElementGroup::fieldValueUpdated(field);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::setVisible(bool b)
{
	// Ensure local screen agg always has same visibility as page
	if (m_localScreenAggregate)
	{
		m_localScreenAggregate->setVisible(b);
	}

	// Propagate to base class
	NdhsCElementGroup::setVisible(b);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::layoutChanged(NdhsCTemplate::CLayout* newLayout, bool animate)
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return;

	if (m_preparingTransition)
	{
		m_pendingLayout = newLayout;
		m_pendingLayoutAnimateFlag = animate;
		return;
	}

	m_isLayoutChange = true;
	m_currentLayout = newLayout;

	jumpTransitionToEnd(false);

	m_transitionDuration = m_template->getDefaultLayoutTime();
	m_transitionVelProfile = m_template->getDefaultLayoutVelocityProfile();

	stopStaticAnimations(true, EAnimationAll);

	m_staticTransitionDone=false;
	if(startTransition(ENdhsAnimationTypeLayoutChange, animate) && !animate)
	{
		onTransitionComplete(false);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::doAction(NdhsCTemplate::CAction* action, int slotNum)
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup) 
		return;

	// Only cancel any existing chained action if we have something to do
	m_chainedAction = "";

	// Now we have one or more things to attempt: if the first isn't
	// possible, we fall back to the second, and so forth until
	// either we have found something we can do or we have run out
	// of options
	bool attemptOk = false;
	typedef LcTmArray<NdhsCTemplate::CAction::CAttempt*> TmAAttempts;
	TmAAttempts::iterator itA = action->attempts.begin();

	for (; itA != action->attempts.end() && !attemptOk; itA++)
	{
		attemptOk = doAttempt(*itA, slotNum);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCComponent::doAttempt(NdhsCTemplate::CAction::CAttempt* attempt, int slotNum)
{
	// if outer group not created for some reason no need to go further
	if (attempt == NULL || !m_outerGroup)
		return false;

	bool attemptOk = false;

	// What we do depends on attempt type
	switch (attempt->attemptType)
	{
		// EScrollBy shifts field by the specified amount
		//
		//    amount : offset to change field by
		//    wrap : whether to 'wrap' between min and max value
		case NdhsCTemplate::CAction::CAttempt::EScrollBy:
		{
			NdhsCTemplate::CAction::CScrollBy* scrollAction = (NdhsCTemplate::CAction::CScrollBy*)(attempt);
			bool isKick = (scrollAction->type == NdhsCTemplate::CAction::CScrollBy::EScrollByTypeKick);
			ENdhsFieldDirection dir = (scrollAction->amount > 0) ? ENdhsFieldDirectionIncreasing : ENdhsFieldDirectionDecreasing;

			// We must be scrolling a named field
			if (scrollAction->field.compareNoCase("_scrollpos") != 0)
			{
				// Get the field value object
				NdhsCField* pField = getFieldValue(scrollAction->field, NULL, -1, NULL);

				if (pField)
				{
					LcTScalar currentValue = pField->getRawFieldData(NULL);
					LcTScalar targetValue = pField->getTargetValue();

					LcTScalar minValue = 0;
					bool minDefined = false;

					LcTScalar maxValue = 0;
					bool maxDefined = false;

					if (scrollAction->minDefined && scrollAction->minExpr)
					{
						NdhsCExpression* minExpr = scrollAction->minExpr->getContextFreeExpression();

						if (minExpr)
						{
							minExpr->evaluate(this, slotNum, NULL);
							minValue = minExpr->getValueScalar();
							minDefined = true;
						}
					}

					if (scrollAction->maxDefined && scrollAction->maxExpr)
					{
						NdhsCExpression* maxExpr = scrollAction->maxExpr->getContextFreeExpression();

						if (maxExpr)
						{
							maxExpr->evaluate(this, slotNum, NULL);
							maxValue = maxExpr->getValueScalar();
							maxDefined = true;
						}
					}

					// A scroll will succeed provided that we're not at one of the limits and the scroll would take us
					// into out of bounds territory
					if ( ((dir == ENdhsFieldDirectionIncreasing) && (!maxDefined || currentValue < maxValue) )
							|| ((dir == ENdhsFieldDirectionDecreasing) && (!minDefined || currentValue > minValue)) )
					{
						attemptOk = true;
					}
					else
					{
						attemptOk = false;
					}

					if (isKick && (scrollAction->duration > 0))
					{
						// Kick scroll - we either start a kick scroll, add to an existing kick scroll, or cancel a kick scroll
						// depending on the state of the field
						if (pField->atRest())
						{
							// Start kick
							if (minDefined && maxDefined)
							{
								pField->setTargetValue(currentValue + scrollAction->amount, minValue, maxValue, scrollAction->duration);
							}
							else
							{
								pField->setTargetValue(currentValue + scrollAction->amount, scrollAction->duration);
							}

							// We need to start animating to allow the field to update
							m_pageManager->startAnimation();
						}
						else
						{
							// Field already kicked...if this kick is in the same direction we increase the kick, otherwise
							// we stop it dead.
							if ( ((dir == ENdhsFieldDirectionIncreasing) && (pField->getVelocity() > 0))
								|| ((dir == ENdhsFieldDirectionDecreasing) && (pField->getVelocity() < 0)))
							{
								// Start kick
								if (minDefined && maxDefined)
								{
									pField->setTargetValue(targetValue + scrollAction->amount, minValue, maxValue, scrollAction->duration);
								}
								else
								{
									// Set target value
									pField->setTargetValue(targetValue + scrollAction->amount, scrollAction->duration);
								}
							}
							else
							{
								// Stop field changing.
								if (minDefined && maxDefined)
								{
									pField->setValue(min(maxValue, max(minValue, currentValue)), true);
								}
								else
								{
									pField->setValue(currentValue, true);
								}
							}
						}
					}
					else if (scrollAction->duration > 0)
					{
						// Field already animating...
						if (pField->atRest() == false)
							pField->setValue(pField->getTargetValue(), false);

						// Start
						if (minDefined && maxDefined)
						{
							pField->setTargetValue(pField->getRawFieldData(NULL) + scrollAction->amount, minValue, maxValue, scrollAction->duration);
						}
						else
						{
							pField->setTargetValue(pField->getRawFieldData(NULL) + scrollAction->amount, scrollAction->duration);
						}

						// We need to start animating to allow the field to update
						m_pageManager->startAnimation();
					}
					else
					{
						// Instantaneous field change
						LcTScalar newValue;

						if (pField->atRest() == false)
						{
							// We will interrupt any kick in progress...note that we don't want to
							// call 'setValue' here as that would notify all observers of a change
							// and we're going to do that shortly anyway
							LcTScalar target = pField->getTargetValue();

							// Obey limits (as target may be outside range)
							if (maxDefined && target > maxValue)
								target = maxValue;
							else if (minDefined && target < minValue)
								target = minValue;

							newValue = target + scrollAction->amount;
						}
						else
						{
							newValue = currentValue + scrollAction->amount;
						}

						// ensure that max/min limits obeyed
						if (maxDefined && newValue > maxValue)
							newValue = maxValue;
						else if (minDefined && newValue < minValue)
							newValue = minValue;

						// Apply the change in value - note that wrapping is not permitted, and that the 'finalUpdate' flag
						// should be set to 'true' so that modules are notified.
						pField->setValue(newValue, true);
					}
				}
			}
			if (attemptOk)
			{
				m_chainedAction = scrollAction->action;
			}
			break;
		}

		// EBack
		//
		//    page : Page to go back to
		//    parentAction : Action to trigger on parent page
		case NdhsCMenuComponentTemplate::CAction::CAttempt::EBack:
		{
			NdhsCPageModel* page = getParentPageRef();

			if(page)
			{
				ENdhsPageState pageState = page->getPageState();

				if (pageState == ENdhsPageStateInteractive
					|| pageState == ENdhsPageStateSelected
					|| pageState == ENdhsPageStateHide)
				{
					if (pageState != ENdhsPageStateInteractive
						&&((NdhsCPageTemplate::CAction::CBack*)(attempt))->page.isEmpty())
					{
						// Need to do a back to this page
						((NdhsCPageTemplate::CAction::CBack*)(attempt))->backToLevel = m_pageManager->getPageStackLevel(page);

						m_chainedAction = ((NdhsCPageTemplate::CAction::CBack*)(attempt))->action;
					}
					else
					{
						((NdhsCPageTemplate::CAction::CBack*)(attempt))->backToLevel = -1;
					}

					attemptOk = m_pageManager->processAttempt(attempt, m_stackLevel);
				}
			}
			break;
		}

		case NdhsCTemplate::CAction::CAttempt::ESignal:
		{
			attemptOk = m_parent->processTrigger(ENdhsNavigationOnSignal,
										getSlot(),
										m_componentName,
										NULL,
										false);
			break;
		}

		// ELink
		//
		//    uri : The link to launch in the browser
		case NdhsCTemplate::CAction::CAttempt::ELink:
		{
			bool moduleLink = false;

			LcTaString link = ((NdhsCTemplate::CAction::CLink*)(attempt))->uriString;

			NdhsCExpression::CExprSkeleton* uriCFExpr = (((NdhsCTemplate::CAction::CLink*)(attempt))->uri).ptr();
			if (link.isEmpty())
			{
				if (uriCFExpr)
				{
					NdhsCExpression* uriExpr = uriCFExpr->getContextFreeExpression();
					if (uriExpr)
					{
						uriExpr->evaluate(this, -1, NULL);
						link = uriExpr->getValueString();
					}
				}
			}
			else
			{
				moduleLink = true;
			}

			LcTaString expandedLink;
			LcTaString linkPrefix;

			// Check for modules trying to navigate via the IFXI_RequestExecuteLink route
			if (moduleLink)
			{
				bool linkValid = m_pageManager->isValidURIToExecuteFromCallBack(link);

				// We entry point ID, or a module link
				if (linkValid && link.find("://") == -1)
				{
					// We got an entry point ID
					link = m_pageManager->getEntryPointMapStack()->getEntryPoint(link);
				}
			}

			// Token replace the URI
			m_pageManager->getTokenStack()->replaceTokens(link,	expandedLink, NULL, NULL, NULL, this, m_stackLevel);

			// Determine the link prefix
			linkPrefix = expandedLink.subString(0, expandedLink.find(":"));

			ENdhsLinkType linkType = m_pageManager->getCon()->getTRLinkType(linkPrefix);

			switch (linkType)
			{
				case ENdhsLinkTypeSyncLinkPlugin:
				{
					// The link will not result in a menu being opened, so we
					// should just fire it now without jumping to end

					// Need to pass in a menu item, so create one
					LcTaOwner<NdhsCMenuItem> mi = NdhsCMenuItem::create(NULL);
					mi->setLinkAttr(expandedLink);

					// launch the link
					attemptOk = m_pageManager->getCon()->launchLink(mi.ptr(), m_stackLevel);

					if(attemptOk)
					{
						executeChainedAction(((NdhsCMenuComponentTemplate::CAction::CLink*)(attempt))->action, slotNum);
					}

					break;
				}

				case ENdhsLinkTypeTheme:
				{
					// Interrupt any inter-page animations
					m_pageManager->jumpTransitionToEnd();

					// Need to pass in a menu item, so create one
					LcTaOwner<NdhsCMenuItem> mi = NdhsCMenuItem::create(NULL);

					mi->setLinkAttr(expandedLink);

					// launch the link
					attemptOk = m_pageManager->getCon()->launchLink(mi.ptr(), m_stackLevel);

					break;
				}

				// If the link is unknown, ignore
				case ENdhsLinkTypeUnknown:
					break;

				// Otherwise pass on to page manager
				default:
				{
					// The link will open a menu, so we must stop the transition
					// and ask the page manager to handle the link
					if (m_animType != ENdhsAnimationTypeNone)
						jumpTransitionToEnd(false);

					attemptOk = m_pageManager->processAttempt(attempt, m_stackLevel, expandedLink);
				}
			}
			break;
		}

		// ESuspend
		//
		//    suspend : Put Ndhs to the background (soft exit)
		case NdhsCTemplate::CAction::CAttempt::ESuspend:
		{
			attemptOk = m_pageManager->processAttempt(attempt, m_stackLevel);

			break;
		}

		// EExit
		//
		//    exit : Exit Ndhs (hard exit)
		case NdhsCTemplate::CAction::CAttempt::EExit:
		{
			attemptOk = m_pageManager->processAttempt(attempt, m_stackLevel);

			break;
		}

		// ESetFocus
		//
		//    setFocus : Set the focus to the current item.
		case NdhsCTemplate::CAction::CAttempt::ESetFocus:
		{
			LcTaString className = ((NdhsCTemplate::CAction::CSetFocus*)attempt)->className;
			if (className.isEmpty())
			{
				attemptOk = trySetFocus();
			}
			else if (className.compareNoCase("_parent") == 0)
			{
				if (m_parent)
				{
					NdhsCComponent* parentComponent = m_parent->findParentComponent();

					if (parentComponent)
					{
						attemptOk = parentComponent->trySetFocus();
					}
					else
					{
						NdhsCPageModel* pageModel = m_parent->findParentPageModel();

						if (pageModel)
						{
							attemptOk = pageModel->trySetFocus();
						}
					}
				}
			}
			else
			{
				attemptOk = trySetFocus(className);
			}

			if (attemptOk)
			{
				executeChainedAction(((NdhsCTemplate::CAction::CSetFocus*)attempt)->action, slotNum);
			}

			break;
		}

		// EUnsetFocus
		//
		//    unsetFocus : unset the current focus.
		case NdhsCTemplate::CAction::CAttempt::EUnsetFocus:
		{
			globalUnsetFocus();
			attemptOk = true;
#ifdef NDHS_JNI_INTERFACE
			NDHS_TRACE(ENdhsTraceLevelInfo, ENdhsTraceModuleGeneral, "Focus change: focus now unset");
#endif

			break;
		}

		// EMoveFocus tabs the focus by the specified amount.
		//
		//    amount : offset to tab by
		//    wrap : whether to 'wrap' between 1 and the maximum tab count.
		case NdhsCPageTemplate::CAction::CAttempt::EMoveFocus:
		{
			NdhsCPageModel* pageModel = findParentPageModel();

			int amount = ((NdhsCPageTemplate::CAction::CTabFocus*)(attempt))->amount;
			bool wrap	= ((NdhsCPageTemplate::CAction::CTabFocus*)(attempt))->wrap;

			if (amount != 0)
			{
				attemptOk = pageModel->moveFocus(amount, wrap);

				if (attemptOk)
				{
					executeChainedAction(((NdhsCPageTemplate::CAction::CTabFocus*)(attempt))->action, slotNum);
				}
			}

			break;
		}

		// EStop
		//
		//    Just consume the event, but do nothing
		case NdhsCTemplate::CAction::CAttempt::EStop:
		{
			attemptOk = true;

			break;
		}

		default:
			break;
	}

	if (!m_chainedAction.isEmpty())
	{
		m_chainedActionSlot = slotNum;
	}

	return attemptOk;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageModel* NdhsCComponent::getParentPageRef()
{
	if (m_parent)
		return m_parent->getParentPageRef();
	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::stateManagerDestroyed(NdhsCStateManager* sm)
{
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCComponent::getField(const LcTmString& field, int slotNum, NdhsCMenuItem* item)
{
	NdhsCField* retVal = NULL;

	if (m_fieldCache)
	{
		retVal = m_fieldCache->getField(field);
	}

	if (!retVal && m_pageManager)
	{
		// Look in global namespace
		NdhsCPlugin* plugin = m_pageManager->getPlugin();

		if (plugin)
		{
			retVal = plugin->getField(field, NULL, -1, NULL);
		}
	}

	// check up parent stack in placeholder case...
	if (field.compareNoCase("_placeholderActive") == 0)
	{
		retVal = m_parent->getField(field, slotNum, item);
	}

	if (!retVal)
	{
		// Check for page paramters
		retVal = m_parent->getPageParamValue(field);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::reloadElement()
{
	// Load the classes.
	prepareComponent();
	reloadFurniture();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::prepareComponent()
{
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::reloadFurniture()
{

	if (m_outerGroup)
	{
		m_outerGroup->retire();
		m_outerGroup.destroy();
	}

	if(m_localScreenAggregate)
	{
		m_localScreenAggregate->retire();
		m_localScreenAggregate.destroy();
	}

	m_localScreenAggregate = m_template->createElementGroup(this,
																						getMenu(),
																						getMenuItem(),
																						"screen",
																						m_stackLevel,
																						getDrawLayerIndex(),
																						this);
	// Create the furniture element group
	LcTaOwner<NdhsCElementGroup> furnitureElementGroup = m_template->createElementGroup(this,
																						getMenu(),
																						getMenuItem(),
																						"page",
																						m_stackLevel,
																						getDrawLayerIndex(),
																						this);
	furnitureElementGroup->setVisible(true);
	m_outerGroup = m_template->createElementGroup(this,
																						getMenu(),
																						getMenuItem(),
																						"outergroup",
																						m_stackLevel,
																						getDrawLayerIndex(),
																						this);
	m_outerGroup->setVisible(true);
	m_outerGroup->setElementGroup(this);
	furnitureElementGroup->setElementGroup(m_outerGroup.ptr());
	furnitureElementGroup->realize(m_outerGroup.ptr());
	m_outerGroup->addGroup("furniture", furnitureElementGroup);
	m_outerGroup->realize(this);

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::updatePosition(LcTScalar position,
										bool positionIncreasing,
										bool updateCache,
										bool finalFrame)
{
	LcTPlacement placement;
	LcTScalar animationFrac = position;

	// Should we force non-sketchy drawing?
	bool forceNonSketchy = false;

	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return;

	// check we are tweening between layouts?
	if (m_isLayoutChange == true)
	{
		NdhsCField* field = m_fieldCache->getField("_transitioning");
		if (field)
		{
			field->setValueBool(true, true);
		}
	}

	// Apply any triggers that have fired in the last frame
	if (m_pAnimTriggerList && (m_animType != ENdhsAnimationTypeDrag))
	{
		// Check for looping
		if (animationFrac < m_lastTriggerPos)
		{
			m_lastTriggerPos = -1;
		}

		int listSize = m_pAnimTriggerList->triggerList.size();
		int i = 0;

		if (listSize > 0)
		{
			while ((i < listSize) &&
					(m_pAnimTriggerList->triggerList[i]->position <= m_lastTriggerPos))
			{
				i++;
			}

			if ((i < listSize) && (m_pAnimTriggerList->triggerList[i]->position <= animationFrac))
			{
				// Do this trigger
				simulateKeyDown(m_pAnimTriggerList->triggerList[i]->key);
				m_lastTriggerPos = m_pAnimTriggerList->triggerList[i]->position;
			}
			else
			{
				m_lastTriggerPos = animationFrac;
			}
		}
	}

	if (m_outerGroup)
	{
		// Position the furniture group
		m_outerGroup->updatePosition(getAnimType(), animationFrac, positionIncreasing, updateCache,
																forceNonSketchy, false, finalFrame);
	}

	// Screen aggregate position
	if(m_localScreenAggregate)
	{
		((NdhsCElementGroup*)m_localScreenAggregate.ptr())->updatePosition(getAnimType(), animationFrac, positionIncreasing, updateCache,
															false, false, finalFrame);
	}
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::populateElementList(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList)
{
	populateWidgetElementMap(widgets, pageWidgetElemList);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::populateWidgetElementMap(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList)
{
	if (m_outerGroup)
	{
		// Add the list of furniture elements
		m_outerGroup->populateElementList(widgets, pageWidgetElemList);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCComponent::onMouseDown(NdhsCPageManager::TPageWidgetElem* entry, const LcTPixelPoint& pt)
{
	// We must ask the element for the widget because a
	// field refresh might have changed the original
	// widget pointer

	bool bConsumed = false;

	m_bMouseDown = true;

	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return false;

	if (entry && entry->element)
		bConsumed = entry->element->onMouseDown(pt);

	if (bConsumed)
		m_mouseFocusElement = entry->element;

	return bConsumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCComponent::onMouseMove(NdhsCPageManager::TPageWidgetElem* entry, const LcTPixelPoint& pt)
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return false;

	// We must ask the element for the widget because a
	// field refresh might have changed the original
	// widget pointer

	bool bConsumed = false;

	if (entry && entry->element)
	{
		bConsumed = entry->element->onMouseMove(pt);
	}

	if (!m_mouseFocusElement && bConsumed)
		m_mouseFocusElement = entry->element;

	return bConsumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCComponent::onMouseUp(NdhsCPageManager::TPageWidgetElem* entry, const LcTPixelPoint& pt)
{
	// We must ask the element for the widget because a
	// field refresh might have changed the original
	// widget pointer

	bool bConsumed = false;
	LcCWidget* widget = NULL;
	bool triggersAllowed = true;
	bool widgetHit = false;

	m_bMouseDown = false;

	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return false;

	if (entry && entry->element)
	{
		widget = entry->element->getWidget();

		// Check to see if widget should handle mouse up - do this before
		// notifying the element of the mouse up in case there's a touchdown
		// displacement.
		if (widget && getSpace())
		{
			LcTVector loc;

			// Get the local co-ordinates.
			loc = getSpace()->mapCanvasToLocal(pt, *widget);

			LcTScalar expandBorder = widget->getTapTolerance();

			// Check to see if this point is within the widget.
			if (widget->contains(loc, expandBorder))
			{
#if defined(LC_PLAT_OGL_20)
				// Calculate the tap position.
				widget->calcTapPosition(pt);
#endif
				widgetHit = true;
			}
		}

		bConsumed = entry->element->onMouseUp(pt);

		// Do not call process triggers if the element is untappable
		triggersAllowed = entry->element->isTappable();
	}

	// The element did not consume the event, try the page
	if (!bConsumed && triggersAllowed)
	{
		LcTaString elementClass;

		// Check that the widget is valid, and that we have a space.
		if (widget && widgetHit)
		{
			// check furniture first for selected widget
			elementClass = m_outerGroup->getClassFromWidget(widget);
		}

		// This is executed even if selectedSlot and elementClass are not populated.
		// The may execute the catch all case if necessary
		bConsumed = processTrigger(ENdhsNavigationStylusTap, -1, elementClass, NULL, false);
	}

	if (m_mouseFocusElement)
		m_mouseFocusElement = NULL;

	updateTouchDownIntrinsicField("", -1);

	return bConsumed;
}
#endif

/*-------------------------------------------------------------------------*//**
	Check whether we need to change the visibility status of the group
*/
bool NdhsCComponent::doPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame)
{
	// if outer group not created for some reason no need to go further
	if (!m_outerGroup)
		return false;

	bool reschedule = false;
	bool finalUpdate = false;

	// Note if this is the last frame
	if (m_animType == ENdhsAnimationTypeNone)
		finalFrame = true;
	else
		finalFrame = m_lastFrame;

	switch (m_animType)
	{
		case ENdhsAnimationTypeInteractiveState:
		{
			if (timestamp >= m_decorationDelayTimestamp)
			{
				m_layoutPos->updateValue(timestamp, finalUpdate);

				LcTScalar currentPos = m_layoutPos->getRawFieldData(NULL);

				// Check for animation complete
				if (m_lastFrame)
				{
					// Transition complete!
					onTransitionComplete(false, true);
					m_lastFrame = false;
				}
				else if (finalUpdate)
				{
					// Next Frame is last frame
					m_lastFrame = true;
					reschedule = true;
				}
				else
				{
					reschedule = true;
				}

				// Now update all the positions
				if (reschedule)
					updatePosition(currentPos, true, false);
			}
			else
			{
				// We're waiting for a transition to start...
				reschedule = true;
			}

			break;
		}

		case ENdhsAnimationTypeStatic:
		{
			reschedule = true;

			break;
		}
		default:
			break;
	}

	// Check fields
	reschedule |= updateFields(timestamp);

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::unLoadGroup()
{
	if (m_outerGroup && m_isEnabled && m_groupUnloaded == false)
	{
		doCleanup();

		m_isEnabled = false;
		m_groupUnloaded = true;
		m_trySetFocus = true;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::loadGroup()
{
	if (m_isEnabled == false && m_groupUnloaded)
	{
		construct();
		
		m_groupUnloaded = false;
		m_outerGroup->loadResources();
		m_localScreenAggregate->loadResources();
		realize(getOwner());

		m_isEnabled = true;

		if (m_trySetFocus)
		{
			NdhsCPageModel* pageModel = m_parent->findParentPageModel();

			if (pageModel)
			{
				pageModel->trySetFocus();
			}
		}		
		m_trySetFocus = false;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::updateTouchDownIntrinsicField(const LcTmString& element, int slot)
{
	LC_UNUSED(slot)

	if (m_fieldCache)
	{
		NdhsCField* field = m_fieldCache->getField("_touchElement");
		if (field)
		{
			field->setValue(element, true);
		}
	}
}

#ifdef LC_USE_MOUSEOVER
/*-------------------------------------------------------------------------*//**
																			 */
void NdhsCComponent::updateMouseOverIntrinsicField(const LcTmString& element, int slot)
{
	if (m_fieldCache)
	{
		NdhsCField* field = m_fieldCache->getField("_mouseoverelement");
		if (field)
		{
			field->setValue(element, true);
		}

		if (m_parent)
		{
			LcTaString componentName = "";
			slot = -1;

			if (element.isEmpty() == false)
			{
				// Get the slot in which component reside
				if (getGroupType() == ENdhsObjectTypeItemComponent)
					slot = getSlot();
				componentName = m_componentName;
			}

			m_parent->updateMouseOverIntrinsicField(componentName, slot);
		}
	}
}
#endif

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCComponent::getPageParamValue(const LcTmString& key)
{
	// Request parent to provide the value
	return m_parent->getPageParamValue(key);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::onSuspend()
{
	if (m_outerGroup)
	{
		m_outerGroup->onSuspend();
	}

	if(m_localScreenAggregate)
	{
		((NdhsCElementGroup*)m_localScreenAggregate.ptr())->onSuspend();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::onResume()
{
	if (m_outerGroup)
	{
		m_outerGroup->onResume();
	}

	if(m_localScreenAggregate)
	{
		((NdhsCElementGroup*)m_localScreenAggregate.ptr())->onResume();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::updateExtentIntrinsicVaraibles(LcTVector extent, int mask)
{
	if ((mask & LcTPlacement::EExtent) == 0 || !m_fieldCache)
		return;

	NdhsCField* field = m_fieldCache->getField("_extentx");

	if (field)
	{
		field->setValue(extent.x, true);
	}

	field = m_fieldCache->getField("_extenty");

	if (field)
	{
		field->setValue(extent.y, true);
	}

	field = m_fieldCache->getField("_extentz");

	if (field)
	{
		field->setValue(extent.z, true);
	}

	// Propagate any changes immediately
	m_pageManager->getCon()->getLaundry()->cleanAll();
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCComponent::trySetFocus()
{
	bool retVal = false;

	if (m_outerGroup && m_focusEnabledField && m_focusEnabledField->getRawFieldDataBool(NULL) && m_outerGroup->visibleOnTransitionComplete())
	{
		if (getTemplate()->isFocusStop())
		{
			globalUnsetFocus();
			retVal = true;
			focusSetOnChild(this, this);
#ifdef NDHS_JNI_INTERFACE
			NDHS_TRACE(ENdhsTraceLevelInfo, ENdhsTraceModuleGeneral, "Focus change: focus now " + getFocusChain());
#endif
		}
		else if (!getTemplate()->getDefaultFocus().isEmpty())
		{
			retVal = trySetFocus(getTemplate()->getDefaultFocus());
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCComponent::trySetFocus(const LcTmString& className)
{
	bool retVal = false;

	if (m_outerGroup && m_focusEnabledField && m_focusEnabledField->getRawFieldDataBool(NULL) && m_outerGroup->visibleOnTransitionComplete())
	{
		NdhsCElement* elementToFocus = getItem(className);

		if (elementToFocus)
		{
			if(elementToFocus==m_focusedElement)
				return true;

			retVal = elementToFocus->trySetFocus();

			if (retVal)
			{
				globalUnsetFocus();
				m_focusedElement = elementToFocus;
				focusSetOnChild(this, this);
			}
#ifdef NDHS_JNI_INTERFACE
			else
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleGeneral, "Tried to set focus to element " + className + " in component " + getGroupName() + " but failed");
			}
#endif
		}
		else
		{
			NdhsCElementGroup* groupToFocus = getGroup(className);

			if (groupToFocus)
			{
				retVal = groupToFocus->trySetFocus();
			}
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::unsetFocus()
{
	if (m_outerGroup)
	{
		setFocusIntrinsics(false, "");

		if (m_focusedElement)
		{
			m_focusedElement->unsetFocus();
			m_focusedElement = NULL;
		}

		if (m_focusedChildComponent)
		{
			m_focusedChildComponent->unsetFocus();
			m_focusedChildComponent = NULL;
		}
	}
	m_inFocus = false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::focusSetOnChild(NdhsCComponent* childComponent, NdhsCComponent* focusComponent)
{
	m_inFocus = true;

	// if outer group not created for some reason no need to go further
	if (!m_outerGroup) 
		return;

	if (childComponent != this)
	{
		m_focusedChildComponent = childComponent;
		setFocusIntrinsics(true, childComponent->getGroupName());
	}
	else
	{
		if (m_focusedElement)
		{
			setFocusIntrinsics(true, m_focusedElement->getElementClassName());
		}
		else
		{
			setFocusIntrinsics(true, "");
		}
	}

	// Notify parents
	if (m_parent)
	{
		NdhsCComponent* parentComponent = m_parent->findParentComponent();

		if (parentComponent)
		{
			parentComponent->focusSetOnChild(this, focusComponent);
		}
		else
		{
			NdhsCPageModel* parentPageModel = m_parent->findParentPageModel();

			if (parentPageModel)
			{
				parentPageModel->focusSetOnChild(this, focusComponent);
			}
			else
			{
				// Error condition - component cannot find parents
				LC_ASSERT(false);
			}
		}
	}
	else
	{
		// Error condition - component has no parent set
		LC_ASSERT(false);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCComponent::moveFocus(int amount, bool wrap)
{
	bool retVal = false;

	if (m_outerGroup && m_focusEnabledField && m_focusEnabledField->getRawFieldDataBool(NULL) && !isHidden())
	{
		int totalTabStops = getTemplate()->getTabOrderCount();
		bool isFocusStop = getTemplate()->isFocusStop();

		if (isFocusStop && !m_focusedElement && !m_focusedChildComponent)
		{
			if (m_inFocus)
			{
				// Component as a whole was focused, cannot move focus internally
				retVal = false;

				// Unset focus so if it wraps around back to us we can accept it
				if (wrap)
				{
					globalUnsetFocus();
				}
			}
			else
			{
				// Component not previously focused, so accept it here
				globalUnsetFocus();
				retVal = true;
				focusSetOnChild(this, this);
#ifdef NDHS_JNI_INTERFACE
				NDHS_TRACE(ENdhsTraceLevelInfo, ENdhsTraceModuleGeneral, "Focus change: focus now " + getFocusChain());
#endif
			}
		}
		else if (totalTabStops > 0)
		{
			NdhsCComponent* previouslyFocusedComponent = m_focusedChildComponent;

			if (m_focusedChildComponent)
			{
				retVal = m_focusedChildComponent->moveFocus(amount, wrap);
			}

			if (!retVal)
			{
				int currentTabIndex = -1;
				int perTabIncrement = (amount > 0) ? 1 : -1;

				if (m_focusedElement)
				{
					ENdhsObjectType objType = m_focusedElement->getElementUse();
					ENdhsGroupType groupType = ENdhsGroupTypeFurniture;

					if (objType == ENdhsObjectTypeItem)
					{
						groupType = ENdhsGroupTypeItem;
					}

					currentTabIndex = getTemplate()->getTabIndex(m_focusedElement->getElementClassName(), groupType);
				}
				else if (previouslyFocusedComponent)
				{
					ENdhsObjectType objType = previouslyFocusedComponent->getGroupType();
					ENdhsGroupType groupType = ENdhsGroupTypeFurniture;

					if (objType == ENdhsObjectTypeItem)
					{
						groupType = ENdhsGroupTypeItem;
					}

					currentTabIndex = getTemplate()->getTabIndex(previouslyFocusedComponent->getGroupName(), groupType);
				}

				int wraps = 0; // To make sure we don't wrap indefinitely
				int nextTabIndex = 0;
				bool cannotMoveFocus = false;

				while (!retVal && !cannotMoveFocus && wraps < 2)
				{
					// Calculate the next Tab Index to look at
					if (currentTabIndex == -1)
					{
						if (!isFocusStop && previouslyFocusedComponent)
						{
							cannotMoveFocus = true;
						}
						else
						{
							if (amount > 0)
							{
								nextTabIndex = 0;
							}
							else
							{
								nextTabIndex = totalTabStops - 1;
							}
						}
					}
					else
					{
						nextTabIndex = currentTabIndex + perTabIncrement;

						if (nextTabIndex < 0)
						{
							if (wrap && isFocusStop)
							{
								nextTabIndex = totalTabStops - 1;
								wraps++;
							}
							else
							{
								cannotMoveFocus = true;
							}
						}
						else if (nextTabIndex >= totalTabStops)
						{
							if (wrap && isFocusStop)
							{
								nextTabIndex = 0;
								wraps++;
							}
							else
							{
								cannotMoveFocus = true;
							}
						}
					}

					if (!cannotMoveFocus)
					{
						NdhsCTemplate::TTabData nextTabData;

						if (getTemplate()->getTabOrderData(nextTabIndex, nextTabData))
						{
							NdhsCElement* elementToFocus = getItem(nextTabData.className);

							if (elementToFocus)
							{
								retVal = elementToFocus->trySetFocus();

								if (retVal)
								{
									globalUnsetFocus();
									m_focusedElement = elementToFocus;
									focusSetOnChild(this, this);
								}
#ifdef NDHS_JNI_INTERFACE
								else
								{
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleGeneral, "Tried to tab focus to element " + nextTabData.className + " in component " + getGroupName() + " but failed");
								}
#endif
							}
							else
							{
								NdhsCElementGroup* groupToFocus = getGroup(nextTabData.className);

								if (groupToFocus)
								{
									ENdhsObjectType groupType = groupToFocus->getGroupType();

									if (groupType == ENdhsObjectTypeFurnitureComponent || groupType == ENdhsObjectTypeItemComponent)
									{
										NdhsCComponent* componentToFocus = (NdhsCComponent*)groupToFocus;

										retVal = componentToFocus->moveFocus(amount, wrap);
									}
								}
							}

							currentTabIndex = nextTabIndex;
						}
						else
						{
							cannotMoveFocus = true;
						}
					}
				}
			}
		}
		else if (!getTemplate()->getDefaultFocus().isEmpty())
		{
			// We have a default focus for this component, so it can accept the focus
			retVal = trySetFocus(getTemplate()->getDefaultFocus());
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::setFocusIntrinsics(bool focused, const LcTmString& focusedClass)
{
	if (m_fieldCache)
	{
		NdhsCField* field = m_fieldCache->getField("_focuselement");
	
		if (field)
		{
			field->setValue(focusedClass, true);
		}
	
		field = m_fieldCache->getField("_focused");
	
		if (field)
		{
			field->setValueBool(focused, true);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCElement* NdhsCComponent::getItem(const LcTmString& elementClass)
{
	NdhsCElement* retVal = NULL;

	if (m_outerGroup)
	{
		retVal = m_outerGroup->getItem(elementClass);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCElementGroup* NdhsCComponent::getGroup(const LcTmString& groupName)
{
	NdhsCElementGroup* retVal = NULL;

	if (m_outerGroup)
	{
		retVal = m_outerGroup->getGroup(groupName);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::checkVisibilityChanges()
{
	// If the thing in focus becomes invisible, global focus is lost
	if (m_inFocus && m_outerGroup)
	{
		LcCSpace* sp = getSpace();
		if (sp && !visibleOnTransitionComplete())
		{
			globalUnsetFocus();
		}
		else
		{
			if (m_focusedChildComponent)
			{
				m_focusedChildComponent->checkVisibilityChanges();
			}
			else if (m_focusedElement)
			{
				if (!m_focusedElement->checkElementVisiblity())
				{
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleGeneral, "Component " + getGroupName() + ": unsetting focus on " + m_focusedElement->getElementClassName() + " as it is invisble in the current layout");
					globalUnsetFocus();
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCComponent::onPlacementChange(int mask)
{
	LcTPlacement componentPlacement = getPlacement();

	updateExtentIntrinsicVaraibles(componentPlacement.extent, mask);
}

#if defined(NDHS_JNI_INTERFACE)

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager::CStaticPreviewCache> NdhsCComponent::takeSnapShot()
{
	LcTaOwner<NdhsCPageManager::CStaticPreviewCache> info = NdhsCPageManager::CStaticPreviewCache::create();
	int index = 0;

	// Take snapshot of current component
	if (info)
	{
		info->slotNumber = getSlot();

		info->componentClassName = m_componentName;

		index = m_path.toLower().find("b:/");
		if (index != -1)
		{
			info->componentPath = m_path.subString(index + 3, m_path.length());

			index = info->componentPath.find("Packages/");

			if (index != -1)
			{
				info->componentPath = m_path.subString(index + 12, m_path.length());

				index = info->componentPath.find("/");
				if (index != -1)
				{
					info->componentPath = info->componentPath.tail(index + 1);
				}
			}
		}
		else
		{
			info->componentPath = m_path;
		}

		if (m_currentLayout)
			info->layoutName = m_currentLayout->name;
		info->identifier = m_identifier;
	}
	return info;
}

/*-------------------------------------------------------------------------*//**
*/
unsigned int NdhsCComponent::distanceFromPage()
{
	m_pageManager->incrementIdentifier();
	return m_parent->distanceFromPage();
}

#endif

#ifdef IFX_SERIALIZATION
NdhsCComponent* NdhsCComponent::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCComponent * comp=new NdhsCComponent();
	comp->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,comp);
	return comp;
}

SerializeHandle	NdhsCComponent::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCComponent) - sizeof(NdhsCElementGroup)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;

	SerializeHandle parentHandle = NdhsCElementGroup::serialize(serializeMaster,true);

	ENdhsCElementGroupType dataType=ENdhsCElementGroupTypeComponent;
	SERIALIZE(dataType,serializeMaster,cPtr)
	SERIALIZE(parentHandle,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_pageManager,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_template,serializeMaster,cPtr)
	SERIALIZE_Ptr(m_parent,serializeMaster,cPtr)
	SERIALIZE(m_isCreated,serializeMaster,cPtr)
	SERIALIZE(m_isEnabled,serializeMaster,cPtr)
	SERIALIZE(m_groupUnloaded,serializeMaster,cPtr)
	SERIALIZE(m_componentRetire,serializeMaster,cPtr)
	SERIALIZE(m_trySetFocus,serializeMaster,cPtr)
	SERIALIZE_Owner(m_outerGroup,serializeMaster,cPtr)
	SERIALIZE(m_animType,serializeMaster,cPtr)
	SERIALIZE(m_previousAnimType,serializeMaster,cPtr)
	SERIALIZE(m_stateChangeAnimComplete,serializeMaster,cPtr)
	SERIALIZE(m_decorationDelayTimestamp,serializeMaster,cPtr)
	SERIALIZE(m_bMouseDown,serializeMaster,cPtr)
	SERIALIZE(m_lastFrame,serializeMaster,cPtr)
	SERIALIZE(m_isLayoutChange,serializeMaster,cPtr)
#ifdef LC_USE_STYLUS
	SERIALIZE_Ptr(m_mouseFocusElement,serializeMaster,cPtr)
#endif
	SERIALIZE(m_transitionDuration,serializeMaster,cPtr)
	SERIALIZE(m_transitionDelay,serializeMaster,cPtr)
	SERIALIZE(m_transitionVelProfile,serializeMaster,cPtr)
	SERIALIZE(m_staticTransitionDone,serializeMaster,cPtr)
	SERIALIZE(m_jumpingToEnd,serializeMaster,cPtr)
	SERIALIZE_String(m_componentName,serializeMaster,cPtr)
	SERIALIZE(m_stopStaticAnimationAll,serializeMaster,cPtr)
	SERIALIZE(m_stopStaticAnimationItem,serializeMaster,cPtr)
	SERIALIZE(m_lastTriggerPos,serializeMaster,cPtr)
	SERIALIZE(m_lastStaticTriggerPos,serializeMaster,cPtr)
	SERIALIZE(m_staticAnimGroup,serializeMaster,cPtr)
	SERIALIZE_Owner(m_fieldCache,serializeMaster,cPtr)
	SERIALIZE_Owner(m_layoutPos,serializeMaster,cPtr)
	SERIALIZE_Ptr((NdhsCElementGroup*)m_localScreenAggregate.ptr(),serializeMaster,cPtr)
	SERIALIZE(m_bRequiresPostTransitionComplete,serializeMaster,cPtr)
	SERIALIZE_String(m_chainedAction,serializeMaster,cPtr)
	SERIALIZE(m_chainedActionSlot,serializeMaster,cPtr)
	SERIALIZE(m_stackLevel,serializeMaster,cPtr)
	SERIALIZE_Ptr(m_focusedElement,serializeMaster,cPtr)
	SERIALIZE_Ptr(m_focusedChildComponent,serializeMaster,cPtr)
	SERIALIZE(m_inFocus,serializeMaster,cPtr)
	SERIALIZE_Ptr(m_focusEnabledField,serializeMaster,cPtr)
	SERIALIZE(m_refreshNeeded,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCComponent::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummy=0;
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle;

	DESERIALIZE(dummy,serializeMaster,cPtr);
	DESERIALIZE(parentHandle,serializeMaster,cPtr);
	NdhsCElementGroup::deSerialize(parentHandle,serializeMaster);

	DESERIALIZE_Reserve(m_pageManager,serializeMaster,cPtr,NdhsCPageManager)
	DESERIALIZE_Reserve(m_template,serializeMaster,cPtr,NdhsCTemplate)

	// Create the transition agent
	LcTaOwner<NdhsCTransitionAgent> temp = NdhsCTransitionAgent::create(m_template , NULL);
	m_transitionAgent = temp;

	DESERIALIZE_Ptr(m_parent,serializeMaster,cPtr,NdhsCElementGroup)
	DESERIALIZE(m_isCreated,serializeMaster,cPtr)
	DESERIALIZE(m_isEnabled,serializeMaster,cPtr)
	DESERIALIZE(m_groupUnloaded,serializeMaster,cPtr)
	DESERIALIZE(m_componentRetire,serializeMaster,cPtr)
	DESERIALIZE(m_trySetFocus,serializeMaster,cPtr)

	m_currentLayout = NULL;
	m_postTransitionCompleteMessage = LcTMessage(this, EPostTransitionCompleteMsg);

	DESERIALIZE_Owner(m_outerGroup,serializeMaster,cPtr,NdhsCElementGroup)
	m_outerGroup->realize(this);
	DESERIALIZE(m_animType,serializeMaster,cPtr)
	DESERIALIZE(m_previousAnimType,serializeMaster,cPtr)
	DESERIALIZE(m_stateChangeAnimComplete,serializeMaster,cPtr)
	DESERIALIZE(m_decorationDelayTimestamp,serializeMaster,cPtr)
	DESERIALIZE(m_bMouseDown,serializeMaster,cPtr)
	DESERIALIZE(m_lastFrame,serializeMaster,cPtr)
	DESERIALIZE(m_isLayoutChange,serializeMaster,cPtr)
#ifdef LC_USE_STYLUS
	DESERIALIZE_Ptr(m_mouseFocusElement,serializeMaster,cPtr,NdhsCElement)
#endif
	DESERIALIZE(m_transitionDuration,serializeMaster,cPtr)
	DESERIALIZE(m_transitionDelay,serializeMaster,cPtr)
	DESERIALIZE(m_transitionVelProfile,serializeMaster,cPtr)
	DESERIALIZE(m_staticTransitionDone,serializeMaster,cPtr)
	DESERIALIZE(m_jumpingToEnd,serializeMaster,cPtr)
	DESERIALIZE_String(m_componentName,serializeMaster,cPtr)
	DESERIALIZE(m_stopStaticAnimationAll,serializeMaster,cPtr)
	DESERIALIZE(m_stopStaticAnimationItem,serializeMaster,cPtr)
	DESERIALIZE(m_lastTriggerPos,serializeMaster,cPtr)
	DESERIALIZE(m_lastStaticTriggerPos,serializeMaster,cPtr)
	DESERIALIZE(m_staticAnimGroup,serializeMaster,cPtr)
	DESERIALIZE_Owner(m_fieldCache,serializeMaster,cPtr,NdhsCFieldCache)
	DESERIALIZE_Owner(m_layoutPos,serializeMaster,cPtr,NdhsCScrollPosField)
	DESERIALIZE_Owner(m_localScreenAggregate,serializeMaster,cPtr,NdhsCElementGroup)
	m_parameters = NULL;
	DESERIALIZE(m_bRequiresPostTransitionComplete,serializeMaster,cPtr)
	DESERIALIZE_String(m_chainedAction,serializeMaster,cPtr)
	DESERIALIZE(m_chainedActionSlot,serializeMaster,cPtr)
	DESERIALIZE(m_stackLevel,serializeMaster,cPtr)
	DESERIALIZE_Ptr(m_focusedElement,serializeMaster,cPtr,NdhsCElement)
	DESERIALIZE_Ptr(m_focusedChildComponent,serializeMaster,cPtr,NdhsCComponent)
	DESERIALIZE(m_inFocus,serializeMaster,cPtr)
	DESERIALIZE_Ptr(m_focusEnabledField,serializeMaster,cPtr,NdhsCField)
	DESERIALIZE(m_refreshNeeded,serializeMaster,cPtr)

	m_localScreenAggregate->realize(m_pageManager);
	LcTaOwner<NdhsCStateManager> tempSM = NdhsCStateManager::create(m_pageManager->getCon()->getLaundry(), m_template, this,m_pageManager);
	m_stateManager = tempSM;
}
#endif /* IFX_SERIALIZATION */

/*-------------------------------------------------------------------------*//**
*/
LcTaString	NdhsCComponent::getTouchDownElement()
{
	if (m_fieldCache)
	{
		NdhsCField* field = m_fieldCache->getField("_touchElement");
		if (field)
		{
			return field->getFieldData(NULL);
		}
	}
	return "";
}
