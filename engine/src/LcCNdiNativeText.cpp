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


#if defined(IFX_USE_NATIVE_FONTS) && !defined(LC_PLAT_OGL)


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCNdiNativeText> LcCNdiNativeText::create(LcCNdiNativeFont* f)
{
	LcTaOwner<LcCNdiNativeText> ref;
	ref.set(new LcCNdiNativeText(f));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
LcCNdiNativeText::~LcCNdiNativeText()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCNdiNativeText::draw(
	const LcTPixelRect& clip,
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

	if (!getTextCanvas())
		return;

	if (opacity < LC_TRANSPARENT_THRESHOLD)
		return;

	if (opacity > LC_OPAQUE_THRESHOLD)
		opacity = 1;

	// Origin is Top Left
	LcTVector localTlPos = getTlPos();
	LcTPlaneRect destRect(	localTlPos.x,
	                      	localTlPos.y + getHeight(),
	                      	localTlPos.z,
	                      	localTlPos.x + getWidth(),
	                      	localTlPos.y);

	LcTPixelDim srcSize(getTextCanvas()->width, getTextCanvas()->height);

	bool bSketch = getFont()->getSpace()->canSketch();

	// Apply the primary light to the object 
	color = getFont()->getSpace()->getLitObjectColor(color);

	((LcCNdiNativeFont*)getFont())->getGraphics()->blitRawBitmap(
		(LcTByte*)getTextCanvas()->alphaBuffer,
		srcSize,
		srcSize.width, // 1 bpp for text
		LcCNdiBitmapFile::KFormatFont,
		NULL,
		getFont()->getSpace()->mapLocalToCanvasSubpixel(destRect),
		clip,
		color,
		opacity,
		!bSketch);

	// If we need to draw a caret
	if ((getCaretCharPos() >= 0) && (getCaretCharPos() <= getStringText().length()))
	{
		drawCaret(clip, color, opacity, bSketch, antiAlias);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCNdiNativeText::drawCaret(
	const LcTPixelRect&	clip,
	LcTColor			color,
	LcTScalar			opacity,
	bool				bSketch,
	bool				antiAlias)
{
	LC_UNUSED(antiAlias);

	// Work out the left position of the text.
	LcTVector localTlPos = getTlPos();
	LcTScalar leftPos = localTlPos.x
						+ getFont()->getTextWidth(	getTextHeight(),
													getStringText().subString(0, getCaretCharPos()));

	// Ensure there is enough space to blit the caret.
	// At the left and right edge, it needs moving in.
	if (leftPos <= localTlPos.x)
		leftPos = localTlPos.x + 0.5;
	else if ((leftPos + LC_CARET_DEST_WIDTH) >= localTlPos.x + getWidth())
		leftPos = localTlPos.x + getWidth() - LC_CARET_DEST_WIDTH;

	// Define the caret position from cached coords.
	// Note: Origin is Top Left
	// Also the carat is slightly shorter then the extent of the
	// text box.
	LcTPlaneRect caretDest( leftPos,
	                        localTlPos.y + getTextHeight(),
	                        localTlPos.z + 0.1,
							leftPos + LC_CARET_DEST_WIDTH,
							localTlPos.y + 0.5);

	// Blit me baby one more time
	// NB: in 3D modes, the color may have already been grayed to apply lighting
	((LcCNdiNativeFont*)getFont())->getGraphics()->blitRawBitmap(
		getCaret(),
		LcTPixelDim(LC_CARET_SRC_WIDTH, LC_CARET_SRC_HEIGHT),
		LC_CARET_SRC_WIDTH,
		LcCNdiBitmapFile::KFormatFont,
		NULL,
		getFont()->getSpace()->mapLocalToCanvasSubpixel(caretDest),
		clip,
		color,
		opacity,
		!bSketch);

}

#endif	// defined(IFX_USE_NATIVE_FONTS) && !defined(LC_PLAT_OGL)
