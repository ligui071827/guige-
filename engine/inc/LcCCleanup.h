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
#ifndef LcCCleanupH
#define LcCCleanupH

#include "inflexionui/engine/inc/LcCBase.h"

// NB: for Symbian, we must integrate with the OS cleanup framework so that 
// leaves triggered by memory allocation failures etc are handled correctly.
// However, we cannot use the cleanup stack directly for LcTaAuto<> items
// because of an anomaly in the construction/destruction order of return values.  
// Therefore we use Symbian's stack only for frames, and manage our own stack 
// for items
#ifdef __SYMBIAN32__
	#include <e32base.h>
	#define LC_CLEANUP_TRAP(_e, _f)										\
		{ TRAP(_e,														\
		  { LcCCleanup* _cs = LcCEnvironment::get()->getCleanupStack();	\
			jmp_buf* pJmpBuf = _cs->pushFrame();						\
			_e = ( pJmpBuf ? setjmp(*pJmpBuf) : -1 );					\
			if (!_e) { _f; _cs->popFrame();	}}
#else
	#include <setjmp.h>
	#define LC_CLEANUP_TRAP(_e, _f)										\
		{	LcCCleanup* _cs = LcCEnvironment::get()->getCleanupStack();	\
			jmp_buf* pJmpBuf = _cs->pushFrame();						\
			_e = ( pJmpBuf ? setjmp(*pJmpBuf) : -1 );					\
			if (!_e) { _f; _cs->popFrame(); }}
#endif

#if !defined(NDHS_OMIT_CLEANUP_STACK_FRAME)

// Cleanup stack frame management - we must ensure that a stack frame
// is pushed on entry to system event handlers and other entry points,
// and popped before returning
#define	LC_CLEANUP_THROW(err)											\
	{	LcCEnvironment* _env = LcCEnvironment::get();					\
		if (_env) 														\
		{																\
			LcCCleanup* _cs = _env->getCleanupStack();					\
			if(_cs && _cs->frameExists())								\
				_cs->leaveFrame(err);									\
		}																\
	}

#define	LC_CLEANUP_PUSH_FRAME(_e)										\
	{	LcCEnvironment* _env = LcCEnvironment::get();					\
		if (_env) 														\
		{	LcCCleanup* _cs = _env->getCleanupStack();					\
			jmp_buf* pJmpBuf = _cs->pushFrame();						\
			_e = ( pJmpBuf ? setjmp(*pJmpBuf) : -1 );					\
			{
	#define	LC_CLEANUP_POP_FRAME(_e)									\
			}															\
			LC_ASSERT(_cs->getItemCount() == 0);						\
			if (!_e)													\
				_cs->popFrame();										\
			else if (_e == IFX_ERROR_RESTART)						\
				LC_CLEANUP_THROW(IFX_ERROR_RESTART);				\
		}															\
	}

#else

// When using the Engine as the Previewer for the UIDesigner, the 
// Engine must not throw any "IFX_ERROR_RESTART" exceptions.
// The 'longjmp' calls which are used by the Engine to throw that exceptions
// do not always have an according 'setjmp'.

#define	LC_CLEANUP_THROW(err)											\
	{																	\
	}

#define	LC_CLEANUP_PUSH_FRAME(_e)										\
	{
#define	LC_CLEANUP_POP_FRAME(_e)										\
	}

#endif

class LcTFrame;

/*-------------------------------------------------------------------------*//**
	Class that manages a cleanup stack of objects with associated 
	cleaner functions
*/
class LcCCleanup : public LcCBase
{
	friend class LcTFrame;

public:

	// Type for cleanup function
	typedef void (*TFunction)(void*);

private:

	// Stack head
	LcTFrame*						m_topFrame;

public:

	// Construction
	inline							LcCCleanup()		{ m_topFrame = 0; }
	
	// For stacking trap frames
	LC_IMPORT		jmp_buf*		pushFrame();
	LC_IMPORT		void			popFrame();

	// For immediate exit of a frame
	LC_IMPORT		void			leaveFrame(int e);

	// For stacking cleanup items
	LC_IMPORT		void			pushItem(TFunction cf, void* obj);
	LC_IMPORT		void			popItem(void* obj);

	// For stack checking
	LC_IMPORT		int				getItemCount();
	inline			bool			frameExists()		{ return (m_topFrame !=	0); }
};

#endif //LcCCleanupH
