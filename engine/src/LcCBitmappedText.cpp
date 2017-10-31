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


#if defined(IFX_USE_BITMAPPED_FONTS)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCBitmappedText::render(
	const LcTVector&	vPos,
	LcTScalar 			ascent,
	const LcTmString& 	text,
	int					caretPos,
	IFX_INT32			*rtl)
{
	((LcCBitmappedFont*)getFont())->render(this, vPos, ascent, text);

	// Store the new string.
	setStringText(text);
	
	// Set the caret character position and its height.
	setCaretCharPos(caretPos);
	setTextHeight(ascent);
}

#endif /* #if defined(IFX_USE_BITMAPPED_FONTS) */
