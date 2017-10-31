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

#include "inflexionui/engine/inc/LcAll.h"
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCText::LcCText(LcCFont* f)
{
	m_font = f;
	m_font->acquire();

	// Create a global caret source bitmap to be used for drawing in all
	// rendering modes.
	lc_memset(m_caretData, 255, LC_CARET_SRC_WIDTH * LC_CARET_SRC_HEIGHT);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCText::~LcCText()
{
	m_font->release();
}

