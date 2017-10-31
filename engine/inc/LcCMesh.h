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
#ifndef LcCMeshH
#define LcCMeshH

#define ND3_USE_CONDENSED_VERTICES 0
#define ND3_TANGENT_SUPPORT


#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTString.h"


// These constants help distinguish all the different '3's in the code
#define VERTICES_PER_TRIANGLE	3
#define COORDS_PER_VERTEX		3
#define NORMALS_PER_TRIANGLE	3
#define COORDS_PER_NORMAL		3
#define TEXCOORDS_PER_TRIANGLE	3
#define TEXTANGENT_PER_TRIANGLE	3
#define COORDS_PER_TEXCOORD		2
#define COORDS_PER_TEXTANGENT	3
#define COORDS_PER_TEXBITANGENT	3

// Offsets of coordinates in coordinate arrays
#define MESH_COORD_OFFSET_X		0
#define MESH_COORD_OFFSET_Y		1
#define MESH_COORD_OFFSET_Z		2

// For OpenGL we store the mesh vertices, normals and tex-coords
// directly using the type most suited to rendering them quickly.
// This will be either GLfloat or GLfixed depending on platform

#ifdef LC_PLAT_OGL
	#include "inflexionui/engine/inc/LcOglCContext.h"
	typedef LcOglTScalar				LcTMeshScalar;
	typedef LcOglTScalar				LcTMeshUnitScalar;
	typedef int							LcTMeshIndex;

	// Conversions between LcTMeshScalar and LcTScalar
	#define LC_MESH_FROM_SCALAR(x)		LC_OGL_FROM_SCALAR(x)
	#define LC_MESH_FROM_FLOAT(x)		LC_OGL_FROM_FLOAT(x)
	#define LC_MESH_TO_SCALAR(x)		LC_OGL_TO_SCALAR(x)
	#define LC_MESH_TO_FLOAT(x)			LC_OGL_TO_FLOAT(x)

	// Conversions between LcTMeshUnitScalar and LcTUnitScalar
	#define LC_MESH_UNIT_FROM_SCALAR(x) LC_OGL_FROM_UNIT_SCALAR(x)
	#define LC_MESH_UNIT_FROM_FLOAT(x)	LC_OGL_FROM_FLOAT(x)
	#define LC_MESH_UNIT_TO_SCALAR(x)	LC_OGL_TO_UNIT_SCALAR(x)
	#define LC_MESH_UNIT_TO_FLOAT(x)	LC_OGL_TO_FLOAT(x)
#else

	// For non-OpenGL, by default use our usual NDE types
	typedef LcTScalar					LcTMeshScalar;
	typedef LcTUnitScalar				LcTMeshUnitScalar;
	typedef unsigned short				LcTMeshIndex;

	// Conversions between LcTMeshScalar and LcTScalar
	#define LC_MESH_FROM_SCALAR(x)		LcTMeshScalar(x)
	#define LC_MESH_FROM_FLOAT(x)		LcTMeshScalar(x)
	#define LC_MESH_TO_SCALAR(x)		LcTScalar(x)
	#define LC_MESH_TO_FLOAT(x)			float(x)

	// Conversions between LcTMeshUnitScalar and LcTUnitScalar
	#define LC_MESH_UNIT_FROM_SCALAR(x) LcTMeshUnitScalar(x)
	#define LC_MESH_UNIT_FROM_FLOAT(x)	LcTMeshUnitScalar(x)
	#define LC_MESH_UNIT_TO_SCALAR(x)	LcTUnitScalar(x)
	#define LC_MESH_UNIT_TO_FLOAT(x)	float(x)
#endif

#define LC_FLOAT_TO_SHORT(x)	((short)((x) * 0x4000))
#define LC_SHORT_TO_FLOAT(x)	(((float)(x)) / 0x4000)

/* An instance of LcCmeshMaterial determines the color or texture of an LcCMeshNode
* LcCMeshNode instances can share materials
*/

class LcCMeshMaterial : public LcCBase
{
protected:
		LcTmString					m_id;				// The id attribute
		LcCMesh*					m_ownerMesh;		// The LcCmesh that owns this node.

public:
		LcTMeshUnitScalar			m_afAmbient[4];
		LcTMeshUnitScalar			m_afSpecular[4];
		LcTMeshUnitScalar			m_afEmissive[4];
		LcTMeshUnitScalar			m_afDiffuse[4];
		LcTMeshScalar				m_fShininess;
		LcTmString					m_sTexture;

		int m_textureCount;

protected:
	LC_IMPORT						LcCMeshMaterial( LcCMesh* mesh );

public:
	// Construction
	LC_IMPORT	static LcTaOwner<LcCMeshMaterial> create(LcCMesh* mesh);
	LC_VIRTUAL						~LcCMeshMaterial();
	LC_IMPORT		void			clean();

					bool			load(LcCXmlElem* nodeEl);

	LC_IMPORT		inline	LcTmString id() {return m_id;}

};

/* An instance of LcCMeshTriangleGroup is a collection of triangles in an LcCMeshGeometry.
* There can be more than one group in a geometry, though typically there will be only one.
* A triangle group belongs to exactly one genometry.
*
* A triangle group is textured if it refers to values in the texturedInterleavedArray, i.e.
* if it has uv texture coordinates.
*/

class LcCMeshTriangleGroup : public LcCBase
{
	LcCMesh* m_ownerMesh;		// The LcCmesh that owns this triangle group
	unsigned m_cFaces;			// The number of triangle faces in this triangle group
	unsigned m_cVertices;		// The number of vertices in the triangle group, == cFaces * VERTICES_PER_TRIANGLE
	unsigned m_iFirstVertex;	// The index of the first vertex in the interleaved array
	bool	 m_isTextured;		// Does the triangle group have texture coordinates ?
	unsigned m_indexArrayIndex;	// The index into LcCmesh::m_arrayOfIndexes

public:
	unsigned m_floatOffset;	// The number of float values to the start of this group's values in the whole-mesh interleaved array, textured or non-textured,
	unsigned m_floatCount;	// The number of float values in the textured or non-textured interleaved array for this triangle group
	unsigned m_positionsFloatOffset;	// The number of float values to the start of the position (vertex) float values  in the whole-mesh interleaved array
	unsigned m_normalsFloatOffset;	// The number of float values to the start of the normals float values  in the whole-mesh interleaved array
	unsigned m_texcoordsFloatOffset;	// The number of float values to the start of the texcoords float values  in the whole-mesh interleaved array
	unsigned m_tangentsFloatOffset;	// The number of float values to the start of the tangents float values  in the whole-mesh interleaved array
	unsigned m_bitangentsFloatOffset;	// The number of float values to the start of the bitangents float values  in the whole-mesh interleaved array

	bool	 m_texcoordsClipped;	// Texcoords have been clipped to fit NPOT texture
	bool	 m_texcoordsReScaled;	// Texcoords have been rescaled to fit NPOT texture

protected:
	LC_IMPORT LcCMeshTriangleGroup(LcCMesh* ownerMesh);

public:
	LC_IMPORT static LcTaOwner<LcCMeshTriangleGroup> create(LcCMesh* mesh);
	LC_IMPORT						~LcCMeshTriangleGroup();
	LC_IMPORT			void		clean();
	LC_IMPORT			bool		load(LcCXmlElem* nodeEl);

	LC_IMPORT inline	LcCMesh*	ownerMesh()	{return m_ownerMesh;} // The LcCmesh that owns this triangle group.
	LC_IMPORT inline	unsigned	cFaces() {return m_cFaces;}		// The number of triangle faces in this triangle group
	LC_IMPORT inline	unsigned	cVertices() {return m_cVertices;}// The number of vertices in the triangle group, == cFaces * VERTICES_PER_TRIANGLE
	LC_IMPORT inline	unsigned	iFirstVertex() {return m_iFirstVertex;} // The index of the first vertex in the interleaved array
	LC_IMPORT inline	bool		isTextured() {return m_isTextured;} // Does the triangle group have texture coordinates ?
	LC_IMPORT inline	unsigned	indexArrayIndex() {return m_indexArrayIndex;}	// The index into LcCmesh::m_arrayOfIndexes
};

/*
* An instance of an LcCMeshGeometry defines the geometry of an LcCMeshNode, using one or more
* LcCMeshTriangleGroups. Geometries can be shared between nodes.
*
*/

class LcCMeshGeometry : public LcCBase
{
	LcTmString	m_id;			// This geometry's id attribute
	LcCMesh*	m_ownerMesh;		// The LcCmesh that owns this geometry.

	LcTmOwnerArray<LcCMeshTriangleGroup>	m_triangleGroups;

protected:
	LC_IMPORT						LcCMeshGeometry( LcCMesh* mesh );

public:
	LC_IMPORT static LcTaOwner<LcCMeshGeometry>				create(LcCMesh* mesh);
	LC_IMPORT												~LcCMeshGeometry();
	LC_IMPORT		 void									clean();
	LC_IMPORT		 bool									load(LcCXmlElem* el);

	LC_IMPORT inline LcTmString								id() {return m_id;}				// This geometry's id attribute
	LC_IMPORT inline LcCMesh*								ownerMesh()	{return m_ownerMesh;} // The LcCmesh that owns this geometry.
	LC_EXPORT inline LcTmOwnerArray<LcCMeshTriangleGroup>*	triangleGroups() { return &m_triangleGroups; }
};

/* An LcCMeshController can be attached to an LcCMeshNode to define the target geometry
* for a morph. Currently only one target geometry is allowed.
*/

class LcCMeshController : public LcCBase
{
		LcTmString			m_id;
		LcCMesh*			m_ownerMesh;

		LcTmString			m_targetGeometryUrl;
		LcCMeshGeometry*	m_targetGeometry;

protected:
	LC_IMPORT				LcCMeshController( LcCMesh* mesh );

public:
	LC_IMPORT	static	LcTaOwner<LcCMeshController>	create(LcCMesh* mesh);
	LC_IMPORT											~LcCMeshController();
	LC_IMPORT			void							clean();
	LC_IMPORT			bool							load(LcCXmlElem* nodeEl);
	LC_EXPORT			bool							resolveTargetGeometry();

	LC_IMPORT	inline	LcTmString						id() {return m_id;}
	LC_EXPORT	inline	LcCMeshGeometry*				targetGeometry() { return m_targetGeometry; }
};

/* An instance of LcCMeshNode is an object or collection of objects in the mesh.
* It can have
* - a geometry which defines its shape,
* - a material which defines its color or texture applied to the geometry
* - a (global) coordinate space transform which the geometry is placed in relative to that of the whole mesh.
*
* A node can contain subnodes, so forming a collection of objects.
*
* In the future it will also have a local coordinate system transform, to define
* its coordinate system relative to its containing node, if it has one.
*
*/

class LcCMeshNode : public LcCBase
{
private:
		LcTmString					m_id;				// The id attribute of this node
		LcCMesh*					m_ownerMesh;		// The LcCmesh that contains this node.

		bool						m_hasOwnCoordinateSpace; // Does this node have a global coordinate space which is different from the whole mesh ?
		LcTTransform				m_localCoordinateSpace; // Space of this node relative to its parent. Currently not used.
		LcTTransform				m_globalCoordinateSpace; // Space of this node relative to the whole mesh

		LcTmString					m_instanceGeometryUrl;	// The url of the geometry for this node
		LcCMeshGeometry*			m_instanceGeometry;		// The geometry for this node

		LcTmString					m_instanceControllerUrl; // The url of the instance controller for this node.
		LcCMeshController*			m_instanceController;	// the instance controller for this node

		LcCMeshNode*				parentMeshNode;			// This node's parent

		LcTmOwnerArray<LcCMeshNode>	m_subNodes;				// The child node of this node

		LcTmArray<LcCMeshMaterial*> m_instanceMaterials;	// The material for each triangle group in this node's geometry


protected:
	LC_IMPORT						LcCMeshNode( LcCMesh* mesh );

public:
	LC_IMPORT	static LcTaOwner<LcCMeshNode>	create(LcCMesh* mesh);
	LC_IMPORT									~LcCMeshNode();
	LC_IMPORT		void						clean();
	LC_IMPORT		bool						load(LcCXmlElem* nodeEl);
	LC_EXPORT		bool						resolveInstanceGeometry();
	LC_EXPORT		bool						resolveInstanceMaterial();
	LC_EXPORT		bool						resolveInstanceController();


	LC_EXPORT		inline LcCMeshMaterial*		instanceMaterial(unsigned short triangleGroupIndex) { return m_instanceMaterials.at(triangleGroupIndex); }
	LC_EXPORT		inline LcCMeshGeometry*		instanceGeometry() { return m_instanceGeometry; }
	LC_EXPORT		inline LcCMeshController*	instanceController() { return m_instanceController; }

	LC_EXPORT		inline bool					hasOwnCoordinateSpace()	{ return m_hasOwnCoordinateSpace; } // Does this node have a global coordinate space which is different from the whole mesh ?
	LC_EXPORT		inline LcTTransform&		localCoordinateSpace() { return m_localCoordinateSpace; } // Space of this node relative to its parent. Currently not used.
	LC_EXPORT		inline LcTTransform&		globalCoordinateSpace() { return m_globalCoordinateSpace; }// Space of this node relative to the whole mesh.

	LC_IMPORT		inline	LcTmString			id() {return m_id;}
	LC_EXPORT		inline LcTmOwnerArray<LcCMeshNode>*	subNodes() { return &m_subNodes; }
};


/*-------------------------------------------------------------------------*//**
	Abstract base class that stores data representing a 3D mesh.  Loading
	from file and various utility methods are provided.  APIs for drawing
	are provided here but must be implemented by platform derived classes.
*/
class LcCMesh : public LcCBase
{
protected:

	#define TEXTURE_FILENAME_LENGTH	128


	LcCWidget*						m_widget;

	// Space which owns us
	LcCSpace*						m_space;
	int								m_refCount;
	bool							m_bUseRefCount;

	LcTmString						m_textureAbsolutePath;
	LcTmString						m_textureRelativePath;

	// Maximum magnitude of vertices of the source of this mesh, before normalization
	LcTScalar						m_maxMag;

	//Scale for displaying this mesh at
	LcTScalar							m_dScale;

	// Bounding box
	LcTScalar							m_minX;
	LcTScalar							m_maxX;
	LcTScalar							m_minY;
	LcTScalar							m_maxY;
	LcTScalar							m_minZ;
	LcTScalar							m_maxZ;

	bool								m_hasTranslucentSubMesh;

	LcTScalar							m_sourceMaxMag;
	LcTScalar							m_scaledBoundingRadius;	//	originalBoundingRadius/(10^int(log10(maxDimension)))

	LC_IMPORT		void				updateBoundingBox();

LC_PRIVATE_INTERNAL_PROTECTED:

	// Helpers
	LC_IMPORT		void				clean();

LC_PRIVATE_INTERNAL_PUBLIC:

	enum ESubMeshDrawMode
	{
		ESubMeshDrawAll,
		ESubMeshDrawOpaqueOnly,
		ESubMeshDrawTranslucentOnly
	};


	// Interleaved array of Vertex, Normal data
	LcTmAlloc<LcTMeshScalar>			m_pInterleavedArray;
	int									m_interleaveByteCount;
	int									m_interleaveByteStride;

	// Interleaved array of Vertex, Normal and TexCoord data
	LcTmAlloc<LcTMeshScalar>			m_pTexturedInterleavedArray;
	int									m_texturedInterleaveByteCount;
	int									m_texturedInterleaveByteStride;

	// Index into interleaved arrays, when indexing is used
	LcTmAlloc<short>					m_pVertexIndex64k;
	int									m_vertexIndex64kByteCount;
	int									m_vertexIndex64kByteStride;

	virtual			LcTaOwner<LcCMeshMaterial> createMeshMaterial(LcCMesh* ownerMesh);


	// revalidation interface
	LC_IMPORT		void			attachWidget(LcCWidget* widget) { m_widget = widget; }
	LC_IMPORT		void			doRevalidate();

	// Reference counting
	inline			void			useRefCount(bool bUseRefCount)	{ m_bUseRefCount = bUseRefCount; }

	// Generic draw() API with optional color request
	virtual			void			draw(const LcTVector& pos,
											LcTColor color,
											LcTScalar opacity,
											ESubMeshDrawMode subMeshDrawMode,
											bool antiAlias) = 0;

LC_PRIVATE_INTERNAL_PUBLIC:

	// e.g texture maps can be initialized only after the mesh is created
	LC_VIRTUAL		void			initializeData(LcIBitmapLoader* bitmapLoader)	{}
	LC_VIRTUAL		void			reloadMeshResources()								{}
	LC_VIRTUAL		void			releaseMeshResources()								{}




protected:

	// Abstract so keep constructor protected
	LC_IMPORT						LcCMesh(LcCSpace* sp);

public:

	// Root of DAE file
	LcCXmlElem*							m_root;

	int 								m_textureCount; // The number of textures used by all the meshes in this file

	LcTmOwnerArray<LcCMeshMaterial>		m_meshMaterials;
	LcTmOwnerArray<LcCMeshGeometry>		m_meshGeometries;
	LcTmOwnerArray<LcCMeshController>	m_meshControllers;
	LcTmOwnerArray<LcCMeshNode>			m_rootMeshNodes;

	bool								m_isIndexed;


	LC_VIRTUAL						~LcCMesh();

	// Manipulate mesh
	LC_IMPORT		void			transform(const LcTTransform& t, bool transformNormals = true);


	// Reference counting
	inline			void			acquire()				{ if (m_bUseRefCount) m_refCount++; }
	LC_VIRTUAL		void			release();


	// Accessors
	inline			LcCSpace*		getSpace()				{ return m_space; }
	inline			LcTScalar		getMinX()				{ return m_minX; }
	inline			LcTScalar		getScaledBoundingRadius()				{ return m_scaledBoundingRadius; }
	inline			LcTScalar		getMaxX()				{ return m_maxX; }
	inline			LcTScalar		getSourceMaxMag()		{ return m_sourceMaxMag; }
	inline			LcTScalar		getMinY()				{ return m_minY; }
	inline			LcTScalar		getMaxY()				{ return m_maxY; }
	inline			LcTScalar		getMinZ()				{ return m_minZ; }
	inline			LcTScalar		getMaxZ()				{ return m_maxZ; }
	inline			bool			isTranslucent()			{ return m_hasTranslucentSubMesh; }
	inline			LcTScalar		getDScale()				{ return m_dScale; }
	inline			LcTaString		getTextureAbsolutePath(){ return m_textureAbsolutePath; }
	inline			LcTaString		getTextureRelativePath(){ return m_textureRelativePath; }
	inline			void			setTextureRelativePath(LcTaString path){m_textureRelativePath=path; }



	inline			LcCXmlElem* 	getRoot(){ return m_root; }


	// Get bounding radius
	LC_EXPORT		void			getScaledBoundingRadius(unsigned char* pND3Buffer, float* output);

	LC_EXPORT		static	bool	fillFloatArrayFromSourceElement(LcCXmlElem* sourceEl,
													LcTaArray<float>& floatArray);

	LC_EXPORT		static	bool	fillColorArrayFromColorElement(LcCXmlElem* colorEl,
													LcTMeshUnitScalar colorArray[4]);

	LC_EXPORT		static LcCXmlElem*		findElem(LcCXmlElem* pLocalRoot,
													const char* pName,
													const char* pAttribute,
													const char* pAttributeValue);

	LC_EXPORT		bool			getTextureFileName(LcCXmlElem* root,
													LcCXmlElem* profileEl,
													const LcTmString& textureName,
													LcTmString& textureImageFileName);	

	LC_IMPORT bool loadDataND3( const LcTmString&	sFilename);
};

#endif //LcCMeshH
