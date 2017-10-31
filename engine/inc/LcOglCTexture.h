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
#ifndef LcOglCTextureH
#define LcOglCTextureH

#include "inflexionui/engine/inc/LcOglCContext.h"

/*-------------------------------------------------------------------------*//**
	Class that encapsulates an OpenGL texture and provides some
	utility functions
*/
class LcOglCTexture : public LcCBase
{
private:
	LcOglCContext*					m_oglContext;

	// Texture details
	GLuint							m_tex[IFX_OGL_MAX_TEXTURES];
	GLenum							m_type;
	GLenum							m_format;
	GLuint							m_externalTextureTarget;

	bool 							m_isTexturePowerOfTwo;

	GLint							m_wrapMode;

	// Multi-texturing and mip-mapping is only available in OpenGL ES 2.0 mode
#if defined(LC_PLAT_OGL_20)
	GLenum							m_unit;
	bool 							m_mipmap;
#endif

	int								m_arrayStride;
	LcTmAlloc<LcOglTScalar>			m_texCoordsDynamic;

	// Class to store the mesh grid information for a given set of dimensions
	class CMeshGrid : public LcCBase
	{
	protected:
									CMeshGrid()	{}
		void						construct(int gridSizeX, 
											  int gridSizeY,
											  LcTPixelRect bitmapRect,
											  LcTPixelDim textureSize);
	public:
		static LcTaOwner<CMeshGrid>	create(int gridSizeX, 
										   int gridSizeY,
										   LcTPixelRect bitmapRect,
										   LcTPixelDim textureSize);

		LcTmAlloc<LcOglTScalar>		interleavedArrayF;
		LcTmAlloc<LcOglTScalar>		interleavedArrayB;
		LcTmAlloc<GLushort>			indices;

#if defined(LC_USE_OGL_VBOS)
		GLuint						arrayBuffers[2];
		GLuint						elementBufferObject;
#endif

									~CMeshGrid();
	};

	// Cache for the grid information
	LcTmOwnerMap<int, CMeshGrid>	m_gridInfoCache;

	LcTPixelRect					m_bitmapRect;
	LcTPixelDim						m_textureSize;

	bool							m_isExternal;

	bool							m_hasPreMultipliedAlpha;

	// For marshaling texture updates onto UI task...
	// If true, whole texture requires recreating
	bool 							m_pendingTexRefresh;

	bool							m_contextChanged;

	// If true, texture requires refreshing from the buffer
	bool							m_pendingTexUpdate;

	// Cached info required for texture creation
	LcTPixelDim 					m_pendingTexRefreshSize;
	GLenum 							m_pendingTexRefreshType;
	GLenum 							m_pendingTexRefreshFormat;

	// Private helpers
	void			findCorners(
						const LcTScalarRect&	src,
						LcTUnitScalar*			tCorners);

	void			drawTriangles(
						const LcOglTScalar*		pVertices,
						const LcOglTScalar*		pNormals,
						const LcOglTScalar*		pTexCoords,
						const GLushort*			pIndices,
						int						iPolyCount,
						LcTColor				color,
						LcTUnitScalar			opacity,
						bool					antiAlias);

	CMeshGrid* 		getGrid(int gridSizeX, int gridSizeY);

LC_PRIVATE_INTERNAL_PUBLIC:

	// Accessors
	void			setExternalTexture(GLuint textures[IFX_OGL_MAX_TEXTURES], LcTPixelDim texSize, LcTPixelRect bitmapRect, GLuint textureTarget);
	void			setPreMultipliedAlpha(bool hasPreMultipliedAlpha)	{ m_hasPreMultipliedAlpha = hasPreMultipliedAlpha; }
	inline			GLuint			getExternalTextureTarget() { return ((m_isExternal) ? m_externalTextureTarget : 0); }
	inline			GLuint			getTexture()		{ return m_tex[0]; }
	inline			GLuint			getTexture(int index)		{ return (index < IFX_OGL_MAX_TEXTURES) ? m_tex[index] : 0; }
	inline			LcTPixelDim		getPhysicalSize() 	{ return m_textureSize; }

#ifdef LC_OGL_DIRECT
	// Reset the texture size parameters.
					void			resetTexture();
#endif

protected:
									LcOglCTexture(LcOglCContext* ctx);


public:
	static LcTaOwner<LcOglCTexture>	create(LcOglCContext* ctx);

	LC_VIRTUAL						~LcOglCTexture();

	// Create/update texture
	LC_IMPORT		bool			createTexture(LcTPixelDim size, GLenum type, GLenum format);

	// Create compressed texture
	LC_IMPORT		bool			createCompressedTexture(LcTPixelDim size,
															GLenum compressionFormat,
															GLuint levels);

	LC_IMPORT		void			updateTexture(void* pTexData);

	LC_IMPORT		void			updateCompressedTexture(GLint level,
															GLenum compressionFormat,
															GLsizei width,
															GLsizei height,
															GLsizei imageSize,
															void* pCompressedTexData);

	inline			void			setTextureUpdateFlag()	{ m_pendingTexUpdate = true; }
	LC_IMPORT		void			requestTextureRefresh(LcTPixelDim size, GLenum type, GLenum format);
	inline			bool			isCreated()				{ return m_textureSize.width > 0; }

	// Draw the texture
	LC_IMPORT		void			drawRectangle(
										const LcTScalarRect&	src,
										const LcTPlaneRect&		dest,
										LcTColor				color,
										LcTUnitScalar			opacity,
										bool					antiAlias,
										int						meshGridX,
										int						meshGridY);

	// request re-create texture in newcontext
	LC_IMPORT		void			requestContextChange(LcOglCContext* oglContext);


	// Pixel transparency test
	inline			bool			isPointTransparent(int iX, int iY)  { return false; }

	// Power-of-2 test
	inline			bool			isPowerOfTwo(unsigned int value)
	{
		return ((value != 0) && !(value & (value - 1)));
	}

	inline			bool			isPOT() { return m_isTexturePowerOfTwo; }

	LC_IMPORT		void			setWrapMode (int wrapMode);
	inline			int				getWrapMode () { return m_wrapMode; }

	// Multi-texturing and mip-mapping is only available in OpenGL ES 2.0 mode
#if defined(LC_PLAT_OGL_20)
	// Get/Set texture unit
	inline			GLenum			getUnit () 				{ return m_unit; }
	inline			void			setUnit (GLenum unit) 	{ m_unit = unit; }

	// Get/Set texture mip-mapping
	inline			bool			getMipmap () 			{ return m_mipmap; }
	inline			void			setMipmap (bool mipmap) { m_mipmap = mipmap; }
#endif
};

#endif //LcOglCTextureH
