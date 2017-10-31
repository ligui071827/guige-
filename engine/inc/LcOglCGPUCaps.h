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
#ifndef LcOglCGPUCapsH
#define LcOglCGPUCapsH

#if	defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20)

#include "inflexionui/engine/inc/LcCBase.h"

/*---------------------------------------------
----------------------------*//**
	Class that encapsulates an OpenGL/EGL extensions and other GPU capabilities
*/
class LcOglCGPUCaps : public LcCBase
{
private:
	typedef	LcTmMap<LcTmString, bool> 	TmExtensionMap;

	// Map to hold supported OGL Extensions
	TmExtensionMap 						m_extensionMap;

	// Maximum 2D texture Size
	LcTPixelDim							m_textureSizeMax;

	// Maximum Cube texture Size
	LcTPixelDim							m_cubeTextureSizeMax;

protected:

public:
	//Construction
	LC_IMPORT		static LcTaOwner<LcOglCGPUCaps>
										create();
	LC_VIRTUAL							~LcOglCGPUCaps();

	LC_IMPORT		bool				isExtensionSupported (const LcTmString &extensionString);

	LC_IMPORT		LcTPixelDim			getTextureMaxSize();
	LC_IMPORT		LcTPixelDim			getCubeTextureMaxSize();
};

#endif // defined(IFX_RENDER_DIRECT_OPENGL) || defined(IFX_RENDER_DIRECT_OPENGL_20)

#endif // LcOglCGPUCapsH
