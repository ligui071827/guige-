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
#ifndef NdhsCTransitionAgentH
#define NdhsCTransitionAgentH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTString.h"
#include "inflexionui/engine/inc/LcTPlacement.h"
#include "inflexionui/engine/inc/NdhsCTemplate.h"

class NdhsCElement;
class LcCWidget;

/*-------------------------------------------------------------------------*//**
	NdhsCTransitionAgent is a utility class instantiated by a Page Template for a
	Page Model.  Once created, it can be used to configure and start transitions
	for all the elements in the Page Model in response to a state change or slot
	reassignment.
*/
class NdhsCTransitionAgent : public LcCBase
{
private:

	NdhsCTemplate*				m_template;
	NdhsCElementGroup*			m_model;
	LcTmOwner<NdhsCTemplate::CLayout> 	m_layout;
	LcTmOwner<NdhsCTemplate::CLayout> 	m_oldLayout;
	int							m_initialSlotActive;
	int							m_finalSlotActive;
	ENdhsVelocityProfile		m_ovveridenVelocityProfile;
	int							m_ovveridenDuration;
	int							m_ovveridenDelay;

	// Two-phase construction
								NdhsCTransitionAgent(NdhsCTemplate* pTemplate, NdhsCElementGroup*	pModel);

	void						construct();

	void 						applyDisplacements(
									const LcTmMap<LcTmString, LcTmString>* pDisplacements,
									NdhsCElement* pElement,
									LcTPlacement& placement);

public:
	static LcTaOwner<NdhsCTransitionAgent> create(NdhsCTemplate* pTemplate, NdhsCElementGroup*	pModel);
	virtual						~NdhsCTransitionAgent();

	NdhsCTemplate::CLayout*	getStartLayout()	{ return m_oldLayout.ptr(); }
	NdhsCTemplate::CLayout*	getEndLayout()		{ return m_layout.ptr(); }

	//transition API
	void						prepareTransition(NdhsCTemplate::CLayout* newLayout,
													int initialSlotActive,
													int finalSlotActive);
	void						ovverideTransitionDetails(ENdhsVelocityProfile velocityProfile,
													int duration,
													int	delay);
	void						resetOvveridenTransitionDetails();

	// Dynamic Animations
	void						getPageAnimationDetails(NdhsCElementGroup* pageToPosition,
									ENdhsAnimationType animType,
									int& duration,
									int& delay,
									ENdhsVelocityProfile& profile,
									int& backgroundDelay,
									int& primaryLightDelay,
									int& primaryLightDuration);
	void						updateAnimationCache(
									NdhsCElementGroup* groupToPosition,
									ENdhsAnimationType animType,
									bool positionIncreasing);
	void						updateAnimationCache(
									NdhsCElement* elementToPosition,
									bool isDetail,
									ENdhsAnimationType animType,
									bool positionIncreasing);

	// Static Animations
	void						getStaticTriggers(NdhsCElementGroup* group);
	void						getStaticAnimation(NdhsCElementGroup* group);
	void						getStaticAnimation(NdhsCElement* element, bool isDetail);
};
#endif // NdhsCTransitionAgentH

// End of File
