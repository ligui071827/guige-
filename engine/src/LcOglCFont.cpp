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


#if defined(IFX_USE_BITMAPPED_FONTS) && defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcOglCFont::~LcOglCFont()
{
	if (m_tex > 0)
		glDeleteTextures(1, &m_tex);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTaOwner<LcCText> LcOglCFont::createText()
{
	// Creates a text object appropriate to this font
	LcTaOwner<LcOglCText> newText = LcOglCText::create(this);
	return newText;
}

#endif // #if defined(LC_PLAT_OGL)
