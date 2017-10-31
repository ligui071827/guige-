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


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCFont::LcCFont(LcCSpace* sp)
{
	m_space		= sp;
	m_iRefCount	= 0;
}

/*-------------------------------------------------------------------------*//**
	Does not tell the caller whether the font object was deleted, because
	the caller should never use it again anyway!
*/
LC_EXPORT void LcCFont::release()
{
	// NB: we don't assume that we CAN delete the object.... the space
	// implementation may pool them
	if (--m_iRefCount <= 0)
		m_space->unloadFont(this);
}

#ifdef LC_ENABLE_CJK_SUPPORT

#define BOL_CHAR_LEN 86
#define EOL_CHAR_LEN 20

// BOL KINSOKU char sorted array. CJK text line can't start with these characters.
LcTWChar	bolkchar[BOL_CHAR_LEN] = {
	'!',	'\'',	')',	',',	'-',
	'.',	':',	';',	'?',	']',
	'_',	'}',	'~', 0x00A8, 0x00B0,
 0x00B4, 0x2010, 0x2015, 0x2019, 0x201D,
 0x2025, 0x2026, 0x2032, 0x2033, 0x2103,
 0x2225, 0x3001, 0x3002, 0x3003, 0x3005,
 0x3006, 0x3007, 0x3009, 0x300B, 0x300D,
 0x300F, 0x3011, 0x3015, 0x3041, 0x3043,
 0x3045, 0x3047, 0x3049, 0x3063, 0x3083,
 0x3085, 0x3087, 0x308E, 0x309B, 0x309C,
 0x309D, 0x309E, 0x30A1, 0x30A3, 0x30A5,
 0x30A7, 0x30A9, 0x30C3, 0x30E3, 0x30E5,
 0x30E7, 0x30EE, 0x30F5, 0x30F6, 0x30FB,
 0x30FC, 0x30FD, 0x30FE, 0x4EDD, 0xFF01,
 0xFF09, 0xFF0C, 0xFF0E, 0xFF0F, 0xFF1A,
 0xFF1B, 0xFF1F, 0xFF3C, 0xFF3D, 0xFF3E,
 0xFF3F, 0xFF40, 0xFF5C, 0xFF5D, 0xFF5E,
 0xFFE3
};

// EOL KINSOKU char sorted array. CJK text line can't end with these characters.
LcTWChar	eolkchar[EOL_CHAR_LEN] = {
	'(',	'[',	'{',	0x00a7, 0x00b0,
	0x2018,	0x201c,	0x2032,	0x2033,	0x2103,
	0x3008,	0x300a,	0x300c,	0x300e,	0x3010,
	0x3014,	0xff08,	0xff20,	0xff3b,	0xff5b
};

/*-------------------------------------------------------------------------*//**
*/
bool LcCFont::isRestrictedChar(
		LcTWChar* charArray, int length, LcTWChar ch)
{
	int start		= 0;
	int end			= length - 1;

	while (start <= end)
	{
		int mid		= (start + end) >> 1;

		if (ch > charArray[mid])
			start	= mid + 1;
		else if (ch < charArray[mid])
			end		= mid - 1;
		else
			return true;
	}

	return false;

}

bool LcCFont::isCJKchar(LcTWChar ch)
{
	return ((0x1100 <= (ch) && (ch) <= 0x11ff) ||
			(0x2e80 <= (ch) && (ch) <= 0xd7ff) ||
			(0xf900 <= (ch) && (ch) <= 0xfaff) ||
			(0xff00 <= (ch) && (ch) <= 0xffef) );
}
#endif // LC_ENABLE_CJK_SUPPORT

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaArray<LcTmString> LcCFont::wrapLines(
	LcTScalar fHeight, const LcTmString& text, LcTScalar fWidth, bool marqueeH)
{
	LcTaArray<LcTmString> out;

	// For this routine only, define a single char placeholder for linebreaks
	LcTWChar internalLineBreakChar = '\001';

	// Split off lines that fit, breaking at spaces
	LcTaString s;

	// Preprocess string for escape characters
	int c = 0;
	int textLength = text.length();
	while (c < textLength)
	{
		if (text[c] == '\\')
		{
			c++;
			if (c < textLength)
			{
				LcTWChar nextChar = text[c];
				if (nextChar == 'n')
				{
					s += internalLineBreakChar;
				}
				else if(((c + 3) < textLength) 
					&& text[c] == '0' 
					&& text[c + 1] == '1'
					&& text[c + 2] == '2')
				{
					s += internalLineBreakChar;
					c++; //0
					c++; //1
				}
				else
				{
					// Unknown escape sequences just have backslash stripped: so eg \k -> k
					// This also covers '\\' case
					s += nextChar;
				}
			}
		}
		else if (text[c] != internalLineBreakChar)
		{
			// Ignore any line break characters originally in the string
			s += text[c];
		}

		c++;
	}

	LcTaString substr;

	while (true)
	{
		// Get number of chars that fit in available width
		int nLen = s.length();

		if (!marqueeH)
		{
			nLen = getTextLengthToFit(fHeight, s, fWidth);
		}

		// Find first occurrence of break character
		int searchStartPos = 0;
		int nBrk = s.find(LcTaString((char)internalLineBreakChar), searchStartPos);

		substr.fromBufUtf8(s.bufUtf8(),nBrk);

		// If break occurs before width reached, must split there
		if (nBrk >= 0 && substr.length() < nLen)
		{
			// Output the bit that fits
			out.push_back(substr);

			// Chop off the bit we've output, and continue
			s = s.subString(substr.length() + 1, 99999);
		}
		else if (nLen == s.length() || nLen < 2)
		{
			// Simple case - we can fit the rest in
			if(s.length() > 0)
				out.push_back(s);
			break;
		}
		else
		{
			// Otherwise search back for a breakable point
			const LcTWChar* pStart	= s.bufWChar();
			const LcTWChar* pA		= pStart + nLen;
			const LcTWChar* pB		= pStart + nLen;
			pA--;
			while (pA >= pStart)
			{
				// Drop out if a line-break is allowed between pA and pB
				if (*pB == L' ')
					break;
				#ifdef LC_ENABLE_CJK_SUPPORT 
				else if (isCJKchar(*pB) || isCJKchar(*pA))
				{
					if (!isRestrictedChar(eolkchar,EOL_CHAR_LEN, *pA) &&
						!isRestrictedChar(bolkchar,BOL_CHAR_LEN, *pB))
						break;
				}
				#endif // LC_ENABLE_CJK_SUPPORT 

				// Simple non-strict Japanese wrap... allow a break if either
				// of the adjacent characters has an UTF8 encoding of 3 or 4 bytes.
				// Might not be correct for other languages though
// MR 07/10/05 - removed due to line breaks at euro or TM chars.
// Need to be a bit less sweeping about allowing breaks
//				if (*pA >= 0x7FF || *pB >= 0x7FF)
//					break;

				// Keep going backwards
				pA--;
				pB--;
			}

			// If we didn't find a break point, break at the fit length
			if (pB > pStart)
				nLen = (int)(pB - pStart);
			else
				pB = pStart + nLen;

			// Skip anything at the break point that should be stripped
			// NB: must finish with pB before using s.subString()
			// NB: AR 25/5/07 We are unable to workout caret position if unspecified number of
			// spaces are stripped. So just strip one space.
			//while (*pB == L' ')
			if (*pB == L' ')
				pB++;

			// Output the bit that fits
			out.push_back(s.subString(0, nLen));

			// Chop off the bit we've output, and continue
			s = s.subString((int)(pB - pStart), 99999);
		}
	}

	return out;
}

/*-------------------------------------------------------------------------*//**
	Returns the number of chars to fit in the given width
*/
LC_EXPORT_VIRTUAL int LcCFont::getTextLengthToFit(
	LcTScalar height, const LcTmString& text, LcTScalar available)
{
	// Check usual case - string fits fine
	int len = text.length();
	if (getTextWidth(height, text) <= available)
		return len;

	// End points of viable lengths, narrowed by binary search
	int doesFit = 0;
	int mightFit = len - 1;

	// Binary search is done when the length that fits is the largest that might
	while (doesFit < mightFit)
	{
		// Test mid-point, rounding up
		int testFit = (doesFit + mightFit + 1) / 2;
		int w = (int)(getTextWidth(height, text.subString(0, testFit)));

		// Change lower or upper end point according to result
		if (w <= available)
			doesFit = testFit;
		else
			mightFit = testFit - 1;
	}

	// Done
	return doesFit;
}

