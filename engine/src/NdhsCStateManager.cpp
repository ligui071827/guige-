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
*/
LcTaOwner<NdhsCStateManager> NdhsCStateManager::create(NdhsCLaundry* laundry, NdhsCTemplate* templ, NdhsIFieldContext* context, NdhsCPageManager* pageManager)
{
	LcTaOwner<NdhsCStateManager> ref;
	ref.set(new NdhsCStateManager(laundry, pageManager));
	ref->construct(templ, context);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCStateManager::CState>	NdhsCStateManager::CState::create()
{
	LcTaOwner<CState> ref;
	ref.set(new CState());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCStateManager::CEventInfo>	NdhsCStateManager::CEventInfo::create()
{
	LcTaOwner<CEventInfo> ref;
	ref.set(new CEventInfo());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
																			 */
LcTaOwner<NdhsCStateManager::CSlotClassTrigger>	NdhsCStateManager::CSlotClassTrigger::create()
{
	LcTaOwner<CSlotClassTrigger> ref;
	ref.set(new CSlotClassTrigger());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCStateManager::CConditionBlock>	NdhsCStateManager::CConditionBlock::create()
{
	LcTaOwner<CConditionBlock> ref;
	ref.set(new CConditionBlock());
	//ref->construct();
	return ref;
}


/*-------------------------------------------------------------------------*//**
*/
void NdhsCStateManager::construct(NdhsCTemplate* templ, NdhsIFieldContext* context)
{
	m_fieldContext = context;

	NdhsCTemplate::TmMLayoutList& layoutList = templ->getLayoutList();

	NdhsCPageTemplate::TmMLayoutList::iterator layoutIt = layoutList.begin();

	for (; layoutIt != layoutList.end(); layoutIt++)
	{
		NdhsCPageTemplate::CState* baseState = (*layoutIt)->stateInfo.ptr();

		if (baseState)
		{
			LcTaOwner<CState> state = constructStateInfo(baseState, context);

			if (state)
			{
				m_states.push_back(state);
			}
		}
	}

	m_currentState = NULL;
}

void NdhsCStateManager::stopObservingFields()
{
	LcTmOwnerArray<CState>::iterator iter= m_states.begin();
	for(;iter!=m_states.end();iter++)
	{
		(*iter)->pressTriggers.clear();
		(*iter)->signalClassTriggers.clear();
		(*iter)->signalSlotTriggers.clear();
		(*iter)->signalSlotClassTriggers.clear();
		(*iter)->tapClassTriggers.clear();
		(*iter)->tapSlotTriggers.clear();
		if((*iter)->catchAllKeyPress)
		{
			(*iter)->catchAllKeyPress.release();
			(*iter)->catchAllKeyPress.destroy();
		}
		if((*iter)->catchAllStylusTap)
		{
			(*iter)->catchAllStylusTap.release();
			(*iter)->catchAllStylusTap.destroy();
		}
		(*iter)->condition.release();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCStateManager::CState> NdhsCStateManager::constructStateInfo(NdhsCPageTemplate::CState* baseStateInfo, NdhsIFieldContext* context)
{
	LcTaOwner<CState> state = CState::create();

	if (state && baseStateInfo)
	{
		LcTmOwnerMap<int, NdhsCPageTemplate::CEventInfo>::iterator pressIt = baseStateInfo->pressTriggers.begin();
		for (; pressIt != baseStateInfo->pressTriggers.end(); pressIt++)
		{
			LcTaOwner<CEventInfo> eventInfo = constructEventInfo(pressIt->second, context);
			state->pressTriggers.add_element(pressIt->first, eventInfo);
		}

		LcTmOwnerMap<int, NdhsCPageTemplate::CEventInfo>::iterator tapSlotIt = baseStateInfo->tapSlotTriggers.begin();
		for (; tapSlotIt != baseStateInfo->tapSlotTriggers.end(); tapSlotIt++)
		{
			LcTaOwner<CEventInfo> eventInfo = constructEventInfo(tapSlotIt->second, context);
			state->tapSlotTriggers.add_element(tapSlotIt->first, eventInfo);
		}

		LcTmOwnerMap<LcTmString, NdhsCPageTemplate::CEventInfo>::iterator tapClassIt = baseStateInfo->tapClassTriggers.begin();
		for (; tapClassIt != baseStateInfo->tapClassTriggers.end(); tapClassIt++)
		{
			LcTaOwner<CEventInfo> eventInfo = constructEventInfo(tapClassIt->second, context);
			state->tapClassTriggers.add_element(tapClassIt->first, eventInfo);
		}

		LcTmOwnerMap<int, NdhsCPageTemplate::CEventInfo>::iterator signalSlotIt = baseStateInfo->signalSlotTriggers.begin();
		for (; signalSlotIt != baseStateInfo->signalSlotTriggers.end(); signalSlotIt++)
		{
			LcTaOwner<CEventInfo> eventInfo = constructEventInfo(signalSlotIt->second, context);
			state->signalSlotTriggers.add_element(signalSlotIt->first, eventInfo);
		}

		LcTmOwnerMap<LcTmString, NdhsCPageTemplate::CEventInfo>::iterator signalClassIt = baseStateInfo->signalClassTriggers.begin();
		for (; signalClassIt != baseStateInfo->signalClassTriggers.end(); signalClassIt++)
		{
			LcTaOwner<CEventInfo> eventInfo = constructEventInfo(signalClassIt->second, context);
			state->signalClassTriggers.add_element(signalClassIt->first, eventInfo);
		}

		NdhsCTemplate::TmASlotClassTriggerList::iterator signalClassSlotIt = baseStateInfo->signalSlotClassTriggers.begin();
		for (; signalClassSlotIt != baseStateInfo->signalSlotClassTriggers.end(); signalClassSlotIt++)
		{
			LcTaOwner<CSlotClassTrigger> onSignalInfo = CSlotClassTrigger::create();
			onSignalInfo->elementClass = (*signalClassSlotIt)->elementClass;
			onSignalInfo->slot = (*signalClassSlotIt)->slot;
			onSignalInfo->eventInfo = constructEventInfo((*signalClassSlotIt)->eventInfo, context);
			state->signalSlotClassTriggers.push_back(onSignalInfo);
		}

		if (baseStateInfo->catchAllKeyPress)
		{
			LcTaOwner<CEventInfo> eventInfo = constructEventInfo(baseStateInfo->catchAllKeyPress, context);
			state->catchAllKeyPress = eventInfo;
		}

		if (baseStateInfo->catchAllStylusTap)
		{
			LcTaOwner<CEventInfo> eventInfo = constructEventInfo(baseStateInfo->catchAllStylusTap, context);
			state->catchAllStylusTap = eventInfo;
		}

		if (baseStateInfo->condition)
		{
			state->condition = baseStateInfo->condition->createExpression(context);

			if (state->condition)
			{
				state->condition->setObserver(this);
			}
		}

		state->layout = baseStateInfo->layout;
	}

	return state;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCStateManager::CEventInfo> NdhsCStateManager::constructEventInfo(NdhsCPageTemplate::CEventInfo* baseEventInfo, NdhsIFieldContext* context)
{
	LcTaOwner<CEventInfo> eventInfo = CEventInfo::create();

	LcTmOwnerArray<NdhsCPageTemplate::CConditionBlock>::iterator conditionIt = baseEventInfo->conditions.begin();

	for (; conditionIt != baseEventInfo->conditions.end(); conditionIt++)
	{
		LcTaOwner<CConditionBlock> conditionBlock = constructConditionBlock((*conditionIt), context);
		eventInfo->conditions.push_back(conditionBlock);
	}

	return eventInfo;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCStateManager::CConditionBlock> NdhsCStateManager::constructConditionBlock(NdhsCPageTemplate::CConditionBlock* baseConditionBlock, NdhsIFieldContext* context)
{
	LcTaOwner<CConditionBlock> condBlock = CConditionBlock::create();

	condBlock->guardExpr = baseConditionBlock->guardExpr->getContextFreeExpression();

	LcTmOwnerMap<LcTmString, NdhsCExpression::CExprSkeleton>::iterator assignmentIterator = baseConditionBlock->assignments.begin();

	for (; assignmentIterator != baseConditionBlock->assignments.end(); assignmentIterator++)
	{
		condBlock->assignments[assignmentIterator->first] = assignmentIterator->second->getContextFreeExpression();
	}

	condBlock->action = baseConditionBlock->action;

	return condBlock;
}

/*-------------------------------------------------------------------------*//**
	Add a field observer
*/
void NdhsCStateManager::addObserver(IObserver* obs)
{
	if (obs)
		m_observers.push_back(obs);
}

/*-------------------------------------------------------------------------*//**
	Remove a field observer
*/
void NdhsCStateManager::removeObserver(IObserver* obs)
{
	bool observerFound = false;
	LcTmArray<IObserver*>::iterator it = m_observers.begin();

	while (it != m_observers.end() && !observerFound)
	{
		if ((*it) == obs)
		{
			it = m_observers.erase(it);
			observerFound = true;
		}
		else
		{
			it++;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCStateManager::processKeypress(int scancode)
{
	bool consumed = false;

	if (m_currentState)
	{
		LcTmOwnerMap<int, CEventInfo>::iterator keyEventIt = m_currentState->pressTriggers.find(scancode);

		if (keyEventIt != m_currentState->pressTriggers.end())
		{
			consumed = processEventInfo(keyEventIt->second, -1);
		}
		else if (m_currentState->catchAllKeyPress)
		{
			consumed = processEventInfo(m_currentState->catchAllKeyPress.ptr(), -1);
		}
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCStateManager::processSlotSignal(int slot)
{
	bool consumed = false;

	if (m_currentState)
	{
		LcTmOwnerMap<int, CEventInfo>::iterator signalEventIt = m_currentState->signalSlotTriggers.find(slot);

		if (signalEventIt != m_currentState->signalSlotTriggers.end())
		{
			consumed = processEventInfo(signalEventIt->second, slot);
		}
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCStateManager::processSlotClassNameSignal(int slot, const LcTmString& className)
{
	bool consumed = false;

	if (m_currentState)
	{
		CSlotClassTrigger *trigger = NULL;
		TmASlotClassTriggerList::iterator it = m_currentState->signalSlotClassTriggers.begin();

		for (; it != m_currentState->signalSlotClassTriggers.end() && trigger == NULL; it++)
		{
			if (*it)
			{
				// We should not call processEventInfo below, as we may end up having incompatible
				// iterator in certain scenarios
				if ((*it)->slot == slot && (*it)->elementClass.compareNoCase(className) == 0)
				{
					trigger = *it;
				}
			}
		}
		if (trigger)
			consumed = processEventInfo(trigger->eventInfo.ptr(), slot);
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCStateManager::processClassSignal(const LcTmString& className)
{
	bool consumed = false;

	if (m_currentState)
	{
		LcTmOwnerMap<LcTmString, CEventInfo>::iterator signalEventIt = m_currentState->signalClassTriggers.find(className);

		if (signalEventIt != m_currentState->signalClassTriggers.end())
		{
			consumed = processEventInfo(signalEventIt->second, -1);
		}
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCStateManager::processSlotTap(int slot,const LcTmString& className)
{
	bool consumed = false;

	if (m_currentState)
	{
		LcTmOwnerMap<int, CEventInfo>::iterator tapEventIt = m_currentState->tapSlotTriggers.find(slot);
		LcTmOwnerMap<LcTmString, CEventInfo>::iterator classTapEventIt = m_currentState->tapClassTriggers.find(className);

		if (tapEventIt != m_currentState->tapSlotTriggers.end())
		{
			consumed = processEventInfo(tapEventIt->second, slot);
		}
		else if (classTapEventIt != m_currentState->tapClassTriggers.end())
		{
			consumed = processEventInfo(classTapEventIt->second, slot);
		}
		else if (m_currentState->catchAllStylusTap)
		{
			consumed = processEventInfo(m_currentState->catchAllStylusTap.ptr(), slot);
		}
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCStateManager::processClassTap(const LcTmString& className)
{
	bool consumed = false;

	if (m_currentState)
	{
		LcTmOwnerMap<LcTmString, CEventInfo>::iterator tapEventIt = m_currentState->tapClassTriggers.find(className);

		if (tapEventIt != m_currentState->tapClassTriggers.end())
		{
			consumed = processEventInfo(tapEventIt->second, -1);
		}
		else if (m_currentState->catchAllStylusTap)
		{
			consumed = processEventInfo(m_currentState->catchAllStylusTap.ptr(), -1);
		}
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCStateManager::processEventInfo(CEventInfo* eventInfo, int slotNum)
{
	bool consumed = false;

	if (eventInfo)
	{
		LcTmOwnerArray<CConditionBlock>::iterator conditionIt = eventInfo->conditions.begin();

		while (!consumed && conditionIt != eventInfo->conditions.end())
		{
			CConditionBlock* condBlock = *conditionIt;

			if (condBlock->guardExpr)
			{
				condBlock->guardExpr->evaluate(m_fieldContext, slotNum, NULL);

				if (condBlock->guardExpr->isError())
				{
					condBlock->guardExpr->errorDiagnostics("Action guard", true);
				}
				else
				{
					if (condBlock->guardExpr->getValueBool() && m_fieldContext)
					{
						consumed = true;

						// Do assignments
						LcTmOwnerMap<LcTmString, NdhsCExpression>::iterator assignIt = condBlock->assignments.begin();
						for (; assignIt != condBlock->assignments.end(); assignIt++)
						{
							NdhsCField* field = m_fieldContext->getField(assignIt->first, slotNum, NULL);
							NdhsCExpression* expr = assignIt->second;

							if (field && expr)
							{
								if (field->isInput())
								{
									field->setDirtyByAssignment(expr, m_fieldContext, slotNum, NULL);
								}
							}
						}
						m_laundry->cleanAll();

						if (condBlock->action)
						{
							broadcastAction(condBlock->action, slotNum);
						}
					}
				}
			}

			if (!consumed)
			{
				conditionIt++;
			}
		}
	}

	return consumed;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCStateManager::broadcastAction(NdhsCPageTemplate::CAction*	action, int slotNum)
{
	// Take a copy of subscriber list, in case anything we call unsubscribes / resubscribes
	// whilst we're processing the list (e.g. a NdhsCElement reloading itself)
	LcTaArray<IObserver*> observerListCopy;
	observerListCopy.assign(m_observers.begin(), m_observers.end());

	LcTmArray<IObserver*>::iterator it = observerListCopy.begin();

	for (;it != observerListCopy.end(); it++)
	{
		IObserver* obs = (*it);

		if (obs && findObserver(obs))
		{
			obs->doAction(action, slotNum);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCStateManager::broadcastLayoutChange(NdhsCTemplate::CLayout* newLayout,bool animate)
{
	// Take a copy of subscriber list, in case anything we call unsubscribes / resubscribes
	// whilst we're processing the list (e.g. a NdhsCElement reloading itself)
	LcTaArray<IObserver*> observerListCopy;
	observerListCopy.assign(m_observers.begin(), m_observers.end());

	LcTmArray<IObserver*>::iterator it = observerListCopy.begin();

	for (;it != observerListCopy.end(); it++)
	{
		IObserver* obs = (*it);

		if (obs && findObserver(obs))
		{
			obs->layoutChanged(newLayout,animate);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCStateManager::broadcastStateManagerDestoyed()
{
	// Take a copy of subscriber list, in case anything we call unsubscribes / resubscribes
	// whilst we're processing the list (e.g. a NdhsCElement reloading itself)
	LcTaArray<IObserver*> observerListCopy;
	observerListCopy.assign(m_observers.begin(), m_observers.end());

	LcTmArray<IObserver*>::iterator it = observerListCopy.begin();

	for (;it != observerListCopy.end(); it++)
	{
		IObserver* obs = (*it);

		if (obs && findObserver(obs))
		{
			obs->stateManagerDestroyed(this);
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Used for double-checking an observer is still in the live array before
	contacting them
*/
bool NdhsCStateManager::findObserver(IObserver* obs)
{
	bool observerFound = false;

	LcTmArray<IObserver*>::iterator findObsIt = m_observers.begin();
	for (; findObsIt != m_observers.end() && !observerFound; findObsIt++)
	{
		if ((*findObsIt) == obs)
		{
			observerFound = true;
		}
	}

	return observerFound;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCStateManager::cleanLaundryItem(LcTTime timestamp)
{
	if (m_dirty)
	{
		testLayoutConditions();

		m_dirty = m_stillDirty;
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCStateManager::testLayoutConditions(bool force)
{
	CState* oldState = m_currentState;
	CState* newState = NULL;

	m_testingLayoutConditions = true;

	LcTmOwnerArray<CState>::iterator stateIt = m_states.begin();

#if defined(NDHS_JNI_INTERFACE)
	LcTaString layoutName = "";
	NdhsCPageManager::CStaticPreviewCache* entry = NULL;

	if(m_pageManager->isInplaceEditMode())
	{
		if (m_snapShot)
		{
			entry = m_pageManager->getCachedEntry(m_snapShot.ptr());
			if (entry)
			{
				layoutName = entry->layoutName;
			}
		}
	}
#endif

	while(stateIt != m_states.end() && newState == NULL)
	{
#if defined(NDHS_JNI_INTERFACE)
		if (layoutName.isEmpty() == false && m_pageManager->isInplaceEditMode())
		{
			if ((*stateIt)->layout->name.compareNoCase(layoutName) == 0)
			{
				newState = (*stateIt);
				layoutName = "";
			}
		}
		else
		{
#endif
			if ((*stateIt)->condition)
			{
				(*stateIt)->condition->evaluate();

				if ((*stateIt)->condition->isError())
				{
					(*stateIt)->condition->errorDiagnostics("Layout condition", true);
				}
				else
				{
					if ((*stateIt)->condition->getValueBool())
					{
						newState = (*stateIt);
					}
				}
			}
#if defined(NDHS_JNI_INTERFACE)
		}
#endif
		stateIt++;

	}

	if ((force || newState != oldState) && m_observers.size() > 0
#if defined(NDHS_JNI_INTERFACE)	 
	&& m_allowLayoutChange
#endif	
	)
	{
		m_currentState = newState;
		/* If there is no layout selected disable the current layout by replacing
		   with NULL layout which will create an empty layout.*/
		broadcastLayoutChange(newState == NULL ? NULL : newState->layout,oldState!=NULL);
	}

	m_testingLayoutConditions = false;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCStateManager::expressionDirty(NdhsCExpression* expr)
{
	if (!m_dirty || m_testingLayoutConditions)
	{
		LcTmOwnerArray<CState>::iterator stateIt = m_states.begin();
		bool ignore = false;

		while (stateIt != m_states.end() && !ignore && (!m_dirty || m_testingLayoutConditions))
		{
			CState* state = *stateIt;
			NdhsCExpression* stateCondition = state->condition.ptr();

			if (stateCondition == expr)
			{
				if (m_testingLayoutConditions)
				{
					// If an expression has been dirtied while testing the layout conditions, re-add this to the laundry
					m_stillDirty = true;

					// We can ignore the rest
					ignore = true;
				}
				else
				{
					m_dirty = true;
					m_laundry->addItem(this);
				}
			}

			if (state == m_currentState)
			{
				// We can ignore the rest
				ignore = true;
			}

			stateIt++;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCStateManager::expressionDestroyed(NdhsCExpression* expr)
{
}

#if defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
*/
void NdhsCStateManager::forceLayout(LcTmString layout)
{
	CState* oldState = m_currentState;
	CState* newState = NULL;

	LcTmOwnerArray<CState>::iterator stateIt = m_states.begin();

	while(stateIt != m_states.end() && newState == NULL)
	{
		LcTmString tmpString = (*stateIt)->layout->name;

		if (tmpString.compareNoCase(layout.toLower()) == 0)
		{
			newState = (*stateIt);
		}

		stateIt++;
	}

	if (newState)
	{
		m_currentState = newState;
		broadcastLayoutChange(newState->layout,false);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCStateManager::setAggregateSnapShotInfo(LcTmOwner<NdhsCPageManager::CStaticPreviewCache> info)
{
	if (m_snapShot)
	{
		m_snapShot->componentClassName = info->componentClassName;
		m_snapShot->componentPath = info->componentPath;
		m_snapShot->identifier = info->identifier;
		m_snapShot->layoutName = info->layoutName;
		m_snapShot->slotNumber = info->slotNumber;
	}
	else
	{
		m_snapShot = info;
	}
}
#endif //defined(NDHS_JNI_INTERFACE)

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCStateManager::isCatchAll()
{
	bool retVal = false;

	if (m_currentState)
	{
		retVal = (m_currentState->catchAllStylusTap.ptr() != NULL);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCStateManager::~NdhsCStateManager()
{
	if (m_laundry)
	{
		m_laundry->removeItem(this);
	}

	broadcastStateManagerDestoyed();
}

/*-------------------------------------------------------------------------*//**
*/
ENdhsPageState	NdhsCStateManager::getStateInfo()
{
	ENdhsPageState pageState = ENdhsPageStateNone;

	if (m_currentState && m_currentState->condition)
		pageState = m_currentState->condition->getPageState();

	return pageState;
}
