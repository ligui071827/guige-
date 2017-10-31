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

extern "C"
{
	#include <math.h>
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
LC_EXPORT_VIRTUAL LcCNativeText::~LcCNativeText()
{
	// If a font canvas has been created then destroy it
	if (m_textCanvas)
	{
		if (IFX_SUCCESS == IFXP_Text_Canvas_Destroy(m_textCanvas))
		{
			m_textCanvas = NULL;
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCNativeText::render(
	const LcTVector&	pos,
	LcTScalar			height,
	const LcTmString& 	text,
	int					caretPos,
	IFX_INT32			*rtlOut)
{
	LcCNativeFont* nativeFont = ((LcCNativeFont*)getFont());

	IFX_UINT32 imgWidth = 0;
	IFX_UINT32 imgHeight = 0;
	
	// Work out how big the text canvas is going to be
	nativeFont->getScaledTextExtent(height, imgWidth, imgHeight, text, rtlOut);

	// Set the caret character position.
	// This must be done here to reset it when there is no text.
	setCaretCharPos(caretPos);
	
	// When we need to draw caret on empty line
	if (!imgWidth && caretPos > -1 && caretPos <= text.length())
		imgWidth = 1;

	// No text No caret to display
	if (imgWidth == 0)
		return;

	// Text top left position
	// NB: pos and height don't affect the rendering
	m_tlPos		= pos;
	m_height	= height;
	m_width		= (imgWidth * height) / imgHeight;
	
	if ((text.length() == 0 || getStringText().compare(text) == 0)
	    && (caretPos == -1 || caretPos > text.length()))
		return;

	// Store the new string.
	setStringText(text);
	
	// Set the caret character height.
	setTextHeight(height);

	// If bitmap has already initialized, close it before reinitializing
	if (m_textCanvas)
	{
		if (IFX_SUCCESS != IFXP_Text_Canvas_Destroy(m_textCanvas))
		{
			// Couldn't destroy the old canvas.
			return;
		}
		m_textCanvas = NULL;
	}

	if (IFX_SUCCESS != IFXP_Text_Canvas_Create(&m_textCanvas, nativeFont->getFontPtr(), (IFX_WCHAR*)text.bufWChar(), rtlOut))
	{
		// Couldn't create the canvas.
		m_textCanvas = NULL;
		LC_CLEANUP_THROW(IFX_ERROR);
	}

	// Verify the returned canvas.
	if ( (!m_textCanvas)                                   ||
	     ((unsigned int)imgWidth != m_textCanvas->width)   || 
	     ((unsigned int)imgHeight != m_textCanvas->height) ||
	     (!m_textCanvas->alphaBuffer)                        )
	{
		// The canvas returned is invalid.
		IFXP_Text_Canvas_Destroy(m_textCanvas);
		m_textCanvas = NULL;
		LC_CLEANUP_THROW(IFX_ERROR);
	}
}

#endif	// defined(IFX_USE_NATIVE_FONTS)

