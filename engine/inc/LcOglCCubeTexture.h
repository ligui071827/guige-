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
#ifndef LcOglCCubeTextureH
#define LcOglCCubeTextureH

#include "inflexionui/engine/inc/LcOglCContext.h"

/*-------------------------------------------------------------------------*//**
	Class that encapsulates an OpenGL texture and provides some
	utility functions
*/
class LcOglCCubeTexture : public LcCBase
{
private:
	LcCSpace*						m_space;

	// Texture details
	GLuint							m_tex;
	GLenum							m_type;
	GLenum							m_format;
	GLenum							m_unit;
	bool 							m_mipmap;
	LcTPixelDim						m_sizeBitmap;
	LcTPixelDim						m_sizePhysical;

	// If true, texture requires refreshing from the buffer
	bool							m_pendingTexUpdate;
	
	// Cached info required for texture creation
	LcTPixelDim 					m_pendingTexRefreshSize;
	GLenum 							m_pendingTexRefreshType;
	GLenum 							m_pendingTexRefreshFormat;

LC_PRIVATE_INTERNAL_PUBLIC:

	// Accessors
	inline			GLuint			getTexture()		{ return m_tex; }
	inline			LcTPixelDim		getPhysicalSize() 	{ return m_sizePhysical; }
	
	inline          bool            isValidCubeFaceTarget (GLenum target)
	{
		return ( (target == GL_TEXTURE_CUBE_MAP_POSITIVE_X) ||
	             (target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y) ||
				 (target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z) ||
				 (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X) ||
				 (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y) ||
				 (target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z) );
	}

protected:
	LC_IMPORT						LcOglCCubeTexture()			{}
	LC_IMPORT		void			construct(LcCSpace* space);
	
public:	
	LC_IMPORT static LcTaOwner<LcOglCCubeTexture> create(LcCSpace* space);
	LC_VIRTUAL						~LcOglCCubeTexture();	
	
	// Create uncompressed cube texture
	LC_IMPORT		bool			createTexture(LcTPixelDim size, GLenum type, GLenum format, GLenum unit, bool mipmap = false);
	
	// Create compressed cube texture
	LC_IMPORT		bool			createCompressedTexture(LcTPixelDim size, 
	                                                        GLenum compressionFormat,
															GLuint levels,
															GLenum unit);
	// Update uncompressed cube map data for all 6 faces														
	LC_IMPORT		void			updateTexture(void* pTexDataXPlus, 
												  void* pTexDataXMinus,
												  void* pTexDataYPlus,
												  void* pTexDataYMinus,
												  void* pTexDataZPlus,
												  void* pTexDataZMinus);
	
	// Update uncompressed cube map data for specified target face
	LC_IMPORT		void			updateTextureFace (GLenum target,
	                                                   void* pTexDataXPlus);												  
									
	// Update compressed cube map data for the specified cube face (target)
	LC_IMPORT		void			updateCompressedTexture(GLenum target,
	                                                        GLint level, 
	                                                        GLenum compressionFormat, 
															GLsizei width, 
															GLsizei height, 
															GLsizei imageSize, 
															void* pCompressedTexData);												  
												  
	inline			void			setTextureUpdateFlag() { m_pendingTexUpdate = true; }
	inline			bool			isCreated()			{ return m_sizePhysical.width > 0; }
	
	// Get/Set texture unit
	inline			void			setTextureUnit (GLenum unit)	{ m_unit = unit; }
	inline			GLenum			getTextureUnit ()				{ return m_unit; }
	
	// Get/Set texture mip-mapping
	inline          bool          	getMipmap () 				{	return m_mipmap;	}
	inline          void            setMipmap (bool mipmap) 	{	m_mipmap = mipmap;	}
};

#endif //LcOglCCubeTextureH
