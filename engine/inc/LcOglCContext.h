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
#ifndef LcOglCContextH
#define LcOglCContextH

#include "inflexionui/engine/inc/LcCBase.h"

#define		IFX_OGL_LIGHT_00						0x0
#define		IFX_OGL_LIGHT_01						0x1
#define		IFX_OGL_LIGHT_02						0x2

// Default Effects' Index
#define		BACKGROUND_EFFECT_INDEX					0x1

#define		ALPHALIGHT00_EFFECT_INDEX				0x2
#define		ALPHALIGHT01_EFFECT_INDEX				0x3
#define		ALPHALIGHT02_EFFECT_INDEX				0x4

#define		LIGHT00_EFFECT_INDEX					0x5
#define		LIGHT01_EFFECT_INDEX					0x6
#define		LIGHT02_EFFECT_INDEX					0x7

#define		TEXLIGHT00_EFFECT_INDEX					0x8
#define		TEXLIGHT01_EFFECT_INDEX					0x9
#define		TEXLIGHT02_EFFECT_INDEX					0xA

#define		ALPHALIGHT00_HIGH_EFFECT_INDEX			0xB
#define		ALPHALIGHT01_HIGH_EFFECT_INDEX			0xC
#define		ALPHALIGHT02_HIGH_EFFECT_INDEX			0xD

#define		LIGHT00_HIGH_EFFECT_INDEX				0xE
#define		LIGHT01_HIGH_EFFECT_INDEX				0xF
#define		LIGHT02_HIGH_EFFECT_INDEX				0x10

#define		TEXLIGHT00_HIGH_EFFECT_INDEX			0x11
#define		TEXLIGHT01_HIGH_EFFECT_INDEX			0x12
#define		TEXLIGHT02_HIGH_EFFECT_INDEX			0x13

class LcCSpace;
class LcCWidget;
class LcTPlacement;
class LcOglCGPUCaps;

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	class	LcOglCEffect;
	class	LcOglCGlobalState;
	class	LcOglCCubeTexture;
	class	LcCCustomBitmapFile;
	class	CTextureInfo;
#endif

#ifndef GL_MULTISAMPLE
	#define GL_MULTISAMPLE 0x809D
#endif

#define		EGL_COVERAGE_BUFFERS_NV					0x30E0
#define		EGL_COVERAGE_SAMPLES_NV					0x30E1

// Use GLfixed for OpenGL parameters if possible (or required)
#if defined(LC_OGL_COMMON_LITE)

	typedef GLfixed						LcOglTScalar;
	#define LC_OGL_SCALAR_TYPE			GL_FIXED
	#define LC_OGL_FROM_FLOAT(v)		(GLfixed((v) * 0x10000))
	#define LC_OGL_TO_FLOAT(v)			(float(v) / 0x10000)
	#define LC_OGL_FX(name)				name##x
	#define LC_OGL_FXV(name)			name##xv
	#define LC_OGL_IX(name)				name##x
	#define LC_OGL_IXV(name)			name##xv
	#define LC_OGL_X(name)				name##x
	#define LC_OGL_XV(name)				name##xv

	// Conversions if using floats for coords, and GLfixed for OpenGL params
	#define LC_OGL_FROM_SCALAR(v)		LC_OGL_FROM_FLOAT(v)
	#define LC_OGL_FROM_UNIT_SCALAR(v)	LC_OGL_FROM_FLOAT(v)
	#define LC_OGL_TO_SCALAR(v)			LC_OGL_TO_FLOAT(v)
	#define LC_OGL_TO_UNIT_SCALAR(v)	LC_OGL_TO_FLOAT(v)

	// OpenGL ES defines glClearDepthx not glClearDepth
	#define LC_OGL_CLEAR_DEPTH		glClearDepthx
	#define LC_OGL_DEPTH_RANGE		glDepthRangex
#else

	// Use floats for both coordinates and OpenGL parameters
	typedef GLfloat						LcOglTScalar;
	#define LC_OGL_SCALAR_TYPE			GL_FLOAT
	#define LC_OGL_FROM_SCALAR(v)		GLfloat(v)
	#define LC_OGL_FROM_UNIT_SCALAR(v)	GLfloat(v)
	#define LC_OGL_FROM_FLOAT(v)		GLfloat(v)
	#define LC_OGL_TO_SCALAR(v)			LcTScalar(v)
	#define LC_OGL_TO_UNIT_SCALAR(v)	LcTUnitScalar(v)
	#define LC_OGL_TO_FLOAT(v)			float(v)
	#define LC_OGL_FX(name)				name##f
	#define LC_OGL_FXV(name)			name##fv
	#define LC_OGL_X(name)				name
	#define LC_OGL_XV(name)				name##v

	#if (defined (GL_VERSION_ES_CM_1_0) && (GL_VERSION_ES_CM_1_0 != 0)) || \
		(defined (GL_VERSION_ES_CM_1_1) && (GL_VERSION_ES_CM_1_1 != 0)) || \
		(defined (GL_OES_VERSION_1_0)	&& (GL_OES_VERSION_1_0	 != 0)) || \
		(defined (GL_OES_VERSION_1_1)	&& (GL_OES_VERSION_1_1	 != 0))
		// OpenGL ES does not define I versions, so use F
		#define LC_OGL_IX(name)			name##f
		#define LC_OGL_IXV(name)		name##fv

		// OpenGL ES defines glClearDepthf not glClearDepth
		#define LC_OGL_CLEAR_DEPTH		glClearDepthf
		#define LC_OGL_DEPTH_RANGE		glDepthRangef

	#elif (defined (GL_ES_VERSION_2_0) && (GL_ES_VERSION_2_0 != 0))
		// OpenGL ES does not define I versions, so use F
		#define LC_OGL_IX(name)			name##f
		#define LC_OGL_IXV(name)		name##fv

		// OpenGL ES 2.0 defines glClearDepthf not glClearDepth
		#define LC_OGL_CLEAR_DEPTH		glClearDepthf
		#define LC_OGL_DEPTH_RANGE		glDepthRangef

	#else
		#define LC_OGL_IX(name)			name##i
		#define LC_OGL_IXV(name)		name##iv

		#define LC_OGL_CLEAR_DEPTH		glClearDepth
		#define LC_OGL_DEPTH_RANGE		glDepthRange
	#endif
#endif


/*-------------------------------------------------------------------------*//**
*/
class LcOglCContext : public LcCBase
{
private:

	// The space this is owned by.
	LcCSpace*						m_space;

	// Instance of GPU Capabilities class
	LcTmOwner<LcOglCGPUCaps>		m_gpuCaps;

	// Current material values, for avoiding redundant changes
	LcOglTScalar					m_curMatAmb[4];
	LcOglTScalar					m_curMatDif[4];
	LcOglTScalar					m_curMatSpc[4];
	LcOglTScalar					m_curMatEmm[4];
	LcOglTScalar					m_curMatShi;

	// Current depth mask value, for avoiding redundant changes
	GLboolean						m_curDepthMask;
	GLboolean						m_curCullFace;
	GLboolean						m_curBlend;

	// Primary light settings.
	LcTColor						m_ambientLightColor;
	LcTColor						m_primaryLightColor;
	LcTQuaternion					m_primaryLightDirection;
	bool							m_primaryLightEnabled;
	LcTColor						m_currentlySetAmbientLightColor;
	LcTColor						m_currentlySetPrimaryLightColor;

	// Scissoring / viewport.
	GLint							m_xOffset;
	GLint							m_yOffset;

	bool							m_enableDithering;

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	int								m_usageIndex;

	typedef LcTmOwnerMap<LcTmString, LcOglCEffect>	TmEffectMap;
	TmEffectMap						m_customEffectMap;

	TmEffectMap						m_defaultEffectMap;

	LcOglCEffect*					m_currentEffect;
	LcTmOwner<LcOglCGlobalState>	m_oglGlobalState;
	LcTmOwner<LcOglCCubeTexture>	m_cubeTexture;
	CTextureInfo*					m_textureInfo;

	bool							m_highQualityEnabled;
	bool							m_openGLInitialized;

	bool							m_bProgramBinarySupported;
	PFNGLPROGRAMBINARYOESPROC		m_fn_glProgramBinaryOES;
	PFNGLGETPROGRAMBINARYOESPROC	m_fn_glGetProgramBinaryOES;

	// Cube map cache (6 faces | textureID)
	typedef LcTmMap<LcTmString, GLuint>	TmMCubemapMap;
	TmMCubemapMap						m_cubemapMap;

	// Load NDI Cube Bitmaps
	LC_IMPORT		GLuint 			loadCubeBitmaps(const LcTmString& file1,
													const LcTmString& file2,
													const LcTmString& file3,
													const LcTmString& file4,
													const LcTmString& file5,
													const LcTmString& file6);

	// Load non-NDI Custom Cube Bitmaps
	LC_IMPORT		GLuint 			loadCustomCubeBitmaps(const LcTmString& file1,
														  const LcTmString& file2,
														  const LcTmString& file3,
														  const LcTmString& file4,
														  const LcTmString& file5,
														  const LcTmString& file6);

	// Read texture data for each custom cube bitmap face
	LC_IMPORT		void			readCubeTextureFaceData (LcCCustomBitmapFile* bmf,
															 LcOglCCubeTexture* cubeTexture,
															 LcTPixelDim size,
															 GLenum target,
															 GLenum format);
#endif

	// Private Helpers
	LC_IMPORT		void			drawAmbientLightColor(LcTColor col);
	LC_IMPORT		void			drawPrimaryLightColor(LcTColor col);
	LC_IMPORT		void			drawPrimaryLightDirection();

protected:

	// Abstract so keep constructor protected
	LC_IMPORT						LcOglCContext()				{}
	LC_IMPORT		void			construct(LcCSpace* space);

public:

	// Construction / Destruction
	LC_IMPORT static LcTaOwner<LcOglCContext> create(LcCSpace* space);
	LC_VIRTUAL						~LcOglCContext();

	// Methods
	LC_IMPORT		void			setupOpenGL(void);			// Init OpenGL for OpenGL 1.1 and 2.0 versions.
	LC_IMPORT		void			cleanup();

#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	LC_IMPORT		bool			loadDefaultEffects(void);
	LC_IMPORT		bool			unloadDefaultEffects(void);
	LC_IMPORT		bool			loadCustomEffects(void);
	LC_IMPORT		bool			unloadCustomEffects(void);
	LC_IMPORT		bool			loadCustomEffect(LcTmString effectName);

	LC_IMPORT		void			switchEffect(int effectIndex, bool bHighQuality);
	LC_IMPORT		void			switchEffect(int effectIndex, LcCWidget *widget);
	LC_IMPORT		bool			getCustomEffectTranslucency(int effectIndex, LcCWidget *widget);

	LC_IMPORT		LcOglCEffect*	addCustomEffect(LcTmString name);
	LC_IMPORT		LcOglCEffect*	addDefaultEffect(LcTmString name);

	LC_IMPORT		LcTmOwner<LcOglCEffect> 	constructEffect(LcTmString& name);

	LC_IMPORT		LcOglCEffect*	getEffectByName(LcTmString name);
	LC_IMPORT		LcOglCGlobalState*	getGlobalState() { return m_oglGlobalState.ptr(); }

	LC_IMPORT int	getEffectUsageIndex() {	return m_usageIndex;	}
	LC_IMPORT void	setEffectUsageIndex(int index = LIGHT00_EFFECT_INDEX) {	m_usageIndex = index;	}

	LC_IMPORT void setCurrentEffect(LcOglCEffect *effect)	{	m_currentEffect = effect;	}
	LC_IMPORT LcOglCEffect*	getCurrentEffect() { return m_currentEffect; }

	LC_IMPORT	void				setTextureInfo (CTextureInfo* textureInfo);
	LC_IMPORT	CTextureInfo*		getTextureInfo ();

	// function pointers for programbinary and getProgramBinary GL APIs
	LC_IMPORT PFNGLPROGRAMBINARYOESPROC	programBinaryFP() { return m_fn_glProgramBinaryOES; }
	LC_IMPORT PFNGLGETPROGRAMBINARYOESPROC	getProgramBinaryFP() { return m_fn_glGetProgramBinaryOES; }

	LC_IMPORT bool						isProgramBinarySupported () { return m_bProgramBinarySupported; }

	LC_IMPORT	GLuint				getCubeBitmaps(const LcTmString& file1,
													const LcTmString& file2,
													const LcTmString& file3,
													const LcTmString& file4,
													const LcTmString& file5,
													const LcTmString& file6);
#endif

	LC_IMPORT		void			boundsUpdated();
	LC_IMPORT		void			transformsChanged(LcCWidget* w);
	LC_IMPORT		void			transformsChanged(LcCWidget* w, LcTTransform& xfm);
	LC_IMPORT		void			transformsChanged(LcTTransform& xfm);
	inline			LcCSpace*		getSpace()		{	return m_space;			}

	LC_IMPORT		LcOglCGPUCaps*	getGPUCaps()	{	return m_gpuCaps.ptr();	}

	// Change current material, avoiding redundant GL calls
	LC_IMPORT		void			setMaterialAmbient(LcOglTScalar* param);
	LC_IMPORT		void			setMaterialDiffuse(LcOglTScalar* param);
	LC_IMPORT		void			setMaterialSpecular(LcOglTScalar* param);
	LC_IMPORT		void			setMaterialEmission(LcOglTScalar* param);
	LC_IMPORT		void			setMaterialShininess(LcOglTScalar param);
	LC_IMPORT		void			setMaterialFlatColor(LcTColor rgb, LcTUnitScalar opacity);
	LC_IMPORT		void			setDepthMask(GLboolean flag);
	LC_IMPORT		void			setCullFace(GLboolean flag);
	LC_IMPORT		void			setBlend(GLboolean flag);

	// Combine Material properties from source and classes section
	LC_IMPORT		void			combineMaterialSpecularColors (LcOglTScalar* param);
	LC_IMPORT		void			combineMaterialEmissiveColors (LcOglTScalar* param);
	LC_IMPORT		void			combineMaterialSpecularShininess (LcOglTScalar* param);

	// Primary light access
	LC_IMPORT		LcTColor		getAmbientLightColor() { return m_ambientLightColor; }
	LC_IMPORT		LcTColor		getPrimaryLightColor() { return m_primaryLightColor; }
	LC_IMPORT		void			updatePrimaryLight(const LcTPlacement& p, int mask);

	// Dithering
	LC_IMPORT		void			setEnableDithering(bool value) { m_enableDithering = value; }

	// Scissoring support
	LC_IMPORT		void			scissor(GLint x, GLint y, GLsizei width, GLsizei height);

	// Clear the cached state
	LC_IMPORT		void			clearCache();

	// Clamp specified value within [min, max] range
	template<typename T> inline T clampValue(const T value, const T min, const T max)
	{
		T result = value;

		// Check against minima
		if(result < min)
			result = min;
		else // Check against maxima
		if(result > max)
			result = max;

		return result;
	}

};

#endif	//LcOglCContextH
