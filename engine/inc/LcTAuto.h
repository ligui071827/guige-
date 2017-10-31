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
#ifndef LcTAutoH
#define LcTAutoH

#if defined(IFX_USE_CLEANUP_STACK)
#include "inflexionui/engine/inc/LcCEnvironment.h"
#endif

/*-------------------------------------------------------------------------*//**
	Wrapper template to provide cleanup facility for local variables.  
	Derives from the parameter class such that in the event of cleanup, 
	the destructor for the object will be called, even if it was declared
	as a local variable in a method that has been unwound
*/
template <class TmObject> class LcTaAuto : public TmObject
{
private:

	// Clean-up function - we don't use a virtual for this, as this would 
	// result in generation of vtables for every LcTaAuto<> type
	static void clean(void* obj);

public:

	// Construction/destruction stacks item using minimal generated code
#if defined(IFX_USE_CLEANUP_STACK)
	inline LcTaAuto()	{ LcCEnvironment::pushCleanupItem(clean, this); }
	inline ~LcTaAuto()	{ LcCEnvironment::popCleanupItem(this); }
#else
	inline LcTaAuto()	{}
	inline ~LcTaAuto()	{}
#endif
};

/*-------------------------------------------------------------------------*//**
	Static cleanup method should be out-of-line ...
*/
template <class TmObject> void LcTaAuto<TmObject>::clean(void* obj)
{
	// Type-specific non-virtual call to base destructor
	((TmObject*)obj)->TmObject::~TmObject();
}

#endif //LcTAutoH

