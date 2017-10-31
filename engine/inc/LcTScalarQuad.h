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
#ifndef LcTScalarQuadH
#define LcTScalarQuadH

#include "inflexionui/engine/inc/LcTVector.h"
class LcTScalarRect;

/*-------------------------------------------------------------------------*//**
	Defines a rectangle in the x-y place for graphics sources
*/
class LcTScalarQuad
{
private:

	LcTVector			m_topLeft;
	LcTVector			m_topRight;
	LcTVector			m_bottomLeft;
	LcTVector			m_bottomRight;

public:
	
	// Construction
	LC_IMPORT			LcTScalarQuad();
	LC_IMPORT			LcTScalarQuad(	const LcTVector& newTopLeft,
	         			              	const LcTVector& newTopRight,
	         			              	const LcTVector& newBottomLeft,
	         			              	const LcTVector& newBottomRight);

	// Mutators
	inline		void	setTopLeft(const LcTVector& newTopLeft)			{ m_topLeft = newTopLeft; }
	inline		void	setTopRight(const LcTVector& newTopRight)		{ m_topRight = newTopRight; }
	inline		void	setBottomLeft(const LcTVector& newBottomLeft)	{ m_bottomLeft = newBottomLeft; }
	inline		void	setBottomRight(const LcTVector& newBottomRight)	{ m_bottomRight = newBottomRight; }
	
	// Accessors
	inline LcTVector	getTopLeft() const		{ return m_topLeft; }
	inline LcTVector	getTopRight() const		{ return m_topRight; }
	inline LcTVector	getBottomLeft() const	{ return m_bottomLeft; }
	inline LcTVector	getBottomRight() const	{ return m_bottomRight; }

	// Methods
	bool				convertToRect(LcTScalarRect& returnRect) const;
	
};

#endif // LcTScalarQuadH

