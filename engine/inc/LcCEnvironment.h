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
#ifndef LcCEnvironmentH
#define LcCEnvironmentH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcCCleanup.h"
#include "inflexionui/engine/inc/LcTAlloc.h"
class LcCSpace;

/*-------------------------------------------------------------------------*//**
	LcCEnvironment provides a single global instance via the static get()
	method.  This stores state global to the UI.  Note that it can only be
	called from the main UI thread.
*/
class LcCEnvironment
{
private:

	// Global state
	LcCSpace*						m_active;
	LcCCleanup						m_stack;
	//for heap dedicated to text data
#ifndef __SYMBIAN32__
	static LcCEnvironment*			s_pEnv;
#endif

	// Construction
	LC_IMPORT						LcCEnvironment();

LC_PRIVATE_INTERNAL_PUBLIC:

	// Short calls to prevent LcTaAuto template code bloat
	LC_IMPORT static void			pushCleanupItem(LcCCleanup::TFunction cf, void* obj);
	LC_IMPORT static void			popCleanupItem(void* obj);

public:
									~LcCEnvironment();

	// Access to singleton instance
	LC_IMPORT static LcCEnvironment* get();
	//create environment on heap
	LC_IMPORT static void			create();
	//clean up environment
	LC_IMPORT static void			free();

	// String heap allocation / deallocation / compression
	LC_IMPORT		void*			stringAlloc(int size);
	LC_IMPORT		void			stringFree(void* pPtr);
	LC_IMPORT		void			compressHeap();

	// Access to cleanup stack
	inline			LcCCleanup*		getCleanupStack()			{ return &m_stack; }

	// Access to state
	inline			void			setActiveSpace(LcCSpace* a)	{ m_active = a; }
	inline			LcCSpace*		getActiveSpace()			{ return m_active; }

	inline			void*			operator new(size_t size) THROW_NONE	
																{ return LcAllocateMemory(size); }
	// Array new
	inline			void*			operator new[](size_t size) THROW_NONE	
																{ return LcAllocateMemory(size); }
#if !defined(LC_STL_DISABLE_NOTHROW)
	// 'nothrow' new
	inline			void*			operator new(size_t size, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																{ return LcAllocateMemory(size); }
	// 'nothrow' array new
	inline			void*			operator new[](size_t size, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																{ return LcAllocateMemory(size); }
#endif
	// Delete overload
	inline			void			operator delete(void* p) THROW_NONE	{ LcDeallocateMemory(p); }
	// Array delete
	inline			void			operator delete[](void* p) THROW_NONE
																{ LcDeallocateMemory(p); }
#if !defined(LC_STL_DISABLE_NOTHROW)
	// 'nothrow' delete
	inline			void			operator delete(void* p, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																{ LcDeallocateMemory(p); }
	// 'nothrow' array delete
	inline			void			operator delete[](void* p, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																{ LcDeallocateMemory(p); }
#endif
};

#endif // LcCEnvironmentH

