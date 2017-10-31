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


#if defined(LC_PLAT_OGL) && defined(LC_USE_LIGHTS)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcOglCLight> LcOglCLight::create(LcCSpace* sp, int lightIndex)
{
	LcTaOwner<LcOglCLight> ref;
	ref.set(new LcOglCLight(sp));
	ref->construct(sp, lightIndex);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCLight::construct(LcCSpace* sp, int lightIndex)
{	
	m_space = sp; 

#if defined (LC_PLAT_OGL_20)
	m_glLight = lightIndex;
#else
	m_glLight = GL_LIGHT0 + lightIndex;
#endif
	m_lightType = LcwCLight::EBulb;

	// It is initially disabled.
	setEnabled(false);
	m_previouslyDisabled = false;
	m_temporaryDisabled = false;

#if defined (LC_PLAT_OGL_20)
		
	LcOglCEffect* currentEffect = m_space->getOglContext()->getCurrentEffect();

	if (currentEffect != NULL)
	{
		currentEffect->disableLight(m_glLight);
	}
	
#else	// for OpenGL 1.1
	LcOglTScalar initialPosition[]	= { 0.0f, 0.0f, 0.0f, 1.0f };
	glDisable(m_glLight);
#endif

	// Initialize the light.
	m_color							= LcTColor::WHITE;
	m_intensity						= 0.0f;
	m_ambient						= LcTColor::GRAY20;
	m_specular						= LcTColor::WHITE;

	// Set the ambient color.
	LcOglTScalar ambientLight[]= {
			LC_OGL_FROM_SCALAR(LcTScalar(int(m_ambient.rgba.r)) / 255.0f) * m_intensity,
			LC_OGL_FROM_SCALAR(LcTScalar(int(m_ambient.rgba.g)) / 255.0f) * m_intensity,
			LC_OGL_FROM_SCALAR(LcTScalar(int(m_ambient.rgba.b)) / 255.0f) * m_intensity,
			1.0f };

#if defined (LC_PLAT_OGL_20)
	if (currentEffect != NULL)
	{
		currentEffect->setAmbientLight(m_glLight, ambientLight);	
	}	
#else
	LC_OGL_FXV(glLight)(m_glLight, GL_AMBIENT, ambientLight);
#endif

	// Set the lights color.
	LcOglTScalar diffuseLight[]= {
			LC_OGL_FROM_SCALAR(LcTScalar(int(m_color.rgba.r)) / 255.0f) * m_intensity,
			LC_OGL_FROM_SCALAR(LcTScalar(int(m_color.rgba.g)) / 255.0f) * m_intensity,
			LC_OGL_FROM_SCALAR(LcTScalar(int(m_color.rgba.b)) / 255.0f) * m_intensity,
			1.0f };

#if defined (LC_PLAT_OGL_20)
	if (currentEffect != NULL)
	{
		currentEffect->setDiffuseLight(m_glLight, diffuseLight);	
	}
#else
	LC_OGL_FXV(glLight)(m_glLight, GL_DIFFUSE, diffuseLight);
#endif

	// Set the specular color.
	LcOglTScalar specularColor[]= {
			LC_OGL_FROM_SCALAR(LcTScalar(int(m_specular.rgba.r)) / 255.0f) * m_intensity,
			LC_OGL_FROM_SCALAR(LcTScalar(int(m_specular.rgba.g)) / 255.0f) * m_intensity,
			LC_OGL_FROM_SCALAR(LcTScalar(int(m_specular.rgba.b)) / 255.0f) * m_intensity,
			1.0f };
			
#if defined (LC_PLAT_OGL_20)
	if (currentEffect != NULL)
	{
		currentEffect->setSpecularLight(m_glLight, specularColor);
	}
#else
	LC_OGL_FXV(glLight)(m_glLight, GL_SPECULAR, specularColor);

	// Setup the light position.
	LC_OGL_FXV(glLight)(m_glLight, GL_POSITION, initialPosition);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcOglCLight::~LcOglCLight()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcOglCLight::draw(	LcTColor color,
											LcTColor ambient,
											LcTScalar intensity)
{
	// Abort if disabled.
	if (!isEnabled())
		return;
		
#if	defined (LC_PLAT_OGL_20)
	LcOglCEffect* currentEffect = m_space->getOglContext()->getCurrentEffect();
#endif

	// Update transforms
	m_space->transformsChanged();

	// Check if an update is required.
	bool forceUpdate = ((m_intensity != intensity) || m_previouslyDisabled);
	if (forceUpdate)
	{
		m_intensity = intensity;
		m_previouslyDisabled = false;
	}

	if (forceUpdate || (m_ambient != ambient))
	{
		// Set the ambient color.
		m_ambient = ambient;
		LcOglTScalar ambientLight[]= {
				LC_OGL_FROM_SCALAR(LcTScalar(int(m_ambient.rgba.r)) / 255.0f) * m_intensity,
				LC_OGL_FROM_SCALAR(LcTScalar(int(m_ambient.rgba.g)) / 255.0f) * m_intensity,
				LC_OGL_FROM_SCALAR(LcTScalar(int(m_ambient.rgba.b)) / 255.0f) * m_intensity,
				1.0f };
				
#if	defined (LC_PLAT_OGL_20)
	currentEffect = m_space->getOglContext()->getCurrentEffect();
	
	if (currentEffect != NULL)
	{
		currentEffect->setAmbientLight(m_glLight, ambientLight);		
	}		
#else
		LC_OGL_FXV(glLight)(m_glLight, GL_AMBIENT, ambientLight);
#endif
	}

	if (forceUpdate || (m_color != color))
	{
		// Set the lights color.
		m_color = color;
		LcOglTScalar diffuseLight[]= {
				LC_OGL_FROM_SCALAR(LcTScalar(int(m_color.rgba.r)) / 255.0f) * m_intensity,
				LC_OGL_FROM_SCALAR(LcTScalar(int(m_color.rgba.g)) / 255.0f) * m_intensity,
				LC_OGL_FROM_SCALAR(LcTScalar(int(m_color.rgba.b)) / 255.0f) * m_intensity,
				1.0f };
#if defined (LC_PLAT_OGL_20)
	if (currentEffect != NULL)
	{
		currentEffect->setDiffuseLight(m_glLight, diffuseLight);		
	}
#else
		LC_OGL_FXV(glLight)(m_glLight, GL_DIFFUSE, diffuseLight);
#endif
	}

	if (forceUpdate)
	{
		// Set the specular color.
		LcOglTScalar specularColor[]= {
				LC_OGL_FROM_SCALAR(LcTScalar(int(m_specular.rgba.r)) / 255.0f) * m_intensity,
				LC_OGL_FROM_SCALAR(LcTScalar(int(m_specular.rgba.g)) / 255.0f) * m_intensity,
				LC_OGL_FROM_SCALAR(LcTScalar(int(m_specular.rgba.b)) / 255.0f) * m_intensity,
				1.0f };

#if defined (LC_PLAT_OGL_20)
	if (currentEffect != NULL)
	{
		currentEffect->setSpecularLight(m_glLight, specularColor);		
	}
#else
		LC_OGL_FXV(glLight)(m_glLight, GL_SPECULAR, specularColor);
#endif
	}

#if defined (LC_PLAT_OGL_20)

	LcOglCGlobalState* globalState = m_space->getOglContext()->getGlobalState();
	
	if (m_lightType == LcwCLight::EDirectional)
	{
		float ModelViewMat[16];
		
		LcOglTScalar directionalPosition[] = { 0.0f, 0.0f, 1.0f, 0.0f };
		
		if (currentEffect != NULL)
		{			
			globalState->getMVMatrixState(ModelViewMat);
						
			// Transform light position w.r.t current model view matrix
			LcTTransform t1, t2;
			t1.identity();
			t2 = t1.setFromRowMajorMatrixArray ((LcTScalar *)ModelViewMat).transpose();
			LcTVector vec = t2.transform (LcTVector (0.0,0.0,1.0));
		
			directionalPosition[0] = vec.x;
			directionalPosition[1] = vec.y;
			directionalPosition[2] = vec.z;
			directionalPosition[3] = 0.0;
		
			currentEffect->setPositionLight(m_glLight, directionalPosition);			
		}
	}
	else
	if (m_lightType == LcwCLight::EBulb)
	{
		LcOglTScalar lightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		
		float ModelViewMat[16];
		
		if (currentEffect != NULL)
		{
			globalState->getMVMatrixState(ModelViewMat);
		
			lightPosition [0] = ModelViewMat [12];
			lightPosition [1] = ModelViewMat [13];
			lightPosition [2] = ModelViewMat [14];
			lightPosition [3] = 1.0;
		
			currentEffect->setPositionLight(m_glLight, lightPosition);			
		}
	}	
#else
	// Set the starting position.
	// Directional has a special position because it is a vector.
	LcOglTScalar bulbPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	LcOglTScalar directionalPosition[] = { 0.0f, 0.0f, 1.0f, 0.0f };

	if (m_lightType == LcwCLight::EDirectional)
	{	
		LC_OGL_FXV(glLight)(m_glLight, GL_POSITION, directionalPosition);
	}
	else
	{
		LC_OGL_FXV(glLight)(m_glLight, GL_POSITION, bulbPosition);
	}
#endif

}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcOglCLight::setAttenuation(LcTScalar constant, LcTScalar linear, LcTScalar quadratic)
{
	#if	defined (LC_PLAT_OGL_20)
	LcOglCEffect* currentEffect = m_space->getOglContext()->getCurrentEffect();
	#endif
	
	if (m_lightType != LcwCLight::EDirectional)
	{
#if defined (LC_PLAT_OGL_20)
		if (currentEffect != NULL)
		{
			currentEffect->setLightAttenuationFactors(m_glLight, constant, linear, quadratic);			
		}		
#else
		LC_OGL_FX(glLight)(m_glLight, GL_CONSTANT_ATTENUATION, constant);  
		LC_OGL_FX(glLight)(m_glLight, GL_LINEAR_ATTENUATION, linear);  
		LC_OGL_FX(glLight)(m_glLight, GL_QUADRATIC_ATTENUATION, quadratic);
#endif
	}
	else
	{
#if defined (LC_PLAT_OGL_20)
		if (currentEffect != NULL)
		{
			currentEffect->setLightAttenuationFactors(m_glLight, LcTScalar(1.0f), LcTScalar(0.0f), LcTScalar(0.0f));
		}		
#else
		LC_OGL_FX(glLight)(m_glLight, GL_CONSTANT_ATTENUATION, LcTScalar(1.0f));
		LC_OGL_FX(glLight)(m_glLight, GL_LINEAR_ATTENUATION, LcTScalar(0.0f));
		LC_OGL_FX(glLight)(m_glLight, GL_QUADRATIC_ATTENUATION, LcTScalar(0.0f));
#endif	
	}
}

/*-------------------------------------------------------------------------*//**
	Enable the light.
*/
LC_EXPORT void LcOglCLight::enableLight()
{
	if (!isEnabled())
	{
#if defined (LC_PLAT_OGL_20)

		LcOglCEffect* currentEffect = m_space->getOglContext()->getCurrentEffect();
		LcOglCGlobalState *state = m_space->getOglContext()->getGlobalState();
		
		currentEffect->enableLight(m_glLight);
		
		if(m_lightType == LcwCLight::EBulb)
		{
			state->incPointLightCount(m_glLight);
		}
		
#else
		glEnable(m_glLight);
#endif
		setEnabled(true);
		m_previouslyDisabled = true;
		m_temporaryDisabled = false;
	}
}

/*-------------------------------------------------------------------------*//**
	Disable the light.
*/
LC_EXPORT void LcOglCLight::disableLight()
{
	if (isEnabled())
	{
#if defined (LC_PLAT_OGL_20)
		LcOglCEffect* currentEffect = m_space->getOglContext()->getCurrentEffect();
		LcOglCGlobalState *state = m_space->getOglContext()->getGlobalState();
		currentEffect->disableLight(m_glLight);
		
		if(m_lightType == LcwCLight::EBulb)
		{
			state->decPointLightCount(m_glLight);
		}
#else
		glDisable(m_glLight);
#endif
		setEnabled(false);
	}
}

/*-------------------------------------------------------------------------*//**
	Retrieve the light index
*/

LC_EXPORT int LcOglCLight::getLightIndex() 
{
#if defined (LC_PLAT_OGL_20)
	return (m_glLight);
#else
	// OGL 1.1 has all light indices referenced w.r.t. GL_LIGHT0
	return (m_glLight - GL_LIGHT0);
#endif
}

#endif // #if defined(LC_PLAT_OGL) && defined(LC_USE_LIGHTS)
