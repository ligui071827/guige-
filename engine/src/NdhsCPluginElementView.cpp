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


#ifdef IFX_USE_PLUGIN_ELEMENTS
#include "inflexionui/engine/inc/NdhsCPluginElementView.h"

/*-------------------------------------------------------------------------*//**
*/
NdhsCPluginElementView::NdhsCPluginElementView(NdhsCPlugin::NdhsCPluginHElement* w)
{
	m_hPluginElement = w;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCPluginElementView::~NdhsCPluginElementView()
{
}

#endif // IFX_USE_PLUGIN_ELEMENTS

