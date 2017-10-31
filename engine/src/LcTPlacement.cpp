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


#include <math.h>

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTPlacement::LcTPlacement()
{
	location	= LcTVector(0, 0, 0);
	extent		= LcTVector(1, 1, 1);
	scale		= LcTVector(1, 1, 1);
	orientation	= LcTQuaternion(); // defaults to (1, 0, 0, 0)
	opacity		= 1;
	color		= LcTColor::WHITE;
	frame		= 0;
	intensity	= 1;
	color2		= LcTColor::GRAY20;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTPlacement::LcTPlacement(
	const LcTVector& lo,
	const LcTVector& ex,
	const LcTVector& sc,
	const LcTQuaternion& ori,
	LcTScalar op,
	LcTColor col,
	int	fr,
	LcTScalar inty,
	LcTColor col2)
{
	location	= lo;
	extent		= ex;
	scale		= sc;
	orientation = ori;
	opacity		= op;
	color		= col;
	frame		= fr;
	intensity	= inty;
	color2		= col2;
}

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

LC_IMPORT void LcTPlacement::operator =(const LcTPlacement & p)
{
	this->location		= p.location;
	this->extent		= p.extent;
	this->scale			= p.scale;
	this->orientation 	= p.orientation;
	this->opacity		= p.opacity;
	this->color			= p.color;
	this->frame			= p.frame;
	this->intensity		= p.intensity;
	this->color2		= p.color2;
	this->centerOffset  = p.centerOffset;

	populateMap(p);
}
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTPlacement::LcTPlacement(const LcTPlacement& p)
{
	this->location		= p.location;
	this->extent		= p.extent;
	this->scale			= p.scale;
	this->orientation 	= p.orientation;
	this->opacity		= p.opacity;
	this->color			= p.color;
	this->frame			= p.frame;
	this->intensity		= p.intensity;
	this->color2		= p.color2;
	this->centerOffset  = p.centerOffset;

	populateMap(p);
}

// Destructor
LC_EXPORT_VIRTUAL LcTPlacement::~LcTPlacement()
{
}
#endif /* #if defined(IFX_RENDER_DIRECT_OPENGL_20) */

#ifdef LC_USE_XML
/*-------------------------------------------------------------------------*//**
	The mask is used on a placement-by-placement basis to
	determine which attributes may be configured in XML.  The application can
	use masks to prevent certain attributes from being modified.
*/
LC_EXPORT int LcTPlacement::configureFromXml(LcOglCEffect *effect, LcCXmlElem* pElem, int iMask, bool bPopulateConfigUnimap)
{
	int	foundMask = 0;

	/////////////////////////
	// Configure the frame
	if (iMask & EFrame)
	{
		LcCXmlAttr* attr = pElem->findAttr(NDHS_TP_XML_FRAME);
		if (attr)
		{
			// Frame numbers are internally 0 to frameCount-1
			// and externally represented as 1 to frameCount
			frame = attr->getVal().toInt()-1;
			if (frame < 0)
				frame = 0;
			foundMask |= EFrame;
		}
	}

	/////////////////////////
	// Configure the color
	if (iMask & EColor)
	{
		LcCXmlAttr* attr = pElem->findAttr(NDHS_TP_XML_COLOR);
		if (attr)
		{
			color = attr->getVal().toInt();
			foundMask |= EColor;
		}
	}

	/////////////////////////
	// Configure the opacity
	if (iMask & EOpacity)
	{
		LcCXmlAttr* attr = pElem->findAttr(NDHS_TP_XML_OPACITY);
		if (attr)
		{
			opacity = attr->getVal().toScalar();

			if (opacity < 0)
				opacity = 0;

			if (opacity > 1)
				opacity = 1;

			foundMask |= EOpacity;
		}
	}

	//////////////////////////
	// Configure the location
	if (iMask & ELocation)
	{
		LcCXmlElem* elemLocation = pElem->find(NDHS_TP_XML_LOCATION);
		if (elemLocation)
		{
			location = LcTVector::createFromXml(elemLocation);
			foundMask |= ELocation;

		}
	}

#ifndef LC_USE_NO_ROTATION
	//////////////////////////
	// Configure the Orientation
	if (iMask & EOrientation)
	{
		// Orientation can be under either name
		LcCXmlElem* elem = pElem->find(NDHS_TP_XML_ORIENTATION);
		if (!elem)
			elem = pElem->find(NDHS_TP_XML_ROTATION);

		if (elem)
		{
			orientation = LcTQuaternion::createFromXml(elem);
			foundMask |= EOrientation;
		}
	}
#endif

	//////////////////////////
	// Configure the extent
	if (iMask & EExtent)
	{
		LcCXmlElem* elemExtent = pElem->find(NDHS_TP_XML_EXTENT);
		if (elemExtent)
		{
			extent = LcTVector::createFromXml(elemExtent);

			foundMask |= EExtent;
		}
	}

	//////////////////////////
	// Configure the scale
	if (iMask & EScale)
	{
		LcCXmlElem* elemScale = pElem->find(NDHS_TP_XML_SCALE);
		if (elemScale)
		{
			scale.x = elemScale->getAttr(NDHS_TP_XML_X, "1").toScalar();
			scale.y = elemScale->getAttr(NDHS_TP_XML_Y, "1").toScalar();
			scale.z = elemScale->getAttr(NDHS_TP_XML_Z, "1").toScalar();

			// A negative scale is meaningless
			scale.capAtZero();

			foundMask |= EScale;
		}
	}

	//////////////////////////
	// Configure the offset
	if (iMask & EOffset)
	{
		LcCXmlElem* elemOffset = pElem->find(NDHS_TP_XML_OFFSET);
		if (elemOffset)
		{
			centerOffset = LcTVector::createFromXml(elemOffset);

			foundMask |= EOffset;
		}
	}

	/////////////////////////
	// Configure the intensity
	if (iMask & EIntensity)
	{
		LcCXmlAttr* attr = pElem->findAttr(NDHS_TP_XML_INTENSITY);
		if (attr)
		{
			intensity = attr->getVal().toScalar();

			if (intensity < 0)
				intensity = 0;

			foundMask |= EIntensity;
		}
	}

	/////////////////////////
	// Configure the supplementary color
	if (iMask & EColor2)
	{
		LcCXmlAttr* attr = pElem->findAttr(NDHS_TP_XML_COLOR2);

		if (attr)
		{
			color2 = attr->getVal().toInt();
			foundMask |= EColor2;
		}
	}

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

	/////////////////////////
	// Configure effect uniforms
	if (iMask & EUniform)
	{
		LcCXmlElem* effectUniforms = pElem->find(NDHS_TP_XML_EFFECT_UNIFORMS);
		LcTaString name, type;

		if (effect && bPopulateConfigUnimap)
		{
			// Populate the layout uniforms map with configurable interpolatable
			// uniforms known for this effect
			effect->populateMapWithConfigUniforms (&layoutUniMap, true); // set interpolatable flag

			foundMask |= EUniform;
		}

		if(effectUniforms && effect)
		{
			LcCXmlElem* uniform = effectUniforms->getFirstChild();

			for(; uniform; uniform = uniform->getNext())
			{
				if (uniform->getName().compareNoCase(NDHS_TP_XML_UNIFORM) != 0)
					continue;

				name = uniform->getAttr(NDHS_TP_XML_NAME);
				type = uniform->getAttr(NDHS_TP_XML_UNIFORM_TYPE);

				LcTaString	values[4];
				LcTaString	value = "value#";

				// Uniform Values
				LcCXmlElem* child = uniform->find(NDHS_TP_XML_UNIFORM_VALUE);

				if(child)
				{
					for(int i = 0; i < 4; i++)
					{
						value = "value#";
						value.replace('#', ('0' + i+1));
						values[i] = child->getAttr(value);
					}
				}

				foundMask |= EUniform;

				effect->addUniformFromPlacement(&layoutUniMap, name, type, values);
			}
		}
	}

#endif	/* #if defined(IFX_RENDER_DIRECT_OPENGL_20) */

	return (foundMask);
}
#endif

/*-------------------------------------------------------------------------*//**
	If any of the parameters are modified, then the return value is non-zero
	(with the mask bit set for each parameter modified).
*/
LC_EXPORT int LcTPlacement::assign(const LcTPlacement&	p1, int mask)
{
	int retval = 0;

	if (mask & ELocation)
	{
		if (!location.equals(p1.location))
		{
			location = p1.location;
			retval |= ELocation;
		}
	}
	if (mask & EExtent)
	{
		if (!extent.equals(p1.extent))
		{
			extent = p1.extent;
			retval |= EExtent;
		}
	}
	if (mask & EScale)
	{
		if (!scale.equals(p1.scale))
		{
			scale = p1.scale;
			retval |= EScale;
		}
	}

	if (mask & EOrientation)
	{
		if (!orientation.equals(p1.orientation))
		{
			orientation = p1.orientation;
			retval |= EOrientation;
		}
	}

	if (mask & EOffset)
	{
		if (!centerOffset.equals(p1.centerOffset))
		{
			centerOffset = p1.centerOffset;
			retval |= EOffset;
		}
	}

	if (mask & EOpacity)
	{
		if (opacity != p1.opacity)
		{
			opacity = p1.opacity;
			retval |= EOpacity;
		}
	}
	if (mask & EColor)
	{
		if (color != p1.color)
		{
			color = p1.color;
			retval |= EColor;
		}
	}
	if (mask & EFrame)
	{
		if (frame != p1.frame)
		{
			frame = p1.frame;
			retval |= EFrame;
		}
	}

	if (mask & EIntensity)
	{
		if (intensity != p1.intensity)
		{
			intensity = p1.intensity;
			retval |= EIntensity;
		}
	}

	if (mask & EColor2)
	{
		if (color2 != p1.color2)
		{
			color2 = p1.color2;
			retval |= EColor2;
		}
	}

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

	if (mask & EUniform)
	{
		if(populateMap (p1))
			retval |= EUniform;
	}

#endif	/* #if defined(IFX_RENDER_DIRECT_OPENGL_20) */

	return (retval);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcTPlacement::offset(const LcTPlacement& p1, int mask)
{
	if (mask & ELocation)
		location.add(p1.location);

	if (mask & EScale)
		scale.scale(p1.scale);

	if (mask & EOpacity)
		opacity *= p1.opacity;

	if (mask & EExtent)
		extent.add(p1.extent);

	if (mask & EFrame)
		frame += p1.frame;

	if (mask & EOffset)
		centerOffset.add(p1.centerOffset);

	if (mask & LcTPlacement::EColor)
	{
		color.rgba.r = (color.rgba.r * p1.color.rgba.r) >> 8;
		color.rgba.g = (color.rgba.g * p1.color.rgba.g) >> 8;
		color.rgba.b = (color.rgba.b * p1.color.rgba.b) >> 8;
	}

	if (mask & EOrientation)
	{
		// Quaternion rotations are combined by multiplication.
		// Note that here we do A*B not B*A - this ordering defines
		// whether the decoration is applied before the natural orientation.
		// Doing A*B ensures that decoration rotations are applied with
		// respect to the object BEFORE orientation (looks better)
		orientation = LcTQuaternion::multiply(orientation, p1.orientation);
	}

	if (mask & EIntensity)
		intensity *= p1.intensity;

	if (mask & EColor2)
	{
		color2.rgba.r = (color2.rgba.r * p1.color2.rgba.r) >> 8;
		color2.rgba.g = (color2.rgba.g * p1.color2.rgba.g) >> 8;
		color2.rgba.b = (color2.rgba.b * p1.color2.rgba.b) >> 8;
	}

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

	if(mask & EUniform)
	{
		LcTmOwnerMap<LcTmString, LcOglCSLType>::iterator it;
		LcTPlacement *t1 = this;
		LcTPlacement *t2 = (LcTPlacement *)&p1;

		LcTaString name;
		LcOglCSLType *slTypeP1;
		LcOglCSLType *slTypeP2;

		for (it=t2->layoutUniMap.begin(); it != t2->layoutUniMap.end(); it++)
		{
			name = it->first;

			slTypeP1 = t1->layoutUniMap[name];

			// If this uniform is unknown to the base placement, move on
			if (!slTypeP1)
				continue;

			slTypeP2 = it->second;

			// Quick check to prevent accidental confusion
			if (slTypeP1->getSLTypeIdentifier().compareNoCase(slTypeP2->getSLTypeIdentifier()))
				continue;

			//--------------------------------------------------------
			// float
			//--------------------------------------------------------
			if (slTypeP1->getSLTypeIdentifier().compareNoCase("float") == 0)
			{
				float valueA = ((LcOglCSLTypeScalar<float> *)slTypeP1)->getValue();
				float valueB = ((LcOglCSLTypeScalar<float> *)slTypeP2)->getValue();

				float finalValue = valueA + valueB;
				((LcOglCSLTypeScalar<float> *)slTypeP1)->setValue(&finalValue);
			}

			//--------------------------------------------------------
			// vec2
			//--------------------------------------------------------
			else
			if (slTypeP1->getSLTypeIdentifier().compareNoCase("vec2") == 0)
			{
				float valuesA [2] = { 0.0 };
				float valuesB [2] = { 0.0 };
				float finalValues [2] = { 0.0 };

				((LcOglCSLTypeVector<float> *)slTypeP1)->getValue(valuesA);
				((LcOglCSLTypeVector<float> *)slTypeP2)->getValue(valuesB);

				finalValues [0] = valuesA [0] + valuesB [0];
				finalValues [1] = valuesA [1] + valuesB [1];

				((LcOglCSLTypeVector<float> *)slTypeP1)->setValue(finalValues);
			}

			//--------------------------------------------------------
			// vec3
			//--------------------------------------------------------
			else
			if (slTypeP1->getSLTypeIdentifier().compareNoCase("vec3") == 0)
			{
				float valuesA [3] = { 0.0 };
				float valuesB [3] = { 0.0 };
				float finalValues [3] = { 0.0 };

				((LcOglCSLTypeVector<float> *)slTypeP1)->getValue(valuesA);
				((LcOglCSLTypeVector<float> *)slTypeP2)->getValue(valuesB);

				finalValues [0] = valuesA [0] + valuesB [0];
				finalValues [1] = valuesA [1] + valuesB [1];
				finalValues [2] = valuesA [2] + valuesB [2];

				((LcOglCSLTypeVector<float> *)slTypeP1)->setValue(finalValues);
			}

			//--------------------------------------------------------
			// vec4
			//--------------------------------------------------------
			else
			if (slTypeP1->getSLTypeIdentifier().compareNoCase("vec4") == 0)
			{
				float valuesA [4] = { 0.0 };
				float valuesB [4] = { 0.0 };
				float finalValues [4] = { 0.0 };

				((LcOglCSLTypeVector<float> *)slTypeP1)->getValue(valuesA);
				((LcOglCSLTypeVector<float> *)slTypeP2)->getValue(valuesB);

				finalValues [0] = valuesA [0] + valuesB [0];
				finalValues [1] = valuesA [1] + valuesB [1];
				finalValues [2] = valuesA [2] + valuesB [2];
				finalValues [3] = valuesA [3] + valuesB [3];

				((LcOglCSLTypeVector<float> *)slTypeP1)->setValue(finalValues);
			}
		}
	}

#endif	/* #if defined(IFX_RENDER_DIRECT_OPENGL_20) */
}

/*-------------------------------------------------------------------------*//**
	On start of the animation we want the image frame to change immediately,
	following which it the image frame is incremented at evenly spaced intervals.
	Only on the very last animation frame should the last image frame
	be displayed.
*/

LC_EXPORT int LcTPlacement::calcFrameChange(int delta, LcTUnitScalar d)
{
	if (delta == 0)
		return 0;

	int direction = 1;

	if (delta < 0)
		direction = -1;

	return direction + (int)((delta - direction) * d);
}

/*-------------------------------------------------------------------------*//**
	Interpolate between two given placements p1 and p2 according to dBit.
	The interpolated values are put into the current LcTPlacement (this).
	Values are only interpolated if the corresponding bit
	in the given mask is set.  If not, the previous value is left unchanged.
	Interpolation now supports interpolating outside of the 0 to 1 range
*/
LC_EXPORT void LcTPlacement::interpolate(
	const LcTPlacement&	p1,
	const LcTPlacement&	p2,
	int					mask,
	LcTScalar			d)
{
	// Clamp range
	LcTUnitScalar unit0 = LcTUnitScalar(0);
	LcTUnitScalar unit1 = LcTUnitScalar(1);

	// c is used for placement attributes that should
	// not handle interpolation where d < 0 or d > 1
	LcTScalar c = max(unit0, min(unit1, d));

	// Interpolate each element if the mask requests it.
	// Effectively, for each we're doing (in vector form) "v1 + dDist*(v2-v1)"

	if (mask & ELocation)
		location = LcTVector::subtract(p2.location, p1.location).scale(d).add(p1.location);

	if (mask & EExtent)
	{
		extent = LcTVector::subtract(p2.extent, p1.extent).scale(d).add(p1.extent);
	}

	if (mask & EScale)
	{
		scale = LcTVector::subtract(p2.scale, p1.scale).scale(d).add(p1.scale);

		// -ve scale is illegal
		scale.capAtZero();
	}

	if (mask & EOrientation)
		orientation = LcTQuaternion::interpolate(p1.orientation, p2.orientation, d);

	if (mask & EOpacity)
	{
		opacity = p1.opacity + (p2.opacity - p1.opacity) * d;

		// -ve opacity is illegal
		if (opacity < 0)
			opacity = 0;
	}

	if (mask & EColor)
		color = LcTColor::mixColors(p1.color, p2.color, c);

	if (mask & EFrame)
		frame = p1.frame + calcFrameChange(p2.frame-p1.frame, c);

	if (mask & EOffset)
		centerOffset = LcTVector::add(p1.centerOffset, (LcTVector::subtract(p2.centerOffset, p1.centerOffset)).scale(c));

	if (mask & EIntensity)
		intensity = p1.intensity + (p2.intensity - p1.intensity) * d;

	if (mask & EColor2)
		color2 = LcTColor::mixColors(p1.color2, p2.color2, c);

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

	if(mask & EUniform)
	{
		LcTmOwnerMap<LcTmString, LcOglCSLType >::iterator it;
		LcTPlacement *t1 = (LcTPlacement *)&p1;
		LcTPlacement *t2 = (LcTPlacement *)&p2;

		LcTaString name;
		LcOglCSLType *slTypeP1;

		LcOglCSLType *ptr;
		LcTaOwner<LcOglCSLType> temp_ptr;

		int sizeA = p1.layoutUniMap.size();
		int sizeB = p2.layoutUniMap.size();

		if (sizeA == sizeB)
		{
			for(it=t1->layoutUniMap.begin(); it != t1->layoutUniMap.end(); it++)
			{
				name = it->first;
				slTypeP1 = it->second;

				LcOglCSLType *slTypeP2;
				slTypeP2 = t2->layoutUniMap[name];

				if ( (!slTypeP1) || (!slTypeP2) )	continue;

				//--------------------------------------------------------
				// float
				//--------------------------------------------------------
				if (slTypeP1->getSLTypeIdentifier().compareNoCase("float") == 0)
				{
					float valueA = ((LcOglCSLTypeScalar<float> *)slTypeP1)->getValue();
					float valueB = ((LcOglCSLTypeScalar<float> *)slTypeP2)->getValue();

					float finalValue = valueA + (valueB - valueA) * d;
					ptr = this->layoutUniMap[name];

					if (!ptr)
					{
						LcTaOwner< LcOglCSLTypeScalar<float> > temp = LcOglCSLTypeScalar<float>::create();

						temp->setSLType(ELcOglSLTypeScalar);
						temp->setDimension(1);

						LcTaString typ = slTypeP1->getSLTypeIdentifier();

						temp->setSLTypeName(name);
						temp->setSLTypeIdentifier(typ);
						temp->setValue(&finalValue);

						temp_ptr = temp;

						layoutUniMap.add_element(name, temp_ptr);
					}
					else
					{
						((LcOglCSLTypeScalar<float> *)ptr)->setValue (&finalValue);
					}
				}

				//--------------------------------------------------------
				// vec2
				//--------------------------------------------------------
				else if (slTypeP1->getSLTypeIdentifier().compareNoCase("vec2") == 0)
				{
					float valuesA [2] = { 0.0 };
					float valuesB [2] = { 0.0 };
					float finalValues [2] = { 0.0 };

					((LcOglCSLTypeVector<float> *)slTypeP1)->getValue(valuesA);
					((LcOglCSLTypeVector<float> *)slTypeP2)->getValue(valuesB);

					finalValues[0] = valuesA[0] + (valuesB[0] - valuesA[0]) * d;
					finalValues[1] = valuesA[1] + (valuesB[1] - valuesA[1]) * d;

					ptr = this->layoutUniMap[name];

					if (!ptr)
					{
						LcTaOwner< LcOglCSLTypeVector<float> > temp = LcOglCSLTypeVector<float>::create(2);

						temp->setSLType(ELcOglSLTypeVector);
						temp->setDimension(2);

						LcTaString typ = slTypeP1->getSLTypeIdentifier();

						temp->setSLTypeName(name);
						temp->setSLTypeIdentifier(typ);
						temp->setValue(finalValues);

						temp_ptr = temp;

						layoutUniMap.add_element(name, temp_ptr);
					}
					else
					{
						((LcOglCSLTypeVector<float> *)ptr)->setValue (finalValues);
					}
				}

				//--------------------------------------------------------
				// vec3
				//--------------------------------------------------------
				else if (slTypeP1->getSLTypeIdentifier().compareNoCase("vec3") == 0)
				{
					float valuesA [3] = { 0.0 };
					float valuesB [3] = { 0.0 };
					float finalValues [3] = { 0.0 };

					((LcOglCSLTypeVector<float> *)slTypeP1)->getValue(valuesA);
					((LcOglCSLTypeVector<float> *)slTypeP2)->getValue(valuesB);

					finalValues[0] = valuesA[0] + (valuesB[0] - valuesA[0]) * d;
					finalValues[1] = valuesA[1] + (valuesB[1] - valuesA[1]) * d;
					finalValues[2] = valuesA[2] + (valuesB[2] - valuesA[2]) * d;

					ptr = this->layoutUniMap[name];

					if (!ptr)
					{
						LcTaOwner< LcOglCSLTypeVector<float> > temp = LcOglCSLTypeVector<float>::create(3);

						temp->setSLType(ELcOglSLTypeVector);
						temp->setDimension(3);

						LcTaString typ = slTypeP1->getSLTypeIdentifier();

						temp->setSLTypeName(name);
						temp->setSLTypeIdentifier(typ);
						temp->setValue(finalValues);

						temp_ptr = temp;

						layoutUniMap.add_element(name, temp_ptr);
					}
					else
					{
						((LcOglCSLTypeVector<float> *)ptr)->setValue (finalValues);
					}
				}

				//--------------------------------------------------------
				// vec4
				//--------------------------------------------------------
				else if (slTypeP1->getSLTypeIdentifier().compareNoCase("vec4") == 0)
				{
					float valuesA [4] = { 0.0 };
					float valuesB [4] = { 0.0 };
					float finalValues [4] = { 0.0 };

					((LcOglCSLTypeVector<float> *)slTypeP1)->getValue(valuesA);
					((LcOglCSLTypeVector<float> *)slTypeP2)->getValue(valuesB);

					finalValues[0] = valuesA[0] + (valuesB[0] - valuesA[0]) * d;
					finalValues[1] = valuesA[1] + (valuesB[1] - valuesA[1]) * d;
					finalValues[2] = valuesA[2] + (valuesB[2] - valuesA[2]) * d;
					finalValues[3] = valuesA[3] + (valuesB[3] - valuesA[3]) * d;

					ptr = this->layoutUniMap[name];

					if (!ptr)
					{
						LcTaOwner< LcOglCSLTypeVector<float> > temp = LcOglCSLTypeVector<float>::create(4);

						temp->setSLType(ELcOglSLTypeVector);
						temp->setDimension(4);

						LcTaString typ = slTypeP1->getSLTypeIdentifier();

						temp->setSLTypeName(name);
						temp->setSLTypeIdentifier(typ);
						temp->setValue(finalValues);

						temp_ptr = temp;

						this->layoutUniMap.add_element(name, temp_ptr);
					}
					else
					{
						((LcOglCSLTypeVector<float> *)ptr)->setValue (finalValues);
					}
				}
			}
		}
	}

#endif	/* #if defined(IFX_RENDER_DIRECT_OPENGL_20) */

}
#ifdef IFX_SERIALIZATION
LcTPlacement* LcTPlacement::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	LcTPlacement * placement=new LcTPlacement();
	placement->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,placement);
	return placement;
}
SerializeHandle	LcTPlacement::serialize(LcCSerializeMaster *serializeMaster,bool force)
{
	SerializeHandle handle=-1;
	if(!force)
	{
		handle=serializeMaster->getHandle(this);
		if(handle!=-1 && serializeMaster->isSerialized(handle))
		{
			return handle;
		}
		else if(handle==-1)
		{
			handle=serializeMaster->newHandle(this);
		}
	}
	else
	{
		handle=serializeMaster->newHandle(this);
	}

	int outputSize = sizeof(LcTPlacement)+sizeof(IFX_INT32);
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;

	SERIALIZE(location,serializeMaster,cPtr)
	SERIALIZE(extent,serializeMaster,cPtr)
	SERIALIZE(centerOffset,serializeMaster,cPtr)
	SERIALIZE(scale,serializeMaster,cPtr)
	SERIALIZE(orientation,serializeMaster,cPtr)
	SERIALIZE(opacity,serializeMaster,cPtr)
	SERIALIZE(color,serializeMaster,cPtr)
	SERIALIZE(frame,serializeMaster,cPtr)
	SERIALIZE(intensity,serializeMaster,cPtr)
	SERIALIZE(color2,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	LcTPlacement::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);

	if(cPtr==NULL) return;

	DESERIALIZE(location,serializeMaster,cPtr)
	DESERIALIZE(extent,serializeMaster,cPtr)
	DESERIALIZE(centerOffset,serializeMaster,cPtr)
	DESERIALIZE(scale,serializeMaster,cPtr)
	DESERIALIZE(orientation,serializeMaster,cPtr)
	DESERIALIZE(opacity,serializeMaster,cPtr)
	DESERIALIZE(color,serializeMaster,cPtr)
	DESERIALIZE(frame,serializeMaster,cPtr)
	DESERIALIZE(intensity,serializeMaster,cPtr)
	DESERIALIZE(color2,serializeMaster,cPtr)
}
#endif /* IFX_SERIALIZATION */

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

LC_IMPORT bool LcTPlacement::populateMap(const LcTPlacement &p)
{
	bool    isPopulate = false;
	LcTmOwnerMap<LcTmString, LcOglCSLType>::iterator it;
	LcTPlacement *temp = (LcTPlacement *)&p;

	int sizeA = this->layoutUniMap.size();
	int sizeB = temp->layoutUniMap.size();

	if(sizeA == sizeB)
	{
		for(it=temp->layoutUniMap.begin(); it != temp->layoutUniMap.end(); it++)
		{
			LcOglCSLType *slType = it->second;

			if (!slType) continue;

			isPopulate = true;

			//----------------------------------------
			// float
			//----------------------------------------
			if (slType->getSLTypeIdentifier().compareNoCase("float") == 0)
			{
				float value = ((LcOglCSLTypeScalar<float> *)it->second)->getValue();

				LcOglCSLTypeScalar<float> *t = (LcOglCSLTypeScalar<float> *)(this->layoutUniMap[it->first]);
				t->setValue(&value);
			}

			//----------------------------------------
			// vec2
			//----------------------------------------
			else if (slType->getSLTypeIdentifier().compareNoCase("vec2") == 0)
			{
				float values[2];

				((LcOglCSLTypeVector<float> *)it->second)->getValue(values);

				LcOglCSLTypeVector<float> *t = (LcOglCSLTypeVector<float> *)(this->layoutUniMap[it->first]);
				t->setValue(values);
			}

			//----------------------------------------
			// vec3
			//----------------------------------------
			else if (slType->getSLTypeIdentifier().compareNoCase("vec3") == 0)
			{
				float values[3];

				((LcOglCSLTypeVector<float> *)it->second)->getValue(values);

				LcOglCSLTypeVector<float> *t = (LcOglCSLTypeVector<float> *)(this->layoutUniMap[it->first]);
				t->setValue(values);
			}

			//----------------------------------------
			// vec4
			//----------------------------------------
			else if (slType->getSLTypeIdentifier().compareNoCase("vec4") == 0)
			{
				float values[4];

				((LcOglCSLTypeVector<float> *)it->second)->getValue(values);
				LcOglCSLTypeVector<float> *t = (LcOglCSLTypeVector<float> *)(this->layoutUniMap[it->first]);
				t->setValue(values);
			}
		}
	}
	else // Maps are not equal on both sides
	{
		for(it=temp->layoutUniMap.begin(); it != temp->layoutUniMap.end(); it++)
		{
			LcOglCSLType *slType = it->second;

			if (!slType) continue;

			isPopulate = true;

			//----------------------------------------
			// float
			//----------------------------------------
			if (slType->getSLTypeIdentifier().compareNoCase("float") == 0)
			{
				float value = ((LcOglCSLTypeScalar<float> *)it->second)->getValue();

				LcTaOwner< LcOglCSLTypeScalar<float> > temp = LcOglCSLTypeScalar<float>::create();

				temp->setSLType(ELcOglSLTypeScalar);
				temp->setSLTypeName(it->first);
				temp->setDimension(1);
				temp->setValue(&value);

				LcTaString type = ((LcOglCSLType*)it->second)->getSLTypeIdentifier();
				temp->setSLTypeIdentifier(type);

				slTypeF = temp;

				// Add to map
				layoutUniMap.add_element(it->first, slTypeF);
			}

			//----------------------------------------
			// vec2
			//----------------------------------------
			else if (slType->getSLTypeIdentifier().compareNoCase("vec2") == 0)
			{
				float values[2];

				((LcOglCSLTypeVector<float> *)it->second)->getValue(values);

				LcTaOwner< LcOglCSLTypeVector<float> > temp = LcOglCSLTypeVector<float>::create(2);

				temp->setSLType(ELcOglSLTypeVector);
				temp->setSLTypeName(it->first);
				temp->setDimension(2);
				temp->setValue(values);

				LcTaString type = ((LcOglCSLType*)it->second)->getSLTypeIdentifier();
				temp->setSLTypeIdentifier(type);

				slTypeF = temp;

				// Add to map
				layoutUniMap.add_element(it->first, slTypeF);
			}

			//----------------------------------------
			// vec3
			//----------------------------------------
			else if (slType->getSLTypeIdentifier().compareNoCase("vec3") == 0)
			{
				float values[3];

				((LcOglCSLTypeVector<float> *)it->second)->getValue(values);

				LcTaOwner< LcOglCSLTypeVector<float> > temp = LcOglCSLTypeVector<float>::create(3);

				temp->setSLType(ELcOglSLTypeVector);
				temp->setSLTypeName(it->first);
				temp->setDimension(3);
				temp->setValue(values);

				LcTaString type = ((LcOglCSLType*)it->second)->getSLTypeIdentifier();
				temp->setSLTypeIdentifier(type);

				slTypeF = temp;

				// Add to map
				layoutUniMap.add_element(it->first, slTypeF);
			}

			//----------------------------------------
			// vec4
			//----------------------------------------
			else if (slType->getSLTypeIdentifier().compareNoCase("vec4") == 0)
			{
				float values[4];

				((LcOglCSLTypeVector<float> *)it->second)->getValue(values);

				LcTaOwner< LcOglCSLTypeVector<float> > temp = LcOglCSLTypeVector<float>::create(4);

				temp->setSLType(ELcOglSLTypeVector);
				temp->setSLTypeName(it->first);
				temp->setDimension(4);
				temp->setValue(values);

				LcTaString type = ((LcOglCSLType*)it->second)->getSLTypeIdentifier();
				temp->setSLTypeIdentifier(type);

				slTypeF = temp;

				// Add to map
				layoutUniMap.add_element(it->first, slTypeF);
			}
		}
	}
	return (isPopulate);
}

#endif	/* #if defined(IFX_RENDER_DIRECT_OPENGL_20) */
