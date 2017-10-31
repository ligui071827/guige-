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
*/
LC_EXPORT LcTaOwner<LcCBitmapFrame> LcCBitmapFrame::create()
{
	LcTaOwner<LcCBitmapFrame> ref;
	ref.set(new LcCBitmapFrame);
	ref->construct();
	return ref;
}

