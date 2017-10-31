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

#ifdef IFX_USE_PLUGIN_ELEMENTS

#include "inflexionui/engine/inc/NdhsCMemoryImage.h"

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCMemoryImage> NdhsCMemoryImage::create(LcCSpace* sp, int w, int h, IFX_BUFFER_FORMAT format, EImageType imageType)
{
	LcTaOwner<NdhsCMemoryImage> ref;
	ref.set(new NdhsCMemoryImage(sp));

	if(ref->construct(w, h, format, imageType) != true)
		ref.destroy();

	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMemoryImage::NdhsCMemoryImage(LcCSpace* sp)
#if defined(LC_PLAT_OGL)
	: LcOglCBitmap(sp)
#else
	: LcCBitmap(sp)
#endif
{
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCMemoryImage::~NdhsCMemoryImage()
{
	m_data.free();
	m_buffer = NULL;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMemoryImage::construct(int width, int height, IFX_BUFFER_FORMAT format, EImageType imageType)
{
#if !defined(LC_PLAT_OGL)
	// always render to the canvas
	m_gfx = getSpace()->getCanvasGfx();
#endif

#if defined(LC_PLAT_OGL)
	m_texture = LcOglCTexture::create(getSpace()->getOglContext());
#endif

	m_imageType = imageType;
	m_format   = format;

	return resize(width, height);
}

#if defined(LC_PLAT_OGL)
/*-------------------------------------------------------------------------*//**
*/
void NdhsCMemoryImage::drawRegion(
	const LcTScalarRect&	src,
	const LcTPlaneRect&		dest,
	const LcTPixelRect&		clip,
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

	// Create the texture and copy the processed data into it.
	m_texture->updateTexture(m_buffer);

	m_texture->drawRectangle(
		src,
		dest,
		color,
		opacity,
		antiAlias,
		meshGridX,
		meshGridY);
}

#else // defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMemoryImage::drawRegion(
	const LcTScalarRect&	src,
	const LcTPlaneRect&		dest,
	const LcTPixelRect&		clip,
	LcTColor				color,
	LcTScalar				opacity,
	bool					antiAlias,
	int						meshGridX,
	int						meshGridY)
{
	LC_UNUSED(meshGridX);
	LC_UNUSED(meshGridY);
	
	if (opacity < LC_TRANSPARENT_THRESHOLD)
		return;

	if (opacity > LC_OPAQUE_THRESHOLD)
		opacity = 1;

	if (m_buffer == NULL || m_format != IFX_32BPP_RGBA)
		return;

	// Draw sketchy if mostly transparent, or being animated
	bool bSketch = getSpace()->canSketch();

	// Render the buffer data
	m_gfx->blitRawBitmap(
		(unsigned char*)m_buffer,
		LcTPixelDim(m_width, m_height),
		m_width * 4,
		LcCNdiBitmapFile::KFormatGraphicTranslucent,
		NULL,
		getSpace()->mapLocalToCanvasSubpixel(dest),
		clip,
		color,
		opacity,
		!bSketch); // use linear sampling if not sketchy
}
#endif // defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMemoryImage::isPointTransparent(int iX, int iY)
{
	bool retVal = false;
	int offset;

	if(iX < 0 || iY < 0)
		return false;

	if ( (m_imageType == EImageTypeInternalBuffer)
		|| (m_imageType == EImageTypeExternalBuffer) )
	{
		// If the buffer is not valid then we won't be drawn.
		if (m_buffer == NULL)
			return true;

		// We can only test for transparency on the 32BPP format
		if (m_format != IFX_32BPP_RGBA)
			return false;

		// calculate offset into data array for specified pixel
		offset = (iY*m_width + iX)*4 + 3;

		if(offset < m_width*m_height*4)
		{
			// within data buffer
			if(m_buffer != NULL && *(LcTByte*)((unsigned char*)m_buffer + offset) == 0x00)
				retVal = true;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMemoryImage::resize(int width, int height)
{
	// Don't allow buffers to be deleted, just resized
	if ((width <= 0) || (height <= 0))
		return false;

	if (m_imageType == EImageTypeInternalBuffer)
	{
		if ((m_height * m_width) < (height * width))
		{
			// New buffer will exceed current buffer's memory - we'll have to re-allocate
			if(m_data != NULL)
				m_data.free();
		}

		// Allocate buffer for image - buffer is always 32BPP RGBA
		if (m_data == NULL)
		{
			m_data.alloc(width * height * sizeof(unsigned));
			m_buffer = m_data;
		}

		if (m_buffer != NULL)
			lc_memset(m_buffer, 0, width * height * sizeof(unsigned));
		else
			return false;
	}

	m_width = width;
	m_height = height;
	setActualSize(LcTPixelDim(width, height));

#if defined(LC_PLAT_OGL)
	if ( (m_imageType == EImageTypeInternalBuffer)
		|| (m_imageType == EImageTypeExternalBuffer) )
	{
		LcTPixelDim imageSize(width, height);
		switch (m_format)
		{
			default:
			case IFX_32BPP_RGBA:
				m_texture->requestTextureRefresh(imageSize, GL_UNSIGNED_BYTE, GL_RGBA);
			break;

			case IFX_24BPP_RGB:
				m_texture->requestTextureRefresh(imageSize, GL_UNSIGNED_BYTE, GL_RGB);
			break;

			case IFX_16BPP_RGB565:
				m_texture->requestTextureRefresh(imageSize, GL_UNSIGNED_SHORT_5_6_5, GL_RGB);
			break;
		}
	}
#endif

	return true;
}

#if defined(LC_PLAT_OGL)
/*-------------------------------------------------------------------------*//**
*/
bool NdhsCMemoryImage::setExternalTexture(GLuint textures[IFX_OGL_MAX_TEXTURES], LcTPixelDim texDim, LcTPixelRect bitmapRect, IFX_UINT32 textureTarget)
{
	if (m_imageType == EImageTypeExternalTexture)
	{
		m_buffer = (void*)textures;
		m_width = bitmapRect.getWidth();
		m_height = bitmapRect.getHeight();
		setActualSize(LcTPixelDim(m_width, m_height));
		m_texture->setExternalTexture(textures, texDim, bitmapRect, textureTarget);
		return true;
	}

	return false;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMemoryImage::setDirty()
{
#if defined(LC_PLAT_OGL)
	if ( (m_imageType == EImageTypeInternalBuffer)
		|| (m_imageType == EImageTypeExternalBuffer) )
	{
		m_texture->setTextureUpdateFlag();
	}
#endif
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMemoryImage::setImageData(LcTByte * data)
{
	if(m_imageType!=EImageTypeInternalBuffer)
		return;

	if(m_data!=NULL)
	{
		m_data.free();
		m_buffer=NULL;
	}
	m_data.attach(data);
	m_buffer=data;
}

/*-------------------------------------------------------------------------*//**
*/
LcTByte* NdhsCMemoryImage::releaseImageData()
{
	if(m_imageType==EImageTypeInternalBuffer && m_data!=NULL)
	{
		m_buffer=NULL;
		return m_data.release();
	}
	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMemoryImage::setBufferData(void *buffer)
{
	switch (m_imageType)
	{
		case EImageTypeInternalBuffer:
		case EImageTypeExternalBuffer:
#if defined(LC_PLAT_OGL)
			m_texture->updateTexture(buffer);
#endif
			m_buffer = buffer;
			setDirty();
		break;

		default:
		break;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCMemoryImage::setPreMultipliedAlpha(bool value)
{
#if defined(LC_PLAT_OGL)
	if (m_texture)
		m_texture->setPreMultipliedAlpha(value);
#endif
}


#endif //IFX_USE_PLUGIN_ELEMENTS
