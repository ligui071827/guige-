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

#ifndef LcCNdiNativeTextH
#define LcCNdiNativeTextH

#include "inflexionui/engine/inc/LcCText.h"
class LcCNdiNativeFont;

/*-------------------------------------------------------------------------*//**
*/
class LcCNdiNativeText : public LcCNativeText
{
private:

	// Two-phase construction
									LcCNdiNativeText(LcCNdiNativeFont* f)
										: LcCNativeText(f) {}

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcCNdiNativeText> create(LcCNdiNativeFont* f);
	virtual							~LcCNdiNativeText();

	// LcCText methods
	LC_VIRTUAL		void			draw(
										const LcTPixelRect&	clip,
										LcTColor			color,
										LcTScalar			opacity,
										bool				antiAlias,
										int					meshGridX,
										int					meshGridY,
										LcCMarqueeRenderer *renderer);
					void			drawCaret(
    									const LcTPixelRect&	clip,
    									LcTColor			color,
    									LcTScalar			opacity,
    									bool				bSketch,
										bool				antiAlias);
};

#endif //LcCNdiNativeTextH
