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
#ifndef	LcOglCEffectH
#define	LcOglCEffectH

#if	defined(IFX_RENDER_DIRECT_OPENGL_20)

typedef enum
{
	EBitmap = 0,
	EMesh

} ERenderTarget;

/*-------------------------------------------------------------------------*//**
*/
class	CTextureInfo : public	LcCBase
{
	GLuint							m_id;
	int								m_type;
	int								m_unit;
	int								m_location;
	bool							m_mipmap;
	bool                            m_isPOT;
	int                             m_wrapMode;
	LcCBitmap*						m_pBitmap;
	TEngineEffectVariable			m_mapping;

protected:
									CTextureInfo() {}

public:
	static	LcTaOwner<CTextureInfo>	create();
	LC_VIRTUAL						~CTextureInfo() {}

	inline			GLuint			getId() { return m_id; }
	inline			int				getType() { return m_type; }
	inline			int				getUnit() { return m_unit; }
	inline			int				getLocation() { return m_location; }
	inline			int				getMapping() { return m_mapping; }
	inline			bool			getMipmap() { return m_mipmap; }
	inline          bool            isPOT() { return m_isPOT; }
	inline          int             getWrapMode() { return m_wrapMode; }
	inline			LcCBitmap*		getBitmap() { return m_pBitmap; }

	inline			void			setId(GLuint id) { this->m_id = id; }
	inline			void			setType(int type) { this->m_type = type; }
	inline			void			setUnit(int unit) { this->m_unit = unit; }
	inline			void			setLocation(int location) { this->m_location = location; }
	inline			void			setMapping(TEngineEffectVariable mapping) { this->m_mapping = mapping; }
	inline			void			setMipmap(bool mipmap) { this->m_mipmap = mipmap; }
	inline          void            setPOT (bool pot) { this->m_isPOT = pot; }
	inline          void            setWrapMode (int wrapMode) {  m_wrapMode = wrapMode; }
	inline			void			setBitmap(LcCBitmap *bitmap) { this->m_pBitmap = bitmap; }
};

// LcOglCEffect class
class LcOglCEffect : public LcCBase
{
private:
	LcTmOwner<LcOglCGlobalState>	m_oglProgramState;

	typedef	LcTmOwnerMap<LcTmString, LcOglCSLType>	TmSLTypeOwnerMap;
	typedef	LcTmOwnerMap<LcTmString, CTextureInfo>	TmTextureInfoMap;
	typedef	LcTmMap<LcTmString, TEngineEffectVariable>	TmSemanticMap;
	typedef	LcTmMap<TEngineEffectVariable, LcTmString>	TmRSemanticMap;

	LcCSpace*						m_space;
	TmSLTypeOwnerMap				m_attributeMap;
	TmSLTypeOwnerMap				m_uniformMap;
	TmSLTypeOwnerMap				m_configUniformMap;
	TmSemanticMap					m_semanticUniMap;
	LcOglCSLType*					m_slTypeUniArray[STD_UNIFORM_END];
	TmSemanticMap					m_semanticAttMap;
	int								m_locationArray[STD_UNIFORM_END];
	TmTextureInfoMap				m_textureMap;

	int								m_usage;
	LcTmString						m_EffectName;
	int								m_programObject;

	IFX_SIGNATURE					m_signature;

	LcTmString						m_vertexShaderFile;
	LcTmString						m_fragShaderFile;

	bool							m_makesTranslucent;
	bool							m_loaded;
	bool							m_binary;
	bool							m_unifiedBinary;
	unsigned int					m_binaryFormat;

	bool							m_bProgramBinarySupported;
	PFNGLPROGRAMBINARYOESPROC		m_fn_glProgramBinaryOES;
	PFNGLGETPROGRAMBINARYOESPROC	m_fn_glGetProgramBinaryOES;

	bool							m_highQualityLighting;

	bool							m_cached;

					void			setUniformMapping(
										LcTmString&	uniformName,
										LcTmString&	mappingName,
										LcTmString&	mappingType);

					void			setAttributeMapping(
										LcTmString&	uniformName,
										LcTmString&	mappingName,
										LcTmString&	mappingType);

					bool			convertStringToBool(LcTmString& s);

					void			addUniformFromTheme(
										TmSLTypeOwnerMap*	map,
							 			LcTmString									name,
							 			LcTmString									type,
							 			LcTmString*									valuesData	=	NULL);

					void			resetProgramState();

#if	defined(LC_USE_DESKTOP_OGL)
					LcTaString		replacePrecisionFromShaderSource(char *shaderSourceActual);
#endif

					unsigned*		readShaderFile(
										const char*	shaderFileName,
										unsigned*	fileSize,
										bool		isDefaultEffect = false);

					bool			compileShadersFromSource(
											GLuint			vShader,
											unsigned char*	vSrc,
											GLuint			fShader,
											unsigned char*	fSrc);
protected:
	LC_IMPORT						LcOglCEffect() {}
	LC_IMPORT		void			construct(LcCSpace* space);

public:

	//Construction
	LC_IMPORT		static LcTaOwner<LcOglCEffect>	create(LcCSpace* space);
	LC_VIRTUAL						~LcOglCEffect();

	// Utility APIs
	LC_IMPORT		void			addAttribute(
										LcTmString	name,
										LcTmString	type,
										LcTmString	mapping);

	// Add uniforms	from the .effect files
	LC_IMPORT		void			addUniformFromEffect(
										LcTmString	name,
										LcTmString	type,
										LcTmString	mapping,
										LcTmString*	defaultData = NULL,
										LcTmString*	minData = NULL,
										LcTmString*	maxData = NULL,
										bool		mipmap = false);

	LC_IMPORT		void			populateMapWithConfigUniforms(
										TmSLTypeOwnerMap*	map,
										bool									bInterpolatable);

	LC_IMPORT		void			populateMapWithMissingConfigUniforms(
										TmSLTypeOwnerMap*	map,
										bool									bInterpolatable);

	LC_IMPORT		void			addUniformFromTemplate(
										TmSLTypeOwnerMap*	map,
										LcTmString								name,
										LcTmString								type,
										LcTmString*								valuesData = NULL);

	LC_IMPORT		void			addUniformFromPlacement(
										TmSLTypeOwnerMap*	map,
										LcTmString								name,
										LcTmString								type,
										LcTmString*								valuesData = NULL);

	// System	APIs.
	LC_IMPORT		bool			loadEffect(unsigned char* vSrc, unsigned char* fSrc, int vLength, int fLength, bool isDefault);
	LC_IMPORT		bool			loadEffect(bool isCustomEffect = false);
	LC_IMPORT		bool			unloadEffect(bool clearMaps);

#if defined (IFX_USE_PLATFORM_FILES) && !defined(NDHS_JNI_INTERFACE)
	LC_IMPORT		bool			loadCachedShaderProgram(LcTaAlloc<unsigned char> &cachedBinaryBuffer, int* size, bool isDefault);
	LC_IMPORT		bool			saveCachedShaderProgram(LcTaAlloc<unsigned char> &cachedBinaryBuffer, int* size, bool isDefault);
#endif

	LC_IMPORT		void			setVShaderFile(LcTmString& vFile);
	LC_IMPORT		LcTaString		getVShaderFile(void);

	LC_IMPORT		void 			setFShaderFile(LcTmString& fFile);
	LC_IMPORT		LcTaString 		getFShaderFile(void);

	LC_IMPORT		int				getEnumeratedLocation(int mapping);

	inline			int				getProgramObject() { return m_programObject; }

	inline			void			setProgramObject(int programObject) { this->m_programObject = programObject; }

	LC_IMPORT		void			setUsage(LcTmString& usage);
	LC_IMPORT		void			setMakesTranslucent(bool makesTranslucent)	{ m_makesTranslucent = makesTranslucent; }
	inline			bool			getMakesTranslucent() { return m_makesTranslucent; }

	inline			void			setBinary(bool binary) { m_binary = binary; }
	inline			bool			isBinary() { return (m_binary); }

	inline			void			setUnifiedBinary(bool unifiedBinary) { m_unifiedBinary = unifiedBinary; }
	inline			bool			isUnifiedBinary() { return (m_unifiedBinary); }

	inline			void			setBinaryFormat (unsigned int binaryFormat) { m_binaryFormat = binaryFormat; }
	inline			unsigned int	getBinaryFormat() { return m_binaryFormat; }

	inline			void			setHighQualityLightingStatus(bool enhanced) { m_highQualityLighting = enhanced; }
	inline			bool			getHighQualityLightingStatus() { return (m_highQualityLighting); }

	inline			bool			isLoaded() { return m_loaded; }

	inline			void			setCached(bool cached) { m_cached = cached; }
	inline			bool			isCached() { return m_cached; }

					int				getUsageIndex()			{ return m_usage; }

	inline			void			setEffectName(LcTmString& name) { this->m_EffectName = name; }

	inline			LcTaString		getEffectName(void) { return (this->m_EffectName); }

	inline			void			setEffectSignature(IFX_SIGNATURE sign) { m_signature = sign; }

	inline			IFX_SIGNATURE	getEffectSignature() { return m_signature; }
	inline			int				getEffectUsage(void) { return m_usage; }

	// Transformation	APIs.
	LC_IMPORT		int				setNormalMatrix(float* normMatrix);
	LC_IMPORT		int				setModelViewMatrix(float* mvMatrix);
	LC_IMPORT		int				setProjectionMatrix(float* projMatrix);

	// Material Property APIs.
	LC_IMPORT		int				setAmbientMaterial(float* ambient);
	LC_IMPORT		int				setDiffuseMaterial(float* diffuse);
	LC_IMPORT		int				setSpecularMaterial(float* specular);
	LC_IMPORT		int				setEmissionMaterial(float* emissive);
	LC_IMPORT		int				setShininessMaterial(float shininess);

	// Light handling APIs.
	LC_IMPORT		int				setAmbientLight(int lightIndex, float* ambient);
	LC_IMPORT		int				setDiffuseLight(int lightIndex, float* diffuse);
	LC_IMPORT		int				setSpecularLight(int lightIndex, float* specular);
	LC_IMPORT		int				setPositionLight(int lightIndex, float* position);
	LC_IMPORT		int				setGlobalAmbientLight(float* global_ambient);

	LC_IMPORT		int				enableLight(int lightIndex);
	LC_IMPORT		int				disableLight(int lightIndex);

	LC_IMPORT		int				setLightAttenuationFactors(int lightIndex, float constant, float linear, float quadratic);

	LC_IMPORT		void			makeCurrent();

	LC_IMPORT		void 			configureVertexAttribArray(void);

	inline			int				getTextureCount() { return m_textureMap.size(); }

	LC_IMPORT		CTextureInfo*	getTextureInfo(int index);
	LC_IMPORT		CTextureInfo*	getTextureInfoByName(LcTmString& name);
	LC_IMPORT		void			bindTextures(LcOglCTexture *texture, ERenderTarget renderTarget, GLuint currentTextureId = 0);
	LC_IMPORT		void			unbindTextures(LcOglCTexture *texture);
	LC_IMPORT		int				getUniformLocByName(const LcTmString name);
	LC_IMPORT		LcOglCSLType*	getSLTypeByMapping(TEngineEffectVariable effectVariable);
	LC_IMPORT		void			setTextureInfo(LcTmString name, GLuint id, bool isPOT);

	LC_IMPORT		int				passUniformInfoToOGL(LcOglCSLType* slType);

	// Test whether this is a valid uniform (covers mapped + unmapped)
	LC_IMPORT		bool			isValidUniform(LcOglCSLType* slType);

	// Test whether this is a valid effect uniform (covers unmapped only)
	LC_IMPORT		bool			isValidConfigUniform(LcOglCSLType* slType);

	LC_IMPORT		int				restoreConfigurableUniformsToDefaults();

	LC_IMPORT		void			cloneSLType(
										LcOglCSLType*							slType,
										LcTmString								name,
										TmSLTypeOwnerMap*	map);

	LC_IMPORT		void			cleanupTextureResources();

	LC_IMPORT		LcTaString		getFormattedLightString(LcTmString& str, int lightIndex)
									{
										str.replace('#', ('0' + lightIndex));
										return str;
									}
};

#endif /* defined(IFX_RENDER_DIRECT_OPENGL_20) */

#endif
