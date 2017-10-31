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
#ifndef LcTVectorH
#define LcTVectorH

#include "inflexionui/engine/inc/LcDefs.h"
class LcTTransform;

#ifdef LC_USE_XML
	class LcCXmlElem;
#endif

/*-------------------------------------------------------------------------*//**
	Encapsulates a vector and operations on and between vectors
*/
class LcTVector
{
public:

	// Public access; no encapsulation!
	LcTScalar						x;
	LcTScalar						y;
	LcTScalar						z;

	// Construction
	inline							LcTVector()												
										{ x = 0;  y = 0;  z = 0; }
	inline							LcTVector(LcTScalar dX, LcTScalar dY, LcTScalar dZ)		
										{ x = dX; y = dY; z = dZ; }
	LC_IMPORT						LcTVector(LcTScalar* aXYZ);

	// Methods
	LC_IMPORT			bool		isZero() const;
	LC_IMPORT			LcTScalar	magnitude() const;
	LC_IMPORT			LcTVector&	add(const LcTVector& v);
	LC_IMPORT			LcTVector&	subtract(const LcTVector& v);
	LC_IMPORT			LcTVector&	scale(LcTScalar d);
	LC_IMPORT			LcTVector&	scale(const LcTVector& v);
	LC_IMPORT			LcTVector&	setMagnitude(LcTScalar d);
	LC_IMPORT			LcTVector&	normalise();
	LC_IMPORT			LcTVector&	cross(const LcTVector& v);
	LC_IMPORT			LcTScalar	dot(const LcTVector& v) const;
	LC_IMPORT			LcTScalar	angle(const LcTVector& vP2) const;
	LC_IMPORT			bool		equals(const LcTVector& v) const;
	LC_IMPORT			LcTVector	getTangent(const LcTVector& vr) const;
	LC_IMPORT			LcTVector	getNormal(const LcTVector& vr) const;
	LC_IMPORT			LcTVector&	capAtZero();

	// Wrappers that don't change operand
	inline static		LcTVector	add(const LcTVector& v1, const LcTVector& v2)		
										{ return LcTVector(v1).add(v2); }
	inline static		LcTVector	subtract(const LcTVector& v1, const LcTVector& v2)	
										{ return LcTVector(v1).subtract(v2); }
	inline static		LcTVector	scale(const LcTVector& v1, LcTScalar d)				
										{ return LcTVector(v1).scale(d); }
	inline static		LcTVector	scale(const LcTVector& v1, const LcTVector& v2)		
										{ return LcTVector(v1).scale(v2); }
	inline static		LcTVector	normalise(const LcTVector& v1)						
										{ return LcTVector(v1).normalise(); }
	inline static		LcTVector	capAtZero(const LcTVector& v1)						
										{ return LcTVector(v1).capAtZero(); }

	// XML encoding
#ifdef LC_USE_XML
	LC_IMPORT static	LcTVector	createFromXml(LcCXmlElem* pXml);
#endif

};

#endif // LcTVectorH

