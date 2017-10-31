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
#ifndef NdhsIFieldContextH
#define NdhsIFieldContextH

#include "inflexionui/engine/inc/LcTString.h"

class NdhsCField;
class NdhsCMenuItem;

/*-------------------------------------------------------------------------*//**
*/
class NdhsIFieldContext
{
public:
    virtual NdhsCField*		getField(const LcTmString& field, int slotNum, NdhsCMenuItem* item) = 0;

	// Used as a backup if a field cannot be found, e.g. in static XML menus
    virtual LcTaString		getString(bool& found, const LcTmString& field, int slotNum, NdhsCMenuItem* item) { found = false; return ""; }

	// Page/Component/Menu Component use this to get parameter present
	// in page link
	virtual NdhsCField*		getPageParamValue(const LcTmString& key) = 0;
};

#endif // NdhsIFieldContextH
