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
#ifndef LcTOwnerH
#define LcTOwnerH

#include "inflexionui/engine/inc/LcDefs.h"
#include "inflexionui/engine/inc/LcTAuto.h"
#include "inflexionui/engine/inc/LcStl.h"
class LcCBase;

/*-------------------------------------------------------------------------*//**
	Encapsulates a pointer to a C-object (must be derived from LcCBase)
	and ensures that this is deleted when the owner dies.  Only one owner
	should point to an object at a time.  It is an error to assign an object
	to an LcTmOwner that already owns one.

	LcTmOwner should only be used for class members as it is not cleanup safe.
	Use LcTaOwner for automatics
*/
template <class CObject> class LcTmOwner
{
private:

	// Object to be protected - use LcCBase to guarantee type is related
	LcCBase*	m_ptr;

public:

	// Construction/destruction
	LcTmOwner()						: m_ptr(0) {}
	~LcTmOwner()					{ delete m_ptr; }

	// Copy and assignment, with upcasting (set() does typechecking)
	template <class TRhs> LcTmOwner(const LcTmOwner<TRhs>& rhs)
		{ CObject* p = (CObject*)((LcTmOwner<TRhs>&)rhs).release(); m_ptr = (LcCBase*)p; }
	template <class TRhs> LcTmOwner& operator=(const LcTmOwner<TRhs>& rhs)
		{ LcTmOwner::set(((LcTmOwner<TRhs>&)rhs).release()); return *this; }

	// Copy and assignment (must come after upcasting version)
	LcTmOwner(const LcTmOwner& rhs)
		: m_ptr(((LcTmOwner&)rhs).release()) {}
	LcTmOwner& operator=(const LcTmOwner& rhs)
		{ m_ptr = ((LcTmOwner&)rhs).release(); return *this; }

	// Methods
	void		set(CObject* p)		{ LC_ASSERT(!m_ptr); m_ptr = (CObject*)p; }
	CObject*	ptr() const			{ return (CObject*)m_ptr; }
	CObject&	operator *() const	{ return *(CObject*)m_ptr; }
	CObject*	operator ->() const	{ return (CObject*)m_ptr; }
	operator	bool() const		{ return (m_ptr != 0); }
	void		destroy()			{ delete m_ptr; m_ptr = 0; }
	CObject*	release()			{ LcCBase* p = m_ptr; m_ptr = 0; return (CObject*)p; }
};


/*-------------------------------------------------------------------------*//**
	Cleanup safe version of LcTmOwner, to be used for automatic variables.
	Note that Ta and Tm versions may be passed/assigned to each other
*/
template <class CObject> class LcTaOwner : public LcTaAuto<LcTmOwner<CObject> >
{
public:

	// Default construction
	LcTaOwner() {}

	// Copy and assignment
	LcTaOwner(const LcTaOwner& rhs)
		{ LcTaOwner::set(((LcTmOwner<CObject>&)rhs).release()); }
	LcTaOwner& operator=(const LcTaOwner& rhs)
		{ LcTaOwner::set(((LcTmOwner<CObject>&)rhs).release()); return *this; }

	// Copy and assignment, from Tm with upcasting (set() does typechecking)
	template <class TRhs> LcTaOwner(const LcTmOwner<TRhs>& rhs)
		{ LcTaOwner::set(((LcTmOwner<TRhs>&)rhs).release()); }
	template <class TRhs> LcTaOwner& operator=(const LcTmOwner<TRhs>& rhs)
		{ LcTaOwner::set(((LcTmOwner<TRhs>&)rhs).release()); return *this; }

	// Copy and assignment, from Tm version (must come after upcasting version)
	LcTaOwner(const LcTmOwner<CObject>& rhs)
		{ LcTaOwner::set(((LcTmOwner<CObject>&)rhs).release()); }
	LcTaOwner& operator=(const LcTmOwner<CObject>& rhs)
		{ LcTaOwner::set(((LcTmOwner<CObject>&)rhs).release()); return *this; }
};

/*-------------------------------------------------------------------------*//**
	Encapsulates an array of owned pointers, and ensures that the objects pointed
	to are deleted when erased from the array, or when the array is destroyed.

	LcTmOwnerArray should only be used for class members as it is not cleanup
	safe.
*/
template <class CObject> class LcTmOwnerArray : public LcTmArray<CObject*>
{
public:
	typedef typename LcTmArray<CObject*>::iterator iterator;

	// Construction/destruction
	LcTmOwnerArray()					{ LcTmArray<CObject*>::clear(); }
	~LcTmOwnerArray()					{ clear(); }

	// Overridden methods to ensure removing elements from the array
	// deletes the objects they are pointing to
	void		clear()
	{
		erase(LcTmArray<CObject*>::begin(), LcTmArray<CObject*>::end());
	}

	iterator	erase(iterator position)
	{
		delete (*position);
		*position = NULL;
		return LcTmArray<CObject*>::erase(position);
	}

	iterator	erase(iterator first, iterator last)
	{
		iterator temp(first);
		while (temp != last)
		{
			delete (*temp);
			(*temp) = NULL;
			temp++;
		}
		return LcTmArray<CObject*>::erase(first, last);
	}

	void		pop_back()				{ erase(LcTmArray<CObject*>::end()-1); }

	LcTaOwner<CObject> release_back()	{ LcTaOwner<CObject> retVal; retVal.set(LcTmArray<CObject*>::back()); LcTmArray<CObject*>::erase(LcTmArray<CObject*>::end()-1); return retVal; }

	// Overridden methods to ensure elements can be safely
	// pushed onto the array, without leaking memory if it fails.
	// Other methods (e.g. [], insert, assign, at) may not be safe and
	// must be used with care
	void		push_back(const LcTmOwner<CObject>& obj)
	{
		LcTmArray<CObject*>::reserve(LcTmArray<CObject*>::size() + 1);
		LcTmArray<CObject*>::push_back(((LcTmOwner<CObject>&)obj).release());
	}
};

/*-------------------------------------------------------------------------*//**
	Encapsulates a map of owned pointers, and ensures that the objects pointed
	to are deleted when erased from the map, or when the map is destroyed.

	LcTmOwnerMap should only be used for class members as it is not cleanup
	safe.
*/
template <class TIndex, class CObject> class LcTmOwnerMap : public LcTmMap<TIndex, CObject*>
{
public:
	typedef typename LcTmMap<TIndex, CObject*>::iterator iterator;

	// Construction/destruction
	LcTmOwnerMap()					{ LcTmMap<TIndex, CObject*>::clear(); }
	~LcTmOwnerMap()					{ clear(); }

	// Overridden methods to ensure removing elements from the map
	// deletes the objects they are pointing to
	void		clear()
	{
		erase(LcTmMap<TIndex, CObject*>::begin(), LcTmMap<TIndex, CObject*>::end());
	}

	void		erase(iterator position)
	{
		delete position->second;
		position->second = NULL;
		LcTmMap<TIndex, CObject*>::erase(position);
	}

	void		erase(iterator first, iterator last)
	{
		iterator temp(first);
		while (temp != last)
		{
			delete (*temp).second;
			(*temp).second = NULL;
			temp++;
		}
		LcTmMap<TIndex, CObject*>::erase(first, last);
	}

	void		pop_back()			{ erase(LcTmMap<TIndex, CObject*>::end()-1); }

	void		add_element(const TIndex& key, LcTmOwner<CObject>& value)
	{
		if((*this).find(key) != (*this).end())
		{
			delete (*this).find(key)->second;
			(*this).find(key)->second = NULL;
		}
		else
		{
			// Do allocation separate to the assignment to
			// avoid memory leaks
			(*this)[key] = NULL;
		}

		(*this)[key] = value.release();
	}
};

/*-------------------------------------------------------------------------*//**
	Encapsulates an array of pointers allocated using allocUnsafe, and ensures
	that the memory pointed to is freed when erased from the array, or when the
	array is destroyed.

	LcTmAllocArray should only be used for class members as it is not cleanup
	safe.
*/
template <class TObject> class LcTmAllocArray : public LcTmArray<TObject*>
{
public:
	typedef typename LcTmArray<TObject*>::iterator iterator;

	// Construction/destruction
	LcTmAllocArray()					{ LcTmArray<TObject*>::clear(); }
	~LcTmAllocArray()					{ clear(); }

	// Overridden methods to ensure removing elements from the array
	// deletes the objects they are pointing to
	void		clear()
	{
		erase(LcTmArray<TObject*>::begin(), LcTmArray<TObject*>::end());
	}

	iterator	erase(iterator position)
	{
		LcTmAlloc<TObject>::freeUnsafe(*position);
		*position = NULL;
		return LcTmArray<TObject*>::erase(position);
	}

	iterator	erase(iterator first, iterator last)
	{
		iterator temp(first);
		while (temp != last)
		{
			LcTmAlloc<TObject>::freeUnsafe(*temp);
			(*temp) = NULL;
			temp++;
		}
		return LcTmArray<TObject*>::erase(first, last);
	}

	void		pop_back()				{ erase(LcTmArray<TObject*>::end()-1); }

	// Overridden methods to ensure elements can be safely
	// pushed onto the array, without leaking memory if it fails.
	// Other methods (e.g. [], insert, assign, at) may not be safe and
	// must be used with care
	void		push_back(const LcTmAlloc<TObject>& obj)
	{
		LcTmArray<TObject*>::reserve(LcTmArray<TObject*>::size() + 1);
		LcTmArray<TObject*>::push_back(((LcTmAlloc<TObject>&)obj).release());
	}
};


#endif //LcTOwnerH
