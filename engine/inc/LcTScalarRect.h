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
#ifndef LcTScalarRectH
#define LcTScalarRectH

#include "inflexionui/engine/inc/LcTVector.h"
class LcTScalarQuad;

/*-------------------------------------------------------------------------*//**
	Defines a rectangle in the x-y place for graphics sources
*/
class LcTScalarRect
{
private:

	LcTScalar					m_left;
	LcTScalar					m_top;
	LcTScalar					m_zDepth;
	LcTScalar					m_right;
	LcTScalar					m_bottom;

public:

	// Construction
	LC_IMPORT					LcTScalarRect();
	LC_IMPORT					LcTScalarRect(	LcTScalar newLeft,
												LcTScalar newTop,
												LcTScalar newZDepth,
												LcTScalar newRight,
												LcTScalar newBottom);

	// Mutators
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
	inline LcTScalar			getHeight() const	{ return m_bottom - m_top; }

	// Methods
	void						convertToQuad(LcTScalarQuad& returnQuad) const;
};

#endif // LcTScalarRectH

