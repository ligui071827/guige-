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
#ifndef LcTAllocH
#define LcTAllocH

// Need this for the placement new used below
#include <new>

#include "inflexionui/engine/inc/LcTAuto.h"

/*-------------------------------------------------------------------------*//**
	Encapsulates a pointer to an item allocated on the heap using new.
	It ensures that the object or buffer is deleted when the alloc object
	dies.

	LcTmAlloc should only be used for class members as it is not cleanup safe.
	Use LcTaAlloc for automatics.

	LcTmAlloc also ensures that memory allocations are done in a way that
	throws an exception and triggers a clean-up if the allocation fails.
	This may not be the case with the global new() operator.  Therefore
	even when encapsulation of the pointer with a LcTmAlloc object is not
	required, it is recommended that all memory allocations be done with
	LcTmAlloc::allocUnsafe().

	Note that LcTmOwner and LcTaOwner should be used for managing C-objects.
	Those classes provide a better level of type safety, enforce proper
	construction and destruction, allow ownership to be passed to or
	returned from methods, and properly support upcasting within the
	class hierarchy.  The LcTAlloc classes are much simpler and intended
	only for simple objects such as buffers.

	Note also that LcTAlloc does not guarantee correct destructor calls for
	arrays of objects with constructors.
*/
template <class TType> class LcTmAlloc
{
private:

	// Object to be protected - use LcCBase to guarantee type is related
	TType*		m_ptr;
	int			m_count;

	// Disallow copy or assignment
	LcTmAlloc(const LcTmAlloc& rhs);
	LcTmAlloc&	operator=(const LcTmAlloc& rhs);

	// Perform an allocation with supplied allocator
	static TType* doAlloc(int count, void* allocator(size_t))
	{
		#ifdef __SYMBIAN32__
			return new(ELeave) TType[count];
		#else
			void* p = allocator(count * sizeof(TType) + sizeof(int));

			if (p)
			{
				int* pTemp = (int*) p;

				// Store count, move pointer on by sizeof(int)
				*pTemp = count;
				pTemp++;
				p = (void*) pTemp;


				if (p)
				{
					// Call placement new on each element in the array
					int i;
					for (i = 0; i < count; i++)
					{
						new ((TType*)p + i) TType;
					}
				}
			}

			return (TType*) p;
		#endif
	}

	// Free memory with supplied deallocator
	static void doFree(void* p, void deallocator(void*))
	{
		#ifdef __SYMBIAN32__
			delete[] p;
		#else
			if (p)
			{
				void* pMem = (void*)(((int*)p) - 1);
				int count = *((int*)pMem);

				// Call destructor on each element in the array
				int i;
				for (i = 0; i < count; i++)
				{
					((TType*)p + i)->~TType();
				}

				// Free memory
				deallocator(pMem);
			}
		#endif
	}

public:

	// Construction/destruction
	LcTmAlloc()						: m_ptr(0), m_count(0) {}
	LcTmAlloc(int count)			: m_ptr(0), m_count(0) { alloc(count); }
	~LcTmAlloc()					{free();}

	// Allocate an array of items (using default constructors if any)
	static TType* allocUnsafe(int count)
	{
		// Use allocUnsafe() even when only simple allocation is required,
		// in platform-agnostic code, to allow platform-specific allocation
		// even on those platforms that don't let us override global
		// operator new, such as Symbian WINS
		return doAlloc(count, LcAllocateMemoryUnsafe);
	}

	// Free an array of items previously allocated with allocUnsafe
	static void freeUnsafe(void * p)
	{
		doFree(p, LcDeallocateMemoryUnsafe);
	}


	// Allocate/free protected pointer
	void		alloc(int count)
	{
		LC_ASSERT(!m_ptr);
		m_ptr = doAlloc(count, LcAllocateMemory);
		m_count = m_ptr ? count : 0;
	}
	void		free()
	{
		doFree(m_ptr, LcDeallocateMemory);
		m_ptr = 0;
		m_count = 0;
	}

	// Methods
	operator	TType*() const		{ return m_ptr; }
	TType&		operator *() const	{ return *m_ptr; }
	TType*		operator ->() const	{ return m_ptr; }
	void		attach(TType* p)	{ free(); m_ptr = p; }
	TType*		release()			{ TType* p = m_ptr; m_ptr = 0; return p; }
	int			count()				{ return m_count; }
};

// LcTByte specialization of the LcTmAlloc class - more efficient for large byte buffers
template <> 
class LcTmAlloc<LcTByte>
{
private:
	// Object to be protected
	LcTByte*	m_ptr;
	int			m_count;

	// Disallow copy or assignment
	LcTmAlloc(const LcTmAlloc& rhs);
	LcTmAlloc&	operator=(const LcTmAlloc& rhs);

	// Perform an allocation with supplied allocator
	static LcTByte* doAlloc(int count, void* allocator(size_t))
	{
		#ifdef __SYMBIAN32__
			return new(ELeave) LcTByte[count];
		#else
			void* p = allocator(count * sizeof(LcTByte));

			return (LcTByte*) p;
		#endif
	}

	// Free memory with supplied deallocator
	static void doFree(void* p, void deallocator(void*))
	{
		#ifdef __SYMBIAN32__
			delete[] p;
		#else
			if(p)
			{
				// Free memory
				deallocator(p);
			}
		#endif
	}

public:

	// Construction/destruction
	LcTmAlloc()						: m_ptr(0), m_count(0) {}
	LcTmAlloc(int count)			: m_ptr(0), m_count(0) { alloc(count); }
	~LcTmAlloc()					{free();}

	// Allocate an array of items (using default constructors if any)
	static LcTByte* allocUnsafe(int count)
	{
		// Use allocUnsafe() even when only simple allocation is required,
		// in platform-agnostic code, to allow platform-specific allocation
		// even on those platforms that don't let us override global
		// operator new, such as Symbian WINS
		return doAlloc(count, LcAllocateMemoryUnsafe);
	}

	// Free an array of items previously allocated with allocUnsafe
	static void freeUnsafe(void * p)
	{
		doFree(p, LcDeallocateMemoryUnsafe);
	}


	// Allocate/free protected pointer
	void		alloc(int count)
	{
		LC_ASSERT(!m_ptr);
		m_ptr = doAlloc(count, LcAllocateMemory);
		m_count = m_ptr ? count : 0;
	}
	void		free()
	{
		doFree(m_ptr, LcDeallocateMemory);
		m_ptr = 0;
		m_count = 0;
	}

	// Methods
	operator	LcTByte*() const	{ return m_ptr; }
	LcTByte&	operator *() const	{ return *m_ptr; }
	LcTByte*	operator ->() const	{ return m_ptr; }
	void		attach(LcTByte* p)	{ free(); m_ptr = p; }
	LcTByte*	release()			{ LcTByte* p = m_ptr; m_ptr = 0; return p; }
	int			count()				{ return m_count; }
};

/*-------------------------------------------------------------------------*//**
	Cleanup safe version of LcTmAlloc, to be used for automatic variables
*/
template <class TType> class LcTaAlloc : public LcTaAuto<LcTmAlloc<TType> >
{
private:

	// Disallow copy or assignment
	LcTaAlloc(const LcTaAlloc& rhs);
	LcTaAlloc& operator=(const LcTaAlloc& rhs);

public:

	// Construction
	LcTaAlloc()						{ LcTmAlloc<TType>();}
	LcTaAlloc(int count)			{ LcTmAlloc<TType>::alloc(count); }
};

#endif //LcTAllocH
