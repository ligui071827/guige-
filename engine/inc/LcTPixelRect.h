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
#ifndef LcTPixelRectH
#define LcTPixelRectH

#include "inflexionui/engine/inc/LcDefs.h"
#include "inflexionui/engine/inc/LcTPixelPoint.h"
#include "inflexionui/engine/inc/LcTPixelDim.h"

/*-------------------------------------------------------------------------*//**
	Encapsulates the location and dimensions of a rectangle
*/
class LcTPixelRect
{
private:

	// Public so no m_
	int							m_left;
	int							m_top;
	int							m_right;
	int							m_bottom;

public:
	
	// Construction
	LC_IMPORT					LcTPixelRect();
	LC_IMPORT					LcTPixelRect(int newLeft, int newTop, int newRight, int newBottom);

	// Mutators
	inline		void			setLeft(int newLeft)		{ m_left = newLeft; }
	inline		void			setTop(int newTop)			{ m_top = newTop; }
	inline		void			setRight(int newRight)		{ m_right = newRight; }
	inline		void			setBottom(int newBottom)	{ m_bottom = newBottom; }

	// Accessors
	inline int					getLeft()	const	{ return m_left; }
	inline int					getTop()	const	{ return m_top; }
	inline int					getRight()	const	{ return m_right; }
	inline int					getBottom()	const	{ return m_bottom; }

	inline int					getWidth() const	{ return m_right - m_left; }
	inline int					getHeight() const	{ return m_bottom - m_top; }

	// Methods
	LC_IMPORT	bool			contains(const LcTPixelPoint& pt) const;
	LC_IMPORT	bool			contains(const LcTPixelRect& r) const;
	LC_IMPORT	bool			intersects(const LcTPixelRect& r) const;
	LC_IMPORT	LcTPixelRect	intersection(const LcTPixelRect& r) const;
	LC_IMPORT	LcTPixelRect	unionWith(const LcTPixelRect& r) const;
};

#endif // LcTPixelRectH

