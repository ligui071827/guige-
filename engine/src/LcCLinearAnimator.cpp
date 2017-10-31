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
LC_EXPORT LcCLinearAnimator::LcCLinearAnimator()
{
	m_iMilli		= 1000;
//	m_iDelay		= 0;
//	m_maxStep		= 0;
//	m_dCurPos		= 0;
//	m_dHaltPos		= 0;
//	m_dStartPos		= 0;
//	m_iStartTime	= 0;
//	m_bAnimating	= false;
//	m_bReversing	= false;
	m_loopCount		= 1;
	m_currentLoopCount = m_loopCount;
//	m_bIsTweener	= false;
//	m_bJustStarted	= false;
}

/*-------------------------------------------------------------------------*//**
*/
LcCLinearAnimator::~LcCLinearAnimator()
{ 
	abort();
}

/*-------------------------------------------------------------------------*//**
	Set the timing parameters for the animation.

	@param iMilli The amount of time that should elapse when animating the item
				 through the full range from 0 to 1.  If animating through a
				 shorter range, the animation will take place over a proportionately
				 shorter time.
				 This value is directly related to the animation interval on the LcCSpace.
	@param iDelay The amount of time in discreet units (usually milliseconds) that
				 the animator wait between the moment that it is asked to start
				 animating, and the moment that it does start animating.
*/
LC_EXPORT void LcCLinearAnimator::setTiming(int iMilli, int iDelay)
{
	m_iMilli		= iMilli;
	m_iDelay		= iDelay;
}

/*-------------------------------------------------------------------------*//**
	This stops the animation, and sends out a LcCLinearAnimator::CEvent with the code
	LC_WE_ABORT_ANIMATE and the originator set to 'this'.
*/
LC_EXPORT_VIRTUAL void LcCLinearAnimator::abort()
{
	m_bJustStarted = false;
	abortAnimation();
}

/*-------------------------------------------------------------------------*//**
	Jump to position A in the animation.

	This stops the animation and resets the location, scale, and opacity to
	the values specified at A.
*/
LC_EXPORT_VIRTUAL void LcCLinearAnimator::jumpToA()
{
	// Must abort any animation in progress
	abort();

	// New position
	m_dCurPos = 0;
	jumpPos(m_dCurPos, LcCAnimator::EAnimStopped);
}

/*-------------------------------------------------------------------------*//**
	Jump to position B in the animation.

	This stops the animation and resets the location, scale, and opacity to
	the values specified at B.
*/
LC_EXPORT_VIRTUAL void LcCLinearAnimator::jumpToB()
{
	// Must abort any animation in progress
	abort();

	// New position
	m_dCurPos = 1;
	jumpPos(m_dCurPos, LcCAnimator::EAnimStopped);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCLinearAnimator::jumpTo(LcTUnitScalar f)
{
	// Must abort any animation in progress
	abort();

	// New position
	m_dCurPos = f;
	jumpPos(m_dCurPos, LcCAnimator::EAnimStopped);
}

/*-------------------------------------------------------------------------*//**
	Now takes an additional parameter, dPos, which allows the animation
	to be stopped at an arbitrary point on the interval from 
	A (0.0) to B (1.0).
*/
LC_EXPORT void LcCLinearAnimator::animateTo(LcTUnitScalar dPos, int dDirection)
{
	// Set stop position
	m_dStartPos		= m_dCurPos;
	m_dHaltPos		= dPos;
	m_bReversing	= (dDirection < 0);

	// May just be changing direction
	if (isAnimating())
	{
		// Start time used to work out distance moved, so reset this to
		// the time at which we changed direction
		m_iStartTime = 0;
	}
	else
	{
		m_bJustStarted = true;
		startAnimation();
	}
}

/*-------------------------------------------------------------------------*//**
	Animate from the current position in the direction of B.

	If the animation was already running in the direction of A, then the
	animation will be reversed.
*/
LC_EXPORT void LcCLinearAnimator::animateToB()
{
	// Animate in A to B direction
	animateTo(1, 1);
}

/*-------------------------------------------------------------------------*//**
	Animate from the current position in the direction of A.

	If the animation was already running in the direction of B, then the
	animation will be reversed.
*/
LC_EXPORT void LcCLinearAnimator::animateToA()
{
	// Animate in B to A direction
	animateTo(0, -1);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCLinearAnimator::setLoop(int count)
{	
	m_loopCount = count;

	// Catch if loop count an invalid number
	if ( (count < 1) && (count != EInfiniteLoop) )
	{
		m_loopCount = 1;
	}

	m_currentLoopCount = m_loopCount;	
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCLinearAnimator::reverse()
{ 
	m_bReversing = !m_bReversing;
	if (m_bReversing)
		animateToA();
	else
		animateToB();
}

/*-------------------------------------------------------------------------*//**
	Called by space just before each frame, to align animated widgets 
	according to given timestamp
*/
LC_EXPORT_VIRTUAL bool LcCLinearAnimator::onAdvanceToTime(LcTTime timestamp)
{
	// Start position is set on first animation event applied to object, since
	// an object first positioned with jumpTo() may not have been painted yet
	if (m_bJustStarted)
	{
		// If we've just started, apply configured delay by starting in the future
		m_iStartTime = timestamp + m_iDelay;
		m_bJustStarted = false;
		m_bDelayElapsed = false;
	}
	else if (m_iStartTime == 0)
	{
		// We may set the start time back to 0 if a change of direction occurs
		m_iStartTime = timestamp;
	}

	// We may not have reached start time yet, if there was a delay
	if (timestamp < m_iStartTime)
	{
		return false;
	}
	else
	{
		if (m_bDelayElapsed == false)
		{
			m_bDelayElapsed = true;

			if (getObserver())
				getObserver()->onAnimationEvent(this, LC_WE_DELAY_COMPLETE);
		}
	}

	// Work out step based on actual current time
	LcTUnitScalar dTravelled	= LcTUnitScalar(LcTScalar((int)(timestamp - m_iStartTime)) / m_iMilli);
	LcTUnitScalar dPrePos		= m_dCurPos;

	// Apply step depending on direction
	if (m_bReversing)
	{
		// Advance backwards
		m_dCurPos = m_dStartPos - dTravelled;

		// Ensure maximum step has not been exceeded
		if (m_maxStep > 0)
			m_dCurPos = max(m_dCurPos, dPrePos - m_maxStep);
	}
	else
	{
		// Advance forwards
		m_dCurPos = m_dStartPos + dTravelled;

		// Ensure maximum step has not been exceeded
		if (m_maxStep > 0)
			m_dCurPos = min(m_dCurPos, dPrePos + m_maxStep);
	}

	// If we reach a limit we have finished
	bool bFinished = true;

	// Note that now we allow animation to 'wrap', we must also
	// check the boundary conditions at 0.0 (A) and 1.0 (B)
	if ((m_currentLoopCount == 1) && 
		((m_bReversing && m_dCurPos <= m_dHaltPos &&  dPrePos >= m_dHaltPos)
	||  (!m_bReversing && m_dCurPos >= m_dHaltPos && dPrePos <= m_dHaltPos)
	||   (m_bReversing && m_dCurPos <= 0 && m_dCurPos + 1 <= m_dHaltPos)
	||  (!m_bReversing && m_dCurPos >= 1 && m_dCurPos - 1 >= m_dHaltPos)))
	{
		m_dCurPos = m_dHaltPos;
	}
	else
	{
		if (m_dCurPos < 0)
		{
			m_dCurPos	+= 1;
			m_dStartPos += 1;

			// Decrease loop count
			if (m_currentLoopCount > 0)
			{
				m_currentLoopCount--;
				onNewLoop();
			}
			else if (m_currentLoopCount == -1)
			{
				// -1 is the signal for an infinite loop
				onNewLoop();
			}
		}
		if (m_dCurPos > 1)
		{
			m_dCurPos	-= 1;
			m_dStartPos -= 1;

			// Decrease loop count
			if (m_currentLoopCount > 0)
			{
				m_currentLoopCount--;
				onNewLoop();
			}
			else if (m_currentLoopCount == -1)
			{
				// -1 is the signal for an infinite loop
				onNewLoop();
			}
		}
		
		// We haven't reached a limit
		bFinished = false;
	}

	// Determine our current animation position to determine our state.
	LcCAnimator::EAnimState animState = LcCAnimator::EAnimMidFrames;
	if (0 == m_dCurPos)
		animState = LcCAnimator::EAnimFirstFrame;
	else if (1 == m_dCurPos)
		animState = LcCAnimator::EAnimLastFrame;

	// Apply to widget
	jumpPos(m_dCurPos, animState);

	// Returns true to tell space the animation has finished
	return bFinished;
}
