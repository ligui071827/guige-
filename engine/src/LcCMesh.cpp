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
#include "math.h"



#if defined(LC_USE_MESHES)


/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT LcCMesh::LcCMesh(LcCSpace* sp)
{
	m_space				= sp;
	m_bUseRefCount		= false;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL LcCMesh::~LcCMesh()
{
	clean();
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void LcCMesh::clean()
{
	if( m_pInterleavedArray )
		m_pInterleavedArray.free();

	if( m_pTexturedInterleavedArray )
		m_pTexturedInterleavedArray.free();

	if( m_pVertexIndex64k )
		m_pVertexIndex64k.free();
}


/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void LcCMesh::release()
{
	if (m_bUseRefCount)
	{
		// NB: we don't assume that we CAN delete the object.... the space
		// implementation may pool them
		if (--m_refCount <= 0 && m_space)
			m_space->unloadMesh(this);
	}
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void LcCMesh::doRevalidate()
{
	// Revalidate widget
	if (m_widget)
		m_widget->revalidate();
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT bool LcCMesh::loadDataND3( const LcTmString&	sFilename)
{
	LcTaOwner<LcCXmlElem>	root;
	LcTaString				err;

	// Abort if unable to load project file
	root = LcCXmlElem::load(sFilename, err);

	m_textureAbsolutePath = sFilename.subString(0, sFilename.length() - sFilename.getWord(-1, NDHS_PLAT_DIR_SEP_CHAR).length());
#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
	m_textureAbsolutePath.replace(NDHS_PLAT_DIR_SEP_CHAR, NDHS_DIR_SEP_CHAR);
#endif
	m_textureRelativePath=m_textureAbsolutePath;

	if (!root.ptr())
	{
		return false;
	}

	m_root = root.ptr();

	m_textureCount = 0;

	// Clean out old data
	clean();

	m_sourceMaxMag = 1.0;
	m_maxMag	= 1.0;
	m_scaledBoundingRadius=1.0;
	m_dScale	= 1.0;

	m_minX		= -1.0;
	m_maxX		= 1.0;
	m_minY		= -1.0;
	m_maxY		= 1.0;
	m_minZ		= -1.0;
	m_maxZ		= 1.0;

	m_hasTranslucentSubMesh=false;

	bool found = false;

	// Get the mesh metadata
	LcCXmlElem* extraEl = m_root->find("extra");
	LcCXmlElem* techniqueEl = extraEl->find("technique");
	LcCXmlElem* el = techniqueEl->find("nd3");
	if( el ) {
		m_scaledBoundingRadius = el->getAttr("sbr").toScalar();
		m_sourceMaxMag = el->getAttr("smm").toScalar();
		m_isIndexed = ( el->getAttr("ixd").compare("true") == 0) ;
	}

	// Get the bounding box
	el = techniqueEl->find("bb");
	if( el ) {
		LcTXMLTextTokenizer tok = LcTXMLTextTokenizer(el->getFirstChild());
		m_minX = tok.getScalarTokenNext();
		m_maxX = tok.getScalarTokenNext();
		m_minY = tok.getScalarTokenNext();
		m_maxY = tok.getScalarTokenNext();
		m_minZ = tok.getScalarTokenNext();
		m_maxZ = tok.getScalarTokenNext();
	}

	// Get the 'scene' element.
	LcCXmlElem* sceneEl = m_root->find("scene");

	LcTaString activeScene = "";
	// Look for the active scene
	for (LcTXmlIterator InstVisSceneIter = LcTXmlIterator(sceneEl, "instance_visual_scene"); InstVisSceneIter.hasMoreElements(); InstVisSceneIter.next())
	{
		activeScene = InstVisSceneIter.getElement()->getAttr("url","");
		if (activeScene.length() > 0)
		{
			activeScene = activeScene.subString(1, 0x7ffff);
			break;
		}
	}

	// There is probably only one <library_materials> tag, but just in case there are more, we iterate here.
	for (LcTXmlIterator libMatsIt = LcTXmlIterator(m_root, "library_materials"); libMatsIt.hasMoreElements(); libMatsIt.next())
	{
		for (LcTXmlIterator materialIt = LcTXmlIterator(libMatsIt.getElement(), "material"); materialIt.hasMoreElements(); materialIt.next())
		{
			// Invoke factory method
			LcTaOwner<LcCMeshMaterial> meshMaterial = createMeshMaterial(this);
			meshMaterial->load( materialIt.getElement() );
			m_meshMaterials.push_back( meshMaterial );
		}
	}

	// There is probably only one <library_geometries> tag, but just in case there are more, we iterate here.
	for (LcTXmlIterator libGeosIt = LcTXmlIterator(m_root, "library_geometries"); libGeosIt.hasMoreElements(); libGeosIt.next())
	{
		for (LcTXmlIterator geometryIt = LcTXmlIterator(libGeosIt.getElement(), "geometry"); geometryIt.hasMoreElements(); geometryIt.next())
		{
			LcTaOwner<LcCMeshGeometry> meshGeometry = LcCMeshGeometry::create(this);
			if( meshGeometry->load( geometryIt.getElement() ) )
			{
			m_meshGeometries.push_back( meshGeometry );
			}
		}
	}

	for (LcTXmlIterator libControllerIt = LcTXmlIterator(m_root, "library_controllers"); libControllerIt.hasMoreElements(); libControllerIt.next())
	{
		for (LcTXmlIterator controllerIt = LcTXmlIterator(libControllerIt.getElement(), "controller"); controllerIt.hasMoreElements(); controllerIt.next())
		{
			LcTaOwner<LcCMeshController> meshController = LcCMeshController::create(this);
			meshController->load( controllerIt.getElement() );
			m_meshControllers.push_back( meshController );
		}
	}

	// Get nodes. Do this after getting materials, geometries, etc.
	// There is probably only one <library_visual_scenes> tag, but just in case there are more, we iterate here.
	for (LcTXmlIterator libVisualScenesIt = LcTXmlIterator(m_root, "library_visual_scenes"); libVisualScenesIt.hasMoreElements(); libVisualScenesIt.next())
	{
		for (LcTXmlIterator visualScenesIt = LcTXmlIterator(libVisualScenesIt.getElement(), "visual_scene"); visualScenesIt.hasMoreElements(); visualScenesIt.next())
		{
			if (visualScenesIt.getElement()->getAttr("id","").compareNoCase(activeScene) == 0)
			{
				for (LcTXmlIterator nodeIt = LcTXmlIterator(visualScenesIt.getElement(), "node"); nodeIt.hasMoreElements(); nodeIt.next())
				{
					LcTaOwner<LcCMeshNode> meshNode = LcCMeshNode::create(this);
					if( meshNode->load( nodeIt.getElement() ) )
					{
						m_rootMeshNodes.push_back( meshNode );
					}
				}
			}
		}
	}

	m_interleaveByteStride			= ( COORDS_PER_VERTEX  + COORDS_PER_NORMAL ) * sizeof(LcTMeshScalar);
	m_texturedInterleaveByteStride	=
		( COORDS_PER_VERTEX
		+ COORDS_PER_NORMAL
		+ COORDS_PER_TEXCOORD
		+ COORDS_PER_TEXTANGENT
		+ COORDS_PER_TEXBITANGENT
		) * sizeof(LcTMeshScalar);

	// Read in binary data
	LcTmString ndbFilename = sFilename.subString(0, sFilename.length() - sFilename.getWord(-1, '.').length())+ "ndb";
	LcTaOwner<LcCReadOnlyFile> ndbFile = LcCReadOnlyFile::openFile(ndbFilename);

	short arrayCount;	// The number of arrays in the ndb file.

	// Read the very basic binary file header
	ndbFile->read(&arrayCount , sizeof(short), 1);

	ndbFile->read(&m_interleaveByteCount, sizeof(int), 1);
	ndbFile->read(&m_texturedInterleaveByteCount, sizeof(int), 1);
	m_vertexIndex64kByteCount = 0;
	if(  arrayCount > 2 ) // indexed
	{
		//Read the index size
		ndbFile->read(&m_vertexIndex64kByteCount, sizeof(int), 1);
	}

	if( m_interleaveByteCount > 0 ) {
		m_pInterleavedArray.alloc(m_interleaveByteCount / sizeof(LcTMeshScalar) );
		if (ndbFile->read(m_pInterleavedArray, m_interleaveByteCount, 1) == 0)
		{
			return false;
		}
	}

	if( m_texturedInterleaveByteCount > 0 ) {
		m_pTexturedInterleavedArray.alloc(m_texturedInterleaveByteCount / sizeof(LcTMeshScalar) );
		if (ndbFile->read(m_pTexturedInterleavedArray, m_texturedInterleaveByteCount, 1) == 0)
		{
			return false;
		}
	}

	if( m_vertexIndex64kByteCount > 0 ) {
		m_pVertexIndex64k.alloc(m_vertexIndex64kByteCount / sizeof(unsigned short ));
		if (ndbFile->read(m_pVertexIndex64k, m_vertexIndex64kByteCount, 1) == 0)
		{
			return false;
		}
	}
	return true;
}

LC_EXPORT_VIRTUAL LcTaOwner<LcCMeshMaterial> LcCMesh::createMeshMaterial(LcCMesh* ownerMesh)
{
	return LcCMeshMaterial::create(ownerMesh);
}

LC_EXPORT LcTaOwner<LcCMeshMaterial> LcCMeshMaterial::create(LcCMesh* ownerMesh)
{
	LcTaOwner<LcCMeshMaterial> ref;
	ref.set(new LcCMeshMaterial(ownerMesh));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT LcCMeshMaterial::LcCMeshMaterial(LcCMesh*  ownerMesh)
{
	m_ownerMesh = ownerMesh;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL LcCMeshMaterial::~LcCMeshMaterial()
{
	clean();
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void LcCMeshMaterial::clean()
{

}

LC_EXPORT bool LcCMeshMaterial::load(LcCXmlElem* materialEl)
{
	m_id = materialEl->getAttr("id");

	LcTaString instanceEffectURL = "";
	LcCXmlElem* instEffect = materialEl->find("instance_effect");
	if (instEffect)
	{
		instanceEffectURL = instEffect->getAttr("url").subString(1, 0x7ffff);
	}

	for (LcTXmlIterator libEffectsIt = LcTXmlIterator(m_ownerMesh->getRoot(), "library_effects"); libEffectsIt.hasMoreElements(); libEffectsIt.next())
	{
		LcCXmlElem* effectEl = LcCMesh::findElem( libEffectsIt.getElement(), "effect", "id", instanceEffectURL.bufUtf8());

		if (effectEl)
		{
			LcCXmlElem* imageEl = effectEl->find("image");
		
			if( imageEl )
			{
				LcCXmlElem* initFromEl = imageEl->find("init_from");				
				if( initFromEl )
				{
					// Get texture file name, discarding path information
					m_sTexture = initFromEl->getFirstChild()->getText();
					// Different tools use different separators
					m_sTexture.replace('\\', NDHS_DIR_SEP_CHAR);
					m_sTexture = m_sTexture.getWord(-1, NDHS_DIR_SEP_CHAR);
					m_textureCount++;
				}
			}

			LcCXmlElem* phongEl = NULL;
			LcCXmlElem* profileEl = effectEl->find("profile_COMMON");
			if (profileEl)
			{
				LcCXmlElem* techniqueEl = profileEl->find("technique");
				if (profileEl)
				{
					phongEl = techniqueEl->find("phong");
				}
			}				

			if( phongEl == NULL )
				return true;

			LcTmString name;
			for( LcCXmlElem*  pEl = phongEl->getFirstChild(); pEl; pEl = pEl->getNext() )
			{
				if(pEl->hasName("emission") )
				{
					LcCMesh::fillColorArrayFromColorElement(pEl->find("color"), m_afEmissive);
				}
				else if(pEl->hasName("ambient") )
				{
					LcCMesh::fillColorArrayFromColorElement(pEl->find("color"), m_afAmbient);
				}
				else if(pEl->hasName("diffuse") )
				{
					if (pEl->find("color"))
					{
						LcCMesh::fillColorArrayFromColorElement(pEl->find("color"), m_afDiffuse);
					}
					else if (pEl->find("texture"))
					{
						// Set diffuse to (1,1,1,1) or the texture won't show
						for (int j = 0; j < 4; j++)
						{
							m_afDiffuse[j]	= LC_MESH_UNIT_FROM_FLOAT(1.0);
						}
					}
				}
				else if(pEl->hasName("specular") )
				{
					LcCMesh::fillColorArrayFromColorElement(pEl->find("color"), m_afSpecular);
				}
				else if(pEl->hasName("shininess") )
				{
					m_fShininess = LC_MESH_FROM_FLOAT(pEl->find("float")->getFirstChild()->getText().toScalar());
				}
			}
		}
	}
	return true;
}



LC_EXPORT LcTaOwner<LcCMeshController> LcCMeshController::create(LcCMesh* ownerMesh)
{
	LcTaOwner<LcCMeshController> ref;
	ref.set(new LcCMeshController(ownerMesh));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT LcCMeshController::LcCMeshController(LcCMesh*  ownerMesh)
{
	m_ownerMesh = ownerMesh;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL LcCMeshController::~LcCMeshController()
{
	clean();
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void LcCMeshController::clean()
{

}

LC_EXPORT bool LcCMeshController::load(LcCXmlElem* controllerEl)
{
	m_id = controllerEl->getAttr("id");

	for (LcTXmlIterator morphIt = LcTXmlIterator(controllerEl, "morph"); morphIt.hasMoreElements(); morphIt.next())
	{
		LcCXmlElem* morphEl = morphIt.getElement();
		for (LcTXmlIterator targetsIt = LcTXmlIterator(morphEl, "targets"); targetsIt.hasMoreElements(); targetsIt.next())
		{
			LcCXmlElem* targetsEl = targetsIt.getElement();
			for (LcTXmlIterator targetInputIt = LcTXmlIterator(targetsEl, "input"); targetInputIt.hasMoreElements(); targetInputIt.next())
			{
				LcCXmlElem* targetInputEl = targetInputIt.getElement();
				if (targetInputEl->getAttr("semantic").compareNoCase("MORPH_TARGET") == 0)
				{
					LcTaString targetSourceURL = targetInputEl->getAttr("source").subString(1, 0x7ffff);
					LcCXmlElem* sourceEl = LcCMesh::findElem( morphEl, "source", "id", targetSourceURL.bufUtf8() );
					if (sourceEl)
					{
						LcCXmlElem* idfArrayEl = sourceEl->find("IDREF_array");
						if (idfArrayEl)
						{
							LcTXMLTextTokenizer tok = LcTXMLTextTokenizer(idfArrayEl->getFirstChild());
							// Assume only one target
							m_targetGeometryUrl = tok.getToken();
						}
					}
				}
			}
		}
	}
	return resolveTargetGeometry();
}

LC_EXPORT bool LcCMeshController::resolveTargetGeometry()
{
	bool found = false;
	for(unsigned i = 0; !found && i < m_ownerMesh->m_meshGeometries.size(); i++ )
	{
		if( m_targetGeometryUrl.length() > 0 && 0 == m_targetGeometryUrl.compareNoCase( m_ownerMesh->m_meshGeometries[i]->id() ) ) 
		{
			m_targetGeometry = m_ownerMesh->m_meshGeometries[i];
			found = true;
		}
	}
	return found;
}

LC_EXPORT LcTaOwner<LcCMeshNode> LcCMeshNode::create(LcCMesh* ownerMesh)
{
	LcTaOwner<LcCMeshNode> ref;
	ref.set(new LcCMeshNode(ownerMesh));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT LcCMeshNode::LcCMeshNode(LcCMesh*  ownerMesh)
{
	m_ownerMesh = ownerMesh;
	m_hasOwnCoordinateSpace = true;
	m_localCoordinateSpace = LcTTransform::identity();
	m_globalCoordinateSpace = LcTTransform::identity();
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL LcCMeshNode::~LcCMeshNode()
{
	clean();
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void LcCMeshNode::clean()
{

}

/*-------------------------------------------------------------------------*/
/**

*/
LC_EXPORT bool LcCMeshNode::load(LcCXmlElem* nodeEl)
{
	bool found = false;
	float v[16];

	m_id = nodeEl->getAttr("id");

	// Look for IFX instance geometry
	for (LcTXmlIterator extraIt = LcTXmlIterator(nodeEl, "extra"); extraIt.hasMoreElements(); extraIt.next())
	{
		LcCXmlElem* pExtraEl = extraIt.getElement();
		LcCXmlElem* techniqueEl = pExtraEl->find("technique");
		if(techniqueEl) 
		{
			LcTaString profile = techniqueEl->getAttr("profile");

			LcCXmlElem* nd3E1 = techniqueEl->find("nd3");
			if(nd3E1 != NULL)
			{
				m_hasOwnCoordinateSpace = m_hasOwnCoordinateSpace 
											&& ((nd3E1->getAttr("has_coordinate_space","true")).compareNoCase("true") == 0);
			}
			
			LcCXmlElem* instanceGeometryEl = techniqueEl->find("ig");
			if(!found && instanceGeometryEl != NULL)
			{
				int instanceGeometryIndex = instanceGeometryEl->getAttr("gi").toInt();
				m_instanceGeometry = m_ownerMesh->m_meshGeometries[instanceGeometryIndex];
				m_instanceGeometryUrl = instanceGeometryEl->getAttr("url");

				for (LcTXmlIterator mbIt = LcTXmlIterator(instanceGeometryEl, "mb"); mbIt.hasMoreElements(); mbIt.next())
				{
					LcCXmlElem* pMaterialBindingEl = mbIt.getElement();
					int instanceMaterialIndex = pMaterialBindingEl->getAttr("mi").toInt();
					m_instanceMaterials.push_back( m_ownerMesh->m_meshMaterials[instanceMaterialIndex] );
					found = true;
				}
			}
		}
	}

	// Get the rest of the node data, and sub nodes
	for( LcCXmlElem* pElement = nodeEl->getFirstChild(); pElement != NULL; pElement = pElement->getNext() )
	{
		// Only recognise matrix tags
		if(pElement->hasName("matrix"))
		{
			LcTXMLTextTokenizer tok = LcTXMLTextTokenizer(pElement->getFirstChild());
			for (unsigned t = 0; t < (4 * 4); t++)
			{
				v[t] = tok.getScalarTokenNext();
			}
			m_globalCoordinateSpace.setFromRowMajorMatrixArray(v);
		}
		else if( pElement->hasName("instance_controller") )
		{
			LcCXmlElem*  pInstanceControllerEl = pElement;
			m_instanceControllerUrl = pInstanceControllerEl->getAttr("url").subString(1, 0x7ffff);
			resolveInstanceController();
		}
		else if(pElement->hasName("node"))
		{
			LcTaOwner<LcCMeshNode> subNode = LcCMeshNode::create( m_ownerMesh );
			subNode->parentMeshNode = this;
			if( subNode->load( pElement ) )
			{
				m_subNodes.push_back( subNode );
			}
		}
		else if(pElement->hasName("instance_node"))
		{
			// There should be no instance nodes - they are expanded to nodes in the importer.
		}
	}
	return true;
}

LC_EXPORT bool LcCMeshNode::resolveInstanceGeometry()
{
	bool found = false;
	for(unsigned i = 0; !found && i < m_ownerMesh->m_meshGeometries.size(); i++ )
	{
		if( m_instanceGeometryUrl.length() > 0 && 0 == m_instanceGeometryUrl.compareNoCase( m_ownerMesh->m_meshGeometries[i]->id() ) )
		{
			m_instanceGeometry = m_ownerMesh->m_meshGeometries[i];
			found = true;
		}
	}
	return found;
}


LC_EXPORT bool LcCMeshNode::resolveInstanceController()
{
	bool found = false;
	for(unsigned i = 0; !found && i < m_ownerMesh->m_meshControllers.size(); i++ )
	{
		if( m_instanceControllerUrl.length() > 0 && 0 == m_instanceControllerUrl.compareNoCase( m_ownerMesh->m_meshControllers[i]->id() ) )
		{
			m_instanceController = m_ownerMesh->m_meshControllers[i];
			found = true;
		}
	}
	return found;
}


// LcCMeshGeometry

LC_EXPORT LcTaOwner<LcCMeshGeometry> LcCMeshGeometry::create(LcCMesh* ownerMesh)
{
	LcTaOwner<LcCMeshGeometry> ref;
	ref.set(new LcCMeshGeometry(ownerMesh));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT LcCMeshGeometry::LcCMeshGeometry(LcCMesh*  ownerMesh)
{
	m_ownerMesh = ownerMesh;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL LcCMeshGeometry::~LcCMeshGeometry()
{
	clean();
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void LcCMeshGeometry::clean()
{

}

LC_EXPORT bool LcCMeshGeometry::load(LcCXmlElem* geometryEl )
{
	m_id = geometryEl->getAttr("id");

	LcCXmlElem* meshEl = geometryEl->find("mesh");

	for (LcTXmlIterator trianglesIt = LcTXmlIterator(meshEl, "triangles"); trianglesIt.hasMoreElements(); trianglesIt.next())
	{
		LcTaOwner<LcCMeshTriangleGroup> triangleGroup = LcCMeshTriangleGroup::create(m_ownerMesh);
		if( triangleGroup->load( trianglesIt.getElement() ) )
		{
			m_triangleGroups.push_back( triangleGroup );
		}
	}
	return true;
}


// LcCMeshTriangleGroup

LC_EXPORT LcTaOwner<LcCMeshTriangleGroup> LcCMeshTriangleGroup::create(LcCMesh* ownerMesh)
{
	LcTaOwner<LcCMeshTriangleGroup> ref;
	ref.set(new LcCMeshTriangleGroup(ownerMesh));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT LcCMeshTriangleGroup::LcCMeshTriangleGroup(LcCMesh*  ownerMesh)
{
	m_ownerMesh = ownerMesh;
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT_VIRTUAL LcCMeshTriangleGroup::~LcCMeshTriangleGroup()
{
	clean();
}

/*-------------------------------------------------------------------------*//**
																			 */
LC_EXPORT void LcCMeshTriangleGroup::clean()
{

}


LC_EXPORT bool LcCMeshTriangleGroup::load(LcCXmlElem* triangleGroupEl )
{
	LcCXmlElem* extraEl = triangleGroupEl->find("extra");

	m_isTextured = false;

	int verticesOffset = 0;
	int normalsOffset = 1;
	int texcoordsOffset = 2;
	int tangentsOffset = 3;	
	int bitangentsOffset = 4;	

	if(extraEl) 
	{
		// We have an <extra> so look for the <index>
		LcCXmlElem* techniqueEl = extraEl->find("technique");
		if(techniqueEl) 
		{
			LcTaString profile = techniqueEl->getAttr("profile");

			LcCXmlElem* meshGeometryEl = techniqueEl->find("tg");
			if(meshGeometryEl) 
			{
				m_isTextured = ( meshGeometryEl->getAttr("txd").compare("true") == 0 );
				m_indexArrayIndex = meshGeometryEl->getAttr("ib").toInt();

				m_cFaces = meshGeometryEl->getAttr("tc").toInt();
				m_cVertices = m_cFaces * VERTICES_PER_TRIANGLE;

				m_floatOffset = meshGeometryEl->getAttr("fo").toInt();
				m_floatCount = meshGeometryEl->getAttr("fc").toInt();

				m_positionsFloatOffset = m_floatOffset;

				m_normalsFloatOffset = m_positionsFloatOffset + COORDS_PER_VERTEX;

				if( !m_isTextured )
				{
					m_iFirstVertex = m_floatOffset  / ( COORDS_PER_VERTEX + COORDS_PER_NORMAL );
				}
				else
				{
					m_iFirstVertex =  m_floatOffset / (
						COORDS_PER_VERTEX
						+ COORDS_PER_NORMAL
						+ COORDS_PER_TEXCOORD
						+ COORDS_PER_TEXTANGENT
						+ COORDS_PER_TEXBITANGENT
						) ;

					m_texcoordsFloatOffset = m_normalsFloatOffset + COORDS_PER_NORMAL;

					m_tangentsFloatOffset = m_texcoordsFloatOffset + COORDS_PER_TEXCOORD;

					m_bitangentsFloatOffset = m_tangentsFloatOffset + COORDS_PER_TEXTANGENT;
				}
				return true;
			}
		}
	}
	return false;
}


/*-------------------------------------------------------------------------*//**
 Fill 'floatArray' from the <float_array> tag under <source> element * 'sourceEl'.
 Return true if success, otherwise false

 PRE: sourceEl != NULL
																			 */

LC_EXPORT bool LcCMesh::fillFloatArrayFromSourceElement(LcCXmlElem* sourceEl, LcTaArray<float>& floatArray)
{
		LcCXmlElem* el = sourceEl->find("float_array");
	if (!el || !el->getFirstChild())
	{
		return false;
	}

	LcTXMLTextTokenizer tok = LcTXMLTextTokenizer(el->getFirstChild());
	int count = el->getAttr("count").toInt();
	for(int i = 0; i < count; i++)
	{ 
		if (!tok.hasMoreTokens())
		{
			return false;
		}
		floatArray.push_back( tok.getScalarTokenNext() );
	}

		return true;
}

/*-------------------------------------------------------------------------*//**
 Fill the color array 'colorArray' from 'colorEl' tag
 Return true if success, otherwise false
																			 */
LC_EXPORT bool LcCMesh::fillColorArrayFromColorElement(LcCXmlElem* colorEl, LcTMeshUnitScalar colorArray[4])
{
	if (!colorEl || !colorEl->getFirstChild())
	{
		return false;
	}

	LcTXMLTextTokenizer tok = LcTXMLTextTokenizer(colorEl->getFirstChild());
	for (int j = 0; j < 4; j++)
	{
		if (!tok.hasMoreTokens())
		{
			return false;
		}

		colorArray[j] = LC_MESH_UNIT_FROM_FLOAT(tok.getScalarTokenNext());
	}
	return true;
}


/*-------------------------------------------------------------------------*//**
Find the element which is a child of 'pLocalRoot' with the name *'pName'
and where *'pAttribute' == *pAttributeValue

Return NULL if not found.

PRE: pLocalRoot != NULL
PRE: pName != NULL
PRE: pAttribute != NULL
PRE: pAttributeValue != NULL
																			 */
LC_EXPORT LcCXmlElem* LcCMesh::findElem(LcCXmlElem* pLocalRoot, const char* pName, const char* pAttribute, const char* pAttributeValue)
{
	bool found = false;
	LcTXmlIterator it = LcTXmlIterator(pLocalRoot, pName);
	while (!found && it.hasMoreElements())
	{
		if (it.getElement()->getAttr(pAttribute).compareNoCase(pAttributeValue) == 0)
		{
			found = true;
		}
		else
		{
			it.next();
		}
	}
	if (found)
	{
		return it.getElement();
	}
	else
	{
		return NULL;
	}
}


#endif // defined(LC_USE_MESHES)
