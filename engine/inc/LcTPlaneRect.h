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
#ifndef LcTPlaneRectH
#define LcTPlaneRectH

#include "inflexionui/engine/inc/LcTVector.h"

/*-------------------------------------------------------------------------*//**
*/
class LcTPlaneRect
{
private:

	LcTScalar					m_left;
	LcTScalar					m_top;
	LcTScalar					m_zDepth;
	LcTScalar					m_right;
	LcTScalar					m_bottom;

public:

	// Construction
	LC_IMPORT					LcTPlaneRect();
	LC_IMPORT					LcTPlaneRect(	LcTScalar newLeft,
												LcTScalar newTop,
												LcTScalar newZDepth,
												LcTScalar newRight,
												LcTScalar newBottom);

	inline		void			setLeft(LcTScalar newLeft)		{ m_left = newLeft; }
	inline		void			setTop(LcTScalar newTop)		{ m_top = newTop; }
	inline		void			setZDepth(LcTScalar newZDepth)	{ m_zDepth = newZDepth; }
	inline		void			setRight(LcTScalar newRight)	{ m_right = newRight; }
	inline		void			setBottom(LcTScalar newBottom)	{ m_bottom = newBottom; }

	// Accessors
	inline LcTScalar			getLeft() const		{ return m_left; }
	inline LcTScalar			getTop() const		{ return m_top; }
	inline LcTScalar			getRight() const	{ return m_right; }
	inline LcTScalar			getBottom() const	{ return m_bottom; }
	inline LcTScalar			getZDepth() const	{ return m_zDepth; }

	inline LcTScalar			getWidth() const	{ return m_right - m_left; }
	inline LcTScalar			getHeight() const	{ return m_top - m_bottom; }

	// Methods
	LC_IMPORT	bool			contains(const LcTVector& v, LcTScalar expandRectEdge = 0.0f) const;
};

#endif // LcTPlaneRectH

