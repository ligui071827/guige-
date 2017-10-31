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
#ifndef LcIImageH
#define LcIImageH

#include "inflexionui/engine/inc/LcTPixelDim.h"
#include "inflexionui/engine/inc/LcTPixelRect.h"
#include "inflexionui/engine/inc/LcTPlaneRect.h"
#include "inflexionui/engine/inc/LcTScalarRect.h"
#include "inflexionui/engine/inc/LcTVector.h"
#include "inflexionui/engine/inc/LcCSpace.h"
class LcCSpace;

#ifdef LC_USE_XML
	class LcCXmlElem;
#endif

#ifdef LC_PLAT_OGL
	class LcOglCTexture;
#endif

/*-------------------------------------------------------------------------*//**
	General base interface for any object that can be used to render flat
	rectangles of pixels in a single operation, whether or not these change
	from frame to frame.  Objects derived from this interface can be
	positioned in a space using the LcwCImage widget
*/
class LcIImage
{
public:

	virtual							~LcIImage()	{}

	// Required interface
	virtual			LcCSpace*		getSpace() 							= 0;
	virtual			LcTPixelDim		getSize()  							= 0;
	virtual			bool			isOpaque() 							= 0;
	virtual			bool			canBeClipped()						= 0;

	virtual			void			draw(
	       			    				const LcTPlaneRect& dest,
	       			    				const LcTPixelRect& clip,
										LcTColor			color,
	       			    				LcTScalar			fOpacity,
										bool				antiAlias,
										int					meshGridX,
										int					meshGridY)	= 0;

	virtual			bool			isTransparent(
										const LcTPlaneRect&	dest,
										const LcTVector&	scale,
										const LcTPlaneRect*	clip,
										const LcTVector&	hitPos)		= 0;

	// May be implemented if image is a shared resource
	virtual			void			acquire()							{}

	// Returns true if the IImage should be deleted
	virtual			bool			release()							{ return false; }

	LC_VIRTUAL		bool  			isTranslucent() { return false; }
};

#endif // LcIImageH
