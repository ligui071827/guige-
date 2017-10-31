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


#if defined(LC_USE_LIGHTS)

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCLight::LcCLight(LcCSpace* sp)
{
	m_space = sp;
	m_isEnabled = false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCLight::~LcCLight()
{
}

#endif // #if defined(LC_USE_LIGHTS)
