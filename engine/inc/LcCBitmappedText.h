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
#ifndef LcCBitmappedTextH
#define LcCBitmappedTextH

#include "inflexionui/engine/inc/LcCText.h"

/*-------------------------------------------------------------------------*//**
	Implementation of LcCText for working with bitmapped fonts.  Delegates
	rendering to the LcCBitmappedFont class, and provides it with an
	interface for setting up a cache of blit regions
*/
class LcCBitmappedText : public LcCText
{
LC_PRIVATE_INTERNAL_PUBLIC:

	// Called by LcCBitmappedFont to add char rectangles to the cache
	// NB: source coords are passed as scalars for accurate clipping
	virtual			void			cacheInit(
										int				maxLength,
										LcTScalar		destLeft,
										LcTScalar		destTop,
										LcTScalar		destBottom,
										LcTScalar		destZ)		= 0;
	virtual			void			cacheChar(
										LcTScalar		srcLeft,
										LcTScalar		srcRight,
										LcTScalar		srcTop,
										LcTScalar		srcBottom,
										LcTScalar		destRight)	= 0;

protected:

	// Abstract so keep constructors protected
	LC_IMPORT						LcCBitmappedText(LcCBitmappedFont* f)
										: LcCText(f) {}
											
public:

	// LcCText methods
	LC_VIRTUAL			void		render(
										const LcTVector&	vPos,
										LcTScalar 			ascent,
										const LcTmString& 	text,
										int					caretPos,
										IFX_INT32			*rtl);
};

#endif //LcCBitmappedTextH
