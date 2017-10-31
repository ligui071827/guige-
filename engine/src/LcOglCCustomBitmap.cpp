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
LC_EXPORT LcTaOwner<LcOglCCustomBitmap> LcOglCCustomBitmap::create(LcCSpace* sp)
{
	LcTaOwner<LcOglCCustomBitmap> ref;
	ref.set(new LcOglCCustomBitmap(sp));
	ref->construct();
	return ref;
}



/*-------------------------------------------------------------------------*//**
*/
void LcOglCCustomBitmap::releaseResources()
{
	m_texture->requestContextChange(getSpace()->getOglContext());
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCCustomBitmap::reloadResources()
{	
	m_texture->requestContextChange(getSpace()->getOglContext());
	open(getFilePath()
			,getMarginLeft()
			,getMarginRight()
			,getMarginTop()
			,getMarginBottom()
			,getFrameCount());	
}


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcOglCCustomBitmap::open(const LcTmString& sFile,
													int marginLeft,
													int marginRight,
													int marginTop,
													int marginBottom,
													int frameCount)
{
	// Load bitmap from NDI file
	LcTaOwner<LcCCustomBitmapFile> bmf = LcCCustomBitmapFile::create();
	if (!bmf->open(sFile))
		return false;

	setFilePath(sFile);

	LcTPixelDim size = bmf->getSize();
	bool bIsCompressed = false;
	
	// Save size
	setActualSize(size);
	setMargins(marginLeft, marginRight, marginTop, marginBottom);
	setFrameCount(frameCount);

	// Determine the type of the loaded image to find out the format.
	GLenum eglFormat = GL_RGBA;
	
	// Decide texture format
	switch (bmf->getFormat())
	{
		case LcCCustomBitmapFile::KFormatGraphicTranslucent:
		{
			eglFormat = GL_RGBA;
			bIsCompressed = false;
		}
		break;
		case LcCCustomBitmapFile::KFormatGraphicOpaque:
		{
			eglFormat = GL_RGB;
			bIsCompressed = false;
		}
		break;
	
		case LcCCustomBitmapFile::KFormatCompressedOpenGL:
		{
			eglFormat = bmf->getOGLCompressionFormat();
			bIsCompressed = true;
		}
		break;
		default:
		break;
	}
		
	// Multi-texturing and mip-mapping is only available in OpenGL ES 2.0 mode
#if defined(LC_PLAT_OGL_20)
	m_texture->setUnit (getTextureUnit());
	m_texture->setMipmap (getTextureMipmap());
#endif
	
	// If our screen depth is only 12-bit then avoid wasting memory
	#ifdef LC_OGL_ONLY_4K_COLORS
		m_bIsOpen = m_texture->createTexture(size,	GL_UNSIGNED_SHORT_4_4_4_4, eglFormat);
	#else
	if (!bIsCompressed)
		m_bIsOpen = m_texture->createTexture(size,	GL_UNSIGNED_BYTE, eglFormat);
	else
		m_bIsOpen = m_texture->createCompressedTexture(size, eglFormat, bmf->getLevelCount());
	#endif

	if (m_bIsOpen)
	{
		LcTByte *image_data = NULL;
		IFX_UINT32 dataSize = 0;
			
		// Create the texture and copy the processed data into it.
		if(eglFormat == GL_RGB || eglFormat == GL_RGBA)
		{	
			if(bmf->readData((void**)&image_data, 0, &dataSize, false) && (dataSize != 0))
			{
				// Send uncompressed data to OpenGL pipeline
				m_texture->updateTexture(image_data);
				
				// only create transparency data when Alpha channel is available.
				if(eglFormat == GL_RGBA)
					createTransparencyData(image_data, size.width * size.height);
				
				// Free resources
				bmf->releaseData(image_data);
			}
		}
		else // compressed texture
		{
			int width = size.width;
			int height = size.height;
			
			// Iterate between first and last level (limits inclusive)
			for (unsigned int i = bmf->getLevelFirst(); i <= bmf->getLevelLast(); i++)
			{
				if(bmf->readData((void**)&image_data, i, &dataSize, false) && (dataSize != 0))
				{
					// Send compressed data to OpenGL pipeline
					m_texture->updateCompressedTexture(i, 			//level 
	                                                   eglFormat, 	// compression format 
													   width, 
													   height, 
													   dataSize, 
													   image_data);
					// Free resources
					bmf->releaseData(image_data);
					
					// Setup for next mip-map level
					image_data = NULL;
					dataSize = 0;
					
					width  = max (width >> 1, 1);
					height = max (height >> 1, 1);
				}
			}
		}
		m_isTranslucent = bmf->isTranslucent();
		
		// close the bitmap file
		bmf->close();
	}

	return m_bIsOpen;
}

#endif // #if defined(LC_PLAT_OGL)
