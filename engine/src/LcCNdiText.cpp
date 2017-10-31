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


#if defined(IFX_USE_BITMAPPED_FONTS) && !defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCNdiText> LcCNdiText::create(LcCNdiFont* f)
{
	LcTaOwner<LcCNdiText> ref;
	ref.set(new LcCNdiText(f));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCNdiText::cacheInit(
	int				maxLength,
	LcTScalar		destLeft,
	LcTScalar		destTop,
	LcTScalar		destBottom,
	LcTScalar		destZ)
{
	// Save fixed values
	m_destLeft		= destLeft;
	m_destTop		= destTop;
	m_destBottom	= destBottom;
	m_destZ			= destZ;

	// Initialize map:  maps from X in virtual string bitmap, to X,Y in font bitmap
	if (maxLength > 0)
	{
		m_mapTextToFont.free();
		m_mapTextToFont.alloc(m_mapLength);
	}
	
	// Used for populating map
	m_mapIndex		= 0;
	m_destRight		= m_destLeft + 1;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCNdiText::cacheChar(
	LcTScalar		srcLeft,
	LcTScalar		srcRight,
	LcTScalar		srcTop,
	LcTScalar		srcBottom,
	LcTScalar		destRight)
{
	int pixWidth = (int)(srcRight - srcLeft);
	
	LC_UNUSED(srcBottom) 
	
	for (int i = 0; i < pixWidth && m_mapIndex < m_mapLength; i++)
	{
		// Map pixel X position in string to char X,Y
		m_mapTextToFont[m_mapIndex].glyphX			= (int)(srcLeft + i);
		m_mapTextToFont[m_mapIndex].glyphYOffset	= (int)srcTop;

		// Increment X pixel count
		m_mapIndex++;
	}

	// Keep overwriting as we want to save last value
	m_destRight = destRight;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCNdiText::render(
	const LcTVector&	vPos,
	LcTScalar 			ascent,
	const LcTmString& 	text,
	int					caretPos,
	IFX_INT32			*rtlOut)
{
	// Set the caret character position.
	// This must be done here to reset it when there is no text.
	setCaretCharPos(caretPos);

	// Work out width of "virtual" string bitmap
	m_mapLength = ((LcCBitmappedFont*)getFont())->getPixelWidth(text);

	// When we need to draw caret on empty line
	if ((0 == m_mapLength) && caretPos > -1 && caretPos <= text.length())
		m_mapLength = 1;
	
	if (0 == m_mapLength)
		return;

	// Causes text map to be populated via calls to cacheInit/cacheChar
	((LcCBitmappedFont*)getFont())->render(this, vPos, ascent, text);

	// Store the new string.
	setStringText(text);
	
	// Set the caret height.
	setTextHeight(ascent);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCNdiText::draw(
	const LcTPixelRect&	clip,
	LcTColor			color,
	LcTScalar			opacity,
	bool				antiAlias,
	int					meshGridX,
	int					meshGridY,
	LcCMarqueeRenderer *renderer)
{
	LC_UNUSED(meshGridX);
	LC_UNUSED(meshGridY);
	LC_UNUSED(renderer);

	if (opacity < LC_TRANSPARENT_THRESHOLD || 0 == m_mapLength)
		return;
	

	if (opacity > LC_OPAQUE_THRESHOLD)
		opacity = 1;

	// Work out optimum draw quality - we use worse quality for
	// better frame rate, if the opacity is low or if the image is part
	// of a widget being animated in a way that makes low quality less obvious
	bool bSketch = getFont()->getSpace()->canSketch();

	// Apply the primary light to the object 
	color = getFont()->getSpace()->getLitObjectColor(color);

	// Build destination rect from cached coords 
	// Note: this is done in local 3D space, not canvas space
	LcTPlaneRect rectDest(
		m_destLeft,
		m_destTop,
		m_destZ,
		m_destRight,
		m_destBottom);

	// Blit our "virtual" string bitmap by passing required mapping to font
	((LcCNdiFont*)getFont())->blit(
		(LcCNdiFont::TTextMap*)m_mapTextToFont,
		m_mapLength,
		rectDest,
		clip,
		color,
		opacity,
		bSketch);

	// If we need to draw a caret
	if ((getCaretCharPos() >= 0) && (getCaretCharPos() <= getStringText().length()))
	{
		drawCaret(clip, color, opacity, bSketch, antiAlias);
	}

}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCNdiText::drawCaret(
	const LcTPixelRect&	clip,
	LcTColor			color,
	LcTScalar			opacity,
	bool				bSketch,
	bool				antiAlias)
{
	LC_UNUSED(antiAlias);

	// Work out the left position of the text.
	LcTScalar leftPos = m_destLeft
						+ getFont()->getTextWidth(	getTextHeight(),
													getStringText().subString(0, getCaretCharPos()));

	// Ensure there is enough space to blit the caret.
	// At the left and right edge, it needs moving in.
	if (leftPos <= m_destLeft)
		leftPos = m_destLeft + 0.5;
	else if ((leftPos + LC_CARET_DEST_WIDTH) >= m_destRight)
		leftPos = m_destRight - LC_CARET_DEST_WIDTH;
						
	// Define the caret position from cached coords.
	// Note: this is done in local 3D space, not canvas space
	LcTPlaneRect caretDest( leftPos,
							m_destTop,
							m_destZ + 0.1,
							leftPos + LC_CARET_DEST_WIDTH,
							m_destTop - getTextHeight());

	// Blit me baby one more time
	// NB: in 3D modes, the color may have already been grayed to apply lighting
	((LcCNdiFont*)getFont())->blitCaret(
		getCaret(),
		caretDest,
		clip,
		color,
		opacity,
		bSketch);
}

#endif	// #if defined(IFX_USE_BITMAPPED_FONTS) && !defined(LC_PLAT_OGL)
