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


#if defined(LC_PLAT_OGL) && defined(LC_USE_MESHES)

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT LcTaOwner<LcOglCMesh> LcOglCMesh::create(LcCSpace* sp)
{
	LcTaOwner<LcOglCMesh> ref;
	ref.set(new LcOglCMesh(sp));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL LcOglCMesh::~LcOglCMesh()
{

#ifdef LC_USE_OGL_VBOS
	if (m_arrayBufferObject[0])
	{
		glDeleteBuffers(2, &m_arrayBufferObject[0]);
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, -(m_interleaveByteCount + m_texturedInterleaveByteCount));
#endif
		m_arrayBufferObject[0] = 0;
		m_arrayBufferObject[1] = 0;
	}

	if (m_elementBufferObject)
	{
		glDeleteBuffers(1, &m_elementBufferObject);
		m_elementBufferObject = 0;
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, -m_vertexIndex64kByteCount);
#endif
	}
#endif

}

/*-------------------------------------------------------------------------*//**
 Initializes the textures and any other data associated with the mesh.
 We do not know whether the nd3 (or dae file) mesh has a texture until we
 have completed loading the data. So after the data is loaded, if there
 is texture information we need to set up a texture. We also possibly need
 to scale the texture coordinates in case the image being used has
 dimensions that are not a power of two.
 */
LC_EXPORT_VIRTUAL void LcOglCMesh::initializeData(LcIBitmapLoader* bitmapLoader)
{
	// Make sure that this function is called only once per mesh.
	if (m_dataInitialized)
	{
		return;
	}

	LcOglCMeshMaterial * material;
	for (unsigned i = 0; i < m_meshMaterials.size(); i++)
	{
		material = (LcOglCMeshMaterial *) m_meshMaterials[i];
		material->initializeTexture(bitmapLoader);
	}
	reScaleTexcoords();
	m_dataInitialized = true;
}


LC_EXPORT_VIRTUAL LcTaOwner<LcCMeshMaterial> LcOglCMesh::createMeshMaterial(LcCMesh* ownerMesh)
{
	return LcOglCMeshMaterial::create(ownerMesh);
}


/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL void LcOglCMesh::releaseMeshResources()
{
#if defined(LC_USE_OGL_VBOS)
	if (m_arrayBufferObject[0])
	{
		glDeleteBuffers(2, &m_arrayBufferObject[0]);
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, -(m_interleaveByteCount + m_texturedInterleaveByteCount));
#endif
		m_arrayBufferObject[0] = 0;
		m_arrayBufferObject[1] = 0;
	}

	if (m_elementBufferObject)
	{
		glDeleteBuffers(1, &m_elementBufferObject);
		m_elementBufferObject = 0;
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, m_vertexIndex64kByteCount);
#endif
	}
#endif
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL void LcOglCMesh::reloadMeshResources()
{
#if defined(LC_USE_OGL_VBOS)
	m_arrayBufferObject[0]=0;
	m_arrayBufferObject[1]=0;
	m_elementBufferObject=0;
#endif
}

/*-------------------------------------------------------------------------*/
/**
Draw this mesh
*/
LC_EXPORT_VIRTUAL void LcOglCMesh::draw(	const LcTVector&	pos,
										LcTColor			color,
										LcTScalar			opacity,
										ESubMeshDrawMode	subMeshDrawMode,
										bool antiAlias)
{

	LcOglCContext* oglContext = getSpace()->getOglContext();

	IFX_DISPLAY* display = getSpace()->getDisplay();

	// Turn on MSAA if it is supported
	if (display->msaa_samples > 1 && antiAlias)
		glEnable(GL_MULTISAMPLE);

	oglContext->setCullFace(GL_TRUE);

#ifdef LC_USE_OGL_VBOS
	if (m_arrayBufferObject[0] == 0)
	{
		// There are no VBOs so create them
		glGenBuffers(2, &m_arrayBufferObject[0]);

		if ( m_interleaveByteCount > 0)
		{
			// Non textured VBO
			glBindBuffer(GL_ARRAY_BUFFER, m_arrayBufferObject[0]);
			glBufferData(GL_ARRAY_BUFFER, m_interleaveByteCount, m_pInterleavedArray, GL_STATIC_DRAW);
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, m_interleaveByteCount);
#endif
		}

		if ( m_texturedInterleaveByteCount > 0 )
		{
			// Textured VBO
			glBindBuffer(GL_ARRAY_BUFFER, m_arrayBufferObject[1]);
			glBufferData(GL_ARRAY_BUFFER, m_texturedInterleaveByteCount, m_pTexturedInterleavedArray, GL_STATIC_DRAW);
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
		getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, m_texturedInterleaveByteCount);
#endif
		}
	}

	if (m_isIndexed)
	{
		// Indexed, so create element buffer too.
		if (m_elementBufferObject == 0)
		{
			// Create the buffer object
			glGenBuffers(1, &m_elementBufferObject);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_vertexIndex64kByteCount, m_pVertexIndex64k, GL_STATIC_DRAW);
#ifdef IFX_ENABLE_BENCHMARKING_HEAP_USAGE
			getSpace()->emitBenchmarkSignal(IFXP_BENCHMARKING_GPU_MEMORY_CHANGED, m_vertexIndex64kByteCount);
#endif
		}
	}
#endif

	getSpace()->transformsChanged();
	m_subMeshCoordinateSpaceActive = false;

	for ( unsigned i = 0; i < m_rootMeshNodes.size(); i++ )
	{
		LcCMeshNode* node = m_rootMeshNodes[i];
		drawMeshNode( node, pos, color, opacity, subMeshDrawMode);
	}


#ifdef LC_USE_OGL_VBOS
	// Unbind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (m_isIndexed)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
#endif


#if !defined(LC_PLAT_OGL_20)
	// Unbind texture and enable the texture coord array.
	glBindTexture(GL_TEXTURE_2D, 0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

	// Turn off MSAA if it is supported
	if (display->msaa_samples > 1 && antiAlias)
		glDisable(GL_MULTISAMPLE);
}


LC_EXPORT_VIRTUAL void LcOglCMesh::drawMeshNode(LcCMeshNode* node,
												const LcTVector&	pos,
												LcTColor			color,
												LcTScalar			opacity,
												ESubMeshDrawMode	subMeshDrawMode )
{
	// Get the geometry for this node.
	LcCMeshGeometry* pGeometry = node->instanceGeometry();
	if ( pGeometry  != NULL)  // Could be null if this node is just a group of subnodes
	{
		LcOglCContext* oglContext = getSpace()->getOglContext();

#if defined(LC_PLAT_OGL_20)
		LcOglCEffect* currentEffect = NULL;
		GLint iLocPosition = -1;
		GLint iLocNormal = -1;
		GLint iLocTexture = -1;
		GLint iLocTangent = -1;
		GLint iLocBiTangent = -1;
#endif

		// Draw each triangle group in the geometry. Usually there will only be one.
		for ( unsigned tg = 0; tg < pGeometry->triangleGroups()->size(); tg++ )
		{
			LcCMeshTriangleGroup* triangleGroup = pGeometry->triangleGroups()->at(tg);
			LcOglCTexture *currentTexture = NULL;

			bool isTranslucent = false;

			// Get the material for this node and triangle group
			LcOglCMeshMaterial* material = (LcOglCMeshMaterial*) node->instanceMaterial(tg);
			if ( material == NULL )
				continue;

			// If only drawing opaques but node material is translucent, skip this triangle group
			isTranslucent = material->m_afDiffuse[3] < 1.0;
			if (subMeshDrawMode == ESubMeshDrawOpaqueOnly && isTranslucent)
				continue;

			// Do we have both texcoords an a material texture
			bool isTextured = triangleGroup->isTextured() && (material->m_oglMeshTextureMap.texture != NULL);

			// If only drawing opaques but texture is translucent, continue to next triangle set
			isTranslucent = isTranslucent
				|| (isTextured && (material->m_oglMeshTextureMap.bmp != NULL && material->m_oglMeshTextureMap.bmp->isTranslucent()));
			if (subMeshDrawMode == ESubMeshDrawOpaqueOnly && isTranslucent)
				continue;

#if defined(LC_PLAT_OGL_20)

			// Has the effect been changed ?
			bool effectUpdated = false;

			if (isTextured)
			{
				if ( !currentEffect || currentEffect->getEffectUsage() != TEXLIGHT00_EFFECT_INDEX )
				{
					oglContext->switchEffect(TEXLIGHT00_EFFECT_INDEX, getSpace()->getPaintWidget());
					currentEffect = oglContext->getCurrentEffect();
					effectUpdated = true;
				}
			}
			else
			{
				if ( !currentEffect || currentEffect->getEffectUsage() != LIGHT00_EFFECT_INDEX )
				{
					oglContext->switchEffect(LIGHT00_EFFECT_INDEX, getSpace()->getPaintWidget() );
					currentEffect = oglContext->getCurrentEffect();
					effectUpdated = true;
				}
			}

			if (currentEffect == NULL)
				continue;

			if ( effectUpdated )
			{
				// Get attribute location for non-fixed attributes
				iLocPosition = currentEffect->getEnumeratedLocation(IFX_VERTEX_COORDS);
				iLocNormal = currentEffect->getEnumeratedLocation(IFX_NORMAL_COORDS);

				if (isTextured )
				{
					// Get updated attribute locations
					iLocTexture  = currentEffect->getEnumeratedLocation(IFX_TEXTURE_COORDS);
					iLocTangent	= currentEffect->getEnumeratedLocation(IFX_TANGENT_COORDS);
					iLocBiTangent = currentEffect->getEnumeratedLocation(IFX_BINORMAL_COORDS);
				}
			}

			// If only drawing opaques but effect is translucent, skip this triangle group
			isTranslucent = isTranslucent || currentEffect->getMakesTranslucent();
			if (subMeshDrawMode == ESubMeshDrawOpaqueOnly && isTranslucent)
				continue;

#endif /* if !defined(LC_PLAT_OGL_20) */

			// If only drawing translucents, and there is nothing translucent, skip this triangle group
			if (subMeshDrawMode == ESubMeshDrawTranslucentOnly && !isTranslucent)
				continue;

			// Copy ambient/diffuse for modulation
			LcOglTScalar diffuse[4];
			LcOglTScalar ambient[4];
			memcpy(diffuse, material->m_afDiffuse, 4 * sizeof(LcOglTScalar));
			memcpy(ambient, material->m_afAmbient, 4 * sizeof(LcOglTScalar));

			// Apply color modulation
			if (color != LcTColor::WHITE && color != LcTColor::NONE)
			{
				diffuse[0] = diffuse[0] * LC_MESH_FROM_SCALAR(LcTScalar(int(color.rgba.r)) / 255.0f);
				diffuse[1] = diffuse[1] * LC_MESH_FROM_SCALAR(LcTScalar(int(color.rgba.g)) / 255.0f);
				diffuse[2] = diffuse[2] * LC_MESH_FROM_SCALAR(LcTScalar(int(color.rgba.b)) / 255.0f);

				ambient[0] = ambient[0] * LC_MESH_FROM_SCALAR(LcTScalar(int(color.rgba.r)) / 255.0f);
				ambient[1] = ambient[1] * LC_MESH_FROM_SCALAR(LcTScalar(int(color.rgba.g)) / 255.0f);
				ambient[2] = ambient[2] * LC_MESH_FROM_SCALAR(LcTScalar(int(color.rgba.b)) / 255.0f);
			}

			// Apply opacity modulation
			if (opacity < LC_OPAQUE_THRESHOLD)
			{
				diffuse[3] *= opacity;
				ambient[3] *= opacity;

				// Ignore specular, emission and shininess if not opaque
				LcOglTScalar black[4] = { 0, 0, 0, 1 };
				oglContext->setMaterialSpecular(black);
				oglContext->setMaterialEmission(black);
				oglContext->setMaterialShininess(0);
			}
			else
			{
				oglContext->setMaterialSpecular( material->m_afSpecular );
				oglContext->setMaterialEmission( material->m_afEmissive );
				oglContext->setMaterialShininess( material->m_fShininess );
			}
			// Set ambient and diffuse colors
			oglContext->setMaterialAmbient(ambient);
			oglContext->setMaterialDiffuse(diffuse);

			// Set up triangle set geometries
			LcTMeshScalar * pArrayPositions;
			LcTMeshScalar * pArrayNormals;
			LcTMeshScalar * pArrayTangents;
			LcTMeshScalar * pArrayBitangents;
			LcTMeshScalar * pArrayTexcoords;
			int iaByteStride;

#ifdef LC_USE_OGL_VBOS

			if ( !triangleGroup->isTextured())
			{
				// Not textured
				pArrayPositions = (LcTMeshScalar *) ( triangleGroup->m_positionsFloatOffset * sizeof( LcTMeshScalar ) );
				pArrayNormals = (LcTMeshScalar *) ( triangleGroup->m_normalsFloatOffset * sizeof( LcTMeshScalar ) );
				iaByteStride = m_interleaveByteStride;

				glBindBuffer(GL_ARRAY_BUFFER, m_arrayBufferObject[0]);
			}
			else
			{
				// Textured
				pArrayPositions =	(LcTMeshScalar *) ( triangleGroup->m_positionsFloatOffset * sizeof( LcTMeshScalar ) );
				pArrayNormals =		(LcTMeshScalar *) ( triangleGroup->m_normalsFloatOffset * sizeof( LcTMeshScalar ) );
				pArrayTexcoords =	(LcTMeshScalar *) ( triangleGroup->m_texcoordsFloatOffset * sizeof( LcTMeshScalar ) );
				pArrayTangents =	(LcTMeshScalar *) ( triangleGroup->m_tangentsFloatOffset * sizeof( LcTMeshScalar ) );
				pArrayBitangents =	(LcTMeshScalar *) ( triangleGroup->m_bitangentsFloatOffset * sizeof( LcTMeshScalar ) );
				iaByteStride = m_texturedInterleaveByteStride;

				glBindBuffer(GL_ARRAY_BUFFER, m_arrayBufferObject[1]);
			}

			if (m_isIndexed)
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_elementBufferObject);
#else
			if ( !triangleGroup->isTextured() )
			{
				// Not textured
				pArrayPositions = &m_pInterleavedArray[triangleGroup->m_positionsFloatOffset];
				pArrayNormals = &m_pInterleavedArray[triangleGroup->m_normalsFloatOffset];
				iaByteStride = m_interleaveByteStride;
			}
			else
			{
				// Textured
				pArrayPositions =	&m_pTexturedInterleavedArray[triangleGroup->m_positionsFloatOffset];
				pArrayNormals =		&m_pTexturedInterleavedArray[triangleGroup->m_normalsFloatOffset];
				pArrayTexcoords =	&m_pTexturedInterleavedArray[triangleGroup->m_texcoordsFloatOffset];
				pArrayTangents =	&m_pTexturedInterleavedArray[triangleGroup->m_tangentsFloatOffset];
				pArrayBitangents =	&m_pTexturedInterleavedArray[triangleGroup->m_bitangentsFloatOffset];
				iaByteStride = m_texturedInterleaveByteStride;
			}
#endif

			// Setup the primary texture.
			if (isTextured)
			{
				currentTexture = material->m_oglMeshTextureMap.texture;
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				currentTexture = NULL;
			}

#if defined(LC_PLAT_OGL_20)
			// Load vertex coodinates only if valid attribute index exists
			if (iLocPosition >= 0)
				glVertexAttribPointer (iLocPosition, 3, LC_OGL_SCALAR_TYPE, GL_FALSE, iaByteStride, pArrayPositions);

			// Load normal coodinates only if valid attribute index exists
			if (iLocNormal >= 0)
				glVertexAttribPointer (iLocNormal, 3, LC_OGL_SCALAR_TYPE, GL_FALSE, iaByteStride, pArrayNormals);

			if (currentTexture != NULL)
			{
				// Load texture coodinates only if valid attribute index exists
				if (iLocTexture >= 0)
					glVertexAttribPointer (iLocTexture,  2, LC_OGL_SCALAR_TYPE, GL_FALSE, iaByteStride, pArrayTexcoords);

				// Load tangent coodinates only if valid attribute index exists
				if (iLocTangent >= 0)
					glVertexAttribPointer (iLocTangent, 3, LC_OGL_SCALAR_TYPE, GL_FALSE, iaByteStride, pArrayTangents);

				// Load bi-tangent coodinates only if valid attribute index exists
				if (iLocBiTangent >= 0)
					glVertexAttribPointer (iLocBiTangent, 3, LC_OGL_SCALAR_TYPE, GL_FALSE, iaByteStride, pArrayBitangents);
			}

			// Bind any custom effect samplers.
			currentEffect->bindTextures(currentTexture, EMesh);

#else	/* if !defined(LC_PLAT_OGL_20) */

			// Set up arrays
			glVertexPointer(3, LC_OGL_SCALAR_TYPE, iaByteStride, pArrayPositions);
			glNormalPointer(LC_OGL_SCALAR_TYPE, iaByteStride, pArrayNormals);

			if (currentTexture != NULL)
			{
				glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				glBindTexture(GL_TEXTURE_2D, currentTexture->getTexture());

				if (material->m_oglMeshTextureMap.texture->isPOT())
				{
					material->m_oglMeshTextureMap.texture->setWrapMode(GL_REPEAT);
				}
				else
				{
					material->m_oglMeshTextureMap.texture->setWrapMode(GL_CLAMP);
				}

				// Works for both LC_USE_OGL_VBOS and ! LC_USE_OGL_VBOS
				glTexCoordPointer(2, LC_OGL_SCALAR_TYPE, iaByteStride, pArrayTexcoords);
			}
			else
			{
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}
#endif
			if ( node->instanceController() )
			{
				// Controllers not supported yet
			}

			// Update the transforms if required
			if (node->hasOwnCoordinateSpace())
			{
				getSpace()->transformsChanged(node->globalCoordinateSpace());
				m_subMeshCoordinateSpaceActive = true;
			}
			else if(m_subMeshCoordinateSpaceActive)
			{
				m_subMeshCoordinateSpaceActive = false;
				getSpace()->transformsChanged();
			}

			if ( !m_isIndexed ) {
				// Not indexed - draw arrays
				glDrawArrays( GL_TRIANGLES, 0, triangleGroup->cVertices() );
			}
			else
			{
				// Indexed - draw elements

#ifdef LC_USE_OGL_VBOS
				glDrawElements(GL_TRIANGLES, triangleGroup->cVertices(), GL_UNSIGNED_SHORT, (GLvoid*) (triangleGroup->indexArrayIndex() * sizeof(unsigned short)) );

#else // non-VBO
				glDrawElements(GL_TRIANGLES, triangleGroup->cVertices(), GL_UNSIGNED_SHORT, &m_pVertexIndex64k[triangleGroup->indexArrayIndex()]);
#endif
			}

			// Unbind the texture(s)
#if defined(LC_PLAT_OGL_20)
			currentEffect->unbindTextures(NULL);
#else
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
			glBindTexture(GL_TEXTURE_2D, 0);
#endif

		}
	}

	// Draw subnodes
	for ( unsigned i = 0; i < node->subNodes()->size(); i++ )
	{
		LcCMeshNode* subNode = node->subNodes()->at(i);
		drawMeshNode( subNode, pos, color, opacity, subMeshDrawMode );
	}
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT LcTaOwner<LcOglCMeshMaterial> LcOglCMeshMaterial::create(LcCMesh* ownerMesh)
{
	LcTaOwner<LcOglCMeshMaterial> ref;
	ref.set(new LcOglCMeshMaterial(ownerMesh));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL LcOglCMeshMaterial::~LcOglCMeshMaterial()
{
	clean();
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void LcOglCMeshMaterial::clean()
{
	if (m_oglMeshTextureMap.bmp != NULL)
	{
		m_oglMeshTextureMap.bmp->release();
		m_oglMeshTextureMap.bmp = NULL;
	}
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL void LcOglCMeshMaterial::initializeTexture(LcIBitmapLoader* bitmapLoader)
{
	if (m_sTexture.length() == 0)
	{
		m_oglMeshTextureMap.bmp = NULL;
		m_oglMeshTextureMap.texture = NULL;
	}
	else
	{
		LcTaString fileName = m_sTexture;
		LcCBitmap* bmp = NULL;
		if (!bitmapLoader)
		{
			fileName = fileName.subString(0, fileName.findLastChar('.')) + ".ndi";
			fileName = m_ownerMesh->getTextureAbsolutePath()+ fileName;
			bmp = m_ownerMesh->getSpace()->getBitmap(fileName);
		}
		else
		{
			bmp = bitmapLoader->getBitmap(m_ownerMesh->getTextureRelativePath() + fileName);
		}

		// The fact that we get a non-null bmp means that it could
		// create the texture successfully
		if (bmp)
		{
			LcTScalarRect	bmpSize;

			bmp->acquire();

			LcOglCTexture* newTexture = ((LcOglCBitmap*)bmp)->getTexture(bmpSize);

			if (newTexture)
			{
				m_oglMeshTextureMap.texture = newTexture;

				LcTPixelDim texSize = m_oglMeshTextureMap.texture->getPhysicalSize();
				if(texSize.height != 0 && texSize.width != 0)
				{
					m_oglMeshTextureMap.bmp = bmp;
					m_oglMeshTextureMap.sScale = bmpSize.getWidth() / texSize.width;
					m_oglMeshTextureMap.tScale = bmpSize.getHeight() / texSize.height;
				}
				else
				{
					bmp->release();
					bmp = NULL;
					m_oglMeshTextureMap.texture = NULL;
				}
			}
		}
		else
		{
			bmp = NULL;
			m_oglMeshTextureMap.texture = NULL;
		}
	}
}


/*-------------------------------------------------------------------------*//**
The OGL texture dimensions have to be a power of two, so the texture
created may not be the size of the image created. If this is the case we
have to scale our texture coordinates to reflect this. We simply scale
the corresponding texture coordinate by the ratio of the original image
dimension to the final texture dimension.
*/

// Note: Will work only under the BIG ASSUMPTION that a given triangle can
// belong to only one sub-mesh, and that the triangles are sequential

LC_EXPORT_VIRTUAL void LcOglCMesh::reScaleTexcoords()
{
	for ( unsigned i = 0; i < m_rootMeshNodes.size(); i++ )
	{
		LcCMeshNode* pNode = m_rootMeshNodes[i];
		reScaleTexcoords( pNode );
	}
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL void LcOglCMesh::reScaleTexcoords(LcCMeshNode* pNode)
{
	LcCMeshGeometry* pGeometry = pNode->instanceGeometry();
	if ( pGeometry  != NULL)  // Could be null if this node is just a group of subnodes
	{
		for ( unsigned tg1 = 0; tg1 < pGeometry->triangleGroups()->size(); tg1++ )
		{
			// Get the material for this node and triangle group
			LcOglCMeshMaterial* pMaterial = (LcOglCMeshMaterial*) pNode->instanceMaterial(tg1);
			if (pMaterial == NULL)
				return;

			for ( unsigned tg2 = 0; tg2 < pGeometry->triangleGroups()->size(); tg2++ )
			{
				LcCMeshTriangleGroup* triangleGroup = pGeometry->triangleGroups()->at(tg2);
				bool canBeTextured = triangleGroup->isTextured() && (pMaterial->m_oglMeshTextureMap.texture != NULL);

				if ( canBeTextured )
				{
					LcTScalar sScale = pMaterial->m_oglMeshTextureMap.tScale;
					LcTScalar tScale = pMaterial->m_oglMeshTextureMap.tScale;

					if (!triangleGroup->m_texcoordsClipped && !pMaterial->m_oglMeshTextureMap.texture->isPOT())
					{
						clipTexcoords(triangleGroup);
						triangleGroup->m_texcoordsClipped = true;
					}

					if ( !triangleGroup->m_texcoordsReScaled && (sScale != 1.0 || tScale != 1.0 ) )
					{
						reScaleTexcoords(triangleGroup,  sScale, tScale);
						triangleGroup->m_texcoordsReScaled = true;
					}
				}
			}
		}
	}
	for ( unsigned i = 0; i < pNode->subNodes()->size(); i++ )
	{
		LcCMeshNode* pSubNode = pNode->subNodes()->at(i);
		reScaleTexcoords( pSubNode );
	}
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void LcOglCMesh::reScaleTexcoords(LcCMeshTriangleGroup *pTriangleGroup, LcTMeshScalar sScale, LcTMeshScalar tScale)
{
	unsigned stride =
		+ COORDS_PER_VERTEX
		+ COORDS_PER_NORMAL
		+ COORDS_PER_TEXCOORD
		+ COORDS_PER_TEXTANGENT
		+ COORDS_PER_TEXBITANGENT;

	unsigned j = pTriangleGroup->m_texcoordsFloatOffset;
	unsigned texcoordsPairsCount = pTriangleGroup->m_floatCount / stride;
	for ( unsigned i = 0; i < texcoordsPairsCount; i++ )
	{
		m_pTexturedInterleavedArray[j] = m_pTexturedInterleavedArray[j] * sScale;
		m_pTexturedInterleavedArray[j+1] = m_pTexturedInterleavedArray[j+1] * tScale;
		j += stride;
	}
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void LcOglCMesh::clipTexcoords(LcCMeshTriangleGroup *pTriangleGroup)
{
	unsigned stride =
		+ COORDS_PER_VERTEX
		+ COORDS_PER_NORMAL
		+ COORDS_PER_TEXCOORD
		+ COORDS_PER_TEXTANGENT
		+ COORDS_PER_TEXBITANGENT;

	unsigned j = pTriangleGroup->m_texcoordsFloatOffset;
	unsigned texcoordsPairsCount = pTriangleGroup->m_floatCount / stride;
	for ( unsigned i = 0; i < texcoordsPairsCount; i++ )
	{
		LcTMeshScalar val = m_pTexturedInterleavedArray[j];
		if ( val > 1.0f)
			m_pTexturedInterleavedArray[j] = 1.0f;
		else if ( val < 0.0f)
			m_pTexturedInterleavedArray[j] = 0.0f;

		val = m_pTexturedInterleavedArray[j+1];
		if ( val > 1.0f)
			m_pTexturedInterleavedArray[j+1] = 1.0f;
		else if ( val < 0.0f)
			m_pTexturedInterleavedArray[j+1] = 0.0f;

		j += stride;
	}
}


#endif // #if defined(LC_PLAT_OGL) && defined(LC_USE_MESHES)
