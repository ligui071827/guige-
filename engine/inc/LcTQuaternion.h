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
#ifndef LcTQuaternionH
#define LcTQuaternionH

#include "inflexionui/engine/inc/LcDefs.h"

/*-------------------------------------------------------------------------*//**
	Encapsulates a quaternion and operations on and between quaternions
*/
class LcTQuaternion
{
public:

	// Public access; no encapsulation!
	LcTScalar							w;
	LcTScalar							x;
	LcTScalar							y;
	LcTScalar							z;

	// Construction
	inline								LcTQuaternion()												
											{ w = 1; x = 0; y = 0; z = 0; }
	inline								LcTQuaternion(LcTScalar dW, LcTScalar dX, LcTScalar dY, LcTScalar dZ)		
											{ w = dW; x = dX; y = dY; z = dZ; normalise(); }

	// Methods
	LC_IMPORT			LcTQuaternion&	normalise();
	inline				LcTQuaternion&	invert()	{ x = -x; y = -y; z = -z; return *this; }
	inline				bool			isZero()	const { return w == 1; }
	inline				bool			isInXY()	const { return (x == 0) && (y == 0); }
	LC_IMPORT			bool			equals(const LcTQuaternion& v) const;
	LC_IMPORT			LcTQuaternion&	multiply(const LcTQuaternion& q2);
	LC_IMPORT			LcTQuaternion&	shortestPath(const LcTQuaternion& q);
	LC_IMPORT			LcTQuaternion&	interpolate(const LcTQuaternion& q2, LcTUnitScalar d);

	// For creating quaternions for 3D rotations
	LC_IMPORT static	LcTQuaternion	rotateAxisAngle(
											const LcTVector& vAxis, 
											LcTScalar dAngle);	
	LC_IMPORT static	LcTQuaternion	rotateEuler(
											LcTScalar azimuth,
											LcTScalar elevation,
											LcTScalar roll);

	// Wrappers that don't change operand
	inline static		LcTQuaternion	multiply(const LcTQuaternion& q1, const LcTQuaternion& q2)		
											{ return LcTQuaternion(q1).multiply(q2); }
	inline static		LcTQuaternion	interpolate(const LcTQuaternion& q1, const LcTQuaternion& q2, LcTScalar d)	
											{ return LcTQuaternion(q1).interpolate(q2, d); }
	inline static		LcTQuaternion	normalise(const LcTQuaternion& q1)						
											{ return LcTQuaternion(q1).normalise(); }
	inline static		LcTQuaternion	invert(const LcTQuaternion& q1)						
											{ return LcTQuaternion(q1).invert(); }

	// XML encoding
#ifdef LC_USE_XML
	LC_IMPORT static	LcTQuaternion	createFromXml(LcCXmlElem* pXml);
#endif
};

#endif // LcTQuaternionH
