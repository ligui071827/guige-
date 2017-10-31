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
#ifndef LcCNdiTextH
#define LcCNdiTextH

#include "inflexionui/engine/inc/LcCBitmappedText.h"

/*-------------------------------------------------------------------------*//**
	Used for text stored in the NDI format.
*/
class LcCNdiText : public LcCBitmappedText
{
private:

	// Fixed values
	LcTScalar						m_destLeft;
	LcTScalar						m_destRight;
	LcTScalar						m_destTop;
	LcTScalar						m_destBottom;
	LcTScalar						m_destZ;

	// Map from effective pixel column into char bitmap
	LcTmAlloc<LcCNdiFont::TTextMap>	m_mapTextToFont;
	int								m_mapLength;
	int								m_mapIndex;

	// Called by LcCBitmappedFont to add char rectangles to the cache
	LC_VIRTUAL		void			cacheInit(
										int				maxLength,
										LcTScalar		destLeft,
										LcTScalar		destTop,
										LcTScalar		destBottom,
										LcTScalar		destZ);
	LC_VIRTUAL		void			cacheChar(
										LcTScalar		srcLeft,
										LcTScalar		srcRight,
										LcTScalar		srcTop,
										LcTScalar		srcBottom,
										LcTScalar		destRight);

	// Two-phase construction
	inline							LcCNdiText(LcCNdiFont* f)
										: LcCBitmappedText(f) {}

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcCNdiText> create(LcCNdiFont* f);

	// LcCText methods
	LC_VIRTUAL			void		render(
										const LcTVector&	vPos,
										LcTScalar 			ascent,
										const LcTmString& 	text,
										int					caretPos,
										IFX_INT32			*rtlOut);
		
	LC_VIRTUAL		void			draw(
										const LcTPixelRect& clip,
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

#endif //LcCNdiTextH
