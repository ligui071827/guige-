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
#ifndef LcTPixelPointH
#define LcTPixelPointH

/*-------------------------------------------------------------------------*//**
	Encapsulates the location of a pixel
*/
class LcTPixelPoint
{
public:

	// Public so no m_
	int		x;
	int		y;

	// Construction
	inline	LcTPixelPoint()					{ x = 0; y = 0; }
	inline	LcTPixelPoint(int ix, int iy)	{ x = ix; y = iy; }
};

#endif // LcTPixelPointH
