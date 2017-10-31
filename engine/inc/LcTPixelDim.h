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
#ifndef LcTPixelDimH
#define LcTPixelDimH

/*-------------------------------------------------------------------------*//**
	Encapsulates the dimensions of a pixel
*/
class LcTPixelDim
{
public:

	// Public so no m_
	int		width;
	int		height;

	// Construction
	inline	LcTPixelDim()				{ width	= 0; height	= 0; }
	inline	LcTPixelDim(int w, int h)	{ width	= w; height	= h; }
};

#endif // LcTPixelDimH

