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
LC_EXPORT LcTVector::LcTVector(LcTScalar* aXYZ)
{
	x = aXYZ[0];
	y = aXYZ[1];
	z = aXYZ[2];
}

/*-------------------------------------------------------------------------*//**
	Test whether the magnitude of a vector is zero.
*/
LC_EXPORT bool LcTVector::isZero() const
{
	return (x == 0) && (y == 0) && (z == 0);
}

/*-------------------------------------------------------------------------*//**
	Returns the magnitude of the vector.
*/
LC_EXPORT LcTScalar LcTVector::magnitude() const
{
	return (LcTScalar)sqrt((x * x) + (y * y) + (z * z));
}

/*-------------------------------------------------------------------------*//**
	Adds the given vector to this one.
	Returns this.
*/
LC_EXPORT LcTVector& LcTVector::add(const LcTVector& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

/*-------------------------------------------------------------------------*//**
	Subtracts the given vector from this one.
	Returns this.
*/
LC_EXPORT LcTVector& LcTVector::subtract(const LcTVector& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

/*-------------------------------------------------------------------------*//**
	Scales the vector by the given factor, preserving its direction.
	Returns this.
*/
LC_EXPORT LcTVector& LcTVector::scale(LcTScalar d)
{
	x *= d;
	y *= d;
	z *= d;
	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTVector& LcTVector::scale(const LcTVector& v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

/*-------------------------------------------------------------------------*//**
	Scales the vector so that it has the given magnitude, preserving its direction.
	Returns this.
*/
LC_EXPORT LcTVector& LcTVector::setMagnitude(LcTScalar d)
{
	// When we have look-up table math implementations, avoid a division on
	// setting mag by having a 1/sqrt lookup.  This would return 1/mag directly
	// so we can more efficiently derive d/mag
	LcTScalar dM = magnitude();
	if (dM > 0 && dM != d)
		scale(d / dM);

	return *this;
}

/*-------------------------------------------------------------------------*//**
	Scales the vector so that it has magnitude 1.0, preserving its direction.
	Returns this.
*/
LC_EXPORT LcTVector& LcTVector::normalise()
{
	// When we have look-up table math implementations, avoid a division on
	// normalization by having a 1/sqrt lookup.  This would return 1/mag directly
	// and we can multiply each component by this
	setMagnitude(1);
	return *this;
}

/*-------------------------------------------------------------------------*//**
	Sets the vector to be the cross product of its previous value and the given vector.
	Returns this.
*/
LC_EXPORT LcTVector& LcTVector::cross(const LcTVector& v)
{
	LcTScalar dX = (y * v.z) - (z * v.y);
	LcTScalar dY = (z * v.x) - (x * v.z);

	z = (x * v.y) - (y * v.x);
	x = dX;
	y = dY;
	return *this;
}

/*-------------------------------------------------------------------------*//**
	Returns the dot product of this vector and the given vector.
*/
LC_EXPORT LcTScalar LcTVector::dot(const LcTVector& v) const
{
	return (x * v.x) + (y * v.y) + (z * v.z);
}

/*-------------------------------------------------------------------------*//**
	Return the angle between this vector and the given vector,
	assuming they are normalized
*/
LC_EXPORT LcTScalar LcTVector::angle(const LcTVector& vP2) const
{
	LcTScalar dDot = dot(vP2);
	return (dDot >= 1 ? LcTScalar(0) : (dDot <= -1 ? (LcTScalar)LC_PI : (LcTScalar)lc_acos(dDot)));
}

/*-------------------------------------------------------------------------*//**
	Returns true if the given vector is equal to this one.
*/
LC_EXPORT bool LcTVector::equals(const LcTVector& v) const
{
	return (x == v.x) && (y == v.y) && (z == v.z);
}

/*-------------------------------------------------------------------------*//**
	Return a new vector which is the component of this vector tangential
	to the given vector (which must be normalized).
*/
LC_EXPORT LcTVector LcTVector::getTangent(const LcTVector& vr) const
{
	return scale(vr, dot(vr));
}

/*-------------------------------------------------------------------------*//**
	Return a new vector which is the component of this vector normal
	to the given vector (which must be normalized).
*/
LC_EXPORT LcTVector LcTVector::getNormal(const LcTVector& vr) const
{
	return subtract(*this, getTangent(vr));
}

/*-------------------------------------------------------------------------*//**
	If any component is less than zero, it will set it to zero
*/
LC_EXPORT LcTVector& LcTVector::capAtZero()
{
	if (x < 0)
		x = 0;

	if (y < 0)
		y = 0;

	if (z < 0)
		z = 0;

	return *this;
}

#ifdef LC_USE_XML
/*-------------------------------------------------------------------------*//**
	Creates a vector from an XML configuration element. The element should
	contain three attributes called "x", "y", and "z".
	Returns a new vector containing the values specified in the XML. if any
	value is unspecified in the XML, it is set to 0.
*/
LC_EXPORT LcTVector LcTVector::createFromXml(LcCXmlElem* pXml)
{
	LcCXmlAttr* attr;
	LcTVector vector;

	vector.x = vector.y = vector.z = 0;

	attr = pXml->findAttr("x");
	if (attr)
		vector.x = attr->getVal().toScalar();

	attr = pXml->findAttr("y");
	if (attr)
		vector.y = attr->getVal().toScalar();

	attr = pXml->findAttr("z");
	if (attr)
		vector.z = attr->getVal().toScalar();

	return vector;
}
#endif
