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

#ifndef LcCNativeFontH
#define LcCNativeFontH

#include "inflexionui/engine/inc/LcCFont.h"

extern "C"
{
	#include "inflexionui/engine/ifxui_porting.h"
}

/*-------------------------------------------------------------------------*//**
*/
class LcCNativeFont : public LcCFont
{
LC_PRIVATE_INTERNAL_PROTECTED:

	// Resources
	LcCFont::EStyle					m_style;
	IFXP_FONT						m_ifxpFont;
	IFXP_FONT_FACE					m_face;

	// Two-phase construction
									LcCNativeFont(LcCSpace* s)
										: LcCFont(s) {}
									
	LC_IMPORT	void				prepareFont();

LC_PRIVATE_INTERNAL_PUBLIC:
	// Access graphics pointers.
	inline		IFXP_FONT			getFontPtr()	{ return m_ifxpFont; }
										
public:

	// Construction/destruction
	virtual							~LcCNativeFont();

	// LcCFont methods
	LC_VIRTUAL	LcTScalar			getTextWidth(
										LcTScalar			reqHeight, 
										const LcTmString&	text);
	LC_VIRTUAL	void				getScaledTextExtent(LcTScalar reqHeight,
														IFX_UINT32& retWidth,
														IFX_UINT32& retHeight,
														const LcTmString& text,
														IFX_INT32 *rtlOut);
	LC_VIRTUAL	LcCFont::EStyle		getStyle()		{ return m_style; }

	// Local methods
	LC_IMPORT	bool				open(
										const LcTmString&	fontName, 
										LcCFont::EStyle		style);
	LC_VIRTUAL	bool  				isNativeFont() { return true; }
};

#endif //LcCNativeFontH
