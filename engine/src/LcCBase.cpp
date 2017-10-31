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

#include <string.h>

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void* LcCBase::base_new(size_t size)
{
	void* p = LcAllocateMemory(size);

	// Zero alloc'ed memory
	lc_memset(p, 0, size);
	return p;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCBase::base_delete(void* p)
{
	LcDeallocateMemory(p);
}
