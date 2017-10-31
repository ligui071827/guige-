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


#if defined(IFX_USE_NATIVE_FONTS) && !defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCNdiNativeFont> LcCNdiNativeFont::create(LcCSpace* sp, LcCNdiGraphics* g)
{
	LcTaOwner<LcCNdiNativeFont> ref;
	ref.set(new LcCNdiNativeFont(sp, g));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
LC_EXPORT_VIRTUAL LcCNdiNativeFont::~LcCNdiNativeFont()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTaOwner<LcCText> LcCNdiNativeFont::createText()
{
	// Creates a text object appropriate to this font
	LcTaOwner<LcCNdiNativeText> newText = LcCNdiNativeText::create(this);
	return newText;
}

#endif // #if defined(IFX_USE_NATIVE_FONTS) && !defined(LC_PLAT_OGL)

