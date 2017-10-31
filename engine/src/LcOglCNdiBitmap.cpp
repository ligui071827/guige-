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
LC_EXPORT LcTaOwner<LcOglCNdiBitmap> LcOglCNdiBitmap::create(LcCSpace* sp)
{
	LcTaOwner<LcOglCNdiBitmap> ref;
	ref.set(new LcOglCNdiBitmap(sp));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcOglCNdiBitmap::open(const LcTmString& sFile)
{	
	// Load bitmap from NDI file
	LcTaOwner<LcCNdiBitmapFile> bmf = LcCNdiBitmapFile::create();
	if (!bmf->open(sFile, LcCNdiBitmapFile::KFormatAny))
		return false;

	setFilePath(sFile);

	LcTPixelDim size = bmf->getSize();

	// Save size
	setActualSize(size);
	setMargins(bmf->getMarginLeft(), bmf->getMarginRight(), bmf->getMarginTop(), bmf->getMarginBottom());
	setFrameCount(bmf->getFrameCount());

	// Determine the type of the loaded image to find out the format.
	m_isTranslucent = true;
	GLenum eglFormat = GL_RGBA;
	if (LcCNdiBitmapFile::KFormatGraphicOpaque == bmf->getFormat())
	{
		eglFormat = GL_RGB;
		m_isTranslucent = false;
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
		m_bIsOpen = m_texture->createTexture(size,	GL_UNSIGNED_BYTE, eglFormat);
	#endif

	if (m_bIsOpen)
	{
		// Create the texture and copy the processed data into it.
		m_texture->updateTexture(bmf->getData());

		if (bmf->getFormat() == LcCNdiBitmapFile::KFormatGraphicTranslucent)
			createTransparencyData(bmf->getData(), size.width * size.height);
	}

	return m_bIsOpen;
}


/*-------------------------------------------------------------------------*//**
*/
void LcOglCNdiBitmap::releaseResources()
{
	m_texture->requestContextChange(getSpace()->getOglContext());
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCNdiBitmap::reloadResources()
{	
	m_texture->requestContextChange(getSpace()->getOglContext());
	open(getFilePath());	
}

#endif // #if defined(LC_PLAT_OGL)
