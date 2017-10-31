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
	Internal class private to LcTCleanup, for managing stack frames
*/
#ifdef __SYMBIAN32__
class LcTFrame : public CBase
#else
class LcTFrame
#endif
{
public:

	// Cleanup item points to object, and method to clean it
	class TItem
	{
	public:

		void*						m_object;
		LcCCleanup::TFunction		m_cleaner;

		// Construction
		TItem(LcCCleanup::TFunction cf, void* obj)  { m_object = obj; m_cleaner = cf; }
	};

	// Trap point for this frame
#ifndef __SYMBIAN32__
	jmp_buf							m_jmpBuf;
#endif

	// Frame stack is singly-linked list
	LcCCleanup*						m_cleanup;
	LcTFrame*						m_prevFrame;

	// Stack of items to clean up
	typedef LcTmArray<TItem>		TmAItem;
	TmAItem							m_itemStack;

	// Construction/destruction
	LC_IMPORT						LcTFrame(LcCCleanup* c);
	LC_IMPORT						~LcTFrame();

	inline			void*			operator new(size_t size) THROW_NONE	
																{ return LcAllocateMemory(size); }
	inline			void*			operator new[](size_t size) THROW_NONE	
																{ return LcAllocateMemory(size); }
#if !defined(LC_STL_DISABLE_NOTHROW)
	inline			void*			operator new(size_t size, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																{ return LcAllocateMemory(size); }
	inline			void*			operator new[](size_t size, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																{ return LcAllocateMemory(size); }
#endif
	inline			void			operator delete(void* p) THROW_NONE	
																{ LcDeallocateMemory(p); }
	inline			void			operator delete[](void* p) THROW_NONE
																{ LcDeallocateMemory(p); }
#if !defined(LC_STL_DISABLE_NOTHROW)
	inline			void			operator delete(void* p, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																{ LcDeallocateMemory(p); }
	inline			void			operator delete[](void* p, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																{ LcDeallocateMemory(p); }
#endif
};

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTFrame::LcTFrame(LcCCleanup* c)
{
	m_cleanup	= c;

	// Add frame to global stack
	m_prevFrame				= m_cleanup->m_topFrame;
	m_cleanup->m_topFrame	= this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTFrame::~LcTFrame()
{
	// When frame is destructed, clean up any non-popped items
	for (int i = (int)m_itemStack.size() - 1; i >= 0; i--)
	{
		TItem item = m_itemStack[i];
		(LcCCleanup::TFunction(item.m_cleaner))(item.m_object);
	}

	// Pop top trap frame, which this should be
	LC_ASSERT(this == m_cleanup->m_topFrame);
	m_cleanup->m_topFrame = m_prevFrame;
}

/*-------------------------------------------------------------------------*//**
	Creates and pushes a new stack frame as a trap return point.  Returns 0
	after creation.  If it returns non-zero, it's because a leave has occurred,
	and the value is the leave code.  This only happens on non-Symbian
	platforms - on Symbian, the native cleanup trap mechanism is used
*/
LC_EXPORT jmp_buf* LcCCleanup::pushFrame()
{
	// Create stack frame and for Symbian, push to Symbian's own stack
#ifdef __SYMBIAN32__
	LcTFrame* f = new (ELeave) LcTFrame(this);
	CleanupStack::PushL(f);
	return 0;
#else

	// For non-Symbian, manually configure frame jump point
	LcTFrame* f = new LcTFrame(this);
	return (f ? &(f->m_jmpBuf) : NULL);
#endif
}

/*-------------------------------------------------------------------------*//**
	Removes frame from stack on normal exit - on leave, popFrame() is skipped
*/
LC_EXPORT void LcCCleanup::popFrame()
{
	// Destroy top trap frame
#ifdef __SYMBIAN32__
	CleanupStack::PopAndDestroy(m_topFrame);
#else

#ifdef IFX_MEMORYTEST_DYNAMIC
	// m_topFrame is null when LcTFrame(this) call
	// from pushFrame fail and later on we popFrame
	// is called
	if (m_topFrame)
#endif
	delete m_topFrame;
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCCleanup::leaveFrame(int e)
{
	// Symbian's own cleanup stack will delete/clean the frame
#ifdef __SYMBIAN32__
	User::Leave(e);
#else

	// Delete the frame, keeping the jump buf.  Deleting cleans up the items,
	// and we do this during leave rather than trap in case the longjmp modifies
	// or frees any of the stack memory in which the auto items reside
	jmp_buf jb;
	memcpy(jb, m_topFrame->m_jmpBuf, sizeof(jb));
	delete m_topFrame;

	// Jump out of frame
	longjmp(jb, e);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCCleanup::pushItem(TFunction cf, void* obj)
{
#ifdef IFX_MEMORYTEST_DYNAMIC
	// When memory allocation request for LcTFrame (from pushFrame()) 
	// is failed forcefully and in the meantime if takeScreenshot() is
	// called from LcCEnvironment, we do have topFrame null, so we get 
	// an exception
	if (m_topFrame)
#endif
	m_topFrame->m_itemStack.push_back(LcTFrame::TItem(cf, obj));
}

/*-------------------------------------------------------------------------*//**
	Pops the specified cleanup item off the stack frame.  Popping is
	optimized towards the item being at the top of the stack, but if the
	item is not at the very top, it will be removed from its actual
	position rather than removing the top element.
*/
LC_EXPORT void LcCCleanup::popItem(void* obj)
{
	if (m_topFrame == NULL)
		return;

	// Find index of item being popped
	int i = (int)m_topFrame->m_itemStack.size() - 1;
	for (; i >= 0; i--)
	{
		// Normally, item being popped will be the top one - this will
		// only be likely to be different when returning values on some
		// platforms where construction/destruction orders aren't opposite
		if (m_topFrame->m_itemStack[i].m_object == obj)
			break;
	}

	// Quit if item not found
	if (i < 0)
		return;

	// Shifts stack items down if required
	for (; i < (int)(m_topFrame->m_itemStack.size()) - 1; i++)
		m_topFrame->m_itemStack[i] = m_topFrame->m_itemStack[i + 1];

	// Abandon top slot
	m_topFrame->m_itemStack.pop_back();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcCCleanup::getItemCount()
{
	return (int)(m_topFrame ? m_topFrame->m_itemStack.size() : 0);
}
