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
#include "inflexionui/engine/inc/NdhsCScrollPosField.h"
#include "inflexionui/engine/inc/NdhsCEntryPointMapStack.h"

#include <math.h>

#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


#define ANIM_NON_SKETCHY_THRESHOLD .001
#define AGG_MASK (LcTPlacement::ELocation | LcTPlacement::EScale | LcTPlacement::EOrientation | LcTPlacement::EOpacity)
#define PRIMARY_LIGHT_MASK (LcTPlacement::EColor | LcTPlacement::EColor2 | LcTPlacement::EOrientation)

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageModel> NdhsCPageModel::create(NdhsCPageManager* pageManager, NdhsCPageTemplate* templateFile, NdhsCPage* page, const LcTmString& link, int stackLevel, LcTmString& templatePath)
{
	LcTaOwner<NdhsCPageModel> ref;
	ref.set(new NdhsCPageModel(pageManager, templateFile, page, link, stackLevel, templatePath));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageModel::NdhsCPageModel(NdhsCPageManager* pageManager, NdhsCPageTemplate* templateFile, NdhsCPage* uiNode, const LcTmString& link, int stackLevel, LcTmString& templatePath)
: NdhsCElementGroup("", NULL, NULL, NULL,0)
{
	LC_ASSERT(pageManager != NULL);
	LC_ASSERT(templateFile != NULL);

	m_pageManager  = pageManager;
	m_pageTemplate = templateFile;
	m_templatePath = templatePath;

	m_uiNode = uiNode;

	// We will use this link to parse page params
	m_link = link;

	m_stackLevel = stackLevel;
	m_isEligible = false;

	// Should always be in the PreOpen state before a menu is shown
	// and no slot active (-1)
	m_pageState      = ENdhsPageStatePreOpen;
	m_prevPageState  = ENdhsPageStatePreOpen;

	m_isLayoutChange = false;
	m_refreshNeeded = false;
	m_backgroundDelay = -1;
	m_primaryLightDelay = -1;
	m_primaryLightDuration = -1;

#if defined(NDHS_JNI_INTERFACE)
	// For each page reset identifier
	m_pageManager->resetIdentifier();

	// All child do have identifier greater then page
	m_identifier = 1;
#endif

#ifdef LC_USE_LIGHTS
	setLightModel(templateFile->getLightModel());
#endif

}

#ifdef IFX_SERIALIZATION
NdhsCPageModel* NdhsCPageModel::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	NdhsCPageModel * page=new NdhsCPageModel();
	if(handle==-1)
		return page;
	page->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,page);
	return page;
}
SerializeHandle	NdhsCPageModel::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCPageModel) - sizeof(NdhsCElementGroup)+sizeof(SerializeHandle)+sizeof(IFX_INT32)*2;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle = NdhsCElementGroup::serialize(serializeMaster,true);
	ENdhsCElementGroupType ty=ENdhsCElementGroupTypePage;
	SERIALIZE(ty,serializeMaster,cPtr)
	SERIALIZE(parentHandle,serializeMaster,cPtr)
	SERIALIZE_String(m_link,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_pageManager,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_pageTemplate,serializeMaster,cPtr)
	SERIALIZE_Reserve(m_uiNode,serializeMaster,cPtr)
	SERIALIZE(m_parentVisibility,serializeMaster,cPtr)
	SERIALIZE(m_showWhenInSub,serializeMaster,cPtr)
	SERIALIZE_String(m_cleanupLink,serializeMaster,cPtr)
	SERIALIZE(m_bMouseDown,serializeMaster,cPtr)
	SERIALIZE(m_previousAnimType,serializeMaster,cPtr)
	SERIALIZE(m_animType,serializeMaster,cPtr)
	SERIALIZE(m_scrollPositive,serializeMaster,cPtr)
	SERIALIZE(m_terminalAnimTimestamp,serializeMaster,cPtr)
	SERIALIZE(m_interactiveAnimTimestamp,serializeMaster,cPtr)
	SERIALIZE(m_insideChangeState,serializeMaster,cPtr)
	SERIALIZE(m_isLayoutChange,serializeMaster,cPtr)
	SERIALIZE(m_refreshNeeded,serializeMaster,cPtr)
	SERIALIZE(m_stackLevel,serializeMaster,cPtr)
	SERIALIZE(m_prevPageState,serializeMaster,cPtr)
	SERIALIZE(m_pageState,serializeMaster,cPtr)
	SERIALIZE(m_primaryLightDuration,serializeMaster,cPtr)
	SERIALIZE(m_primaryLightDelay,serializeMaster,cPtr)
	SERIALIZE(m_transitionVelProfile,serializeMaster,cPtr)
	SERIALIZE(m_transitionDelay,serializeMaster,cPtr)
	SERIALIZE(m_transitionDuration,serializeMaster,cPtr)
	SERIALIZE(m_backgroundDelay,serializeMaster,cPtr)
	SERIALIZE(m_bRequiresPostTransitionComplete,serializeMaster,cPtr)
	SERIALIZE(m_lastFrame,serializeMaster,cPtr)
	SERIALIZE(m_ignoreScrollPosUpdate,serializeMaster,cPtr)
	SERIALIZE(m_lastStaticTriggerPos,serializeMaster,cPtr)
	SERIALIZE(m_lastTriggerPos,serializeMaster,cPtr)
	SERIALIZE(m_stopStaticAnimationItem,serializeMaster,cPtr)
	SERIALIZE(m_stopStaticAnimationAll,serializeMaster,cPtr)
	SERIALIZE(m_activePrimaryLightMask,serializeMaster,cPtr)
	SERIALIZE_Placement(m_activePrimaryLightPlacement,serializeMaster,cPtr)
	SERIALIZE_Placement(m_currentPrimaryLightPlacement,serializeMaster,cPtr)
	SERIALIZE(m_primaryLightAnimate,serializeMaster,cPtr)
	SERIALIZE(m_primaryLightBaseEndMask,serializeMaster,cPtr)
	SERIALIZE_Placement(m_primaryLightBaseEnd,serializeMaster,cPtr)
	SERIALIZE(m_primaryLightBaseStartMask,serializeMaster,cPtr)
	SERIALIZE_Placement(m_primaryLightBaseStart,serializeMaster,cPtr)
	SERIALIZE(m_localScreenAggregate,serializeMaster,cPtr)
	SERIALIZE(m_outerGroup,serializeMaster,cPtr)
	SERIALIZE(m_stateChange,serializeMaster,cPtr)
	SERIALIZE(m_fieldCache,serializeMaster,cPtr)
	SERIALIZE_MapString(m_paramMap,serializeMaster,cPtr)
#ifdef LC_USE_STYLUS
	SERIALIZE_Ptr(m_mouseFocusElement,serializeMaster,cPtr)
#endif
	SERIALIZE_Ptr(m_focusedTargetComponent,serializeMaster,cPtr)
	SERIALIZE_Ptr(m_focusedChildComponent,serializeMaster,cPtr)
	SERIALIZE_Ptr(m_focusedElement,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}

void NdhsCPageModel::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	int dummySize=0;
	void * cPtr=serializeMaster->getOffset(handle);
	SerializeHandle parentHandle;
	DESERIALIZE(dummySize,serializeMaster,cPtr)
	DESERIALIZE(parentHandle,serializeMaster,cPtr)
	NdhsCElementGroup::deSerialize(parentHandle,serializeMaster);
	serializeMaster->setPointer(handle,this);
	DESERIALIZE_String(m_link,serializeMaster,cPtr)
	DESERIALIZE_Reserve(m_pageManager,serializeMaster,cPtr,NdhsCPageManager)
	DESERIALIZE_Reserve(m_pageTemplate,serializeMaster,cPtr,NdhsCPageTemplate)
	DESERIALIZE_Reserve(m_uiNode,serializeMaster,cPtr,NdhsCPage)
	DESERIALIZE(m_parentVisibility,serializeMaster,cPtr)
	DESERIALIZE(m_showWhenInSub,serializeMaster,cPtr)
	DESERIALIZE_String(m_cleanupLink,serializeMaster,cPtr)
	DESERIALIZE(m_bMouseDown,serializeMaster,cPtr)
	DESERIALIZE(m_previousAnimType,serializeMaster,cPtr)
	DESERIALIZE(m_animType,serializeMaster,cPtr)
	DESERIALIZE(m_scrollPositive,serializeMaster,cPtr)
	DESERIALIZE(m_terminalAnimTimestamp,serializeMaster,cPtr)
	DESERIALIZE(m_interactiveAnimTimestamp,serializeMaster,cPtr)
	DESERIALIZE(m_insideChangeState,serializeMaster,cPtr)
	DESERIALIZE(m_isLayoutChange,serializeMaster,cPtr)
	DESERIALIZE(m_refreshNeeded,serializeMaster,cPtr)
	DESERIALIZE(m_stackLevel,serializeMaster,cPtr)
	DESERIALIZE(m_prevPageState,serializeMaster,cPtr)
	DESERIALIZE(m_pageState,serializeMaster,cPtr)
	DESERIALIZE(m_primaryLightDuration,serializeMaster,cPtr)
	DESERIALIZE(m_primaryLightDelay,serializeMaster,cPtr)
	DESERIALIZE(m_transitionVelProfile,serializeMaster,cPtr)
	DESERIALIZE(m_transitionDelay,serializeMaster,cPtr)
	DESERIALIZE(m_transitionDuration,serializeMaster,cPtr)
	DESERIALIZE(m_backgroundDelay,serializeMaster,cPtr)
	DESERIALIZE(m_bRequiresPostTransitionComplete,serializeMaster,cPtr)
	DESERIALIZE(m_lastFrame,serializeMaster,cPtr)
	DESERIALIZE(m_ignoreScrollPosUpdate,serializeMaster,cPtr)
	DESERIALIZE(m_lastStaticTriggerPos,serializeMaster,cPtr)
	DESERIALIZE(m_lastTriggerPos,serializeMaster,cPtr)
	DESERIALIZE(m_stopStaticAnimationItem,serializeMaster,cPtr)
	DESERIALIZE(m_stopStaticAnimationAll,serializeMaster,cPtr)
	DESERIALIZE(m_activePrimaryLightMask,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_activePrimaryLightPlacement,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_currentPrimaryLightPlacement,serializeMaster,cPtr)
	DESERIALIZE(m_primaryLightAnimate,serializeMaster,cPtr)
	DESERIALIZE(m_primaryLightBaseEndMask,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_primaryLightBaseEnd,serializeMaster,cPtr)
	DESERIALIZE(m_primaryLightBaseStartMask,serializeMaster,cPtr)
	DESERIALIZE_Placement(m_primaryLightBaseStart,serializeMaster,cPtr)
	DESERIALIZE_Owner(m_localScreenAggregate,serializeMaster,cPtr,NdhsCElementGroup)
	DESERIALIZE_Owner(m_outerGroup,serializeMaster,cPtr,NdhsCElementGroup)
	m_outerGroup->realize(this);
	DESERIALIZE_Owner(m_stateChange,serializeMaster,cPtr,NdhsCScrollPosField)
	DESERIALIZE_Owner(m_fieldCache,serializeMaster,cPtr,NdhsCFieldCache)
	DESERIALIZE_MapString(m_paramMap,serializeMaster,cPtr)
#ifdef LC_USE_STYLUS
	DESERIALIZE_Ptr(m_mouseFocusElement,serializeMaster,cPtr,NdhsCElement)
#endif
	DESERIALIZE_Ptr(m_focusedTargetComponent,serializeMaster,cPtr,NdhsCComponent)
	DESERIALIZE_Ptr(m_focusedChildComponent,serializeMaster,cPtr,NdhsCComponent)
	DESERIALIZE_Ptr(m_focusedElement,serializeMaster,cPtr,NdhsCElement)

	m_localScreenAggregate->realize(m_pageManager);

	// Create the transition agent
	LcTaOwner<NdhsCTransitionAgent> temp = NdhsCTransitionAgent::create(m_pageTemplate , NULL);
	m_transitionAgent = temp;
	LcTaOwner<NdhsCStateManager> tempSM = NdhsCStateManager::create(m_pageManager->getCon()->getLaundry(), m_pageTemplate, this,m_pageManager);
	m_stateManager = tempSM;
}
#endif /* IFX_SERIALIZATION */

/*-------------------------------------------------------------------------*//**
*/
NdhsCPageModel::~NdhsCPageModel()
{
	retire();

	if (m_outerGroup)
		m_outerGroup->retire();

	if (m_localScreenAggregate)
		m_localScreenAggregate->retire();

	if (m_stateManager)
		m_stateManager.destroy();

	m_paramMap.clear();

	if(m_pageManager)
	{
		m_pageManager->releaseUiNode(m_uiNode);
#ifdef IFX_USE_STYLUS
		m_pageManager->ignoreEntry(this);
#endif
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::postConstruct()
{

	LcCAggregate::construct();

	m_parentVisibility    = m_pageTemplate->getParentVisibility();
	m_showWhenInSub       = m_pageTemplate->getShowWhenInSub();
	m_cleanupLink		  = m_pageTemplate->getCleanupLink();

	parsePageParamters();

	// Create the layout change field
	LcTaOwner<NdhsCScrollPosField> tempPos = NdhsCScrollPosField::create(m_pageManager->getCon(), NULL,
													NDHS_VELOCITY_PROFILE_CATAPULT_MAX_DRAWBACK,
													NDHS_VELOCITY_PROFILE_BOUNCE_MAX_EXTENSION,
													false,
													0.0f);
	m_stateChange = tempPos;
	m_stateChange->setIgnoreLaundry(true);
	// Note that we don't observe the layoutPos field, as we're the only entity that can modify it.

	LcTaOwner<NdhsCFieldCache> tempFC = NdhsCFieldCache::create(m_pageManager->getCon());
	m_fieldCache = tempFC;

	// Create page intrinsic fields
	m_fieldCache->createPageIntrinsicFields();

	// Add local varaibles
	m_fieldCache->addLocalVariables(m_pageTemplate, this);

	// Clean Laundry
	m_pageManager->getCon()->getLaundry()->cleanAll();

	// Create the transition agent
	LcTaOwner<NdhsCTransitionAgent> temp = NdhsCTransitionAgent::create(m_pageTemplate , NULL);
	m_transitionAgent = temp;

	LcTaOwner<NdhsCStateManager> tempSM = NdhsCStateManager::create(m_pageManager->getCon()->getLaundry(), m_pageTemplate, this, m_pageManager);
	m_stateManager = tempSM;

#if defined(NDHS_JNI_INTERFACE)
	m_stateManager->allowLayoutChange(true);
#endif

	reloadFurniture();

	// Now that the element hierarchy is in place and fully connected, we can let
	// the elements try and acquire their resources (they may depend on parameters,
	// etc. so we can't do it when the elements are created)
	onResume();

	// Pages should begin invisible and become visible
	// after the opening delay
	setVisible(false);

	m_localScreenAggregate->realize(m_pageManager);
	realize(m_pageManager);

	// Set page model
#ifdef IFX_USE_SCRIPTS
	if (NdhsCScriptExecutor::getInstance())
	{
		NdhsCScriptExecutor::getInstance()->setPageModel(this);
	}
#endif //IFX_USE_SCRIPTS

#ifdef IFX_GENERATE_SCRIPTS
	if (NdhsCScriptGenerator::getInstance())
	{
		NdhsCScriptGenerator::getInstance()->setStartTime();
	}
#endif //IFX_GENERATE_SCRIPTS

}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPageModel::onRealize()
{
	NdhsCElementGroup::onRealize();

	if(m_stateManager)
	{
#if defined(NDHS_JNI_INTERFACE)
		if (m_stateManager)
			m_stateManager->setAggregateSnapShotInfo(takeSnapShot());
#endif
		m_stateManager->addObserver(this);
		m_stateManager->testLayoutConditions();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::onRetire()
{
	NdhsCElementGroup::onRetire();

	resetTransitionCache();

	// Terminate any static animations
	stopStaticAnimations(true, EAnimationAll);

	jumpTransitionToEnd(false);

	m_bRequiresPostTransitionComplete = false;

	if (m_stateManager)
		m_stateManager->removeObserver(this);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::createFieldCache(NdhsCTemplate* templ)
{
	LcTaOwner<NdhsCFieldCache> tempFC = NdhsCFieldCache::create(m_pageManager->getCon());
	m_fieldCache = tempFC;

	// Add parameters
	m_fieldCache->addInputParameters(templ, this, this);

	// Add local varaibles
	m_fieldCache->addLocalVariables(templ, this);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::destroyUnwantedElements()
{
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::reloadFurniture()
{

	LcTmOwner<NdhsCElementGroup> tempOuterGroup;


	m_localScreenAggregate = m_pageTemplate->createElementGroup(this,
																						NULL,
																						NULL,
																						"screen",
																						m_stackLevel,
																						getDrawLayerIndex(),
																						NULL);

	// Create the furniture element group
	LcTmOwner<NdhsCElementGroup> furnitureElementGroup = m_pageTemplate->createElementGroup(this,
																							NULL,
																							NULL,
																							"page",
																							m_stackLevel,
																							getDrawLayerIndex(),
																							this);
	furnitureElementGroup->setVisible(true);
	tempOuterGroup = m_pageTemplate->createElementGroup(this,
																							NULL,
																							NULL,
																							"outergroup",
																							m_stackLevel,
																							getDrawLayerIndex(),
																							this);
	tempOuterGroup->setVisible(true);
	tempOuterGroup->setElementGroup(this);
	furnitureElementGroup->setElementGroup(tempOuterGroup.ptr());
	furnitureElementGroup->realize(tempOuterGroup.ptr());
	tempOuterGroup->addGroup("furniture", furnitureElementGroup);
	tempOuterGroup->realize(this);

	if (m_outerGroup)
	{
		m_outerGroup->retire();
		m_outerGroup.destroy();
	}

	m_outerGroup = tempOuterGroup;

}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPageModel::refreshPage(IFX_HMENU hMenu, bool immediate)
{
	// Do not do any post-transition complete processing if pending -
	// wait for the page refresh process to schedule anew if necessary.
	if (m_bRequiresPostTransitionComplete)
		m_bRequiresPostTransitionComplete = false;

	// If the state is close, the page is
	// about to be destroyed
	if (m_pageState == ENdhsPageStateClose)
		return;

#ifdef LC_USE_STYLUS
	// Cancel any current drag operation
	m_pageManager->onMouseCancel(this, NULL, false);
	m_mouseFocusElement = NULL;
#endif

	// make sure that onTransitionComplete
	// does not start any new transitions
	m_refreshNeeded = false;

	// Now recreate the menu
	reloadFurniture();

	// The positioning code requires some sort of transition, so
	// we set up a 'dummy' terminal state transition to the current
	// state, taking no time.
	ENdhsPageState cachedState = m_pageState;
	changeState(ENdhsPageStateOpen, 0, NULL, false);
	changeState(cachedState, 0, NULL, false);

	// Complete the dummy state transition to correctly position all the
	// elements
	onTransitionComplete(false);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT NdhsCField* NdhsCPageModel::getFieldValue(const LcTmString& fieldName,
													NdhsCMenu* menu,
													int item,
													NdhsCElement* element)
{
	return getField(fieldName, -1, NULL);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::updatePosition(LcTScalar position,
										bool positionIncreasing,
										bool updateCache,
										bool finalFrame)
{
	LcTPlacement placement;
	int mask = 0;
	LcTScalar animationFrac = position;

	// If layout being changed updated field
	if (m_isLayoutChange == true)
	{
		NdhsCField* field = m_fieldCache->getField("_transitioning");
		if (field)
		{
			field->setValueBool(true, true);
		}
	}

	// Check to see if we need to show or hide the page
	if ((m_animType == ENdhsAnimationTypeInteractiveState)
		|| (m_animType == ENdhsAnimationTypeTerminalState))
	{
		LcTScalar showPoint = (LcTScalar)(m_transitionDuration ? (m_transitionDelay / m_transitionDuration) : 0);

		if (m_pageState == ENdhsPageStateOpen || m_pageState == ENdhsPageStateShow)
		{
			if ((getVisible() == false) && (position >= showPoint))
				setVisible(true);
		}

		if (m_pageState == ENdhsPageStateClose || m_pageState == ENdhsPageStateHide)
		{
			if ((getVisible() == true) && finalFrame)
			{
				// We set the page to invisible now when the page is closing or hiding
				// because the other page(s) could still be animating for some time
				setVisible(false);
			}
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
				m_pageManager->simulateKeyDown(m_pAnimTriggerList->triggerList[i]->key);
				m_lastTriggerPos = m_pAnimTriggerList->triggerList[i]->position;
			}
			else
			{
				m_lastTriggerPos = animationFrac;
			}
		}
	}

	if ((!m_pageManager->isPrimaryLightOverrideActive()) && (m_primaryLightAnimate || finalFrame))
	{
		// Reset cache
		m_currentPrimaryLightPlacement = m_primaryLightBaseStart;

		// Interpolate
		m_primaryLightBasePath.getPlacement(animationFrac, m_currentPrimaryLightPlacement);
		mask = m_primaryLightBasePath.getMask();

		// Limit the mask to permissible mask values
		mask &= PRIMARY_LIGHT_MASK;

		// Now update the primary light
		m_pageManager->setPrimaryLightPlacement(m_currentPrimaryLightPlacement, mask);
	}

	// Position the furniture groups
	if (m_outerGroup)
	{
		m_outerGroup->updatePosition(getAnimType(), animationFrac, positionIncreasing, updateCache,
															false, false, finalFrame);
	}

	// Screen aggregate position
	if (m_localScreenAggregate)
	{
		m_localScreenAggregate->updatePosition(getAnimType(), animationFrac, positionIncreasing, updateCache,
															false, false, finalFrame);
	}
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageModel::onMouseDown(NdhsCPageManager::TPageWidgetElem* entry, const LcTPixelPoint& pt)
{
	// We must ask the element for the widget because a
	// field refresh might have changed the original
	// widget pointer

	bool bConsumed = false;

	m_bMouseDown = true;

	if (entry && entry->element)
		bConsumed = entry->element->onMouseDown(pt);

	if (bConsumed)
		m_mouseFocusElement = entry->element;

	return bConsumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageModel::onMouseMove(NdhsCPageManager::TPageWidgetElem* entry, const LcTPixelPoint& pt)
{
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
bool NdhsCPageModel::onMouseUp(NdhsCPageManager::TPageWidgetElem* entry, const LcTPixelPoint& pt)
{
	// We must ask the element for the widget because a
	// field refresh might have changed the original
	// widget pointer

	bool bConsumed = false;
	LcCWidget* widget = NULL;
	bool triggersAllowed = true;
	bool widgetHit = false;

	m_bMouseDown = false;

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
		if (m_pageState == ENdhsPageStateInteractive || m_pageState == ENdhsPageStateSelected)
		{
			LcTaString elementClass;

			// Check that the widget was hit.
			if (widget && widgetHit)
			{
				// check furniture first for selected widget
				elementClass = getPageAggregate()->getClassFromWidget(widget);

				// We are just checking page furniture, no need to check for
				// component/menu component, because in case of component
				// or menu component their mouse related functions will be
				// called
			}

			// This is executed even if selectedSlot and elementClass are not populated.
			// The may execute the catch all case if necessary
			bConsumed = processTrigger(ENdhsNavigationStylusTap, -1, elementClass, NULL, false);
		}
	}

	if (m_mouseFocusElement)
		m_mouseFocusElement = NULL;

	updateTouchDownIntrinsicField("", -1);

	return bConsumed;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::fieldValueUpdated(NdhsCField* field)
{
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
				m_pageManager->simulateKeyDown(m_pStaticTriggerList->cleanupTrigger);

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
					m_pageManager->simulateKeyDown(m_pStaticTriggerList->triggerList[i]->key);
					m_lastStaticTriggerPos = m_pStaticTriggerList->triggerList[i]->position;
				}
				else
				{
					m_lastStaticTriggerPos = frac;
				}
			}
		}
	}
	else
	{
		NdhsCElementGroup::fieldValueUpdated(field);
	}

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::fieldDestroyed(NdhsCField* field)
{
	if (field == m_pStaticAnimFieldTrigger)
	{
		m_pStaticAnimFieldTrigger = NULL;
	}

	NdhsCElementGroup::fieldDestroyed(field);
}

#ifdef LC_USE_STYLUS
/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::populateWidgetElementMap(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList)
{
	NdhsCElementGroup* agg = getPageAggregate();
	if (agg)
	{
		agg->populateElementList(widgets, pageWidgetElemList);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::populateElementList(NdhsCPageManager::TmAWidgets& widgets, NdhsCPageManager::TmAPageWidgetElem& pageWidgetElemList)
{
	NdhsCElementGroup* agg = getPageAggregate();
	if (agg)
		agg->populateElementList(widgets, pageWidgetElemList);
}

#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::setPrimaryLightTransitionBasePlacements(LcTPlacement start,
					int startMask,
					LcTPlacement end,
					int endMask)
{
	m_primaryLightBaseStart = start;
	m_primaryLightBaseStartMask = startMask;
	m_primaryLightBaseEnd = end;
	m_primaryLightBaseEndMask = endMask;

	if (ENdhsPageStateInteractive == m_pageState)
	{
		m_activePrimaryLightPlacement = m_primaryLightBaseEnd;
		m_activePrimaryLightMask = m_primaryLightBaseEndMask;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::setVisible(bool b)
{
#if defined(NDHS_JNI_INTERFACE)
	// In case of static preview make sure we are visible
	if (m_pageManager->isInStaticPreviewMode())
		b = true;
#endif
	// Ensure local screen agg always has same visibility as page
	if (m_localScreenAggregate)
	{
		m_localScreenAggregate->setVisible(b);
	}

	// Propagate to base class
	LcCAggregate::setVisible(b);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageModel::componentDoPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame)
{
	bool reschedule = false;

	NdhsCElementGroup* agg = getPageAggregate();
	if (agg)
	{
		if (agg->componentDoPrepareForFrameUpdate(timestamp, finalFrame))
			reschedule = true;
	}

	return reschedule;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::componentsJumpTransitionToEnd(bool setIdle)
{
	// If no page aggregate no need to go further
	NdhsCElementGroup* agg = getPageAggregate();
	if (agg)
	{
		agg->componentsJumpTransitionToEnd(setIdle);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPageModel::resumeStaticAnimations()
{
	// If page got any static animation in current layout start it
	if (startStaticAnimations(EAnimationAll))
		m_animType = ENdhsAnimationTypeStatic;

	// If no page aggregate no need to go further
	NdhsCElementGroup* agg = getPageAggregate();
	if (agg)
	{
		agg->resumeStaticAnimations();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCPageModel::changeState(ENdhsPageState state, int activeSlot, const NdhsCPageTemplate::CAction::CAttempt* attempt, bool animate, bool statisPreview, bool generateSignal)
{
	ENdhsAnimationType animType = ENdhsAnimationTypeInteractiveState;
	ENdhsVelocityProfile velocityProfile = ENdhsVelocityProfileUnknown;
	int duration = 0;
	int delay = 0;
	m_insideChangeState = true;
	bool willAnimate = false;

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

	// Setup the new state
	m_prevPageState = m_pageState;
	m_pageState 	= state;

	if (statisPreview)
	{
		animType = ENdhsAnimationTypeTerminalState;
		m_pageState = m_stateManager->getStateInfo();

		if (m_pageState == ENdhsPageStateNone)
			m_pageState = ENdhsPageStateInteractive;

#if defined(NDHS_JNI_INTERFACE)
		// We already have layout for static preview
		// So stop any other from this point
		m_stateManager->allowLayoutChange(false);
#endif
	}

	updatePageStateField(m_pageState);

	if (m_pageState == ENdhsPageStateInteractive
		&& m_prevPageState == ENdhsPageStateOpen)
	{
		// Set the default focus
		trySetFocus();
	}

#ifdef LC_USE_STYLUS
	// Clean up mouse references if necessary
	if (m_pageState == ENdhsPageStateClose)
	{
		m_pageManager->onMouseCancel(this, NULL, false);
		m_mouseFocusElement = NULL;
	}
#endif

	switch (m_prevPageState)
	{
		case ENdhsPageStateOpen:
		{

			animType = ENdhsAnimationTypeTerminalState;
			break;
		}

		case ENdhsPageStateSelected:
		case ENdhsPageStateLaunch:
		{
			animType = ENdhsAnimationTypeTerminalState;

			break;
		}

		case ENdhsPageStateShow:
		{
			animType = ENdhsAnimationTypeTerminalState;

#ifdef NDHS_JNI_INTERFACE
			LcTaString focusedObject;
			if (m_focusedElement)
			{
				focusedObject = m_focusedElement->getFocusChain();
			}
			else if (m_focusedChildComponent)
			{
				focusedObject = m_focusedChildComponent->getObjectWithFocus();
			}
			else
				focusedObject = m_uiNode->getPageName();

			NDHS_TRACE(ENdhsTraceLevelInfo, ENdhsTraceModuleGeneral, "Focus change: focus now " + focusedObject);
#endif
			break;
		}

		case ENdhsPageStateHide:
		{
			animType = ENdhsAnimationTypeTerminalState;

			break;
		}

		case ENdhsPageStatePreOpen:
		{
			// Active slot doesn't change
			animType = ENdhsAnimationTypeTerminalState;

#ifdef IFX_GENERATE_SCRIPTS
			if (NdhsCScriptGenerator::getInstance())
				NdhsCScriptGenerator::getInstance()->setEventStartTime(getSpace()->getTimestamp());
#endif //IFX_GENERATE_SCRIPTS

			break;
		}

		case ENdhsPageStateInteractive:
		{
			switch (m_pageState)
			{
				case ENdhsPageStateHide:
				case ENdhsPageStateSelected:
				case ENdhsPageStateLaunch:
				{
					// Going to a terminal state,
					// but slot may temporarily
					// change to inactive (-1)
					animType = ENdhsAnimationTypeTerminalState;

					break;
				}

				case ENdhsPageStateShow:
				{
					// Going to a terminal state,
					// so the slot doesn't change
					animType = ENdhsAnimationTypeTerminalState;

					break;
				}

				case ENdhsPageStateClose:
				{
					// Going back, so no slot will be active
					animType = ENdhsAnimationTypeTerminalState;

					break;
				}

				case ENdhsPageStateInteractive:
				{
					// Staying in the interactive layout,
					// so used the supplied value

					break;
				}

				default:
					break;
			}

			break;
		}

		default:
			break;
	}

	// Reset the static animation flag
	m_staticTransitionDone = false;

	// Check if we are doing the static animations
	if (m_prevPageState == m_pageState)
	{
		animType = ENdhsAnimationTypeStatic;

		if (animate == true)
			m_staticTransitionDone = true;
	}

	m_transitionDuration = duration;
	m_transitionDelay = delay;
	m_transitionVelProfile = velocityProfile;
	m_backgroundDelay = -1;
	m_primaryLightDelay = -1;
	m_primaryLightDuration = -1;

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
		if (m_pageState == ENdhsPageStateInteractive && m_isEligible == false && generateSignal)
		{
			m_isEligible = true;
			getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_STARTED);
		}
	}
#endif

	m_insideChangeState = false;
	// We tell the caller if we're in a terminal state transition or not
	return (m_animType == ENdhsAnimationTypeTerminalState);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPageModel::onTransitionComplete(bool setIdle, bool generateSignal)
{
#if defined(IFX_ENABLE_BENCHMARKING_FRAME_RATE) || defined(IFX_ENABLE_BENCHMARKING_HEAP_USAGE)
	if (m_pageState == ENdhsPageStateInteractive && m_isEligible == true && generateSignal)
	{
		m_isEligible = false;
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_TRANSITION_COMPLETE);
	}
#endif
	// If outer group do not exists, no need to go further
	if (m_outerGroup.ptr() == NULL)
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
	ENdhsAnimationType animType = m_animType;

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

	//
	// Handle transition complete...
	//
	if ((animType == ENdhsAnimationTypeTerminalState) || (animType == ENdhsAnimationTypeInteractiveState))
	{
		// ...the page
		if (m_pageState == ENdhsPageStateShow || m_pageState == ENdhsPageStateHide)
		// If its in the show state, the visibility will be
		// set after the next transition delay
		{
			setVisible(false);
		}
		else if (m_pageState == ENdhsPageStateClose)
		// Hide the page straight away
		{
			setVisible(false);
		}
		else
		{
			setVisible(m_outerGroup->visibleOnTransitionComplete());
		}
	}

	// If we did a skip and there is a cleanup link, execute it
	if (m_pageState == ENdhsPageStateClose
		&& (m_prevPageState == ENdhsPageStateHide
			|| m_prevPageState == ENdhsPageStateSelected))
	{
		if (!m_cleanupLink.isEmpty())
		{
			LcTaString linkPrefix;

			// Determine the link prefix
			LcTaString expandedLink;
			m_pageManager->getTokenStack()->replaceTokens(m_cleanupLink, expandedLink, NULL, NULL, NULL, this, m_stackLevel);
			linkPrefix = expandedLink.subString(0, expandedLink.find(":"));

			ENdhsLinkType linkType = m_pageManager->getCon()->getTRLinkType(linkPrefix);

			// We only launch function links here and not menus
			if (linkType == ENdhsLinkTypeSyncLinkPlugin)
			{
				// Need to pass in a menu item, so create one
				LcTaOwner<NdhsCMenuItem> mi = NdhsCMenuItem::create(NULL);
				mi->setLinkAttr(expandedLink);

				// Launch the link
				m_pageManager->getCon()->launchLink(mi.ptr(), m_stackLevel);
			}
		}
	}

	// Check to see if there is a chained action, or a static animation that might need
	// to be started, or a focus element that needs updating.
	else if (!setIdle
		&& ((!m_chainedAction.isEmpty() && m_pageState != ENdhsPageStateShow)
		|| ((m_staticTransitionDone == false) && (m_jumpingToEnd == false)
			&& (m_pageState == ENdhsPageStateInteractive || m_pageState == ENdhsPageStateSelected))
		))
	{
		// Ok...we need to do some more actions
		m_bRequiresPostTransitionComplete = true;

		// Request that the Page Manager schedule the post-TC event
		m_pageManager->schedulePostTransitionComplete();
	}

}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPageModel::jumpTransitionToEnd(bool setIdle)
{
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
			m_pageManager->simulateKeyDown(m_pAnimTriggerList->cleanupTrigger);
		}

		m_pAnimTriggerList = NULL;
	}

	// Interrupt - Jump any previous transition to the end
	switch (m_animType)
	{
		case ENdhsAnimationTypeInteractiveState:
		{
			// Not reseting chained action, because we
			// will be here during layout change and will
			// result in reseting chained action

			// Set the layout field to 'end'
			m_stateChange->setValue(1.0, true);

			// Now do the 'on transition complete' step.
			onTransitionComplete(setIdle);

			break;
		}

		case ENdhsAnimationTypeTerminalState:
		{
			// If we get here, we are interrupting the transition
			// so the chained action will not execute
			m_chainedAction = "";

			// Set the terminal field to 'end'
			m_stateChange->setValue(1.0, true);

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
LC_EXPORT void NdhsCPageModel::postTransitionComplete()
{

#ifdef IFX_GENERATE_SCRIPTS
	if (!m_bRequiresPostTransitionComplete)
		if (NdhsCScriptGenerator::getInstance())
			NdhsCScriptGenerator::getInstance()->cancelCaptureMove();
#endif

	// Only do anything if we've recorded that we need to...
	if (!m_bRequiresPostTransitionComplete)
		return;

	volatile int memError = 0;

	// Note we no longer need to perform any actions.
	m_bRequiresPostTransitionComplete = false;

	m_isLayoutChange = false;

	// If layout being changed updated field
	NdhsCField* field = m_fieldCache->getField("_transitioning");
	if (field)
	{
		field->setValueBool(false, true);
	}

	// Prepare cleanup frame
	LC_CLEANUP_PUSH_FRAME(memError);

	if (memError == 0)
	{
		// Now do the chained action
		if (!m_chainedAction.isEmpty() && m_pageState != ENdhsPageStateShow)
		{
			executeChainedAction(m_chainedAction, m_chainedActionSlot);
		}

		// Only carry on if a chained action didn't start a transition, or schedule another post transition complete
		// message (as it would if the chained action duration was 0ms)
		if (!m_bRequiresPostTransitionComplete && ((m_animType == ENdhsAnimationTypeNone) || (m_animType == ENdhsAnimationTypeStatic)))
		{
			if ((m_staticTransitionDone == false) && (m_jumpingToEnd == false)
				&& (m_pageState == ENdhsPageStateInteractive || m_pageState == ENdhsPageStateSelected))
			{
				// Start the static transition, but never when the transition completes
				// because we jumped to the end of a transition
				(void)changeState(m_pageState, 0, NULL, true);
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
LC_EXPORT bool NdhsCPageModel::processTrigger(	int code,
												int slot,
												LcTmString& elementClass,
												LcTmArray<NdhsCTemplate::CAction::CAttempt*>* optionsAttempts,
												bool fromModule)
{
	volatile int memError = 0;
	bool bSkipToEnd = false;
	bool bConsumed = false;

	// Prepare cleanup frame
	LC_CLEANUP_PUSH_FRAME(memError);

	if (memError == 0)
	{
		int localCode = code;

		// Only consider focus while we are interactive
		if (m_pageState == ENdhsPageStateInteractive)
		{
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
		}

		if(bSkipToEnd == false)
		{
			if (localCode == ENdhsNavigationOptionsMenu
				|| localCode == ENdhsNavigationOpenLink
				|| localCode == ENdhsNavigationChainAction)
			{
				bool attemptOk = false;
				LcTmArray<NdhsCTemplate::CAction::CAttempt*>::iterator it = optionsAttempts->begin();

				for (; it != optionsAttempts->end() && !attemptOk; it++)
				{
					attemptOk = doAttempt(*it, slot);
				}

				bConsumed = attemptOk;
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
					bConsumed = m_stateManager->processSlotTap(slot, elementClass);
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
bool NdhsCPageModel::executeChainedAction(const LcTmString& action, int slotNum)
{
	bool retVal = false;

	if (!action.isEmpty())
	{
		NdhsCTemplate::CAction* pActionObj = m_pageTemplate->getAction(action);

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
bool NdhsCPageModel::startTransition(ENdhsAnimationType animType,
									bool animate)
{
	bool willAnimate = true;

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
			}
			// Drop through...
			case ENdhsAnimationTypeScroll:
			case ENdhsAnimationTypeScrollKick:
			case ENdhsAnimationTypeDrag:
			{
				// We're about to make a whole bunch of pointers stale, so we need to clear them up first
				resetTransitionCache();

				// Update the starting and ending layout objects.
				m_transitionAgent->prepareTransition(m_stateManager->getCurrentLayout(), -1, -1);

				// Record transition type
				m_previousAnimType = m_animType;
				m_animType = animType;

				// Get info for the transition to come - note that this may replace duration/delay/profile
				// and bg delay for terminal state transitions if decorations are supported.
				m_transitionAgent->getPageAnimationDetails(this,
														getAnimType(),
														m_transitionDuration,
														m_transitionDelay,
														m_transitionVelProfile,
														m_backgroundDelay,
														m_primaryLightDelay,
														m_primaryLightDuration);

				if(animType == ENdhsAnimationTypeLayoutChange)
				{
					animType = ENdhsAnimationTypeInteractiveState;
					m_animType = animType;
				}

				// Check the primary light animation
				// For base tweens, always set up the shortest rotation path
				m_primaryLightBaseEnd.orientation.shortestPath(m_primaryLightBaseStart.orientation);

				// Now work out the 'delta' placement between start and end points
				m_primaryLightBasePath.setPathData(m_primaryLightBaseStart, m_primaryLightBaseEnd, m_primaryLightBaseStartMask | m_primaryLightBaseEndMask, ENdhsDecoratorTypeLinear);

				m_primaryLightAnimate = (m_primaryLightBasePath.getMask() != LcTPlacement::ENone);

				if (m_primaryLightBaseStartMask != 0)
					m_pageManager->setPrimaryLightPlacement(m_primaryLightBaseStart, m_primaryLightBaseStartMask);

				// OK...we've got the timing details, we need to set up the relevant field to control animation
				bool finalUpdate;

				LcTTime timestamp = getSpace()->getTimestamp();

				if (animType == ENdhsAnimationTypeInteractiveState)
				{
					m_stateChange->setValue(0.0, false);
					m_stateChange->setTargetValue(1.0, ENdhsFieldDirectionIncreasing, false, m_transitionDuration,
												ENdhsScrollFieldModeNormal, m_transitionVelProfile);

					// We need to cache the timestamp locally...Note that we don't
					// call 'updateValue' here, as we don't want the animator to start
					// until the delay has finished, unless delay is 0.
					if (m_transitionDelay == 0)
					{
						m_stateChange->updateValue(timestamp, finalUpdate);
					}

					m_interactiveAnimTimestamp = timestamp + m_transitionDelay;
					m_stateChangeAnimComplete = false;
				}
				else if (animType == ENdhsAnimationTypeTerminalState)
				{
					m_stateChange->setValue(0.0, false);
					m_stateChange->setTargetValue(1.0, ENdhsFieldDirectionIncreasing, false, m_transitionDuration,
												ENdhsScrollFieldModeNormal, m_transitionVelProfile);

					// We need to cache the timestamp locally...Note that we don't
					// call 'updateValue' here, as we don't want the animator to start
					// until the delay has finished, unless delay is 0 .
					if (m_transitionDelay == 0)
					{
						m_stateChange->updateValue(timestamp, finalUpdate);
					}

					m_terminalAnimTimestamp = timestamp + m_transitionDelay;
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
void NdhsCPageModel::resetTransitionCache()
{
	// furniture group
	if (m_outerGroup)
		m_outerGroup->resetTransitionCache();
}

/*-------------------------------------------------------------------------*//**
	Check whether we need to change the visibility status of the group
*/
bool NdhsCPageModel::doPrepareForFrameUpdate(LcTTime timestamp, bool& finalFrame)
{
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
			if (timestamp >= m_interactiveAnimTimestamp)
			{
				m_stateChange->updateValue(timestamp, finalUpdate);

				LcTScalar currentPos = m_stateChange->getRawFieldData(NULL);

				if (currentPos == 1 && m_stateChange->isAnimating() == false)
					finalUpdate = true;

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
				if (reschedule || finalUpdate)
					updatePosition(currentPos, true, false, finalUpdate);

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

		case ENdhsAnimationTypeTerminalState:
		{
			if (timestamp >= m_terminalAnimTimestamp)
			{
				m_stateChange->updateValue(timestamp, finalUpdate);

				LcTScalar currentPos = m_stateChange->getRawFieldData(NULL);

				if (currentPos == 1 && m_stateChange->isAnimating() == false)
					finalUpdate = true;

				// Check for animation complete
				if (m_lastFrame)
				{
					// Transition complete!
					m_stateChangeAnimComplete = true;
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
				if (reschedule || finalUpdate)
					updatePosition(currentPos, true, false, finalUpdate);
			}
			else
			{
				// We're waiting for a transition to start...
				reschedule = true;
			}

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
bool NdhsCPageModel::updateFields(LcTTime timestamp)
{
	bool reschedule = false;

	// Now check the static animation fields
	if (m_staticAnimFields.size())
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
*/
bool NdhsCPageModel::isTransitioning()
{
	bool status = (m_animType == ENdhsAnimationTypeTerminalState);

	if (status == false)
	{
		NdhsCElementGroup* agg = getPageAggregate();
		if (agg)
		{
			return agg->isComponentTransitioning();
		}
	}
	return status;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPageModel::processKeyUp(int code)
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
void NdhsCPageModel::layoutChanged(NdhsCTemplate::CLayout* newLayout,bool animate)
{
	m_isLayoutChange = true;
	m_currentLayout = newLayout;

	if(m_insideChangeState)
		return;

	jumpTransitionToEnd(false);

	m_transitionDuration = m_pageTemplate->getDefaultLayoutTime();
	m_transitionVelProfile = m_pageTemplate->getDefaultLayoutVelocityProfile();

	stopStaticAnimations(true, EAnimationAll);

	m_staticTransitionDone = false;
	if(startTransition(ENdhsAnimationTypeLayoutChange, animate) && !animate)
	{
		onTransitionComplete(false);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::doAction(NdhsCTemplate::CAction* action, int slotNum)
{
	// Only cancel any existing chained action if we have something to do
	m_chainedAction = "";

	// Now we have one or more things to attempt: if the first isn't
	// possible, we fall back to the second, and so forth until
	// either we have found something we can do or we have run out
	// of options
	bool attemptOk = false;

	typedef LcTmArray<NdhsCPageTemplate::CAction::CAttempt*> TmAAttempts;
	TmAAttempts::iterator itA = action->attempts.begin();

	for (; itA != action->attempts.end() && !attemptOk; itA++)
	{
		attemptOk = doAttempt(*itA, slotNum);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageModel::doAttempt(NdhsCTemplate::CAction::CAttempt* attempt, int slotNum)
{
	if (attempt == NULL)
		return false;

	bool attemptOk = false;

	// What we do depends on attempt type
	switch (attempt->attemptType)
	{
		// EScrollBy shifts field by the specified amount
		//
		//    amount : offset to change field by
		//    wrap : whether to 'wrap' between min and max value
		case NdhsCPageTemplate::CAction::CAttempt::EScrollBy:
		{
			NdhsCPageTemplate::CAction::CScrollBy* scrollAction = (NdhsCPageTemplate::CAction::CScrollBy*)(attempt);
			bool isKick = (scrollAction->type == NdhsCPageTemplate::CAction::CScrollBy::EScrollByTypeKick);
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

								if (minExpr->isError() || !minExpr->isNumeric())
								{
									minExpr->errorDiagnostics("ScrollBy minimum value", true);
								}
								else
								{
									minValue = minExpr->getValueScalar();
									minDefined = true;
								}
							}
						}

						if (scrollAction->maxDefined && scrollAction->maxExpr)
						{
							NdhsCExpression* maxExpr = scrollAction->maxExpr->getContextFreeExpression();

							if (maxExpr)
							{
								maxExpr->evaluate(this, slotNum, NULL);

								if (maxExpr->isError() || !maxExpr->isNumeric())
								{
									maxExpr->errorDiagnostics("ScrollBy maximum value", true);
								}
								else
								{
									maxValue = maxExpr->getValueScalar();
									maxDefined = true;
								}
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
								pField->setValue(pField->getTargetValue(), true);

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
				if(attemptOk)
				{
					m_chainedAction = scrollAction->action;
				}
				break;
			}

			// EBack
			//
			//    page : Page to go back to
			//    parentAction : Action to trigger on parent page
			case NdhsCPageTemplate::CAction::CAttempt::EBack:
			{
				if (m_pageState == ENdhsPageStateInteractive
						|| m_pageState == ENdhsPageStateSelected
						|| m_pageState == ENdhsPageStateHide)
				{
					if (m_pageState != ENdhsPageStateInteractive
						&&((NdhsCPageTemplate::CAction::CBack*)(attempt))->page.isEmpty())
					{
						// Need to do a back to this page
						((NdhsCPageTemplate::CAction::CBack*)(attempt))->backToLevel = m_pageManager->getPageStackLevel(this);

						m_chainedAction = ((NdhsCPageTemplate::CAction::CBack*)(attempt))->action;
					}
					else
					{
						((NdhsCPageTemplate::CAction::CBack*)(attempt))->backToLevel = -1;
					}

					attemptOk = m_pageManager->processAttempt(attempt, m_stackLevel);
				}

				break;
			}

			// ELink
			//
			//    uri : The link to launch in the browser
			case NdhsCPageTemplate::CAction::CAttempt::ELink:
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

							if (uriExpr->isError())
							{
								uriExpr->errorDiagnostics("Link action URI", true);
							}
							else
							{
								link = uriExpr->getValueString();
							}
						}
					}
				}
				else
				{
					moduleLink = true;
				}

				LcTaString expandedLink;
				LcTaString linkPrefix;

				if (m_pageState == ENdhsPageStateInteractive || m_pageState == ENdhsPageStateSelected)
				{
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
							if (m_pageState == ENdhsPageStateInteractive
								|| m_pageState == ENdhsPageStateSelected
								|| m_pageState == ENdhsPageStateHide)
							{
								// The link will open a menu, so we must stop the transition
								// and ask the page manager to handle the link
								if (m_animType != ENdhsAnimationTypeNone)
									jumpTransitionToEnd(false);

								attemptOk = m_pageManager->processAttempt(attempt, m_stackLevel, expandedLink);
							}
						}
					}
				}
			break;
		}

		// ESuspend
		//
		//    suspend : Put Ndhs to the background (soft exit)
		case NdhsCPageTemplate::CAction::CAttempt::ESuspend:
		{
			attemptOk = m_pageManager->processAttempt(attempt, m_stackLevel);

				break;
		}

		// EExit
		//
		//    exit : Exit Ndhs (hard exit)
		case NdhsCPageTemplate::CAction::CAttempt::EExit:
		{
			attemptOk = m_pageManager->processAttempt(attempt, m_stackLevel);

			break;
		}

		// ESetFocus
		//
		//    setFocus : Set the focus to the current item.
		case NdhsCPageTemplate::CAction::CAttempt::ESetFocus:
		{
			if (m_pageState == ENdhsPageStateInteractive)
			{
				LcTaString className = ((NdhsCTemplate::CAction::CSetFocus*)attempt)->className;
				if (className.isEmpty())
				{
					attemptOk = trySetFocus();
				}
				else if (className.compareNoCase("_parent") == 0)
				{
					attemptOk = false;
				}
				else
				{
					attemptOk = trySetFocus(className);
				}

				if (attemptOk)
				{
					executeChainedAction(((NdhsCTemplate::CAction::CSetFocus*)attempt)->action, slotNum);
				}
			}
			break;
		}

		// EUnsetFocus
		//
		//    unsetFocus : unset the current focus.
		case NdhsCPageTemplate::CAction::CAttempt::EUnsetFocus:
		{
			pageWideUnsetFocus();
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
			int maxTabCount = m_pageTemplate->getTabOrderCount();
			if (m_pageState == ENdhsPageStateInteractive && maxTabCount > 0)
			{
				int amount = ((NdhsCPageTemplate::CAction::CTabFocus*)(attempt))->amount;
				bool wrap	= ((NdhsCPageTemplate::CAction::CTabFocus*)(attempt))->wrap;

				if (amount != 0)
				{
					attemptOk = moveFocus(amount, wrap);

					if (attemptOk)
					{
						executeChainedAction(((NdhsCPageTemplate::CAction::CTabFocus*)(attempt))->action, slotNum);
					}
				}
			}

			break;
		}

		// EStop
		//
		//    Just consume the event, but do nothing
		case NdhsCPageTemplate::CAction::CAttempt::EStop:
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
void NdhsCPageModel::stateManagerDestroyed(NdhsCStateManager* sm)
{
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField*	NdhsCPageModel::getField(const LcTmString& field, int slotNum, NdhsCMenuItem* item)
{
	NdhsCField*	retVal = NULL;

	// First check local cache
	retVal = m_fieldCache->getField(field);

	if (!retVal && m_uiNode)
	{
		retVal = m_uiNode->getField(field, 0, NULL);
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

	if (!retVal)
	{
		// Check for page paramters
		retVal = getPageParamValue(field);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageModel::startStaticAnimations(EAnimationGroup groupToStart)
{
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
void NdhsCPageModel::stopStaticAnimations(bool immediateStop, EAnimationGroup groupToStop)
{
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
					m_pageManager->simulateKeyDown(m_pStaticTriggerList->cleanupTrigger);
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
void NdhsCPageModel::setStaticAnimationTrigger(NdhsCPageTemplate::CLayoutDecoration::CDecorationInfo::CTriggersList* pTriggerList)
{
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
NdhsCScrollPosField* NdhsCPageModel::getStaticAnimationField(NdhsCField::IObserver* observer)
{
	NdhsCScrollPosField* retVal = NULL;

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
void NdhsCPageModel::releaseStaticAnimationField(NdhsCScrollPosField* field)
{
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
*/
void NdhsCPageModel::updatePageStateField(ENdhsPageState pageState)
{
	NdhsCField* field = m_fieldCache->getField("_pagestate");

	if (field)
	{
		switch (pageState)
		{
			case ENdhsPageStateOpen:
			{
				field->setValue("open", true);
				break;
			}
			case ENdhsPageStateClose:
			{
				field->setValue("close", true);
				break;
			}
			case ENdhsPageStateInteractive:
			{
				field->setValue("interactive", true);
				break;
			}
			case ENdhsPageStateHide:
			{
				field->setValue("hide", true);
				break;
			}
			case ENdhsPageStateSelected:
			{
				field->setValue("selected", true);
				break;
			}
			case ENdhsPageStateShow:
			{
				field->setValue("show", true);
				break;
			}
			case ENdhsPageStateLaunch:
			{
				field->setValue("launch", true);
				break;
			}

			default:
				break;
		}

		// Propagate any changes immediately
		m_pageManager->getCon()->getLaundry()->cleanAll();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::updateTouchDownIntrinsicField(const LcTmString& element, int slot)
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
void NdhsCPageModel::updateMouseOverIntrinsicField(const LcTmString& element, int slot)
{
	LC_UNUSED(slot)

	if(m_fieldCache)
	{
		NdhsCField* field = m_fieldCache->getField("_mouseoverelement");
		if (field)
		{
			field->setValue(element, true);
		}
	}
}
#endif

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageModel::checkNaviation(const LcTmString& uri)
{
	// If we can't navigate away from this node, disallow
	if (m_uiNode && m_uiNode->allowNavigation())
	{
		// Invalid URI's not allowed
		if (uri.isEmpty())
			return false;

		// Check if navigation to uri allowed or not
		if (m_uiNode->isPageInList(uri))
			return true;
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCField* NdhsCPageModel::getPageParamValue(const LcTmString& key)
{
	TmMPageParams::iterator itParam = m_paramMap.find(key.toLower());

	// See if it is present in our param map
	if (itParam != m_paramMap.end())
	{
		return itParam->second.ptr();
	}
	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::onSuspend()
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
void NdhsCPageModel::onResume()
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
	Retrieve the plug-in index of the active item on a specified plug-in menu.
	Returns -1 for an error.
*/
int NdhsCPageModel::getHMenuActiveItemIndex(IFX_HMENU hMenu)
{
	if (hMenu == NULL)
		return -1;

	NdhsCElementGroup* agg = getPageAggregate();
	if (agg)
	{
		return agg->getHMenuActiveItemIndex(hMenu);
	}
	return -1;
}

/*-------------------------------------------------------------------------*//**
	Retrieve the full path of a file for a specified plug-in menu manifest file.
	Returns false for an error.
*/
bool NdhsCPageModel::getFullFilePath(	IFX_HMENU hMenu,
										const LcTmString& searchFile,
										LcTmString& returnFilePath,
										int index)
{
	if (hMenu == NULL || searchFile.isEmpty())
		return false;

	NdhsCElementGroup* agg = getPageAggregate();
	if (agg)
	{
		return agg->getFullFilePath(hMenu, searchFile, returnFilePath, index);
	}
	return false;

}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::componentRefreshField(const LcTmString& field, int item)
{
	if (field.isEmpty())
		return;

	NdhsCElementGroup* agg = getPageAggregate();
	if (agg)
	{
		agg->componentRefreshField(field, item);
	}
}

#if defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::componentOnTransitionComplete(bool setIdle)
{
	NdhsCElementGroup* agg = getPageAggregate();
	if (agg)
	{
		agg->componentOnTransitionComplete(setIdle);
	}
}
#endif

/*---------------------------------------------------------------------------*//**
*/
bool NdhsCPageModel::parsePageParamters()
{
	IFX_HUTILITY hUtility;
	IFX_INT32 count = 0;
	LcTWChar* ptr = NULL;
	bool paramError = false;
	LcTaString key = "", val = "", link = m_link;

	// Initialize URI Parser
	if (IFXU_UriParser_Initialize( &hUtility, link.bufWChar()) != IFX_SUCCESS)
		return false;

	// Get parameter count
	IFXU_UriParser_GetParameterCount (hUtility, &count);

	for (int i = 0; i < count; i++)
	{
		IFX_INT32 size = 0;
		paramError = false;

		if (IFXU_UriParser_GetParameterKeySize(hUtility, i, &size, NULL) == IFX_SUCCESS)
		{
			ptr = LcTmAlloc<LcTWChar>::allocUnsafe(size);

			if (ptr)
			{
				if (IFXU_UriParser_GetParameterKeyData(hUtility, i, ptr, NULL) == IFX_SUCCESS)
				{
					key = LcTaString(key.fromBufWChar(ptr, size).bufUtf8());
				}
				else
				{
					paramError = true;
				}

				LcTmAlloc<LcTWChar>::freeUnsafe(ptr);
			}
		}
		else
		{
			paramError = true;
		}

		if (paramError == false)
		{
			size = 0;

			// We can have paramter with empty values
			IFX_RETURN_STATUS status = IFXU_UriParser_GetParameterValueSize(hUtility, i, &size, NULL);
			if (status == IFX_SUCCESS)
			{
				ptr = LcTmAlloc<LcTWChar>::allocUnsafe(size);

				if (ptr)
				{
					status = IFXU_UriParser_GetParameterValueData(hUtility, i, ptr, NULL);
					if (status == IFX_SUCCESS)
					{
						val = LcTaString(val.fromBufWChar(ptr, size).bufUtf8());
					}
					else if (status == IFXU_URI_ERROR_INVALID_PARAM)
					{
						val = "";
					}
					else
					{
						paramError = true;
					}

					LcTmAlloc<LcTWChar>::freeUnsafe(ptr);
				}
			}
			else if (status == IFXU_URI_ERROR_INVALID_PARAM)
			{
				val = "";
			}
			else
			{
				paramError = true;
			}
		}

		if (paramError == false)
		{
			LcTaOwner<NdhsCField> field = NdhsCField::create(m_pageManager->getCon(), key, false, NULL, -1, NULL,
			IFXI_FIELD_MODE_OUTPUT, IFXI_FIELD_SCOPE_LOCAL, IFXI_FIELDDATA_STRING);

			if (field)
			{
				field->setValue(val, true);
				m_paramMap[key.toLower()] = field;
			}

		}
	}

	// Destory URI Parser
	IFXU_UriParser_Destroy( hUtility );

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageModel::trySetFocus()
{
	bool retVal = true;

	if (!m_pageTemplate->getDefaultFocus().isEmpty())
	{
		retVal = trySetFocus(m_pageTemplate->getDefaultFocus());
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::unsetFocus()
{
	if (m_pageState == ENdhsPageStateInteractive && !isHidden())
	{
		setFocusIntrinsics("");

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
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::focusSetOnChild(NdhsCComponent* childComponent, NdhsCComponent* focusComponent)
{
	setFocusIntrinsics(childComponent->getGroupName());

	m_focusedTargetComponent = focusComponent;
	m_focusedChildComponent = childComponent;

	// Clean any dirty fields
	m_pageManager->getCon()->getLaundry()->cleanAll();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::pageWideUnsetFocus()
{
	if (m_pageState == ENdhsPageStateInteractive)
	{
		unsetFocus();
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCPageModel::trySetFocus(const LcTmString& className)
{
	bool retVal = false;

	if (m_outerGroup)
	{
		NdhsCElement* elementToFocus = m_outerGroup->getItem(className);

		if (elementToFocus)
		{
			if(elementToFocus==m_focusedElement)
				return true;

			retVal = elementToFocus->trySetFocus();

			if (retVal)
			{
				pageWideUnsetFocus();
				m_focusedElement = elementToFocus;
				setFocusIntrinsics(m_focusedElement->getElementClassName());
			}
#ifdef NDHS_JNI_INTERFACE
			else
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleGeneral, "Tried to set focus to element " + className + " on page " + getGroupName() + " but failed");
			}
#endif
		}
		else
		{
			NdhsCElementGroup* groupToFocus = m_outerGroup->getGroup(className);

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
bool NdhsCPageModel::moveFocus(int amount, bool wrap)
{
	bool retVal = false;

	if (m_pageState != ENdhsPageStateInteractive)
	{
		NdhsCPageModel* activePage = m_pageManager->getActivePage();

		if (activePage != this)
		{
			retVal = m_pageManager->getActivePage()->moveFocus(amount, wrap);
		}
	}
	else if (m_outerGroup)
	{
		NdhsCComponent* previouslyFocusedComponent = m_focusedChildComponent;

		if (m_focusedChildComponent)
		{
			retVal = m_focusedChildComponent->moveFocus(amount, wrap);
		}

		if (!retVal)
		{
			int totalTabStops = getTemplate()->getTabOrderCount();

			if (totalTabStops > 0)
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
						if (amount > 0)
						{
							nextTabIndex = 0;
						}
						else
						{
							nextTabIndex = totalTabStops - 1;
						}
					}
					else
					{
						nextTabIndex = currentTabIndex + perTabIncrement;

						if (nextTabIndex < 0)
						{
							if (wrap)
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
							if (wrap)
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
							NdhsCElement* elementToFocus = m_outerGroup->getItem(nextTabData.className);

							if (elementToFocus)
							{
								retVal = elementToFocus->trySetFocus();

								if (retVal)
								{
									pageWideUnsetFocus();
									m_focusedElement = elementToFocus;
									setFocusIntrinsics(m_focusedElement->getElementClassName());
								}
#ifdef NDHS_JNI_INTERFACE
								else
								{
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleGeneral, "Tried to tab focus to element " + nextTabData.className + " on page " + getGroupName() + " but failed");
								}
#endif
							}
							else
							{
								NdhsCElementGroup* groupToFocus = m_outerGroup->getGroup(nextTabData.className);

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
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::setFocusIntrinsics(const LcTmString& focusedClass)
{
	if(m_fieldCache)
	{
		NdhsCField* field = m_fieldCache->getField("_focuselement");

		if (field)
		{
			field->setValue(focusedClass, true);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPageModel::checkVisibilityChanges()
{
	if (m_focusedChildComponent)
	{
		m_focusedChildComponent->checkVisibilityChanges();
	}
	else if (m_focusedElement)
	{
		if (!m_focusedElement->checkElementVisiblity())
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleGeneral, "Page " + getGroupName() + ": unsetting focus on " + m_focusedElement->getElementClassName() + " as it is invisble in the current layout");
			pageWideUnsetFocus();
		}
	}
}

#if defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCPageManager::CStaticPreviewCache> NdhsCPageModel::takeSnapShot()
{
	LcTaOwner<NdhsCPageManager::CStaticPreviewCache> info = NdhsCPageManager::CStaticPreviewCache::create();

	// Take snapshot of current page
	if (info)
	{
		info->slotNumber = -1;
		// no unique name for page needed at the moment
		info->componentClassName = "page";
		info->componentPath = m_templatePath;
		if (m_currentLayout)
			info->layoutName = m_currentLayout->name;
		info->identifier = m_identifier;
	}
	return info;
}

/*-------------------------------------------------------------------------*//**
*/
unsigned int NdhsCPageModel::distanceFromPage()
{
	return m_pageManager->getIdentifier();
}

#endif

/*-------------------------------------------------------------------------*//**
*/
LcTaString	NdhsCPageModel::getTouchDownElement()
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
