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
#ifndef LcTTransform4H
#define LcTTransform4H

#include "inflexionui/engine/inc/LcTVector.h"
class LcTTransform;

#ifdef LC_PLAT_OGL
	#include "inflexionui/engine/inc/LcOglCContext.h"
#endif

/*-------------------------------------------------------------------------*//**
	Performs standard matrix transformations on a 4*4 matrix
*/
class LcTTransform4
{
	friend class LcTTransform;

private:

	LcTScalar							m_ad[16];
	LcTScalar							m_maxW;

public:

	// Construction
	LC_IMPORT							LcTTransform4();
	LC_IMPORT							LcTTransform4(const LcTTransform&);

	// Methods
	LC_IMPORT			LcTVector		transformIgnoreZ(const LcTVector& v) const;
	LC_IMPORT			LcTVector		transformWithZ(const LcTVector& v) const;
	LC_IMPORT			LcTTransform4&	multiply(const LcTTransform4& t);
	inline				LcTTransform4&	multiply(const LcTTransform& t)
											{ return multiply(LcTTransform4(t)); }

	// For creating specific types of transform
	LC_IMPORT static	LcTTransform4	identity();
	LC_IMPORT static	LcTTransform4	frustum(
											LcTScalar dLeft, LcTScalar dRight,
											LcTScalar dBottom, LcTScalar dTop,
											LcTScalar dNear, LcTScalar dFar);
	LC_IMPORT static	LcTTransform4	canvas(
											LcTScalar dXo, LcTScalar dYo, 
											LcTScalar dEyeCanvas, LcTScalar dEyeGlobal);

	// Extract data as OpenGL column-major array
#ifdef LC_PLAT_OGL
	LC_IMPORT			void			getData(LcOglTScalar* f) const;
#endif
};

#endif // LcTTransform4H

