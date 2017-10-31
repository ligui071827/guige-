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
#ifndef LcOglCTextH
#define LcOglCTextH

/*-------------------------------------------------------------------------*//**
	Implementation of bitmapped text that draws characters from an 
	OpenGL texture
*/
class LcOglCText : public LcCBitmappedText
{
private:
	// Size/actual
	int								m_length;

	// Initialized values
	LcOglTScalar					m_destLeft;
	LcOglTScalar					m_destRight;
	LcOglTScalar					m_destTop;
	LcOglTScalar					m_destBottom;
	LcOglTScalar					m_destZ;

	// Arrays for direct OpenGL access
	LcTmAlloc<LcOglTScalar>			m_pVertices;
	LcTmAlloc<LcOglTScalar>			m_pNormals;
	LcTmAlloc<LcOglTScalar>			m_pTexCoords;
	LcTmAlloc<GLushort>				m_pIndex;

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
	LC_IMPORT						LcOglCText(LcOglCFont* f)
										: LcCBitmappedText(f) {}

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcOglCText> create(LcOglCFont* f);

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
										bool				antiAlias);
};

#endif //LcOglCTextH
