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
LC_EXPORT LcTaOwner<LcCCustomBitmap> LcCCustomBitmap::create(LcCSpace* sp, LcCNdiGraphics* gfx)
{
	LcTaOwner<LcCCustomBitmap> ref;
	ref.set(new LcCCustomBitmap(sp, gfx));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCCustomBitmap::open(const LcTmString& sFile,
												int marginLeft,
												int marginRight,
												int marginTop,
												int marginBottom,
												int frameCount)
{
	// Load Custom - 
	LcTaOwner<LcCCustomBitmapFile> newCustom = LcCCustomBitmapFile::create();
	m_custom = newCustom;
	if (!m_custom->open(sFile,
			marginLeft,
			marginRight,
			marginTop,
			marginBottom,
			frameCount))
	{
		return false;
	}

	if(m_custom->getFormat()==LcCCustomBitmapFile::KFormatCompressedOpenGL)
		return false;

	LcTByte * imageData=NULL;
	IFX_UINT32 length=0;

	// read and cache data
	if(!m_custom->readData((void**)&imageData,0,&length,true))
		return false;

	m_custom->releaseData(imageData);
	m_isTranslucent = m_custom->isTranslucent();
	m_custom->close();

	// Set the image dimensions, margins and frame count.
	setActualSize(m_custom->getSize());
	setMargins(marginLeft, marginRight, marginTop, marginBottom);
	setFrameCount(frameCount);
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcCCustomBitmap::draw(
	int 					frameNo,
	const LcTPlaneRect&		dest,
	const LcTPixelRect&		clipRect,
	LcTColor				color,
	LcTScalar				opacity)
{
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

	m_gfx->blitCustomBitmap(
		m_custom.ptr(),
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
LC_EXPORT_VIRTUAL bool LcCCustomBitmap::isPointTransparent(int iX, int iY)
{
	// Not a transparent pixel.
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcCCustomBitmap::isOpaque()
{
	return m_custom->getFormat() == LcCCustomBitmapFile::KFormatGraphicOpaque;
}

#endif // #if !defined(LC_PLAT_OGL)
