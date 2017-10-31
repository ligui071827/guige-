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
LC_EXPORT LcCBitmappedFont::LcCBitmappedFont(LcCSpace* sp)
: LcCFont(sp)
{
	// Choose a mapping table to assign characters to bitmap positions
	const unsigned short map[] = LC_BMF_MAP_EURO1;

	// Initialize map (note construction will have cleared others to zero)
	for (int i = 0; i < LC_BMF_MAP_GLYPHS; i++)
	{
		if (map[i] >= LC_BMF_MAP_FIRST && map[i] <= LC_BMF_MAX_CHAR)
			m_map[map[i] - LC_BMF_MAP_FIRST] = i + LC_BMF_ASCII_GLYPHS;
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCBitmappedFont::calculateWidths(void* pData)
{
	int index	= 0;
	int width	= 0;
	bool bBlack = false;

	// First get height from white column data in very first
	// column of the first character (i.e. space).  We check
	// from (x,y) = (0,1) to (0,20).
	m_height = 0;
	for (int y = 1; y < LC_BMF_GLYPH_HEIGHT; y++)
	{
		// Returns true if pixel white, and resets to black
		if (checkPixel(pData, 0, y))
			m_height++;
		else
			break;
	}

	// impose minimum height on the font - stops crash in open GL for corrupted fonts
	if (m_height == 0)
		m_height = 1;

	// Get widths from white line data
	for (int row = 0; row < LC_BMF_ROWS; row++)
	{
		for (int glyph = 0; glyph < LC_BMF_COLUMNS; glyph++)
		{
			for (int col = 0; col < LC_BMF_GLYPH_WIDTH; col++)
			{
				// Returns true if pixel white, and resets to black
				if ((LC_BMF_GLYPH_WIDTH-1 > width)
					&& checkPixel(pData, glyph * LC_BMF_GLYPH_WIDTH + col, row * LC_BMF_GLYPH_HEIGHT))
				{
					bBlack = false;//got a width marker
					width++;
				}

				// Don't want to put a value into memory after the end of our array
				else if (!bBlack || col == LC_BMF_GLYPH_WIDTH - 1)
				{
					m_widths[index] = width;
					width = 0;
					index ++;
					bBlack = true;
					break;//next glyph
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Maps a Unicode character value to a position in the font bitmap
*/
int LcCBitmappedFont::mapCharToPos(LcTWChar ch)
{
	if (ch < LC_BMF_ASCII_FIRST)
	{
		// Control code
		return -1;
	}
	else if (ch < LC_BMF_MAP_FIRST)
	{
		// Low character codes map directly to positions
		return ch - LC_BMF_ASCII_FIRST;
	}
	else if (ch <= LC_BMF_MAX_CHAR)
	{
		// Higher character codes are translated via map
		return m_map[ch - LC_BMF_MAP_FIRST];
	}
	else
	{
		// Special cases, and characters out of mappable range.  These may be
		// mapped onto character values in the reserved control char range, which
		// is in mappable range.  Chosen values correspond to MS codepage 437
		switch (ch)
		{
			case 0x20AC:	return m_map[128 - LC_BMF_MAP_FIRST];	// euro
			case 0x2122:	return m_map[153 - LC_BMF_MAP_FIRST];	// TM
		}
	}

	// Invalid character
	return -1;
}

/*-------------------------------------------------------------------------*//**
	Works out positions of successive characters on both canvas and font
	bitmap, and passes source and destination rectangles to the text cache
*/
LC_EXPORT void LcCBitmappedFont::render(
	LcCBitmappedText*	pText,
	const LcTVector&	vPos,
	LcTScalar 			ascent,
	const LcTmString& 	text)
{
	// Char count of string to draw
	int length				= text.length();

	// Values used in calculations
	LcTScalar	scale		= ascent / m_height;

	// Non-X vertex values - these don't advance
	LcTScalar	vertPosZ	= vPos.z;
	LcTScalar	vertTop		= vPos.y + ascent;
	LcTScalar	vertBottom	= vPos.y;

	// Tracking of X position
	LcTScalar	vertLeft	= vPos.x;

	// If the string is empty, set the left position
	// in case there is a caret.
	if (length == 0)
	{
		pText->cacheInit(
			length,
			vertLeft,
			vertTop,
			vertBottom,
			vertPosZ);
	}
	else	
	{
		// Iterate characters in string
		bool bFirstChar = true;
		for (int i = 0; i < length; i++)
		{
			// Get character position in bitmap
			int charPos = mapCharToPos(text[i]);
			if (charPos < 0)
				continue;

			// Work out vertex positions, unclipped
			LcTScalar vertWidth	= m_widths[charPos] * scale;
			LcTScalar vertRight	= vertLeft + vertWidth;

			// Find pixel position of bitmap character (from top left)
			LcTScalar bmpLeft	= LcTScalar(LC_BMF_GLYPH_WIDTH) * (charPos % LC_BMF_COLUMNS);
			LcTScalar bmpTop	= 1 + LcTScalar(LC_BMF_GLYPH_HEIGHT) * (charPos / LC_BMF_COLUMNS);

			// Get pixel size of bitmap character
			LcTScalar bmpWidth	= (LcTScalar)m_widths[charPos];

			// Space (first char) is shifted one pixel right to make way for height marker
			if (charPos == 0)
				bmpLeft += 1;

			// For the first non-clipped character...
			if (bFirstChar)
			{
				bFirstChar = false;

				// ...we must provide the starting and fixed values
				pText->cacheInit(
					length,
					vertLeft,
					vertTop,
					vertBottom,
					vertPosZ);
			}

			// Cache the clipped character dimensions
			pText->cacheChar(
				bmpLeft,
				bmpLeft + bmpWidth,
				bmpTop,
				bmpTop + m_height,
				vertRight);

			// Move position along by width of char
			vertLeft = vertRight;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcCBitmappedFont::getPixelWidth(
	const LcTmString& text)
{
	int width = 0;

	for (int i = 0; i < text.length(); i++)
	{
		// Get character position in bitmap
		int charPos = mapCharToPos(text[i]);
		if (charPos < 0)
			continue;

		// Add width to total
		width += m_widths[charPos];
	}

	return width;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTScalar LcCBitmappedFont::getTextWidth(
	LcTScalar			height,
	const LcTmString&	text)
{
	// If font didn't load, do nothing
	if (m_height == 0)
		return 0;

	// Work out font scaling factor
	LcTScalar scale = height / m_height;
	return scale * getPixelWidth(text);
}

/*-------------------------------------------------------------------------*//**
	More efficient re-implementation of base class method
*/
LC_EXPORT_VIRTUAL int LcCBitmappedFont::getTextLengthToFit(
	LcTScalar			height,
	const LcTmString&	text,
	LcTScalar			width)
{
	// If font didn't load, do nothing
	if (m_height == 0)
		return 0;

	// Work out font scaling factor
	LcTScalar availableWidth	= width * (m_height / height);
	LcTScalar totalWidth		= 0;

	int i = 0;
	for (; i < text.length(); i++)
	{
		// Get character position in bitmap
		int charPos = mapCharToPos(text[i]);
		if (charPos < 0)
			continue;

		// Add width to total
		totalWidth += m_widths[charPos];

		// Break is before incrementing i, so i is number that do fit
		if (totalWidth > availableWidth)
			break;
	}

	return i;
}

#endif /* #if defined(IFX_USE_BITMAPPED_FONTS) */
