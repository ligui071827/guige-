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

#include "inflexionui/engine/inc/LcOglCGlobalState.h"

#if defined (LC_PLAT_OGL_20)
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcOglCGlobalState> LcOglCGlobalState::create(LcCSpace* space)
{
	LcTaOwner<LcOglCGlobalState> ref;
	ref.set(new LcOglCGlobalState);
	ref->construct(space);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCGlobalState::initShaderDataType()
{
	// Scalar types
	m_shaderDataTypeMap["int"]			=	ELcInt;
    m_shaderDataTypeMap["bool"]  		=   ELcBool;
    m_shaderDataTypeMap["float"]  		=   ELcFloat;

	// Integer vectors
	m_shaderDataTypeMap["ivec2"]  		=   ELcIVec2;
    m_shaderDataTypeMap["ivec3"]  		=   ELcIVec3;
    m_shaderDataTypeMap["ivec4"]  		=   ELcIVec4;

	// Boolean vectors
	m_shaderDataTypeMap["bvec2"]  		=   ELcBVec2;
    m_shaderDataTypeMap["bvec3"]  		=   ELcBVec3;
    m_shaderDataTypeMap["bvec4"]  		=   ELcBVec4;

	// Float vectors
    m_shaderDataTypeMap["vec2"]  		=   ELcVec2;
    m_shaderDataTypeMap["vec3"]  		=   ELcVec3;
    m_shaderDataTypeMap["vec4"]  		=   ELcVec4;

	// Square Matrices
    m_shaderDataTypeMap["mat2"]  		=   ELcMat2;
    m_shaderDataTypeMap["mat3"]  		=   ELcMat3;
    m_shaderDataTypeMap["mat4"]  		=   ELcMat4;

	// 2D and Cube Textures
    m_shaderDataTypeMap["sampler2D"]  	=	ELcSampler2D;
    m_shaderDataTypeMap["samplerCube"]  =   ELcSamplerCube;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCGlobalState::initEngineMappingTable()
{
	//----------------------------------
	// Supported GLSL ES Attributes
	//----------------------------------
	// Vertex coordinates
	m_engineMappings[IFX_VERTEX_COORDS]   = ELcVec3;
	m_engineEffectVariableMap["IFX_VERTEX_COORDS"] = IFX_VERTEX_COORDS;

	// Normal coordinates
	m_engineMappings[IFX_NORMAL_COORDS]   = ELcVec3;
	m_engineEffectVariableMap["IFX_NORMAL_COORDS"]   = IFX_NORMAL_COORDS;

	// Texture coordinates
	m_engineMappings[IFX_TEXTURE_COORDS]  = ELcVec2;
	m_engineEffectVariableMap["IFX_TEXTURE_COORDS"]  = IFX_TEXTURE_COORDS;

	// Tangent coordinates (used to operate in tangent space)
	m_engineMappings[IFX_TANGENT_COORDS]  = ELcVec3;
	m_engineEffectVariableMap["IFX_TANGENT_COORDS"]  = IFX_TANGENT_COORDS;

	// Bi-Tangent/Bi-Normal coordinates (used to operate in tangent space)
	m_engineMappings[IFX_BINORMAL_COORDS] = ELcVec3;
	m_engineEffectVariableMap["IFX_BINORMAL_COORDS"] = IFX_BINORMAL_COORDS;

	m_engineMappings[IFX_VERTEX_POSX_PLANE] = ELcFloat;
	m_engineEffectVariableMap["IFX_VERTEX_POSX_PLANE"] = IFX_VERTEX_POSX_PLANE;
	
	m_engineMappings[IFX_VERTEX_POSY_PLANE] = ELcFloat;
	m_engineEffectVariableMap["IFX_VERTEX_POSY_PLANE"] = IFX_VERTEX_POSY_PLANE;	

	//----------------------------------
	// Supported GLSL ES Uniforms
	//----------------------------------

	// ModelView transformation matrix
	m_engineMappings[IFX_MODELVIEW] = ELcMat4;
	m_engineEffectVariableMap["IFX_MODELVIEW"] = IFX_MODELVIEW;

	// Projection transformation matrix
	m_engineMappings[IFX_PROJECTION] = ELcMat4;
	m_engineEffectVariableMap["IFX_PROJECTION"] = IFX_PROJECTION;

	// Normal transformation matrix (Inverse-Transpose of ModelView matrix)
	m_engineMappings[IFX_NORMAL] = ELcMat3;
	m_engineEffectVariableMap["IFX_NORMAL"] = IFX_NORMAL;

	// Current Active Texture -> Corresponds to graphic resource
	m_engineMappings[IFX_PRIMARY_TEXTURE] = ELcSampler2D;
	m_engineEffectVariableMap["IFX_CURRENT_TEXTURE"] = IFX_PRIMARY_TEXTURE;
	m_engineEffectVariableMap["IFX_PRIMARY_TEXTURE"] = IFX_PRIMARY_TEXTURE;

	// Secondary samplers
	m_engineMappings[IFX_SECONDARY_TEXTURE_0] = ELcSampler2D;
	m_engineEffectVariableMap["IFX_SECONDARY_TEXTURE_0"] = IFX_SECONDARY_TEXTURE_0;
	m_engineMappings[IFX_SECONDARY_TEXTURE_1] = ELcSampler2D;
	m_engineEffectVariableMap["IFX_SECONDARY_TEXTURE_1"] = IFX_SECONDARY_TEXTURE_1;
	m_engineMappings[IFX_SECONDARY_TEXTURE_2] = ELcSampler2D;
	m_engineEffectVariableMap["IFX_SECONDARY_TEXTURE_2"] = IFX_SECONDARY_TEXTURE_2;

	// Material Ambient Color
	m_engineMappings[IFX_MATERIAL_AMBIENT] = ELcVec4;
	m_engineEffectVariableMap["IFX_MATERIAL_AMBIENT"] = IFX_MATERIAL_AMBIENT;

	// Material Diffuse Color
	m_engineMappings[IFX_MATERIAL_DIFFUSE] = ELcVec4;
	m_engineEffectVariableMap["IFX_MATERIAL_DIFFUSE"] = IFX_MATERIAL_DIFFUSE;

	// Material Specular Color
	m_engineMappings[IFX_MATERIAL_SPECULAR] = ELcVec4;
	m_engineEffectVariableMap["IFX_MATERIAL_SPECULAR"] = IFX_MATERIAL_SPECULAR;

	// Material Emissive Color
	m_engineMappings[IFX_MATERIAL_EMISSIVE] = ELcVec4;
	m_engineEffectVariableMap["IFX_MATERIAL_EMISSIVE"] = IFX_MATERIAL_EMISSIVE;

	// Material Specular Exponent
	m_engineMappings[IFX_MATERIAL_EXPONENT] = ELcFloat;
	m_engineEffectVariableMap["IFX_MATERIAL_EXPONENT"] = IFX_MATERIAL_EXPONENT;

	// Material Global Ambient Color
	m_engineMappings[IFX_GLOBAL_AMBIENT] = ELcVec4;
	m_engineEffectVariableMap["IFX_GLOBAL_AMBIENT"] = IFX_GLOBAL_AMBIENT;

	// Light properties
	for (int i=0; i < IFX_OGL_NUM_LIGHTS; i++)
	{
		m_engineMappings[IFX_AMBIENT_LIGHT_0 + i] = ELcVec4;
		m_engineEffectVariableMap["IFX_AMBIENT_LIGHT_" + LcTaString().fromInt(i)] = (TEngineEffectVariable)(IFX_AMBIENT_LIGHT_0 + i);
		m_engineMappings[IFX_DIFFUSE_LIGHT_0 + i] = ELcVec4;
		m_engineEffectVariableMap["IFX_DIFFUSE_LIGHT_" + LcTaString().fromInt(i)] = (TEngineEffectVariable)(IFX_DIFFUSE_LIGHT_0 + i);
		m_engineMappings[IFX_SPECULAR_LIGHT_0 + i] = ELcVec4;
		m_engineEffectVariableMap["IFX_SPECULAR_LIGHT_" + LcTaString().fromInt(i)] = (TEngineEffectVariable)(IFX_SPECULAR_LIGHT_0 + i);
		m_engineMappings[IFX_LIGHT_POSITION_0 + i] = ELcVec4;
		m_engineEffectVariableMap["IFX_LIGHT_POSITION_" + LcTaString().fromInt(i)] = (TEngineEffectVariable)(IFX_LIGHT_POSITION_0 + i);
		m_engineMappings[IFX_ATTENUATION_FACTORS_0 + i] = ELcVec3;
		m_engineEffectVariableMap["IFX_ATTENUATION_FACTORS_" + LcTaString().fromInt(i)] = (TEngineEffectVariable)(IFX_ATTENUATION_FACTORS_0 + i);
		m_engineMappings[IFX_COMPUTE_ATTENUATION_0 + i] = ELcBool;
		m_engineEffectVariableMap["IFX_COMPUTE_ATTENUATION_" + LcTaString().fromInt(i)] = (TEngineEffectVariable)(IFX_COMPUTE_ATTENUATION_0 + i);
		m_engineMappings[IFX_LIGHT_STATE_0 + i] = ELcBool;
		m_engineEffectVariableMap["IFX_LIGHT_STATE_" + LcTaString().fromInt(i)] = (TEngineEffectVariable)(IFX_LIGHT_STATE_0 + i);
	}

	// Element location in 3D space
	m_engineMappings[IFX_ELEMENT_LOCATION] = ELcVec3;
	m_engineEffectVariableMap["IFX_ELEMENT_LOCATION"] = IFX_ELEMENT_LOCATION;

	// Element opcaity value
	m_engineMappings[IFX_ELEMENT_OPACITY] = ELcFloat;
	m_engineEffectVariableMap["IFX_ELEMENT_OPACITY"] = IFX_ELEMENT_OPACITY;

	// Tap X Location
	m_engineMappings[IFX_TAP_X] = ELcFloat;
	m_engineEffectVariableMap["IFX_TAP_X"] = IFX_TAP_X;

	// Tap Y Location
	m_engineMappings[IFX_TAP_Y] = ELcFloat;
	m_engineEffectVariableMap["IFX_TAP_Y"] = IFX_TAP_Y;

	// Tap Z Location
	m_engineMappings[IFX_TAP_Z] = ELcFloat;
	m_engineEffectVariableMap["IFX_TAP_Z"] = IFX_TAP_Z;

	// Tap XY Location
	m_engineMappings[IFX_TAP_XY] = ELcVec2;
	m_engineEffectVariableMap["IFX_TAP_XY"] = IFX_TAP_XY;

	// Tap XYZ Location
	m_engineMappings[IFX_TAP_XYZ] = ELcVec3;
	m_engineEffectVariableMap["IFX_TAP_XYZ"] = IFX_TAP_XYZ;

	// Screen Width
	m_engineMappings[IFX_SCREEN_WIDTH] = ELcInt;
	m_engineEffectVariableMap["IFX_SCREEN_WIDTH"] = IFX_SCREEN_WIDTH;

	// Screen Height
	m_engineMappings[IFX_SCREEN_HEIGHT] = ELcInt;
	m_engineEffectVariableMap["IFX_SCREEN_HEIGHT"] = IFX_SCREEN_HEIGHT;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCGlobalState::construct(LcCSpace* space)
{
	m_space = space;
	m_lightStatistics.enableLightCount = 0;
	m_lightStatistics.enablePointLightCount = 0;

	for (int i = 0; i < IFX_OGL_NUM_LIGHTS; i++)
		m_lightStatistics.enablePointLightBin[i] = 0;

	m_lightStatistics.isSecondaryLightEnable = false;
}

LC_EXPORT_VIRTUAL LcOglCGlobalState::~LcOglCGlobalState()
{
	// Clear shader data type map
	m_shaderDataTypeMap.clear();
}

LC_EXPORT void LcOglCGlobalState::setNormalMatrixState(float *matrix)
{
	memcpy(m_normalMatrix, matrix, sizeof (float) * 16);
}

LC_EXPORT void LcOglCGlobalState::setMVMatrixState(float *matrix)
{
	memcpy(m_mvMatrix, matrix, sizeof (float) * 16);
}

LC_EXPORT void LcOglCGlobalState::setProjMatrixState(float *matrix)
{
	memcpy(m_projMatrix, matrix, sizeof (float) * 16);
}

LC_EXPORT void LcOglCGlobalState::setMaterialAmbientState(float* ambient)
{
	memcpy(m_materialAmbient, ambient, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::setMaterialDiffuseState(float* diffuse)
{
	memcpy(m_materialDiffuse, diffuse, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::setMaterialSpecularState(float* specular)
{
	memcpy(m_materialSpecular, specular, sizeof(float) *4);
}

LC_EXPORT void LcOglCGlobalState::setMaterialEmissionState(float* emissive)
{
	memcpy(m_materialEmissive, emissive, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::setMaterialShininessState(float shininess)
{
	m_materialShininess = shininess;
}

LC_EXPORT void LcOglCGlobalState::setLightAmbientState(const int lightIndex, float* ambientLight)
{
	if (lightIndex < IFX_OGL_NUM_LIGHTS)
		memcpy(m_lights[lightIndex].ambient, ambientLight, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::setLightDiffuseState(const int lightIndex, float* diffuse)
{
	if (lightIndex < IFX_OGL_NUM_LIGHTS)
		memcpy(m_lights[lightIndex].diffuse, diffuse, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::setLightSpecularState(const int lightIndex, float* specular)
{
	if (lightIndex < IFX_OGL_NUM_LIGHTS)
		memcpy(m_lights[lightIndex].specular, specular, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::setLightPositionState(const int lightIndex, float* position)
{
	if (lightIndex < IFX_OGL_NUM_LIGHTS)
		memcpy(m_lights[lightIndex].position, position, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::setGlobalAmbientLightState(float* global_ambient)
{
	memcpy(m_globalAmbient, global_ambient, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::enableGlobalLighting(void)
{
	m_globalLightingStatus = true;
}

LC_EXPORT void LcOglCGlobalState::disableGlobalLighting(void)
{
	m_globalLightingStatus = false;
}

LC_EXPORT void LcOglCGlobalState::setLightEnableState(const int index)
{
	if (index < IFX_OGL_NUM_LIGHTS)
	{
		m_lights[index].status = GL_TRUE;

		if(m_lightStatistics.enableLightCount < IFX_OGL_NUM_LIGHTS)
			m_lightStatistics.enableLightCount++;
	}
}

LC_EXPORT void LcOglCGlobalState::setLightDisableState(const int index)
{
	if (index < IFX_OGL_NUM_LIGHTS)
	{
		m_lights[index].status = GL_FALSE;

		if(m_lightStatistics.enableLightCount > 1)
			m_lightStatistics.enableLightCount--;
	}
}

LC_EXPORT int LcOglCGlobalState::getLightStatus(int index) const
{
	if (index < IFX_OGL_NUM_LIGHTS)
		return m_lights[index].status;
	return -1;
}

LC_EXPORT int LcOglCGlobalState::getEnableLightCount()
{
	int count = 0;

	for(int index=0; index < IFX_OGL_NUM_LIGHTS; index++)
	{
		if (m_lights[index].status == GL_TRUE)
			count++;
	}

	return (count);
}

LC_EXPORT int LcOglCGlobalState::getEnablePointLightCount()
{
	m_lightStatistics.enablePointLightCount = m_lightStatistics.enablePointLightBin[0] +
											  m_lightStatistics.enablePointLightBin[1] +
											  m_lightStatistics.enablePointLightBin[2];

	return (m_lightStatistics.enablePointLightCount);
}

LC_EXPORT void LcOglCGlobalState::incPointLightCount(int index)
{
	m_lightStatistics.enablePointLightBin[index] = 1;

	m_lightStatistics.enablePointLightCount++;

	if(m_lightStatistics.enablePointLightCount > 0)
		m_lightStatistics.isSecondaryLightEnable = true;
}

LC_EXPORT void LcOglCGlobalState::decPointLightCount(int index)
{
	m_lightStatistics.enablePointLightBin[index] = 0;

	m_lightStatistics.enablePointLightCount--;

	if(m_lightStatistics.enablePointLightCount <= 0)
		m_lightStatistics.isSecondaryLightEnable = false;
}

LC_EXPORT bool LcOglCGlobalState::getPointLightStatus()
{
	return(m_lightStatistics.isSecondaryLightEnable);
}

LC_EXPORT void LcOglCGlobalState::setLightAttenuationFactorsState(const int lightIndex, float constant, float linear, float quadratic)
{
	if (lightIndex < IFX_OGL_NUM_LIGHTS)
	{
		m_lights[lightIndex].attenuation[0] = constant;
		m_lights[lightIndex].attenuation[1] = linear;
		m_lights[lightIndex].attenuation[2] = quadratic;
	}
}


/* get functions */
LC_EXPORT void LcOglCGlobalState::getNormalMatrixState(float *matrix)
{
	memcpy(matrix, m_normalMatrix, sizeof (float) * 16);
}

LC_EXPORT void LcOglCGlobalState::getMVMatrixState(float *matrix)
{
	memcpy(matrix, m_mvMatrix, sizeof (float) * 16);
}

LC_EXPORT void LcOglCGlobalState::getProjMatrixState(float *matrix)
{
	memcpy(matrix, m_projMatrix, sizeof(float) * 16);
}

LC_EXPORT void LcOglCGlobalState::getMaterialAmbientState(float* ambient)
{
	memcpy(ambient, m_materialAmbient, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::getMaterialDiffuseState(float* diffuse)
{
	memcpy(diffuse, m_materialDiffuse, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::getMaterialSpecularState(float* specular)
{
	memcpy(specular, m_materialSpecular, sizeof(float) *4);
}

LC_EXPORT void LcOglCGlobalState::getMaterialEmissionState(float* emissive)
{
	memcpy(emissive, m_materialEmissive, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::getMaterialShininessState(float* shininess)
{
	 *shininess = m_materialShininess;
}

LC_EXPORT void LcOglCGlobalState::getLightAmbientState(const int lightIndex, float* ambientLight)
{
	if (lightIndex < IFX_OGL_NUM_LIGHTS)
		memcpy(ambientLight, m_lights[lightIndex].ambient, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::getLightDiffuseState(const int lightIndex, float* diffuse)
{
	if (lightIndex < IFX_OGL_NUM_LIGHTS)
		memcpy(diffuse, m_lights[lightIndex].diffuse, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::getLightSpecularState(const int lightIndex, float* specular)
{
	if (lightIndex < IFX_OGL_NUM_LIGHTS)
		memcpy(specular, m_lights[lightIndex].specular, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::getLightPositionState(const int lightIndex, float* position)
{
	if (lightIndex < IFX_OGL_NUM_LIGHTS)
		memcpy(position, m_lights[lightIndex].position, sizeof(float) * 4);
}

LC_EXPORT void LcOglCGlobalState::getGlobalAmbientLightState(float* global_ambient)
{
	memcpy(global_ambient, m_globalAmbient, sizeof(float) * 4);
}

LC_EXPORT bool LcOglCGlobalState::getGlobalLightingStatus(void) const
{
	return (m_globalLightingStatus);
}

LC_EXPORT void LcOglCGlobalState::getLightAttenuationFactorsState(const int lightIndex, float* constant, float* linear, float* quadratic)
{
	if (lightIndex < IFX_OGL_NUM_LIGHTS)
	{
		*constant = m_lights[lightIndex].attenuation[0];
		*linear = m_lights[lightIndex].attenuation[1];
		*quadratic = m_lights[lightIndex].attenuation[2];
	}
}

#endif	/* #if defined (LC_PLAT_OGL_20) */
