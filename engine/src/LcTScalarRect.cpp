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


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTScalarRect::LcTScalarRect()
{
	m_top		= 0;
	m_left		= 0;
	m_zDepth	= 0;
	m_right		= 0;
	m_bottom	= 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTScalarRect::LcTScalarRect(
	LcTScalar newLeft,
	LcTScalar newTop,
	LcTScalar newZDepth,
	LcTScalar newRight,
	LcTScalar newBottom)
{
	m_left		= newLeft;
	m_top 		= newTop;
	m_zDepth	= newZDepth;
	m_right		= newRight;
	m_bottom	= newBottom;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcTScalarRect::convertToQuad(LcTScalarQuad& returnQuad) const
{
	returnQuad.setTopLeft(LcTVector(m_left, m_top, m_zDepth));
	returnQuad.setTopRight(LcTVector(m_right, m_top, m_zDepth));
	returnQuad.setBottomLeft(LcTVector(m_left, m_bottom, m_zDepth));
	returnQuad.setBottomRight(LcTVector(m_right, m_bottom, m_zDepth));
}

