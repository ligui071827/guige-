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

// If two numbers differ by less that this tolerance figure, we consider them
// the same.
#define ZERO_TOLERANCE .001

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTScalarQuad::LcTScalarQuad()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTScalarQuad::LcTScalarQuad(
	const LcTVector& newTopLeft,
	const LcTVector& newTopRight,
	const LcTVector& newBottomLeft,
	const LcTVector& newBottomRight)
{
	m_topLeft		= newTopLeft;
	m_topRight 		= newTopRight;
	m_bottomLeft	= newBottomLeft;
	m_bottomRight	= newBottomRight;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcTScalarQuad::convertToRect(LcTScalarRect& returnRect) const
{
	bool retVal = false;
	if ((lc_fabs(m_topLeft.z - m_bottomRight.z) <= ZERO_TOLERANCE)
		&& (lc_fabs(m_topLeft.y - m_topRight.y) <= ZERO_TOLERANCE)
		&& (lc_fabs(m_bottomLeft.y - m_bottomRight.y) <= ZERO_TOLERANCE)
		&& (lc_fabs(m_topLeft.x - m_bottomLeft.x) <= ZERO_TOLERANCE)
		&& (lc_fabs(m_topRight.x - m_bottomRight.x) <= ZERO_TOLERANCE)
		&& (m_topLeft.x <= m_topRight.x)	// Check the item is not elevated 180 degrees
		&& (m_topLeft.y <= m_bottomLeft.y))	// Check the item is not rolled 180 degrees
	{
		returnRect.setLeft(m_topLeft.x);
		returnRect.setTop(m_topLeft.y);
		returnRect.setZDepth(m_topLeft.z);
		returnRect.setRight(m_bottomRight.x);
		returnRect.setBottom(m_bottomRight.y);
		retVal = true;
	}

	return retVal;
}
