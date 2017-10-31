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
#ifndef LcCFontH
#define LcCFontH

#include "inflexionui/engine/inc/LcStl.h"
#include "inflexionui/engine/inc/LcTVector.h"
#include "inflexionui/engine/inc/LcTColor.h"
#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcCText.h"
class LcCSpace;
class LcTPlaneRect;

/*-------------------------------------------------------------------------*//**
	Generic font interface to be supported by platform-specific font
	implementations.  Encapsulates a font/style, metrics information and 
	target space, plus any cached resources etc that the font requires.
	Acts as a factory for text objects which can be used for drawing text
	to the target space.
*/
class LcCFont : public LcCBase
{
public:

	// Font constants
	enum EStyle 
	{ 
		DEFAULT		= -1,
		REGULAR		= 0x0, 
		BOLD		= 0x1, 
		ITALIC		= 0x2,
		BOLDITALIC	= 0x3 
	};

private:

	LcCSpace*						m_space;
	int								m_iRefCount;
	LcTmString						m_filePath;

protected:

	// Effectively abstract
	LC_IMPORT						LcCFont(LcCSpace* sp);


LC_PRIVATE_INTERNAL_PUBLIC:

#ifdef LC_OGL_DIRECT
	// Reload the font internals.
	virtual			void			reloadFont(const LcTmString& fileName) = 0;
#endif

#ifdef LC_ENABLE_CJK_SUPPORT
	static			bool			isCJKchar(LcTWChar ch);
					bool			isRestrictedChar(
										LcTWChar*			charArray,
										int					length, 
										LcTWChar			ch);									
#endif 

public:

	// Destruction
	virtual							~LcCFont() {}

	// Reference counting
	inline			void			acquire()		{ m_iRefCount++; }
	LC_IMPORT		void			release();

	// Access methods
	inline			LcCSpace*		getSpace()		{ return m_space; }

	// Text creation
	virtual		LcTaOwner<LcCText>	createText()					= 0;

	// Text length/width calculations
	virtual			LcTScalar		getTextWidth(
										LcTScalar			height, 
										const LcTmString&	text)	= 0;
	LC_VIRTUAL		int				getTextLengthToFit(
										LcTScalar			height, 
										const LcTmString&	text, 
										LcTScalar			width);

	// Utilities - height and width should be in same coordinate units
	// although these units are not defined
	LC_IMPORT LcTaArray<LcTmString>	wrapLines(
										LcTScalar			height, 
										const LcTmString&	text, 
										LcTScalar			width,
										bool				marqueeH);

					void			setFilePath(LcTaString file){m_filePath=file;}
					LcTaString		getFilePath(){return m_filePath;}

	LC_VIRTUAL		void  			releaseResources() {}
	LC_VIRTUAL		void  			reloadResources() {}
	LC_VIRTUAL		bool  			isNativeFont() { return false; }
};

#endif // LcCFontH

