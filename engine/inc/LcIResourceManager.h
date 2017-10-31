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
#ifndef LcIResourceManagerH
#define LcIResourceManagerH

#include "inflexionui/engine/inc/LcTString.h"
class LcCSpace;
class LcCBitmap;
class LcCFont;

#ifdef LC_USE_MESHES
	class LcCMesh;
#endif

/*-------------------------------------------------------------------------*//**
	A resource manager interface corresponds to a set of resources (bitmaps,
	sounds, configuration) stored in one place.  This set of resources is
	typically associated with a LAF or applet.  An interface of
	this type may be expected to be passed to factory classes so that they
	can initialize themselves from resources.
*/
class LcIResourceManager
{
public:

	// Methods required to access and create resources
	virtual LcTaString		getImagePath()			= 0;
	virtual	LcTaString		getXMLPath()			= 0;
	virtual LcCSpace*		getSpace()				= 0;
	virtual LcCBitmap*		getBitmap(const LcTmString& file) = 0;
	virtual LcCFont*		getFont(const LcTmString& name, LcCFont::EStyle style) = 0;

#ifdef LC_USE_MESHES
	virtual LcCMesh*		getMesh(const LcTmString& file) = 0;
#endif
};

#endif

