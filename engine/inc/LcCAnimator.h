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
#ifndef LcCAnimatorH
#define LcCAnimatorH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTWidgetEvent.h"

/*-------------------------------------------------------------------------*//**
	An LcCCAnimator is an abstract class that can be attached to an 
	LcCSpace to perform an animation function before each frame.
	This enables events to be sent to widgets when an animation on that 
	widget starts, stops or aborts, and also allows the animator to be 
	combined into a compound animation.
*/
class LcCAnimator : public LcCBase
{
public:

	// This is the position an animation 
	enum EAnimState
	{
		EAnimStopped,
		EAnimFirstFrame,
		EAnimMidFrames,
		EAnimLastFrame
	};

	// Interface that receives notification of animation events
	class IObserver
	{
	public:

		virtual		void			onAnimationEvent(LcCAnimator* a, int code) = 0;
	};

	// Event class used by LcCSpace to translate event notifications
	// through the above interface, into widget events.  TEvent objects
	// are not forwarded by the LcCAnimator itself
	class TEvent : public LcTWidgetEvent
	{
	private:

		LcCAnimator*				m_originator;
	
	public:
	
		// Just for convenience - widget set by fire method
									TEvent(int code, LcCAnimator* originator)
										: LcTWidgetEvent(code)  
										{ m_originator = originator; }
	
		// Access
		inline		LcCAnimator*	getOriginator()				{ return m_originator; }
	};

private:

	// Connected objects
	IObserver*						m_observer;
	LcCSpace*						m_space;

	// Animation state
	EAnimState						m_animState;

LC_PRIVATE_INTERNAL_PUBLIC:

	// Animation frames triggered by LcCSpace
	LC_VIRTUAL		bool			advanceToTime(LcTTime timestamp);

LC_PROTECTED_INTERNAL_PUBLIC:

	// Derived animator class must implement these
	virtual			int				getMask()								= 0;

	// Mask values.
	// Note: low-word bit values are defined in LcTPlacement
	// and must not be used here.
	enum
	{
		EMaskImageContent	= 0x10000,
		EMaskDisallowSketch	= 0x20000,
		EMaskAnimating		= 0x40000,
		EMaskForceSketch	= 0x80000
	};

protected:

	// Derived animator classes should call these from their 
	// animateToB() and abort() implementations, etc
	LC_IMPORT		void			startAnimation();
	LC_IMPORT		void			abortAnimation();

	// Derived animator class must implement these
	virtual			LcCSpace*		getSpace()								= 0;
	virtual			bool			onAdvanceToTime(LcTTime timestamp)		= 0;
	virtual			void			onWantRevalidate()						{}

	// Abstract so constructor protected
	LC_IMPORT						LcCAnimator();

public:

	// Configuration
	inline			void			setObserver(IObserver* o)	{ m_observer = o; }
	inline			IObserver*		getObserver()				{ return m_observer; }

	// Forward animation - this API is not exhaustive and derived classes may
	// do more than this, but this is the minimum API required for sequencing
	virtual			bool			canSync()					{ return false; }
	virtual			void			jumpToA()					= 0;
	virtual			void			animateFromAToB()			= 0;
	virtual			void			abort()						= 0;

	// Reverse animation - optionally used by sequencing
	virtual			bool			canReverse()				{ return false; }
	virtual			void			jumpToB()					{}
	virtual			void			animateFromBToA()			{}
	virtual			void			reverse()					{}
	
	// Query
	inline			bool			isAnimating()				{ return m_animState != EAnimStopped; }
};

#endif // LcCAnimatorH
