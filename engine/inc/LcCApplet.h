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
#ifndef LcCAppletH
#define LcCAppletH

#include "inflexionui/engine/inc/LcStl.h"
#include "inflexionui/engine/inc/LcCAggregate.h"
#include "inflexionui/engine/inc/LcIResourceManager.h"
class LcTWidgetEvent;
class LcTModelEvent;
class LcCAggregate;

/*-------------------------------------------------------------------------*//**
	An applet is a kind of aggregate that can be realized on a space.
	The root of any widget hierarchy must be an applet, and this is the
	point at which event bubbling stops.  Applets are also the points at
	which LcCSkins are attached.
*/
class LcCApplet : public LcCAggregate, public LcIResourceManager
{
	LC_DECLARE_RTTI(LcCApplet, LcCAggregate)

private:

	// Properties/connections
	LcCSpace*						m_space;
	LcTmString						m_appName;

	// Not allowed for applets (though unfortunately can still call through base pointer)
					bool			realize(LcCAggregate* s);
	LC_VIRTUAL		void			doOnRetire();
	virtual			bool			parentIsVisible()		{ return m_space != 0; }
	virtual			bool			parentIsHidden()		{ return m_space == NULL; }

protected:

	// Abstract so keep constructors protected
	inline							LcCApplet()				{}
	LC_IMPORT		void			construct(const LcTmString& appName);

	LC_IMPORT		bool			checkFileExists(const LcTmString& file);

public:

	// Destruction
	LC_VIRTUAL						~LcCApplet();

	// Applet should only be realized on a space
	LC_IMPORT		bool			realize(LcCSpace* s);

	// Properties
	inline			LcCSpace*		getSpace()				{ return m_space; }
	virtual			LcCApplet*		getApplet()				{ return this; }
	inline			LcTaString		getAppName()			{ return m_appName; }

	// LcIResourceManager methods - applets shouldn't be forced to
	// provide paths
	virtual			LcTaString		getImagePath()			{ return ""; }
	virtual			LcTaString		getXMLPath()			{ return ""; }


	// Resources - handles search paths and asks space to load the actual resource
	LC_VIRTUAL		LcCBitmap*		getBitmap(const LcTmString& file);
	LC_VIRTUAL		LcCFont*		getFont(const LcTmString& name, LcCFont::EStyle style);

		// Meshes are optional
#ifdef LC_USE_MESHES
	LC_VIRTUAL		LcCMesh*		getMesh(const LcTmString& file);
#endif

	// Event overrides - make abstract again
	virtual			void			onWidgetEvent(LcTWidgetEvent* e)	= 0;

	// Text property search overrides
	LC_VIRTUAL		LcTaString		findFontName();
	LC_VIRTUAL		LcCFont::EStyle	findFontStyle();
	LC_VIRTUAL		LcTColor		findFontColor();
	LC_VIRTUAL		LcTColor		findFontColorDisabled();
};

#endif // LcCAppletH

