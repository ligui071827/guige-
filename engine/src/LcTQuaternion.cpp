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
	Note that quaternions are kept in normalized form so this function
	is for internal use only
*/
LC_EXPORT LcTQuaternion& LcTQuaternion::normalise()
{
	LcTScalar mag = (w * w) + (x * x) + (y * y) + (z * z);
	if (mag == 0)
	{
		// If axis is zero, normalize by setting angle to zero
		w = 1;
	}
	else if (mag != 1)
	{
		// Scale so that magnitude becomes 1
		mag	= 1 / sqrt(mag);

		w *= mag;
		x *= mag;
		y *= mag;
		z *= mag;
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcTQuaternion::equals(const LcTQuaternion& q) const
{
	return (w == q.w) && (x == q.x) && (y == q.y) && (z == q.z);
}

/*-------------------------------------------------------------------------*//**
	Multiplies this quaterion by the given one (not commutative).
	Returns this.  Note that when multiplying to combine rotations,
	A*B is equivalent to rotating by B then A.
*/
LC_EXPORT LcTQuaternion& LcTQuaternion::multiply(const LcTQuaternion& q)
{
	LcTScalar tw = (w * q.w) - (x * q.x) - (y * q.y) - (z * q.z);
	LcTScalar tx = (w * q.x) + (x * q.w) + (y * q.z) - (z * q.y);
	LcTScalar ty = (w * q.y) - (x * q.z) + (y * q.w) + (z * q.x);
	LcTScalar tz = (w * q.z) + (x * q.y) - (y * q.x) + (z * q.w);

	w = tw;
	x = tx;
	y = ty;
	z = tz;

	return normalise();
}

/*-------------------------------------------------------------------------*//**
	Compares the quaternion to the given one, and if the angle between
	them is >180 degrees, invert to bring it <180
*/
LC_EXPORT LcTQuaternion& LcTQuaternion::shortestPath(const LcTQuaternion& q)
{
	LcTScalar cosHalfTheta = (w * q.w) + (x * q.x) + (y * q.y) + (z * q.z);
	if (cosHalfTheta < 0)
	{
		// Theta > 180, so invert the quaternion to follow shortest path
		w = -w;
		x = -x;
		y = -y;
		z = -z;
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
	Linearly interpolate between this quaternion and the given quaternion,
	according to the parameter d (in the range 0-1).
*/
LC_EXPORT LcTQuaternion& LcTQuaternion::interpolate(const LcTQuaternion& q, LcTUnitScalar d)
{
	LcTScalar fCosHalfA		= (w * q.w) + (x * q.x) + (y * q.y) + (z * q.z);
	LcTScalar fSinHalfA		= 0.0;
	LcTScalar fHalfA		= 0.0;

	if( fCosHalfA < 1.0)
	{
		fSinHalfA	= (LcTScalar)lc_sqrt(1.0f - fCosHalfA * fCosHalfA);
		fHalfA		= (LcTScalar)lc_acos(fCosHalfA);
	}

	// Avoid div-by-zero: occurs at angle = 0 or 360 so quaternions
	// are actually the same anyway
	if (lc_fabs(fSinHalfA) > 0.001)
	{
		// SLERP
		LcTScalar fSin1oHalfA	= 1 / fSinHalfA;
		LcTScalar weightA		= (LcTScalar)lc_sin((1 - d) * fHalfA) * fSin1oHalfA;
		LcTScalar weightB		= (LcTScalar)lc_sin(     d  * fHalfA) * fSin1oHalfA;

		// Now interpolate each field
		w						= (w * weightA) + (q.w * weightB);
		x						= (x * weightA) + (q.x * weightB);
		y						= (y * weightA) + (q.y * weightB);
		z						= (z * weightA) + (q.z * weightB);

		// Always keep quaternions normalized
		normalise();
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
	Calculate the quaternion corresponding to the given axis and angle.
	Angle must be in radians
*/
LC_EXPORT LcTQuaternion LcTQuaternion::rotateAxisAngle(
	const LcTVector& vAxis,
	LcTScalar dAngle)
{
	LcTQuaternion q;

	dAngle /= 2;

	LcTScalar fSinA		= (LcTScalar)lc_sin(dAngle);
	LcTVector vAxisN	= LcTVector::normalise(vAxis);

	q.w					= (LcTScalar)lc_cos(dAngle);
	q.x					= vAxisN.x * fSinA;
	q.y					= vAxisN.y * fSinA;
	q.z					= vAxisN.z * fSinA;

	// Always keep quaternions normalized
	return q.normalise();
}

/*-------------------------------------------------------------------------*//**
	Calculate the quaternion corresponding to the three given Euler angles,
	which must be in radians
*/
LC_EXPORT LcTQuaternion LcTQuaternion::rotateEuler(
	LcTScalar azimuth,
	LcTScalar elevation,
	LcTScalar roll)
{
	LcTQuaternion q;

	// NB: we are also inverting so that angles are applied anti-clockwise
	azimuth				= -(azimuth / 2);
	elevation			= -(elevation / 2);
	roll				= -(roll / 2);

	LcTScalar fSinA		= (LcTScalar)lc_sin(azimuth);
	LcTScalar fCosA		= (LcTScalar)lc_cos(azimuth);

	LcTScalar fSinE		= (LcTScalar)lc_sin(elevation);
	LcTScalar fCosE		= (LcTScalar)lc_cos(elevation);

	LcTScalar fSinR		= (LcTScalar)lc_sin(roll);
	LcTScalar fCosR		= (LcTScalar)lc_cos(roll);

	// Some of these multiplications could be optimized away
	q.w					= (fCosA * fCosR * fCosE) - (fSinA * fSinR * fSinE);
	q.x					= (fSinA * fSinR * fCosE) + (fCosA * fCosR * fSinE);
	q.y					= (fSinA * fCosR * fCosE) + (fCosA * fSinR * fSinE);
	q.z					= (fCosA * fSinR * fCosE) - (fSinA * fCosR * fSinE);

	// Always keep quaternions normalized
	return q.normalise();
}

#ifdef LC_USE_XML
/*-------------------------------------------------------------------------*//**
	Creates a quaternion from an XML configuration element. The element should
	contain either three attributes called "azimuth", "elevation", and "roll",
	or an axis and angle in the form of four attributes "x", "y", "z" and "angle".
*/
LC_EXPORT LcTQuaternion LcTQuaternion::createFromXml(LcCXmlElem* pXml)
{
	LcTQuaternion q;

	if (pXml->getName() == NDHS_TP_XML_ORIENTATION)
	{
		LcTScalar azimuth	= pXml->getAttr(NDHS_TP_XML_AZIMUTH, "0").toScalar();
		LcTScalar elevation	= pXml->getAttr(NDHS_TP_XML_ELEVATION, "0").toScalar();
		LcTScalar roll		= pXml->getAttr(NDHS_TP_XML_ROLL, "0").toScalar();

		// Note that angles in XML are in degrees - convert on first reading
		q = rotateEuler(
				LC_RADIANS(azimuth),
				LC_RADIANS(elevation),
				LC_RADIANS(roll));
	}
	else if (pXml->getName() == NDHS_TP_XML_ROTATION)
	{
		LcTVector axis		= LcTVector::createFromXml(pXml);
		LcTScalar angle		= pXml->getAttr(NDHS_TP_XML_ANGLE, "0").toScalar();

		// Cap angle just below 360 because users will expect 360 to give a
		// full rotation, but we can't use 360 because we will lose the axis
		angle				= max(-359.5f, min(angle, 359.5f));

		// Note that angles in XML are in degrees - convert on first reading
		q = rotateAxisAngle(
				axis,
				LC_RADIANS(angle));
	}

	return q;
}
#endif
