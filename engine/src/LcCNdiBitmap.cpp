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


#if !defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCNdiBitmap> LcCNdiBitmap::create(LcCSpace* sp, LcCNdiGraphics* gfx)
{
	LcTaOwner<LcCNdiBitmap> ref;
	ref.set(new LcCNdiBitmap(sp, gfx));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCNdiBitmap::open(const LcTmString& sFile)
{
	// Load NDI - Compressed implementation draws directly from NDI image
	LcTaOwner<LcCNdiBitmapFile> newNdi = LcCNdiBitmapFile::create();
	m_ndi = newNdi;
	if (!m_ndi->open(sFile, LcCNdiBitmapFile::KFormatAny))
	{
		return false;
	}

	// Set the image dimensions, margins and frame count.
	setActualSize(m_ndi->getSize());
	setMargins(m_ndi->getMarginLeft(), m_ndi->getMarginRight(), m_ndi->getMarginTop(), m_ndi->getMarginBottom());
	setFrameCount(m_ndi->getFrameCount());

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCNdiBitmap::draw(
	int 					frameNo,
	const LcTPlaneRect&		dest,
	const LcTPixelRect&		clipRect,
	LcTColor				color,
	LcTScalar				opacity,
	bool					antiAlias,
	int						meshGridX,
	int						meshGridY)
{
	LC_UNUSED(antiAlias)
	LC_UNUSED(meshGridX)
	LC_UNUSED(meshGridY)

	if (opacity < LC_TRANSPARENT_THRESHOLD)
		return;

	if (opacity > LC_OPAQUE_THRESHOLD)
		opacity = 1;

	if (frameNo < 0)
		frameNo = 0;

	if (frameNo > getFrameCount() - 1)
		frameNo = getFrameCount() - 1;

	// Draw sketchy if mostly transparent, or being animated
	bool bSketch = getSpace()->canSketch();

	// Apply the primary light to the object
	color = getSpace()->getLitObjectColor(color);

	// Blit me baby one more time
	// NB: take extent from unmapped destination rect
	// NB2: note that the integer rounding makes margin scaling approximate
	m_gfx->blitNdiBitmap(
		m_ndi.ptr(),
		frameNo,
		LcTPixelDim((int)dest.getWidth(), (int)dest.getHeight()),
		getSpace()->mapLocalToCanvasSubpixel(dest),
		clipRect,
		color,
		opacity,
		!bSketch); // use linear sampling if not sketchy
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCNdiBitmap::isPointTransparent(int iX, int iY)
{
#if defined(IFX_RENDER_INTERNAL_COMPRESSED)
	// Get pixel alpha value (requires RLE decode)
	TRlePos csr = m_ndi->getRleIndex()[iY];
	rleAdvance(csr, iX);

	// Transparency found?
	if (csr.ptr[0] == 0)
		return true;

#else
	// Get index into data from pixel point
	int pixelPos   = (((iY * m_ndi->getSize().width) + iX) * 4) + 3;

	// Transparency found?
	if (m_ndi->getData()[pixelPos] == 0)
	{
		return true;
	}
#endif

	// Not a transparent pixel.
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCNdiBitmap::isOpaque()
{
	return m_ndi->getFormat() == LcCNdiBitmapFile::KFormatGraphicOpaque;
}

#endif // #if !defined(LC_PLAT_OGL)
