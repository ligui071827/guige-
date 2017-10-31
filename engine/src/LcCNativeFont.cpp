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

#if defined(IFX_USE_NATIVE_FONTS)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCNativeFont::~LcCNativeFont()
{
	// Clean up resources
	IFXP_Text_Close_Font(m_ifxpFont);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCNativeFont::open(const LcTmString& fontName, LcCFont::EStyle style)
{
	LcTaString faceName = fontName;

	// Store the member variables.
	m_style = style;

	// Update the face and alignment settings
	prepareFont();

	if (IFX_SUCCESS == IFXP_Text_Open_Font(&m_ifxpFont, m_face, faceName.bufWChar()))
	{
		if (m_ifxpFont)
			return true;
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTScalar LcCNativeFont::getTextWidth(LcTScalar reqHeight,
														const LcTmString& text)
{
	// Scale height in line with current theme
	LcTScalar screenWidth = (LcTScalar)getSpace()->getCanvasBounds().getWidth();
	LcTScalar globalWidth = getSpace()->getGlobalExtent().x;
	LcTScalar scaledHeight = reqHeight;
	
	if (globalWidth > 0)
	{
		LcTScalar scaleFactor = screenWidth / globalWidth;
		scaledHeight *= scaleFactor;
	}

	IFX_UINT32 textWidth = 0;
	IFX_UINT32 textHeight = (IFX_UINT32)scaledHeight;

	if (IFX_SUCCESS == IFXP_Text_Get_Char_Height(m_ifxpFont, &textHeight))
	{
		IFX_INT32 rtl = -1;
		if (IFX_SUCCESS == IFXP_Text_Get_Width(m_ifxpFont, (IFX_WCHAR*)text.bufWChar(), text.length(), &textWidth, &rtl))
		{
			// Return width, scaled to requested height
			return (reqHeight * textWidth) / textHeight;
		}
	}
	
	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCNativeFont::getScaledTextExtent(LcTScalar reqHeight, IFX_UINT32& retWidth, IFX_UINT32& retHeight, const LcTmString& text, IFX_INT32 *rtlOut)
{

	// Scale height in line with current theme
	LcTScalar screenWidth = (LcTScalar)getSpace()->getCanvasBounds().getWidth();
	LcTScalar globalWidth = getSpace()->getGlobalExtent().x;
	LcTScalar scaledHeight = reqHeight;
	
	if (globalWidth > 0)
	{
		LcTScalar scaleFactor = screenWidth / globalWidth;
		scaledHeight *= scaleFactor;
	}

	IFX_UINT32 height = (IFX_UINT32)scaledHeight;
	IFX_UINT32 textWidth = 0;
	
	if (IFX_SUCCESS == IFXP_Text_Get_Char_Height(m_ifxpFont, &height))
	{
		if (IFX_SUCCESS == IFXP_Text_Get_Width(m_ifxpFont, (IFX_WCHAR*)text.bufWChar(), text.length(), &textWidth, rtlOut))
		{
			retWidth = textWidth;
			retHeight = height;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCNativeFont::prepareFont()
{
	LcCFont::EStyle style			= getStyle();

	switch (style)
	{
		default:
		case REGULAR:
			m_face = IFXP_FONT_FACE_NORMAL;
		break;

		case BOLD:
			m_face = IFXP_FONT_FACE_BOLD;
		break;

		case ITALIC:
			m_face = IFXP_FONT_FACE_ITALIC;
		break;

		case BOLDITALIC:
			m_face = IFXP_FONT_FACE_BOLDITALIC;
		break;
	}
}

#endif /* #if defined(IFX_USE_NATIVE_FONTS) */

