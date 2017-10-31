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
#ifndef LcOglCLightH
#define LcOglCLightH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcwCLight.h"

/*-------------------------------------------------------------------------*//**
	Abstract base class that stores data representing a light.
*/
class LcOglCLight : public LcCLight
{

	int								m_glLight;
	LcwCLight::TLightType			m_lightType;
	bool							m_previouslyDisabled;

	// Light values
	LcTColor						m_color;
	LcTColor						m_ambient;
	LcTColor						m_specular;
	LcTScalar						m_intensity;
	
	LcCSpace* 						m_space;
	

LC_PRIVATE_INTERNAL_PUBLIC:

	// Generic draw() API with optional color request
	LC_VIRTUAL		void			draw(	LcTColor			color,
											LcTColor			ambient,
											LcTScalar			intensity);

protected:

	// Abstract so keep constructor protected
	LC_IMPORT						LcOglCLight(LcCSpace* sp) : LcCLight(sp) {}
	LC_IMPORT		void			construct(LcCSpace* space, int lightIndex);

public:

	// Construction
	LC_IMPORT	static LcTaOwner<LcOglCLight> create(LcCSpace* sp, int lightIndex);

	// Destruction
	LC_VIRTUAL						~LcOglCLight();

	// Configure the light.
	LC_IMPORT		void			setAttenuation(LcTScalar constant, LcTScalar linear, LcTScalar quadratic);
	LC_IMPORT		void			setSpecularColor(LcTColor col)				{ m_specular = col; }
	LC_IMPORT		void			setLightType(LcwCLight::TLightType type)	{ m_lightType = type; }

	// Enable and disable the light.
	LC_VIRTUAL		void			enableLight();
	LC_VIRTUAL		void			disableLight();

	LC_IMPORT       int             getLightIndex(); 
};

#endif // LcOglCLightH
