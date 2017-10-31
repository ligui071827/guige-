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
#ifndef LcCNdiFontH
#define LcCNdiFontH

#include "inflexionui/engine/inc/LcCBitmappedFont.h"
class LcCNdiBitmapFile;
class LcCNdiGraphics;

/*-------------------------------------------------------------------------*//**
	Implementation of NDE bitmapped font class that draws from NDI to
	graphics surface directly
*/
class LcCNdiFont : public LcCBitmappedFont
{
public:

	// An array of these maps from virtual string bitmap into the glyph bitmap
	struct TTextMap
	{
		short glyphX;
		short glyphYOffset;
	};

private:

	// Glyphs drawn directly from loaded 24bpp NDI data
	LcTmOwner<LcCNdiBitmapFile>		m_ndi;

	// Graphics object to draw to
	LcCNdiGraphics*					m_gfx;

	// LcCBitmappedFont methods
	LC_VIRTUAL		bool			checkPixel(void* pData, int x, int y);

	// Two-phase construction
	inline							LcCNdiFont(LcCSpace* sp, LcCNdiGraphics* gfx)
										: LcCBitmappedFont(sp) { m_gfx = gfx; }

LC_PRIVATE_INTERNAL_PUBLIC:

	inline		LcCNdiGraphics*		getGraphics()	{ return m_gfx; }

					// Used by LcCNdiText for drawing strings
					void			blit(
										TTextMap*				mapTextToFont,
										int						mapLength,
										const LcTPlaneRect&		dest,
										const LcTPixelRect&		rectClip,
										LcTColor				color,
										LcTScalar				opacity,
										bool					bSketch);

					// Used by LcCNdiText for drawing carets
					void			blitCaret(
										LcTByte*				caretData,
										const LcTPlaneRect&		caretDest,
										const LcTPixelRect&		rectClip, 
										LcTColor				color,
										LcTScalar				opacity,
										bool					bSketch);

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcCNdiFont> create(LcCSpace* sp, LcCNdiGraphics* gfx);

	// LcCFont methods
	LC_VIRTUAL	LcTaOwner<LcCText>	createText();

	// Local methods
	LC_IMPORT		bool			open(const LcTmString& fileName);
};

#endif //LcCNdiFontH
