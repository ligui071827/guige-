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

#if	defined(LC_PLAT_OGL_20)

// Default/Min/Max values for int uniforms (int, ivec2,	ivec3, ivec4)
#define	UNIFORM_DEFAULT_INT_VALUE		0
#define	UNIFORM_MIN_INT_VALUE			0
#define	UNIFORM_MAX_INT_VALUE			255

// Default/Min/Max values for float	uniforms (float, vec2, vec3, vec4)
#define	UNIFORM_DEFAULT_FLOAT_VALUE		0.0f
#define	UNIFORM_MIN_FLOAT_VALUE			0.0f
#define	UNIFORM_MAX_FLOAT_VALUE			1.0f

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcOglCEffect> LcOglCEffect::create(LcCSpace* space)
{
	LcTaOwner<LcOglCEffect>	ref;
	ref.set(new	LcOglCEffect);
	ref->construct(space);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::construct(LcCSpace* space)
{
	m_space	= space;

	m_vertexShaderFile = "";
	m_fragShaderFile = "";

	// Effect is not in	a loaded state as default
	m_loaded = false;

	// A fresh effect is considered	cached once	it has been	added to a map - to	prevent	recreation
	m_cached = false;

	m_makesTranslucent = false;

	for (int i = 0; i < STD_UNIFORM_END; i++)
		m_locationArray[i] = -1;

	// Assume binary shader	option is disabled as default and it is	not	unified	either
	setBinary(false);
	setBinaryFormat	(0);
	setUnifiedBinary (false);

	// Disable high	quality	lighting by	default	- use only with	default	effects!
	setHighQualityLightingStatus (false);

	// Create state	for	program	object associated with this	effect
	m_oglProgramState =	LcOglCGlobalState::create(space);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcOglCEffect::~LcOglCEffect()
{
	// clear resources,	unload everything related to this effect
	unloadEffect (true);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcOglCEffect::convertStringToBool (LcTmString& s)
{
	return (s.compare("1") == 0)
		|| (s.compareNoCase("true")	== 0);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::setUniformMapping(LcTmString& name, LcTmString& mappingName, LcTmString& mappingType)
{
	// Cannot add attribute	blindly	into the mapping table
	// We first	need to	verify whether the specified mapping is	valid with proper
	// "name" and "type"
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();
	TEngineEffectVariable effectVariable = globalState->getEngineEffectVariable(mappingName);

	// compare the two mapping types provided the entry	exists in mapping table
	if (globalState->getShaderDataType(mappingType) == globalState->getShaderDataType(effectVariable))
	{
		// Validated, add to map then
		m_semanticUniMap[name] = effectVariable;
		m_slTypeUniArray[effectVariable] = m_uniformMap[name];
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::setAttributeMapping(LcTmString& name, LcTmString& mappingName,	LcTmString&	mappingType)
{
	// Cannot add attribute	blindly	into the mapping table
	// We first	need to	verify whether the specified mapping is	valid with proper
	// "name" and "type"
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();
	TEngineEffectVariable effectVariable = globalState->getEngineEffectVariable(mappingName);

	// compare the two mapping types provided the entry	exists in mapping table
	if (globalState->getShaderDataType(mappingType) == globalState->getShaderDataType(effectVariable))
	{
		// Validated, add to map then
		m_semanticAttMap[name] = effectVariable;
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::addAttribute(LcTmString name, LcTmString type,	LcTmString mapping)
{
	LcTaOwner<LcOglCSLType>	ptr;

	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();
	ShaderDataType dataType = globalState->getShaderDataType(type);

	switch(dataType)
	{
		//------------------------------------
		// float
		//------------------------------------
		case ELcFloat:
		{
			LcTaOwner< LcOglCSLTypeScalar<float> >attribute	= LcOglCSLTypeScalar<float>::create();

			attribute->setSLType(ELcOglSLTypeScalar);
			attribute->setSLTypeName(name);
			attribute->setDimension(1);

			ptr	= attribute;
		}
		break;

		//------------------------------------
		// vec2
		//------------------------------------
		case ELcVec2:
		{
			LcTaOwner< LcOglCSLTypeVector<float> > attribute = LcOglCSLTypeVector<float>::create(2);

			attribute->setSLType(ELcOglSLTypeVector);
			attribute->setSLTypeName(name);
			attribute->setDimension(2);

			ptr	= attribute;
		}
		break;

		//------------------------------------
		// vec3
		//------------------------------------
		case ELcVec3:
		{
			LcTaOwner< LcOglCSLTypeVector<float> > attribute = LcOglCSLTypeVector<float>::create(3);

			attribute->setSLType(ELcOglSLTypeVector);
			attribute->setSLTypeName(name);
			attribute->setDimension(3);

			ptr	= attribute;
		}
		break;

		//------------------------------------
		// vec4
		//------------------------------------
		case ELcVec4:
		{
			LcTaOwner< LcOglCSLTypeVector<float> > attribute = LcOglCSLTypeVector<float>::create(4);

			attribute->setSLType(ELcOglSLTypeVector);
			attribute->setSLTypeName(name);
			attribute->setDimension(4);

			ptr	= attribute;
		}
		break;

		//------------------------------------
		// Unsupported attribute types
		//------------------------------------
		case ELcMat2:
		case ELcMat3:
		case ELcMat4:
		default:
			// Matrices	are	currently not supported	as attributes in BroadPeak.
			// Other data types	(bool, bvec2, bvec3, bvec4,	int, ivec2,	ivec3, ivec4,
			// sampler2D, samplerCube) cannot be specified as attributes as
			// prescribed by OpenGL	ES shading language	specifications
		break;
	}

	m_attributeMap.add_element(name, ptr);
	setAttributeMapping(name, mapping, type);
}
/*-------------------------------------------------------------------------*//**
	Add	uniforms from the .effect files.
*/
LC_EXPORT void LcOglCEffect::addUniformFromEffect(LcTmString name,
												  LcTmString type,
												  LcTmString mapping,
												  LcTmString* defaultData,
												  LcTmString* minData,
												  LcTmString* maxData,
												  bool mipmap)
{
	LcTaOwner<LcOglCSLType>	ptr;

	// Name	and	type fields	are	mandatory.
	if(name.isEmpty() || type.isEmpty())
	{
		return;
	}

	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();
	ShaderDataType dataType = globalState->getShaderDataType(type);

	switch(dataType)
	{
		//-------------------------------------
		// int
		//-------------------------------------
		case ELcInt:
		{
			int	defaultValue = 0;
			int	minValue = 0;
			int	maxValue = 0;

			LcTaOwner< LcOglCSLTypeScalar<int> > uniform = LcOglCSLTypeScalar<int>::create();

			uniform->setSLType(ELcOglSLTypeScalar);
			uniform->setSLTypeName(name);
			uniform->setDimension(1);

			if(defaultData)
			{
				defaultValue = (defaultData[0].isEmpty()) ?	(UNIFORM_DEFAULT_INT_VALUE)	: defaultData[0].toInt();
			}

			if(minData)
			{
				minValue = (minData[0].isEmpty()) ?	(UNIFORM_MIN_INT_VALUE)	: minData[0].toInt();
			}

			if(maxData)
			{
				maxValue = (maxData[0].isEmpty()) ?	(UNIFORM_MAX_INT_VALUE)	: maxData[0].toInt();
			}

			uniform->setValue(&defaultValue);
			uniform->setDefaultValue(&defaultValue);
			uniform->setMinValue (&minValue);
			uniform->setMaxValue (&maxValue);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// bool
		//-------------------------------------
		case ELcBool:
		{
			bool defaultValue =	false;
			bool minMaxValue = false;

			LcTaOwner< LcOglCSLTypeScalar<bool>	> uniform =	LcOglCSLTypeScalar<bool>::create();

			uniform->setSLType(ELcOglSLTypeScalar);
			uniform->setSLTypeName(name);
			uniform->setDimension(1);

			if(defaultData)
			{
				defaultValue = convertStringToBool(defaultData[0]);
			}

			uniform->setValue(&defaultValue);
			uniform->setDefaultValue(&defaultValue);

			// There is	no min/max for "bool", so we set always	set	it as "false"
			uniform->setMinValue(&minMaxValue);
			uniform->setMaxValue(&minMaxValue);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// float
		//-------------------------------------
		case ELcFloat:
		{
			float defaultValue = 0;
			float minValue = 0;
			float maxValue = 0;

			LcTaOwner< LcOglCSLTypeScalar<float> > uniform = LcOglCSLTypeScalar<float>::create();

			uniform->setSLType(ELcOglSLTypeScalar);
			uniform->setSLTypeName(name);
			uniform->setDimension(1);

			if(defaultData)
			{
				defaultValue = (defaultData[0].isEmpty()) ?	(UNIFORM_DEFAULT_FLOAT_VALUE) :	defaultData[0].toScalar();
			}

			if(minData)
			{
				minValue = (minData[0].isEmpty()) ?	(UNIFORM_MIN_FLOAT_VALUE) :	minData[0].toScalar();
			}

			if(maxData)
			{
				maxValue = (maxData[0].isEmpty()) ?	(UNIFORM_MAX_FLOAT_VALUE) :	maxData[0].toScalar();
			}

			uniform->setValue(&defaultValue);
			uniform->setDefaultValue(&defaultValue);
			uniform->setMinValue(&minValue);
			uniform->setMaxValue(&maxValue);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// ivec2
		//-------------------------------------
		case ELcIVec2:
		{
			int	iDefaultValues [2] = {0};
			int	iMinValues [2] = {0};
			int	iMaxValues [2] = {0};

			if (defaultData)
			{
				iDefaultValues [0] = (defaultData[0].isEmpty())	? (UNIFORM_DEFAULT_INT_VALUE) :	defaultData[0].toInt();
				iDefaultValues [1] = (defaultData[1].isEmpty())	? (UNIFORM_DEFAULT_INT_VALUE) :	defaultData[1].toInt();
			}

			if (minData)
			{
				iMinValues [0] = (minData[0].isEmpty())	? (UNIFORM_MIN_INT_VALUE) :	minData[0].toInt();
				iMinValues [1] = (minData[1].isEmpty())	? (UNIFORM_MIN_INT_VALUE) :	minData[1].toInt();
			}

			if (maxData)
			{
				iMaxValues [0] = (maxData[0].isEmpty())	? (UNIFORM_MAX_INT_VALUE) :	maxData[0].toInt();
				iMaxValues [1] = (maxData[1].isEmpty())	? (UNIFORM_MAX_INT_VALUE) :	maxData[1].toInt();
			}

			LcTaOwner< LcOglCSLTypeVector<int> > uniform = LcOglCSLTypeVector<int>::create(2);
			uniform->setValue(iDefaultValues);

			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(2);

			uniform->setDefaultValue (iDefaultValues);
			uniform->setMinValue (iMinValues);
			uniform->setMaxValue (iMaxValues);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// ivec3
		//-------------------------------------
		case ELcIVec3:
		{
			int	iDefaultValues [3] = {0};
			int	iMinValues [3] = {0};
			int	iMaxValues [3] = {0};

			if (defaultData)
			{
				iDefaultValues [0] = (defaultData[0].isEmpty())	? (UNIFORM_DEFAULT_INT_VALUE) :	defaultData[0].toInt();
				iDefaultValues [1] = (defaultData[1].isEmpty())	? (UNIFORM_DEFAULT_INT_VALUE) :	defaultData[1].toInt();
				iDefaultValues [2] = (defaultData[2].isEmpty())	? (UNIFORM_DEFAULT_INT_VALUE) :	defaultData[2].toInt();
			}

			if (minData)
			{
				iMinValues [0] = (minData[0].isEmpty())	? (UNIFORM_MIN_INT_VALUE) :	minData[0].toInt();
				iMinValues [1] = (minData[1].isEmpty())	? (UNIFORM_MIN_INT_VALUE) :	minData[1].toInt();
				iMinValues [2] = (minData[2].isEmpty())	? (UNIFORM_MIN_INT_VALUE) :	minData[2].toInt();
			}

			if (maxData)
			{
				iMaxValues [0] = (maxData[0].isEmpty())	? (UNIFORM_MAX_INT_VALUE) :	maxData[0].toInt();
				iMaxValues [1] = (maxData[1].isEmpty())	? (UNIFORM_MAX_INT_VALUE) :	maxData[1].toInt();
				iMaxValues [2] = (maxData[2].isEmpty())	? (UNIFORM_MAX_INT_VALUE) :	maxData[2].toInt();
			}

			LcTaOwner< LcOglCSLTypeVector<int> > uniform = LcOglCSLTypeVector<int>::create(3);
			uniform->setValue(iDefaultValues);

			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(3);

			uniform->setDefaultValue (iDefaultValues);
			uniform->setMinValue (iMinValues);
			uniform->setMaxValue (iMaxValues);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// ivec4
		//-------------------------------------
		case ELcIVec4:
		{
			int	iDefaultValues [4] = { 0, 0, 0 , 0 };
			int	iMinValues [4] = { 0 };
			int	iMaxValues [4] = { 1 };

			if (defaultData)
			{
				iDefaultValues [0] = (defaultData[0].isEmpty())	? (UNIFORM_DEFAULT_INT_VALUE) :	defaultData[0].toInt();
				iDefaultValues [1] = (defaultData[1].isEmpty())	? (UNIFORM_DEFAULT_INT_VALUE) :	defaultData[1].toInt();
				iDefaultValues [2] = (defaultData[2].isEmpty())	? (UNIFORM_DEFAULT_INT_VALUE) :	defaultData[2].toInt();
				iDefaultValues [3] = (defaultData[3].isEmpty())	? (UNIFORM_DEFAULT_INT_VALUE) :	defaultData[3].toInt();
			}

			if (minData)
			{
				iMinValues [0] = (minData[0].isEmpty())	? (UNIFORM_MIN_INT_VALUE) :	minData[0].toInt();
				iMinValues [1] = (minData[1].isEmpty())	? (UNIFORM_MIN_INT_VALUE) :	minData[1].toInt();
				iMinValues [2] = (minData[2].isEmpty())	? (UNIFORM_MIN_INT_VALUE) :	minData[2].toInt();
				iMinValues [3] = (minData[3].isEmpty())	? (UNIFORM_MIN_INT_VALUE) :	minData[3].toInt();
			}

			if (maxData)
			{
				iMaxValues [0] = (maxData[0].isEmpty())	? (UNIFORM_MAX_INT_VALUE) :	maxData[0].toInt();
				iMaxValues [1] = (maxData[1].isEmpty())	? (UNIFORM_MAX_INT_VALUE) :	maxData[1].toInt();
				iMaxValues [2] = (maxData[2].isEmpty())	? (UNIFORM_MAX_INT_VALUE) :	maxData[2].toInt();
				iMaxValues [3] = (maxData[3].isEmpty())	? (UNIFORM_MAX_INT_VALUE) :	maxData[3].toInt();
			}

			LcTaOwner< LcOglCSLTypeVector<int> > uniform = LcOglCSLTypeVector<int>::create(4);
			uniform->setValue(iDefaultValues);

			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(4);

			uniform->setDefaultValue (iDefaultValues);
			uniform->setMinValue (iMinValues);
			uniform->setMaxValue (iMaxValues);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// bvec2
		//-------------------------------------
		case ELcBVec2:
		{
			bool bDefaultValues	[2]	= {	false };
			bool bMinMaxValues [2] = { false };

			if(defaultData)
			{
				bDefaultValues[0] =	convertStringToBool(defaultData[0]);
				bDefaultValues[1] =	convertStringToBool(defaultData[1]);
			}

			LcTaOwner< LcOglCSLTypeVector<bool>	> uniform =	LcOglCSLTypeVector<bool>::create(2);
			uniform->setValue(bDefaultValues);

			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(2);

			// There is	no min/max for "bool", so we set always	set	it as "false"
			uniform->setDefaultValue(bDefaultValues);
			uniform->setMinValue(bMinMaxValues);
			uniform->setMaxValue(bMinMaxValues);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// bvec3
		//-------------------------------------
		case ELcBVec3:
		{
			bool bDefaultValues	[3]	= {	false };
			bool bMinMaxValues [3] = { false };

			if(defaultData)
			{
				bDefaultValues[0] =	convertStringToBool(defaultData[0]);
				bDefaultValues[1] =	convertStringToBool(defaultData[1]);
				bDefaultValues[2] =	convertStringToBool(defaultData[2]);
			}

			LcTaOwner< LcOglCSLTypeVector<bool>	> uniform =	LcOglCSLTypeVector<bool>::create(3);
			uniform->setValue(bDefaultValues);

			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(3);

			// There is	no min/max for "bool", so we set always	set	it as "false"
			uniform->setDefaultValue(bDefaultValues);
			uniform->setMinValue(bMinMaxValues);
			uniform->setMaxValue(bMinMaxValues);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// bvec4
		//-------------------------------------
		case ELcBVec4:
		{
			bool bDefaultValues	[4]	= {	false };
			bool bMinMaxValues [4] = { false };

			if(defaultData)
			{
				bDefaultValues[0] =	convertStringToBool(defaultData[0]);
				bDefaultValues[1] =	convertStringToBool(defaultData[1]);
				bDefaultValues[2] =	convertStringToBool(defaultData[2]);
				bDefaultValues[3] =	convertStringToBool(defaultData[3]);
			}

			LcTaOwner< LcOglCSLTypeVector<bool>	> uniform =	LcOglCSLTypeVector<bool>::create(4);
			uniform->setValue(bDefaultValues);

			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(4);

			// There is	no min/max for "bool", so we set always	set	it as "false"
			uniform->setDefaultValue(bDefaultValues);
			uniform->setMinValue(bMinMaxValues);
			uniform->setMaxValue(bMinMaxValues);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// vec2
		//-------------------------------------
		case ELcVec2:
		{
			float fDefaultValues [2] = {0};
			float fMinValues [2] = {0};
			float fMaxValues [2] = {0};

			if (defaultData)
			{
				fDefaultValues [0] = (defaultData[0].isEmpty())	? (UNIFORM_DEFAULT_FLOAT_VALUE)	: defaultData[0].toScalar();
				fDefaultValues [1] = (defaultData[1].isEmpty())	? (UNIFORM_DEFAULT_FLOAT_VALUE)	: defaultData[1].toScalar();
			}

			if (minData)
			{
				fMinValues [0] = (minData[0].isEmpty())	? (UNIFORM_MIN_FLOAT_VALUE)	: minData[0].toScalar();
				fMinValues [1] = (minData[1].isEmpty())	? (UNIFORM_MIN_FLOAT_VALUE)	: minData[1].toScalar();
			}

			if (maxData)
			{
				fMaxValues [0] = (maxData[0].isEmpty())	? (UNIFORM_MAX_FLOAT_VALUE)	: maxData[0].toScalar();
				fMaxValues [1] = (maxData[1].isEmpty())	? (UNIFORM_MAX_FLOAT_VALUE)	: maxData[1].toScalar();
			}

			LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(2);
			uniform->setValue(fDefaultValues);

			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(2);

			uniform->setDefaultValue (fDefaultValues);
			uniform->setMinValue (fMinValues);
			uniform->setMaxValue (fMaxValues);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// vec3
		//-------------------------------------
		case ELcVec3:
		{
			float fDefaultValues [3] = {0.0};
			float fMinValues [3] = {0.0};
			float fMaxValues [3] = {0.0};

			if (defaultData)
			{
				fDefaultValues [0] = (defaultData[0].isEmpty())	? (UNIFORM_DEFAULT_FLOAT_VALUE)	: defaultData[0].toScalar();
				fDefaultValues [1] = (defaultData[1].isEmpty())	? (UNIFORM_DEFAULT_FLOAT_VALUE)	: defaultData[1].toScalar();
				fDefaultValues [2] = (defaultData[2].isEmpty())	? (UNIFORM_DEFAULT_FLOAT_VALUE)	: defaultData[2].toScalar();
			}

			if (minData)
			{
				fMinValues [0] = (minData[0].isEmpty())	? (UNIFORM_MIN_FLOAT_VALUE)	: minData[0].toScalar();
				fMinValues [1] = (minData[1].isEmpty())	? (UNIFORM_MIN_FLOAT_VALUE)	: minData[1].toScalar();
				fMinValues [2] = (minData[2].isEmpty())	? (UNIFORM_MIN_FLOAT_VALUE)	: minData[2].toScalar();
			}

			if (maxData)
			{
				fMaxValues [0] = (maxData[0].isEmpty())	? (UNIFORM_MAX_FLOAT_VALUE)	: maxData[0].toScalar();
				fMaxValues [1] = (maxData[1].isEmpty())	? (UNIFORM_MAX_FLOAT_VALUE)	: maxData[1].toScalar();
				fMaxValues [2] = (maxData[2].isEmpty())	? (UNIFORM_MAX_FLOAT_VALUE)	: maxData[2].toScalar();
			}

			LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(3);
			uniform->setValue(fDefaultValues);

			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(3);

			uniform->setDefaultValue (fDefaultValues);
			uniform->setMinValue (fMinValues);
			uniform->setMaxValue (fMaxValues);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// vec4
		//-------------------------------------
		case ELcVec4:
		{
			float fDefaultValues [4] = {0};
			float fMinValues [4] = {0};
			float fMaxValues [4] = {0};

			if (defaultData)
			{
				fDefaultValues [0] = (defaultData[0].isEmpty())	? (UNIFORM_DEFAULT_FLOAT_VALUE)	: defaultData[0].toScalar();
				fDefaultValues [1] = (defaultData[1].isEmpty())	? (UNIFORM_DEFAULT_FLOAT_VALUE)	: defaultData[1].toScalar();
				fDefaultValues [2] = (defaultData[2].isEmpty())	? (UNIFORM_DEFAULT_FLOAT_VALUE)	: defaultData[2].toScalar();
				fDefaultValues [3] = (defaultData[3].isEmpty())	? (UNIFORM_DEFAULT_FLOAT_VALUE)	: defaultData[3].toScalar();
			}

			if (minData)
			{
				fMinValues [0] = (minData[0].isEmpty())	? (UNIFORM_MIN_FLOAT_VALUE)	: minData[0].toScalar();
				fMinValues [1] = (minData[1].isEmpty())	? (UNIFORM_MIN_FLOAT_VALUE)	: minData[1].toScalar();
				fMinValues [2] = (minData[2].isEmpty())	? (UNIFORM_MIN_FLOAT_VALUE)	: minData[2].toScalar();
				fMinValues [3] = (minData[3].isEmpty())	? (UNIFORM_MIN_FLOAT_VALUE)	: minData[3].toScalar();
			}

			if (maxData)
			{
				fMaxValues [0] = (maxData[0].isEmpty())	? (UNIFORM_MAX_FLOAT_VALUE)	: maxData[0].toScalar();
				fMaxValues [1] = (maxData[1].isEmpty())	? (UNIFORM_MAX_FLOAT_VALUE)	: maxData[1].toScalar();
				fMaxValues [2] = (maxData[2].isEmpty())	? (UNIFORM_MAX_FLOAT_VALUE)	: maxData[2].toScalar();
				fMaxValues [3] = (maxData[3].isEmpty())	? (UNIFORM_MAX_FLOAT_VALUE)	: maxData[3].toScalar();
			}

			LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(4);

			uniform->setValue(fDefaultValues);

			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(4);

			uniform->setDefaultValue (fDefaultValues);
			uniform->setMinValue (fMinValues);
			uniform->setMaxValue (fMaxValues);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// mat2
		 //-------------------------------------
		case ELcMat2:
		{
			LcTaOwner< LcOglCSLTypeMatrix<float> > uniform = LcOglCSLTypeMatrix<float>::create(2);

			uniform->setSLType(ELcOglSLTypeMatrix);
			uniform->setSLTypeName(name);
			uniform->setDimension(2);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// mat3
		//-------------------------------------
		case ELcMat3:
		{
			LcTaOwner< LcOglCSLTypeMatrix<float> > uniform = LcOglCSLTypeMatrix<float>::create(3);

			uniform->setSLType(ELcOglSLTypeMatrix);
			uniform->setSLTypeName(name);
			uniform->setDimension(3);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// mat4
		//-------------------------------------
		case ELcMat4:
		{
			LcTaOwner< LcOglCSLTypeMatrix<float> > uniform = LcOglCSLTypeMatrix<float>::create(4);

			uniform->setSLType(ELcOglSLTypeMatrix);
			uniform->setSLTypeName(name);
			uniform->setDimension(4);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// sampler2D
		//-------------------------------------
		case ELcSampler2D:
		{
			LcTaOwner< LcOglCSLTypeScalar<LcTmString> >	uniform	= LcOglCSLTypeScalar<LcTmString>::create();
			uniform->setSLType(ELcOglSLTypeScalar);
			uniform->setSLTypeName(name);
			uniform->setDimension(1);

			LcTaOwner<CTextureInfo>	texture	= CTextureInfo::create();

			texture->setId(0);
			texture->setType(GL_TEXTURE_2D);
			texture->setUnit(GL_TEXTURE0);
			texture->setLocation(-1);
			texture->setMapping(globalState->getEngineEffectVariable(mapping));
			texture->setMipmap (mipmap);
			texture->setPOT (false);
			texture->setWrapMode (GL_CLAMP);
			m_textureMap.add_element(name, texture);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// samplerCube
		//-------------------------------------
		case ELcSamplerCube:
		{
			LcTaOwner< LcOglCSLTypeVector<LcTmString> >	uniform	= LcOglCSLTypeVector<LcTmString>::create(6);

			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(6);
			ptr	= uniform;

			LcTaOwner<CTextureInfo>	texture	= CTextureInfo::create();
			texture->setId(0);
			texture->setType(GL_TEXTURE_CUBE_MAP);
			texture->setUnit(GL_TEXTURE0);
			texture->setLocation(-1);
			texture->setMapping(globalState->getEngineEffectVariable(mapping));
			texture->setMipmap (mipmap);
			texture->setPOT (false);
			texture->setWrapMode (GL_CLAMP);

			m_textureMap.add_element(name, texture);
		}
		break;

		//-------------------------------------
		// default
		//-------------------------------------
		default:
			// Do nothing ....
		break;

	} // switch

	if (ptr)
	{
		ptr->setSLTypeName(name);
		ptr->setSLTypeIdentifier(type);

		// If uniform is not mapped	to some	semantic
		if(mapping.isEmpty())
		{
			// Add to our configurable (non-mapped uniforms)
			cloneSLType(ptr.ptr(), name, &m_configUniformMap);
		}

		m_uniformMap.add_element(name, ptr);
		setUniformMapping(name,	mapping, type);
	}
}
/*-------------------------------------------------------------------------*//**
	Add	uniforms from theme	(template/component	etc.) specified	via
	<effectUniforms> <uniform> tag on class	level (In ALL Layouts).
*/
LC_EXPORT void LcOglCEffect::addUniformFromTemplate(TmSLTypeOwnerMap* map,
													LcTmString name,
													LcTmString type,
													LcTmString*	valuesData)
{
	if (!map)
	{
		return;
	}

	// "name" and "type" fields	are	mandatory.
	if(name.isEmpty() || type.isEmpty())
	{
		return;
	}

	// Check whether a uniform with	this name and types	does exist in the uniform
	// map for this	effect.	In case	it does	not, we	should return from here	as
	// it makes	sense to discard a uniform whose name and/or type does not
	// match with any of the uniforms defined by this effect
	if (m_uniformMap.find(name)	== m_uniformMap.end())
	{
		// We do not have uniform with this	name in	the	map, thus it is	an invalid
		// effect uniform coming from the theme
		return;
	}
	else
	{
		// Type	check the SLType
		LcOglCSLType *slTypeLocal =	m_uniformMap[name];

		if (!isValidConfigUniform(slTypeLocal))
		{
			// A uniform with this name	exists,	but	that has a different
			// uniform type	than this incoming effect uniform
			return;
		}
	}

	addUniformFromTheme	(map, name,	type, valuesData);
}

/*-------------------------------------------------------------------------*//**
	Add	uniforms from theme	(layout/animations etc.)
	specified via <effectUniforms> <uniform> tag on	layout level (In THIS Layout)
*/
LC_EXPORT void LcOglCEffect::addUniformFromPlacement(TmSLTypeOwnerMap* map,
													 LcTmString	name,
													 LcTmString	type,
													 LcTmString* valuesData)
{
	if (!map)
	{
		return;
	}

	// "name" and "type" fields	are	mandatory.
	if(name.isEmpty() || type.isEmpty())
	{
		return;
	}

	// We cannot always	perform	validation checks against the effect as	uniform	here are
	// arriving	from placements/layouts	etc. and it	is not always possible to
	// establish explicit binding with the effect. For instance, for
	// static animations, it is	not	possible to	know in	advance	to which
	// element effect this animation will be applied to. It	is becasue the decorations
	// are parsed after	animations and only	decoration specify that	binding	which
	// is not known	while parsing animations/static	displacements
	addUniformFromTheme	(map, name,	type, valuesData);
}

/*-------------------------------------------------------------------------*//**
	Add	uniforms from theme	(layout/template etc.)
	specified via <effectUniforms> <uniform> tag
*/
void LcOglCEffect::addUniformFromTheme(TmSLTypeOwnerMap* map,
									   LcTmString name,
									   LcTmString type,
									   LcTmString* valuesData)
{
	LcTaOwner<LcOglCSLType>	ptr;

	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();
	ShaderDataType dataType = globalState->getShaderDataType(type);

	switch(dataType)
	{
		//-------------------------------------
		// int
		//-------------------------------------
		case ELcInt:
		{
			int	value =	0;

			LcTaOwner< LcOglCSLTypeScalar<int> > uniform = LcOglCSLTypeScalar<int>::create();

			uniform->setSLType(ELcOglSLTypeScalar);
			uniform->setSLTypeName(name);
			uniform->setDimension(1);

			if(valuesData)
			{
				value =	(valuesData[0].isEmpty()) ?	(value)	: valuesData[0].toInt();
			}

			uniform->setValue(&value);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// bool
		//-------------------------------------
		case ELcBool:
		{
			bool value = false;

			LcTaOwner< LcOglCSLTypeScalar<bool>	> uniform =	LcOglCSLTypeScalar<bool>::create();

			uniform->setSLType(ELcOglSLTypeScalar);
			uniform->setSLTypeName(name);
			uniform->setDimension(1);

			if(valuesData)
			{
				value =	convertStringToBool(valuesData[0]);
			}

			uniform->setValue(&value);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// float
		//-------------------------------------
		case ELcFloat:
		{
			float value	= 0.0f;

			LcTaOwner< LcOglCSLTypeScalar<float> > uniform = LcOglCSLTypeScalar<float>::create();

			uniform->setSLType(ELcOglSLTypeScalar);
			uniform->setSLTypeName(name);
			uniform->setDimension(1);

			if(valuesData)
			{
				value =	(valuesData[0].isEmpty()) ?	(value)	: valuesData[0].toScalar();
			}

			uniform->setValue(&value);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// ivec2
		//-------------------------------------
		case ELcIVec2:
		{
			int	iValues	[2]	= {	0 };

			if (valuesData)
			{
				iValues	[0]	= (valuesData[0].isEmpty())	? (iValues[0]) : valuesData[0].toInt();
				iValues	[1]	= (valuesData[1].isEmpty())	? (iValues[1]) : valuesData[1].toInt();
			}

			LcTaOwner< LcOglCSLTypeVector<int> > uniform = LcOglCSLTypeVector<int>::create(2);

			uniform->setValue(iValues);
			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(2);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// ivec3
		//-------------------------------------
		case ELcIVec3:
		{
			int	iValues	[3]	= {	0 };

			if (valuesData)
			{
				iValues	[0]	= (valuesData[0].isEmpty())	? (iValues[0]) : valuesData[0].toInt();
				iValues	[1]	= (valuesData[1].isEmpty())	? (iValues[1]) : valuesData[1].toInt();
				iValues	[2]	= (valuesData[2].isEmpty())	? (iValues[2]) : valuesData[2].toInt();
			}

			LcTaOwner< LcOglCSLTypeVector<int> > uniform = LcOglCSLTypeVector<int>::create(3);

			uniform->setValue(iValues);
			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(3);

			ptr	= uniform;

		}
		break;

		//-------------------------------------
		// ivec4
		//-------------------------------------
		case ELcIVec4:
		{
			int	iValues	[4]	= {	0 };

			if (valuesData)
			{
				iValues	[0]	= (valuesData[0].isEmpty())	? (iValues[0]) : valuesData[0].toInt();
				iValues	[1]	= (valuesData[1].isEmpty())	? (iValues[1]) : valuesData[1].toInt();
				iValues	[2]	= (valuesData[2].isEmpty())	? (iValues[2]) : valuesData[2].toInt();
				iValues	[3]	= (valuesData[3].isEmpty())	? (iValues[3]) : valuesData[3].toInt();
			}

			LcTaOwner< LcOglCSLTypeVector<int> > uniform = LcOglCSLTypeVector<int>::create(4);

			uniform->setValue(iValues);
			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(4);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// bvec2
		//-------------------------------------
		case ELcBVec2:
		{
			bool bValues [2] = { false };

			if(valuesData)
			{
				bValues[0] = convertStringToBool(valuesData[0]);
				bValues[1] = convertStringToBool(valuesData[1]);
			}

			LcTaOwner< LcOglCSLTypeVector<bool>	> uniform =	LcOglCSLTypeVector<bool>::create(2);

			uniform->setValue(bValues);
			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(2);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// bvec3
		//-------------------------------------
		case ELcBVec3:
		{
			bool bValues [3] = { false };

			if(valuesData)
			{
				bValues[0] = convertStringToBool(valuesData[0]);
				bValues[1] = convertStringToBool(valuesData[1]);
				bValues[2] = convertStringToBool(valuesData[2]);
			}

			LcTaOwner< LcOglCSLTypeVector<bool>	> uniform =	LcOglCSLTypeVector<bool>::create(3);

			uniform->setValue(bValues);
			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(3);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// bvec4
		//-------------------------------------
		case ELcBVec4:
		{
			bool bValues [4] = { false };

			if(valuesData)
			{
				bValues[0] = convertStringToBool(valuesData[0]);
				bValues[1] = convertStringToBool(valuesData[1]);
				bValues[2] = convertStringToBool(valuesData[2]);
				bValues[3] = convertStringToBool(valuesData[3]);
			}

			LcTaOwner< LcOglCSLTypeVector<bool>	> uniform =	LcOglCSLTypeVector<bool>::create(4);

			uniform->setValue(bValues);
			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(4);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// vec2
		//-------------------------------------
		case ELcVec2:
		{
			float fValues [2] =	{ 0.0f };

			if (valuesData)
			{
				fValues	[0]	= (valuesData[0].isEmpty())	? (fValues[0]) : valuesData[0].toScalar();
				fValues	[1]	= (valuesData[1].isEmpty())	? (fValues[1]) : valuesData[1].toScalar();
			}

			LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(2);

			uniform->setValue(fValues);
			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(2);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// vec3
		//-------------------------------------
		case ELcVec3:
		{
			float fValues [3] =	{ 0.0f };

			if (valuesData)
			{
				fValues	[0]	= (valuesData[0].isEmpty())	? (fValues[0]) : valuesData[0].toScalar();
				fValues	[1]	= (valuesData[1].isEmpty())	? (fValues[1]) : valuesData[1].toScalar();
				fValues	[2]	= (valuesData[2].isEmpty())	? (fValues[2]) : valuesData[2].toScalar();
			}

			LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(3);

			uniform->setValue(fValues);
			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(3);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// vec4
		//-------------------------------------
		case ELcVec4:
		{
			float fValues [4] =	{ 0.0f };

			if (valuesData)
			{
				fValues	[0]	= (valuesData[0].isEmpty())	? (fValues[0]) : valuesData[0].toScalar();
				fValues	[1]	= (valuesData[1].isEmpty())	? (fValues[1]) : valuesData[1].toScalar();
				fValues	[2]	= (valuesData[2].isEmpty())	? (fValues[2]) : valuesData[2].toScalar();
				fValues	[3]	= (valuesData[3].isEmpty())	? (fValues[3]) : valuesData[3].toScalar();
			}

			LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(4);

			uniform->setValue(fValues);
			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(4);

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// sampler2D
		//-------------------------------------
		case ELcSampler2D:
		{
			LcTaOwner< LcOglCSLTypeScalar<LcTmString> >	uniform	= LcOglCSLTypeScalar<LcTmString>::create();

			uniform->setSLType(ELcOglSLTypeScalar);
			uniform->setSLTypeName(name);
			uniform->setDimension(1);

			if(map == NULL)
			{
				LcTaOwner<CTextureInfo>	texture	= CTextureInfo::create();

				texture->setId(0);
				texture->setType(GL_TEXTURE_2D);
				texture->setUnit(GL_TEXTURE0);
				texture->setLocation(-1);
				texture->setPOT (false);
				texture->setWrapMode (GL_CLAMP);

				m_textureMap.add_element(name, texture);
			}
			else
			{
				uniform->setValue(&valuesData[0]);
			}

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// samplerCube
		//-------------------------------------
		case ELcSamplerCube:
		{
			LcTaOwner< LcOglCSLTypeVector<LcTmString> >	uniform	= LcOglCSLTypeVector<LcTmString>::create(6);

			if (map	!= NULL)
			{
				uniform->setValue(valuesData);
			}

			uniform->setSLType(ELcOglSLTypeVector);
			uniform->setSLTypeName(name);
			uniform->setDimension(6);

			if(map == NULL)
			{
				LcTaOwner<CTextureInfo>	texture	= CTextureInfo::create();
				texture->setId(0);
				texture->setType(GL_TEXTURE_CUBE_MAP);
				texture->setUnit(GL_TEXTURE0);
				texture->setLocation(-1);
				texture->setPOT (false);
				texture->setWrapMode (GL_CLAMP);

				m_textureMap.add_element(name, texture);
			}

			ptr	= uniform;
		}
		break;

		//-------------------------------------
		// mat2, mat3 and mat4 are currently not supported as
		// effect uniforms within the theme
		//-------------------------------------
		case ELcMat2:
		case ELcMat3:
		case ELcMat4:
		default:
			// Do nothing ....
		break;
	}

	if (ptr)
	{
		ptr->setSLTypeName(name);
		ptr->setSLTypeIdentifier(type);

		if (map	!= NULL)
		{
			map->add_element(name, ptr);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
unsigned* LcOglCEffect::readShaderFile(const char* shaderFileName, unsigned	*fileSize, bool	isDefaultEffect)
{
	unsigned* shaderData = NULL;
	unsigned int shaderFileSize = 0;
	LcTaOwner<LcCReadOnlyFile> shaderFile;

	if(isDefaultEffect)
		shaderFile = LcCReadOnlyFile::openFile(shaderFileName, EShader);
	else
		shaderFile = LcCReadOnlyFile::openFile(shaderFileName);

	if (shaderFile)
	{
		shaderFileSize = shaderFile->size();
	}

	if (shaderFileSize > 0)
	{
		shaderData = LcTmAlloc<unsigned>::allocUnsafe((shaderFileSize + 1 + 4) / sizeof(unsigned));

		if (shaderData == NULL)
		{
			shaderFile->close();
			return NULL;
		}

		if(shaderData)
			lc_memset(shaderData, 0, shaderFileSize + 1);
		else
			return NULL;

		// Move	the	file pointer to	the	start of the shader	file
		shaderFile->seek(0, LcCReadOnlyFile::EStart);

		if (shaderFileSize != shaderFile->read(shaderData, 1, shaderFileSize))
		{
			// Could not succeed in	reading	the	file contents, so free the data	buffer
			LcTmAlloc<unsigned>::freeUnsafe(shaderData);
			shaderData = NULL;
		}

		shaderFile->close();
	}

	// Set the shader file size	to be returned to the calling function
	*fileSize =	shaderFileSize;

	return (shaderData);
}

/*-------------------------------------------------------------------------*//**
	This function call will	take place in case of loading the custom effect.
*/
LC_EXPORT bool LcOglCEffect::loadEffect(bool isDefaultEffect)
{
	unsigned fileVSize = 0;
	unsigned fileFSize = 0;
	unsigned* vSrc = NULL;
	unsigned* fSrc = NULL;

#if defined(NDHS_TRACE_ENABLED)
	char logString[1024];
#endif

	// Check for the presence of the ProgramBinary extension.
	if(m_space->getOglContext()->isProgramBinarySupported())
	{
		m_fn_glProgramBinaryOES = m_space->getOglContext()->programBinaryFP();
		m_fn_glGetProgramBinaryOES = m_space->getOglContext()->getProgramBinaryFP();

		if(m_fn_glProgramBinaryOES && m_fn_glGetProgramBinaryOES)
		{
			m_bProgramBinarySupported = true;
		}
	}

	// Effect is already loaded, so	need to	reload it
	if (m_loaded ==	true)
	{
		return m_loaded;
	}

	// Source option or	separate binary
	if (!isUnifiedBinary())
	{
		// Read	vertex shader file
		if (isDefaultEffect)
			vSrc = readShaderFile(m_vertexShaderFile.bufUtf8(),	&fileVSize,	isDefaultEffect);
		else
			vSrc = readShaderFile(m_vertexShaderFile.bufUtf8(),	&fileVSize);

		if (vSrc ==	NULL)
		{
#if defined(NDHS_TRACE_ENABLED)
			lc_sprintf(logString, "Failed to read V Shader: %s", m_vertexShaderFile.bufUtf8());
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, logString);
#endif
			return m_loaded;
		}

		// Read	fragment shader	file
		if(isDefaultEffect)
			fSrc = readShaderFile(m_fragShaderFile.bufUtf8(), &fileFSize, isDefaultEffect);
		else
			fSrc = readShaderFile(m_fragShaderFile.bufUtf8(), &fileFSize);
		if (fSrc ==	NULL)
		{
#if defined(NDHS_TRACE_ENABLED)
			lc_sprintf(logString, "Failed to read F Shader: %s", m_fragShaderFile.bufUtf8());
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, logString);
#endif
			return false;
		}
	}
	else	// Unified binary
	{
		// Read	unified	binary shader file - still stored in as	vertex shader file name!
		if(isDefaultEffect)
			vSrc = readShaderFile(m_vertexShaderFile.bufUtf8(),	&fileVSize,	isDefaultEffect);
		else
			vSrc = readShaderFile(m_vertexShaderFile.bufUtf8(),	&fileVSize);
	}

	// Now load	the	effect after data has been read	from files
#if defined(NDHS_TRACE_ENABLED)
	lc_sprintf(logString, "Loading V Shader: %s and F Shader: %s", m_vertexShaderFile.bufUtf8(), m_fragShaderFile.bufUtf8());
	NDHS_TRACE_VERBOSE(ENdhsTraceLevelInfo, ENdhsTraceModuleOGL, logString);
#endif

	loadEffect(	(unsigned char *) vSrc,	(unsigned char *) fSrc,	fileVSize, fileFSize, isDefaultEffect);

	// Free	resources
	if (vSrc !=	NULL)
	{
		LcTmAlloc<unsigned>::freeUnsafe(vSrc);
	}
	if (fSrc !=	NULL)
	{
		LcTmAlloc<unsigned>::freeUnsafe(fSrc);
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool LcOglCEffect::compileShadersFromSource(GLuint	vShader,
											unsigned char* vSrc,
											GLuint fShader,
											unsigned char* fSrc)
{
	bool status = false;
	const char* src = NULL;

	if(vSrc	== NULL	|| fSrc	== NULL)
	{
		return false;
	}

	GLint isCompiled = 0;

#if defined(NDHS_TRACE_ENABLED)
	char infoLog[1024];
	char logString[1280];
#endif


#if	defined(LC_USE_DESKTOP_OGL)
	LcTaString updatedShaderSource = replacePrecisionFromShaderSource((char*)vSrc);
	src = updatedShaderSource.bufUtf8();
#else
	src	= (const char*) vSrc;
#endif

	glShaderSource(vShader,	1, (const char **)&src,	NULL);
	glCompileShader(vShader);
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &isCompiled);

	if(isCompiled)
	{

#if	defined(LC_USE_DESKTOP_OGL)
		updatedShaderSource = replacePrecisionFromShaderSource((char*)fSrc);
		src = updatedShaderSource.bufUtf8();
#else
		src	= (const char *)fSrc;
#endif

		glShaderSource(fShader,	1, (const char **)&src,	NULL);
		glCompileShader(fShader);

		// Check the compile status
		glGetShaderiv(fShader, GL_COMPILE_STATUS, &isCompiled);

#if defined(NDHS_TRACE_ENABLED)
   		if (!isCompiled)
   		{
			// Give the user some chance to find out what has gone wrong
			glGetShaderInfoLog(fShader, 1024, NULL, infoLog);

			lc_sprintf(logString, "Failed to compile fragment shader (%d): %s", fShader, infoLog);
			NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, logString);
   		}
#endif

	}
#if defined(NDHS_TRACE_ENABLED)
	else
	{
		// Give the user some chance to find out what has gone wrong
		glGetShaderInfoLog(vShader, 1024, NULL, infoLog);

		lc_sprintf(logString, "Failed to compile vertex shader (%d): %s", vShader, infoLog);
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, logString);
	}
#endif

	if(isCompiled == 1)
		status = true;

	return status;
}

/*-------------------------------------------------------------------------*//**
	This function loads the shaders associated with the effect.
*/
LC_EXPORT bool LcOglCEffect::loadEffect(unsigned char*	vSrc, unsigned char* fSrc, int vLength,	int	fLength, bool isDefault)
{
	bool status = true;
	GLint	isLinked = 0;
	GLuint	programObject =	0;
	LcTaAlloc<unsigned char> cachedBinaryBuffer;
	int cachedBinaryBufferSize = 0;

	enum ESourceType {ESource, EPreCompileBinary, EUnifiedBinary, ECachedBinary};

	ESourceType	sourceType = ESource;

	if(isBinary())
	{
		sourceType = isUnifiedBinary() ? EUnifiedBinary : EPreCompileBinary;
	}

	GLuint vShader;
	GLuint fShader;

	if(sourceType == ESource ||	sourceType == EPreCompileBinary)
	{
		if(vSrc	== NULL	|| fSrc	== NULL)
		{
			return false;
		}
	}
#if defined (IFX_USE_PLATFORM_FILES) && !defined(NDHS_JNI_INTERFACE)

	// Check to see if we have a cached version of the source shader.
	if (sourceType == ESource && m_bProgramBinarySupported)
	{
		if (loadCachedShaderProgram(cachedBinaryBuffer, &cachedBinaryBufferSize, isDefault))
			sourceType = ECachedBinary;
	}
#endif

	if(sourceType == ESource || sourceType == EPreCompileBinary)
	{
		// Create vertex and fragment shaders handles
		vShader	= glCreateShader(GL_VERTEX_SHADER);
		fShader	= glCreateShader(GL_FRAGMENT_SHADER);
	}

	// Compile vertex and fragment shaders from	source (not	binary)
	if (sourceType == ESource)
	{
		status = compileShadersFromSource(vShader, vSrc, fShader, fSrc);
	}
	else if (sourceType	== EPreCompileBinary)//	Binary
	{
		// Specify vertex shader binary	data
		glShaderBinary (1, &vShader, getBinaryFormat(),	vSrc, vLength);

		// Specify fragment	shader binary data
		glShaderBinary (1, &fShader, getBinaryFormat(),	fSrc, fLength);
	}

	if(status)
	{
		// Create the program object
		programObject =	glCreateProgram	();

		if(sourceType == ESource || sourceType == EPreCompileBinary)
		{
			glAttachShader(programObject, vShader);
			glAttachShader(programObject, fShader);

			// Link the program
			glLinkProgram(programObject);
		}
#if !defined(NDHS_JNI_INTERFACE)
		else if (sourceType == EUnifiedBinary)
		{
			status = false;

			if(vSrc != NULL)
			{
				if (m_bProgramBinarySupported)
				{
					m_fn_glProgramBinaryOES(programObject, getBinaryFormat(), vSrc, vLength);
					status = true;
				}
			}
		}

#if defined (IFX_USE_PLATFORM_FILES)
		else if (sourceType == ECachedBinary)
		{
			m_fn_glProgramBinaryOES(programObject, getBinaryFormat(), cachedBinaryBuffer, cachedBinaryBufferSize);
			status = true;
		}
#endif // defined (IFX_USE_PLATFORM_FILES)
#endif // !defined(NDHS_JNI_INTERFACE)

	}

	if(status)
	{
		// Check the link status
		glGetProgramiv(programObject, GL_LINK_STATUS, &isLinked	);

		if (isLinked)
		{
			// Use this	program
			glUseProgram (programObject);
			setProgramObject(programObject);
			status = true;

#if defined (IFX_USE_PLATFORM_FILES) && !defined(NDHS_JNI_INTERFACE)

			if ((sourceType == ESource) && m_bProgramBinarySupported)
			{
				saveCachedShaderProgram(cachedBinaryBuffer, &cachedBinaryBufferSize, isDefault);
			}
#endif
		}
		else
		{
			#if defined(NDHS_TRACE_ENABLED)
				char infoLog[1024];
				char logString[1280];

				// Give the user some chance to find out what has gone wrong
				glGetProgramInfoLog(programObject, 1024, NULL, infoLog);

				lc_sprintf(logString, "Failed to link program (%d): %s", programObject, infoLog);
				NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, logString);
			#endif

			glDeleteProgram(programObject);
			programObject =	0;
			setProgramObject(programObject);
			status = false;
		}
	}

	if(sourceType == ESource ||	sourceType == EPreCompileBinary)
	{
		glDeleteShader(vShader);
		glDeleteShader(fShader);
	}

	if(!status)
	{
		return status;
	}

	TmSLTypeOwnerMap::reverse_iterator rit;
	LcTaString		localName;
	LcOglCSLType*	localSLType;
	TEngineEffectVariable	localMapping;

	// Traverse	attribute map (contains	all	attributes present in the .effect file)
	for(rit=m_attributeMap.rbegin(); rit !=	m_attributeMap.rend(); rit++)
	{
		localName =	(*rit).first;
		localSLType	= (*rit).second;

		if (!localSLType)	continue;

		GLint loc =	glGetAttribLocation(programObject, localName.bufUtf8());

		localSLType->setLocationIndex(loc);
		localMapping = m_semanticAttMap[localName];

		if (localMapping != STD_UNIFORM_NONE)
			m_locationArray[localMapping]	= loc;
	}

	TmSLTypeOwnerMap::iterator it;

	// Traverse	uniform	map	(contains all uniforms present in the .effect file)
	for(it=m_uniformMap.begin(); it	!= m_uniformMap.end(); it++)
	{
		localName =	(*it).first;
		localSLType	= (*it).second;

		if (!localSLType)
			continue;

		GLint loc =	glGetUniformLocation(programObject,	localName.bufUtf8());
		localSLType->setLocationIndex(loc);
		localMapping = m_semanticUniMap[localName];

		// If the uniform has a	valid location index, set its value	for	the	first
		// time	from the default values	coming from	the	effect file
		if (loc	>= 0)
		{
			passUniformInfoToOGL (localSLType);
		}

		// If uniform is mapped	to some	semantic
		if (localMapping != STD_UNIFORM_NONE)
			m_locationArray[localMapping]	= loc;
	}

	TmTextureInfoMap::iterator tit;

	CTextureInfo* textureInfo =	NULL;
	int	cLoc = getEnumeratedLocation (IFX_PRIMARY_TEXTURE);
	int	i =	0;

	for(tit=m_textureMap.begin(); tit != m_textureMap.end(); tit++)
	{
		localName =	(*tit).first;
		textureInfo	= (*tit).second;

		localSLType	= m_uniformMap[localName];

		if (!localSLType ||	!textureInfo)	continue;

		GLint loc =	localSLType->getLocationIndex();

		textureInfo->setLocation (loc);
		textureInfo->setId (0);

		switch ( localSLType->getSLType() )
		{
			case ELcOglSLTypeScalar:
				textureInfo->setType (GL_TEXTURE_2D);
			break;

			case ELcOglSLTypeVector:
				textureInfo->setType (GL_TEXTURE_CUBE_MAP);
			break;

			default:
				textureInfo->setType (GL_TEXTURE_2D);
			break;
		}

		if (cLoc ==	loc)
		{
			textureInfo->setUnit (GL_TEXTURE0);

			glActiveTexture	(GL_TEXTURE0);
			glUniform1i	(loc, 0);
		}
		else
		{
			textureInfo->setUnit (i	+ GL_TEXTURE1);

			glActiveTexture	(i + GL_TEXTURE1);
			glUniform1i	(loc, i+1);

			i++;
		}
	}

	if (status)
		m_loaded = true;

	return (status);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcOglCEffect::unloadEffect(bool	clearMaps)
{
	if (m_programObject)
	{
		glDeleteProgram	(m_programObject);
		m_programObject	= 0;
	}

	// Cleanup texture bitmaps ... this	is specifically	essential
	// for pause-resume	under Android for texture based	effects	that
	// rely	on additional textures
	cleanupTextureResources	();

	// We have to make sure	that if	the	effect is a	custom one,
	// we cannot clear the maps	at runtime.	Otherwise we will lose
	// the entries present in the map. For default effects,	we can
	// refill the maps so there	is no issue	there. But for custom
	// effects,	we only	delete the program object while	retaining
	// the map entries to use that for re-loading
	// At closeup, all effects can be unloaded without distinction.
	// The distinction only	applies	to unloading-loading sequence at
	// runtime
	if (clearMaps == true)
	{
		// clear attributes	map
		m_attributeMap.clear();

		// clear configurable(effect) uniforms map
		m_configUniformMap.clear();

		// clear uniforms map
		m_uniformMap.clear();

		// clear map for uniform mapping
		m_semanticUniMap.clear();

		// clear map for attribute mapping
		m_semanticAttMap.clear();

		// clear map for texture related information
		m_textureMap.clear();
	}

	// Reset the state of program so that it could be re-initialized once the
	// program object is recreated
	resetProgramState ();

	m_loaded = false;
	m_cached = false;

	return true;
}

#if defined (IFX_USE_PLATFORM_FILES) && !defined(NDHS_JNI_INTERFACE)
/*-------------------------------------------------------------------------*//**
	Attempts to load a binary version of the source shader for faster loading.
*/
LC_EXPORT bool LcOglCEffect::loadCachedShaderProgram(LcTaAlloc<unsigned char> &cachedBinaryBuffer, int* size, bool isDefault)
{
	bool status = true;
	bool fileOpen = false;
	IFXP_FILE file;
	LcTaString fileName;
	GLenum binaryFormat = 0;
	GLsizei programLength = 0;

	if (status)
	{
		LcTaString dir = "";

		if (!isDefault)
		{
			dir = m_vertexShaderFile.subString(0,m_vertexShaderFile.findLastChar(NDHS_PLAT_DIR_SEP_CHAR));
			if (dir.isEmpty())
				status = false;
		}
		else
		{
			dir = "C:" + (LcTaString)NDHS_PLAT_DIR_SEP + "_RuntimeEffects";
		}

		if (status)
		{
			fileName = getEffectName() + ".cached";
			fileName = fileName.tail(fileName.findLastChar(NDHS_PLAT_DIR_SEP_CHAR));
			fileName = dir + fileName;
			if (IFX_SUCCESS != IFXP_File_Open(&file, fileName.bufUtf8()))
				status = false;
			else
				fileOpen = true;
		}
	}

	IFX_UINT32 transferred;
	IFX_UINT32 length;

	// Read the program size as the first 4 bytes.
	if (status)
	{
		if(IFX_SUCCESS != IFXP_File_Read(file, (void*)&length, sizeof(IFX_UINT32), &transferred))
			status = false;
		if (transferred != sizeof(IFX_UINT32))
			status = false;

		programLength = length;
	}

	// Read the effect signature and verify.
	if (status)
	{
		IFX_SIGNATURE signature;
		if(IFX_SUCCESS != IFXP_File_Read(file, (void*)&signature, sizeof(IFX_SIGNATURE), &transferred))
			status = false;
		if (transferred != sizeof(IFX_SIGNATURE))
			status = false;
		if(lc_memcmp(signature.hash, m_signature.hash, IFX_HASH_SIZE * sizeof(IFX_UINT32)) != 0)
			status = false;
	}

	// Read the binary format as the second 4 bytes.
	if (status)
	{
		if(IFX_SUCCESS != IFXP_File_Read(file, (void*)&binaryFormat, sizeof(IFX_UINT32), &transferred))
			status = false;
		if (transferred != sizeof(IFX_UINT32))
			status = false;
	}

	// Read the program data.
	if (status)
	{
		// Set the format
		setBinaryFormat(binaryFormat);

		// Allocate the buffer.
		cachedBinaryBuffer.alloc(programLength);
		*size = programLength;

		lc_memset(cachedBinaryBuffer, 0, programLength);

		if(IFX_SUCCESS != IFXP_File_Read(file, (void*)cachedBinaryBuffer, programLength, &transferred))
			status = false;
		if (transferred != (IFX_UINT32)programLength)
			status = false;
	}

	// Close the file
	if (fileOpen)
		IFXP_File_Close(file);

#if defined(NDHS_TRACE_ENABLED)
	if (status)
	{
		char logString[1024];
		lc_sprintf(logString, "Retrieved cached effect binary: %s", fileName.bufUtf8());
		NDHS_TRACE_VERBOSE(ENdhsTraceLevelInfo, ENdhsTraceModuleOGL, logString);
	}
#endif

	return status;
}

/*-------------------------------------------------------------------------*//**
	Saves a binary version of the source shader for faster loading next time.
*/
LC_EXPORT bool LcOglCEffect::saveCachedShaderProgram(LcTaAlloc<unsigned char> &cachedBinaryBuffer, int* size, bool isDefault)
{
	bool status = true;
	bool fileOpen = false;
	IFXP_FILE file;
	LcTaString fileName;
	GLenum binaryFormat = 0;
	GLsizei programLength = 0;
	GLsizei reqLength = 0;

	// Get the length of the shader binary program in memory.
	glGetProgramiv (getProgramObject(), GL_PROGRAM_BINARY_LENGTH_OES, &programLength);

	if (programLength == 0)
		status = false;

	if (status)
	{
		cachedBinaryBuffer.alloc(programLength);
		lc_memset(cachedBinaryBuffer, 0, programLength);

		// Get the program binary
		reqLength = programLength;
		m_fn_glGetProgramBinaryOES(getProgramObject(), reqLength, &programLength, &binaryFormat, cachedBinaryBuffer);

		if (programLength != reqLength)
		{
			status = false;
		}
		*size = programLength;
	}

	if (status)
	{
		LcTaString dir = "";

		if (!isDefault)
		{
			dir = m_vertexShaderFile.subString(0,m_vertexShaderFile.findLastChar(NDHS_PLAT_DIR_SEP_CHAR));
			if (dir.isEmpty())
				status = false;
		}
		else
		{
			dir = "C:" + (LcTaString)NDHS_PLAT_DIR_SEP + "_RuntimeEffects";
		}

		if (status)
		{
			fileName = getEffectName() + ".cached";
			fileName = fileName.tail(fileName.findLastChar(NDHS_PLAT_DIR_SEP_CHAR));
			fileName = dir + fileName;
			IFXP_Dir_Create(dir.bufUtf8());

			if (IFX_SUCCESS != IFXP_File_Create(&file, fileName.bufUtf8()))
				status = false;
			else
				fileOpen = true;
		}
	}

	IFX_UINT32 transferred;

	// Write the program size as the first 4 bytes.
	if (status)
	{
		IFX_UINT32 length = programLength;
		if(IFX_SUCCESS != IFXP_File_Write(file, (void*)&length, sizeof(IFX_UINT32), &transferred))
			status = false;
		if (transferred != sizeof(IFX_UINT32))
			status = false;
	}

	// Write the effect signature.
	if (status)
	{
		if(IFX_SUCCESS != IFXP_File_Write(file, (void*)&m_signature, sizeof(IFX_SIGNATURE), &transferred))
			status = false;
		if (transferred != sizeof(IFX_SIGNATURE))
			status = false;
	}

	// Write the binary format as the third 4 bytes.
	if (status)
	{
		if(IFX_SUCCESS != IFXP_File_Write(file, (void*)&binaryFormat, sizeof(IFX_UINT32), &transferred))
			status = false;
		if (transferred != sizeof(IFX_UINT32))
			status = false;
	}

	// Write the program data.
	if (status)
	{
		if(IFX_SUCCESS != IFXP_File_Write(file, (void*)cachedBinaryBuffer, programLength, &transferred))
			status = false;
		if (transferred != (IFX_UINT32)programLength)
			status = false;
	}

	// Close the file
	if (fileOpen)
		IFXP_File_Close(file);

#if defined(NDHS_TRACE_ENABLED)
	if (!status)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleOGL, "Failed to save compiled effect binary to cache");
	}
	else
	{
		char logString[1024];
		lc_sprintf(logString, "Saved compiled effect binary to cache: %s", fileName.bufUtf8());
		NDHS_TRACE_VERBOSE(ENdhsTraceLevelInfo, ENdhsTraceModuleOGL, logString);
	}
#endif

	return status;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::resetProgramState ()
{
	if(!m_oglProgramState)
		return;

	float defaultMatrix[16]	= {	0.0	};
	float defaultMaterialLightColor[4] = { 0.0 };

	// Reset transformation	matrices
	m_oglProgramState->setMVMatrixState(defaultMatrix);
	m_oglProgramState->setNormalMatrixState(defaultMatrix);
	m_oglProgramState->setProjMatrixState(defaultMatrix);

	// Reset Material properties
	m_oglProgramState->setMaterialAmbientState(defaultMaterialLightColor);
	m_oglProgramState->setMaterialDiffuseState(defaultMaterialLightColor);
	m_oglProgramState->setMaterialSpecularState(defaultMaterialLightColor);
	m_oglProgramState->setMaterialEmissionState(defaultMaterialLightColor);
	m_oglProgramState->setMaterialShininessState(0.0);

	// Reset Light properties
	for	(int lightIndex=0; lightIndex <	IFX_OGL_NUM_LIGHTS;	lightIndex++)
	{
		m_oglProgramState->setLightAmbientState(lightIndex,	defaultMaterialLightColor);
		m_oglProgramState->setLightDiffuseState(lightIndex,	defaultMaterialLightColor);
		m_oglProgramState->setLightSpecularState(lightIndex, defaultMaterialLightColor);
		m_oglProgramState->setLightPositionState(lightIndex, defaultMaterialLightColor);
		m_oglProgramState->setGlobalAmbientLightState(defaultMaterialLightColor);
		m_oglProgramState->setLightDisableState(lightIndex)	;
		m_oglProgramState->setLightAttenuationFactorsState(lightIndex, 0.0,	0.0, 0.0);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT void LcOglCEffect::cleanupTextureResources ()
{
	TmTextureInfoMap::iterator it;
	CTextureInfo *textureInfo =	NULL;

	for(it=m_textureMap.begin(); it	!= m_textureMap.end(); it++)
	{
		textureInfo	= (*it).second;

		// Proceed only	if the texture info	exists and has
		// a valid bitmap as well
		if (textureInfo	&& textureInfo->getBitmap())
		{
			m_space->unloadBitmap(textureInfo->getBitmap());
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT CTextureInfo*	LcOglCEffect::getTextureInfo (int index)
{
	TmTextureInfoMap::iterator it	= m_textureMap.begin();

	int	i=0;

	while(i	< index)
	{
		it++;
		i++;
	}

	return (*it).second;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT CTextureInfo*	LcOglCEffect::getTextureInfoByName (LcTmString&	name)
{
	return (m_textureMap[name]);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::bindTextures(LcOglCTexture *texture, ERenderTarget renderTarget, GLuint currentTextureId)
{
	TmTextureInfoMap::iterator it;
	CTextureInfo *textureInfo =	NULL;
	GLuint texId =	0;
	int textureWrapMode = 0;
	bool isCurrentTexture = false;
	bool isTexturePOT = false;
	GLuint externalTextureTarget = 0;

	for(it=m_textureMap.begin(); it	!= m_textureMap.end(); it++)
	{
		textureInfo	= (*it).second;

		// Proceed only	if the texture info	exists
		if (textureInfo)
		{
			// Set the texture unit
			glActiveTexture	(textureInfo->getUnit());

			// Check if	this sampler is mapped to an intrinsic texture
			if(textureInfo->getMapping() >= IFX_PRIMARY_TEXTURE && textureInfo->getMapping() <= IFX_SECONDARY_TEXTURE_2)
			{
				if (texture)
				{
					int index = textureInfo->getMapping() - IFX_PRIMARY_TEXTURE;
					texId =	texture->getTexture(index);
					textureWrapMode = texture->getWrapMode();
					isTexturePOT = texture->isPOT();
					isCurrentTexture = true;
					externalTextureTarget = texture->getExternalTextureTarget();
				}
				else
				{
					texId = currentTextureId;
					textureWrapMode = GL_CLAMP;
					isTexturePOT = false;
					isCurrentTexture = false;
				}
			}
			else
			{
				texId =	textureInfo->getId();
				textureWrapMode = textureInfo->getWrapMode();
				isTexturePOT = textureInfo->isPOT();
				isCurrentTexture = false;
			}

			// Select current texture with appropriate target
			if(externalTextureTarget == 0)
				glBindTexture(textureInfo->getType(), texId);
			else
				glBindTexture(externalTextureTarget, texId);

			externalTextureTarget = 0;

			// Select appropriate wrapping mode
			switch (renderTarget)
			{
				//---------------------
				case EBitmap:
				//---------------------
				{
					if (isCurrentTexture)
					{
						texture->setWrapMode (GL_CLAMP);
					}
					else if (textureWrapMode != GL_CLAMP)
					{
						LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
						LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

						textureInfo->setWrapMode (GL_CLAMP);
					}
				}
				break;

				//---------------------
				case EMesh:
				//---------------------
				{
					if (isTexturePOT)
					{
						if (isCurrentTexture)
						{
							texture->setWrapMode (GL_REPEAT);
						}
						else if (textureWrapMode != GL_REPEAT)
						{
							LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
							LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

							textureInfo->setWrapMode (GL_REPEAT);
						}
					}
					else // NPOT or PaddedPOT
					{
						if (isCurrentTexture)
						{
							texture->setWrapMode (GL_CLAMP);
						}
						else if (textureWrapMode != GL_CLAMP)
						{
							LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
							LC_OGL_IX(glTexParameter)(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

							textureInfo->setWrapMode (GL_CLAMP);
						}
					}
				}
				break;
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::unbindTextures(LcOglCTexture *texture)
{
	TmTextureInfoMap::iterator it;
	CTextureInfo *textureInfo =	NULL;
	GLuint externalTextureTarget = 0;

	if (texture)
	{
		externalTextureTarget = texture->getExternalTextureTarget();
	}

	for(it=m_textureMap.begin(); it	!= m_textureMap.end(); it++)
	{
		textureInfo	= (*it).second;

		// Proceed only	if the texture info	exists
		if (textureInfo)
		{
			// Set the texture unit
			glActiveTexture	(textureInfo->getUnit());

			if((externalTextureTarget != 0)
				&& (textureInfo->getMapping() == IFX_PRIMARY_TEXTURE))
			{
				glBindTexture(externalTextureTarget, 0);
				externalTextureTarget = 0;
			}
			else
			{
				glBindTexture(textureInfo->getType(), 0);
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::getEnumeratedLocation(int mapping)
{
	return m_locationArray[mapping];
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setNormalMatrix(float *	normMatrix)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location
		GLint iLocNormal = getEnumeratedLocation(IFX_NORMAL);
		float normalMatrix [9] = { 0 };

		// Create 3x3 normal matrix	by copying data	from the rotation part of the inverse transpose	matrix in column major form
		normalMatrix [0] = normMatrix [0];
		normalMatrix [3] = normMatrix [4];
		normalMatrix [6] = normMatrix [8];

		normalMatrix [1] = normMatrix [1];
		normalMatrix [4] = normMatrix [5];
		normalMatrix [7] = normMatrix [9];

		normalMatrix [2] = normMatrix [2];
		normalMatrix [5] = normMatrix [6];
		normalMatrix [8] = normMatrix [10];

		// Load	Normal matrix only if uniform location is valid
		if (iLocNormal >= 0)
		{
			float normal[16] = { 0.0 } ;
			m_oglProgramState->getNormalMatrixState	(normal);

			if (memcmp(normMatrix, normal, sizeof(float) * 16))
			{
				glUniformMatrix3fv (iLocNormal,	1, GL_FALSE, normalMatrix);

				// Update program state
				m_oglProgramState->setNormalMatrixState	(normMatrix);
			}
		}

		// Update global state.
		globalState->setNormalMatrixState(normMatrix);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setModelViewMatrix(float *mvMatrix)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location
		GLint iLocModelView	= getEnumeratedLocation(IFX_MODELVIEW);

		// Load	ModelView matrix only if uniform location is valid
		if (iLocModelView >= 0)
		{
			float mv[16] = { 0.0 } ;
			m_oglProgramState->getMVMatrixState	(mv);

			if (memcmp(mvMatrix, mv, sizeof(float) * 16))
			{
				glUniformMatrix4fv(iLocModelView, 1, GL_FALSE, mvMatrix);

				// Update program state
				m_oglProgramState->setMVMatrixState	(mvMatrix);
			}
		}

		// Update global state.
		globalState->setMVMatrixState(mvMatrix);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setProjectionMatrix(float *projMatrix)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location
		GLint iLocProjection = getEnumeratedLocation(IFX_PROJECTION);

		// Load	Projection matrix only if uniform index	is valid
		if (iLocProjection >= 0)
		{
			float proj[16] = { 0.0 } ;
			m_oglProgramState->getProjMatrixState (proj);

			if (memcmp(projMatrix, proj, sizeof(float) * 16))
			{
				glUniformMatrix4fv(iLocProjection, 1, GL_FALSE,	projMatrix);

				// Update program state
				m_oglProgramState->setProjMatrixState (projMatrix);
			}
		}

		// Update global state.
		globalState->setProjMatrixState(projMatrix);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setAmbientMaterial(float *ambient)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location
		GLint iLocMaterialAmbient =	getEnumeratedLocation(IFX_MATERIAL_AMBIENT);

		// Specify Material	Ambient	color only if uniform location is valid
		if (iLocMaterialAmbient	>= 0)
		{
			float amb[4] = { 0.0 } ;
			m_oglProgramState->getMaterialAmbientState (amb);

			if (memcmp(ambient,	amb, sizeof(float) * 4))
			{
				glUniform4fv (iLocMaterialAmbient,	(GLsizei) 1, (const	GLfloat*) ambient);

				// Update program state
				m_oglProgramState->setMaterialAmbientState (ambient);
			}
		}

		// Update global state.
		globalState->setMaterialAmbientState(ambient);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setDiffuseMaterial(float *diffuse)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location
		GLint iLocMaterialDiffuse =	getEnumeratedLocation(IFX_MATERIAL_DIFFUSE);

		// Specify Material	Diffuse	color only if uniform location is valid
		if (iLocMaterialDiffuse	>= 0)
		{
			float diff[4] =	{ 0.0 }	;
			m_oglProgramState->getMaterialDiffuseState (diff);

			if (memcmp(diffuse,	diff, sizeof(float)	* 4))
			{
				glUniform4fv (iLocMaterialDiffuse,	(GLsizei) 1, (const	GLfloat*) diffuse);

				// Update program state
				m_oglProgramState->setMaterialDiffuseState (diffuse);
			}
		}

		// Update global state.
		globalState->setMaterialDiffuseState(diffuse);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setSpecularMaterial(float *specular)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location
		GLint iLocMaterialSpecular = getEnumeratedLocation(IFX_MATERIAL_SPECULAR);

		// Specify Material	Specular color only	if uniform location	is valid
		if (iLocMaterialSpecular >=	0)
		{
			float spec[4] =	{ 0.0 }	;
			m_oglProgramState->getMaterialSpecularState	(spec);

			if (memcmp(specular, spec, sizeof(float) * 4))
			{
				glUniform4fv (iLocMaterialSpecular,	 (GLsizei) 1, (const GLfloat*) specular);

				// Update program state
				m_oglProgramState->setMaterialSpecularState	(specular);
			}
		}

		// Update global state.
		globalState->setMaterialSpecularState(specular);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setEmissionMaterial(float *emissive)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location
		GLint iLocMaterialEmissive = getEnumeratedLocation(IFX_MATERIAL_EMISSIVE);

		// Specify Material	Emissive color only	if uniform location	is valid
		if (iLocMaterialEmissive >=	0)
		{
			float emiss[4] = { 0.0 } ;
			m_oglProgramState->getMaterialEmissionState	(emiss);

			if (memcmp(emissive, emiss,	sizeof(float) *	4))
			{
				glUniform4fv (iLocMaterialEmissive,	 (GLsizei) 1, (const GLfloat*) emissive);

				// Update program state
				m_oglProgramState->setMaterialEmissionState	(emissive);
			}
		}

		// Update global state.
		globalState->setMaterialEmissionState(emissive);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setShininessMaterial(float shininess)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location
		GLint iLocMaterialSpecularExp =	getEnumeratedLocation(IFX_MATERIAL_EXPONENT);

		// Specify Material	Specular Exponent only if uniform location is valid
		if (iLocMaterialSpecularExp	>= 0)
		{
			float shin = 0.0;
			m_oglProgramState->getMaterialShininessState (&shin);

			if (shininess != shin)
			{
				glUniform1f	(iLocMaterialSpecularExp, (GLfloat)	shininess);

				// Update program state
				m_oglProgramState->setMaterialShininessState (shininess);
			}
		}

		// Update global state.
		globalState->setMaterialShininessState(shininess);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setAmbientLight(int	lightIndex,	float *ambientLight)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location	for	Light with index
		GLint iLocLightAmbient = getEnumeratedLocation(IFX_AMBIENT_LIGHT_0 + lightIndex);

		// Specify Ambient Light color only	if uniform location	is valid
		if (iLocLightAmbient >=	0)
		{
			float ambient[4] = { 0.0 } ;
			m_oglProgramState->getLightAmbientState	(lightIndex, ambient);

			if (memcmp(ambientLight, ambient, sizeof(float)	* 4))
			{
				glUniform4fv(iLocLightAmbient, (GLsizei) 1,	(const GLfloat*) ambientLight);

				// Update program state
				m_oglProgramState->setLightAmbientState	(lightIndex, ambientLight);
			}
		}

		// Update global state.
		globalState->setLightAmbientState(lightIndex, ambientLight);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setDiffuseLight(int	lightIndex,	float *diffuse)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location	for	Light with index
		GLint iLocLightDiffuse = getEnumeratedLocation(IFX_DIFFUSE_LIGHT_0 + lightIndex);

		// Specify Diffuse Light color only	if uniform location	is valid
		if (iLocLightDiffuse >=	0)
		{
			float diff[4] =	{ 0.0 }	;
			m_oglProgramState->getLightDiffuseState	(lightIndex, diff);

			if (memcmp(diffuse,	diff, sizeof(float)	* 4))
			{
				glUniform4fv(iLocLightDiffuse, (GLsizei) 1,	(const GLfloat*) diffuse);

				// Update program state
				m_oglProgramState->setLightDiffuseState	(lightIndex, diffuse);
			}
		}

		// Update global state.
		globalState->setLightDiffuseState(lightIndex, diffuse);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setSpecularLight(int lightIndex, float *specular)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location	for	Light with index
		GLint iLocLightSpecular = getEnumeratedLocation(IFX_SPECULAR_LIGHT_0 + lightIndex);

		// Specify Specular	Light color	only if	uniform	location is	valid
		if (iLocLightSpecular >= 0)
		{
			float spec[4] =	{ 0.0 }	;
			m_oglProgramState->getLightSpecularState (lightIndex, spec);

			if (memcmp(specular, spec, sizeof(float) * 4))
			{
				glUniform4fv(iLocLightSpecular,	(GLsizei) 1, (const	GLfloat*) specular);

				// Update program state
				m_oglProgramState->setLightSpecularState (lightIndex, specular);
			}
		}

		// Update global state.
		globalState->setLightSpecularState(lightIndex, specular);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setPositionLight(int lightIndex, float *position)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location	for	Light with index
		GLint iLocLightPosition = getEnumeratedLocation(IFX_LIGHT_POSITION_0 + lightIndex);

		// Specify Light Position only if uniform location is valid
		if (iLocLightPosition >= 0)
		{
			float pos[4] = { 0.0 } ;
			m_oglProgramState->getLightPositionState (lightIndex, pos);

			if (memcmp(position, pos, sizeof(float)	* 4))
			{
				glUniform4fv(iLocLightPosition,	(GLsizei) 1, (const	GLfloat*) position);

				// Update program state
				m_oglProgramState->setLightPositionState (lightIndex, position);
			}
		}

		// Update global state.
		globalState->setLightPositionState(lightIndex, position);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setGlobalAmbientLight(float	*global_ambient)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();
	if (this->m_programObject != 0)
	{
		// Get uniform location	for	Light with index
		GLint iLocGlobalAmbient	= getEnumeratedLocation(IFX_GLOBAL_AMBIENT);

		// Specify Global Ambient color	only if	uniform	location is	valid
		if (iLocGlobalAmbient >= 0)
		{
			float gamb[4] =	{ 0.0 }	;
			m_oglProgramState->getGlobalAmbientLightState (gamb);

			if (memcmp(global_ambient, gamb, sizeof(float) * 4))
			{
				glUniform4fv (iLocGlobalAmbient,  (GLsizei)	1, (const GLfloat*)	global_ambient);

				// Update program state
				m_oglProgramState->setGlobalAmbientLightState (global_ambient);
			}
		}

		// Update global state.
		globalState->setGlobalAmbientLightState(global_ambient);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
	Enable local light at index	= lightIndex.
*/
LC_EXPORT int LcOglCEffect::enableLight(int	lightIndex)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location	for	Light with index
		GLint iLocLightEnable = getEnumeratedLocation(IFX_LIGHT_STATE_0 + lightIndex);

		// Set light enable	uniform	provided location is valid
		if (iLocLightEnable	>= 0)
		{
			int	lightStatus	= m_oglProgramState->getLightStatus	(lightIndex);

			if (lightStatus	!= GL_TRUE)
			{
				glUniform1i	(iLocLightEnable, GL_TRUE);

				// Update program state
				m_oglProgramState->setLightEnableState(lightIndex);
			}
		}

		// Update global state.
		globalState->setLightEnableState(lightIndex);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
	Disable	local light	at index = lightIndex.
*/
LC_EXPORT int LcOglCEffect::disableLight(int lightIndex)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location	for	Light with index
		GLint iLocLightDisable = getEnumeratedLocation(IFX_LIGHT_STATE_0 + lightIndex);

		// Set light disable uniform provided location is valid
		if (iLocLightDisable >=	0)
		{
			int	lightStatus	= m_oglProgramState->getLightStatus	(lightIndex);

			if (lightStatus	!= GL_FALSE)
			{
				glUniform1i	(iLocLightDisable, GL_FALSE);

				// Update program state
				m_oglProgramState->setLightDisableState(lightIndex);
			}
		}

		// Update global state.
		globalState->setLightDisableState(lightIndex);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::setLightAttenuationFactors(int lightIndex, float constant, float linear, float quadratic)
{
	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();

	if (this->m_programObject != 0)
	{
		// Get uniform location
		GLint iLocLightAttFactors = getEnumeratedLocation(IFX_ATTENUATION_FACTORS_0 + lightIndex);

		LcTScalar light_attenuation_factors	[3];
		light_attenuation_factors [0] =	constant;
		light_attenuation_factors [1] =	linear;
		light_attenuation_factors [2] =	quadratic;

		// Set light attenuation factors provided uniform location is valid
		if (iLocLightAttFactors	>= 0)
		{
			float constant_, linear_, quadratic_ = 0.0;
			m_oglProgramState->getLightAttenuationFactorsState (lightIndex,	&constant_,	&linear_, &quadratic_);

			if ( (constant != constant_) ||
				 (linear !=	linear_) ||
				 (quadratic	!= quadratic_) )
			{
				glUniform3fv (iLocLightAttFactors, (GLsizei) 1,	(const GLfloat*) light_attenuation_factors);

				// Update program state
				m_oglProgramState->setLightAttenuationFactorsState (lightIndex,	constant, linear, quadratic);
			}
		}

		// Update global state.
		globalState->setLightAttenuationFactorsState(lightIndex, constant, linear, quadratic);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::makeCurrent()
{
	if(m_programObject != 0)
	{
		glUseProgram(m_programObject);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::getUniformLocByName(const LcTmString name)
{
	int	loc	= -1;
	LcOglCSLType *slType = m_uniformMap[name];

	if (slType)
		loc	= slType->getLocationIndex();

	return (loc);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcOglCSLType*	LcOglCEffect::getSLTypeByMapping(TEngineEffectVariable effectVariable)
{
	return m_slTypeUniArray[effectVariable];
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::setTextureInfo(LcTmString name, GLuint id, bool isPOT)
{
	CTextureInfo *textureInfo =	m_textureMap[name];

	if (textureInfo)
	{
		textureInfo->setId(id);
		textureInfo->setPOT(isPOT);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::setVShaderFile(LcTmString&	vFile)
{
	m_vertexShaderFile = vFile;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaString LcOglCEffect::getVShaderFile(void)
{
	return m_vertexShaderFile;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::setFShaderFile(LcTmString&	fFile)
{
	m_fragShaderFile = fFile;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaString LcOglCEffect::getFShaderFile(void)
{
	return m_fragShaderFile;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::configureVertexAttribArray	(void)
{
	int	index =	0;

	index += (getEnumeratedLocation(IFX_VERTEX_COORDS) >= 0);
	index += (getEnumeratedLocation(IFX_NORMAL_COORDS) >= 0);
	index += (getEnumeratedLocation(IFX_TEXTURE_COORDS) >= 0);
	index += (getEnumeratedLocation(IFX_TANGENT_COORDS) >= 0);
	index += (getEnumeratedLocation(IFX_BINORMAL_COORDS) >= 0);
	index += (getEnumeratedLocation(IFX_VERTEX_POSX_PLANE) >= 0);
	index += (getEnumeratedLocation(IFX_VERTEX_POSY_PLANE) >= 0);

	for	(int i=0; i	<5;	i++)
	{
		if (i <	index)
		{
			glEnableVertexAttribArray (i);
		}
		else
		{
			glDisableVertexAttribArray (i);
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Test whether this is a valid uniform (covers mapped	+ unmapped ).
*/
LC_EXPORT bool LcOglCEffect::isValidUniform	(LcOglCSLType *slType)
{
	bool isValid = false;

	// Check from within the uniform map that has all uniforms
	// defined for this	effect
	if (slType && m_uniformMap.size())
	{
		// Find	the	slType with	this name within the configurable uniforms
		LcTaString lookupName =	slType->getSLTypeName();
		LcTaString lookupType =	slType->getSLTypeIdentifier();

		// If the entry	with this name exists in the uniforms map
		if (m_uniformMap.find(lookupName) != m_uniformMap.end())
		{
			LcOglCSLType *slTypeLocal =	m_uniformMap [lookupName];

			// Make	sure that the type of the SL-Type also matches with	the	type defined in	.effect
			if (slTypeLocal)
			{
				// Set the validity	flag
				isValid	= slTypeLocal->getSLTypeIdentifier().compareNoCase(lookupType) == 0;
			}
		}
	}

	return isValid;
}

/*-------------------------------------------------------------------------*//**
	Test whether this is a valid effect	uniform	(covers	unmapped only).
*/
LC_EXPORT bool LcOglCEffect::isValidConfigUniform (LcOglCSLType	*slType)
{
	bool isValid = false;

	if (slType && m_configUniformMap.size())
	{
		// Find	the	slType with	this name within the configurable uniforms
		LcTaString lookupName =	slType->getSLTypeName();
		LcTaString lookupType =	slType->getSLTypeIdentifier();

		// If the entry	with this name exists in the config	uniforms map
		if (m_configUniformMap.find(lookupName)	!= m_configUniformMap.end())
		{
			LcOglCSLType *slTypeLocal =	m_configUniformMap [lookupName];

			// Make	sure that the type of the SLtype also matches with the type	defined	in .effect
			if (slTypeLocal)
			{
				// Set the validity	flag
				isValid	= slTypeLocal->getSLTypeIdentifier().compareNoCase(lookupType) == 0;
			}
		}
	}

	return isValid;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::passUniformInfoToOGL (LcOglCSLType *slType)
{
	int	error =	GL_NO_ERROR;

	if (!isValidUniform(slType))
	{
		error =	-1;
		return error;
	}

	LcOglCContext *ctx = m_space->getOglContext();
	LcOglCGlobalState* globalState = ctx->getGlobalState();

	int	loc	= slType->getLocationIndex();

	if (loc	< 0)
	{
		// The uniform location	is not valid, so return	from here without even
		// checking	the	data type of the uniform as	value of a uniform with
		// invalid locatio index cannot	be set
		error =	-1;
		return error;
	}

	ShaderDataType dataType = globalState->getShaderDataType(slType->getSLTypeIdentifier());

	switch(dataType)
	{
		//---------------------------------
		// int
		//---------------------------------
		case ELcInt:
		{
			int	iValue = ((LcOglCSLTypeScalar<int> *)slType)->getValue();

			// Get the local SL	type
			LcOglCSLType *slTypeLocal =	m_uniformMap [slType->getSLTypeName()];

			// Get bounds
			int	iMinValue =	((LcOglCSLTypeScalar<int> *)slTypeLocal)->getMinValue();
			int	iMaxValue =	((LcOglCSLTypeScalar<int> *)slTypeLocal)->getMaxValue();

			// Pass	the	value to OGL2 pipeline
			glUniform1i	(loc, ctx->clampValue(iValue, iMinValue, iMaxValue));
		}
		break;

		//---------------------------------
		// bool
		//---------------------------------
		case ELcBool:
		{
			bool bValue	= ((LcOglCSLTypeScalar<bool> *)slType)->getValue();

			// No bound	checking required in case of "bool", just pass the value to	OGL2 pipeline
			glUniform1i	(loc, bValue);
		}
		break;

		//---------------------------------
		// float
		//---------------------------------
		case ELcFloat:
		{
			float fValue = ((LcOglCSLTypeScalar<float> *)slType)->getValue();

			// Get the local SL	type
			LcOglCSLType *slTypeLocal =	m_uniformMap [slType->getSLTypeName()];

			// Get bounds
			float fMinValue	= ((LcOglCSLTypeScalar<float> *)slTypeLocal)->getMinValue();
			float fMaxValue	= ((LcOglCSLTypeScalar<float> *)slTypeLocal)->getMaxValue();

			// Pass	the	value to OGL2 pipeline
			glUniform1f	(loc, ctx->clampValue(fValue, fMinValue, fMaxValue));
		}
		break;

		//---------------------------------
		// ivec2
		//---------------------------------
		case ELcIVec2:
		{
			int	iValues	[2]	= {0};
			int	iMinValues [2] = {0};
			int	iMaxValues [2] = {0};

			// Get the local SL	type
			LcOglCSLType *slTypeLocal =	m_uniformMap[slType->getSLTypeName()];

			// Get current value
			((LcOglCSLTypeVector<int> *)slType)->getValue(iValues);

			// Get bounds
			((LcOglCSLTypeVector<int> *)slTypeLocal)->getMinValue(iMinValues);
			((LcOglCSLTypeVector<int> *)slTypeLocal)->getMaxValue(iMaxValues);

			// Pass	the	value to OGL2 pipeline
			glUniform2i	(loc,
						 ctx->clampValue(iValues[0], iMinValues[0],	iMaxValues[0]),
						 ctx->clampValue(iValues[1], iMinValues[1],	iMaxValues[1]));
		}
		break;

		//---------------------------------
		// ivec3
		//---------------------------------
		case ELcIVec3:
		{
			int	iValues	[3]	= {0};
			int	iMinValues [3] = {0};
			int	iMaxValues [3] = {0};

			// Get the local SL	type
			LcOglCSLType *slTypeLocal =	m_uniformMap [slType->getSLTypeName()];

			// Get current value
			((LcOglCSLTypeVector<int> *)slType)->getValue(iValues);

			// Get bounds
			((LcOglCSLTypeVector<int> *)slTypeLocal)->getMinValue(iMinValues);
			((LcOglCSLTypeVector<int> *)slTypeLocal)->getMaxValue(iMaxValues);

			// Pass	the	value to OGL2 pipeline
			glUniform3i	(loc,
						 ctx->clampValue(iValues[0], iMinValues[0],	iMaxValues[0]),
						 ctx->clampValue(iValues[1], iMinValues[1],	iMaxValues[1]),
						 ctx->clampValue(iValues[2], iMinValues[2],	iMaxValues[2]));
		}
		break;

		//---------------------------------
		// ivec4
		//---------------------------------
		case ELcIVec4:
		{
			int	iValues	[4]	= {0};
			int	iMinValues [4] = {0};
			int	iMaxValues [4] = {0};

			// Get the local SL	type
			LcOglCSLType *slTypeLocal =	m_uniformMap [slType->getSLTypeName()];

			// Get current value
			((LcOglCSLTypeVector<int> *)slType)->getValue(iValues);

			// Get bounds
			((LcOglCSLTypeVector<int> *)slTypeLocal)->getMinValue(iMinValues);
			((LcOglCSLTypeVector<int> *)slTypeLocal)->getMaxValue(iMaxValues);

			// Pass	the	value to OGL2 pipeline
			glUniform4i	(loc,
						 ctx->clampValue(iValues[0], iMinValues[0],	iMaxValues[0]),
						 ctx->clampValue(iValues[1], iMinValues[1],	iMaxValues[1]),
						 ctx->clampValue(iValues[2], iMinValues[2],	iMaxValues[2]),
						 ctx->clampValue(iValues[3], iMinValues[3],	iMaxValues[3]));
		}
		break;

		//---------------------------------
		// bvec2
		//---------------------------------
		case ELcBVec2:
		{
			bool bValues[2]	= {false};

			((LcOglCSLTypeVector<bool> *)slType)->getValue(bValues);

			// No bound	checking required in case of "bool", just pass the value to	OGL2 pipeline
			glUniform2i	(loc, bValues[0], bValues[1]);
		}
		break;

		//---------------------------------
		// bvec3
		//---------------------------------
		case ELcBVec3:
		{
			bool bValues[3]	= {false};

			((LcOglCSLTypeVector<bool> *)slType)->getValue(bValues);

			// No bound	checking required in case of "bool", just pass the value to	OGL2 pipeline
			glUniform3i	(loc, bValues[0], bValues[1], bValues[2]);
		}
		break;

		//---------------------------------
		// bvec4
		//---------------------------------
		case ELcBVec4:
		{
			bool bValues[4]	= {false};

			((LcOglCSLTypeVector<bool> *)slType)->getValue(bValues);

			// No bound	checking required in case of "bool", just pass the value to	OGL2 pipeline
			glUniform4i	(loc, bValues[0], bValues[1], bValues[2], bValues[3]);
		}
		break;

		//---------------------------------
		// vec2
		//---------------------------------
		case ELcVec2:
		{
			float fValues [2] =	{0};
			float fMinValues [2] = {0};
			float fMaxValues [2] = {0};

			// Get the local SL	type
			LcOglCSLType *slTypeLocal =	m_uniformMap [slType->getSLTypeName()];

			// Get current value
			((LcOglCSLTypeVector<float>	*)slType)->getValue(fValues);

			// Get bounds
			((LcOglCSLTypeVector<float>	*)slTypeLocal)->getMinValue(fMinValues);
			((LcOglCSLTypeVector<float>	*)slTypeLocal)->getMaxValue(fMaxValues);

			// Pass	the	value to OGL2 pipeline
			glUniform2f	(loc,
						 ctx->clampValue(fValues[0], fMinValues[0],	fMaxValues[0]),
						 ctx->clampValue(fValues[1], fMinValues[1],	fMaxValues[1]) );
		}
		break;

		//---------------------------------
		// vec3
		//---------------------------------
		case ELcVec3:
		{
			float fValues [3] =	{0};
			float fMinValues [3] = {0};
			float fMaxValues [3] = {0};

			// Get the local SL	type
			LcOglCSLType *slTypeLocal =	m_uniformMap [slType->getSLTypeName()];

			// Get current value
			((LcOglCSLTypeVector<float>	*)slType)->getValue(fValues);

			// Get bounds
			((LcOglCSLTypeVector<float>	*)slTypeLocal)->getMinValue(fMinValues);
			((LcOglCSLTypeVector<float>	*)slTypeLocal)->getMaxValue(fMaxValues);

			// Pass	the	value to OGL2 pipeline
			glUniform3f	(loc,
						 ctx->clampValue(fValues[0], fMinValues[0],	fMaxValues[0]),
						 ctx->clampValue(fValues[1], fMinValues[1],	fMaxValues[1]),
						 ctx->clampValue(fValues[2], fMinValues[2],	fMaxValues[2]) );
		}
		break;

		//---------------------------------
		// vec4
		//---------------------------------
		case ELcVec4:
		{
			float fValues [4] =	{0};
			float fMinValues [4] = {0};
			float fMaxValues [4] = {0};

			// Get the local SL	type
			LcOglCSLType *slTypeLocal =	m_uniformMap [slType->getSLTypeName()];

			// Get current value
			((LcOglCSLTypeVector<float>	*)slType)->getValue(fValues);

			// Get bounds
			((LcOglCSLTypeVector<float>	*)slTypeLocal)->getMinValue(fMinValues);
			((LcOglCSLTypeVector<float>	*)slTypeLocal)->getMaxValue(fMaxValues);

			// Pass	the	value to OGL2 pipeline
			glUniform4f	(loc,
						 ctx->clampValue(fValues[0], fMinValues[0],	fMaxValues[0]),
						 ctx->clampValue(fValues[1], fMinValues[1],	fMaxValues[1]),
						 ctx->clampValue(fValues[2], fMinValues[2],	fMaxValues[2]),
						 ctx->clampValue(fValues[3], fMinValues[3],	fMaxValues[3]) );
		}
		break;

		//---------------------------------
		// Unsupported types
		//---------------------------------
		case ELcMat2:
		case ELcMat3:
		case ELcMat4:
		case ELcSampler2D:
		case ELcSamplerCube:
		default:
			// Do nothing for these	data types ...
		break;
	}

	// Get the error
	if (error != GL_NO_ERROR)
	{
		error =	-1;
	}

	return (error);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcOglCEffect::restoreConfigurableUniformsToDefaults ()
{
	int	status = 0;

	LcTaString localName;
	LcOglCSLType *slType;

	TmSLTypeOwnerMap::iterator it;

	for(it=m_configUniformMap.begin(); it != m_configUniformMap.end(); it++)
	{
		localName =	(*it).first;
		slType = (*it).second;

		passUniformInfoToOGL (slType);
	}

	return status;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::populateMapWithConfigUniforms (TmSLTypeOwnerMap*	map, bool bInterpolatable)
{
	LcTaString name;
	LcOglCSLType *slType = NULL;

	TmSLTypeOwnerMap::iterator it;

	// No need to proceed if map does not exist
	if (!map)
	{
		return;
	}

	for(it=m_configUniformMap.begin(); it != m_configUniformMap.end(); it++)
	{
		name = (*it).first;
		slType = (*it).second;

		if (slType && !name.isEmpty())
		{
			// Add the entry to	our	input map so that the uniforms are made	available
			// for layout tweening and static animations, even if no effect/configiurable uniform
			// is specified	by the user
			// Sampler, boolean and int uniforms will not be added because they are not interpolatable.
			if (slType->isInterpolatable() == bInterpolatable)
			{
				cloneSLType(slType,	name, map);
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::populateMapWithMissingConfigUniforms (TmSLTypeOwnerMap* map,	bool bInterpolatable)
{
	LcTaString uniformName;
	LcOglCSLType *slType = NULL;

	TmSLTypeOwnerMap::iterator it;

	// No need to proceed if map does not exist
	if (!map)
	{
		return;
	}

	for(it=m_configUniformMap.begin(); it != m_configUniformMap.end(); it++)
	{
		uniformName	= (*it).first;
		slType = (*it).second;

		if (slType->isInterpolatable() != bInterpolatable)
			continue;

		// Locate the entry	with this name
		if ( (*map).find(uniformName) != (*map).end() )
		{
			// Test	whether	the	slType is valid
			if (!isValidConfigUniform((*map)[uniformName]))
			{
				// First erase this	invalid	entry and then add a new one taking	from config	uniforms map
				map->erase(it);
				cloneSLType(slType,	uniformName, map);
			}
		}
		else
		{
			// Uniform does	not	even exist in the incoming map,	so add it
			cloneSLType(slType,	uniformName, map);
		}
	}

	// Now finally traverse	the	updated	static placement
	// to erase	those additional entries that are not part
	// of this effect in any way
	// CAUTION:	DO not increment the iterator in the for loop, as this creates issues
	// on certain map implementations like that	of VisualStudio. The safe approach is
	// to increment	the	iterator down the block, depending on whether we perform an
	// erase operation or not; to keep the iterator	state consistent under all
	// conditions
	for(it=(*map).begin(); it != (*map).end();)
	{
		uniformName	= (*it).first;

		// Locate that entry in	config uniforms	map, erase it if not found
		if (m_configUniformMap.find(uniformName) ==	m_configUniformMap.end())
		{
			// Erase this invalid additional unnecessary entry
			(*map).erase (it++);
		}
		else
		{
			// We did not perform the erase, simply	move the iterator forward
			it++;
		}
	}
}


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::setUsage(LcTmString& usage)
{
	int	usageIndex = -1;

	if ( usage.compareNoCase("Background") == 0)
		usageIndex = BACKGROUND_EFFECT_INDEX;
	else if	( usage.compareNoCase("AlphaLight00") == 0)
		usageIndex = ALPHALIGHT00_EFFECT_INDEX;
	else if	( usage.compareNoCase("AlphaLight01") == 0)
		usageIndex = ALPHALIGHT01_EFFECT_INDEX;
	else if	( usage.compareNoCase("AlphaLight02") == 0)
		usageIndex = ALPHALIGHT02_EFFECT_INDEX;
	else if	( usage.compareNoCase("Light00") ==	0)
		usageIndex = LIGHT00_EFFECT_INDEX;
	else if	( usage.compareNoCase("Light01") ==	0)
		usageIndex = LIGHT01_EFFECT_INDEX;
	else if	( usage.compareNoCase("Light02") ==	0)
		usageIndex = LIGHT02_EFFECT_INDEX;
	else if	( usage.compareNoCase("TexLight00")	== 0)
		usageIndex = TEXLIGHT00_EFFECT_INDEX;
	else if	( usage.compareNoCase("TexLight01")	== 0)
		usageIndex = TEXLIGHT01_EFFECT_INDEX;
	else if	( usage.compareNoCase("TexLight02")	== 0)
		usageIndex = TEXLIGHT02_EFFECT_INDEX;

	else if	( usage.compareNoCase("HighAlphaLight00") == 0)
		usageIndex = ALPHALIGHT00_HIGH_EFFECT_INDEX;
	else if	( usage.compareNoCase("HighAlphaLight01") == 0)
		usageIndex = ALPHALIGHT01_HIGH_EFFECT_INDEX;
	else if	( usage.compareNoCase("HighAlphaLight02") == 0)
		usageIndex = ALPHALIGHT02_HIGH_EFFECT_INDEX;
	else if	( usage.compareNoCase("HighLight00") ==	0)
		usageIndex = LIGHT00_HIGH_EFFECT_INDEX;
	else if	( usage.compareNoCase("HighLight01") ==	0)
		usageIndex = LIGHT01_HIGH_EFFECT_INDEX;
	else if	( usage.compareNoCase("HighLight02") ==	0)
		usageIndex = LIGHT02_HIGH_EFFECT_INDEX;
	else if	( usage.compareNoCase("HighTexLight00")	== 0)
		usageIndex = TEXLIGHT00_HIGH_EFFECT_INDEX;
	else if	( usage.compareNoCase("HighTexLight01")	== 0)
		usageIndex = TEXLIGHT01_HIGH_EFFECT_INDEX;
	else if	( usage.compareNoCase("HighTexLight02")	== 0)
		usageIndex = TEXLIGHT02_HIGH_EFFECT_INDEX;

	if ( usage.compareNoCase("Light") == 0)
		usageIndex = LIGHT00_EFFECT_INDEX;
	else if	( usage.compareNoCase("Tex") ==	0)
		usageIndex = BACKGROUND_EFFECT_INDEX;
	else if	( usage.compareNoCase("TexLight") == 0)
		usageIndex = TEXLIGHT00_EFFECT_INDEX;
	else if	( usage.compareNoCase("AlphaLight")	== 0)
		usageIndex = ALPHALIGHT00_EFFECT_INDEX;

	m_usage = usageIndex;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCEffect::cloneSLType (LcOglCSLType*	slType,	LcTmString name, TmSLTypeOwnerMap* map)
{
	LcTaOwner<LcOglCSLType>	slTypeF;

	if (!slType)
	{
		return ;
	}

	//----------------------------------------
	// float
	//----------------------------------------
	if (slType->getSLTypeIdentifier().compareNoCase("float") ==	0)
	{
		float value	= ((LcOglCSLTypeScalar<float> *)slType)->getValue();

		LcTaOwner< LcOglCSLTypeScalar<float> > uniform = LcOglCSLTypeScalar<float>::create();

		uniform->setSLType(ELcOglSLTypeScalar);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(1);
		uniform->setValue(&value);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// bool
	//----------------------------------------
	else
	if (slType->getSLTypeIdentifier().compareNoCase("bool")	== 0)
	{
		bool value = ((LcOglCSLTypeScalar<bool>	*)slType)->getValue();

		LcTaOwner< LcOglCSLTypeScalar<bool>	> uniform =	LcOglCSLTypeScalar<bool>::create();

		uniform->setSLType(ELcOglSLTypeScalar);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(1);
		uniform->setValue(&value);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// int
	//----------------------------------------
	else
	if (slType->getSLTypeIdentifier().compareNoCase("int") == 0)
	{
		int	value =	((LcOglCSLTypeScalar<int> *)slType)->getValue();

		LcTaOwner< LcOglCSLTypeScalar<int> > uniform = LcOglCSLTypeScalar<int>::create();

		uniform->setSLType(ELcOglSLTypeScalar);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(1);
		uniform->setValue(&value);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// sampler2D
	//----------------------------------------
	else
	if (slType->getSLTypeIdentifier().compareNoCase("sampler2D") ==	0)
	{
		LcTaString value = ((LcOglCSLTypeScalar<LcTmString>	*)slType)->getValue();

		LcTaOwner< LcOglCSLTypeScalar<LcTmString> >	uniform	= LcOglCSLTypeScalar<LcTmString>::create();

		uniform->setSLType(ELcOglSLTypeScalar);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(1);
		uniform->setValue(&value);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// vec2
	//----------------------------------------
	else if	(slType->getSLTypeIdentifier().compareNoCase("vec2") ==	0)
	{
		float values[2];

		((LcOglCSLTypeVector<float>	*)slType)->getValue(values);

		LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(2);

		uniform->setSLType(ELcOglSLTypeVector);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(2);
		uniform->setValue(values);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// bvec2
	//----------------------------------------
	else if	(slType->getSLTypeIdentifier().compareNoCase("bvec2") == 0)
	{
		bool values[2];

		((LcOglCSLTypeVector<bool> *)slType)->getValue(values);

		LcTaOwner< LcOglCSLTypeVector<bool>	> uniform =	LcOglCSLTypeVector<bool>::create(2);

		uniform->setSLType(ELcOglSLTypeVector);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(2);
		uniform->setValue(values);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// ivec2
	//----------------------------------------
	else if	(slType->getSLTypeIdentifier().compareNoCase("ivec2") == 0)
	{
		int	values[2];

		((LcOglCSLTypeVector<int> *)slType)->getValue(values);

		LcTaOwner< LcOglCSLTypeVector<int> > uniform = LcOglCSLTypeVector<int>::create(2);

		uniform->setSLType(ELcOglSLTypeVector);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(2);
		uniform->setValue(values);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// vec3
	//----------------------------------------
	else if	(slType->getSLTypeIdentifier().compareNoCase("vec3") ==	0)
	{
		float values[3];

		((LcOglCSLTypeVector<float>	*)slType)->getValue(values);

		LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(3);

		uniform->setSLType(ELcOglSLTypeVector);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(3);
		uniform->setValue(values);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// bvec3
	//----------------------------------------
	else if	(slType->getSLTypeIdentifier().compareNoCase("bvec3") == 0)
	{
		bool values[3];

		((LcOglCSLTypeVector<bool> *)slType)->getValue(values);

		LcTaOwner< LcOglCSLTypeVector<bool>	> uniform =	LcOglCSLTypeVector<bool>::create(3);

		uniform->setSLType(ELcOglSLTypeVector);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(3);
		uniform->setValue(values);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// ivec3
	//----------------------------------------
	else if	(slType->getSLTypeIdentifier().compareNoCase("ivec3") == 0)
	{
		int	values[3];

		((LcOglCSLTypeVector<int> *)slType)->getValue(values);

		LcTaOwner< LcOglCSLTypeVector<int> > uniform = LcOglCSLTypeVector<int>::create(3);

		uniform->setSLType(ELcOglSLTypeVector);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(3);
		uniform->setValue(values);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// vec4
	//----------------------------------------
	else if	(slType->getSLTypeIdentifier().compareNoCase("vec4") ==	0)
	{
		float values[4];

		((LcOglCSLTypeVector<float>	*)slType)->getValue(values);

		LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(4);

		uniform->setSLType(ELcOglSLTypeVector);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(4);
		uniform->setValue(values);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// bvec4
	//----------------------------------------
	else if	(slType->getSLTypeIdentifier().compareNoCase("bvec4") == 0)
	{
		bool values[4];

		((LcOglCSLTypeVector<bool> *)slType)->getValue(values);

		LcTaOwner< LcOglCSLTypeVector<bool>	> uniform =	LcOglCSLTypeVector<bool>::create(4);

		uniform->setSLType(ELcOglSLTypeVector);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(4);
		uniform->setValue(values);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//----------------------------------------
	// ivec4
	//----------------------------------------
	else if	(slType->getSLTypeIdentifier().compareNoCase("ivec4") == 0)
	{
		int	values[4];

		((LcOglCSLTypeVector<int> *)slType)->getValue(values);

		LcTaOwner< LcOglCSLTypeVector<int> > uniform = LcOglCSLTypeVector<int>::create(4);

		uniform->setSLType(ELcOglSLTypeVector);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(4);
		uniform->setValue(values);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	//-------------------------------------//////////////
	// samplerCube
	//-------------------------------------/////////////
	else if	(slType->getSLTypeIdentifier().compareNoCase("samplerCube")	== 0)
	{
		LcTaString values[6];

		((LcOglCSLTypeVector<LcTmString> *)slType)->getValue(values);

		LcTaOwner< LcOglCSLTypeVector<LcTmString> >	uniform	= LcOglCSLTypeVector<LcTmString>::create(6);

		uniform->setSLType(ELcOglSLTypeVector);
		uniform->setSLTypeName(slType->getSLTypeName());
		uniform->setDimension(6);
		uniform->setValue(values);

		LcTaString type	= ((LcOglCSLType*)slType)->getSLTypeIdentifier();
		uniform->setSLTypeIdentifier(type);

		slTypeF	= uniform;
	}

	if (slTypeF)
	{
		map->add_element(name, slTypeF);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<CTextureInfo> CTextureInfo::create()
{
	LcTaOwner<CTextureInfo>	ref;
	ref.set(new	CTextureInfo);
	//ref->construct();
	return ref;
}

#if	defined(LC_USE_DESKTOP_OGL)

#include <iostream>
#include <string.h>
using namespace	std;

LC_EXPORT LcTaString LcOglCEffect::replacePrecisionFromShaderSource (char *shaderSourceActual)
{
	string str (shaderSourceActual);

	int	pos	= -1;
	int	length = 0;
	int	count =	0;

	// 1 - Look	for	"precision"
	length = str.length();
	pos	= str.find ("precision");

	while (	(pos < length) && (pos >= 0) )
	{
		count++;

		str.replace	(pos,					  // start position	in str
					 strlen("precision"),	  // how many characters
					 "//precision");		  // source	for	replacement

		length = str.length();

		pos	= str.find ("precision", pos+3);
	}

	// 2 - Look	for	"mediump"
	length = str.length();
	pos	= str.find ("mediump");

	while (	(pos < length) && (pos >= 0) )
	{
		count++;

		str.replace	(pos,						// start position in str
					 strlen("mediump") + 1,		// how many	characters
					 "");						// source for replacement

		length = str.length();

		pos	= str.find ("mediump", pos+1);
	}

	// 3 - Look	for	"lowp"
	pos	= str.find ("lowp");
	while (	(pos < length) && (pos >= 0) )
	{
		count++;

		str.replace	(pos,					 //	start position in str
					 strlen("lowp")	+ 1,	 //	how	many characters
					 "");					 //	source for replacement

		length = str.length();

		pos	= str.find ("lowp");
	}

	// 4 - Look	for	"highp"
	pos	= str.find ("highp");
	while (	(pos < length) && (pos >= 0) )
	{
		count++;

		str.replace	(pos,					  // start position	in str
					 strlen("highp") + 1,	  // how many characters
					 "");					  // source	for	replacement

		length = str.length();

		pos	= str.find ("highp", pos+1);
	}

	return LcTaString(str.c_str());
}

#endif /* defined(LC_USE_DESKTOP_OGL) */

#endif	/* #if defined(LC_PLAT_OGL_20) */
