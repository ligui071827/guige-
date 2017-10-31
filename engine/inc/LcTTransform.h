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
#ifndef LcTTransformH
#define LcTTransformH

#include "inflexionui/engine/inc/LcTVector.h"
class LcTTransform4;

#ifdef LC_PLAT_OGL
	#include "inflexionui/engine/inc/LcOglCContext.h"
#endif

/*-------------------------------------------------------------------------*//**
	Performs the standard matrix transformations on a 3 * 4 matrix
*/
class LcTTransform
{
	friend class LcTTransform4;

private:

	// Bottom row is always 0,0,0,1 so optimize away
	LcTScalar							m_ad[12];

public:

	// Construction
	LC_IMPORT							LcTTransform();

	// Methods
	LC_IMPORT			LcTTransform&	setFromRowMajorMatrixArray(LcTScalar *matrixArray);
	LC_IMPORT			LcTTransform&	scale(LcTScalar dX, LcTScalar dY, LcTScalar dZ);
	LC_IMPORT			LcTTransform&	translate(LcTScalar dX, LcTScalar dY, LcTScalar dZ);
	LC_IMPORT			LcTTransform&	translatePrefix(LcTScalar dX, LcTScalar dY, LcTScalar dZ);
	LC_IMPORT			LcTTransform&	multiply(const LcTTransform& t);
	LC_IMPORT			LcTTransform&	invert();
	LC_IMPORT			LcTTransform&	transpose();
	LC_IMPORT			LcTVector		transform(const LcTVector& v) const;

	// Wrappers
	inline				LcTTransform&	scale(const LcTVector& v)			
											{ return scale(v.x, v.y, v.z); }
	inline				LcTTransform&	translate(const LcTVector& v)		
											{ return translate(v.x, v.y, v.z); }
	inline				LcTTransform&	translatePrefix(const LcTVector& v)	
											{ return translatePrefix(v.x, v.y, v.z); }

	// For creating specific types of transform
	LC_IMPORT static	LcTTransform	identity();
	LC_IMPORT static	LcTTransform	rotate(const LcTQuaternion& quat);	

	// Extract data as OpenGL column-major array
#ifdef LC_PLAT_OGL
	LC_IMPORT			void			getData(LcOglTScalar* f) const;
#endif
};

#endif // LcTTransformH
