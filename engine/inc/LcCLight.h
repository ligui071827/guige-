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
#ifndef LcCLightH
#define LcCLightH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTString.h"
#include "inflexionui/engine/inc/LcwCLight.h"

/*-------------------------------------------------------------------------*//**
	Abstract base class that stores data representing a light.
*/
class LcCLight : public LcCBase
{
	// Space which owns us
	LcCSpace*						m_space;
	bool							m_isEnabled;

LC_PRIVATE_INTERNAL_PUBLIC:

	// Generic draw() API with optional color request
	LC_VIRTUAL		void			draw(	LcTColor			color,
											LcTColor			ambient,
											LcTScalar			intensity)	{}
protected:

	bool							m_temporaryDisabled;

	// Abstract so keep constructor protected
	LC_IMPORT						LcCLight(LcCSpace* sp);
					void			setEnabled(bool enable)	{ m_isEnabled = enable; }

public:

	LC_VIRTUAL						~LcCLight();

	// Accessors
	inline			LcCSpace*		getSpace()				{ return m_space; }


	// Configure the light.
	LC_VIRTUAL		void			setAttenuation(LcTScalar constant, LcTScalar linear, LcTScalar quadratic) = 0;
	LC_VIRTUAL		void			setSpecularColor(LcTColor col) = 0;
	LC_VIRTUAL		void			setLightType(LcwCLight::TLightType type) = 0;

	// Enable and disable the light.
	LC_VIRTUAL		void			enableLight() = 0;
	LC_VIRTUAL		void			disableLight() = 0;
					bool			isEnabled()				{ return m_isEnabled; }
					bool			isTemporaryDisabled()	{ return m_temporaryDisabled; }
					void			temporaryDisable()		{ m_temporaryDisabled = true; disableLight(); }
};

#endif // LcCLightH
