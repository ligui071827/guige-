/***************************************************************************
*
*			   Copyright 2006 Mentor Graphics Corporation
*						 All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
****************************************************************************/

#include "inflexionui/engine/inc/LcAll.h"

#ifdef LC_HDRSTOP
	#pragma	hdrstop
#endif

#if	defined(LC_PLAT_OGL)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcOglCContext> LcOglCContext::create(LcCSpace* space)
{
	LcTaOwner<LcOglCContext> ref;
	ref.set(new	LcOglCContext);
	ref->construct(space);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCContext::construct(LcCSpace* space)
{
	m_space	= space;

#if	defined	(LC_PLAT_OGL_20)

	m_openGLInitialized	= false;

	m_oglGlobalState = LcOglCGlobalState::create(space);
	m_oglGlobalState->initShaderDataType();
	m_oglGlobalState->initEngineMappingTable();

#endif

	// Create instance of GPU Capabilities class
	m_gpuCaps = LcOglCGPUCaps::create();

	m_ambientLightColor	= LcTColor::NONE;
	m_primaryLightColor	= LcTColor::NONE;
	m_currentlySetAmbientLightColor	= LcTColor::NONE;
	m_currentlySetPrimaryLightColor	= LcTColor::NONE;
}

LC_EXPORT_VIRTUAL LcOglCContext::~LcOglCContext()
{
	cleanup();
#if	defined(LC_PLAT_OGL_20)
	m_customEffectMap.clear();
	m_cubemapMap.clear();
#endif
}

LC_EXPORT void LcOglCContext::cleanup()
{
	#if	defined(LC_PLAT_OGL_20)
	TmMCubemapMap::iterator it = m_cubemapMap.begin();

	GLuint tex=0;
	for(;it!=m_cubemapMap.end();it++)
	{
		tex= it->second;
		glDeleteTextures(1,&tex);
	}
	m_cubemapMap.clear();
	#endif
}

#if	defined(LC_PLAT_OGL_20)
/* --------------------	 Setup for OpenGL ES 2.0 -------------------------
	Do all one-off common OpenGL setup here	- lighting,	hints etc.
	These are the default cases.  Where	a method changes the default it	should
	restore	it afterwards so that other	methods	can	assume the defaults	are	in
	place, for efficiency.	E.g. if	the	default	is blending	enabled, any method
	that disables blending should re-enable	it afterwards.

	These defaults are chosen to suit the basic	behavior of	LcOglCImage
	and	LcOglCFont classes.	 Applications may override or extend these defaults
	after initialization, e.g. if lighting is required.
*/

LC_EXPORT void LcOglCContext::setupOpenGL()
{
	// Ensure that our cached state	is reset
	clearCache();

	m_openGLInitialized	= true;

	m_oglGlobalState->enableGlobalLighting();

	m_primaryLightEnabled =	true;
	m_highQualityEnabled = false;

	setEffectUsageIndex(-1);

	// Use blending on demand (disabled by default)
	glDisable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE);

	// Image/font classes both use textures, so	set	up as default
	glEnable(GL_TEXTURE_2D);

	// By default assume we	have no	row-packing	unless specified
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// No culling by default, but in case enabled -	image/font use CCW face	as front
	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);

#if defined(IFX_USE_DITHERING) && !defined(NDHS_JNI_INTERFACE)
	if(m_enableDithering)
		glEnable(GL_DITHER);
	else
#endif
		glDisable(GL_DITHER);

	// Disable MSAA by default (enabled around the parts that need it)
	glDisable(GL_MULTISAMPLE);

	// Default to off, as this is what the cache function below	defaults to.
	glDepthMask(GL_FALSE);

	glEnable(GL_DEPTH_TEST);

	LC_OGL_X(glClearColor)(	LC_OGL_FROM_SCALAR(0),
							LC_OGL_FROM_SCALAR(0),
							LC_OGL_FROM_SCALAR(0),
							LC_OGL_FROM_SCALAR(0));

#ifndef	LC_PAINT_FULL
	// Ensure the color	and	depth buffers are cleared in partial paint mode.
	glClear(GL_COLOR_BUFFER_BIT	| GL_DEPTH_BUFFER_BIT);
#endif

	// Check for the presence of the ProgramBinary extension.
	if(getGPUCaps()->isExtensionSupported("GL_OES_get_program_binary"))
	{
#ifdef LC_USE_EGL
		m_fn_glProgramBinaryOES = (PFNGLPROGRAMBINARYOESPROC)eglGetProcAddress("glProgramBinaryOES");
		m_fn_glGetProgramBinaryOES = (PFNGLGETPROGRAMBINARYOESPROC)eglGetProcAddress("glGetProgramBinaryOES");
#else
		m_fn_glProgramBinaryOES = &glProgramBinaryOES;
		m_fn_glGetProgramBinaryOES = &glGetProgramBinaryOES;
#endif
		if(m_fn_glProgramBinaryOES && m_fn_glGetProgramBinaryOES)
		{
			m_bProgramBinarySupported = true;
		}
	}
}

#else

/* --------------------	 Setup for OpenGL ES 1.1 -------------------------*/
LC_EXPORT void LcOglCContext::setupOpenGL()
{
	// Lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Ensure that our cached state	is reset
	clearCache();

	m_primaryLightEnabled =	true;

	// Use blending on demand (disabled by default)
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Arrays used by image/font classes
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	// Image/font classes both use textures, so	set	up as default
	glEnable(GL_TEXTURE_2D);
	LC_OGL_IX(glTexEnv)(GL_TEXTURE_ENV,	GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// By default assume we	have no	row-packing	unless specified
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// No culling by default, but in case enabled -	image/font use CCW face	as front
	glFrontFace(GL_CCW);
	glDisable(GL_CULL_FACE);

#if defined(IFX_USE_DITHERING) && !defined(NDHS_JNI_INTERFACE)
	if(m_enableDithering)
		glEnable(GL_DITHER);
	else
#endif
		glDisable(GL_DITHER);

	// Disable MSAA by default (enabled around the bits that need it)
	glDisable(GL_MULTISAMPLE);

	glEnable(GL_NORMALIZE);

	// Default to off, as this is what the cache function below	defaults to.
	glDepthMask(GL_FALSE);

	glEnable(GL_DEPTH_TEST);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	LC_OGL_X(glClearColor)(	LC_OGL_FROM_SCALAR(0),
							LC_OGL_FROM_SCALAR(0),
							LC_OGL_FROM_SCALAR(0),
							LC_OGL_FROM_SCALAR(0));

#ifndef	LC_PAINT_FULL
	// Ensure the color	and	depth buffers are cleared in partial paint mode.
	glClear(GL_COLOR_BUFFER_BIT	| GL_DEPTH_BUFFER_BIT);
#endif
}

#endif /* #if defined(LC_PLAT_OGL_20) */

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCContext::transformsChanged(LcCWidget* w, LcTTransform& xfm)
{
	LcTTransform modelView;

	// If widget provided...
	if (w)
		modelView = w->getXfmToGlobal() ;
	else
		modelView = LcTTransform::identity();

	modelView.multiply( xfm );

	transformsChanged(modelView);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCContext::transformsChanged(LcCWidget* w)
{
	LcTTransform modelView;

	// If widget provided...
	if (w)
		modelView = w->getXfmToGlobal() ;
	else
		modelView = LcTTransform::identity();

	transformsChanged(modelView);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCContext::transformsChanged(LcTTransform& xfm)
{
#if	defined	(LC_PLAT_OGL_20)
	LcOglCEffect *effect = getCurrentEffect();
#endif

	// ...set modelview according to its local coord space
	LcOglTScalar ogd[16];
	xfm.getData(ogd);

#if	defined	(LC_PLAT_OGL_20)
	LcOglTScalar invt [16];
	LcTTransform M = xfm;
		M.invert().transpose().getData(invt);

		if(effect)
		{
			effect->setModelViewMatrix(ogd);
			effect->setNormalMatrix(invt);
		}

#else
		LC_OGL_FX(glLoadMatrix)(ogd);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCContext::boundsUpdated()
{
	// Get canvas properties
	LcTPixelRect canvasBounds	= m_space->getCanvasBounds();
	LcTVector globalExtent		= m_space->getGlobalExtent();
	LcTPixelPoint ptOrigin2D	= m_space->getCanvasOrigin2D();

#if	defined(LC_OGL_DIRECT)

	// In OGL Direct mode, we have to apply	the	X and Y	offsets	ourselves.
	// The IFXP	layer can not do it	for	us.

	// This	modification sets up OpenGL	to a viewport that is smaller
	// than	the	display, and adjusts the perceived canvas size so that it
	// will	discard	anything that is rendered outside its boundary rather
	// than	scaling	the	scene.

	// Setting the viewport	alone moves	the	corner position, however OpenGL
	// scales the scene	to fit rather than cropping.
	IFX_DISPLAY* display = m_space->getDisplay();

	// OpenGL works	with bottom	left = 0,0 and we want top left	to be 0,0
	// so apply	an offset if the screen	dimensions do not match	the	canvas.
	int	yOffset	= display->height -	canvasBounds.getHeight();

	m_xOffset =	display->offsetX;
	m_yOffset =	-display->offsetY +	yOffset;

	// Update the viewport.
	glViewport(	m_xOffset,
				m_yOffset,
				canvasBounds.getWidth(),
				canvasBounds.getHeight());

#else

	// Update the viewport.
	glViewport(	0,
				0,
				canvasBounds.getWidth(),
				canvasBounds.getHeight());

#endif

	// In future, local	perspectives may require projection	changes	between
	// paint operations, i.e. in the transformsChanged() handler.  But for now,
	// changing	bounds is the only action which	can	result in projection changing

	// The eye distance	should be in proportion	with the x,y scales	to cope	with
	// rotated widgets properly, so	by default we use the width
	LcTScalar dEye = globalExtent.z;

	// Position	of eye within bounds, normalized to	0.0	to 1.0
	LcTScalar widthRatio  =	(LcTScalar(ptOrigin2D.x) - LcTScalar(canvasBounds.getLeft())) /	LcTScalar(canvasBounds.getWidth());
	LcTScalar heightRatio =	(LcTScalar(ptOrigin2D.y) - LcTScalar(canvasBounds.getTop())) / LcTScalar(canvasBounds.getHeight());

	// Halving accounts	for	scaling	down to	near plane halfway between eye and z=0
	LcTScalar dNearWidth  =	(globalExtent.x	/ 2);
	LcTScalar dNearHeight =	(globalExtent.y	/ 2);

	// Perspective
	LcTTransform4 xfmProjection	= LcTTransform4::frustum(
		-dNearWidth	* widthRatio,
		dNearWidth * (1	- widthRatio),
		-dNearHeight * (1 -	heightRatio),
		dNearHeight	* heightRatio,
		dEye / 2,
		dEye * 2);

	// At this point the transform gives us	normalized x,y i.e.	-1 to +1
	// Must	move back by the eye distance so that the eye is at	the	origin.
	LcTTransform xfmEye	= LcTTransform::identity().translatePrefix(0, 0, -dEye);
	xfmProjection.multiply(xfmEye);

#if	!defined (LC_PLAT_OGL_20)
	// Change projection matrix
	glMatrixMode(GL_PROJECTION);
#endif

	// Apply matrix	calculated above
	LcOglTScalar ogd[16];
	xfmProjection.getData(ogd);

#if	!defined (LC_PLAT_OGL_20)
	LC_OGL_FX(glLoadMatrix)(ogd);

	// Change modelview	matrix
	glMatrixMode(GL_MODELVIEW);
#else

	LcOglCEffect* effect = getCurrentEffect();

	if (effect)
	{
		effect->setProjectionMatrix(ogd);
	}

#endif
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::setMaterialAmbient(LcOglTScalar* param)
{
	if (memcmp(param, m_curMatAmb, sizeof(LcOglTScalar)	* 4))
	{
		#if	defined	(LC_PLAT_OGL_20)
			LcOglCEffect* effect = getCurrentEffect();
			if(effect)
			{
				effect->setAmbientMaterial(param);
			}
		#else
			LC_OGL_FXV(glMaterial)(GL_FRONT_AND_BACK, GL_AMBIENT, param);
		#endif

		memcpy(m_curMatAmb,	param, sizeof(LcOglTScalar)	* 4);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::setMaterialDiffuse(LcOglTScalar* param)
{
	if (memcmp(param, m_curMatDif, sizeof(LcOglTScalar)	* 4))
	{
#if	defined	(LC_PLAT_OGL_20)
		LcOglCEffect* effect = getCurrentEffect();
		if(effect)
		{
			effect->setDiffuseMaterial(param);
		}
#else
		LC_OGL_FXV(glMaterial)(GL_FRONT_AND_BACK, GL_DIFFUSE, param);
#endif

		memcpy(m_curMatDif,	param, sizeof(LcOglTScalar)	* 4);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::setMaterialSpecular(LcOglTScalar* param)
{
	// Hold the param value in a local variable for manipulation
	LcOglTScalar localParam[] = { 0.0, 0.0, 0.0, 1.0 } ;

	// Initialize with the incoming param value
	memcpy(localParam, param, sizeof(LcOglTScalar) * 4);

	// Combine material specular colors from class and source
	combineMaterialSpecularColors (localParam);

	// Compare with currently maintained material specular color
	if (memcmp(localParam, m_curMatSpc, sizeof(LcOglTScalar) * 4))
	{
#if	defined (LC_PLAT_OGL_20)
		LcOglCEffect* effect = getCurrentEffect();
		if(effect)
		{
			effect->setSpecularMaterial(localParam);
		}
#else
		LC_OGL_FXV(glMaterial)(GL_FRONT_AND_BACK, GL_SPECULAR, localParam);
#endif

		memcpy(m_curMatSpc, localParam, sizeof(LcOglTScalar) * 4);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::setMaterialEmission(LcOglTScalar* param)
{
	// Hold the param value in a local variable for manipulation
	LcOglTScalar localParam[] = { 0.0, 0.0, 0.0, 1.0 } ;

	// Initialize with the incoming param value
	memcpy(localParam, param, sizeof(LcOglTScalar) * 4);

	// Combine material emissive colors from class and source
	combineMaterialEmissiveColors (localParam);

	// Compare with currently maintained material emissive color
	if (memcmp(localParam, m_curMatEmm, sizeof(LcOglTScalar) * 4))
	{
#if defined (LC_PLAT_OGL_20)
		LcOglCEffect* effect = getCurrentEffect();
		if(effect)
		{
			effect->setEmissionMaterial(localParam);
		}
#else
		LC_OGL_FXV(glMaterial)(GL_FRONT_AND_BACK, GL_EMISSION, localParam);
#endif

		memcpy(m_curMatEmm, localParam, sizeof(LcOglTScalar) * 4);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::setMaterialShininess(LcOglTScalar param)
{
	// Hold the param value in a local variable for manipulation
	LcOglTScalar localParam = param;

	// Combine material specular shininess factors from class and source
	combineMaterialSpecularShininess (&localParam);

	if (localParam != m_curMatShi)
	{
#if defined (LC_PLAT_OGL_20)
		LcOglCEffect* effect = getCurrentEffect();
		if(effect)
		{
			effect->setShininessMaterial(localParam);
		}
#else
		LC_OGL_FX(glMaterial)(GL_FRONT_AND_BACK, GL_SHININESS, localParam);
#endif

		m_curMatShi = localParam;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::setMaterialFlatColor(LcTColor rgb, LcTUnitScalar opacity)
{
	// Set to matte
	LcOglTScalar color[4] =	{ 0, 0,	0, 1 };
	setMaterialSpecular(color);
	setMaterialEmission(color);
	setMaterialShininess(0);

	// Expand given	color into array format
	color[0] = LC_OGL_FROM_SCALAR(LcTScalar(int(rgb.rgba.r)) / 255);
	color[1] = LC_OGL_FROM_SCALAR(LcTScalar(int(rgb.rgba.g)) / 255),
	color[2] = LC_OGL_FROM_SCALAR(LcTScalar(int(rgb.rgba.b)) / 255),
	color[3] = LC_OGL_FROM_UNIT_SCALAR(opacity);

	// Apply ambient and diffuse together (if either have changed)
	if (memcmp(color, m_curMatAmb, sizeof(LcOglTScalar)	* 4)
	||	memcmp(color, m_curMatDif, sizeof(LcOglTScalar)	* 4))
	{
#if	defined	(LC_PLAT_OGL_20)
		LcOglCEffect* effect = getCurrentEffect();
		if(effect)
		{
			effect->setAmbientMaterial(color);
			effect->setDiffuseMaterial(color);
		}
#else
		LC_OGL_FXV(glMaterial)(GL_FRONT_AND_BACK, GL_AMBIENT, color);
		LC_OGL_FXV(glMaterial)(GL_FRONT_AND_BACK, GL_DIFFUSE, color);
#endif

		memcpy(m_curMatAmb,	color, sizeof(LcOglTScalar)	* 4);
		memcpy(m_curMatDif,	color, sizeof(LcOglTScalar)	* 4);
	}
}

// Combine Material	properties from	source and classes section
/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::combineMaterialSpecularColors (LcOglTScalar* sourceColor)
{
	LcOglTScalar classesSectionColor[4]	= {	0.0, 0.0, 0.0, 1.0 } ;
	LcTColor color = LcTColor::BLACK;

	// Get the material	specular color associatd with this paint widget
	if (m_space->getPaintWidget())
		color =	m_space->getPaintWidget()->getMaterialSpecularColor();

	// Expand given	color into array format
	classesSectionColor[0] = LC_OGL_FROM_SCALAR(LcTScalar(int(color.rgba.r)) / 255);
	classesSectionColor[1] = LC_OGL_FROM_SCALAR(LcTScalar(int(color.rgba.g)) / 255);
	classesSectionColor[2] = LC_OGL_FROM_SCALAR(LcTScalar(int(color.rgba.b)) / 255);
	classesSectionColor[3] = LC_OGL_FROM_SCALAR(LcTScalar(int(color.rgba.a)) / 255);

	// Only	Add	the	color components, leaving the alpha	channel	of the
	// source un-affected
	sourceColor[0] = sourceColor[0]	+ classesSectionColor[0];
	sourceColor[1] = sourceColor[1]	+ classesSectionColor[1];
	sourceColor[2] = sourceColor[2]	+ classesSectionColor[2];
	sourceColor[3] = sourceColor[3];
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::combineMaterialEmissiveColors (LcOglTScalar* sourceColor)
{
	LcOglTScalar classesSectionColor[4]	= {	0.0, 0.0, 0.0, 1.0 } ;
	LcTColor color = LcTColor::BLACK;

	// Get the material	emissive color associatd with this paint widget
	if (m_space->getPaintWidget())
		color =	m_space->getPaintWidget()->getMaterialEmissiveColor();

	// Expand given	color into array format
	classesSectionColor[0] = LC_OGL_FROM_SCALAR(LcTScalar(int(color.rgba.r)) / 255);
	classesSectionColor[1] = LC_OGL_FROM_SCALAR(LcTScalar(int(color.rgba.g)) / 255);
	classesSectionColor[2] = LC_OGL_FROM_SCALAR(LcTScalar(int(color.rgba.b)) / 255);
	classesSectionColor[3] = LC_OGL_FROM_SCALAR(LcTScalar(int(color.rgba.a)) / 255);

	// Only	Add	the	color components, leaving the alpha	channel	of the
	// source un-affected
	sourceColor[0] = sourceColor[0]	+ classesSectionColor[0];
	sourceColor[1] = sourceColor[1]	+ classesSectionColor[1];
	sourceColor[2] = sourceColor[2]	+ classesSectionColor[2];
	sourceColor[3] = sourceColor[3];
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::combineMaterialSpecularShininess (LcOglTScalar*	sourceShininess)
{
	LcOglTScalar classesSectionShininess = 0.0;

	// Get the material	specular shininess associatd with paint	widget
	if (m_space->getPaintWidget())
		classesSectionShininess	= m_space->getPaintWidget()->getMaterialSpecularShininess();

	// Add the shininess contributions from	both sides
	*sourceShininess = *sourceShininess	+ classesSectionShininess;
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::setDepthMask(GLboolean flag)
{
	if (m_curDepthMask != flag)
	{
		glDepthMask(flag);
		m_curDepthMask = flag;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::setCullFace(GLboolean flag)
{
	if (m_curCullFace != flag)
	{
		if (GL_FALSE ==	flag)
			glDisable(GL_CULL_FACE);
		else
			glEnable(GL_CULL_FACE);

		m_curCullFace =	flag;
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcOglCContext::setBlend(GLboolean flag)
{
	if (m_curBlend != flag)
	{
		if (GL_FALSE ==	flag)
			glDisable(GL_BLEND);
		else
			glEnable(GL_BLEND);

		m_curBlend =	flag;
	}
}


/*-------------------------------------------------------------------------*//**
	Helper - update	the	primary	light.
*/
LC_EXPORT void LcOglCContext::updatePrimaryLight(	const LcTPlacement&	placement,
													int	mask)
{
	if (mask & LcTPlacement::EOrientation)
	{
		if (!m_primaryLightDirection.equals(placement.orientation))
		{
			m_primaryLightDirection	= placement.orientation;
			drawPrimaryLightDirection();
		}
	}

	if (mask & LcTPlacement::EColor)
	{
		if (m_primaryLightColor	!= placement.color)
		{
			m_primaryLightColor	= placement.color;
			drawPrimaryLightColor(m_primaryLightColor);
		}
	}

	if (mask & LcTPlacement::EColor2)
	{
		if (m_ambientLightColor	!= placement.color2)
		{
			m_ambientLightColor	= placement.color2;
			drawAmbientLightColor(m_ambientLightColor);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCContext::drawAmbientLightColor(LcTColor col)
{
	if (col	!= m_currentlySetAmbientLightColor)
	{
		m_currentlySetAmbientLightColor	= col;

		// Convert the color to	OGL	values.
		LcOglTScalar globalAmbientLight[]= {
				LC_OGL_FROM_SCALAR(LcTScalar(int(col.rgba.r)) /	255.0f),
				LC_OGL_FROM_SCALAR(LcTScalar(int(col.rgba.g)) /	255.0f),
				LC_OGL_FROM_SCALAR(LcTScalar(int(col.rgba.b)) /	255.0f),
				1.0f };

#if	defined	(LC_PLAT_OGL_20)
		LcOglCEffect *effect = getCurrentEffect();

		if(effect)
		{
			effect->setGlobalAmbientLight(globalAmbientLight);
		}
#else
		LC_OGL_FXV(glLightModel)(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);
#endif
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCContext::drawPrimaryLightColor(LcTColor col)
{
	if (col	!= m_currentlySetPrimaryLightColor)
	{
		m_currentlySetPrimaryLightColor	= col;

		// Convert the color to	OGL	values.
		LcOglTScalar primaryLightColour[]= {
				LC_OGL_FROM_SCALAR(LcTScalar(int(col.rgba.r)) /	255.0f),
				LC_OGL_FROM_SCALAR(LcTScalar(int(col.rgba.g)) /	255.0f),
				LC_OGL_FROM_SCALAR(LcTScalar(int(col.rgba.b)) /	255.0f),
				1.0f };

#if	defined	(LC_PLAT_OGL_20)
		LcOglCEffect *effect = getCurrentEffect();

		if(effect)
		{
			effect->setDiffuseLight(0, primaryLightColour);
		}
#else
		LC_OGL_FXV(glLight)(GL_LIGHT0, GL_DIFFUSE, primaryLightColour);
#endif
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCContext::drawPrimaryLightDirection()
{
	// Set the default position	(straight down the z axis.
	LcOglTScalar newPosition[] = { 0.0f, 0.0f, 1.0f, 0.0f };

	// Create a	rotation matrix.
	LcTTransform orientation = LcTTransform::rotate(m_primaryLightDirection);
	orientation.translatePrefix(0.0f, 0.0f,	1.0f);

	// Rotate the modelview.
	LcOglTScalar ogd[16];
	orientation.getData(ogd);

#if	defined	(LC_PLAT_OGL_20)
	LcOglCEffect *effect = getCurrentEffect();
	LcOglTScalar invt[16];

	orientation.invert().transpose().getData(invt);

	if(effect)
	{
		effect->setModelViewMatrix(ogd);
		effect->setNormalMatrix(invt);

		// Transform light position w.r.t current model-view matrix
		LcTTransform tfm = LcTTransform::identity().setFromRowMajorMatrixArray ((LcTScalar *)ogd).transpose();

		LcTVector directionalPosition = tfm.transform (LcTVector (0.0f, 0.0f, 1.0f));

		newPosition [0] = directionalPosition.x;
		newPosition [1] = directionalPosition.y;
		newPosition [2] = directionalPosition.z;
		newPosition [3] = 0.0f;

		effect->setPositionLight(0, newPosition);
	}
#else
	// Change modelview	matrix
	glMatrixMode(GL_MODELVIEW);
	LC_OGL_FX(glLoadMatrix)(ogd);

	LC_OGL_FXV(glLight)(GL_LIGHT0, GL_POSITION,	newPosition);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT void LcOglCContext::scissor(GLint	x, GLint y,	GLsizei	width, GLsizei height)
{
	glScissor(x	+ m_xOffset, y + m_yOffset,	width, height);
}

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT void LcOglCContext::clearCache()
{
	// Material	Ambient	- default is (0.2, 0.2,	0.2, 1.0)
	LcOglTScalar materialAmbient[]=	{ 0.2f,	0.2f, 0.2f,	1.0f };
	memcpy(m_curMatAmb,	materialAmbient, sizeof(LcOglTScalar) *	4);

	// Material	Diffuse	- default is (0.8, 0.8,	0.8, 1.0)
	LcOglTScalar materialDiffuse[]=	{ 0.8f,	0.8f, 0.8f,	1.0f };
	memcpy(m_curMatDif,	materialDiffuse, sizeof(LcOglTScalar) *	4);

	// Material	Specular - default is BLACK
	LcOglTScalar materialSpecular[]= { 0.0f, 0.0f, 0.0f, 1.0f };
	memcpy(m_curMatSpc,	materialSpecular, sizeof(LcOglTScalar) * 4);

	// Material	Emissive - default is BLACK
	LcOglTScalar materialEmissive[]= { 0.0f, 0.0f, 0.0f, 1.0f };
	memcpy(m_curMatEmm,	materialEmissive, sizeof(LcOglTScalar) * 4);

	// Material	Shininess -	default	is ZERO
	LcOglTScalar materialShininess = 0.0f;
	m_curMatShi	= materialShininess;

	m_curDepthMask = GL_FALSE;
	m_curCullFace =	GL_FALSE;
	m_curBlend =	GL_FALSE;

	m_ambientLightColor	= LcTColor::BLACK;
	m_primaryLightColor	= LcTColor::WHITE;
	m_primaryLightDirection	= LcTQuaternion();
	m_primaryLightEnabled =	true;
	m_currentlySetAmbientLightColor	= LcTColor::BLACK;
	m_currentlySetPrimaryLightColor	= LcTColor::WHITE;

#if	!defined (LC_PLAT_OGL_20)
	// Now explicitly set the values to	bring the OGL 1.1 system at
	// a consistent	initial	state. OpenGL ES 2.0 takes care	of this
	// situation during	initialization of default effects

	// Primary light ambient level - default is	BLACK
	LcOglTScalar globalAmbientLight[]= { 0.0f, 0.0f, 0.0f, 1.0f	};
	LC_OGL_FXV(glLightModel)(GL_LIGHT_MODEL_AMBIENT, globalAmbientLight);

	// Primary light diffuse component - default is	WHITE
	LcOglTScalar primaryLightColour[]= { 1.0f, 1.0f, 1.0f, 1.0f	};
	LC_OGL_FXV(glLight)(GL_LIGHT0, GL_DIFFUSE, primaryLightColour);

	// Set primary specular	component to black - however default is	WHITE!
	LcOglTScalar specularColor[]= {0.0f, 0.0f, 0.0f, 1.0f };
	LC_OGL_FXV(glLight)(GL_LIGHT0, GL_SPECULAR,	specularColor);

	// Set material	Ambient
	LC_OGL_FXV(glMaterial)(GL_FRONT_AND_BACK, GL_AMBIENT, materialAmbient);

	// Set material	dffuse
	LC_OGL_FXV(glMaterial)(GL_FRONT_AND_BACK, GL_DIFFUSE, materialDiffuse);

	// Set material	specular
	LC_OGL_FXV(glMaterial)(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);

	// Set material	emissive
	LC_OGL_FXV(glMaterial)(GL_FRONT_AND_BACK, GL_EMISSION, materialEmissive);

	// Set material	shininess
	LC_OGL_FX(glMaterial)(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
#else
	m_cubemapMap.clear();
#endif
}

#if	defined(LC_PLAT_OGL_20)

/*-------------------------------------------------------------------------*//**
	Default	Implementation of getBitmap	for	cube map.
*/
LC_EXPORT GLuint LcOglCContext::getCubeBitmaps(const LcTmString& file1,
											const LcTmString& file2,
											const LcTmString& file3,
											const LcTmString& file4,
											const LcTmString& file5,
											const LcTmString& file6)
{
	// Use first file to determine the extension for files associated with this	cube map
	LcTaString ext = file1.getWord(-1, '.');

	// NDI Cube	Bitmaps
	if(ext.compareNoCase("ndi")	== 0)
	{
		return loadCubeBitmaps(file1,
							   file2,
							   file3,
							   file4,
							   file5,
							   file6);
	}
	else //	Custom non-NDI Cube	Bitmaps
	{
		return loadCustomCubeBitmaps(file1,
									 file2,
									 file3,
									 file4,
									 file5,
									 file6);
	}
}

/*-------------------------------------------------------------------------*//**
	Implementation of loadCubeBitmaps
*/
LC_IMPORT GLuint LcOglCContext::loadCubeBitmaps(const LcTmString& file1,
											 const LcTmString& file2,
											 const LcTmString& file3,
											 const LcTmString& file4,
											 const LcTmString& file5,
											 const LcTmString& file6)
{
	GLenum	unit	= GL_TEXTURE0;
	GLenum	eglFormat =	GL_RGBA;
	bool	mipmap	= false;
	bool	cubeStatus = false;

	LcTPixelDim	size(0,	0);
	LcTaOwner<LcCNdiBitmapFile>	bmf1, bmf2,	bmf3, bmf4,	bmf5, bmf6;

	// Combine all 6 face file names to serve as key for the cache map
	LcTaString fileNames = file1.toLower() +
						   file2.toLower() +
						   file3.toLower() +
						   file4.toLower() +
						   file5.toLower() +
						   file6.toLower();

	// First see if Cube map is in the cache
	TmMCubemapMap::iterator it = m_cubemapMap.find(fileNames);
	if (it != m_cubemapMap.end())
	{
		// Return cached texture ID
		return (*it).second;
	}

	bmf1 = LcCNdiBitmapFile::create();
	bmf2 = LcCNdiBitmapFile::create();
	bmf3 = LcCNdiBitmapFile::create();
	bmf4 = LcCNdiBitmapFile::create();
	bmf5 = LcCNdiBitmapFile::create();
	bmf6 = LcCNdiBitmapFile::create();

	if (bmf1->open(file1, LcCNdiBitmapFile::KFormatAny))
	{
		size = bmf1->getSize();
	}

	if (bmf2->open(file2, LcCNdiBitmapFile::KFormatAny))
	{
		size = bmf2->getSize();
	}

	if (bmf3->open(file3, LcCNdiBitmapFile::KFormatAny))
	{
		size = bmf3->getSize();
	}

	if (bmf4->open(file4, LcCNdiBitmapFile::KFormatAny))
	{
		size = bmf4->getSize();
	}

	if (bmf5->open(file5, LcCNdiBitmapFile::KFormatAny))
	{
		size = bmf5->getSize();
	}

	if (bmf6->open(file6, LcCNdiBitmapFile::KFormatAny))
	{
		size = bmf6->getSize();
	}

	if (size.width == 0	|| size.height == 0)
	{
		return 0;
	}

	if (LcCNdiBitmapFile::KFormatGraphicOpaque	== bmf1->getFormat())
		eglFormat =	GL_RGB;

	if (LcCNdiBitmapFile::KFormatGraphicOpaque	== bmf2->getFormat())
		eglFormat =	GL_RGB;

	if (LcCNdiBitmapFile::KFormatGraphicOpaque	== bmf3->getFormat())
		eglFormat =	GL_RGB;

	if (LcCNdiBitmapFile::KFormatGraphicOpaque	== bmf4->getFormat())
		eglFormat =	GL_RGB;

	if (LcCNdiBitmapFile::KFormatGraphicOpaque	== bmf5->getFormat())
		eglFormat =	GL_RGB;

	if (LcCNdiBitmapFile::KFormatGraphicOpaque	== bmf6->getFormat())
		eglFormat =	GL_RGB;

	CTextureInfo *textureInfo =	getTextureInfo ();

	if (textureInfo)
	{
		unit = textureInfo->getUnit();
		mipmap = textureInfo->getMipmap();
	}

	m_cubeTexture =	LcOglCCubeTexture::create(m_space);

	// If our screen depth is only 12-bit then avoid wasting memory
	#ifdef LC_OGL_ONLY_4K_COLORS
		cubeStatus = m_cubeTexture->createTexture (size, GL_UNSIGNED_SHORT_4_4_4_4,	eglFormat, unit, mipmap);
	#else
		cubeStatus = m_cubeTexture->createTexture (size, GL_UNSIGNED_BYTE, eglFormat, unit,	mipmap);
	#endif

	if (cubeStatus)
	{
		// Create the texture and copy the processed data into it.
		m_cubeTexture->updateTexture(bmf1->getData(),
									 bmf2->getData(),
									 bmf3->getData(),
									 bmf4->getData(),
									 bmf5->getData(),
									 bmf6->getData());

		// Add to cube map cache
		m_cubemapMap[fileNames] = m_cubeTexture->getTexture();

		return m_cubemapMap[fileNames];
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
	Implementation of loadCustomCubeBitmaps
*/
LC_IMPORT GLuint LcOglCContext::loadCustomCubeBitmaps(const LcTmString& file1,
												   const LcTmString& file2,
												   const LcTmString& file3,
												   const LcTmString& file4,
												   const LcTmString& file5,
												   const LcTmString& file6)
{
	GLenum	unit	= GL_TEXTURE0;
	GLenum	eglFormat =	GL_RGBA;
	GLenum	compressedFormat = 0;
	GLuint	levels = 0;
	LcCCustomBitmapFile::EFormat bmfFormat;

	bool	mipmap	= false;
	bool	cubeStatus = false;
	bool	isCompressed = false;
	bool	status = false;

	LcTPixelDim	size(0,	0);
	LcTaOwner<LcCCustomBitmapFile> bmf[6];
	LcTaString fileNameArray[6];

	// Combine all 6 face file names to serve as key for the cache map
	LcTaString fileNames = file1.toLower() +
						   file2.toLower() +
						   file3.toLower() +
						   file4.toLower() +
						   file5.toLower() +
						   file6.toLower();

	fileNameArray[0] = file1;
	fileNameArray[1] = file2;
	fileNameArray[2] = file3;
	fileNameArray[3] = file4;
	fileNameArray[4] = file5;
	fileNameArray[5] = file6;

	// First see if Cube map is in the cache
	TmMCubemapMap::iterator it = m_cubemapMap.find(fileNames);
	if (it != m_cubemapMap.end())
	{
		// Return cached texture ID
		return (*it).second;
	}

	bmf[0] = LcCCustomBitmapFile::create();
	bmf[1] = LcCCustomBitmapFile::create();
	bmf[2] = LcCCustomBitmapFile::create();
	bmf[3] = LcCCustomBitmapFile::create();
	bmf[4] = LcCCustomBitmapFile::create();
	bmf[5] = LcCCustomBitmapFile::create();

	// Now try and load each of the six faces of the cubemap.  Note that we require
	// at least one of the images to exist, and we also require that each image matches
	// the other image properties.  If an image doesn't match its predecessors, we unload
	// it and that face will not have an image.
	for (int i = 0; i < 6; ++i)
	{
		// check if face exists
		if(bmf[i] && bmf[i]->open(fileNameArray[i]))
		{
			if(status)
			{
				if(bmf[i]->getSize().width != size.width
					|| bmf[i]->getSize().height != size.height
					|| bmfFormat !=  bmf[i]->getFormat()
					|| levels != bmf[i]->getLevelCount()
					|| compressedFormat != bmf[i]->getOGLCompressionFormat())
				{
					// This image doesn't match the others, so we shall not use it
					bmf[i]->close();
					bmf[i].destroy();
				}
			}
			else
			{
				// We have an image!
				size = bmf[i]->getSize();
				bmfFormat = bmf[i]->getFormat();
				levels = bmf[i]->getLevelCount();
				compressedFormat = bmf[i]->getOGLCompressionFormat();
				status = true;
			}
		}
	}

	if(status)
	{
		// Determine the type of the loaded	image to find out the format.

		// Decide texture format taking	first face (X +) as	reference
		switch (bmfFormat)
		{
			case LcCCustomBitmapFile::KFormatGraphicTranslucent:
			{
				eglFormat =	GL_RGBA;
				isCompressed = false;
			}
			break;

			case LcCCustomBitmapFile::KFormatGraphicOpaque:
			{
				eglFormat =	GL_RGB;
				isCompressed = false;
			}
			break;

			case LcCCustomBitmapFile::KFormatCompressedOpenGL:
			{
				eglFormat =	compressedFormat;
				isCompressed = true;
			}
			break;

			default:
			break;
		}

		CTextureInfo *textureInfo =	getTextureInfo ();

		if (textureInfo)
		{
			unit = textureInfo->getUnit();
			mipmap = textureInfo->getMipmap();
		}

		m_cubeTexture =	LcOglCCubeTexture::create(m_space);

		if (m_cubeTexture)
		{
			// If our screen depth is only 12-bit then avoid wasting memory
	#ifdef LC_OGL_ONLY_4K_COLORS
			cubeStatus = m_cubeTexture->createTexture (size, GL_UNSIGNED_SHORT_4_4_4_4,	eglFormat, unit, mipmap);
	#else
			if (!isCompressed)
				cubeStatus = m_cubeTexture->createTexture (size, GL_UNSIGNED_BYTE, eglFormat, unit,	mipmap);
			else
				cubeStatus = m_cubeTexture->createCompressedTexture	(size, eglFormat, levels, unit);
	#endif
		}

		// Texture created,	now	update its data
		if (cubeStatus)
		{
			// X Faces

			// X +
			if (bmf[0])
			{
				readCubeTextureFaceData	(bmf[0].ptr(),
									 m_cubeTexture.ptr(),
									 size,
									 GL_TEXTURE_CUBE_MAP_POSITIVE_X,
									 eglFormat);
			}

			// X -
			if (bmf[1])
			{
				readCubeTextureFaceData	(bmf[1].ptr(),
									 m_cubeTexture.ptr(),
									 size,
									 GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
									 eglFormat);
			}

			// Y Faces

			// Y +
			if (bmf[2])
			{
				readCubeTextureFaceData	(bmf[2].ptr(),
									 m_cubeTexture.ptr(),
									 size,
									 GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
									 eglFormat);
			}

			// Y -
			if (bmf[3])
			{
				readCubeTextureFaceData	(bmf[3].ptr(),
									 m_cubeTexture.ptr(),
									 size,
									 GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
									 eglFormat);
			}

			// Z Faces

			// Z +
			if (bmf[4])
			{
				readCubeTextureFaceData	(bmf[4].ptr(),
									 m_cubeTexture.ptr(),
									 size,
									 GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
									 eglFormat);
			}

			// Z -
			if (bmf[5])
			{
				readCubeTextureFaceData	(bmf[5].ptr(),
									 m_cubeTexture.ptr(),
									 size,
									 GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
									 eglFormat);
			}

			// Add to cube map cache
			m_cubemapMap[fileNames] = m_cubeTexture->getTexture();
		}
	}

	// Close all open files
	for (int i = 0; i < 6; ++i)
	{
		if (bmf[i])
			bmf[i]->close();
	}

	// Return cube map texture ID as returned by OpenGL
	if (cubeStatus)
		return m_cubeTexture->getTexture();
	else
		return (0);
}

LC_IMPORT void LcOglCContext::readCubeTextureFaceData (LcCCustomBitmapFile*	bmf,
													   LcOglCCubeTexture* cubeTexture,
													   LcTPixelDim size,
													   GLenum target,
													   GLenum format)
{
	LcTByte	*image_data	= NULL;
	IFX_UINT32 dataSize	= 0;

	if (!bmf)
		return;

	// Create the texture and copy the processed data into it.
	if(format == GL_RGB	|| format == GL_RGBA)
	{
		if(bmf->readData((void**)&image_data, 0, &dataSize,	false) && (dataSize	!= 0))
		{
			// Send	uncompressed data to OpenGL	pipeline for this cube face
			cubeTexture->updateTextureFace(target, image_data);

			// Free	resources
			bmf->releaseData(image_data);
		}
	}
	else //	compressed texture
	{
		int	width =	size.width;
		int	height = size.height;

		// Iterate between first and last level	(limits	inclusive)
		for	(unsigned int i	= bmf->getLevelFirst();	i <= bmf->getLevelLast(); i++)
		{
			if(bmf->readData((void**)&image_data, i, &dataSize,	false) && (dataSize	!= 0))
			{
				// Send	compressed data	to OpenGL pipeline
				cubeTexture->updateCompressedTexture(target,	// target cube face
													 i,			// level
													 format,	// compression format
													 width,
													 height,
													 dataSize,
													 image_data);
				// Free	resources
				bmf->releaseData(image_data);

				// Setup for next mip-map level
				image_data = NULL;
				dataSize = 0;

				width  = max (width	>> 1, 1);
				height = max (height >>	1, 1);
			}
		}
	}
}

LC_IMPORT void LcOglCContext::setTextureInfo (CTextureInfo*	textureInfo)
{
	m_textureInfo =	textureInfo;
}

LC_IMPORT	CTextureInfo*	LcOglCContext::getTextureInfo ()
{
	return m_textureInfo;
}

/* --------------------- Load Custom Effects ----------------------------------------------	*/
LC_IMPORT bool	LcOglCContext::loadCustomEffects(void)
{
	bool status = true;
	LcTaString		effectName;
	LcOglCEffect*	effect;

	LcTmOwnerMap<LcTmString, LcOglCEffect>::iterator it;

	for(it=m_customEffectMap.begin(); it !=	m_customEffectMap.end(); it++)
	{
		effectName = (*it).first;
		effect = (*it).second;

		if (effect == NULL)
			continue;

		setCurrentEffect (effect);

		// First check whether the effect has already been loaded
		if (effect->isLoaded() == false)
		{
			status = effect->loadEffect	();

			// Custom effect loaded	successfully, now set it up	only if	it created first time ...
			if (status)
			{
				float Identity[16] = {	1.0, 0.0, 0.0, 0.0,
										0.0, 1.0, 0.0, 0.0,
										0.0, 0.0, 1.0, 0.0,
										0.0, 0.0, 0.0, 1.0	};

				// Setup transformation	matrices - with	Identity as	default
				m_currentEffect->setModelViewMatrix(Identity);
				m_currentEffect->setProjectionMatrix(Identity);
				m_currentEffect->setNormalMatrix(Identity);

				float material_ambient_color  [4] =	{ 0.0f,	0.0f, 0.0f,	1.0f } ;
				float material_diffuse_color  [4] =	{ 0.0f,	0.0f, 0.0f,	1.0f } ;
				float material_specular_color [4] =	{ 0.0f,	0.0f, 0.0f,	1.0f } ;
				float material_emissive_color [4] =	{ 0.0f,	0.0f, 0.0f,	1.0f } ;
				float material_scene_color	  [4] =	{ 0.2f,	0.2f, 0.2f,	1.0f } ;

				m_currentEffect->setAmbientMaterial(material_ambient_color);
				m_currentEffect->setDiffuseMaterial(material_diffuse_color);
				m_currentEffect->setSpecularMaterial(material_specular_color);
				m_currentEffect->setEmissionMaterial(material_emissive_color);
				m_currentEffect->setShininessMaterial((GLfloat)	0.0);

				m_currentEffect->setGlobalAmbientLight((GLfloat*) material_scene_color);

				float light_attenuation_factors	[3]	= {	1.0, 0.0, 0.0 };

				// Defaults	for	LIGHT0 (primary	light)
				float light_position_0		 [4] = { 0.0, 0.0, 1.0,	0.0	} ;
				float light_ambient_color_0	 [4] = { 0.0, 0.0, 0.0,	1.0	} ;
				float light_diffuse_color_0	 [4] = { 1.0, 1.0, 1.0,	1.0	} ;
				float light_specular_color_0 [4] = { 1.0, 1.0, 1.0,	1.0	} ;

				// Defaults	for	all	lights other than LIGHT0
				float light_position_n		 [4] = { 0.0, 0.0, 0.0,	1.0	} ;
				float light_ambient_color_n	 [4] = { 0.0, 0.0, 0.0,	1.0	} ;
				float light_diffuse_color_n	 [4] = { 0.0, 0.0, 0.0,	1.0	} ;
				float light_specular_color_n [4] = { 0.0, 0.0, 0.0,	1.0	} ;

				m_currentEffect->setPositionLight (0, (GLfloat*) light_position_0);
				m_currentEffect->setAmbientLight  (0, (GLfloat*) light_ambient_color_0);
				m_currentEffect->setDiffuseLight  (0, (GLfloat*) light_diffuse_color_0);
				m_currentEffect->setSpecularLight (0, (GLfloat*) light_specular_color_0);

				m_currentEffect->setLightAttenuationFactors	(0,
													light_attenuation_factors[0],
													light_attenuation_factors[1],
													light_attenuation_factors[2]);

				// Initialize Light	properties for all lights other	than LIGHT0
				for	(int i = 1;	i <	IFX_OGL_NUM_LIGHTS;	i++)
				{
					m_currentEffect->setPositionLight (i, (GLfloat*) light_position_n);
					m_currentEffect->setAmbientLight  (i, (GLfloat*) light_ambient_color_n);
					m_currentEffect->setDiffuseLight  (i, (GLfloat*) light_diffuse_color_n);
					m_currentEffect->setSpecularLight (i, (GLfloat*) light_specular_color_n);

					m_currentEffect->setLightAttenuationFactors	(i,
													  light_attenuation_factors[0],
													  light_attenuation_factors[1],
													  light_attenuation_factors[2]);
				}

				// Set Light enable	states - disable all as	default
				for	(int i = 0;	i <	IFX_OGL_NUM_LIGHTS;	i++)
					m_currentEffect->disableLight (i);

				// Enable Light	0
				m_currentEffect->enableLight (0);

				// Set the position	for	first secondary	light
				float light_position_1 [4] = { 0, 0, 0,	1.0	} ;
				m_currentEffect->setPositionLight (1, (GLfloat*) light_position_1);

				// Enable attributes for non-fixed attributes
				m_currentEffect->configureVertexAttribArray();
			}
		}
	}

	return (status);
}

/* --------------------- Un-Load Custom	Effects	---------------------------------------------- */
LC_IMPORT bool LcOglCContext::unloadCustomEffects(void)
{
	bool status = true;
	LcOglCEffect*	effect = NULL;
	bool clearMaps = false;

	LcTmOwnerMap<LcTmString, LcOglCEffect>::iterator it;

	for(it=m_customEffectMap.begin(); it !=	m_customEffectMap.end(); it++)
	{
		effect = (*it).second;

		if (effect == NULL)
			continue;

		// Un-load the effect, but retain the map entries
		status = effect->unloadEffect (clearMaps);
	}

	return status;
}

/*--------------------------------------------------------------------------
 * Function	Name: loadCustomEffect
 * Input		: effectName
 *--------------------------------------------------------------------------*/
LC_IMPORT bool LcOglCContext::loadCustomEffect(LcTmString effectName)
{
	bool status = true;
	LcOglCEffect*	effect;
	LcOglCEffect*	existingEffect = NULL;

	if (m_openGLInitialized	== false)
	{
		return false;
	}

	TmEffectMap::iterator it = m_customEffectMap.find(effectName);

	// First see if	the	Effect is already in the cache
	if (it != m_customEffectMap.end())
	{
		 effect	= (*it).second;
	}
	else
	{
		return false;
	}

	if (effect == NULL)
	{
		return false;
	}

	// Store the existing effect so	that we	may	switch to this as soon as this custom effect
	// has been	created	and	properly initialized
	existingEffect = getCurrentEffect();

	setCurrentEffect (effect);
	status = effect->loadEffect	();

	// Custom effect loaded	successfully, now set it up	provided
	// it is not already loaded	...
	if ( status &&	(effect->isLoaded()	== false) )
	{
		LcOglCGlobalState *state = getGlobalState();
		float mv[16], proj[16],	normal[16];
		float ambient[4], diffuse[4], specular[4], emissive[4];
		float ambientLight[4], diffuseLight[4],	specularLight[4], positionLight[4];
		float shininess	= 0;
		float global_ambient[4];
		float constant,	linear,	quadratic;

		effect = getCurrentEffect();
		effect->makeCurrent();

		state->getMVMatrixState(mv);
		state->getProjMatrixState(proj);
		state->getNormalMatrixState(normal);

		effect->setModelViewMatrix(mv);
		effect->setProjectionMatrix(proj);
		effect->setNormalMatrix(normal);

		state->getMaterialAmbientState(ambient);
		state->getMaterialDiffuseState(diffuse);
		state->getMaterialSpecularState(specular);
		state->getMaterialEmissionState(emissive);
		state->getMaterialShininessState(&shininess);

		effect->setAmbientMaterial(ambient);
		effect->setDiffuseMaterial(diffuse);
		effect->setSpecularMaterial(specular);
		effect->setEmissionMaterial(emissive);
		effect->setShininessMaterial(shininess);

		for(int	lightIndex=0; lightIndex<IFX_OGL_NUM_LIGHTS; lightIndex++)
		{
			state->getLightAmbientState(lightIndex,	ambientLight);
			state->getLightDiffuseState(lightIndex,	diffuseLight);
			state->getLightSpecularState(lightIndex, specularLight);
			state->getLightPositionState(lightIndex, positionLight);
			state->getLightAttenuationFactorsState(lightIndex, &constant, &linear, &quadratic);

			effect->setAmbientLight(lightIndex,	ambientLight);
			effect->setDiffuseLight(lightIndex,	diffuseLight);
			effect->setSpecularLight(lightIndex, specularLight);
			effect->setPositionLight(lightIndex, positionLight);
			effect->setLightAttenuationFactors(lightIndex, constant, linear, quadratic);

			if(state->getLightStatus(lightIndex))
				effect->enableLight(lightIndex);
			else
				effect->disableLight(lightIndex);
		}

		state->getGlobalAmbientLightState(global_ambient);
		effect->setGlobalAmbientLight(global_ambient);

		// Enable attributes for non-fixed attributes
		effect->configureVertexAttribArray();
	}

	// We are done with	loading	this custom, now switch	to effect that was in action
	// just	before this	custom effect was loaded
	if (existingEffect)
	{
		setCurrentEffect (existingEffect);

		existingEffect->makeCurrent();

		// Enable attributes for non-fixed attributes
		existingEffect->configureVertexAttribArray();
	}

	return (status);
}

/* --------------------- Load Default Effects ---------------------------------------------- */
LC_IMPORT bool LcOglCContext::loadDefaultEffects()
{
	bool status = true;
	LcTaString		effectName;
	LcOglCEffect*	effect;

	LcTmOwnerMap<LcTmString, LcOglCEffect>::iterator it;

	for(it=m_defaultEffectMap.begin(); it != m_defaultEffectMap.end(); it++)
	{
		effectName = (*it).first;
		effect = (*it).second;

		if (effect == NULL)
			continue;

		setCurrentEffect (effect);

		// First check whether the effect has already been loaded
		if (effect->isLoaded() == false)
		{
			status = effect->loadEffect	(true);

			// Custom effect loaded	successfully, now set it up	only if	it created first time ...
			if (status)
			{
				float Identity[16] = {	1.0, 0.0, 0.0, 0.0,
										0.0, 1.0, 0.0, 0.0,
										0.0, 0.0, 1.0, 0.0,
										0.0, 0.0, 0.0, 1.0	};

				// Setup transformation	matrices - with	Identity as	default
				m_currentEffect->setModelViewMatrix(Identity);
				m_currentEffect->setProjectionMatrix(Identity);
				m_currentEffect->setNormalMatrix(Identity);

				float material_ambient_color  [4] =	{ 0.0f,	0.0f, 0.0f,	1.0f } ;
				float material_diffuse_color  [4] =	{ 0.0f,	0.0f, 0.0f,	1.0f } ;
				float material_specular_color [4] =	{ 0.0f,	0.0f, 0.0f,	1.0f } ;
				float material_emissive_color [4] =	{ 0.0f,	0.0f, 0.0f,	1.0f } ;
				float material_scene_color	  [4] =	{ 0.2f,	0.2f, 0.2f,	1.0f } ;

				m_currentEffect->setAmbientMaterial(material_ambient_color);
				m_currentEffect->setDiffuseMaterial(material_diffuse_color);
				m_currentEffect->setSpecularMaterial(material_specular_color);
				m_currentEffect->setEmissionMaterial(material_emissive_color);
				m_currentEffect->setShininessMaterial((GLfloat)	0.0);

				m_currentEffect->setGlobalAmbientLight((GLfloat*) material_scene_color);

				float light_attenuation_factors	[3]	= {	1.0, 0.0, 0.0 };

				// Defaults	for	LIGHT0 (primary	light)
				float light_position_0		 [4] = { 0.0, 0.0, 1.0,	0.0	} ;
				float light_ambient_color_0	 [4] = { 0.0, 0.0, 0.0,	1.0	} ;
				float light_diffuse_color_0	 [4] = { 1.0, 1.0, 1.0,	1.0	} ;
				float light_specular_color_0 [4] = { 1.0, 1.0, 1.0,	1.0	} ;

				// Defaults	for	all	lights other than LIGHT0
				float light_position_n		 [4] = { 0.0, 0.0, 0.0,	1.0	} ;
				float light_ambient_color_n	 [4] = { 0.0, 0.0, 0.0,	1.0	} ;
				float light_diffuse_color_n	 [4] = { 0.0, 0.0, 0.0,	1.0	} ;
				float light_specular_color_n [4] = { 0.0, 0.0, 0.0,	1.0	} ;

				m_currentEffect->setPositionLight (0, (GLfloat*) light_position_0);
				m_currentEffect->setAmbientLight  (0, (GLfloat*) light_ambient_color_0);
				m_currentEffect->setDiffuseLight  (0, (GLfloat*) light_diffuse_color_0);
				m_currentEffect->setSpecularLight (0, (GLfloat*) light_specular_color_0);

				m_currentEffect->setLightAttenuationFactors	(0,
													light_attenuation_factors[0],
													light_attenuation_factors[1],
													light_attenuation_factors[2]);

				// Initialize Light	properties for all lights other	than LIGHT0
				for	(int i = 1;	i <	IFX_OGL_NUM_LIGHTS;	i++)
				{
					m_currentEffect->setPositionLight (i, (GLfloat*) light_position_n);
					m_currentEffect->setAmbientLight  (i, (GLfloat*) light_ambient_color_n);
					m_currentEffect->setDiffuseLight  (i, (GLfloat*) light_diffuse_color_n);
					m_currentEffect->setSpecularLight (i, (GLfloat*) light_specular_color_n);

					m_currentEffect->setLightAttenuationFactors	(i,
													  light_attenuation_factors[0],
													  light_attenuation_factors[1],
													  light_attenuation_factors[2]);
				}

				// Set Light enable	states - disable all as	default
				for	(int i = 0;	i <	IFX_OGL_NUM_LIGHTS;	i++)
					m_currentEffect->disableLight (i);

				// Enable Light	0
				m_currentEffect->enableLight (0);

				// Set the position	for	first secondary	light
				float light_position_1 [4] = { 0, 0, 0,	1.0	} ;
				m_currentEffect->setPositionLight (1, (GLfloat*) light_position_1);

				// Enable attributes for non-fixed attributes
				m_currentEffect->configureVertexAttribArray();
			}
		}
	}
	return status;
}

/* --------------------- Un-Load Default Effects ----------------------------------------------	*/
LC_IMPORT bool LcOglCContext::unloadDefaultEffects(void)
{
	bool status = true;
	LcOglCEffect*	effect = NULL;
	bool clearMaps = false;

	LcTmOwnerMap<LcTmString, LcOglCEffect>::iterator it;

	for(it=m_defaultEffectMap.begin(); it != m_defaultEffectMap.end(); it++)
	{
		effect = (*it).second;

		if (effect == NULL)
			continue;

		// Un-load the effect, but retain map entries
		status = effect->unloadEffect (clearMaps);
	}

	return status;
}

/* --------------------- Effect Translucency ---------------------------------------------- */
LC_IMPORT bool LcOglCContext::getCustomEffectTranslucency(int effectIndex, LcCWidget *widget)
{
	IFX_RETURN_STATUS status = IFX_SUCCESS;
	LcOglCEffect* eff =	NULL;
	bool bUseCustomEffect =	false;
	unsigned int initSize =	m_customEffectMap.size();

	bool output = false;

	LcTaString effectName =	"";

	if (widget)
	{
		// Get effect name from	current	paint widget (if widget	exists)
		effectName = widget->getVisualEffectName();

		// Determine the value for this	expression-based flag to know whether custom effect	has	to
		// be applied
		bUseCustomEffect = widget->getUseCustomEffect();
	}

	if(!effectName.isEmpty() &&	bUseCustomEffect)
	{
		eff	= m_customEffectMap[effectName];

		// First of	all	make sure that the effect is valid by inspecting its "loaded" status
		if ( (eff == NULL) || (eff->isLoaded() == false) )
		{
			status = IFX_ERROR;
		}
		else
		{
			// check the usage as specified	in the effect file,	and	identify conflict;
			// revert to default behavior is usage has conflict
			if (eff->getUsageIndex() !=	effectIndex)
			{
				status = IFX_ERROR;
				eff	= NULL;
			}
		}
	}
	else
	{
		status = IFX_ERROR;
	}

	if(m_customEffectMap.size()	> initSize)
	{
		m_customEffectMap[effectName] =	NULL;
		eff	= NULL;

		status = IFX_ERROR;
	}

	if(status == IFX_SUCCESS)
	{
		output = eff->getMakesTranslucent();
	}

	return output;
}

/* --------------------- Switch	Effect ---------------------------------------------- */
LC_IMPORT void LcOglCContext::switchEffect(int effectIndex,	LcCWidget *widget)
{
	bool status = true;
	LcOglCEffect* eff =	NULL;
	bool bHighQuality =	false;
	bool bUseCustomEffect =	false;
	unsigned int initSize =	m_customEffectMap.size();

	LcTaString effectName =	"";

	if (widget)
	{
		// Get effect name from	current	paint widget (if widget	exists)
		effectName = widget->getVisualEffectName();

		// Determine quality for default behavior of this widget
		bHighQuality = widget->getOpenGLRenderQualitySetting().compareNoCase ("high") == 0;

		// Determine the value for this	expression-based flag to know whether custom effect	has	to
		// be applied
		bUseCustomEffect = widget->getUseCustomEffect();
	}

	// CAUTION:	Custom effect will be applied only if "useCustomEffect"	expression based attribute
	// evalutes	to "true". If unspecified, it evaluates	to false.
	if(!effectName.isEmpty() &&	bUseCustomEffect)
	{
		eff	= m_customEffectMap[effectName];

		// First of	all	make sure that the effect is valid by inspecting its "loaded" status
		if ( (eff == NULL) || (eff->isLoaded() == false) )
		{
			status = false;
		}
		else
		{
			// check the usage as specified	in the effect file,	and	identify conflict;
			// revert to default behavior is usage has conflict
			if (eff->getUsageIndex() !=	effectIndex)
			{
				status = false;
				eff	= NULL;
			}
		}
	}
	else
	{
		status = false;
	}

	if(m_customEffectMap.size()	> initSize)
	{
		m_customEffectMap[effectName] =	NULL;
		eff	= NULL;

		status = false;
	}

	if (status == false)
	{
		switchEffect (effectIndex, bHighQuality);

		return;
	}

	setEffectUsageIndex(-1);
	setCurrentEffect(eff);

	LcOglCGlobalState *state = getGlobalState();
	float mv[16], proj[16],	normal[16];
	float ambient[4], diffuse[4], specular[4], emissive[4];
	float ambientLight[4], diffuseLight[4],	specularLight[4], positionLight[4];
	float shininess	= 0;
	float global_ambient[4];
	float constant,	linear,	quadratic;

	LcOglCEffect* effect = getCurrentEffect();
	if(effect)
		effect->makeCurrent();
	else
		return;

	// Restore the effect to defaults (only	configurable uniforms) as rest will	be handled
	// by engine via semantic mapping mechanism
	effect->restoreConfigurableUniformsToDefaults();

	widget->passWidgetInfoToOGL();

	state->getMVMatrixState(mv);
	state->getProjMatrixState(proj);
	state->getNormalMatrixState(normal);

	effect->setModelViewMatrix(mv);
	effect->setProjectionMatrix(proj);
	effect->setNormalMatrix(normal);

	state->getMaterialAmbientState(ambient);
	state->getMaterialDiffuseState(diffuse);
	state->getMaterialSpecularState(specular);
	state->getMaterialEmissionState(emissive);
	state->getMaterialShininessState(&shininess);

	effect->setAmbientMaterial(ambient);
	effect->setDiffuseMaterial(diffuse);
	effect->setSpecularMaterial(specular);
	effect->setEmissionMaterial(emissive);
	effect->setShininessMaterial(shininess);

	for(int	lightIndex=0; lightIndex<IFX_OGL_NUM_LIGHTS; lightIndex++)
	{
		state->getLightAmbientState(lightIndex,	ambientLight);
		state->getLightDiffuseState(lightIndex,	diffuseLight);
		state->getLightSpecularState(lightIndex, specularLight);
		state->getLightPositionState(lightIndex, positionLight);
		state->getLightAttenuationFactorsState(lightIndex, &constant, &linear, &quadratic);

		effect->setAmbientLight(lightIndex,	ambientLight);
		effect->setDiffuseLight(lightIndex,	diffuseLight);
		effect->setSpecularLight(lightIndex, specularLight);
		effect->setPositionLight(lightIndex, positionLight);
		effect->setLightAttenuationFactors(lightIndex, constant, linear, quadratic);

		if(state->getLightStatus(lightIndex))
			effect->enableLight(lightIndex);
		else
			effect->disableLight(lightIndex);
	}

	state->getGlobalAmbientLightState(global_ambient);
	effect->setGlobalAmbientLight(global_ambient);

	// Enable attributes for non-fixed attributes
	effect->configureVertexAttribArray();

	// Pass	the	theme information from this	widget to OpenGL ES	shaders	(if	any)
	widget->passWidgetThemeInfoToOGL ();
}

LC_IMPORT void LcOglCContext::switchEffect(int effectIndex,	bool bHighQuality)
{
	int	totalLightCount	= 0;
	float mv[16], proj[16],	normal[16];
	float ambient[4], diffuse[4], specular[4], emissive[4];
	float ambientLight[4], diffuseLight[4],	specularLight[4], positionLight[4];
	float shininess	= 0;
	float global_ambient[4];
	float constant,	linear,	quadratic;
	static int internalState = -1;

	LcOglCGlobalState *state = getGlobalState();
	bool pointLightStatus =	state->getPointLightStatus();
	totalLightCount	= state->getEnableLightCount();		//TH

	int	internalMask = ((int(pointLightStatus))	<< 9) |	(totalLightCount <<	5);;

	// Return if we	are	already	using the same effect with same	quality	settings
	if(	(effectIndex ==	m_usageIndex) && (m_highQualityEnabled == bHighQuality)	)
	{
		if((internalState == (internalMask | effectIndex)) || (internalState ==	-1))
		{
			return;
		}
	}

	setEffectUsageIndex(effectIndex);

	switch (effectIndex)
	{
		case BACKGROUND_EFFECT_INDEX:
		{
			// load	BACKGROUND_EFFECT_INDEX
			effectIndex	= BACKGROUND_EFFECT_INDEX;
			internalState =	internalMask | BACKGROUND_EFFECT_INDEX;
		}
		break;

		case ALPHALIGHT00_EFFECT_INDEX:
		{
			internalState =	internalMask | ALPHALIGHT00_EFFECT_INDEX;

			//secondar_light_flag =	0, light_count = 1,	load ALPHALIGHT01_EFFECT_INDEX
			if((pointLightStatus ==	false) && (totalLightCount == 1))
			{
				effectIndex	= bHighQuality ? ALPHALIGHT01_HIGH_EFFECT_INDEX	: ALPHALIGHT01_EFFECT_INDEX;
			}

			//secondar_light_flag =	0, light_count >= 2, load ALPHALIGHT00_EFFECT_INDEX
			if((state->getPointLightStatus() ==	false) && (totalLightCount >= 2))
			{
				effectIndex	= bHighQuality ? ALPHALIGHT00_HIGH_EFFECT_INDEX	: ALPHALIGHT00_EFFECT_INDEX;
			}

			//secondar_light_flag =	1, light_count = 2,	load ALPHALIGHT02_EFFECT_INDEX
			if((state->getPointLightStatus() ==	true) && (totalLightCount == 2))
			{
				effectIndex	= bHighQuality ? ALPHALIGHT02_HIGH_EFFECT_INDEX	: ALPHALIGHT02_EFFECT_INDEX;
			}

			//secondar_light_flag =	1, light_count >= 2, load ALPHALIGHT00_EFFECT_INDEX
			if((state->getPointLightStatus() ==	true) && (totalLightCount >	2))
			{
				effectIndex	= bHighQuality ? ALPHALIGHT00_HIGH_EFFECT_INDEX	: ALPHALIGHT00_EFFECT_INDEX;
			}
		}
		break;

		case LIGHT00_EFFECT_INDEX:
		{
			internalState =	internalMask | LIGHT00_EFFECT_INDEX;

			//secondar_light_flag =	0, light_count = 1,	load LIGHT01_EFFECT_INDEX
			if((state->getPointLightStatus() ==	false) && (totalLightCount == 1))
			{
				effectIndex	= bHighQuality ? LIGHT01_HIGH_EFFECT_INDEX : LIGHT01_EFFECT_INDEX;
			}

			//secondar_light_flag =	0, light_count >= 2, load LIGHT00_EFFECT_INDEX
			else if((state->getPointLightStatus() ==	false) && (totalLightCount >= 2))
			{
				effectIndex	= bHighQuality ? LIGHT00_HIGH_EFFECT_INDEX : LIGHT00_EFFECT_INDEX;
			}

			//secondar_light_flag =	1, light_count = 2,	load LIGHT02_EFFECT_INDEX
			else if((state->getPointLightStatus() ==	true) && (totalLightCount == 2))
			{
				effectIndex	= bHighQuality ? LIGHT02_HIGH_EFFECT_INDEX : LIGHT02_EFFECT_INDEX;
			}

			//secondar_light_flag =	1, light_count = 3,	load LIGHT00_EFFECT_INDEX
			else if((state->getPointLightStatus() ==	true) && (totalLightCount >	2))
			{
				effectIndex	= bHighQuality ? LIGHT00_HIGH_EFFECT_INDEX : LIGHT00_EFFECT_INDEX;
			}
			else
			{
				effectIndex	= bHighQuality ? LIGHT00_HIGH_EFFECT_INDEX : LIGHT00_EFFECT_INDEX;
			}
		}
		break;

		case TEXLIGHT00_EFFECT_INDEX:
		{
			internalState =	internalMask | TEXLIGHT00_EFFECT_INDEX;

			//secondar_light_flag =	0, light_count = 1,	load TEXLIGHT01_EFFECT_INDEX
			if((state->getPointLightStatus() ==	false) && (totalLightCount == 1))
			{
				effectIndex	= bHighQuality ? TEXLIGHT01_HIGH_EFFECT_INDEX :	TEXLIGHT01_EFFECT_INDEX;
			}

			//secondar_light_flag =	0, light_count >= 2, load TEXLIGHT00_EFFECT_INDEX
			if((state->getPointLightStatus() ==	false) && (totalLightCount >= 2))
			{
				effectIndex	= bHighQuality ? TEXLIGHT00_HIGH_EFFECT_INDEX :	TEXLIGHT00_EFFECT_INDEX;
			}

			//secondar_light_flag =	1, light_count = 2,	load TEXLIGHT02_EFFECT_INDEX
			if((state->getPointLightStatus() ==	true) && (totalLightCount == 2))
			{
				effectIndex	= bHighQuality ? TEXLIGHT02_HIGH_EFFECT_INDEX :	TEXLIGHT02_EFFECT_INDEX;
			}

			//secondar_light_flag =	1, light_count = 3,	load TEXLIGHT00_EFFECT_INDEX
			if((state->getPointLightStatus() ==	true) && (totalLightCount >	2))
			{
				effectIndex	= bHighQuality ? TEXLIGHT00_HIGH_EFFECT_INDEX :	TEXLIGHT00_EFFECT_INDEX;
			}
		}
		break;

		default:
			// Invalid case ...
		break;
	}

	m_highQualityEnabled = bHighQuality;

	// Retrieve	default	effect from	the	map
	LcTmOwnerMap<LcTmString, LcOglCEffect>::iterator it;
	LcOglCEffect* defaultEffect	= NULL;

	for(it=m_defaultEffectMap.begin(); it != m_defaultEffectMap.end(); it++)
	{
		defaultEffect =	(*it).second;

		if ( (defaultEffect->getUsageIndex() ==	effectIndex) &&
			 (defaultEffect->getHighQualityLightingStatus()	== bHighQuality) )
		{
			setCurrentEffect(defaultEffect);
			break;
		}
	}

	LcOglCEffect* effect = getCurrentEffect();
	if(effect)
		effect->makeCurrent();
	else
		return;
	state->getMVMatrixState(mv);
	state->getProjMatrixState(proj);
	state->getNormalMatrixState(normal);

	effect->setModelViewMatrix(mv);
	effect->setProjectionMatrix(proj);
	effect->setNormalMatrix(normal);

	state->getMaterialAmbientState(ambient);
	state->getMaterialDiffuseState(diffuse);
	state->getMaterialSpecularState(specular);
	state->getMaterialEmissionState(emissive);
	state->getMaterialShininessState(&shininess);

	effect->setAmbientMaterial(ambient);
	effect->setDiffuseMaterial(diffuse);
	effect->setSpecularMaterial(specular);
	effect->setEmissionMaterial(emissive);
	effect->setShininessMaterial(shininess);

	for(int	lightIndex=0; lightIndex<IFX_OGL_NUM_LIGHTS; lightIndex++)
	{
		state->getLightAmbientState(lightIndex,	ambientLight);
		state->getLightDiffuseState(lightIndex,	diffuseLight);
		state->getLightSpecularState(lightIndex, specularLight);
		state->getLightPositionState(lightIndex, positionLight);
		state->getLightAttenuationFactorsState(lightIndex, &constant, &linear, &quadratic);

		effect->setAmbientLight(lightIndex,	ambientLight);
		effect->setDiffuseLight(lightIndex,	diffuseLight);
		effect->setSpecularLight(lightIndex, specularLight);
		effect->setPositionLight(lightIndex, positionLight);
		effect->setLightAttenuationFactors(lightIndex, constant, linear, quadratic);

		if(state->getLightStatus(lightIndex))
			effect->enableLight(lightIndex);
		else
			effect->disableLight(lightIndex);
	}

	state->getGlobalAmbientLightState(global_ambient);

	effect->setGlobalAmbientLight(global_ambient);

	// Enable attributes for non-fixed attributes
	effect->configureVertexAttribArray();
}

/* --------------------	 addCustomEffect  -------------------------*/
LC_EXPORT LcOglCEffect*	LcOglCContext::addCustomEffect(LcTmString name)
{
	TmEffectMap::iterator it = m_customEffectMap.find(name);

	// First see if	the	Effect is already in the cache
	if (it != m_customEffectMap.end())
	{
		// Map already contains	the	effect implies the fact	that this effect
		// has been	cached ...
		(*it).second->setCached(true);

		return (*it).second;
	}
	else
	{
		// Custom Effect is	not	cached,	so create one
		LcTaOwner<LcOglCEffect>	customEffect = LcOglCEffect::create(m_space);
		if(customEffect)
		{
			m_customEffectMap.add_element(name,	customEffect);

			return m_customEffectMap[name];
		}
	}

	// Failure
	return NULL;
}

/* --------------------	 addDefaultEffect  -------------------------*/
LC_EXPORT LcOglCEffect*	LcOglCContext::addDefaultEffect(LcTmString name)
{
	TmEffectMap::iterator it = m_defaultEffectMap.find(name);

	// First see if	the	Effect is already in the default effect	cache
	if (it != m_defaultEffectMap.end())
	{
		// Map already contains	the	effect implies the fact	that this effect
		// has been	cached ...
		(*it).second->setCached(true);

		return (*it).second;
	}
	else
	{
		// Default Effect is not cached, so	create one
		LcTaOwner<LcOglCEffect>	defaultEffect =	LcOglCEffect::create(m_space);
		if(defaultEffect)
		{
			m_defaultEffectMap.add_element(name, defaultEffect);

			return m_defaultEffectMap[name];
		}
	}

	// Failure
	return NULL;
}

/* --------------------	 getEffectByName  -------------------------*/
LC_EXPORT LcOglCEffect*	LcOglCContext::getEffectByName(LcTmString name)
{
	return (m_customEffectMap[name]);
}

#endif	/* #if defined(LC_PLAT_OGL_20) */

#endif // #if defined(LC_PLAT_OGL)
