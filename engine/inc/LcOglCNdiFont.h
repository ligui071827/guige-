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
#ifndef LcOglCNdiFontH
#define LcOglCNdiFontH

#include "inflexionui/engine/inc/LcOglCFont.h"

/*-------------------------------------------------------------------------*//**
*/
class LcOglCNdiFont : public LcOglCFont
{
protected:

	// Used for reading/deleting width indicator lines
	LC_VIRTUAL	bool		checkPixel(void* pData, int x, int y);

	// Allow only 2-phase construction
	inline					LcOglCNdiFont(LcCSpace* sp) : LcOglCFont(sp) {}


LC_PRIVATE_INTERNAL_PUBLIC:

#ifdef LC_OGL_DIRECT
	// Reload the font internals.
	virtual			void	reloadFont(const LcTmString& fileName) { open(fileName); }
#endif

public:

	// Loads font from single-channel NDI bitmap file
	LC_IMPORT	bool		open(const LcTmString& fileName);

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcOglCNdiFont> create(LcCSpace* sp);

	LC_VIRTUAL		void  			releaseResources();
	LC_VIRTUAL		void  			reloadResources();
};

#endif //LcOglCNdiFontH
