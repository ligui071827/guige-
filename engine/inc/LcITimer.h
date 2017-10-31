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
#ifndef LcITimerH
#define LcITimerH

/*-------------------------------------------------------------------------*//**
*/
class LcITimer
{
public:

	class IEvent
	{
	public:
	
		virtual void onExecute()						= 0;
	};

	// Methods to be implemented
	virtual		bool	start()							= 0;
	virtual		bool	stop()							= 0;
	virtual		bool	schedule(IEvent* e, int iTime)	= 0;
	virtual		bool	scheduleImmediate(IEvent* e)	= 0;
	virtual		void	cancel(IEvent* e)				= 0;

#if defined(IFX_USE_SCRIPTS) || defined (IFX_GENERATE_SCRIPTS)
	virtual		bool	scheduleScriptTimer(IEvent* e, int iTime)	= 0;
	virtual		void	cancelScript(IEvent* e)			= 0;
#endif

};

#endif // LcITimerH


