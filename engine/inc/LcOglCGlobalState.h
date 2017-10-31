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
#ifndef LcOglCGlobalStateH
#define LcOglCGlobalStateH

/*-------------------------------------------------------------------------*//**
*/
#if defined(LC_PLAT_OGL_20)

#define 	IFX_OGL_NUM_LIGHTS		3
#define 	IFX_OGL_MAX_TEXTURES	4

// Available engine mappings
typedef enum
{
	STD_UNIFORM_NONE,

	// Uniforms
	IFX_MODELVIEW,
	IFX_PROJECTION,
	IFX_NORMAL,
	IFX_CURRENT_TEXTURE,
	IFX_PRIMARY_TEXTURE = IFX_CURRENT_TEXTURE,
	IFX_SECONDARY_TEXTURE_0,
	IFX_SECONDARY_TEXTURE_1,
	IFX_SECONDARY_TEXTURE_2,
	IFX_MATERIAL_AMBIENT,
	IFX_MATERIAL_DIFFUSE,
	IFX_MATERIAL_SPECULAR,
	IFX_MATERIAL_EMISSIVE,
	IFX_MATERIAL_EXPONENT,
	IFX_GLOBAL_AMBIENT,
	IFX_AMBIENT_LIGHT_0,
	IFX_AMBIENT_LIGHT_1,
	IFX_AMBIENT_LIGHT_2,
	IFX_DIFFUSE_LIGHT_0,
	IFX_DIFFUSE_LIGHT_1,
	IFX_DIFFUSE_LIGHT_2,
	IFX_SPECULAR_LIGHT_0,
	IFX_SPECULAR_LIGHT_1,
	IFX_SPECULAR_LIGHT_2,
	IFX_LIGHT_POSITION_0,
	IFX_LIGHT_POSITION_1,
	IFX_LIGHT_POSITION_2,
	IFX_ATTENUATION_FACTORS_0,
	IFX_ATTENUATION_FACTORS_1,
	IFX_ATTENUATION_FACTORS_2,
	IFX_COMPUTE_ATTENUATION_0,
	IFX_COMPUTE_ATTENUATION_1,
	IFX_COMPUTE_ATTENUATION_2,
	IFX_LIGHT_STATE_0,
	IFX_LIGHT_STATE_1,
	IFX_LIGHT_STATE_2,
	IFX_ELEMENT_LOCATION,
	IFX_ELEMENT_OPACITY,
	IFX_TAP_X,
	IFX_TAP_Y,
	IFX_TAP_Z,
	IFX_TAP_XY,
	IFX_TAP_XYZ,
	IFX_SCREEN_WIDTH,
	IFX_SCREEN_HEIGHT,

	// Attributes
	IFX_VERTEX_COORDS,
	IFX_NORMAL_COORDS,
	IFX_TEXTURE_COORDS,
	IFX_TANGENT_COORDS,
	IFX_BINORMAL_COORDS,

	// Vertex position on 2D plane {(0,0) - (1,1)}
	IFX_VERTEX_POSX_PLANE,
	IFX_VERTEX_POSY_PLANE,

	STD_UNIFORM_END
} TEngineEffectVariable;

//Shader basic data types.
enum ShaderDataType
{
    ELcInt,
    ELcBool,
    ELcFloat,

	ELcIVec2,
    ELcIVec3,
    ELcIVec4,

    ELcBVec2,
    ELcBVec3,
    ELcBVec4,

	ELcVec2,
    ELcVec3,
    ELcVec4,

    ELcMat2,
    ELcMat3,
    ELcMat4,

    ELcSampler2D,
    ELcSamplerCube
};

typedef	struct _lights
{
	float	ambient[4];
	float	diffuse[4];
	float	specular[4];
	float	position[4];
	float	attenuation[3];
	int		status;

}IFX_OGL_LIGHTS;

typedef struct _lights_statistics
{
	unsigned int	enableLightCount;
	unsigned int 	enablePointLightCount;
	unsigned int	enablePointLightBin[IFX_OGL_NUM_LIGHTS];
	bool			isSecondaryLightEnable;
}IFX_OGL_LIGHTS_STATISTICS;

class LcOglCGlobalState : public LcCBase
{
private:
	float				m_normalMatrix[16];
	float				m_mvMatrix[16];
	float				m_projMatrix[16];

	float				m_globalAmbient[4];
	bool				m_globalLightingStatus;

	float 				m_materialAmbient[4];
	float 				m_materialDiffuse[4];
	float 				m_materialSpecular[4];
	float 				m_materialEmissive[4];
	float 				m_materialShininess;

	IFX_OGL_LIGHTS		m_lights[IFX_OGL_NUM_LIGHTS];
	IFX_OGL_LIGHTS_STATISTICS	m_lightStatistics;
	LcCSpace*			m_space;

protected:
	LC_IMPORT						LcOglCGlobalState()				{}
	LC_IMPORT		void			construct(LcCSpace* space);

public:
	//Construction
	LC_IMPORT static LcTaOwner<LcOglCGlobalState> create(LcCSpace* space);
	LC_VIRTUAL						~LcOglCGlobalState();

	// Map to associate the strings with the enum values
	typedef	LcTmMap<LcTmString, ShaderDataType>		TmShaderDataType;
	TmShaderDataType 	m_shaderDataTypeMap;
	typedef	LcTmMap<LcTmString, TEngineEffectVariable>	TmEngineEffectVariable;
	TmEngineEffectVariable	m_engineEffectVariableMap;

	// Array to associate the engine mappings with their data types
	ShaderDataType 	m_engineMappings[STD_UNIFORM_END];

	LC_IMPORT void initEngineMappingTable();
	LC_IMPORT void	initShaderDataType();

	ShaderDataType getShaderDataType(TEngineEffectVariable uniformType)	{ return m_engineMappings[uniformType]; }
	ShaderDataType getShaderDataType(LcTmString dataType)	{return m_shaderDataTypeMap[dataType]; }
	TEngineEffectVariable	getEngineEffectVariable(LcTmString variableName)	{return m_engineEffectVariableMap[variableName]; }

	LC_IMPORT void	setNormalMatrixState(float *matrix);
	LC_IMPORT void	getNormalMatrixState(float *matrix);

	LC_IMPORT void	setMVMatrixState(float *matrix);
	LC_IMPORT void	getMVMatrixState(float *matrix);

	LC_IMPORT void	setProjMatrixState(float *matrix);
	LC_IMPORT void	getProjMatrixState(float *matrix);

	LC_IMPORT void 	setMaterialAmbientState(float* ambient);
	LC_IMPORT void	setMaterialDiffuseState(float* diffuse);
	LC_IMPORT void	setMaterialSpecularState(float* specular);
	LC_IMPORT void	setMaterialEmissionState(float* emissive);
	LC_IMPORT void 	setMaterialShininessState(float shininess);

	LC_IMPORT void 	getMaterialAmbientState(float* ambient);
	LC_IMPORT void	getMaterialDiffuseState(float* diffuse);
	LC_IMPORT void	getMaterialSpecularState(float* specular);
	LC_IMPORT void	getMaterialEmissionState(float* emissive);
	LC_IMPORT void 	getMaterialShininessState(float* shininess);

	LC_IMPORT void 	setLightAmbientState(const int lightIndex, float* ambientLight);
	LC_IMPORT void	setLightDiffuseState(const int lightIndex, float* diffuse);
	LC_IMPORT void	setLightSpecularState(const int lightIndex, float* specular);
	LC_IMPORT void	setLightPositionState(const int lightIndex, float* position);
	LC_IMPORT void	setGlobalAmbientLightState(float* global_ambient);

	LC_IMPORT void 	getLightAmbientState(const int lightIndex, float* ambientLight);
	LC_IMPORT void	getLightDiffuseState(const int lightIndex, float* diffuse);
	LC_IMPORT void	getLightSpecularState(const int lightIndex, float* specular);
	LC_IMPORT void	getLightPositionState(const int lightIndex, float* position);
	LC_IMPORT void	getGlobalAmbientLightState(float* global_ambient);

	LC_IMPORT void 	enableGlobalLighting(void);
	LC_IMPORT void 	disableGlobalLighting(void);
	LC_IMPORT bool 	getGlobalLightingStatus(void) const;

	LC_IMPORT void 	setLightEnableState(const int index);
	LC_IMPORT void 	setLightDisableState(const int index) ;
	LC_IMPORT int 	getLightStatus(int index) const;
	LC_IMPORT int	getEnableLightCount();
	LC_IMPORT int	getEnablePointLightCount();
	LC_IMPORT void	incPointLightCount(int index);
	LC_IMPORT void	decPointLightCount(int index);
	LC_IMPORT bool  getPointLightStatus();

	LC_IMPORT void	setLightAttenuationFactorsState(const int lightIndex, float constant, float linear, float quadratic);
	LC_IMPORT void	getLightAttenuationFactorsState(const int lightIndex, float* constant, float* linear, float* quadratic);
};

#elif defined (LC_PLAT_OGL)
	#define 	IFX_OGL_MAX_TEXTURES	1
#endif	//#if defined (LC_PLAT_OGL_20)

#endif /* LcOglCGlobalStateH */
