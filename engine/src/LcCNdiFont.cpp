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
LC_EXPORT LcTaOwner<LcCNdiFont> LcCNdiFont::create(LcCSpace* sp, LcCNdiGraphics* gfx)
{
	LcTaOwner<LcCNdiFont> ref;
	ref.set(new LcCNdiFont(sp, gfx));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcTaOwner<LcCText> LcCNdiFont::createText()
{
	// Creates a text object appropriate to this font
	LcTaOwner<LcCNdiText> newText = LcCNdiText::create(this);
	return newText;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCNdiFont::open(const LcTmString& fileName)
{
	// Load bitmap from NDI file
	LcTaOwner<LcCNdiBitmapFile> newNdi = LcCNdiBitmapFile::create();
	m_ndi = newNdi;
	if (!m_ndi->open(fileName, LcCNdiBitmapFile::KFormatFont)
	||   m_ndi->getSize().width != 256
	||   m_ndi->getSize().height != 256)
		return false;

	// This sets up width array in base, using checkPixel()
	calculateWidths(NULL);

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCNdiFont::checkPixel(void* pData, int x, int y)
{
	LC_UNUSED(pData)

	// For linear NDI data we just do a straight index
	unsigned char* pByte	= m_ndi->getData();
	int index				= (y * 256) + x;

	// NB: treat as black if value is less than 1/16 intensity
	bool nonBlack			= (pByte[index] > LC_ALPHA_MIN);
	pByte[index]			= 0;

	return nonBlack;
}

/*-------------------------------------------------------------------------*//**
*/
void LcCNdiFont::blit(
	TTextMap*				mapTextToFont, 
	int						mapLength,
	const LcTPlaneRect&		dest,
	const LcTPixelRect&		rectClip,
	LcTColor				color,
	LcTScalar				opacity,
	bool					bSketch)
{
	if (opacity < LC_TRANSPARENT_THRESHOLD)
		return;

	if (opacity > LC_OPAQUE_THRESHOLD)
		opacity = 1;
	// Draw to attached surface using given text-to-font map
	m_gfx->blitNdiText(
		m_ndi.ptr(),
		mapTextToFont,
		LcTPixelDim(mapLength, getPixelHeight()),
		getSpace()->mapLocalToCanvasSubpixel(dest),
		rectClip,
		color,
		opacity,
		!bSketch); // use draw linear sampling if not sketchy
}

/*-------------------------------------------------------------------------*//**
	Used by LcCNdiText for drawing carets
*/
void LcCNdiFont::blitCaret(
	LcTByte*				caretData,
	const LcTPlaneRect&		caretDest,
	const LcTPixelRect&		rectClip, 
	LcTColor				color,
	LcTScalar				opacity,
	bool					bSketch)
{
	// Blit me baby one more time
	m_gfx->blitRawBitmap(
		caretData,
		LcTPixelDim(LC_CARET_SRC_WIDTH, LC_CARET_SRC_HEIGHT),
		LC_CARET_SRC_WIDTH,
		LcCNdiBitmapFile::KFormatFont,
		NULL,
		getSpace()->mapLocalToCanvasSubpixel(caretDest),
		rectClip,
		color,
		opacity,
		!bSketch);
}

#endif

