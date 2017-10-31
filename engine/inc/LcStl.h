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
#ifndef LcStlH
#define LcStlH

#include "inflexionui/engine/inc/LcDefs.h"
#include "inflexionui/engine/inc/LcTAuto.h"

/*-------------------------------------------------------------------------*//**
	FILE CURRENTLY EXISTS ONLY FOR BACKWARD COMPATIBILITY
	WILL BE SUBSUMED BY LcAll.h AND NEW CONTAINER CLASSES SOON
*/

// Include STL containers and map NDE container names onto them, for now

/* If platform specific includes are provided, use them, else use the default one. */

#ifdef      IFX_STL_INC_PATH_VECTOR
#include    IFX_STL_INC_PATH_VECTOR
#else
#include <vector>
#endif

#ifdef      IFX_STL_INC_PATH_DEQUE
#include    IFX_STL_INC_PATH_DEQUE
#else
#include <deque>
#endif

#ifdef      IFX_STL_INC_PATH_MAP
#include    IFX_STL_INC_PATH_MAP
#else
#include <map>
#endif

#ifdef      IFX_STL_INC_PATH_UTILITY
#include    IFX_STL_INC_PATH_UTILITY
#else
#include <utility>
#endif

#ifdef      IFX_STL_INC_PATH_SET
#include    IFX_STL_INC_PATH_SET
#else
#include <set>
#endif

#ifdef      IFX_STL_INC_PATH_ALGORITHM
#include    IFX_STL_INC_PATH_ALGORITHM
#else
#include <algorithm>
#endif

#ifdef      IFX_STL_INC_PATH_FUNCTIONAL
#include    IFX_STL_INC_PATH_FUNCTIONAL
#else
#include <functional>
#endif

#include <stddef.h>

#if defined(LC_STL_USING_ENABLED)
using std::vector;
using std::deque;
using std::map;
using std::multimap;
using std::sort;
using std::find;
using std::set;
using std::pair;
using std::make_pair;
using std::less;
using std::iterator_traits;
#endif

// We provide our own allocator template to allow us to control the memory allocation
// mechanism used by the STL classes.
template <class T>
class ifx_allocator;

// specialized class for void as void references are illegal in c++
template <>
class ifx_allocator<void>
{
	public:
		typedef void*			pointer;
		typedef const void*		const_pointer;
		// reference to void members are impossible.
		typedef void 			value_type;
		template <class U> struct rebind { typedef ifx_allocator<U> other; };
};

template <class T>
class ifx_allocator
{
	public:
		typedef size_t			size_type;
		typedef ptrdiff_t		difference_type;
		typedef T*				pointer;
		typedef const T*		const_pointer;
		typedef T&				reference;
		typedef const T&		const_reference;
		typedef T				value_type;
		template <class U> struct rebind { typedef ifx_allocator<U> other; };

		// Assume the application memory pool is already constructed before
		// any STL allocations take place.  Note that THROW_NONE is set up in LcDefs.h
		// according to whether the compiler supports 'throw()' or not.
		template <class U> ifx_allocator(const ifx_allocator<U>&) THROW_NONE	{}
		ifx_allocator() THROW_NONE												{}
		ifx_allocator(const ifx_allocator&) THROW_NONE							{}

		// Non-virtual destructor - this class is not designed to be inherited
		// from.
		~ifx_allocator() THROW_NONE										{}

		// Get addresses of references
		pointer address(reference x) const								{ return &x; }
		const_pointer address(const_reference x) const					{ return &x; }

		// Map the allocate, deallocate functions to the platform-specific
		// wrappers
		pointer allocate(size_type n, ifx_allocator<void>::const_pointer hint = 0)
																		{
																			void* pMem = LcAllocateMemory(n * sizeof(T));
																			LC_UNUSED(hint);
																			return static_cast<pointer>(pMem);
																		}
		void deallocate(void* p, size_type n)							{
																			LC_UNUSED(n);
																			LcDeallocateMemory(p);
																		}

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
		/* used by the _Tree template in VC++ 6 (and maybe earlier) */
		char  *_Charalloc(size_type _N)
																		{
																			void* pMem = LcAllocateMemory(_N * sizeof(char));
																			return static_cast<char *>(pMem);
																		}
#endif
		// Max number of items that can be allocated - we don't know this
		// so return something suitably huge.
		size_type max_size() const THROW_NONE							{ return (0xFFFFFFFF / sizeof(T)); }

		// Construct using placement new.
		void construct(pointer p, const T& val)							{ new (p) T(val); }
		// Destroy using the class destructor
		void destroy(pointer p)											{ p->~T(); }
};

// Standard allocator functionality allowing allocator to be used
// with STL containers - these say that one allocator can dealloc
// storage allocated by another allocator
template <class T1, class T2>
bool operator==(const ifx_allocator<T1>&, const ifx_allocator<T2>&) THROW_NONE { return true; }

template <class T1, class T2>
bool operator!=(const ifx_allocator<T1>&, const ifx_allocator<T2>&) THROW_NONE { return false; }

// Derive vector and map classes from the STL versions, using our allocator
// "M" versions of each
template <class T> class LcTmArray			: public LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP vector	<T, ifx_allocator<T> >
{
public:
};
template <class T1, class T2> class LcTmMap : public LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP map		<T1, T2, less<T1>, ifx_allocator<pair<const T1, T2> > >
{
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	/* VC++ 6 won't compile with our map usage unless we re-implement this method */
	public:
	referent_type& operator[](const key_type& _Keyval)
		{	// find element matching _Keyval or insert with default mapped
		iterator _Where = this->lower_bound(_Keyval);
		if (_Where == this->end()
			|| this->_Tr.key_comp()(_Keyval, (*_Where).first))
			_Where = this->insert(_Where,
				value_type(_Keyval, referent_type()));
		return ((*_Where).second);
		}
#endif
};

// Cleanup-safe "A" version of array container
template <class T> class LcTaArray : public LcTaAuto<LcTmArray<T> >
{
public:
	LcTaArray()										{}
	LcTaArray(const LcTaArray& x)					{ LcTmArray<T>::operator=(x); }
	LcTaArray(const LcTmArray<T>& x)				{ LcTmArray<T>::operator=(x); }
};

// Cleanup-safe "A" version of map container
template <class T1, class T2> class LcTaMap : public LcTaAuto<LcTmMap<T1, T2> >
{
public:
	LcTaMap()							{}
	LcTaMap(const LcTaMap& x)			{ LcTmMap<T1, T2>::operator=(x); }
	LcTaMap(const LcTmMap<T1, T2>& x)	{ LcTmMap<T1, T2>::operator=(x); }
};

// Sorting algorithm that doesn't recurse (the STL sort is prone to stack overflows when
// sorting certain key frame lists).  We'll use Shell sort, as it should be pretty quick
// for nearly-sorted arrays.

#define IFX_STL_SORT_SPAN_ARRAY_SIZE 9

// Version using a specified comparison function
template <class _RandomAccessIter, class _Compare>
inline void IFX_ShellSort(_RandomAccessIter __first, _RandomAccessIter __last, _Compare __comp)
{
	// Optimal span pattern - good for arrays of up to 3k in size, which should cope with
	// all key frame lists / trigger lists / menu item arrays.  It will work with larger
	// arrays, but at sub-optimal speed.
	const unsigned spanSize[IFX_STL_SORT_SPAN_ARRAY_SIZE] = {1, 4, 10, 23, 57, 132, 301, 701, 1750};

	// How many items are we sorting
	unsigned arraySize = (__last - __first);

	// Find best first gap size
	int i = IFX_STL_SORT_SPAN_ARRAY_SIZE-1;
	for(; i >= 0 && spanSize[i] > arraySize; i--);

	// Now start Shell sort
	for(; i >= 0; i--)
	{
		unsigned span = spanSize[i];
		for(_RandomAccessIter elementToMove = __first + span; elementToMove < __last; elementToMove++)
		{
			typename iterator_traits<_RandomAccessIter>::value_type temp = *elementToMove;
			_RandomAccessIter elementInRowAbove = elementToMove;
			while(elementInRowAbove >= __first + span && __comp(temp, *(elementInRowAbove-span)))
			{
				*elementInRowAbove = *(elementInRowAbove-span);
				elementInRowAbove -= span;
			}
			*elementInRowAbove = temp;
		}
	}
}

// Version using the '<' operator
template <class _RandomAccessIter>
inline void IFX_ShellSort(_RandomAccessIter __first, _RandomAccessIter __last)
{
	// Optimal span pattern - good for arrays of up to 3k in size, which should cope with
	// all key frame lists / trigger lists / menu item arrays.  It will work with larger
	// arrays, but at sub-optimal speed.
	const unsigned spanSize[IFX_STL_SORT_SPAN_ARRAY_SIZE] = {1, 4, 10, 23, 57, 132, 301, 701, 1750};

	// How many items are we sorting
	unsigned arraySize = (__last - __first);

	// Find best first gap size
	int i = IFX_STL_SORT_SPAN_ARRAY_SIZE-1;
	for(; i >= 0 && spanSize[i] > arraySize; i--);

	// Now start Shell sort
	for(; i >= 0; i--)
	{
		unsigned span = spanSize[i];
		for(_RandomAccessIter elementToMove = __first + span; elementToMove < __last; elementToMove++)
		{
			typename iterator_traits<_RandomAccessIter>::value_type temp = *elementToMove;
			_RandomAccessIter elementInRowAbove = elementToMove;
			while(elementInRowAbove >= __first + span && (temp < *(elementInRowAbove-span)))
			{
				*elementInRowAbove = *(elementInRowAbove-span);
				elementInRowAbove -= span;
			}
			*elementInRowAbove = temp;
		}
	}
}

// Get rid of these completely if possible
#define LcTMultiMap		multimap
#define LcTQueue		deque
#define LcTPair			pair
#define makePair		make_pair

#endif // LcStlH
