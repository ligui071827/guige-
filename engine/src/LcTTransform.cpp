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
#define INDEX(r,c)  ((c) * 3 + (r))
#define INDEX4(r,c) ((c) * 4 + (r))


/*-------------------------------------------------------------------------*//**
	Constructs an empty matrix.
*/
LC_EXPORT LcTTransform::LcTTransform()
{
	for (int i = 0; i < 12; i++)
		m_ad[i] = 0;
}

/*-------------------------------------------------------------------------*//**
	Set the transform from an array in row-major configuration.
*/
LC_EXPORT LcTTransform& LcTTransform::setFromRowMajorMatrixArray(LcTScalar *rowMajorMatrixArray)
{
	for (int r = 0; r < 3; r++)
	{
		for (int c = 0; c < 4; c++)
		{
			m_ad[INDEX(r, c)] = rowMajorMatrixArray[r*4+c];
		}
	}
	return *this;
}

/*-------------------------------------------------------------------------*//**
	Returns data as column-major array (as used by OpenGL)
*/
#ifdef LC_PLAT_OGL
LC_EXPORT void LcTTransform::getData(LcOglTScalar* f) const
{
	// Copy top 3 rows - keep efficient as may be called for every layer
	for (int i = 0; i < 3; i++)
	{
		f[INDEX4(i,0)] = LC_OGL_FROM_SCALAR(m_ad[INDEX(i,0)]);
		f[INDEX4(i,1)] = LC_OGL_FROM_SCALAR(m_ad[INDEX(i,1)]);
		f[INDEX4(i,2)] = LC_OGL_FROM_SCALAR(m_ad[INDEX(i,2)]);
		f[INDEX4(i,3)] = LC_OGL_FROM_SCALAR(m_ad[INDEX(i,3)]);
	}

	// Set bottom row
	f[INDEX4(3,0)] = 0;
	f[INDEX4(3,1)] = 0;
	f[INDEX4(3,2)] = 0;
	f[INDEX4(3,3)] = LC_OGL_FROM_SCALAR(LcTScalar(1));
}
#endif

/*-------------------------------------------------------------------------*//**
	Constructs and returns an identity matrix.
*/
LC_EXPORT LcTTransform LcTTransform::identity()
{
	// Others should default to zero
	LcTTransform t;
	t.m_ad[INDEX(0,0)]	= 1;
	t.m_ad[INDEX(1,1)]	= 1;
	t.m_ad[INDEX(2,2)]	= 1;
	return t;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTTransform& LcTTransform::scale(LcTScalar dX, LcTScalar dY, LcTScalar dZ)
{
	// Calculates: this x S
	for (int i = 0; i < 3; i++)
	{
		// OK for first arg (r) to be a variable; keep second (c) constant
		m_ad[INDEX(i,0)] *= dX;
		m_ad[INDEX(i,1)] *= dY;
		m_ad[INDEX(i,2)] *= dZ;
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTTransform& LcTTransform::translatePrefix(LcTScalar dX, LcTScalar dY, LcTScalar dZ)
{
	// Calculates: T x this (note the opposite to scaling)
	m_ad[INDEX(0,3)]	+= dX;
	m_ad[INDEX(1,3)]	+= dY;
	m_ad[INDEX(2,3)]	+= dZ;
	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTTransform& LcTTransform::translate(LcTScalar dX, LcTScalar dY, LcTScalar dZ)
{
	// Calculates: this x T (use prefix where possible as it's much cheaper)
	for (int i = 0; i < 3; i++)
	{
		// OK for first arg (r) to be a variable; keep second (c) constant
		m_ad[INDEX(i,3)]	+=
			m_ad[INDEX(i,0)] * dX +
			m_ad[INDEX(i,1)] * dY +
			m_ad[INDEX(i,2)] * dZ;
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
	Constructs and returns a rotation transform from the given quaternion
*/
LC_EXPORT LcTTransform LcTTransform::rotate(const LcTQuaternion& qi)
{
	LcTQuaternion q = qi;
	q.normalise();

	LcTScalar	w2		= q.w * 2;
	LcTScalar	wx2		= w2 * q.x;
	LcTScalar	wy2		= w2 * q.y;
	LcTScalar	wz2		= w2 * q.z;

	LcTScalar	x2		= q.x * 2;
	LcTScalar	xx2		= x2 * q.x;
	LcTScalar	xy2		= x2 * q.y;
	LcTScalar	xz2		= x2 * q.z;

	LcTScalar	y2		= q.y * 2;
	LcTScalar	yy2		= y2 * q.y;
	LcTScalar	yz2		= y2 * q.z;

	LcTScalar	zz2		= q.z * q.z * 2;

	LcTTransform t;
	t.m_ad[INDEX(0,0)]	= 1 - yy2 - zz2;
	t.m_ad[INDEX(0,1)]	= xy2 - wz2;
	t.m_ad[INDEX(0,2)]	= xz2 + wy2;
	t.m_ad[INDEX(1,0)]	= xy2 + wz2;
	t.m_ad[INDEX(1,1)]	= 1 - xx2 - zz2;
	t.m_ad[INDEX(1,2)]	= yz2 - wx2;
	t.m_ad[INDEX(2,0)]	= xz2 - wy2;
	t.m_ad[INDEX(2,1)]	= yz2 + wx2;
	t.m_ad[INDEX(2,2)]	= 1 - xx2 - yy2;
	return t;
}

/*-------------------------------------------------------------------------*//**
	Multiplies transforms
*/
LC_EXPORT LcTTransform& LcTTransform::multiply(const LcTTransform& t)
{
	int i;
	LcTScalar ad[12];

	// Multiply elements (note, computes this * t)
	for (i = 0; i < 3; i++)
	{
		// Loop unrolled - OK for first arg (r) to be a variable,
		// but slow due to second multiply unless second (c) is constant
		ad[INDEX(i,0)] =
			m_ad[INDEX(i,0)] * t.m_ad[INDEX(0,0)] +
			m_ad[INDEX(i,1)] * t.m_ad[INDEX(1,0)] +
			m_ad[INDEX(i,2)] * t.m_ad[INDEX(2,0)];
		ad[INDEX(i,1)] =
			m_ad[INDEX(i,0)] * t.m_ad[INDEX(0,1)] +
			m_ad[INDEX(i,1)] * t.m_ad[INDEX(1,1)] +
			m_ad[INDEX(i,2)] * t.m_ad[INDEX(2,1)];
		ad[INDEX(i,2)] =
			m_ad[INDEX(i,0)] * t.m_ad[INDEX(0,2)] +
			m_ad[INDEX(i,1)] * t.m_ad[INDEX(1,2)] +
			m_ad[INDEX(i,2)] * t.m_ad[INDEX(2,2)];
		ad[INDEX(i,3)] =
			m_ad[INDEX(i,0)] * t.m_ad[INDEX(0,3)] +
			m_ad[INDEX(i,1)] * t.m_ad[INDEX(1,3)] +
			m_ad[INDEX(i,2)] * t.m_ad[INDEX(2,3)];

		// Deal with bottom row of 0,0,0,1 (transform is linear)
		ad[INDEX(i,3)] += m_ad[INDEX(i,3)];
	}

	// Copy new array
	for (i = 0; i < 12; i++)
		m_ad[i] = ad[i];

	return *this;
}

/*-------------------------------------------------------------------------*//**
	Inverts this transform.
*/
LC_EXPORT LcTTransform& LcTTransform::invert()
{
	// The inverse of the transform is the inverse of the
	// 3x3 rotation/scaling part of the transform and a modification of the
	// translation part.

	// Use the handy formula that the inverse of a matrix is one over the
	// determinant times by the transpose of the adjoint.  Even more handily,
	// if we say the columns of the matrix are the vectors C1, C2, C3, then
	// this formula can be expressed as:
	// Inverse = 1/(C1.C2xC3) Transpose{C2xC3 C3xC1 C1xC2}
	// for a 3x3 matrix.  This will take 30 multiplications and 11 additions,
	// compared to 39 mults and 14 additions for the full expansion of the
	// inverse of a 3x3 matrix.
	LcTVector C1(m_ad[INDEX(0,0)],m_ad[INDEX(1,0)],m_ad[INDEX(2,0)]);
	LcTVector C2(m_ad[INDEX(0,1)],m_ad[INDEX(1,1)],m_ad[INDEX(2,1)]);
	LcTVector C3(m_ad[INDEX(0,2)],m_ad[INDEX(1,2)],m_ad[INDEX(2,2)]);
	LcTVector offset(-m_ad[INDEX(0,3)],-m_ad[INDEX(1,3)],-m_ad[INDEX(2,3)]);

	// Work out the adjoint matrix columns
	LcTVector adjtC1 = C2;
	LcTVector adjtC2 = C3;
	LcTVector adjtC3 = C1;
	adjtC1.cross(C3);
	adjtC2.cross(C1);
	adjtC3.cross(C2);

	// Work out the determinant
	LcTScalar det = C1.dot(adjtC1);

	// Matrices with 0 determinants are not invertible - return the matrix
	// unchanged
	if (det == 0)
		return *this;

	// Fill the rotation/scale part of the transform
	LcTScalar oneoverdet = (LcTScalar)1.0 / det;

	m_ad[INDEX(0,0)] = oneoverdet*adjtC1.x;
	m_ad[INDEX(0,1)] = oneoverdet*adjtC1.y;
	m_ad[INDEX(0,2)] = oneoverdet*adjtC1.z;
	m_ad[INDEX(1,0)] = oneoverdet*adjtC2.x;
	m_ad[INDEX(1,1)] = oneoverdet*adjtC2.y;
	m_ad[INDEX(1,2)] = oneoverdet*adjtC2.z;
	m_ad[INDEX(2,0)] = oneoverdet*adjtC3.x;
	m_ad[INDEX(2,1)] = oneoverdet*adjtC3.y;
	m_ad[INDEX(2,2)] = oneoverdet*adjtC3.z;

	// Translation part - the translation part of the transform is the origin of
	// the starting coordinate system in the transformed coordinates, so the
	// translation part of the inverse should be the origin of the transformed
	// coordinate space in the starting co-ordinate system.  Easiest way to get
	// this is to reverse the direction of the original offset, and rotate it
	// with the inverse of the 3x3 rotation/scale matrix
	m_ad[INDEX(0,3)] = m_ad[INDEX(0,0)] * offset.x + m_ad[INDEX(0,1)] * offset.y
						+ m_ad[INDEX(0,2)] * offset.z;
	m_ad[INDEX(1,3)] = m_ad[INDEX(1,0)] * offset.x + m_ad[INDEX(1,1)] * offset.y
						+ m_ad[INDEX(1,2)] * offset.z;
	m_ad[INDEX(2,3)] = m_ad[INDEX(2,0)] * offset.x + m_ad[INDEX(2,1)] * offset.y
						+ m_ad[INDEX(2,2)] * offset.z;

	return *this;
}

/*-------------------------------------------------------------------------*//**
	Returns the linear part of the transpose (i.e. translation values are
	thrown away)
*/
LC_EXPORT LcTTransform& LcTTransform::transpose()
{
	// Transpose 3x3 part of matrix
	for (int i = 0; i < 3; i++)
	{
		for (int j = i + 1; j < 3; j++)
		{
			LcTScalar tmp = m_ad[INDEX(i,j)];
			m_ad[INDEX(i,j)] = m_ad[INDEX(j,i)];
			m_ad[INDEX(j,i)] = tmp;
		}
	}

	// Transpose fake bottom row
	m_ad[INDEX(0,3)] = 0;
	m_ad[INDEX(1,3)] = 0;
	m_ad[INDEX(2,3)] = 0;
	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTVector LcTTransform::transform(const LcTVector& v) const
{
	// Apply transformation matrix
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

	// Save values
	return LcTVector(dX, dY, dZ);
}


