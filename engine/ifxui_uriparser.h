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
#ifndef IFXUI_URIPARSER_H
#define IFXUI_URIPARSER_H

#include "inflexionui/engine/ifxui_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/*  URI Parser Definitions  */
#define IFXU_URI_ERROR_INVALID_URI          (IFX_ERROR_USER_DEFINED)
#define IFXU_URI_ERROR_NOT_FOUND            (IFX_ERROR_USER_DEFINED+1)
#define IFXU_URI_ERROR_GENERAL              (IFX_ERROR_USER_DEFINED+2)
#define IFXU_URI_ERROR_INVALID_HANDLE       (IFX_ERROR_USER_DEFINED+3)
#define IFXU_URI_ERROR_INVALID_PARAM        (IFX_ERROR_USER_DEFINED+4)
#define IFXU_URI_ERROR_INVALID_LINK_PREFIX  (IFX_ERROR_USER_DEFINED+5)

IFX_RETURN_STATUS   IFXU_UriParser_Initialize(
                        IFX_HUTILITY       *hUtility,
                        const IFX_WCHAR    *szLink);

IFX_RETURN_STATUS   IFXU_UriParser_Destroy(
                        IFX_HUTILITY        hUtility);

/* Get methods for URI parsing */
IFX_RETURN_STATUS   IFXU_UriParser_GetLinkPrefixSize(
                        IFX_HUTILITY        hUtility,
                        IFX_INT32          *size);

IFX_RETURN_STATUS   IFXU_UriParser_GetLinkPrefixData(
                        IFX_HUTILITY        hUtility,
                        IFX_WCHAR          *szLinkPrefix);

IFX_RETURN_STATUS   IFXU_UriParser_GetLinkBodySize(
                        IFX_HUTILITY        hUtility,
                        IFX_INT32          *size,
                        const IFX_WCHAR    *defaultValue);

IFX_RETURN_STATUS   IFXU_UriParser_GetLinkBodyStringData(
                        IFX_HUTILITY        hUtility,
                        IFX_WCHAR          *szLinkBody,
                        const IFX_WCHAR    *defaultValue);

IFX_RETURN_STATUS   IFXU_UriParser_GetLinkBodyInt(
                        IFX_HUTILITY        hUtility,
                        IFX_INT32          *pLinkBody,
                        IFX_INT32           defaultValue);

IFX_RETURN_STATUS   IFXU_UriParser_GetLinkBodyTime(
                        IFX_HUTILITY        hUtility,
                        IFX_TIME           *pLinkBody,
                        IFX_TIME            defaultValue);

IFX_RETURN_STATUS   IFXU_UriParser_GetLinkBodyFloat(
                        IFX_HUTILITY        hUtility,
                        float              *pLinkBody,
                        float               defaultValue);

IFX_RETURN_STATUS   IFXU_UriParser_GetLinkBodyBool(
                        IFX_HUTILITY        hUtility,
                        IFX_BOOL           *pLinkBody,
                        IFX_BOOL            defaultValue);

IFX_RETURN_STATUS   IFXU_UriParser_GetParamStringSize(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_INT32          *size,
                        const IFX_WCHAR    *defaultValue);

IFX_RETURN_STATUS   IFXU_UriParser_GetParamStringData(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_WCHAR          *szValue,
                        const IFX_WCHAR    *defaultValue);

IFX_RETURN_STATUS   IFXU_UriParser_GetParamInt(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_INT32          *value,
                        IFX_INT32           defaultValue);

IFX_RETURN_STATUS   IFXU_UriParser_GetParamFloat(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        float              *value,
                        float               defaultValue);

IFX_RETURN_STATUS   IFXU_UriParser_GetParamTime(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_TIME           *value,
                        IFX_TIME            defaultValue);
                        
IFX_RETURN_STATUS   IFXU_UriParser_GetParamBool(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_BOOL           *value,
                        IFX_BOOL            defaultValue);

/* Set methods for URI construction */
IFX_RETURN_STATUS   IFXU_UriParser_SetLinkPrefix(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szLinkPrefix);

IFX_RETURN_STATUS   IFXU_UriParser_SetLinkBodyString(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szLinkBody);

IFX_RETURN_STATUS IFXU_UriParser_SetLinkBodyInt(
                        IFX_HUTILITY        hUtility,
                        IFX_INT32           linkBody);

IFX_RETURN_STATUS IFXU_UriParser_SetLinkBodyFloat(
                        IFX_HUTILITY        hUtility,
                        float               linkBody);

IFX_RETURN_STATUS IFXU_UriParser_SetLinkBodyTime(
                        IFX_HUTILITY        hUtility,
                        IFX_TIME            linkBody);

IFX_RETURN_STATUS IFXU_UriParser_SetLinkBodyBool(
                        IFX_HUTILITY        hUtility,
                        IFX_BOOL            linkBody);

IFX_RETURN_STATUS   IFXU_UriParser_SetParamString(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        const IFX_WCHAR    *szValue);

IFX_RETURN_STATUS   IFXU_UriParser_SetParamInt(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_INT32           value);

IFX_RETURN_STATUS   IFXU_UriParser_SetParamFloat(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        float               value);

IFX_RETURN_STATUS IFXU_UriParser_SetParamTime(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_TIME            value);

IFX_RETURN_STATUS   IFXU_UriParser_SetParamBool(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_BOOL            value);

IFX_RETURN_STATUS   IFXU_UriParser_GetLinkStringSize(
                        IFX_HUTILITY        hUtility,
                        IFX_INT32          *size);

IFX_RETURN_STATUS   IFXU_UriParser_GetLinkStringData(
                        IFX_HUTILITY        hUtility,
                        IFX_WCHAR          *szLinkString);

IFX_RETURN_STATUS   IFXU_UriParser_GetParameterCount(
                        IFX_HUTILITY hUtility, 
                        IFX_INT32 *count);
                        
IFX_RETURN_STATUS   IFXU_UriParser_GetParameterKeySize(
                        IFX_HUTILITY hUtility, 
                        IFX_INT32 paramIndex, 
                        IFX_INT32 *size, 
                        const IFX_WCHAR *defaultValue);
                        
IFX_RETURN_STATUS   IFXU_UriParser_GetParameterKeyData(
                        IFX_HUTILITY hUtility, 
                        IFX_INT32 paramIndex, 
                        IFX_WCHAR *szValue, 
                        const IFX_WCHAR *defaultValue);
                        
IFX_RETURN_STATUS   IFXU_UriParser_GetParameterValueSize(
                        IFX_HUTILITY hUtility, 
                        IFX_INT32 paramIndex, 
                        IFX_INT32 *size, 
                        const IFX_WCHAR *defaultValue);
                        
IFX_RETURN_STATUS   IFXU_UriParser_GetParameterValueData(
                        IFX_HUTILITY hUtility, 
                        IFX_INT32 paramIndex, 
                        IFX_WCHAR *szValue, 
                        const IFX_WCHAR *defaultValue);
                        

#ifdef __cplusplus
}
#endif

#endif /* IFXUI_URIPARSER_H */
