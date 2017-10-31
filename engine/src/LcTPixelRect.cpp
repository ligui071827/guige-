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
LC_EXPORT LcTPixelRect::LcTPixelRect()
{
	m_left		= 0;
	m_top		= 0;
	m_right		= 0;
	m_bottom	= 0;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTPixelRect::LcTPixelRect(int newLeft, int newTop, int newRight, int newBottom)
{
	m_left		= newLeft;
	m_top		= newTop;
	m_right		= newRight;
	m_bottom	= newBottom;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcTPixelRect::contains(const LcTPixelPoint& pt) const
{
	return	pt.x >= m_left	&&  pt.x < m_right
	 &&		pt.y >= m_top	&&  pt.y < m_bottom;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcTPixelRect::contains(const LcTPixelRect& r) const
{
	return	r.m_left >= m_left	&&  r.m_right  <= m_right
	 &&		r.m_top >= m_top		&&  r.m_bottom <= m_bottom;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcTPixelRect::intersects(const LcTPixelRect& r) const
{
	// Get edges of intersection
	int iL = max(m_left, r.m_left);
	int iR = min(m_right, r.m_right);
	int iT = max(m_top, r.m_top);
	int iB = min(m_bottom, r.m_bottom);

	return iL < iR && iT < iB;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTPixelRect LcTPixelRect::intersection(const LcTPixelRect& r) const
{
	// Get edges of intersection
	int iL = max(m_left, r.m_left);
	int iR = min(m_right, r.m_right);
	int iT = max(m_top, r.m_top);
	int iB = min(m_bottom, r.m_bottom);

	// Return empty rectangle unless there is overlap
	if (iL < iR && iT < iB)
		return LcTPixelRect(iL, iT, iR, iB);
	else
		return LcTPixelRect();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTPixelRect LcTPixelRect::unionWith(const LcTPixelRect& r) const
{
	// Get edges of union
	int iL = min(m_left, r.m_left);
	int iR = max(m_right, r.m_right);
	int iT = min(m_top, r.m_top);
	int iB = max(m_bottom, r.m_bottom);

	return LcTPixelRect(iL, iT, iR, iB);
}


