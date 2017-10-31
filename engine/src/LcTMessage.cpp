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
LC_EXPORT LcTMessage::LcTMessage(IHandler* pHdlr, int iID)
{
	m_pTimer		= NULL;
	m_pHandler		= pHdlr;
	m_iID	 		= iID;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcTMessage::onExecute()
{
	// Do first so that handler can re-schedule
	m_pTimer = NULL;

	// Handle message
	m_pHandler->onMessage(m_iID, m_iParam);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcTMessage::schedule(LcITimer* pTimer, int iParam, int iTime)
{
	// If timer already set it means we're already scheduled
	if (m_pTimer)
		cancel();

	// Schedule with new time/param
	m_pTimer = pTimer;
	m_iParam = iParam;
	m_pTimer->schedule(this, iTime);
}

/*-------------------------------------------------------------------------*//**
	Special form of schedule - this will insert this message at the head of the
	queue, guaranteeing that it will execute at the earliest opportunity (unless
	another message calls scheduleImmediate, of course!)
*/
LC_EXPORT void LcTMessage::scheduleImmediate(LcITimer* pTimer, int iParam)
{
	// If timer already set it means we're already scheduled
	if (m_pTimer)
		cancel();

	// Schedule with new time/param
	m_pTimer = pTimer;
	m_iParam = iParam;
	m_pTimer->scheduleImmediate(this);
}

#if defined(IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
	Schedule script message. 
*/
LC_EXPORT void LcTMessage::scheduleScriptTimer(LcITimer* pTimer, int iParam, int iTime)
{
	// Schedule time
	m_pTimer = pTimer;
	m_iParam = iParam;
	m_pTimer->scheduleScriptTimer(this, iTime);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcTMessage::cancelScript()
{
	// Unless timer is set we aren't scheduled so can't cancel
	if (m_pTimer)
	{
		m_pTimer->cancelScript(this);
		m_pTimer = NULL;
	}
}
#endif

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcTMessage::cancel()
{
	// Unless timer is set we aren't scheduled so can't cancel
	if (m_pTimer)
	{
		m_pTimer->cancel(this);
		m_pTimer = NULL;
	}
}
