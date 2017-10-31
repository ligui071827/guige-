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


#if defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcOglCBitmap::~LcOglCBitmap()
{
	m_transData.free();
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCBitmap::construct()
{
	m_texture = LcOglCTexture::create(getSpace()->getOglContext());

	LcCBitmap::construct();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcOglCTexture* LcOglCBitmap::getTexture(LcTScalarRect& src)
{
	// Bitmap size
	LcTPixelDim d	= getSize();

	// Must return the texture size including all frames etc
	src.setLeft(0);
	src.setTop(0);
	src.setZDepth(0);
	src.setRight((LcTScalar)d.width);
	src.setBottom((LcTScalar)(d.height * getFrameCount()));

	// Done
	return m_texture.ptr();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcOglCBitmap::drawRegion(
	const LcTScalarRect&	srcRect,
	const LcTPlaneRect&		destRect,
	const LcTPixelRect&		clipRect,
	LcTColor				color,
	LcTScalar				opacity,
	bool					antiAlias,
	int						meshGridX,
	int						meshGridY)
{
	if (opacity < LC_TRANSPARENT_THRESHOLD)
		return;

	if (opacity > LC_OPAQUE_THRESHOLD)
		opacity = 1;
	
	// Thunderbirds are GO...
	m_texture->drawRectangle(
		srcRect,
		destRect,
		color,
		opacity,
		antiAlias,
		meshGridX,
		meshGridY);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcOglCBitmap::createTransparencyData(LcTByte* bmpData, int numberPixels)
{
	// Source data true length
	int sourceLen = numberPixels * 4;
	int buffSize  = (numberPixels>>3) + 1;

	if(m_transData)
		return;
	// Create and initialize transparency data buffer
	m_transData.alloc(buffSize);
	lc_memset(m_transData, 0, buffSize * sizeof(LcTByte));

	int bytePos = -1;
	int bitPos  = 0;

	int alphaChannelLocation = 3;

	for (int n = alphaChannelLocation; n < sourceLen; n += 4)
	{
		// every fourth byte is the alpha value
		int alphaValue = bmpData[n];

		// increment on full byte
		if (bitPos % 8 == 0)
		{
			bitPos = 0;
			bytePos++;
		}

		// check pixel alpha status
		if (alphaValue < LC_ALPHA_MIN)
			m_transData[bytePos] |= (1 << bitPos);

		bitPos++;
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL bool LcOglCBitmap::isPointTransparent(int iX, int iY)
{
	if (!m_transData)
		return false;

	// Get index into data from pixel point
	int pixelPos   = (iY * getSize().width) + iX;
	int bytePos    = pixelPos >> 3;
	int bitPos     = pixelPos % 8;

	// found transparency?
	if (m_transData[bytePos] & ((LcTByte)1 << bitPos))
	{
		return true;
	}

	// not a transparent pixel
	return false;
}

#endif // #if defined(LC_PLAT_OGL)
