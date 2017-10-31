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
#include "inflexionui/engine/inc/NdhsCEntryPointMapStack.h"
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


/*-------------------------------------------------------------------------*//**
	Creation
*/
LcTaOwner<NdhsCEntryPointMapStack> NdhsCEntryPointMapStack::create()
{
	LcTaOwner<NdhsCEntryPointMapStack> ref;
	ref.set(new NdhsCEntryPointMapStack());
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Construction
*/
void NdhsCEntryPointMapStack::construct()
{
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCEntryPointMapStack::~NdhsCEntryPointMapStack()
{
	clear();
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCEntryPointMapStack::pushMap(const LcTmString& id, const LcTmString& nodeUri)
{
	// Add in map
	m_pairs[id.toLower()] = nodeUri;
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCEntryPointMapStack::getEntryPoint(const LcTmString& id)
{
	// If the class exists then return the data.
	TmMPairs::iterator iter = m_pairs.find(id.toLower());

	return ( (iter == m_pairs.end()) ? "" : (iter->second) );
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCEntryPointMapStack::clear()
{
	m_pairs.clear();
}
