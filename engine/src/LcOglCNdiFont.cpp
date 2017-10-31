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


#if defined(IFX_USE_BITMAPPED_FONTS) && defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcOglCNdiFont> LcOglCNdiFont::create(LcCSpace* sp)
{
	LcTaOwner<LcOglCNdiFont> ref;
	ref.set(new LcOglCNdiFont(sp));
	ref->construct();
	return ref;
}



/*-------------------------------------------------------------------------*//**
*/
void LcOglCNdiFont::releaseResources()
{
	GLuint tex=getTexture();
	if(tex>0)
	{
		glDeleteTextures(1, &tex);
		setTexture(0);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCNdiFont::reloadResources()
{
	open(getFilePath());
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcOglCNdiFont::open(const LcTmString& fileName)
{
	// Load bitmap from NDI file
	LcTaOwner<LcCNdiBitmapFile> bmf = LcCNdiBitmapFile::create();
	if (!bmf->open(fileName, LcCNdiBitmapFile::KFormatFont)
	||   bmf->getSize().width != 256
	||   bmf->getSize().height != 256)
		return false;

	setFilePath(fileName);

	// This sets up width array in base, using checkPixel()
	calculateWidths(bmf->getData());

	// Create texture
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

#if !defined(NDHS_JNI_INTERFACE)

	// For OpenGL ES we can store the font texture in compressed paletted
	// format, which will halve the size but may have an adverse impact on speed

	if(getSpace()->getOglContext()->getGPUCaps()->isExtensionSupported("GL_OES_compressed_paletted_texture"))
	{
		// We require 16 palette entries of RGBA4, and a nybble index per pixel
		// NB: we never bother expanding font to 8888 even if we use 8888 images
		const int dataSize = 16*2 + 256*256/2;
		LcTaAlloc<unsigned char> pNewData(dataSize);

		// Populate the palette with white pixels with each of the 16 alpha values
		int i;
		for (i = 0; i < 16; i++)
			((unsigned short*)(unsigned char*)pNewData)[i] = (unsigned short)(0xFFF0 | i);

		// Iterate 2 pixels (one byte) at a time
		LcTByte* pData8 = bmf->getData();
		for (i = 0; i < 256*256/2; i++)
		{
			// Set output nybbles from input bytes
			pNewData[32+i] = ((pData8[0] & 0xF0) | ((pData8[1] & 0xF0) >> 4));
			pData8 += 2;
		}

		// Create compressed paletted texture from the data
		glCompressedTexImage2D(
			GL_TEXTURE_2D,
			0,
			#if defined(GL_OES_compressed_paletted_texture)
			GL_PALETTE4_RGBA4_OES,
			#else
			0,
			#endif
			256,
			256,
			0,
			dataSize,
			pNewData);
	}
	else

#endif // !defined(NDHS_JNI_INTERFACE)

	// Create 1-component texture with alpha only
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_ALPHA,
		256,
		256,
		0,
		GL_ALPHA,
		GL_UNSIGNED_BYTE,
		bmf->getData());

	// Set texture parameters here
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	setTexture(tex);

	return true;
}

/*-------------------------------------------------------------------------*//**
	Returns true if the pixel is initially non-black.
	Resets the pixel to black before returning.
*/
LC_EXPORT_VIRTUAL bool LcOglCNdiFont::checkPixel(void* pData, int x, int y)
{
	unsigned char* pByte	= (unsigned char*)pData;
	int index				= (y * 256) + x;

	// NB: treat as black if value is less than 1/16 intensity
	bool nonBlack			= (pByte[index] > 0x10);
	pByte[index]			= 0;

	return nonBlack;
}

#endif // #if defined(IFX_USE_BITMAPPED_FONTS) && defined(LC_PLAT_OGL)
