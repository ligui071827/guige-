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
#ifndef LcCBaseH
#define LcCBaseH


typedef int SerializeHandle;

// For debugging
#if defined(__SYMBIAN32__) && defined(__WINS__)

	// Counter for heap check nesting
	// NB: Symbian's import/export directives won't work for variables
	#if defined(LC_BUILD_NDE_DLL) || defined(LC_USE_STATIC_NDE)
		__declspec(dllexport) extern int g_heapCheckCount;
	#else
		__declspec(dllimport) extern int g_heapCheckCount;
	#endif

	// Heap locking (can be nested)
	#define LC_START_HEAP_LOCK		\
		{ if (!g_heapCheckCount++) __UHEAP_SETFAIL(RHeap::EDeterministic, 1); }
	#define LC_END_HEAP_LOCK		\
		{ if (!--g_heapCheckCount) __UHEAP_RESET; }

	// Heap balancing (cells not bytes)
	#define LC_START_HEAP_BALANCE	__UHEAP_MARK
	#define LC_END_HEAP_BALANCE		__UHEAP_MARKEND
#else

	// NULL implementations for release builds
	#define LC_START_HEAP_LOCK
	#define LC_END_HEAP_LOCK
	#define LC_START_HEAP_BALANCE
	#define LC_END_HEAP_BALANCE
#endif


/*-------------------------------------------------------------------------*//**
	Root base class for all NDE classes with C prefix (including LcC, LcwC, etc).

	Provides a base type and virtual destructor for use by the LcTOwner classes,
	and also ensures that all new() allocations of C objects must be made via
	a create() 2-phase constructor.  Allocated memory is also zero-initialized
	to save code space in class constructors.
*/
class LcCBase
{
private:

	// Disallow copy/assignment through LcCBase hierarchy
	LcCBase(const LcCBase&);
	LcCBase& operator=(const LcCBase&);

	static		void*	base_new(size_t size);
	static		void	base_delete(void* p);
protected:

	// Abstract hence protected constructor
	inline				LcCBase()	{}
	inline		void	construct()	{}

	// Zeroed memory allocation - and protected to prevent direct new
	LC_IMPORT	void*	operator new(size_t size) THROW_NONE			{ return base_new(size); }
	// Array new
	LC_IMPORT	void*	operator new[](size_t size) THROW_NONE			{ return base_new(size); }

#if !defined(LC_STL_DISABLE_NOTHROW)
	// 'nothrow' new
	LC_IMPORT	void*	operator new(size_t size, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																		{ return base_new(size); }
	// 'nothrow' array new
	LC_IMPORT	void*	operator new[](size_t size, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																		{ return base_new(size); }
#endif

public:

	// Force destructors to be virtual for all C-classes
	virtual				~LcCBase()	{}
	
	// Delete overload
	LC_IMPORT	void	operator delete(void* p) THROW_NONE				{ base_delete(p); }
	// Array delete
	LC_IMPORT	void	operator delete[](void* p) THROW_NONE			{ base_delete(p); }

#if !defined(LC_STL_DISABLE_NOTHROW)
	// 'nothrow' delete
	LC_IMPORT	void	operator delete(void* p, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																		{ base_delete(p); }
	// 'nothrow' array delete
	LC_IMPORT	void	operator delete[](void* p, const LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP nothrow_t&) THROW_NONE
																		{ base_delete(p); }
#endif
};

#endif //LcCBaseH

