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
LC_EXPORT LcTPlaneRect::LcTPlaneRect()
{
	m_top		= 0;
	m_left		= 0;
	m_zDepth	= 0;
	m_right		= 0;
	m_bottom	= 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTPlaneRect::LcTPlaneRect(
	LcTScalar newLeft,
	LcTScalar newTop,
	LcTScalar newZDepth,
	LcTScalar newRight,
	LcTScalar newBottom)
{
	m_top		= newTop;
	m_left		= newLeft;
	m_zDepth	= newZDepth;
	m_right		= newRight;
	m_bottom	= newBottom;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcTPlaneRect::contains(const LcTVector& v, LcTScalar expandRectEdge) const
{
	return	v.x >= m_left - expandRectEdge &&  v.x < m_right + expandRectEdge
	 &&		v.y >= m_bottom - expandRectEdge	&&  v.y < m_top + expandRectEdge;
}
