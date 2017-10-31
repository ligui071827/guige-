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

#include <math.h>

#define ZERO_TOLERANCE .001

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcwCLight> LcwCLight::create()
{
	LcTaOwner<LcwCLight> ref;
	ref.set(new LcwCLight);
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcwCLight::LcwCLight()
	: LcCWidget()
{
	m_attenuationConstant	= 1.0f;
	m_attenuationLinear		= 0.0f;
	m_attenuationQuadratic	= 0.0f;
	m_lightType				= EBulb;
	m_specularColor			= LcTColor::WHITE;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcwCLight::~LcwCLight()
{
	retire();
}

void LcwCLight::releaseResources()
{
	if(m_light)
	{
		setLight(NULL);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCLight::reloadResources()
{
	LcCWidget::reloadResources();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcwCLight::setLight(LcCLight* light)
{
	// Do nothing if mesh isn't changing
	if (m_light == light)
		return;

	// Detach from previous
	if (m_light)
	{
		getSpace()->releaseSecondaryLight(m_light);
		m_light->disableLight();
		m_light = NULL;
	}

	// If we are setting a mesh rather than clearing
	if (light)
	{
		m_light = light;

		m_light->setLightType(m_lightType);
		m_light->setAttenuation(m_attenuationConstant, m_attenuationLinear, m_attenuationQuadratic);
		m_light->setSpecularColor(m_specularColor);
		m_light->enableLight();
	}

	// Trigger repaint
	LcCWidget::setDirty();
	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCLight::onRealize()
{
	// If the widget is realized add it to the spaces light list.
	LcCSpace* sp = getSpace();
	if (sp)
	{
		sp->addLightWidget(this);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCLight::onRetire()
{
	// Remove the widget.
	LcCSpace* sp = getSpace();
	if (sp)
	{
		setLight(NULL);
		sp->removeLightWidget(this);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCLight::onPaint(const LcTPixelRect& clip)
{
	if (m_light)
	{
		LcTScalar intensity = getPlacement().intensity;
		bool lightEnabled = m_light->isEnabled();
		bool intensityZero = lc_fabs(intensity) < ZERO_TOLERANCE;

		// If the light intensity is 0, disable the light.
		if (lightEnabled && intensityZero)
		{
			m_light->disableLight();
		}
		else if (!lightEnabled && !intensityZero)
		{
			m_light->enableLight();
		}

		// Draw the light.
		m_light->draw(	getPlacement().color,
						getPlacement().color2,
						intensity);
	}
}

/*-------------------------------------------------------------------------*//**
	Sets the desired visibility of this widget.  The widget may still not
	actually appear yet, if it is not realized or its parent is not visible
*/
LC_EXPORT void LcwCLight::setVisible(bool b)
{

	if (b == this->getVisible())
		return;

	// If the visibility has changed dirty the light.
	LcCWidget::setVisible(b);
	LcCWidget::setDirty();
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCLight::setAttenuation(LcTScalar constant, LcTScalar linear, LcTScalar quadratic)
{
	m_attenuationConstant	= constant;
	m_attenuationLinear		= linear;
	m_attenuationQuadratic	= quadratic;
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCLight::setLightType(TLightType type)
{
	m_lightType = type;
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCLight::setSpecularColor(LcTColor color)
{
	m_specularColor = color;
}

#endif // #if defined(LC_PLAT_OGL) && defined(LC_USE_LIGHTS)
