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

// Column-major implementation in line with OpenGL
#define INDEX(r,c)  ((c) * 4 + (r))
#define INDEX3(r,c) ((c) * 3 + (r))


/*-------------------------------------------------------------------------*//**
	Constructs an empty matrix.
*/
LC_EXPORT LcTTransform4::LcTTransform4()
{
	for (int i = 0; i < 16; i++)
		m_ad[i] = 0;
}

/*-------------------------------------------------------------------------*//**
	Copies a linear matrix.
*/
LC_EXPORT LcTTransform4::LcTTransform4(const LcTTransform& t)
{
	// Copy top 3 rows
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 4; j++)
			m_ad[INDEX(i,j)] = t.m_ad[INDEX3(i,j)];

	// Set bottom row
	m_ad[INDEX(3,0)] = 0;
	m_ad[INDEX(3,1)] = 0;
	m_ad[INDEX(3,2)] = 0;
	m_ad[INDEX(3,3)] = 1;
}

/*-------------------------------------------------------------------------*//**
	Returns data as column-major array (as used by OpenGL)
*/
#ifdef LC_PLAT_OGL
LC_EXPORT void LcTTransform4::getData(LcOglTScalar* f) const
{
	for (int i = 0; i < 16; i++)
		f[i] = LC_OGL_FROM_SCALAR(m_ad[i]);
}
#endif

/*-------------------------------------------------------------------------*//**
	Constructs and returns an identity matrix.
*/
LC_EXPORT LcTTransform4 LcTTransform4::identity()
{
	// Others should default to zero
	LcTTransform4 t;
	t.m_ad[INDEX(0,0)]	= 1;
	t.m_ad[INDEX(1,1)]	= 1;
	t.m_ad[INDEX(2,2)]	= 1;
	t.m_ad[INDEX(3,3)]	= 1;
	return t;
}

/*-------------------------------------------------------------------------*//**
	Constructs and returns a transformation to the frustum specified
*/
LC_EXPORT LcTTransform4 LcTTransform4::frustum(
	LcTScalar dL, LcTScalar dR, LcTScalar dB, LcTScalar dT, LcTScalar dN, LcTScalar dF)
{
	// Note that fixed-point scalars may not have enough precision for 14th term
	LcTTransform4 t;
	t.m_ad[INDEX(0,0)]	= (2 * dN) / (dR - dL);
	t.m_ad[INDEX(0,2)]	= (dR + dL) / (dR - dL);
	t.m_ad[INDEX(1,1)]	= (2 * dN) / (dT - dB);
	t.m_ad[INDEX(1,2)]	= (dT + dB) / (dT - dB);
	t.m_ad[INDEX(2,2)]	= - (dF + dN) / (dF - dN);
	t.m_ad[INDEX(2,3)]	= - 2 * float(dF) * float(dN) / float(dF - dN);
	t.m_ad[INDEX(3,2)]	= -1;
	return t;
}

/*-------------------------------------------------------------------------*//**
	Return a projection to simple X, Y canvas coordinates.  This implements
	perspective and scaling but does not normalize the coordinates, and does
	not cater for mapping Z
*/
LC_EXPORT LcTTransform4 LcTTransform4::canvas(
	LcTScalar dXo, LcTScalar dYo, LcTScalar dEyeCanvas, LcTScalar dEyeGlobal)
{
	// For floats, we are not bothered about overflow
	LcTTransform4 t;
	t.m_ad[INDEX(0,0)]	= -dEyeCanvas;
	t.m_ad[INDEX(0,2)]	= dXo;
	t.m_ad[INDEX(1,1)]	= dEyeCanvas;
	t.m_ad[INDEX(1,2)]	= dYo;
	t.m_ad[INDEX(2,3)]	= dEyeGlobal; // return dEyeGlobal/z in z
	t.m_ad[INDEX(3,2)]	= LcTScalar(1);

	t.m_maxW			= -dEyeCanvas * (LcTScalar)0.125;
	return t;
}

/*-------------------------------------------------------------------------*//**
	Multiplies transforms
*/
LC_EXPORT LcTTransform4& LcTTransform4::multiply(const LcTTransform4& t)
{
	int i, j;
	LcTScalar ad[16];

	// Rows (j) in t2, times columns (i) in t1
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			// Don't bother unrolling, as projection multiply is rare
			ad[INDEX(i,j)] =
				m_ad[INDEX(i,0)] * t.m_ad[INDEX(0,j)] +
				m_ad[INDEX(i,1)] * t.m_ad[INDEX(1,j)] +
				m_ad[INDEX(i,2)] * t.m_ad[INDEX(2,j)] +
				m_ad[INDEX(i,3)] * t.m_ad[INDEX(3,j)];
		}
	}

	// Copy new array
	for (i = 0; i < 16; i++)
		m_ad[i] = ad[i];

	return *this;
}

/*-------------------------------------------------------------------------*//**
	After transformation by projection, Z is always returned as zero
*/
LC_EXPORT LcTVector LcTTransform4::transformIgnoreZ(const LcTVector& v) const
{
	// Apply transformation matrix - note vector W is assumed to be 1
	LcTScalar dX =
		m_ad[INDEX(0,0)] * v.x +
		m_ad[INDEX(0,1)] * v.y +
		m_ad[INDEX(0,2)] * v.z +
		m_ad[INDEX(0,3)];
	LcTScalar dY =
		m_ad[INDEX(1,0)] * v.x +
		m_ad[INDEX(1,1)] * v.y +
		m_ad[INDEX(1,2)] * v.z +
		m_ad[INDEX(1,3)];
	LcTScalar dW =
		m_ad[INDEX(3,0)] * v.x +
		m_ad[INDEX(3,1)] * v.y +
		m_ad[INDEX(3,2)] * v.z +
		m_ad[INDEX(3,3)];

	// Apply W before returning, as our vectors are not homogeneous
	LcTScalar d1W = 1/dW;
	return LcTVector(dX * d1W, dY * d1W, 0);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTVector LcTTransform4::transformWithZ(const LcTVector& v) const
{
	// Apply transformation matrix - note vector W is assumed to be 1
	LcTScalar dX =
		m_ad[INDEX(0,0)] * v.x +
		m_ad[INDEX(0,1)] * v.y +
		m_ad[INDEX(0,2)] * v.z +
		m_ad[INDEX(0,3)];
	LcTScalar dY =
		m_ad[INDEX(1,0)] * v.x +
		m_ad[INDEX(1,1)] * v.y +
		m_ad[INDEX(1,2)] * v.z +
		m_ad[INDEX(1,3)];
	LcTScalar dZ =
		m_ad[INDEX(2,0)] * v.x +
		m_ad[INDEX(2,1)] * v.y +
		m_ad[INDEX(2,2)] * v.z +
		m_ad[INDEX(2,3)];
	LcTScalar dW =
		m_ad[INDEX(3,0)] * v.x +
		m_ad[INDEX(3,1)] * v.y +
		m_ad[INDEX(3,2)] * v.z +
		m_ad[INDEX(3,3)];

	// Limit dW so that dEye/dZ < 8
	if (dW > m_maxW)
	{
		dW = m_maxW;
	}

	// Apply W before returning, as our vectors are not homogeneous	
	return LcTVector(dX / dW, dY / dW, dZ / dW);
}
