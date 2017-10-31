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
#ifndef LcTMessageH
#define LcTMessageH

#include "inflexionui/engine/inc/LcITimer.h"

/*-------------------------------------------------------------------------*//**
	Message handling (a specialized case of event scheduling which includes
	an event ID and parameter)
*/
class LcTMessage : public LcITimer::IEvent
{
public:

	// Message handler must implement this
	class IHandler
	{
	public:

		virtual void onMessage(int iID, int iParam) = 0;
	};

public:

	// Message details
	LcITimer*						m_pTimer;
	IHandler*						m_pHandler;
	int								m_iID;
	int								m_iParam;

private:

	// LcITimer::IEvent methods
	LC_VIRTUAL		void			onExecute();

public:

	// create constructor
	LC_IMPORT						LcTMessage(IHandler* pHdlr, int iID);

	// Scheduling API - can only schedule each object once at a time
	LC_IMPORT		void			schedule(LcITimer* pTimer, int iParam, int iTime);
	LC_IMPORT		void			scheduleImmediate(LcITimer* pTimer, int iParam);

#if defined(IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
	LC_IMPORT		void			scheduleScriptTimer(LcITimer* pTimer, int iParam, int iTime);
	LC_IMPORT		void			cancelScript();
#endif

	inline			int				isScheduled()	{ return m_pTimer != 0; }
	inline			int				getParam()		{ return m_iParam; }
	LC_IMPORT		void			cancel();
};

#endif // LcTMessageH

