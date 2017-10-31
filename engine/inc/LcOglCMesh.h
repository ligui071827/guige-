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
#ifndef LcOglCMeshH
#define LcOglCMeshH

#include "inflexionui/engine/inc/LcCMesh.h"


class LcOglCMeshMaterial : public LcCMeshMaterial {

protected:
	LC_IMPORT						LcOglCMeshMaterial( LcCMesh* mesh ) :  LcCMeshMaterial( mesh ){}

public:
		// Construction
	LC_IMPORT	static				LcTaOwner<LcOglCMeshMaterial> create(LcCMesh* mesh);
	
	// Destruction
	LC_VIRTUAL						~LcOglCMeshMaterial();
	LC_IMPORT		void			clean();

	struct TOglMeshTextureMap
	{
		LcCBitmap*				bmp;
		LcOglCTexture*			texture;  
		LcTScalar				sScale;
		LcTScalar				tScale;
	};

	bool					m_dataInitialized;

	TOglMeshTextureMap		m_oglMeshTextureMap;

	LC_IMPORT		void	initializeTexture(LcIBitmapLoader* bitmapLoader);
};

/*-------------------------------------------------------------------------*//**
	LcOglCMesh is an OpenGL-specific class derived from LcCMesh
	and is used by all ogl meshes (textured and non-textured)
*/
class LcOglCMesh : public LcCMesh
{

private:

	bool						m_dataInitialized;
	bool						m_subMeshCoordinateSpaceActive;

#if defined(LC_USE_OGL_VBOS)
	GLuint						m_arrayBufferObject[2];
	GLuint						m_elementBufferObject;
#endif

					void			scaleTextureCoordinates();

LC_PRIVATE_INTERNAL_PUBLIC:

	LC_VIRTUAL		void			draw(	const LcTVector&	pos,
											LcTColor			color,
											LcTScalar			opacity,
											ESubMeshDrawMode	subMeshDrawMode,
											bool antiAlias);

	LC_VIRTUAL		void			drawMeshNode( LcCMeshNode* node,
												const LcTVector&	pos,
												LcTColor			color,
												LcTScalar			opacity,
												ESubMeshDrawMode	subMeshDrawMode);

	LC_VIRTUAL		void			initializeData(LcIBitmapLoader* bitmapLoader);

	LC_VIRTUAL		void			reScaleTexcoords();
	LC_VIRTUAL		void			reScaleTexcoords(LcCMeshNode *pNode);
	LC_VIRTUAL		void			reScaleTexcoords(LcCMeshTriangleGroup *pTriangleGroup, LcTMeshScalar sScale, LcTMeshScalar tScale);
	LC_VIRTUAL		void			clipTexcoords(LcCMeshTriangleGroup *pTriangleGroup);
	
	LC_VIRTUAL		LcTaOwner<LcCMeshMaterial> 			createMeshMaterial(LcCMesh * ownerMesh );

protected:

	// Allow only 2-phase construction
	LC_IMPORT						LcOglCMesh(LcCSpace* sp) : LcCMesh(sp) {}

public:

	// Construction
	LC_IMPORT	static LcTaOwner<LcOglCMesh> create(LcCSpace* sp);

	// Destruction
	LC_VIRTUAL						~LcOglCMesh();

	LC_VIRTUAL		void			reloadMeshResources();
	LC_VIRTUAL		void			releaseMeshResources();
};

#endif //LcOglCMeshH
