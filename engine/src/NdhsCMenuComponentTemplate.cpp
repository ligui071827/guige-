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


#define AGG_MASK (LcTPlacement::ELocation | LcTPlacement::EScale | LcTPlacement::EOrientation | LcTPlacement::EOpacity | LcTPlacement::EOffset)

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuComponentTemplate::CMenuAction> NdhsCMenuComponentTemplate::CMenuAction::create()
{
	LcTaOwner<CMenuAction> ref;
	ref.set(new CMenuAction());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuComponentTemplate::CMenuAction::CJumpBy>			NdhsCMenuComponentTemplate::CMenuAction::CJumpBy::create()
{
	LcTaOwner<CJumpBy> ref;
	ref.set(new CJumpBy());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuComponentTemplate::CMenuAction::CJumpTo>			NdhsCMenuComponentTemplate::CMenuAction::CJumpTo::create()
{
	LcTaOwner<CJumpTo> ref;
	ref.set(new CJumpTo());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuComponentTemplate::CMenuAction::CSelect>			NdhsCMenuComponentTemplate::CMenuAction::CSelect::create()
{
	LcTaOwner<CSelect> ref;
	ref.set(new CSelect());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuComponentTemplate::CMenuAction::CDeactivate>		NdhsCMenuComponentTemplate::CMenuAction::CDeactivate::create()
{
	LcTaOwner<CDeactivate> ref;
	ref.set(new CDeactivate());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMenuComponentTemplate> NdhsCMenuComponentTemplate::create(NdhsCPageManager* pageManager, LcTmOwner<LcCXmlElem>& root)
{
	LcTaOwner<NdhsCMenuComponentTemplate> ref;
	ref.set(new NdhsCMenuComponentTemplate(pageManager, root));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuComponentTemplate::NdhsCMenuComponentTemplate(NdhsCPageManager* pageManager, LcTmOwner<LcCXmlElem>& root)
{
	m_pageManager = pageManager;
	m_root = root;

	// First active of -1 used for inactive layout
	// If first active is not specified in the XML
	// file, leave it as -1
	m_firstActive = -1;

}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMenuComponentTemplate::~NdhsCMenuComponentTemplate()
{
	cleanup();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponentTemplate::cleanup()
{
	//
	// Settings Element
	//

	m_slotCount = 0;
	m_firstSelectable = 0;
	m_lastSelectable = 0;
	m_firstActive = -1;
	m_populateFrom = -1;
	m_defaultLayoutTime = 0;
	m_defaultScrollTime = 0;
	m_defaultTerminalTime = 0;
	m_defaultLayoutVelocityProfile = ENdhsVelocityProfileLinear;
	m_defaultScrollVelocityProfile = ENdhsVelocityProfileLinear;
	m_defaultTerminalVelocityProfile = ENdhsVelocityProfileLinear;
	m_fullMode = false;
	m_scrollWrap = false;
	m_scrollSpan = 1;
	m_scrollRebound = 0;

	//
	// Classes Element
	//
	m_itemClasses.clear();
}

/*-------------------------------------------------------------------------*//**
	Sets any default values in a fully merged layout that have not already been
	manually configured by the XML layout
*/
void NdhsCMenuComponentTemplate::setDefaultValues(CLayout& layout)
{
	// Set default primary light ambiance if not already set
	if (!(layout.primaryLightLayout.mask & LcTPlacement::EColor2))
	{
		layout.primaryLightLayout.mask |= LcTPlacement::EColor2;
		layout.primaryLightLayout.layout.color2 = LcTColor::GRAY50;
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenuComponentTemplate::configureFromXml(const LcTmString& designSize, int stackLevel, int& nestedComponentLevel)
{
	LcTaString err;
	bool result = false;
	bool success = true;

	cleanup();

	if (m_root)
	{
		// Loop simply allow us to break out if an error occurs
		do
		{
			//
			// Settings Element
			//

			if (false == (success = configureSettingsFromXml(m_root->find(NDHS_TP_XML_SETTINGS))))
			{
				break;
			}


			//
			// Actions Element
			//

			LcCXmlElem* eActions = m_root->find(NDHS_TP_XML_ACTIONS);
			if (eActions == NULL)
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Actions section missing");
			}

			LcCXmlElem* eAction = eActions ? eActions->getFirstChild() : NULL;
			for (; eAction && success; eAction = eAction->getNext())
			{
				if (eAction->getName().compareNoCase(NDHS_TP_XML_ACTION) != 0)
					continue;

				success = configureActionFromXml(eAction);
			}


			if (!success)
				break;


			// Configure any default actions
			success = configureDefaultActions();


			if (!success)
				break;

			//
			// Classes Element
			//

			if (false == (success = configureClassesFromXml(m_root->find(NDHS_TP_XML_CLASSES), stackLevel, nestedComponentLevel)))
			{
				break;
			}


			//
			// Layouts Element
			//

			LcCXmlElem* eLayouts = m_root->find(NDHS_TP_XML_LAYOUTS);
			LcCXmlElem* eLayout = eLayouts ? eLayouts->getFirstChild() : NULL;

			success = true;
			for (; eLayout && success; eLayout = eLayout->getNext())
			{
				if (eLayout->getName().compareNoCase(NDHS_TP_XML_LAYOUT) != 0)
					continue;

				success = configureLayoutFromXml(eLayout, stackLevel);
			}



			if (!success || !eLayouts)
			{
				NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Layouts section missing");
				success = false;
				break;
			}

			//
			// Animations Element
			//

			LcCXmlElem* eAnimations = m_root->find(NDHS_TP_XML_ANIMATIONS);
			LcCXmlElem* eAnimation = eAnimations ? eAnimations->getFirstChild() : NULL;
			for (; eAnimation && success; eAnimation = eAnimation->getNext())
			{
				if (eAnimation->getName().compareNoCase(NDHS_TP_XML_ANIMATION) != 0)
					continue;

				success = configureAnimationFromXml(eAnimation, stackLevel);
			}

			if (!success)
				break;

			//
			// Decorations Element
			//

			LcCXmlElem* eDecorations = m_root->find(NDHS_TP_XML_DECORATIONS);
			LcCXmlElem* rootNode = eDecorations ? eDecorations->getFirstChild() : NULL;
			for (; rootNode && success; rootNode = rootNode->getNext())
			{

					configureStaticDecorationFromXml(rootNode);
				// Not critical if this fails, just go onto the next one
			}
			if (!success)
				break;

		} while (false);

		result = success;

		m_root.destroy();
	}


	if (result == false)
		cleanup();

	return result;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponentTemplate::configureActionFromXml(LcCXmlElem* eAction)
{
	// Get the action name - mandatory
	LcTaString actionName = eAction->getAttr(NDHS_TP_XML_NAME).toLower();
	if (actionName.length() <= 0)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Action name is missing");
		return false;
	}

	if (m_actions.find(actionName) != m_actions.end())
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Action name already exists");

		// Return true so that it still loads and just uses the first action of that name
		return true;
	}

	LcTaOwner<NdhsCTemplate::CAction> action = NdhsCTemplate::CAction::create();

	// We could end up with an action  with no attempts,
	// but that wont cause a problem

	// Look for child tags of this action, and add them in order
	// (order is important in determining precedence of actions to take!)
	LcCXmlElem* eAttempt = eAction->getFirstChild();

	if (eAttempt == NULL)
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Action has no attempts specified");
	}

	for (; eAttempt; eAttempt = eAttempt->getNext())
	{
		configureAttemptFromXml(eAttempt, action.ptr());
	}

	m_actions.add_element(actionName, action);

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMenuComponentTemplate::configureAttemptFromXml(LcCXmlElem* eAttempt, CAction* action)
{
	LcTaString attemptName = eAttempt->getName().toLower();

	if (attemptName.compare(NDHS_TP_XML_JUMP_BY) == 0)
	{
		int amount = eAttempt->getAttr(NDHS_TP_XML_AMOUNT, "0").toInt();
		bool wrap = LcCXmlAttr::strToBool(eAttempt->getAttr(NDHS_TP_XML_WRAP));

		// Fail if amount is zero, which does nothing
		if (amount != 0)
		{
			LcTaOwner<CMenuAction::CJumpBy> attempt = CMenuAction::CJumpBy::create();

			attempt->attemptType = CAction::CAttempt::EJumpBy;
			attempt->amount = amount;
			attempt->wrap = wrap;
			attempt->action	= eAttempt->getAttr(NDHS_TP_XML_ACTION).toLower();
			attempt->velocityProfile = strToVelocityProfile(eAttempt->getAttr(NDHS_TP_XML_VELOCITY_PROFILE).toLower());

			LcCXmlAttr* attr = eAttempt->findAttr(NDHS_TP_XML_DURATION);

			if (attr)
			{
				attempt->duration = attr->getVal().toInt();

				if (attempt->duration < 0)
				{
					// Set to -1 so that we use the default
					attempt->duration = -1;

					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "JumpBy duration must not be negative");
				}
			}
			else
			{
				// Set to -1 so that we use the default
				attempt->duration = -1;
			}

			// Fill in the defaults for velocity profile and duration if unspecified
			if (ENdhsVelocityProfileUnknown == attempt->velocityProfile)
			{
				attempt->velocityProfile = m_defaultLayoutVelocityProfile;
			}

			if (-1 == attempt->duration)
			{
				attempt->duration = m_defaultLayoutTime;
			}

			action->attempts.push_back((LcTaOwner<CAction::CAttempt>&)attempt);
		}
		else
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'jumpBy' amount 0 is not permitted");
		}
	}
	else if (attemptName.compare(NDHS_TP_XML_JUMP_TO) == 0)
	{
		int slot = eAttempt->getAttr(NDHS_TP_XML_SLOT, "-1").toInt();

		if (slot == 0)
		{
			// Set the slot to -1 so that it is ignored
			slot = -1;
		}
		else if (slot > 0)
		{
			slot--;
		}

		// Check within selectable range
		if (slot >= m_firstSelectable && slot <= m_lastSelectable)
		{
			LcTaOwner<CMenuAction::CJumpTo> attempt = CMenuAction::CJumpTo::create();

			attempt->attemptType = CAction::CAttempt::EJumpTo;
			attempt->slot = slot;
			attempt->action	= eAttempt->getAttr(NDHS_TP_XML_ACTION).toLower();
			attempt->velocityProfile = strToVelocityProfile(eAttempt->getAttr(NDHS_TP_XML_VELOCITY_PROFILE).toLower());

			LcCXmlAttr* attr = eAttempt->findAttr(NDHS_TP_XML_DURATION);

			if (attr)
			{
				attempt->duration = attr->getVal().toInt();

				if (attempt->duration < 0)
				{
					// Set to -1 so that we use the default
					attempt->duration = -1;

					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "JumpTo duration must not be negative");
				}
			}
			else
			{
				// Set to -1 so that we use the default
				attempt->duration = -1;
			}

			// Fill in the defaults for velocity profile and duration if unspecified
			if (ENdhsVelocityProfileUnknown == attempt->velocityProfile)
			{
				attempt->velocityProfile = m_defaultLayoutVelocityProfile;
			}

			if (-1 == attempt->duration)
			{
				attempt->duration = m_defaultLayoutTime;
			}

			action->attempts.push_back((LcTaOwner<CAction::CAttempt>&)attempt);
		}
		else
		{
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'jumpTo' slot value must be between firstSelectable and lastSelectable");
		}
	}
	else if (attemptName.compare(NDHS_TP_XML_DEACTIVATE) == 0)
	{
		LcTaOwner<CMenuAction::CDeactivate> attempt = CMenuAction::CDeactivate::create();

		attempt->attemptType	= CAction::CAttempt::EDeactivate;
		attempt->action			= eAttempt->getAttr(NDHS_TP_XML_ACTION).toLower();
		attempt->velocityProfile = strToVelocityProfile(eAttempt->getAttr(NDHS_TP_XML_VELOCITY_PROFILE).toLower());

		LcCXmlAttr* attr = eAttempt->findAttr(NDHS_TP_XML_DURATION);

		if (attr)
		{
			attempt->duration = attr->getVal().toInt();

			if (attempt->duration < 0)
			{
				// Set to -1 so that we use the default
				attempt->duration = -1;

				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Deactivate duration must not be negative");
			}
		}
		else
		{
			// Set to -1 so that we use the default
			attempt->duration = -1;
		}

		// Fill in the defaults for velocity profile and duration if unspecified
		if (ENdhsVelocityProfileUnknown == attempt->velocityProfile)
		{
			attempt->velocityProfile = m_defaultLayoutVelocityProfile;
		}

		if (-1 == attempt->duration)
		{
			attempt->duration = m_defaultLayoutTime;
		}

		action->attempts.push_back((LcTaOwner<CAction::CAttempt>&)attempt);
	}
	else
	{
		// Pass to base class to process
		NdhsCTemplate::configureAttemptFromXml(eAttempt, action);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponentTemplate::configureDefaultActions()
{
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponentTemplate::configureSettingsFromXml(LcCXmlElem* eSettings)
{
	LcCXmlAttr* a;

	if (!eSettings)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Settings section missing");
		return false;
	}

	m_dataSource = NdhsCExpression::CExprSkeleton::create(eSettings->getAttr(NDHS_TP_XML_DATA_SOURCE));

	LcTaString templateType = eSettings->getAttr(NDHS_TP_XML_TEMPLATE_TYPE);
	m_templateType = determineTemplateType(templateType);

#ifdef LC_USE_LIGHTS
	m_lightModel = ENdhsLightModelNormal;
	LcTaString lightModel = eSettings->getAttr(NDHS_TP_XML_LIGHT_MODEL, "normal");
	if (lightModel.compareNoCase("simple") == 0)
	{
		m_lightModel = ENdhsLightModelSimple;
	}
#endif

	// Scroll Wrap When Equal - defaults to false
	LcCXmlElem* eScrollSettings = eSettings->find(NDHS_TP_XML_SCROLL);
	if (eScrollSettings)
	{
		m_fullMode		 	= LcCXmlAttr::strToBool(eScrollSettings->getAttr(NDHS_TP_XML_FULL));
		m_scrollWrap 	 	= LcCXmlAttr::strToBool(eScrollSettings->getAttr(NDHS_TP_XML_WRAP));


		a = eScrollSettings->findAttr(NDHS_TP_XML_SPAN);
		if (a)
		{
			m_scrollSpan	= a->getVal().toInt();
		}
		else
		{
			m_scrollSpan	= 1;
		}

		// Get rebound value
		m_scrollRebound = eScrollSettings->getAttr(NDHS_TP_XML_REBOUND, "0").toScalar();
		if (m_scrollRebound < 0)
		{
			m_scrollRebound = 0;
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Scroll rebound cannot be negative");
		}
		else if (m_scrollRebound > 0.999f)
		{
			m_scrollRebound = 0.999f;
			NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Scroll capped at .999");
		}

		// Identify the kick options.
		// Max velocity defaults to 'infinite'
		m_scrollKickMaxVelocity = -1;
		LcCXmlElem* eScrollKickSettings = eScrollSettings->find(NDHS_TP_XML_KICK);
		if (eScrollKickSettings)
		{
			// kick maxVelocity option.
			a = eScrollKickSettings->findAttr(NDHS_TP_XML_MAX_VELOCITY);
			if (a)
			{
				m_scrollKickMaxVelocity	= a->getVal().toScalar();
				if (m_scrollKickMaxVelocity < 0)
				{
					m_scrollKickMaxVelocity = -1;
					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "kick maxVelocity cannot be negative");
				}
			}
		}
	}
	else
	{
		m_fullMode		 	= false;
		m_scrollWrap 		= false;
		m_scrollSpan		= 1;
		m_scrollRebound     = 0;
	}

	// Slots element
	LcCXmlElem* eSlots = eSettings->find(NDHS_TP_XML_SLOTS);
	if (eSlots)
	{
		// Slot Count
		a = eSlots->findAttr(NDHS_TP_XML_COUNT);
		if (!a)
		{
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Missing slot count");
			return false;
		}

		// Slot count must be > 0
		int slotCount = a->getVal().toInt();
		if(slotCount <= 0)
		{
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Slot count must be greater than 0");
			return false;
		}

		m_slotCount = slotCount;

		// First Selectable Slot
		m_firstSelectable = eSlots->getAttr(NDHS_TP_XML_FIRST_SELECTABLE, "-1").toInt();

		// Make sure its within range
		if ((m_firstSelectable <= 0) || (m_firstSelectable > m_slotCount))
		{
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "firstSelectable must be between 1 and slot count");
			return false;
		}
		else
		{
			m_firstSelectable--;
		}

		// Last Selectable Slot
		m_lastSelectable = eSlots->getAttr(NDHS_TP_XML_LAST_SELECTABLE, "-1").toInt();

		// Make sure its within range
		// Need to add back one to the check against m_firstSelectable because
		// we have just subtracted 1
		if ((m_lastSelectable < m_firstSelectable + 1) || (m_lastSelectable > m_slotCount))
		{
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "lastSelectable must be between firstSelectable and slot count");
			return false;
		}
		else
		{
			m_lastSelectable--;
		}

		a = eSlots->findAttr(NDHS_TP_XML_FIRST_ACTIVE);
		if (!a)
		{
			// If not present, the inactive layout is first active
			m_firstActive = -1;
		}
		else
		{
			m_firstActive = a->getVal().toInt() - 1;

			if ((m_firstActive < m_firstSelectable) || (m_firstActive > m_lastSelectable))
			{
				NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "firstActive must be between firstSelectable and lastSelectable or missing for inactive state");
				return false;
			}
		}

		a = eSlots->findAttr(NDHS_TP_XML_POPULATE_FROM);
		if (!a)
		{
			// If not present, the first slot to populate is the first active slot, or the first selectable slot if there is no first active slot
			m_populateFrom = (m_firstActive != -1) ? m_firstActive : m_firstSelectable;
		}
		else
		{
			m_populateFrom = a->getVal().toInt() - 1;

			if ((m_populateFrom < 0) || (m_populateFrom > m_slotCount - 1))
			{
				NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "populateFrom must be a valid slot number (or missing)");
				return false;
			}
		}
	}
	else
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleTemplate, "Slot settings section missing");
		return false;
	}

	// Configure timing information
	configureTimingFromXML(eSettings->find(NDHS_TP_XML_TIMING));

	// Configure focus information
	configureFocusSettingsFromXML(eSettings->find(NDHS_TP_XML_FOCUS));

	// Configure variables now
	configureVariablesFromXML(eSettings->find(NDHS_TP_XML_VARIABLES));

	// Configure parameters now
	configureParameterFromXML(eSettings->find(NDHS_TP_XML_PARAMETERS));

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenuComponentTemplate::configureStaticDecorationFromXml(LcCXmlElem* decorationRoot)
{
	// Don't need a decorations section, so
	// just return true if there isn't one
	if (!decorationRoot)
		return true;

	LcTaOwner<CLayoutDecoration> initialState = CLayoutDecoration::create();

	if(configureStaticDecorationTypesFromXml(decorationRoot, initialState.ptr()))
	{
		m_layoutDecorationsMap.push_back(initialState);
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool NdhsCMenuComponentTemplate::configureStaticDecorationTypesFromXml(LcCXmlElem* eInitialState, CLayoutDecoration* initialState)
{
	typedef enum {
		ETypeStateChange = 0,
		ETypeItemSlot,
		ETypeStatic
	} EType;

	EType type;

	// Look for child state changes
	LcCXmlElem* eDecorationType = eInitialState;

	{
		LcTaString decorationType = eDecorationType->getName().toLower();

		LcTaOwner<CLayoutDecoration::CDecorationInfo> info = CLayoutDecoration::CDecorationInfo::create();
		CLayoutDecoration::CDecorationInfo* infoPtr = info.ptr();

		LcTaString toLayout = "";
		if(decorationType.compareNoCase(NDHS_TP_XML_TRANSITION) == 0)
		{
			type = ETypeStateChange;

			LcCXmlAttr* attr = eDecorationType->findAttr(NDHS_TP_XML_TO_LAYOUT);
			if (attr)
			{
				toLayout = attr->getVal();

				if(toLayout.length()<=0)
				{
					toLayout="";
				}
			}

			attr = eDecorationType->findAttr(NDHS_TP_XML_LAYOUT);
			if (!attr)
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration's transition has no from layout specified");

				// Not a valid element, so carry on to the next one
				info.destroy();
				return false;
			}

			LcTaString fromLayout = attr->getVal();

			if(fromLayout.length()<=0)
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration's layout name invalid");

				// Not a valid element, so carry on to the next one
				info.destroy();
				return false;
			}

			initialState->layout=toLayout;
			initialState->fromLayout=fromLayout;


			// timing

			LcCXmlElem* eTiming = eDecorationType->find(NDHS_TP_XML_TIMING);
			if (eTiming)
			{
				info->timingDataSet = true;

				// A '-1' value denotes that this field is not present.
				info->timingData.delay				= eTiming->getAttr(NDHS_TP_XML_DELAY, "0").toInt();
				info->timingData.velocityProfile	= strToVelocityProfile(eTiming->getAttr(NDHS_TP_XML_VELOCITY_PROFILE).toLower());

				if (info->timingData.delay < 0)
				{
					info->timingData.delay = 0;

					NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Timing delay for a terminal transition must not be negative");
				}

				attr = eTiming->findAttr(NDHS_TP_XML_DURATION);

				if (attr)
				{
					info->timingData.duration = attr->getVal().toInt();

					if (info->timingData.duration < 0)
					{
						// Set to -1 so that we use the default
						info->timingData.duration = -1;

						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Timing duration for a terminal transition must not be negative");
					}
				}
				else
				{
					// Set to -1 so that we use the default
					info->timingData.duration = -1;
				}

				attr = eTiming->findAttr(NDHS_TP_XML_BACKGROUND_DELAY);

				if (attr)
				{
					info->timingData.backgroundDelay = attr->getVal().toInt();

					if (info->timingData.backgroundDelay < 0)
					{
						// Set to -1 so that we use the default
						info->timingData.backgroundDelay = -1;

						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Background delay for a terminal transition must not be negative");
					}
				}
				else
				{
					// Set to -1 so that we use the default
					info->timingData.backgroundDelay = -1;
				}

				attr = eTiming->findAttr(NDHS_TP_XML_PRIMARY_LIGHT_DELAY);

				if (attr)
				{
					info->timingData.primaryLightDelay = attr->getVal().toInt();

					if (info->timingData.primaryLightDelay < 0)
					{
						// Set to -1 so that we use the default
						info->timingData.primaryLightDelay = -1;

						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Primary light delay for a terminal transition must not be negative");
					}
				}
				else
				{
					// Set to -1 so that we use the default
					info->timingData.primaryLightDelay = -1;
				}

				attr = eTiming->findAttr(NDHS_TP_XML_PRIMARY_LIGHT_DURATION);

				if (attr)
				{
					info->timingData.primaryLightDuration = attr->getVal().toInt();

					if (info->timingData.primaryLightDuration < 0)
					{
						// Set to -1 so that we use the default
						info->timingData.primaryLightDuration = -1;

						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Primary light duration for a terminal transition must not be negative");
					}
				}
				else
				{
					// Set to -1 so that we use the default
					info->timingData.primaryLightDuration = -1;
				}
			}

			initialState->stateDecorations.push_back(info);
		}

		else if (decorationType.compareNoCase(NDHS_TP_XML_SCROLL) == 0)
		{
			LcCXmlAttr* attr = eDecorationType->findAttr(NDHS_TP_XML_LAYOUT);
			if (!attr)
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration's has no layout specified");
				return false;
			}

			LcTaString strLayout = attr->getVal();

			initialState->layout=strLayout;

			type = ETypeItemSlot;

			if (!initialState->scrollDecoration)
			{
				initialState->scrollDecoration = info;
			}
			else
			{
				// Discard any duplicates
				info.destroy();
				return false;
			}
		}
		else if (decorationType.compareNoCase(NDHS_TP_XML_STATIC) == 0)
		{
			LcCXmlAttr* attr = eDecorationType->findAttr(NDHS_TP_XML_LAYOUT);
			if (!attr)
			{
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration's has no layout specified");
				return false;
			}

			LcTaString strLayout = attr->getVal();

			initialState->layout=strLayout;

			if (!initialState->staticDecoration)
			{
				type = ETypeStatic;
				initialState->staticDecoration = info;
			}
			else
			{
				// If a second static is found, just ignore it
				NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "You can only have one 'static' within an initial state in the decorations section");

				info.destroy();
				return false;
			}
		}
		else
		{
			// Not a valid element, so carry on to the next one
			info.destroy();
			return false;
		}

		LcCXmlElem * eComponent = eDecorationType->getFirstChild();

		// Look for the child components
		for (; eComponent; eComponent = eComponent->getNext())
		{
			LcTaString component = eComponent->getName().toLower();

			if (component.compareNoCase(NDHS_TP_XML_PAGE) == 0)
			{
				LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

				if (configureDecorationRefFromXml(eComponent, type == ETypeStatic, animRef.ptr()))
				{
					infoPtr->furnitureAnimations.add_element("page", animRef);
				}
			}
			else if (component.compareNoCase(NDHS_TP_XML_MENU) == 0)
			{
				LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

				if (configureDecorationRefFromXml(eComponent, type == ETypeStatic, animRef.ptr()))
				{
					infoPtr->furnitureAnimations.add_element("menu", animRef);
				}
			}
			else if (component.compareNoCase(NDHS_TP_XML_FURNITURE) == 0)
			{
				LcCXmlElem* eElement = eComponent->getFirstChild();

				for (; eElement; eElement = eElement->getNext())
				{
					LcTaString element = eElement->getName().toLower();

					if (element.compareNoCase(NDHS_TP_XML_ELEMENT) == 0)
					{
						LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

						if (configureDecorationRefFromXml(eElement, type == ETypeStatic, animRef.ptr()))
						{
							LcTaString elementClass = eElement->getAttr(NDHS_TP_XML_CLASS).toLower();

							if (elementClass.length() > 0)
							{
								infoPtr->furnitureAnimations.add_element(elementClass, animRef);
							}
							else
							{
								infoPtr->defaultFurnitureAnimation.destroy();
								infoPtr->defaultFurnitureAnimation = animRef;
							}
						}
					}
				}
			}
			else if(component.compareNoCase(NDHS_TP_XML_OUTER_GROUP) == 0)
			{
				LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

				if (configureDecorationRefFromXml(eComponent, type == ETypeStatic, animRef.ptr()))
				{

					infoPtr->outerGroupAnimations=animRef;
				}
			}

			else if (component.compareNoCase(NDHS_TP_XML_ITEMS) == 0)
			{
				// Look for child state changes

				LcCXmlElem* eItem = eComponent->getFirstChild();

				for (; eItem; eItem = eItem->getNext())
				{
					LcTaString item = eItem->getName().toLower();

					if (item.compareNoCase(NDHS_TP_XML_DEFAULTS) == 0)
					{
						// Look for child state changes
						LcCXmlElem* eElement = eItem->getFirstChild();

						for (; eElement; eElement = eElement->getNext())
						{
							LcTaString element = eElement->getName().toLower();

							if (element.compareNoCase(NDHS_TP_XML_SLOT) == 0)
							{
								LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

								if (configureDecorationRefFromXml(eElement, type == ETypeStatic, animRef.ptr()))
								{
									infoPtr->defaultItemAnimations->elementAnimations.add_element("slot", animRef);
								}
							}
							else if (element.compareNoCase(NDHS_TP_XML_ELEMENT) == 0)
							{
								LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

								if (configureDecorationRefFromXml(eElement, type == ETypeStatic, animRef.ptr()))
								{
									LcTaString elementClass = eElement->getAttr(NDHS_TP_XML_CLASS).toLower();

									if (elementClass.length() > 0)
									{
										infoPtr->defaultItemAnimations->elementAnimations.add_element(elementClass, animRef);
									}
									else
									{
										infoPtr->defaultItemAnimations->defaultElementAnimation.destroy();
										infoPtr->defaultItemAnimations->defaultElementAnimation = animRef;
									}
								}
							}
						}
					}
					else if (item.compareNoCase(NDHS_TP_XML_ITEM) == 0)
					{
						LcTaOwner<CLayoutDecoration::CDecorationInfo::CDecorationItem> decorationItem = CLayoutDecoration::CDecorationInfo::CDecorationItem::create();
						CLayoutDecoration::CDecorationInfo::CDecorationItem* decorationItemPtr = decorationItem.ptr();

						LcCXmlAttr* attr = NULL;
						if ((type != ETypeItemSlot) && ((attr = eItem->findAttr(NDHS_TP_XML_SLOT)) != NULL))
						{
							LcTaString strSlot = attr->getVal();

							if (strSlot.compareNoCase(NDHS_TP_XML_ACTIVE) == 0)
							{
								if (infoPtr->slotItemAnimations.find(CLayoutDecoration::EActive) == infoPtr->slotItemAnimations.end())
								{
									infoPtr->slotItemAnimations.add_element(CLayoutDecoration::EActive, decorationItem);
								}
								else
								{
									decorationItem.destroy();
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's slot value has been specified twice");
									continue;
								}
							}
							else if (strSlot.compareNoCase(NDHS_TP_XML_TO_ACTIVE) == 0)
							{
								if (infoPtr->slotItemAnimations.find(CLayoutDecoration::EToActive) == infoPtr->slotItemAnimations.end())
								{
									infoPtr->slotItemAnimations.add_element(CLayoutDecoration::EToActive, decorationItem);
								}
								else
								{
									decorationItem.destroy();
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's slot value has been specified twice");
									continue;
								}
							}
							else if (strSlot.compareNoCase(NDHS_TP_XML_FROM_ACTIVE) == 0)
							{
								if (infoPtr->slotItemAnimations.find(CLayoutDecoration::EFromActive) == infoPtr->slotItemAnimations.end())
								{
									infoPtr->slotItemAnimations.add_element(CLayoutDecoration::EFromActive, decorationItem);
								}
								else
								{
									decorationItem.destroy();
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's slot value has been specified twice");
									continue;
								}
							}
							else
							{
								int slot = strSlot.toInt() - 1;

								if (slot >= 0  && slot < m_slotCount)
								{
									if (infoPtr->slotItemAnimations.find(slot) == infoPtr->slotItemAnimations.end())
									{
										infoPtr->slotItemAnimations.add_element(slot, decorationItem);
									}
									else
									{
										decorationItem.destroy();
										NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's slot value has been specified twice");
										continue;
									}
								}
								else
								{
									decorationItem.destroy();
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's slot value is invalid");
									continue;
								}
							}
						}
						else if (type != ETypeItemSlot)
						{
							decorationItem.destroy();
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item has no slot specified");
							continue;
						}
						else
						{
							LcTaString strToSlot = eItem->getAttr(NDHS_TP_XML_TO_SLOT, "any");
							LcTaString strFromSlot = eItem->getAttr(NDHS_TP_XML_FROM_SLOT, "any");

							LcTmOwnerMap<int, CLayoutDecoration::CDecorationInfo::CFromSlot>::iterator itFromSlot;

							CLayoutDecoration::CDecorationInfo::CFromSlot* fromSlotInfo;

							if (strToSlot.compareNoCase(NDHS_TP_XML_ACTIVE) == 0)
							{
								if ((itFromSlot = infoPtr->toSlotItemAnimations.find(CLayoutDecoration::EActive)) == infoPtr->toSlotItemAnimations.end())
								{
									LcTaOwner<CLayoutDecoration::CDecorationInfo::CFromSlot> newFromSlot = CLayoutDecoration::CDecorationInfo::CFromSlot::create();
									fromSlotInfo = newFromSlot.ptr();
									infoPtr->toSlotItemAnimations.add_element(CLayoutDecoration::EActive, newFromSlot);
								}
								else
								{
									fromSlotInfo = itFromSlot->second;
								}
							}
							else if (strToSlot.compareNoCase(NDHS_TP_XML_ANY) == 0)
							{
								if ((itFromSlot = infoPtr->toSlotItemAnimations.find(CLayoutDecoration::ESlotAny)) == infoPtr->toSlotItemAnimations.end())
								{
									LcTaOwner<CLayoutDecoration::CDecorationInfo::CFromSlot> newFromSlot = CLayoutDecoration::CDecorationInfo::CFromSlot::create();
									fromSlotInfo = newFromSlot.ptr();
									infoPtr->toSlotItemAnimations.add_element(CLayoutDecoration::ESlotAny, newFromSlot);
								}
								else
								{
									fromSlotInfo = itFromSlot->second;
								}
							}
							else
							{
								int slot = strToSlot.toInt() - 1;

								if (slot >= 0  && slot < m_slotCount)
								{
									if ((itFromSlot = infoPtr->toSlotItemAnimations.find(slot)) == infoPtr->toSlotItemAnimations.end())
									{
										LcTaOwner<CLayoutDecoration::CDecorationInfo::CFromSlot> newFromSlot = CLayoutDecoration::CDecorationInfo::CFromSlot::create();
										fromSlotInfo = newFromSlot.ptr();
										infoPtr->toSlotItemAnimations.add_element(slot, newFromSlot);
									}
									else
									{
										fromSlotInfo = itFromSlot->second;
									}
								}
								else
								{
									decorationItem.destroy();
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's toSlot value is invalid");
									continue;
								}
							}

							if (strFromSlot.compareNoCase(NDHS_TP_XML_ACTIVE) == 0)
							{
								if (fromSlotInfo->fromSlotItemAnimations.find(CLayoutDecoration::EActive) == fromSlotInfo->fromSlotItemAnimations.end())
								{
									fromSlotInfo->fromSlotItemAnimations.add_element(CLayoutDecoration::EActive, decorationItem);
								}
								else
								{
									decorationItem.destroy();
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's toSlot/fromSlot pair has been specified twice");
									continue;
								}
							}
							else if (strFromSlot.compareNoCase(NDHS_TP_XML_ANY) == 0)
							{
								if (fromSlotInfo->fromSlotItemAnimations.find(CLayoutDecoration::ESlotAny) == fromSlotInfo->fromSlotItemAnimations.end())
								{
									fromSlotInfo->fromSlotItemAnimations.add_element(CLayoutDecoration::ESlotAny, decorationItem);
								}
								else
								{
									decorationItem.destroy();
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's toSlot/fromSlot pair has been specified twice");
									continue;
								}
							}
							else if (strFromSlot.subString(0, 1).compare("+") == 0
								|| strFromSlot.subString(0, 1).compare("-") == 0)
							{
								int offset = strFromSlot.toInt();

								if (fromSlotInfo->offsetSlotItemAnimations.find(offset) == fromSlotInfo->offsetSlotItemAnimations.end())
								{
									fromSlotInfo->offsetSlotItemAnimations.add_element(offset, decorationItem);
								}
								else
								{
									decorationItem.destroy();
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's toSlot/fromSlot pair has been specified twice");
									continue;
								}
							}
							else
							{
								int slot = strFromSlot.toInt() - 1;

								if (slot >= 0  && slot < m_slotCount)
								{
									if (fromSlotInfo->fromSlotItemAnimations.find(slot) == fromSlotInfo->fromSlotItemAnimations.end())
									{
										fromSlotInfo->fromSlotItemAnimations.add_element(slot, decorationItem);
									}
									else
									{
										decorationItem.destroy();
										NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's toSlot/fromSlot pair has been specified twice");
										continue;
									}
								}
								else
								{
									decorationItem.destroy();
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "Decoration item's fromSlot value is invalid");
									continue;
								}
							}
						}

						// Look for child state changes
						LcCXmlElem* eElement = eItem->getFirstChild();

						for (; eElement; eElement = eElement->getNext())
						{
							LcTaString element = eElement->getName().toLower();

							if (element.compareNoCase(NDHS_TP_XML_SLOT) == 0)
							{
								LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

								if (configureDecorationRefFromXml(eElement, type == ETypeStatic, animRef.ptr()))
								{
									decorationItemPtr->elementAnimations.add_element("slot", animRef);
								}
							}
							else if (element.compareNoCase(NDHS_TP_XML_ELEMENT) == 0)
							{
								LcTaOwner<CLayoutDecoration::CDecorationInfo::CAnimationRef> animRef = CLayoutDecoration::CDecorationInfo::CAnimationRef::create();

								if (configureDecorationRefFromXml(eElement, type == ETypeStatic, animRef.ptr()))
								{
									LcTaString elementClass = eElement->getAttr(NDHS_TP_XML_CLASS).toLower();

									if (elementClass.length() > 0)
									{
										decorationItemPtr->elementAnimations.add_element(elementClass, animRef);
									}
									else
									{
										decorationItemPtr->defaultElementAnimation.destroy();
										decorationItemPtr->defaultElementAnimation = animRef;
									}
								}
							}
						}
					}
				}
			}
			else if (component.compareNoCase(NDHS_TP_XML_TRIGGERS) == 0)
			{
				infoPtr->triggersSet = true;

				if (type == ETypeStatic)
				{
					// Static trigger decorations have additional attributes
					LcTaString duration = eComponent->getAttr(NDHS_TP_XML_DURATION);

					if(!duration.isEmpty())
					{
						int d = duration.toInt();
						if (d > 0)
						{
							infoPtr->triggers->transitionTime = d;
						}
						else
						{
							// duration is mandatory, so ignore the triggers altogether
							infoPtr->triggersSet = false;
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'duration' attribute of 'trigger' is not valid");
							continue;
						}
					}
					else
					{
						// duration is mandatory, so ignore the triggers altogether
						infoPtr->triggersSet = false;
						NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'trigger' for a static animation is missing the mandatory attribute 'duration'");
						continue;
					}

					LcTaString loop = eComponent->getAttr(NDHS_TP_XML_LOOP, "1");

					if(loop.compareNoCase("infinite") == 0)
					{
						infoPtr->triggers->loopCount = -1;
					}
					else
					{
						int l = loop.toInt();
						if (l > 0)
						{
							infoPtr->triggers->loopCount = l;
						}
						else
						{
							// Default to one loop in error case
							infoPtr->triggers->loopCount = 1;

							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'triggers' loop count must not be negative");
						}
					}
				}

				// Look for cleanup trigger
				LcTaString cleanupTrigger = eComponent->getAttr(NDHS_TP_XML_CLEANUP_TRIGGER).toLower();

				if (cleanupTrigger.length() > 0)
				{
					LcTmArray<int>* keyCodes = m_pageManager->getKeyCodes(cleanupTrigger);

					if (keyCodes != NULL)
					{
						// There may be more that one scan code associated
						// with the same key name but they'll all resolve to
						// the same action, so we just store the first in the
						// array
						if (keyCodes->size() > 0)
							infoPtr->triggers->cleanupTrigger = (*keyCodes)[0];
					}
					else
					{
						bool catchAll;

						(void) convertKeyToInt(cleanupTrigger, infoPtr->triggers->cleanupTrigger, catchAll);
					}

					infoPtr->triggers->hasCleanupTrigger = true;
				}
				else
				{
					infoPtr->triggers->hasCleanupTrigger = false;
				}

				// Look for triggers
				LcCXmlElem* eTrigger = eComponent->getFirstChild();

				for (; eTrigger; eTrigger = eTrigger->getNext())
				{
					if (eTrigger->getName().compareNoCase(NDHS_TP_XML_TRIGGER) == 0)
					{
						LcTaString key = eTrigger->getAttr(NDHS_TP_XML_KEY).toLower();
						int convertedKey = 0;
						LcTScalar position = eTrigger->getAttr(NDHS_TP_XML_POSITION, "-1").toScalar();

						if (key.length() > 0)
						{
							LcTmArray<int>* keyCodes = m_pageManager->getKeyCodes(key);

							if (keyCodes != NULL)
							{
								// There may be more that one scan code associated
								// with the same key name but they'll all resolve to
								// the same action, so we just store the first in the
								// array
								if (keyCodes->size() > 0)
									convertedKey = (*keyCodes)[0];
							}
							else
							{
								bool catchAll;

								if ((convertKeyToInt(key, convertedKey, catchAll) == false) || (catchAll == true))
								{
									NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "decoration trigger has an invalid key specified");
								}
							}
						}
						else
						{
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "decoration trigger is missing the mandatory attribute 'key'");
						}

						if (position > -1)
						{
							if ((position > 100) || (position < 0))
							{
								NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "'position' attribute of 'trigger' cannot be less than 0 or greater than 100");
								continue;
							}

							infoPtr->triggers->addTrigger(convertedKey, position / 100.0f);
						}
						else
						{
							NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, "decoration trigger is missing the mandatory attribute 'position'");
						}
					}
				}
				// Also sort the trigger list
				if (infoPtr->triggers)
					IFX_ShellSort(infoPtr->triggers->triggerList.begin(), infoPtr->triggers->triggerList.end(), triggerCompare);
			}
		}
	}
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMenuComponentTemplate::isOuterGroup(const LcTaString& elementName)
{
	return elementName.compareNoCase(NDHS_TP_XML_OUTER_GROUP) == 0;
}
