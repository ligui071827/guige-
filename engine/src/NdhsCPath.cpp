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


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<NdhsCPath> NdhsCPath::create()
{
	LcTaOwner<NdhsCPath> ref;
	ref.set(new NdhsCPath);
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/

NdhsCPath::NdhsCPath()
{
	clearPath();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPath::clearPath()
{
	// Set both ends of path to default values
	m_A = LcTPlacement();
	m_B = LcTPlacement();

	m_rDelta = 0;
	m_gDelta = 0;
	m_bDelta = 0;
	m_aDelta = 0;

	m_r2Delta = 0;
	m_g2Delta = 0;
	m_b2Delta = 0;
	m_a2Delta = 0;

	// Reset the mask
	m_mask = 0;

	m_locationAnimType = ENdhsDecoratorTypeLinear;
	m_extentAnimType = ENdhsDecoratorTypeLinear;
	m_frameAnimType = ENdhsDecoratorTypeLinear;
	m_colorAnimType = ENdhsDecoratorTypeLinear;
	m_scaleAnimType = ENdhsDecoratorTypeLinear;
	m_rotationAnimType = ENdhsDecoratorTypeLinear;
	m_opacityAnimType = ENdhsDecoratorTypeLinear;
	m_offsetAnimType = ENdhsDecoratorTypeLinear;
	m_intensityAnimType = ENdhsDecoratorTypeLinear;
	m_color2AnimType = ENdhsDecoratorTypeLinear;
}

/*-------------------------------------------------------------------------*//**
	Calculate delta's for linear anim.
*/
LC_EXPORT void NdhsCPath::setPathData(
	const LcTPlacement& a, const LcTPlacement& b, int mask, ENdhsDecoratorType type)
{
	// Set A to be the absolute reference placement (pos = 0)
	m_A				= a;
	m_mask			= 0;

	m_locationAnimType = type;
	m_extentAnimType = type;
	m_frameAnimType = type;
	m_colorAnimType = type;
	m_scaleAnimType = type;
	m_rotationAnimType = type;
	m_opacityAnimType = type;
	m_offsetAnimType = type;
	m_intensityAnimType = type;
	m_color2AnimType = type;

	// Set B to be the relative offset from A (pos = 1)
	if (mask & LcTPlacement::ELocation)
	{
		m_B.location = LcTVector::subtract(b.location, a.location);

		if (m_B.location.isZero() == false)
			m_mask |= LcTPlacement::ELocation;
	}
	if (mask & LcTPlacement::EExtent)
	{
		m_B.extent = LcTVector::subtract(b.extent, a.extent);

		if (m_B.extent.isZero() == false)
			m_mask |= LcTPlacement::EExtent;
	}
	if (mask & LcTPlacement::EScale)
	{
		m_B.scale = LcTVector::subtract(b.scale, a.scale);

		if (m_B.scale.isZero() == false)
			m_mask |= LcTPlacement::EScale;
	}
	if (mask & LcTPlacement::EOrientation)
	{
		// Orientation is a special case - store the end value not the offset,
		m_B.orientation = b.orientation;

		if (m_B.orientation.equals(m_A.orientation) == false)
			m_mask |= LcTPlacement::EOrientation;
	}
	if (mask & LcTPlacement::EOpacity)
	{
		m_B.opacity = b.opacity - a.opacity;

		if (m_B.opacity != 0)
			m_mask |= LcTPlacement::EOpacity;
	}
	if (mask & LcTPlacement::EColor)
	{
		m_rDelta = b.color.rgba.r - a.color.rgba.r;
		m_gDelta = b.color.rgba.g - a.color.rgba.g;
		m_bDelta = b.color.rgba.b - a.color.rgba.b;
		m_aDelta = b.color.rgba.a - a.color.rgba.a;

		if ((m_rDelta != 0) || (m_gDelta != 0)
				|| (m_bDelta != 0) || (m_aDelta != 0))
			m_mask |= LcTPlacement::EColor;
	}
	if (mask & LcTPlacement::EFrame)
	{
		m_B.frame = b.frame - a.frame;

		if (m_B.frame != 0)
			m_mask |= LcTPlacement::EFrame;
	}
	if (mask & LcTPlacement::EOffset)
	{
		m_B.centerOffset = LcTVector::subtract(b.centerOffset, a.centerOffset);

		if (m_B.centerOffset.isZero() == false)
			m_mask |= LcTPlacement::EOffset;
	}

	if (mask & LcTPlacement::EIntensity)
	{
		m_B.intensity = b.intensity - a.intensity;

		if (m_B.intensity != 0)
			m_mask |= LcTPlacement::EIntensity;
	}

	if (mask & LcTPlacement::EColor2)
	{
		m_r2Delta = b.color2.rgba.r - a.color2.rgba.r;
		m_g2Delta = b.color2.rgba.g - a.color2.rgba.g;
		m_b2Delta = b.color2.rgba.b - a.color2.rgba.b;
		m_a2Delta = b.color2.rgba.a - a.color2.rgba.a;

		if ((m_r2Delta != 0) || (m_g2Delta != 0)
				|| (m_b2Delta != 0) || (m_a2Delta != 0))
			m_mask |= LcTPlacement::EColor2;
	}

#if defined(LC_PLAT_OGL_20)

	if (mask & LcTPlacement::EUniform)
	{
		LcTmOwnerMap<LcTmString, LcOglCSLType>::iterator it;
		LcTPlacement *A = (LcTPlacement *)&a;
		LcTPlacement *B = (LcTPlacement *)&b;

		LcTmString resourceName;
		LcTmString name;
		LcOglCSLType *slType = NULL;

		LcTaOwner<LcOglCSLType> ptr;

		int sizeA = A->layoutUniMap.size();
		int sizeB = B->layoutUniMap.size();

		if (sizeA == sizeB)
		{
			for(it=A->layoutUniMap.begin(); it != A->layoutUniMap.end(); it++)
			{
				name = it->first;
				slType = it->second;

				LcOglCSLType *slTypeB;

				slTypeB = B->layoutUniMap[name];

				if ( (!slType) || (!slTypeB) )	continue;

				//--------------------------------------------------------
				// float
				//--------------------------------------------------------
				if (slType->getSLTypeIdentifier().compareNoCase("float") == 0)
				{
					float valueA = ((LcOglCSLTypeScalar<float> *)slType)->getValue();
					float valueB = ((LcOglCSLTypeScalar<float> *)slTypeB)->getValue();
					float deltaValue = (valueB - valueA);

					// Create a new SLType with delta value(s)
					LcTaOwner< LcOglCSLTypeScalar<float> > uniform = LcOglCSLTypeScalar<float>::create();

					uniform->setSLType(ELcOglSLTypeScalar);
					uniform->setSLTypeName(name);
					uniform->setDimension(1);
					uniform->setValue(&deltaValue);

					ptr = uniform;

					LcTmString typ = slType->getSLTypeIdentifier();

					ptr->setSLTypeName(name);
					ptr->setSLTypeIdentifier(typ);

					m_B.layoutUniMap.add_element(name, ptr);

					if (deltaValue != 0.0)
						m_mask |= LcTPlacement::EUniform;
				}

				//--------------------------------------------------------
				// vec2
				//--------------------------------------------------------
				else if (slType->getSLTypeIdentifier().compareNoCase("vec2") == 0)
				{
					float valueA[2];
					float valueB[2];
					float deltaValue[2];

					((LcOglCSLTypeVector<float> *)slType)->getValue(valueA);
					((LcOglCSLTypeVector<float> *)slTypeB)->getValue(valueB);

					deltaValue[0] = (valueB[0] - valueA[0]);
					deltaValue[1] = (valueB[1] - valueA[1]);

					// Create a new SLType with delta value(s)
					LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(2);

					uniform->setValue(deltaValue);
					uniform->setSLType(ELcOglSLTypeVector);
					uniform->setSLTypeName(name);
					uniform->setDimension(2);

					ptr = uniform;

					LcTmString typ = slType->getSLTypeIdentifier();

					ptr->setSLTypeName(name);
					ptr->setSLTypeIdentifier(typ);

					m_B.layoutUniMap.add_element(name, ptr);

					if ( (deltaValue[0] != 0.0) || (deltaValue[1] != 0.0) )
						m_mask |= LcTPlacement::EUniform;
				}

				//--------------------------------------------------------
				// vec3
				//--------------------------------------------------------
				else if (slType->getSLTypeIdentifier().compareNoCase("vec3") == 0)
				{
					float valueA[3];
					float valueB[3];
					float deltaValue[3];

					((LcOglCSLTypeVector<float> *)slType)->getValue(valueA);
					((LcOglCSLTypeVector<float> *)slTypeB)->getValue(valueB);

					deltaValue[0] = (valueB[0] - valueA[0]);
					deltaValue[1] = (valueB[1] - valueA[1]);
					deltaValue[2] = (valueB[2] - valueA[2]);

					// Create a new SLType with delta value(s)
					LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(3);

					uniform->setValue(deltaValue);
					uniform->setSLType(ELcOglSLTypeVector);
					uniform->setSLTypeName(name);
					uniform->setDimension(3);

					ptr = uniform;

					LcTmString typ = slType->getSLTypeIdentifier();

					ptr->setSLTypeName(name);
					ptr->setSLTypeIdentifier(typ);

					m_B.layoutUniMap.add_element(name, ptr);

					if ( (deltaValue[0] != 0.0) || (deltaValue[1] != 0.0) || (deltaValue[2] != 0.0) )
						m_mask |= LcTPlacement::EUniform;
				}

				//--------------------------------------------------------
				// vec4
				//--------------------------------------------------------
				else if (slType->getSLTypeIdentifier().compareNoCase("vec4") == 0)
				{
					float valueA[4];
					float valueB[4];
					float deltaValue[4];

					((LcOglCSLTypeVector<float> *)slType)->getValue(valueA);
					((LcOglCSLTypeVector<float> *)slTypeB)->getValue(valueB);

					deltaValue[0] = (valueB[0] - valueA[0]);
					deltaValue[1] = (valueB[1] - valueA[1]);
					deltaValue[2] = (valueB[2] - valueA[2]);
					deltaValue[3] = (valueB[3] - valueA[3]);

					// Create a new SLType with delta value(s)
					LcTaOwner< LcOglCSLTypeVector<float> > uniform = LcOglCSLTypeVector<float>::create(4);

					uniform->setValue(deltaValue);
					uniform->setSLType(ELcOglSLTypeVector);
					uniform->setSLTypeName(name);
					uniform->setDimension(4);

					ptr = uniform;

					LcTmString typ = slType->getSLTypeIdentifier();

					ptr->setSLTypeName(name);
					ptr->setSLTypeIdentifier(typ);

					m_B.layoutUniMap.add_element(name, ptr);

					if ( (deltaValue[0] != 0.0) || (deltaValue[1] != 0.0) || (deltaValue[2] != 0.0) || (deltaValue[3] != 0.0) )
						m_mask |= LcTPlacement::EUniform;
				}
			}
		}
	}
#endif
}

/*-------------------------------------------------------------------------*//**
	Sets a straight path for the widget to follow.

	The widget's location will traverse linearly between 'a' and 'b'.

	@param a The start location of the path.
	@param b The end location of the path.
*/
LC_EXPORT void NdhsCPath::setLocation(const LcTVector& a, const LcTVector& b, ENdhsDecoratorType type)
{
	m_A.location	= a;
	m_B.location	= LcTVector::subtract(b, a);

	if (m_B.location.isZero() == false)
		m_mask |= LcTPlacement::ELocation;

	m_locationAnimType = type;
}

/*-------------------------------------------------------------------------*//**
	Sets animated range of extent for the widget.

	The widget's location will traverse linearly between 'a' and 'b'.

	@param a The start extent of the path.
	@param b The end extent of the path.
*/
LC_EXPORT void NdhsCPath::setExtent(const LcTVector& a, const LcTVector& b, ENdhsDecoratorType type)
{
	m_A.extent		= a;
	m_B.extent		= LcTVector::subtract(b, a);

	if (m_B.extent.isZero() == false)
		m_mask |= LcTPlacement::EExtent;

	m_extentAnimType = type;
}

/*-------------------------------------------------------------------------*//**
	Set a range of opacity for the widget to animate between.

	@param a The start opacity in the range 0 to 1 inclusive.
	@param b The end opacity in the range 0 to 1 inclusive.
*/
LC_EXPORT void NdhsCPath::setOpacity(LcTScalar a, LcTScalar b, ENdhsDecoratorType type)
{
	m_A.opacity		= a;
	m_B.opacity		= b - a;

	if (m_B.opacity != 0)
		m_mask |= LcTPlacement::EOpacity;

	m_opacityAnimType = type;
}

/*-------------------------------------------------------------------------*//**
	Set a range of color and alpha for the widget to animate between.

	@param a The start color.
	@param b The end color.
*/
LC_EXPORT void NdhsCPath::setColor(LcTColor& a, LcTColor& b, ENdhsDecoratorType type)
{
	m_A.color			= a;
	m_rDelta			= b.rgba.r - a.rgba.r;
	m_gDelta			= b.rgba.g - a.rgba.g;
	m_bDelta			= b.rgba.b - a.rgba.b;
	m_aDelta			= b.rgba.a - a.rgba.a;

	if ((m_rDelta != 0) || (m_gDelta != 0)
			|| (m_bDelta != 0) || (m_aDelta != 0))
		m_mask |= LcTPlacement::EColor;

	m_colorAnimType		= type;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPath::setColorA(const LcTColor& a)
{
	m_A.color			= a;
	m_rDelta			= m_B.color.rgba.r - a.rgba.r;
	m_gDelta			= m_B.color.rgba.g - a.rgba.g;
	m_bDelta			= m_B.color.rgba.b - a.rgba.b;
	m_aDelta			= m_B.color.rgba.a - a.rgba.a;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPath::setColorB(const LcTColor& b, ENdhsDecoratorType type)
{
	m_B.color			= b;
	m_mask				|= LcTPlacement::EColor;

	// Only set when appropriate
	if (type != ENdhsDecoratorTypeIgnore)
	{
		m_colorAnimType	= type;
	}
}

/*-------------------------------------------------------------------------*//**
	Set a range of frame index for the widget to animate between.

	@param a The start frame.
	@param b The end frame.
*/
LC_EXPORT void NdhsCPath::setFrame(int a, int b, ENdhsDecoratorType type)
{
	m_A.frame		= a;
	m_B.frame		= b - a;

	if (m_B.frame != 0)
		m_mask |= LcTPlacement::EFrame;

	m_frameAnimType = type;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPath::setOffset(const LcTVector& a, const LcTVector& b, ENdhsDecoratorType type)
{
	m_A.centerOffset 	= a;
	m_B.centerOffset 	= LcTVector::subtract(b, a);

	if (m_B.centerOffset.isZero() == false)
		m_mask |= LcTPlacement::EOffset;

	m_offsetAnimType 	= type;
}

/*-------------------------------------------------------------------------*//**
	Set a range of scaling for the widget to animate between.

	@param a The starting scale vector.
	@param b The ending scale vector.
*/
LC_EXPORT void NdhsCPath::setScale(const LcTVector& a, const LcTVector& b, ENdhsDecoratorType type)
{
	m_A.scale		= a;
	m_B.scale		= LcTVector::subtract(b, a);

	if (m_B.scale.isZero() == false)
		m_mask |= LcTPlacement::EScale;

	m_scaleAnimType = type;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPath::setOrientation(const LcTQuaternion& a, const LcTQuaternion& b, ENdhsDecoratorType type)
{
	m_A.orientation		= a;
	m_B.orientation		= b;

	if (m_B.orientation.equals(m_A.orientation) == false)
		m_mask |= LcTPlacement::EOrientation;

	m_rotationAnimType	= type;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPath::setIntensity(LcTScalar a, LcTScalar b, ENdhsDecoratorType type)
{
	m_A.intensity	= a;
	m_B.intensity	= b - a;

	if (m_B.intensity != 0)
		m_mask |= LcTPlacement::EIntensity;

	m_intensityAnimType = type;
}

/*-------------------------------------------------------------------------*//**
	Set a range of color and alpha for the widget to animate between.

	@param a The start color2.
	@param b The end color2.
*/
LC_EXPORT void NdhsCPath::setColor2(LcTColor& a, LcTColor& b, ENdhsDecoratorType type)
{
	m_A.color2			= a;
	m_r2Delta			= b.rgba.r - a.rgba.r;
	m_g2Delta			= b.rgba.g - a.rgba.g;
	m_b2Delta			= b.rgba.b - a.rgba.b;
	m_a2Delta			= b.rgba.a - a.rgba.a;

	if ((m_r2Delta != 0) || (m_g2Delta != 0)
			|| (m_b2Delta != 0) || (m_a2Delta != 0))
		m_mask |= LcTPlacement::EColor2;

	m_color2AnimType	= type;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPath::setColor2A(const LcTColor& a)
{
	m_A.color2			= a;
	m_r2Delta			= m_B.color2.rgba.r - a.rgba.r;
	m_g2Delta			= m_B.color2.rgba.g - a.rgba.g;
	m_b2Delta			= m_B.color2.rgba.b - a.rgba.b;
	m_a2Delta			= m_B.color2.rgba.a - a.rgba.a;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void NdhsCPath::setColor2B(const LcTColor& b, ENdhsDecoratorType type)
{
	m_B.color2			= b;
	m_mask				|= LcTPlacement::EColor2;

	// Only set when appropriate
	if (type != ENdhsDecoratorTypeIgnore)
	{
		m_color2AnimType	= type;
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int NdhsCPath::getMask() const
{
	// But return only those bits which are legal for path animations
	return (m_mask & LcTPlacement::EAll);
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCPath::getPlacement(LcTUnitScalar d, LcTPlacement& sa)
{
	// Clamp range
	LcTUnitScalar unit0 = LcTUnitScalar(0);
	LcTUnitScalar unit1 = LcTUnitScalar(1);
	LcTScalar c = max(unit0, min(unit1, d));

	if (m_mask & LcTPlacement::ELocation)
	{
		switch(m_locationAnimType)
		{
			case ENdhsDecoratorTypeLinear:
			{
				sa.location = LcTVector::scale(m_B.location, d).add(m_A.location);
			}
			break;

			case ENdhsDecoratorTypeSticky:
			{
				if (d < 1)
					sa.location = m_A.location;
				else
					sa.location = m_B.location;
			}
			break;

			default:
			break;
		}
	}

	if (m_mask & LcTPlacement::EExtent)
	{
		switch(m_extentAnimType)
		{
			case ENdhsDecoratorTypeLinear:
			{
				sa.extent = LcTVector::scale(m_B.extent, d).add(m_A.extent);
				sa.extent.capAtZero();
			}
			break;

			case ENdhsDecoratorTypeSticky:
			{
				if (d < 1)
					sa.extent = m_A.extent;
				else
					sa.extent = m_B.extent;
			}
			break;

			default:
			break;
		}
	}

	if (m_mask & LcTPlacement::EScale)
	{
		switch(m_scaleAnimType)
		{
			case ENdhsDecoratorTypeLinear:
			{
				sa.scale = LcTVector::scale(m_B.scale, d).add(m_A.scale);
				sa.scale.capAtZero();
			}
			break;

			case ENdhsDecoratorTypeSticky:
			{
				if (d < 1)
					sa.scale = m_A.scale;
				else
					sa.scale = LcTVector::scale(m_B.scale, 1).add(m_A.scale);
			}
			break;

			default:
			break;
		}
	}

	if (m_mask & LcTPlacement::EOrientation)
	{
		switch(m_rotationAnimType)
		{
			case ENdhsDecoratorTypeLinear:
			{
				// Note: orientation is a special case and m_B contains the end orientation
				// not the difference between the start and end, as other fields do
				sa.orientation = LcTQuaternion::interpolate(m_A.orientation, m_B.orientation, d);
			}
			break;

			case ENdhsDecoratorTypeSticky:
			{
				if (d < 1)
					sa.orientation = m_A.orientation;
				else
					sa.orientation = m_B.orientation;
			}
			break;

			default:
			break;
		}
	}

	if (m_mask & LcTPlacement::EOpacity)
	{
		switch(m_opacityAnimType)
		{
			case ENdhsDecoratorTypeLinear:
			{
				sa.opacity = m_A.opacity + m_B.opacity * d;

				if (sa.opacity < 0)
					sa.opacity = 0;
			}
			break;

			case ENdhsDecoratorTypeSticky:
			{
				if (d < 1)
					sa.opacity = m_A.opacity;
				else
					sa.opacity = m_A.opacity + m_B.opacity * 1;
			}
			break;

			default:
			break;
		}
	}

	if (m_mask & LcTPlacement::EColor)
	{
		// No sticky for color
		sa.color.rgba.r = (unsigned char)(m_A.color.rgba.r + m_rDelta * c);
		sa.color.rgba.g = (unsigned char)(m_A.color.rgba.g + m_gDelta * c);
		sa.color.rgba.b = (unsigned char)(m_A.color.rgba.b + m_bDelta * c);
		sa.color.rgba.a = (unsigned char)(m_A.color.rgba.a + m_aDelta * c);
	}

	if (m_mask & LcTPlacement::EFrame)
	{
		switch(m_frameAnimType)
		{
			case ENdhsDecoratorTypeLinear:
			{
				sa.frame = m_A.frame + calcDecoratorFrameChange(m_B.frame, c);
			}
			break;

			case ENdhsDecoratorTypeSticky:
			{
				if (d < 1)
					sa.frame = m_A.frame;
				else
					sa.frame = m_A.frame + calcDecoratorFrameChange(m_B.frame, 1);
			}
			break;

			default:
			break;
		}
	}

	if (m_mask & LcTPlacement::EOffset)
	{
		switch(m_offsetAnimType)
		{
			case ENdhsDecoratorTypeLinear:
			{
				sa.centerOffset = LcTVector::scale(m_B.centerOffset, d).add(m_A.centerOffset);
			}
			break;

			case ENdhsDecoratorTypeSticky:
			{
				if (d < 1)
					sa.centerOffset = m_A.centerOffset;
				else
					sa.centerOffset = m_B.centerOffset;
			}
			break;

			default:
			break;
		}
	}

	if (m_mask & LcTPlacement::EIntensity)
	{
		switch(m_intensityAnimType)
		{
			case ENdhsDecoratorTypeLinear:
			{
				sa.intensity = m_A.intensity + m_B.intensity * d;

				if (sa.intensity < 0)
					sa.intensity = 0;
			}
			break;

			case ENdhsDecoratorTypeSticky:
			{
				if (d < 1)
					sa.intensity = m_A.intensity;
				else
					sa.intensity = m_A.intensity + m_B.intensity;
			}
			break;

			default:
			break;
		}
	}

	if (m_mask & LcTPlacement::EColor2)
	{
		// No sticky for color
		sa.color2.rgba.r = (unsigned char)(m_A.color2.rgba.r + m_r2Delta * c);
		sa.color2.rgba.g = (unsigned char)(m_A.color2.rgba.g + m_g2Delta * c);
		sa.color2.rgba.b = (unsigned char)(m_A.color2.rgba.b + m_b2Delta * c);
		sa.color2.rgba.a = (unsigned char)(m_A.color2.rgba.a + m_a2Delta * c);
	}

#if defined(LC_PLAT_OGL_20)

	// uniforms
	if (m_mask & LcTPlacement::EUniform)
	{
		LcTmOwnerMap<LcTmString, LcOglCSLType>::iterator it;

		LcTmString resourceName;
		LcTmString name;
		LcOglCSLType *slType;

		LcOglCSLType *ptr = NULL;

		int sizeA = m_A.layoutUniMap.size();
		int sizeB = m_B.layoutUniMap.size();

		if (sizeA == sizeB)
		{
			for(it=m_A.layoutUniMap.begin(); it != m_A.layoutUniMap.end(); it++)
			{
				name = it->first;
				slType = it->second;

				LcOglCSLType *slTypeB;

				slTypeB = m_B.layoutUniMap[name];

				if ( (!slType) || (!slTypeB) )	continue;

				//--------------------------------------------------------
				// float
				//--------------------------------------------------------
				if (slType->getSLTypeIdentifier().compareNoCase("float") == 0)
				{
					float valueA = ((LcOglCSLTypeScalar<float> *)slType)->getValue();
					float valueB = ((LcOglCSLTypeScalar<float> *)slTypeB)->getValue();

					// Now calculate the actual value
					ptr = sa.layoutUniMap[name];

					float finalValue = (c * valueB) + (valueA);

					((LcOglCSLTypeScalar<float> *)ptr)->setValue (&finalValue);

					LcTmString typ = slType->getSLTypeIdentifier();

					ptr->setSLTypeName(name);
					ptr->setSLTypeIdentifier(typ);

					sa.layoutUniMap[name] = ptr;
				}

				//--------------------------------------------------------
				// vec2
				//--------------------------------------------------------
				else if (slType->getSLTypeIdentifier().compareNoCase("vec2") == 0)
				{
					float valueA[2];
					float valueB[2];

					((LcOglCSLTypeVector<float> *)slType)->getValue(valueA);
					((LcOglCSLTypeVector<float> *)slTypeB)->getValue(valueB);

					// Now calculate the actual value
					ptr = sa.layoutUniMap[name];

					float finalValues[2];

					finalValues[0] = (c * valueB[0]) + (valueA[0]);
					finalValues[1] = (c * valueB[1]) + (valueA[1]);

					((LcOglCSLTypeVector<float> *)ptr)->setValue(finalValues);

					LcTmString typ = slType->getSLTypeIdentifier();

					ptr->setSLTypeName(name);
					ptr->setSLTypeIdentifier(typ);

					sa.layoutUniMap[name] = ptr;
				}

				//--------------------------------------------------------
				// vec3
				//--------------------------------------------------------
				else if (slType->getSLTypeIdentifier().compareNoCase("vec3") == 0)
				{
					float valueA[3];
					float valueB[3];

					((LcOglCSLTypeVector<float> *)slType)->getValue(valueA);
					((LcOglCSLTypeVector<float> *)slTypeB)->getValue(valueB);

					// Now calculate the actual value
					ptr = sa.layoutUniMap[name];

					float finalValues[3];

					finalValues[0] = (c * valueB[0]) + (valueA[0]);
					finalValues[1] = (c * valueB[1]) + (valueA[1]);
					finalValues[2] = (c * valueB[2]) + (valueA[2]);

					((LcOglCSLTypeVector<float> *)ptr)->setValue(finalValues);

					LcTmString typ = slType->getSLTypeIdentifier();

					ptr->setSLTypeName(name);
					ptr->setSLTypeIdentifier(typ);

					sa.layoutUniMap[name] = ptr;
				}

				//--------------------------------------------------------
				// vec4
				//--------------------------------------------------------
				else if (slType->getSLTypeIdentifier().compareNoCase("vec4") == 0)
				{
					float valueA[4];
					float valueB[4];

					((LcOglCSLTypeVector<float> *)slType)->getValue(valueA);
					((LcOglCSLTypeVector<float> *)slTypeB)->getValue(valueB);

					// Now calculate the actual value
					ptr = sa.layoutUniMap[name];

					float finalValues[4];

					finalValues[0] = (c * valueB[0]) + (valueA[0]);
					finalValues[1] = (c * valueB[1]) + (valueA[1]);
					finalValues[2] = (c * valueB[2]) + (valueA[2]);
					finalValues[3] = (c * valueB[3]) + (valueA[3]);

					((LcOglCSLTypeVector<float> *)ptr)->setValue(finalValues);

					LcTmString typ = slType->getSLTypeIdentifier();

					ptr->setSLTypeName(name);
					ptr->setSLTypeIdentifier(typ);

					sa.layoutUniMap[name] = ptr;
				}
			}
		}
	}
#endif
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT NdhsCPath& NdhsCPath::operator=(const NdhsCPath& path)
{
	m_A = path.m_A;
	m_B = path.m_B;

	m_rDelta = path.m_rDelta;
	m_gDelta = path.m_gDelta;
	m_bDelta = path.m_bDelta;
	m_aDelta = path.m_aDelta;
	m_mask = path.m_mask;

	m_LocationBasic = path.m_LocationBasic;;

	m_locationAnimType  = path.m_locationAnimType;
	m_extentAnimType = path.m_extentAnimType;
	m_frameAnimType = path.m_frameAnimType;
	m_colorAnimType = path.m_colorAnimType;
	m_scaleAnimType = path.m_scaleAnimType;
	m_rotationAnimType = path.m_rotationAnimType;
	m_opacityAnimType = path.m_opacityAnimType;
	m_offsetAnimType = path.m_offsetAnimType;
	m_intensityAnimType = path.m_intensityAnimType;
	m_color2AnimType = path.m_color2AnimType;

	return *(this);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int NdhsCPath::calcDecoratorFrameChange(int delta, LcTUnitScalar d)
{
	if (delta == 0)
		return 0;

	return (int)(delta * d);
}

