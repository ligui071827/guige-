/***************************************************************************
*
*				Copyright 2006 Mentor Graphics Corporation
*						  All Rights Reserved.
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

// Array of Vertex (x,y,z), Normal (x, y, z), UV (u, v) and VPOS(x, y)
const int GRID_ARRAY_STRIDE = (3 + 3 + 2 + 1 + 1);

// Convert x, y to a single int
#define COMPOSE_GRID_ID(x, y)	(x << 16 | y)

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<LcOglCTexture::CMeshGrid> LcOglCTexture::CMeshGrid::create(int gridSizeX, 
															 int gridSizeY, 
															 LcTPixelRect bitmapRect, 
															 LcTPixelDim textureSize)
{
	LcTaOwner<CMeshGrid> ref;
	ref.set(new CMeshGrid());
	ref->construct(gridSizeX, gridSizeY, bitmapRect, textureSize);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCTexture::CMeshGrid::construct(int gridSizeX, 
									 int gridSizeY,
									 LcTPixelRect bitmapRect,
									 LcTPixelDim textureSize)
{
	int numTriangles = gridSizeX * gridSizeY * 2;
	int numVertices = (gridSizeX + 1) * (gridSizeY + 1);

	LcOglTScalar gridFactorX = 1.0f / gridSizeX;
	LcOglTScalar gridFactorY = 1.0f / gridSizeY;

	// Work out the overall UV coordinates
	LcTUnitScalar corners[4];

	if (textureSize.width < 0)
	{
		corners[1]	= LC_OGL_FROM_UNIT_SCALAR(LC_DIV_TO_UNIT(bitmapRect.getLeft(), -textureSize.width));
		corners[0]	= LC_OGL_FROM_UNIT_SCALAR(LC_DIV_TO_UNIT(bitmapRect.getRight(), -textureSize.width));
	}
	else
	{
		corners[0]	= LC_OGL_FROM_UNIT_SCALAR(LC_DIV_TO_UNIT(bitmapRect.getLeft(), textureSize.width));
		corners[1]	= LC_OGL_FROM_UNIT_SCALAR(LC_DIV_TO_UNIT(bitmapRect.getRight(), textureSize.width));
	}

	if (textureSize.height < 0)
	{
		corners[3]	= LC_OGL_FROM_UNIT_SCALAR(LC_DIV_TO_UNIT(bitmapRect.getBottom(), -textureSize.height));
		corners[2]	= LC_OGL_FROM_UNIT_SCALAR(LC_DIV_TO_UNIT(bitmapRect.getTop(), -textureSize.height));
	}
	else
	{
		corners[2]	= LC_OGL_FROM_UNIT_SCALAR(LC_DIV_TO_UNIT(bitmapRect.getBottom(), textureSize.height));
		corners[3]	= LC_OGL_FROM_UNIT_SCALAR(LC_DIV_TO_UNIT(bitmapRect.getTop(), textureSize.height));
	}

	// Allocate the data blocks
	interleavedArrayF.alloc(numVertices * GRID_ARRAY_STRIDE);
	interleavedArrayB.alloc(numVertices * GRID_ARRAY_STRIDE);
	indices.alloc(numTriangles * 3);

	// Clear the data
	memset(interleavedArrayF, 0, numVertices * GRID_ARRAY_STRIDE * sizeof(LcOglTScalar));
	memset(interleavedArrayB, 0, numVertices * GRID_ARRAY_STRIDE * sizeof(LcOglTScalar));

	// Set up the pointers
	LcOglTScalar* ptr = interleavedArrayF;

	float vposxGrid = 1.0f/gridSizeX;
	float vposyGrid = 1.0f/gridSizeY;

	// Work out the Vertex and UV data
	for (int y = 0; y <= gridSizeY; ++y)
	{
		for (int x = 0; x <= gridSizeX; ++x)
		{
			// Vertex (x,y,z)
			ptr[0] = LC_OGL_FROM_UNIT_SCALAR((x * gridFactorX) - 0.5);
			ptr[1] = LC_OGL_FROM_UNIT_SCALAR(0.5 - (y * gridFactorY));

			// UV (u, v)
			ptr[6] = LC_OGL_FROM_UNIT_SCALAR(((gridSizeX - x) * gridFactorX * corners[0]) + (x * gridFactorX * corners[1]));
			ptr[7] = LC_OGL_FROM_UNIT_SCALAR(((gridSizeY - y) * gridFactorY * corners[3]) + (y * gridFactorY * corners[2]));

			ptr[8] = vposxGrid * x;
			ptr[9] = vposyGrid * y;

			ptr += GRID_ARRAY_STRIDE;
		}
	}

	// The Vertex and UV data is the same between these arrays.
	memcpy(interleavedArrayB, interleavedArrayF, numVertices * GRID_ARRAY_STRIDE * sizeof(LcOglTScalar));

	LcOglTScalar* nf = interleavedArrayF;
	LcOglTScalar* nb = interleavedArrayB;

	// Now update the normals - always (0, +/- 1, 0)
	for (int y = 0; y <= gridSizeY; ++y)
	{
		for (int x = 0; x <= gridSizeX; ++x)
		{
			// Front facing Normal (x, y, z)
			nf[5] = LC_OGL_FROM_UNIT_SCALAR(1.0);
			nf += GRID_ARRAY_STRIDE;

			// Back facing Normal (x, y, z)
			nb[5] = LC_OGL_FROM_UNIT_SCALAR(-1.0);
			nb += GRID_ARRAY_STRIDE;
		}
	}

	GLushort* in = indices;

	// Work out the index data
	for (int y = 1; y <= gridSizeY; ++y)
	{
		for (int x = 0; x < gridSizeX; ++x)
		{
			*in++ = (y * (gridSizeX + 1)) + x;
			*in++ = ((y-1) * (gridSizeX + 1)) + x;
			*in++ = ((y-1) * (gridSizeX + 1)) + x + 1;

			*in++ = (y * (gridSizeX + 1)) + x;
			*in++ = ((y-1) * (gridSizeX + 1)) + x + 1;
			*in++ = (y * (gridSizeX + 1)) + x + 1;
		}
	}


#ifdef LC_USE_OGL_VBOS
	int numElements = gridSizeX * gridSizeY * 2 * 3;

	// Create the buffer object
	glGenBuffers(2, arrayBuffers);

	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, numVertices * GRID_ARRAY_STRIDE * sizeof(LcOglTScalar), interleavedArrayF, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, arrayBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, numVertices * GRID_ARRAY_STRIDE * sizeof(LcOglTScalar), interleavedArrayB, GL_STATIC_DRAW);

	// Create the buffer object
	glGenBuffers(1, &elementBufferObject);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numElements * sizeof(GLushort), indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LcOglCTexture::CMeshGrid::~CMeshGrid()
{
#ifdef LC_USE_OGL_VBOS
	if (arrayBuffers[0])
		glDeleteBuffers(2, arrayBuffers);

	if (elementBufferObject)
		glDeleteBuffers(1, &elementBufferObject);
#endif
	if (interleavedArrayF)
		interleavedArrayF.free();
	if (interleavedArrayB)
		interleavedArrayB.free();
	if (indices)
		indices.free();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcOglCTexture::~LcOglCTexture()
{
	if (!m_isExternal && m_textureSize.width > 0)
	{
		glDeleteTextures(1, m_tex);
		m_tex[0] = 0;
		
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
	if (m_format == GL_ALPHA)
		m_oglContext->getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, -m_textureSize.width * m_textureSize.height);
	else
		m_oglContext->getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, -m_textureSize.width * m_textureSize.height * 4);
#endif
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<LcOglCTexture> LcOglCTexture::create(LcOglCContext *ctx)
{
	LcTaOwner<LcOglCTexture> ref;
	ref.set(new LcOglCTexture(ctx));
	return ref;
}

LcOglCTexture::LcOglCTexture(LcOglCContext* ctx)
:	m_oglContext(ctx)
#if defined(LC_PLAT_OGL_20)
	,
	m_unit(GL_TEXTURE0)
#endif
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCTexture::requestTextureRefresh(LcTPixelDim size, GLenum type, GLenum format)
{
	m_pendingTexRefresh = true;
	m_pendingTexRefreshSize = size;
	m_pendingTexRefreshType = type;
	m_pendingTexRefreshFormat = format;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCTexture::requestContextChange(LcOglCContext* oglContext)
{
	m_oglContext = oglContext;
	m_contextChanged = true;

	if(m_tex[0] > 0)
	{
		glDeleteTextures(1, m_tex);
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
	if (m_format == GL_ALPHA)
		m_oglContext->getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, -m_textureSize.width * m_textureSize.height);
	else
		m_oglContext->getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, -m_textureSize.width * m_textureSize.height * 4);
#endif
		m_tex[0] = 0;
	}
}

/*-------------------------------------------------------------------------*//**
	NB! This must not be called in any other context than the UI task, or
	the gl calls will fail.
*/
LC_EXPORT bool LcOglCTexture::createTexture(LcTPixelDim size, GLenum type, GLenum format)
{
	// Determine texture size to fit
	int reqWidth	= 32;
	int reqHeight	= 32;
	bool isPaddedPOT = false;

	// Grid info cache is now invalid - clear it out
	m_gridInfoCache.clear();

	m_isTexturePowerOfTwo = isPowerOfTwo (size.width) && isPowerOfTwo (size.height);

	if (m_isTexturePowerOfTwo)
	{
		// Use the dimensions as it is
		reqWidth = size.width;
		reqHeight = size.height;
	}
	else
	{
		// Ceil dimensions to nearest power-of-2
		while (reqWidth < size.width)
			reqWidth <<= 1;

		while (reqHeight < size.height)
			reqHeight <<= 1;

		isPaddedPOT = true;

#if defined(LC_PLAT_OGL_20)
		// Checking threshold for RAM saving
		if(((100 *((reqWidth * reqHeight) - (size.width * size.height)))/(reqWidth * reqHeight)) >= LC_TEXTURE_NPOT_THRESHOLD)
		{
			// NPOT saving is sufficient to justify its usage
			reqWidth  = size.width;
			reqHeight = size.height;

			// Mark it as NPOT
			isPaddedPOT = false;
		}
#endif
	}

	m_bitmapRect.setLeft(0);
	m_bitmapRect.setRight(size.width);
	m_bitmapRect.setTop(0);
	m_bitmapRect.setBottom(size.height);

	// Note that sub image will need to be redrawn
	m_pendingTexUpdate = true;

	// If texture is already created...
	// will not remove old texture id if context is changed
	if (!m_contextChanged && m_textureSize.width > 0)
	{
		// ...check whether the size, type and format enable it to be reused
		if (m_format == format
		&&	m_type == type
		&&	m_textureSize.width >= reqWidth
		&&	m_textureSize.height >= reqHeight)
			return true;

		// Delete old texture if cannot be reused
		glDeleteTextures(1, m_tex);
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
	if (m_format == GL_ALPHA)
		m_oglContext->getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, -m_textureSize.width * m_textureSize.height);
	else
		m_oglContext->getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, -m_textureSize.width * m_textureSize.height * 4);
#endif
	}

	// Save info
	m_textureSize.width		= reqWidth;
	m_textureSize.height	= reqHeight;
	m_format				= format;
	m_type					= type;

#if defined(LC_PLAT_OGL_20)
	// Set the specified unit as the active texture unit
	glActiveTexture(m_unit);
#endif

	// Create texture and attach data
	glGenTextures(1, m_tex);
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
	if (m_format == GL_ALPHA)
		m_oglContext->getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, m_textureSize.width * m_textureSize.height);
	else
		m_oglContext->getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, m_textureSize.width * m_textureSize.height * 4);
#endif

	glBindTexture(GL_TEXTURE_2D, m_tex[0]);

	unsigned* pInitData = NULL;

	// Allocate and clear texture memory only if texture is Padded POT - assume maximum size is 32bpp
	if (isPaddedPOT)
	{
		pInitData = LcTmAlloc<unsigned>::allocUnsafe(m_textureSize.width * m_textureSize.height);
		if (pInitData == NULL)
		{
			return false;
		}
		lc_memset(pInitData, 0, m_textureSize.width * m_textureSize.height * sizeof(unsigned));
	}

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		m_format,
		m_textureSize.width,
		m_textureSize.height,
		0,
		m_format,
		m_type,
		pInitData);

	// Deallocate the buffer if it was allocated previously for padded POT
	if (pInitData && isPaddedPOT)
		LcTmAlloc<unsigned>::freeUnsafe(pInitData);

	// Set mapping parameters for new texture
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Default wrapping mode is GL_CLAMP_TO_EDGE, appropriate wrapping mode will
	// be set during the draw call
	m_wrapMode = GL_CLAMP;

	LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapMode);
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapMode);

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcOglCTexture::createCompressedTexture(LcTPixelDim size,
													  GLenum compressionFormat,
													  GLuint levels)
{
	// Determine texture size to fit
	int reqWidth	= 32;
	int reqHeight	= 32;

	// Grid info cache is now invalid - clear it out
	m_gridInfoCache.clear();

	m_isTexturePowerOfTwo = isPowerOfTwo (size.width) && isPowerOfTwo (size.height);

	reqWidth = size.width;
	reqHeight = size.height;

	m_bitmapRect.setLeft(0);
	m_bitmapRect.setRight(size.width);
	m_bitmapRect.setTop(0);
	m_bitmapRect.setBottom(size.height);

	// Note that sub image will need to be redrawn
	m_pendingTexUpdate = true;

	// Save info
	m_textureSize.width		= reqWidth;
	m_textureSize.height	= reqHeight;
	m_format				= compressionFormat;
	m_type					= 0;

#if defined(LC_PLAT_OGL_20)
	// Set the specified unit as the active texture unit
	glActiveTexture(m_unit);
#endif

	// Create texture and attach data
	glGenTextures(1, m_tex);

	glBindTexture(GL_TEXTURE_2D, m_tex[0]);

	// Set mapping parameters for new texture
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (levels > 1)
		LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	else
		LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Default wrapping mode is GL_CLAMP_TO_EDGE, appropriate wrapping mode will
	// be set during the draw call
	m_wrapMode = GL_CLAMP;

	LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_wrapMode);
	LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_wrapMode);

	return true;
}
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCTexture::updateTexture(void* pTexData)
{
	if (m_pendingTexRefresh)
	{
		(void)createTexture(m_pendingTexRefreshSize, m_pendingTexRefreshType, m_pendingTexRefreshFormat);
		m_pendingTexRefresh = false;
	}

	if (m_pendingTexUpdate)
	{

#if defined(LC_PLAT_OGL_20)
		glActiveTexture(m_unit);
#endif

		// Copy the RGBA data into the current texture
		glBindTexture(GL_TEXTURE_2D, m_tex[0]);

		glTexSubImage2D(
			GL_TEXTURE_2D,
			0,
			m_bitmapRect.getLeft(),
			m_bitmapRect.getTop(),
			m_bitmapRect.getWidth(),
			m_bitmapRect.getHeight(),
			m_format,
			m_type,
			pTexData);

#if defined(LC_PLAT_OGL_20)
		if (m_mipmap)
		{
			LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			glGenerateMipmap (GL_TEXTURE_2D);
		}
#endif

		m_pendingTexUpdate = false;
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCTexture::updateCompressedTexture(GLint level,
													  GLenum compressionFormat,
													  GLsizei width,
													  GLsizei height,
													  GLsizei imageSize,
													  void* pCompressedTexData)
{
#if defined(LC_PLAT_OGL_20)
	glActiveTexture(m_unit);
#endif

	// Bind it as current texture
	glBindTexture(GL_TEXTURE_2D, m_tex[0]);

#if !defined(NDHS_JNI_INTERFACE)

	glCompressedTexImage2D (GL_TEXTURE_2D, // target
							level,
							compressionFormat,
							width,
							height,
							0,
							imageSize,
							pCompressedTexData);

#endif // !defined(NDHS_JNI_INTERFACE)
}

/*-------------------------------------------------------------------------*//**
*/
LcOglCTexture::CMeshGrid* LcOglCTexture::getGrid(int gridSizeX, int gridSizeY)
{
	CMeshGrid* retVal = NULL;
	int gridID = COMPOSE_GRID_ID(gridSizeX, gridSizeY);
	LcTmOwnerMap<int, CMeshGrid>::iterator gridIt = m_gridInfoCache.find(gridID);

	if (gridIt == m_gridInfoCache.end())
	{
		// Not found in the cache - create new grid
		LcTaOwner<CMeshGrid> newGrid = CMeshGrid::create(gridSizeX, gridSizeY, m_bitmapRect, m_textureSize);
		retVal = newGrid.ptr();
		m_gridInfoCache.add_element(gridID, newGrid);
	}
	else
	{
		retVal = gridIt->second;
	}

	return retVal;
}

#ifdef LC_OGL_DIRECT
/*-------------------------------------------------------------------------*//**
	Reset the texture size so that it is re-created without deleting.
	Be very careful when using this, as an existing texture will not be
	deleted. This will cause a memory leak on most platforms.
	This is used by the PPC PowerVR configuration that deletes the
	contexts / textures when entering suspend.
*/
void LcOglCTexture::resetTexture()
{
	// Grid info cache is now invalid - clear it out
	m_gridInfoCache.clear();

	m_bitmapRect.setLeft(0);
	m_bitmapRect.setRight(0);
	m_bitmapRect.setTop(0);
	m_bitmapRect.setBottom(0);
	m_textureSize.width		= 0;
	m_textureSize.height	= 0;
}
#endif

/*-------------------------------------------------------------------------*//**
	Return corners corresponding to src, in LRBT order
*/
void LcOglCTexture::findCorners(
	const LcTScalarRect&	src,
	LcTUnitScalar*			tCorners)
{
	// Texture edges (L, R)
	if (m_textureSize.width >= 0)
	{
		tCorners[0]	= LC_DIV_TO_UNIT(m_bitmapRect.getLeft() + src.getLeft(), m_textureSize.width);
		tCorners[1]	= LC_DIV_TO_UNIT(m_bitmapRect.getLeft() + src.getRight(), m_textureSize.width);
	}
	else
	{
		tCorners[1]	= LC_DIV_TO_UNIT(m_bitmapRect.getLeft() + src.getLeft(), -m_textureSize.width);
		tCorners[0]	= LC_DIV_TO_UNIT(m_bitmapRect.getLeft() + src.getRight(), -m_textureSize.width);
	}

	// Calc texture coords (B, T) relative to top edge, as LcTScalarRect has TL-origin
	if (m_textureSize.height >= 0)
	{
		tCorners[3]	= LC_DIV_TO_UNIT(m_bitmapRect.getTop() + src.getTop(), m_textureSize.height);
		tCorners[2]	= LC_DIV_TO_UNIT(m_bitmapRect.getTop() + src.getBottom(), m_textureSize.height);
	}
	else
	{
		tCorners[2]	= LC_DIV_TO_UNIT(m_bitmapRect.getTop() + src.getTop(), -m_textureSize.height);
		tCorners[3]	= LC_DIV_TO_UNIT(m_bitmapRect.getTop() + src.getBottom(), -m_textureSize.height);
	}
}

/*-------------------------------------------------------------------------*//**
	Draw the given 4 vertices (row major, bottom up) mapping the texture
	section specified by src (in pixels) on to them.  The mapping is
	generated on the fly which is slower, but using this method is simpler
	for a basic rectangular draw such as bitmap drawRegion()
*/
LC_EXPORT void LcOglCTexture::drawRectangle(
	const LcTScalarRect&	src,
	const LcTPlaneRect&		dest,
	LcTColor				color,
	LcTUnitScalar			opacity,
	bool					antiAlias,
	int						meshGridX,
	int						meshGridY)
{
	if (!m_isExternal && (m_textureSize.width <= 0 || opacity < LC_TRANSPARENT_THRESHOLD))
		return;

#if !defined(LC_PLAT_OGL_20)
	// Mesh grids are only supported in OpenGL2
	meshGridX = 1;
	meshGridY = 1;
#endif

	CMeshGrid* grid = getGrid(meshGridX, meshGridY);
	LC_ASSERT(grid);

	// If grid hasn't been found, something seriously wrong - bailing
	if (!grid)
		return;

	LcTTransform xfm = LcTTransform::identity();
	xfm.translate(dest.getLeft() + (dest.getWidth() / 2.0f), dest.getBottom() + (dest.getHeight() / 2.0f), dest.getZDepth());
	xfm.scale(dest.getWidth(), dest.getHeight(), dest.getWidth());

	m_oglContext->getSpace()->transformsChanged(xfm);

	GLushort* pIndices = grid->indices;
	LcOglTScalar* pVertices;
	LcOglTScalar* pTexCoords;
	LcOglTScalar* pNormals;

	if (m_oglContext->getSpace()->isEyeFacing())
	{
		pVertices = &grid->interleavedArrayF[0];
		pNormals = &grid->interleavedArrayF[3];
		pTexCoords = &grid->interleavedArrayF[3 + 3];
	}
	else
	{
		pVertices = &grid->interleavedArrayB[0];
		pNormals = &grid->interleavedArrayB[3];
		pTexCoords = &grid->interleavedArrayB[3 + 3];
	}

	if (!(src.getTop() == m_bitmapRect.getTop() && src.getBottom() == m_bitmapRect.getBottom() &&
		src.getLeft() == m_bitmapRect.getLeft() && src.getRight() == m_bitmapRect.getRight()))
	{
		// We must dynamically generate the UV coords when only part of the source is being rendered.
		int numVertices = (meshGridX + 1) * (meshGridY + 1);
		LcOglTScalar gridFactorX = 1.0f / meshGridX;
		LcOglTScalar gridFactorY = 1.0f / meshGridY;

		LcTUnitScalar corners[4];
		findCorners(src, corners);

		if (!m_texCoordsDynamic)
		{
			m_texCoordsDynamic.alloc(numVertices * GRID_ARRAY_STRIDE);
		}
		else
		{
			// Allow the array to grow to the largest size needed
			if (m_texCoordsDynamic.count() < numVertices * GRID_ARRAY_STRIDE)
			{
				m_texCoordsDynamic.free();
				m_texCoordsDynamic.alloc(numVertices * GRID_ARRAY_STRIDE);
			}
		}

		// We need to create a dynamic set of UV coordinates.
		LcOglTScalar* ptr = m_texCoordsDynamic;
		for (int y = 0; y <= meshGridY; ++y)
		{
			for (int x = 0; x <= meshGridX; ++x)
			{
				// UV (u, v)
				ptr[6] = LC_OGL_FROM_UNIT_SCALAR(((meshGridX - x) * gridFactorX * corners[0]) + (x * meshGridX * corners[1]));
				ptr[7] = LC_OGL_FROM_UNIT_SCALAR(((meshGridY - y) * gridFactorY * corners[3]) + (y * meshGridY * corners[2]));
				ptr += GRID_ARRAY_STRIDE;
			}
		}

		pTexCoords = &m_texCoordsDynamic[3 + 3];
	}

#ifdef LC_USE_OGL_VBOS

	// We can only use VBOs if the whole texture is being drawn
	else
	{
		// Set up arrays
		if (m_oglContext->getSpace()->isEyeFacing())
			glBindBuffer(GL_ARRAY_BUFFER, grid->arrayBuffers[0]);
		else
			glBindBuffer(GL_ARRAY_BUFFER, grid->arrayBuffers[1]);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, grid->elementBufferObject);

		// These are the offsets into the VBO, not absoloute addresses.
		pVertices = (LcOglTScalar*) (0 * sizeof(LcOglTScalar));
		pNormals = (LcOglTScalar*) (3 * sizeof(LcOglTScalar));
		pTexCoords = (LcOglTScalar*) (6 * sizeof(LcOglTScalar));
		pIndices = (GLushort*)0;
	}

#endif

	// Now draw it
	drawTriangles(
		pVertices,
		pNormals,
		pTexCoords,
		pIndices,
		meshGridX * meshGridY * 2,
		color,
		opacity,
		antiAlias);

#ifdef LC_USE_OGL_VBOS
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCTexture::drawTriangles(
	const LcOglTScalar*		pVertices,
	const LcOglTScalar*		pNormals,
	const LcOglTScalar*		pTexCoords,
	const GLushort*			pIndices,
	int						iPolyCount,
	LcTColor				color,
	LcTUnitScalar			opacity,
	bool					antiAlias)
{
	if (!m_isExternal && (m_textureSize.width <= 0 || opacity < LC_TRANSPARENT_THRESHOLD))
		return;

	IFX_DISPLAY* display = m_oglContext->getSpace()->getDisplay();

#if defined(LC_PLAT_OGL_20)

	LcOglCGlobalState* globalState = m_oglContext->getGlobalState();
	LcOglTScalar pTangents[3 * 4];
	LcOglTScalar pBiTangents[3 * 4];
	LcOglTScalar pVertexPosX[4];
	LcOglTScalar pVertexPosY[4];
	LcOglTScalar value = m_oglContext->getSpace()->isEyeFacing() ? 1.0f : -1.0f;

	// Texture with Lighting enabled
	if (globalState->getGlobalLightingStatus() == true)
	{
		m_oglContext->switchEffect((m_format == GL_ALPHA) ? ALPHALIGHT00_EFFECT_INDEX : TEXLIGHT00_EFFECT_INDEX,
								   m_oglContext->getSpace()->getPaintWidget());
	}
	else
	{
		// Texture without lighting - quality is always assumed to be "NORMAL"
		bool bHighQuality = false;
		m_oglContext->switchEffect(BACKGROUND_EFFECT_INDEX, bHighQuality);
	}

	LcOglCEffect* currentEffect = m_oglContext->getCurrentEffect();
#endif

	m_oglContext->setCullFace(GL_FALSE);

#if defined (LC_PLAT_OGL_20)

	if (currentEffect != NULL)
	{
		// Get attribute location for non-fixed attributes
		GLint iLocPosition	= currentEffect->getEnumeratedLocation(IFX_VERTEX_COORDS);
		GLint iLocTexture	= currentEffect->getEnumeratedLocation(IFX_TEXTURE_COORDS);
		GLint iLocNormal	= currentEffect->getEnumeratedLocation(IFX_NORMAL_COORDS);
		GLint iLocTangent	= currentEffect->getEnumeratedLocation(IFX_TANGENT_COORDS);
		GLint iLocBiTangent = currentEffect->getEnumeratedLocation(IFX_BINORMAL_COORDS);
		GLint iLocVertexPosX = currentEffect->getEnumeratedLocation(IFX_VERTEX_POSX_PLANE);
		GLint iLocVertexPosY = currentEffect->getEnumeratedLocation(IFX_VERTEX_POSY_PLANE);

		// Load vertex coodinates only if valid attribute index exists
		if (iLocPosition >= 0)
			glVertexAttribPointer (iLocPosition, 3, LC_OGL_SCALAR_TYPE, GL_FALSE, GRID_ARRAY_STRIDE * sizeof(LcOglTScalar), pVertices);

		// Load texture coodinates only if valid attribute index exists
		if (iLocTexture >= 0)
			glVertexAttribPointer (iLocTexture, 2, LC_OGL_SCALAR_TYPE, GL_FALSE, GRID_ARRAY_STRIDE * sizeof(LcOglTScalar), pTexCoords);

		// Load normal coodinates only if valid attribute index exists
		if (iLocNormal >= 0)
			glVertexAttribPointer (iLocNormal, 3, LC_OGL_SCALAR_TYPE, GL_FALSE, GRID_ARRAY_STRIDE * sizeof(LcOglTScalar), pNormals);

		// Load Tangent coordinates only if needed
		if (iLocTangent >= 0)
		{
			pTangents[0] = value;		pTangents[1]  = 0.0;		pTangents[2]  = 0.0;
			pTangents[3] = value;		pTangents[4]  = 0.0;		pTangents[5]  = 0.0;
			pTangents[6] = value;		pTangents[7]  = 0.0;		pTangents[8]  = 0.0;
			pTangents[9] = value;		pTangents[10] = 0.0;		pTangents[11] = 0.0;

			glVertexAttribPointer (iLocTangent, 3, LC_OGL_SCALAR_TYPE, GL_FALSE, 0, pTangents);
		}

		// Load Bi-Tangent coordinates only if needed
		if (iLocBiTangent >= 0)
		{
			pBiTangents[0] = 0.0;		pBiTangents[1]	= value;		pBiTangents[2]	= 0.0;
			pBiTangents[3] = 0.0;		pBiTangents[4]	= value;		pBiTangents[5]	= 0.0;
			pBiTangents[6] = 0.0;		pBiTangents[7]	= value;		pBiTangents[8]	= 0.0;
			pBiTangents[9] = 0.0;		pBiTangents[10] = value;		pBiTangents[11] = 0.0;

			glVertexAttribPointer (iLocBiTangent, 3, LC_OGL_SCALAR_TYPE, GL_FALSE, 0, pBiTangents);
		}
		
		if (iLocVertexPosX >= 0)
		{
			if (pVertices != 0)
			{
				pVertexPosX[2] = 0.0;		pVertexPosX[3] = 1.0;
				pVertexPosX[0] = 0.0;		pVertexPosX[1] = 1.0;
				glVertexAttribPointer (iLocVertexPosX, 1, LC_OGL_SCALAR_TYPE, GL_FALSE, 0, pVertexPosX);
			}
			else
			{
				glVertexAttribPointer (iLocVertexPosX, 1, LC_OGL_SCALAR_TYPE, GL_FALSE, GRID_ARRAY_STRIDE * sizeof(LcOglTScalar), (LcOglTScalar*) (8 * sizeof(LcOglTScalar)));
			}
		}

		if (iLocVertexPosY >= 0)
		{
			if (pVertices != 0)
			{
				pVertexPosY[2] = 0.0;		pVertexPosY[3] = 0.0;
				pVertexPosY[0] = 1.0;		pVertexPosY[1] = 1.0;
				glVertexAttribPointer (iLocVertexPosY, 1, LC_OGL_SCALAR_TYPE, GL_FALSE, 0, pVertexPosY);
			}
			else
			{
				glVertexAttribPointer (iLocVertexPosY, 1, LC_OGL_SCALAR_TYPE, GL_FALSE, GRID_ARRAY_STRIDE * sizeof(LcOglTScalar), (LcOglTScalar*) (9 * sizeof(LcOglTScalar)));
			}
		}

		// Bind texture(s)
		currentEffect->bindTextures(this, EBitmap);
	}

#else // GL 1.1

	// Set up for draw (these arrays always enabled by default)
	glVertexPointer(3, LC_OGL_SCALAR_TYPE, GRID_ARRAY_STRIDE * sizeof(LcOglTScalar), pVertices);
	glTexCoordPointer(2, LC_OGL_SCALAR_TYPE, GRID_ARRAY_STRIDE * sizeof(LcOglTScalar), pTexCoords);
	glNormalPointer(LC_OGL_SCALAR_TYPE, GRID_ARRAY_STRIDE * sizeof(LcOglTScalar), pNormals);

	// Select current texture
	if(m_isExternal)
		glBindTexture(m_externalTextureTarget, m_tex[0]);
	else
		glBindTexture(GL_TEXTURE_2D, m_tex[0]);

	setWrapMode (GL_CLAMP);
#endif

	// No color means no modulation, which means white
	if (color == LcTColor::NONE)
		color = LcTColor::WHITE;

	// Textures are modulated onto a pre-lit base rectangle subject to
	// lighting, using material rather than cColor to avoid state switches.
	// NB: we always call this, as even opaque white requires a reset
	m_oglContext->setMaterialFlatColor(color, opacity);

	// Turn on MSAA if it is supported
	if (display->msaa_samples > 1 && antiAlias)
		glEnable(GL_MULTISAMPLE);

	if (m_hasPreMultipliedAlpha)
#if defined (LC_PLAT_OGL_20)
		glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
#else
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif

	glDrawElements(
		GL_TRIANGLES,
		iPolyCount * 3,
		GL_UNSIGNED_SHORT,
		pIndices);

	// Turn on MSAA if it is supported
	if (display->msaa_samples > 1 && antiAlias)
		glDisable(GL_MULTISAMPLE);

	if (m_hasPreMultipliedAlpha)
#if defined (LC_PLAT_OGL_20)
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);
#else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

#if defined (LC_PLAT_OGL_20)
	if (currentEffect != NULL)
	{
		// Un-bind all the textures
		currentEffect->unbindTextures(this);
	}
#else
	// Select current texture
	if(m_isExternal)
		glBindTexture(m_externalTextureTarget, 0);
	else
		glBindTexture(GL_TEXTURE_2D, 0);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCTexture::setExternalTexture(GLuint textures[IFX_OGL_MAX_TEXTURES], LcTPixelDim texSize, LcTPixelRect bitmapRect, GLuint textureTarget)
{
	// Grid info cache is now invalid - clear it out
	m_gridInfoCache.clear();

	if (!m_isExternal)
	{
		// Only delete the texture if it is internally owned.
		glDeleteTextures(1, m_tex);
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
	if (m_format == GL_ALPHA)
		m_oglContext->getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, -m_textureSize.width * m_textureSize.height);
	else
		m_oglContext->getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, -m_textureSize.width * m_textureSize.height * 4);
#endif
	}

	for (int i=0; i<IFX_OGL_MAX_TEXTURES; ++i)
		m_tex[i] = textures[i];
	m_isExternal = true;
	m_bitmapRect = bitmapRect;
	m_textureSize = texSize;
	m_externalTextureTarget = textureTarget;
}

LC_EXPORT void LcOglCTexture::setWrapMode (int wrapMode)
{
	// Update the wrapping mode if different from current wrapping mode
	if (wrapMode != m_wrapMode)
	{
		m_wrapMode = wrapMode;

		LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint) wrapMode);
		LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint) wrapMode);
	}
}

#endif // #if defined(LC_PLAT_OGL)
