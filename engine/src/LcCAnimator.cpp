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
LC_EXPORT LcCAnimator::LcCAnimator()			
{ 
	m_space		= 0; 
	m_observer	= 0; 
	m_animState	= EAnimStopped; 
}

/*-------------------------------------------------------------------------*//**
	Derived animator should call to start animation
*/
LC_EXPORT void LcCAnimator::startAnimation()
{
	// Get the space we will animate using
	m_space = getSpace();

	// Validate config
	if (m_animState != EAnimStopped || !m_space)
		return;

	// Attach to space to trigger updates per frame
	m_space->addAnimator(this);
	
	// OK to proceed
	m_animState = EAnimFirstFrame;

	// Send event to observer
	if (m_observer)
		m_observer->onAnimationEvent(this, LC_WE_START_ANIMATE);
}

/*-------------------------------------------------------------------------*//**
	Derived animator should call to abort animation
*/
LC_EXPORT void LcCAnimator::abortAnimation()
{
	if (m_animState == EAnimStopped)
		return;

	// No more frame updates - note that retiring a widget before 
	// aborting animation is a programming error which will cause a crash
	m_space->removeAnimator(this);

	// OK to abort
	m_animState = EAnimStopped;

	// Change in animation state may affect paint hints
	onWantRevalidate();

	// Send event to observer
	// NB: must do last, as event may delete or modify animator
	if (m_observer)
		m_observer->onAnimationEvent(this, LC_WE_ABORT_ANIMATE);
}

/*-------------------------------------------------------------------------*//**
	Space will call before each frame, to continue animation.  Derived animator 
	should return true from onAdvanceToTime() to indicate completion of the
	animation
*/
LC_EXPORT_VIRTUAL bool LcCAnimator::advanceToTime(LcTTime timestamp)
{
	// If state is ELastFrame, it means the previous animation update 
	// was the last, and we were just waiting for that frame to get painted.
	// We now know it was painted, so we can send the STOP event now
	if (m_animState == EAnimLastFrame)
	{
		// ...we really are stopping now
		m_animState = EAnimStopped;

		// Change in animation state may affect paint hints
		onWantRevalidate();

		// Send event to observer
		// NB: must do last, as event may delete or modify animator
		if (m_observer)
			m_observer->onAnimationEvent(this, LC_WE_STOP_ANIMATE);

		// Tell space to not bother us with another call
		return true;
	}

	// Do the next animation update, and if it's the last frame, set the
	// state to ELastFrame to allow the frame to be painted
	if (onAdvanceToTime(timestamp))
		m_animState = EAnimLastFrame;

	// If we were doing first frame, we're not any more
	if (m_animState == EAnimFirstFrame)
		m_animState = EAnimMidFrames;

	// Will get another update later
	return false;
}

