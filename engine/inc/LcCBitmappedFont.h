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
#ifndef LcCBitmappedFontH
#define LcCBitmappedFontH

/*-------------------------------------------------------------------------*//**
	NDE bitmapped font parameters.  These are exposed here for access by
	the font creation tool, which should #define LC_BMF_VALUES_ONLY.

	NDE font bitmaps currently support 144 glyphs, or as determined by
	parameters below.  For Latin languages, these glyphs represent the 
	following characters:-
		Glyphs 0-94 (95 chars) - Unicode characters 32-126 (basic Latin)
		Glyphs 95-143 (49 chars) - Characters from one of tables below
*/
#define LC_BMF_BITMAP_WIDTH		256		// bitmap size
#define LC_BMF_BITMAP_HEIGHT	256
#define LC_BMF_COLUMNS			12		// glyphs per row
#define LC_BMF_ROWS				12		// glyphs per column
#define LC_BMF_GLYPH_WIDTH		21		// cell size
#define LC_BMF_GLYPH_HEIGHT		21
#define LC_BMF_MAX_CHAR			255		// last Unicode char supported
#define	LC_BMF_NUM_GLYPHS		144		// number of glyphs in font bitmap
#define LC_BMF_ASCII_GLYPHS		95		// first N chars map to ASCII sequence
#define LC_BMF_ASCII_FIRST		32		// first char of ASCII sequence mapped
#define LC_BMF_MAP_GLYPHS		(LC_BMF_NUM_GLYPHS - LC_BMF_ASCII_GLYPHS)
#define LC_BMF_MAP_FIRST		(LC_BMF_ASCII_FIRST + LC_BMF_ASCII_GLYPHS)

// Covers French and Spanish
#define LC_BMF_MAP_EURO1								\
{														\
	128, 153, 161, 163, 169, 171, 174, 186, 187, 191,	\
	192, 193, 194, 196, 199, 200, 201, 202,	203, 205,	\
	206, 207, 209, 211, 212, 217, 218, 219,	220, 224,	\
	225, 226, 228, 231, 232, 233, 234, 235, 237, 238,	\
	239, 241, 243, 244, 249, 250, 251, 252, 255			\
}

// The rest of the file is not required by the FontCreator tool
#ifndef LC_BMF_VALUES_ONLY

#include "inflexionui/engine/inc/LcCFont.h"
class LcCBitmappedText;

/*-------------------------------------------------------------------------*//**
	Implementation of an NDE font that stores all glyph information is a 
	single bitmap, and draws text by blitting rectangular sections of this
	to typesetting rectangles on the canvas
*/
class LcCBitmappedFont : public LcCFont
{
private:

	// Font metrics
	int								m_widths[LC_BMF_NUM_GLYPHS];
	int								m_height;

	// Character code map
	int								m_map[LC_BMF_MAX_CHAR + 1 - LC_BMF_MAP_FIRST];

	// Private helpers
					int				mapCharToPos(LcTWChar ch);

	// Helper to be implemented by derived class
	virtual			bool			checkPixel(void* pData, int x, int y) = 0;

LC_PRIVATE_INTERNAL_PUBLIC:

	// Rendering method called by LcCBitmappedText
	LC_IMPORT		void			render(
										LcCBitmappedText*	pText,
										const LcTVector&	vPos,
										LcTScalar 			ascent,
										const LcTmString& 	text);

protected:

	// Helpers
	LC_IMPORT		void			calculateWidths(void* pData);

	// Effectively abstract so keep constructors protected
	LC_IMPORT						LcCBitmappedFont(LcCSpace* sp);

public:

	// LcCFont methods
	LC_VIRTUAL		LcTScalar		getTextWidth(
										LcTScalar			height, 
										const LcTmString&	text);
	LC_VIRTUAL		int				getTextLengthToFit(
										LcTScalar			height, 
										const LcTmString&	text, 
										LcTScalar			width);

	// Utilities
	LC_IMPORT		int				getPixelWidth(const LcTmString& text);
	LC_IMPORT		int				getPixelHeight()	{ return m_height; }
};

#endif //LC_BMF_VALUES_ONLY
#endif //LcCBitmappedFontH
