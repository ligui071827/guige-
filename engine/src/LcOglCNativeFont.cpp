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


#if defined(IFX_USE_NATIVE_FONTS) && defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcOglCNativeFont> LcOglCNativeFont::create(LcCSpace* sp)
{
	LcTaOwner<LcOglCNativeFont> ref;
	ref.set(new LcOglCNativeFont(sp));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
LC_EXPORT_VIRTUAL LcOglCNativeFont::~LcOglCNativeFont()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTaOwner<LcCText> LcOglCNativeFont::createText()
{
	// Creates a text object appropriate to this font
	LcTaOwner<LcOglCNativeText> newText = LcOglCNativeText::create(this);
	return newText;
}

#endif // #if defined(IFX_USE_NATIVE_FONTS) && defined(LC_PLAT_OGL)

