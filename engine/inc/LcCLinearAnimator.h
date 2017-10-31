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
#ifndef LcCLinearAnimatorH
#define LcCLinearAnimatorH

#include "inflexionui/engine/inc/LcCSpace.h"
#include "inflexionui/engine/inc/LcTWidgetEvent.h"

/*-------------------------------------------------------------------------*//**
	A linear animator is an animator that changes some property of a system
	along a linear range.  The range is defined as 0 to 1 inclusive.
	This is an abstract class - the derived class must define jumpPos(d)
	such that it updates the system state to point d in the range 0 to 1.
*/
class LcCLinearAnimator : public LcCAnimator
{
public:

	enum
	{
		EInfiniteLoop = -1
	};

private:
	
	// Configuration
	int							m_iMilli;
	int							m_iDelay;
	LcTUnitScalar				m_maxStep;

	// State
	LcTUnitScalar				m_dCurPos;
	LcTUnitScalar				m_dHaltPos;
	LcTUnitScalar				m_dStartPos;
	LcTTime						m_iStartTime;
	bool						m_bReversing;
	int							m_loopCount;
	int							m_currentLoopCount;
	bool						m_bJustStarted;
	bool						m_bDelayElapsed;

	// LcCAnimator methods
	LC_VIRTUAL		bool		onAdvanceToTime(LcTTime timestamp);

protected:

	// Interface to be implemented by derived animator class
	virtual			void		jumpPos(LcTUnitScalar d, LcCAnimator::EAnimState animationState) = 0;

	virtual			void		onNewLoop(){}

	// Abstract so constructor protected
	LC_IMPORT					LcCLinearAnimator();

public:

	// Construction/destruction
	virtual						~LcCLinearAnimator();

	// Configuration
	LC_IMPORT		void		setTiming(int iMilli, int iDelay);
	inline			void		setTiming(int iMilli)		{ setTiming(iMilli, 0); }
	LC_IMPORT		void		setLoop(int count);
	inline			void		setMaxStep(LcTUnitScalar m)	{ m_maxStep = m; }
	
	// LcCAnimator interface - forward animation
	virtual			bool		canSync()					{ return m_maxStep <= 0; }
	LC_VIRTUAL		void		jumpToA();
	virtual			void		animateFromAToB()			{ jumpToA(); animateToB(); }
	LC_VIRTUAL		void		abort();

	// LcCAnimator interface - optional reverse animation
	virtual			bool		canReverse()				{ return true; }
	LC_VIRTUAL		void		jumpToB();	
	virtual			void		animateFromBToA()			{ jumpToB(); animateToA(); }
	LC_VIRTUAL		void		reverse();

	// Path-specific animation - can start/stop at specified position
	LC_IMPORT		void		jumpTo(LcTUnitScalar f);
	LC_IMPORT		void		animateTo(LcTUnitScalar dPos, int dDirection);
	LC_IMPORT		void		animateToA();
	LC_IMPORT		void		animateToB();

	// Query
	inline			float		getPos()					{ return m_dCurPos; }
	inline			bool		isAtA()						{ return m_dCurPos == 0; }
	inline			bool		isAtB()						{ return m_dCurPos == 1; }
	inline			bool		isReversing()				{ return m_bReversing; }
};

#endif // LcCLinearAnimatorH
