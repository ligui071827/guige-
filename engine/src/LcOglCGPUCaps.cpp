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


#if	defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcOglCGPUCaps> LcOglCGPUCaps::create()
{
	LcTaOwner<LcOglCGPUCaps> ref;
	ref.set(new LcOglCGPUCaps);
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcOglCGPUCaps::~LcOglCGPUCaps()
{
	m_extensionMap.clear();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcOglCGPUCaps::isExtensionSupported (const LcTmString &extensionString)
{
	// Make sure we got valid extension
	if (extensionString.isEmpty())
		return false;

	// Convert to lower case to enforce lookup consistency
	LcTaString lookupExtension = extensionString.toLower();

	// Lookup extension in our map
	TmExtensionMap::iterator it = m_extensionMap.find(lookupExtension);

	// if extension not present put it in our map
	if (it == m_extensionMap.end())
	{
		// Check whether extension supported or not
		if (IFXP_Check_GL_Extension(lookupExtension.bufUtf8()) != IFX_SUCCESS)
		{
			// Mark extension false in our map, so that we can retrieve it fast in future
			m_extensionMap[lookupExtension] = false;
			return false;
		}
		else
		{
			// Mark extension true in our map, so that we can retrieve it fast in future
			m_extensionMap[lookupExtension] = true;
			return true;
		}
	}
	return it->second;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTPixelDim LcOglCGPUCaps::getTextureMaxSize()
{
	GLint texture_dimensions_max = 0;

	glGetIntegerv (GL_MAX_TEXTURE_SIZE, &texture_dimensions_max);

	return LcTPixelDim (texture_dimensions_max, texture_dimensions_max);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTPixelDim LcOglCGPUCaps::getCubeTextureMaxSize()
{
	GLint cube_texture_dimensions_max = 0;

	// Cube maps are only supported by OpenGL ES 2.0
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	glGetIntegerv (GL_MAX_CUBE_MAP_TEXTURE_SIZE, &cube_texture_dimensions_max);
#endif

	return LcTPixelDim (cube_texture_dimensions_max, cube_texture_dimensions_max);
}
#endif // defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20)
