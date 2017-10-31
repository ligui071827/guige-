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


#if defined(LC_PLAT_OGL_20)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcOglCCubeTexture> LcOglCCubeTexture::create(LcCSpace* space)
{
	LcTaOwner<LcOglCCubeTexture> ref;
	ref.set(new LcOglCCubeTexture);
	ref->construct(space);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCCubeTexture::construct(LcCSpace* space)
{
	m_space = space;
	m_mipmap = false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcOglCCubeTexture::~LcOglCCubeTexture()
{
	if (m_sizePhysical.width > 0)
		glDeleteTextures(1, &m_tex);
}

/*-------------------------------------------------------------------------*//**
	NB! This must not be called in any other context than the UI task, or
	the gl calls will fail.
*/
LC_EXPORT bool LcOglCCubeTexture::createTexture(LcTPixelDim size, GLenum type, GLenum format, GLenum unit, bool mipmap)
{
	// Determine texture size to fit
	int reqWidth	= 32;
	int reqHeight	= 32;
	
	while (reqWidth < size.width)
		reqWidth <<= 1;
	while (reqHeight < size.height)
		reqHeight <<= 1;

	m_sizeBitmap = size;

	// Note that sub image will need to be redrawn
	m_pendingTexUpdate = true;

	// If texture is already created...
	if (m_sizePhysical.width > 0)
	{
		// ...check whether the size, type and format enable it to be reused
		if (m_format == format
		&&	m_type == type
		&&	m_sizePhysical.width >= reqWidth
		&&	m_sizePhysical.height >= reqHeight)
			return true;

		// Delete old texture if cannot be reused
		glDeleteTextures(1, &m_tex);
	}

	// Save info
	m_sizePhysical.width	= reqWidth;
	m_sizePhysical.height	= reqHeight;
	m_format				= format;
	m_type					= type;
	m_unit                  = unit;
	m_mipmap                = mipmap;

	// Allocate and clear texture memory - assume maximum size is 32bpp
	unsigned* pInitData = LcTmAlloc<unsigned>::allocUnsafe(m_sizePhysical.width * m_sizePhysical.height);

	if (pInitData == NULL)
	{
		return false;
	}

	// Memset all face images with zero
	lc_memset(pInitData, 0, m_sizePhysical.width * m_sizePhysical.height * sizeof(unsigned));

	// IMPORTANT: DO NOT add anything after this point that may leave,
	// because then the above memory will not be freed.

	glEnable (GL_TEXTURE_CUBE_MAP);
	m_tex = 0;

	// Create texture and attach data
	glGenTextures(1, &m_tex);

	// Set the specified unit as the active texture unit
	glActiveTexture(unit);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);

	// Positive X
	glTexImage2D(
		GL_TEXTURE_CUBE_MAP_POSITIVE_X,
		0,
		m_format,
		m_sizePhysical.width,
		m_sizePhysical.height,
		0,
		m_format,
		m_type,
		pInitData);

	// Negative X
	glTexImage2D(
		GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
		0,
		m_format,
		m_sizePhysical.width,
		m_sizePhysical.height,
		0,
		m_format,
		m_type,
		pInitData);

	// Positive Y
	glTexImage2D(
		GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
		0,
		m_format,
		m_sizePhysical.width,
		m_sizePhysical.height,
		0,
		m_format,
		m_type,
		pInitData);

	// Negative Y
	glTexImage2D(
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
		0,
		m_format,
		m_sizePhysical.width,
		m_sizePhysical.height,
		0,
		m_format,
		m_type,
		pInitData);

	// Positive Z
	glTexImage2D(
		GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
		0,
		m_format,
		m_sizePhysical.width,
		m_sizePhysical.height,
		0,
		m_format,
		m_type,
		pInitData);

	// Negative Z
	glTexImage2D(
		GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
		0,
		m_format,
		m_sizePhysical.width,
		m_sizePhysical.height,
		0,
		m_format,
		m_type,
		pInitData);

	LcTmAlloc<unsigned>::freeUnsafe(pInitData);

	// Set mapping parameters for new cubemap texture
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);

	return true;
}

// Create compressed cube texture
LC_EXPORT bool LcOglCCubeTexture::createCompressedTexture(LcTPixelDim size,
	                                   					  GLenum compressionFormat,
									   					  GLuint levels,
									   					  GLenum unit)
{
	// Determine texture size to fit
	int reqWidth	= 32;
	int reqHeight	= 32;

	while (reqWidth < size.width)
		reqWidth <<= 1;
	while (reqHeight < size.height)
		reqHeight <<= 1;

	m_sizeBitmap = size;

	// Note that sub image will need to be redrawn
	m_pendingTexUpdate = true;

	// Save info
	m_sizePhysical.width	= reqWidth;
	m_sizePhysical.height	= reqHeight;
	m_format				= compressionFormat;
	m_type					= 0;      // no meaning for compressed textures
	m_unit                  = unit;
	m_mipmap                = false;  // runtime mip-mapping not applicable to compressed case

	glEnable (GL_TEXTURE_CUBE_MAP);
	m_tex = 0;

	// Create texture and attach data
	glGenTextures(1, &m_tex);

	// Set the specified unit as the active texture unit
	glActiveTexture(unit);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);

	// Set mapping parameters for new cubemap texture
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Take care of pre mip-mapping - set appropriate minification filter (only if we have levels
	// in addition to the base level!)
	if (levels > 1)
		LC_OGL_IX(glTexParameter)(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		LC_OGL_IX(glTexParameter)(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	LC_OGL_IX(glTexParameter)(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP);
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP);

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCCubeTexture::updateTexture(void* pTexDataXPlus,
												void* pTexDataXMinus,
												void* pTexDataYPlus,
												void* pTexDataYMinus,
												void* pTexDataZPlus,
												void* pTexDataZMinus)
{
	if (m_pendingTexUpdate)
	{
		// Set Texture's Unit as active texture unit
		glActiveTexture(m_unit);

		// Copy the RGBA data into the current texture
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);

		if (pTexDataXPlus)
		{
			// Positive X
			glTexSubImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_X,
							0,
							0, 0,
							m_sizeBitmap.width,
							m_sizeBitmap.height,
							m_format,
							m_type,
							pTexDataXPlus);
		}

		if (pTexDataXMinus)
		{
			// Negative X
			glTexSubImage2D(
							GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
							0,
							0, 0,
							m_sizeBitmap.width,
							m_sizeBitmap.height,
							m_format,
							m_type,
							pTexDataXMinus);
		}

		if(pTexDataYPlus)
		{
			// Positive Y
			glTexSubImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
							0,
							0, 0,
							m_sizeBitmap.width,
							m_sizeBitmap.height,
							m_format,
							m_type,
							pTexDataYPlus);
		}

		if (pTexDataYMinus)
		{
			// Negative Y
			glTexSubImage2D(
							GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
							0,
							0, 0,
							m_sizeBitmap.width,
							m_sizeBitmap.height,
							m_format,
							m_type,
							pTexDataYMinus);
		}

		if (pTexDataZPlus)
		{
			// Positive Z
			glTexSubImage2D(
							GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
							0,
							0, 0,
							m_sizeBitmap.width,
							m_sizeBitmap.height,
							m_format,
							m_type,
							pTexDataZPlus);
		}

		if (pTexDataZMinus)
		{
			// Negative Z
			glTexSubImage2D(
							GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
							0,
							0, 0,
							m_sizeBitmap.width,
							m_sizeBitmap.height,
							m_format,
							m_type,
							pTexDataZMinus);
		}

		if (m_mipmap)
		{
			LC_OGL_IX(glTexParameter)(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			glGenerateMipmap (GL_TEXTURE_CUBE_MAP);
		}

		m_pendingTexUpdate = false;
	}
}

/*-------------------------------------------------------------------------*//**
*/
// Update uncompressed cube map data for specified target face
LC_EXPORT void LcOglCCubeTexture::updateTextureFace (GLenum target,
	                              					 void* pTexFaceData)
{
	// Proceed only if we have a valid cube face target
	if (isValidCubeFaceTarget(target) &&
	   (pTexFaceData != NULL))
	{
		// Set Texture's Unit as active texture unit
		glActiveTexture(m_unit);

		// Bind this texture to current texture unit
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);

		// Send data to OpenGL pipeline for this target face
		glTexSubImage2D (target,
						 0,
						 0, 0,
						 m_sizeBitmap.width,
						 m_sizeBitmap.height,
						 m_format,
						 m_type,
						 pTexFaceData);
	}
}
/*-------------------------------------------------------------------------*//**
*/
// Update compressed cube map data for the specified cube face (target)
LC_EXPORT void LcOglCCubeTexture::updateCompressedTexture(GLenum target,
	                                                      GLint level,
	                                                      GLenum compressionFormat,
												  	      GLsizei width,
														  GLsizei height,
														  GLsizei imageSize,
														  void* pCompressedTexData)
{
	// Proceed only if we have a valid cube face target
	if (isValidCubeFaceTarget(target))
	{
		// Set Texture's Unit as active texture unit
		glActiveTexture(m_unit);

		// Copy the RGBA data into the current texture
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);

		glCompressedTexImage2D (target, // cube map face
                            	level,
								compressionFormat,
								width,
								height,
                            	0,
                            	imageSize,
                            	pCompressedTexData);
	}
}

#endif // #if defined(LC_PLAT_OGL_20)
