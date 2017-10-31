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

#include "inflexionui/engine/ifxui_defs.h"
#include "inflexionui/engine/ifxui_porting.h"

#include "inflexionui/engine/inc/ifxui_integration.h"
#include "inflexionui/engine/ifxui_module_integration.h"
#include "inflexionui/engine/ifxui_uriparser.h"
#include "inflexionui/engine/ifxui_rtl.h"
#include "inflexionui/engine/inc/LcDefs.h"

/**********************************
* CONSTANTS                       *
**********************************/

/* Maximum string buffer length to hold a single int */
#define MAX_INT_STRING_LEN 30

/* Maximum string buffer length to hold a single float */
#define MAX_FLOAT_STRING_LEN 60


/**********************************
* PROTOTYPES                      *
**********************************/
static IFX_RETURN_STATUS IL_MapString(
        const IFXI_STRING_MAP   pMap[],
        IFX_INT32               sizeMap,
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        IFXI_STRING            *pSrc);

static IFX_RETURN_STATUS IL_MapTime(
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        IFX_TIME                src);

static int replaceString(IFX_WCHAR *strDest, size_t maxsize, const IFX_WCHAR *format, const IFX_WCHAR *replacementStr);
static int replaceInt(IFX_WCHAR *strDest, size_t maxsize, const IFX_WCHAR *format, int num);
static int replaceFloat(IFX_WCHAR *strDest, size_t maxsize, const IFX_WCHAR *format, float num);


/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_GetFieldData                                                */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function is called by Inflexion UI Engine immediately       */
/*      after each successful call to IFXI_GetFieldSize from which a     */
/*      non-zero size was returned, to retrieve the field data.          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hSession                            IL Session                   */
/*      type                                Indicates what data is being */
/*                                          queried                      */
/*      hElement                            Element handle, if available */
/*      hMenu                               Menu handle, if available    */
/*      item                                Menu item index, if available*/
/*      szField                             Name of field                */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      szData                              Buffer into which the field  */
/*                                          data should be copied        */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_GetFieldData(
        IFX_HIL             hSession,
        IFX_FIELD_TYPE      type,
        IFX_HELEMENT        hElement,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        const IFX_WCHAR    *szField,
        IFX_WCHAR          *szData)
{
    IFX_RETURN_STATUS retVal = IFX_SUCCESS;
    IFXI_SESSION *ils = IFXE_SessionFromHIL(hSession);
    IFXI_STRING* cacheStr = IFXI_CacheStrFromSession(ils);
    const IFX_WCHAR* cacheStrData = IFXE_StringData(cacheStr);

    if (cacheStrData)
    {
        lc_wcscpy(szData, cacheStrData);
        IFXE_StringFree(cacheStr);
    }

    else
    {
        retVal = IFX_ERROR;
    }

    return retVal;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_RequestMenuRefresh                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Notifies the UI Engine that the contents of a menu that is       */
/*      currently open (whether visible or not) has changed.             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hModuleId                           Module ID                    */
/*      hMenuId                             Menu ID                      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_RequestMenuRefresh(
        IFX_HMODULEID       hModuleId,
        IFX_HMENUID         hMenuId)
{
    IFXI_SESSION* ils = IFXE_SessionFromHModuleID(hModuleId);

    return IFXE_Callback(IFXI_HuiFromSession(ils), IFX_REFRESH_MENU, (IFX_HELEMENT)NULL,
        (IFX_HMENU)hMenuId, 0, NULL, NULL);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_RequestQueryActiveItem                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Queries the UI Engine about which menu item is currently the     */
/*      active item in the menu identified by the parameter              */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hModuleId                           Module ID                    */
/*      hMenuId                             Menu ID                      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      pItem                               if an active item was found, */
/*                                          it will be filled with the   */
/*                                          index (0-based) of the menu  */
/*                                          item in the active slot.     */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_RequestQueryActiveItem(
        IFX_HMODULEID       hModuleId,
        IFX_HMENUID         hMenuId,
        IFX_INT32          *pItem)
{
    IFXI_SESSION* ils = IFXE_SessionFromHModuleID(hModuleId);

    return IFXE_Callback(IFXI_HuiFromSession(ils), IFX_QUERY_ACTIVE_ITEM, (IFX_HELEMENT)NULL,
        (IFX_HMENU)hMenuId, 0, NULL, pItem);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_RequestRefreshBufferedElement                               */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Notifies the UI Engine that the element needs to be repainted on */
/*      the next paint cycle.                                            */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hModuleId                           Module ID                    */
/*      hElementId                          Element ID                   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_RequestRefreshBufferedElement(
        IFX_HMODULEID       hModuleId,
        IFX_HELEMENTID      hElementId)
{
    IFXI_SESSION* ils = IFXE_SessionFromHModuleID(hModuleId);

    return IFXE_Callback(IFXI_HuiFromSession(ils), IFX_REFRESH_BUFFERED_ELEMENT,
        (IFX_HELEMENT)hElementId, (IFX_HMENU)NULL, 0, NULL, NULL);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_RequestResizeBufferedElementBuffer                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Notifies UI Engine that the element would like its associated    */
/*      buffer to be resized                                             */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hModuleId                           Module ID                    */
/*      hElementId                          Element ID                   */
/*      x                                   Width of buffer              */
/*      y                                   Height of buffer             */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_RequestResizeBufferedElementBuffer(
        IFX_HMODULEID       hModuleId,
        IFX_HELEMENTID      hElementId,
        IFX_INT32           x,
        IFX_INT32           y)
{
    IFXI_SESSION* ils = IFXE_SessionFromHModuleID(hModuleId);
    IFX_BUFFER_SIZE bufferSize;

    bufferSize.x = x;
    bufferSize.y = y;

    return IFXE_Callback(IFXI_HuiFromSession(ils), IFX_RESIZE_BUFFERED_ELEMENT_BUFFER,
        (IFX_HELEMENT)hElementId, (IFX_HMENU)NULL, 0, &bufferSize, NULL);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_RequestResourceSearch                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Finds a specified file in the resource search path and returns a */
/*      full file path.                                                  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hModuleId                           Module ID                    */
/*      hMenuId                             Menu ID                      */
/*      szResource                          Name of file to find         */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      pszFullPath                         Absolute path of file        */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_RequestResourceSearch(
        IFX_HMODULEID       hModuleId,
        IFX_HMENUID         hMenuId,
        const IFX_WCHAR    *szResource,
        IFX_WCHAR          *pszFullPath)
{
    IFXI_SESSION* ils = IFXE_SessionFromHModuleID(hModuleId);

    return IFXE_Callback(IFXI_HuiFromSession(ils), IFX_RESOURCE_SEARCH, (IFX_HELEMENT)NULL,
        (IFX_HMENU)hMenuId, 0, szResource, pszFullPath);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_RequestFullScreenStart                                      */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Requests that the UI Engine allow the specified plug-in element  */
/*      to draw to the whole screen                                      */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hModuleId                           Module ID                    */
/*      hElementId                          Element ID                   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_RequestFullScreenStart(
        IFX_HMODULEID       hModuleId,
        IFX_HELEMENTID      hElementId)
{
    IFXI_SESSION* ils = IFXE_SessionFromHModuleID(hModuleId);

    return IFXE_Callback(IFXI_HuiFromSession(ils), IFX_FULL_SCREEN_START,
        (IFX_HELEMENT)hElementId, (IFX_HMENU)NULL, 0, NULL, NULL);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_RequestFullScreenStop                                       */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      Requests that the UI Engine stop displaying in full screen mode  */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hModuleId                           Module ID                    */
/*      hElementId                          Element ID                   */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_RequestFullScreenStop(
        IFX_HMODULEID       hModuleId,
        IFX_HELEMENTID      hElementId)
{
    IFXI_SESSION* ils = IFXE_SessionFromHModuleID(hModuleId);

    return IFXE_Callback(IFXI_HuiFromSession(ils), IFX_FULL_SCREEN_STOP,
        (IFX_HELEMENT)hElementId, (IFX_HMENU)NULL, 0, NULL, NULL);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_RequestTriggerKey                                           */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function will cause the UI Engine to simulate the given key */
/*      trigger                                                          */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hModuleId                           Module ID                    */
/*      scancode                            Scancode of key trigger      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_RequestTriggerKey(
        IFX_HMODULEID       hModuleId,
        IFX_INT32           scancode)
{
    IFX_RETURN_STATUS retVal = IFX_ERROR;
    IFXI_SESSION* ils = IFXE_SessionFromHModuleID(hModuleId);
    IFXI_MODULE_SESSION* modSession = (IFXI_MODULE_SESSION*)hModuleId;
    IFX_INT32 result = 0;

    /* Check input params */
    if (!ils || !modSession)
    {
        return IFX_ERROR;
    }

    if (IFX_SUCCESS == IFXE_ExclusivityPermitted(IFXI_HuiFromSession(ils), (IFX_HEXCLUSIVITY)hModuleId, &result))
    {
        if (0 == result)
        {
            retVal = IFX_ERROR_EXCLUSIVITY;
        }

        else
        {
            retVal = IFXE_Callback(IFXI_HuiFromSession(ils), IFX_TRIGGER_KEY, (IFX_HELEMENT)NULL,
                (IFX_HMENU)NULL, 0, (void*)scancode, NULL);
        }
    }

    return retVal;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_RequestExecuteLink                                          */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function will cause the UI Engine to execute the given      */
/*      executable link.                                                 */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hModuleId                           Module ID                    */
/*      szUri                               Link to execute              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_RequestExecuteLink(
    IFX_HMODULEID               hModuleId,
    const IFX_WCHAR            *szUri)
{
    IFX_RETURN_STATUS retVal = IFX_ERROR;
    IFXI_SESSION* ils = IFXE_SessionFromHModuleID(hModuleId);
    IFXI_MODULE_SESSION* modSession = (IFXI_MODULE_SESSION*)hModuleId;
    IFX_INT32 result = 0;

    /* Check input params */
    if (!ils || !modSession)
    {
        return IFX_ERROR;
}

    if (IFX_SUCCESS == IFXE_ExclusivityPermitted(IFXI_HuiFromSession(ils), (IFX_HEXCLUSIVITY)hModuleId, &result))
    {
        if (0 == result)
        {
            retVal = IFX_ERROR_EXCLUSIVITY;
        }

        else
        {
            retVal = IFXE_Callback(IFXI_HuiFromSession(ils), IFX_COMMAND_LINK, (IFX_HELEMENT)NULL,
                (IFX_HMENU)NULL, 0, szUri, NULL);
        }
    }

    return retVal;
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_RequestCloseMenu                                            */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function will cause the UI Engine to close the given        */
/*      dynamic menu.                                                    */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hModuleId                           Module ID                    */
/*      hMenuId                             Menu ID                      */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_RequestCloseMenu(
    IFX_HMODULEID       hModuleId,
    IFX_HMENUID         hMenuId)
{
    IFXI_SESSION* ils = IFXE_SessionFromHModuleID(hModuleId);

    return IFXE_Callback(IFXI_HuiFromSession(ils), IFX_CLOSE_MENU, (IFX_HELEMENT)NULL,
        (IFX_HMENU)hMenuId, 0, NULL, NULL);
}

/*************************************************************************/
/*                                                                       */
/* FUNCTION                                                              */
/*                                                                       */
/*      IFXI_RequestSetActiveItem                                        */
/*                                                                       */
/* DESCRIPTION                                                           */
/*                                                                       */
/*      This function will cause the specified item to become active     */
/*                                                                       */
/* INPUTS                                                                */
/*                                                                       */
/*      hModuleId                           Module ID                    */
/*      hMenuId                             Menu ID                      */
/*      item                                Menu item index              */
/*                                                                       */
/* OUTPUTS                                                               */
/*                                                                       */
/*      IFX_SUCCESS                         If successful                */
/*      IFX_ERROR                           Any error                    */
/*                                                                       */
/*************************************************************************/
IFX_RETURN_STATUS IFXI_RequestSetActiveItem(
    IFX_HMODULEID       hModuleId,
    IFX_HMENUID         hMenuId,
    IFX_INT32           item)
{
    IFXI_SESSION* ils = IFXE_SessionFromHModuleID(hModuleId);

    return IFXE_Callback(IFXI_HuiFromSession(ils), IFX_SET_ACTVE_ITEM, (IFX_HELEMENT)NULL,
        (IFX_HMENU)hMenuId, item, NULL, NULL);
}

/**********************************************************
 * URI helper fns
 */

/*-------------------------------------------------------------------------*//**
    Get the value of a string parameter from a URI
    hSession - Uri parser utility session
    szParam - Name of the parameter
    defaultValue - Default value for the parameter if not present
    returns - Pointer to dynamically allocated string containing value:
        OWNERSHIP OF STRING IS TRANSFERRED. Pointer will be NULL in case
        of error.
*/
IFX_WCHAR* IFXE_GetUriStringParam(
        IFX_HUTILITY        hSession,
        const IFX_WCHAR    *szParam,
        const IFX_WCHAR    *szDefaultValue)
{
    IFX_BOOL success = IFX_TRUE;
    IFX_WCHAR *retVal = NULL;
    IFX_INT32 len = 0;

    /* Check input parameters are valid */
    if (!szParam)
    {
        return NULL;
    }

    if (IFX_SUCCESS != IFXU_UriParser_GetParamStringSize(
            hSession, szParam, &len, szDefaultValue))
    {
        success = IFX_FALSE;
    }

    if (success)
    {
        retVal = (IFX_WCHAR*)IFXE_AllocateMemoryUnsafe(len * sizeof(IFX_WCHAR));
        if (retVal == NULL)
        {
            success = IFX_FALSE;
        }
    }

    if (success)
    {
        if (IFX_SUCCESS != IFXU_UriParser_GetParamStringData(hSession,
                                                szParam,
                                                retVal,
                                                szDefaultValue))
        {
            success = IFX_FALSE;
        }
    }

    if (!success)
    {
        if (retVal)
        {
            IFXE_FreeMemoryUnsafe(retVal);
            retVal = NULL;
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Get the value of a string body from a URI
    hSession - Uri parser utility session
    returns - Pointer to dynamically allocated string containing body:
        OWNERSHIP OF STRING IS TRANSFERRED. Pointer will be NULL in case
        of error.
*/
IFX_WCHAR* IFXE_GetUriBody(
        IFX_HUTILITY        hSession,
        const IFX_WCHAR    *szDefaultValue)
{
    IFX_BOOL success = IFX_TRUE;
    IFX_WCHAR *retVal = NULL;
    IFX_INT32 len = 0;

    if (IFX_SUCCESS != IFXU_UriParser_GetLinkBodySize(hSession, &len, szDefaultValue))
    {
        success = IFX_FALSE;
    }

    if (success)
    {
        retVal = (IFX_WCHAR*)IFXE_AllocateMemoryUnsafe(len * sizeof(IFX_WCHAR));
        if (retVal == NULL)
        {
            success = IFX_FALSE;
        }
    }

    if (success)
    {
        if (IFX_SUCCESS != IFXU_UriParser_GetLinkBodyStringData(hSession,
                                                retVal, szDefaultValue))
        {
            success = IFX_FALSE;
        }
    }

    if (!success)
    {
        if (retVal)
        {
            IFXE_FreeMemoryUnsafe(retVal);
            retVal = NULL;
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Returns the IFXI_SESSION from a given IFX_HIL handle
*/
IFXI_SESSION* IFXE_SessionFromHIL(
        IFX_HIL             hil)
{
    return (IFXI_SESSION*)hil;
}

/*-------------------------------------------------------------------------*//**
    Returns the IFXI_SESSION from a given IFX_HMODULEID
*/
IFXI_SESSION* IFXE_SessionFromHModuleID(
        IFX_HMODULEID       hModuleId)
{
    return hModuleId ? ((IFXI_MODULE_SESSION*)hModuleId)->ils : NULL;
}

/**********************************************************
 * Mapping fns
 */

/*-------------------------------------------------------------------------*//**
    Maps a string to a string
    pMap - The mapping to use
    sizeMap - The size of pMap
    szDefValue - Default value if no map found
    pDest - Destination string
    pSrc - Source string
    Returns - Success of operation
*/
IFX_RETURN_STATUS IL_MapString(
        const IFXI_STRING_MAP   pMap[],
        IFX_INT32               sizeMap,
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        IFXI_STRING            *pSrc)
{
    IFX_RETURN_STATUS retVal = IFX_SUCCESS;
    IFX_BOOL found = IFX_FALSE;
    const IFX_WCHAR* srcData = IFXE_StringData(pSrc);

    if (pMap && srcData)
    {
        int mapIndex = 0;
        for(; mapIndex < sizeMap && !found; mapIndex++)
        {
            if (lc_wcscmpi(srcData, pMap[mapIndex].input) == 0)
            {
                found = IFX_TRUE;
                IFXE_StringAssignConst(pDest, pMap[mapIndex].value);
            }
        }
    }

    if (!found && szDefValue)
    {
        int defValueLen = lc_wcslen(szDefValue);
        int srcLen = 0;

        if (srcData)
        {
            srcLen = lc_wcslen(srcData);
        }

        found = IFX_TRUE;
        retVal = IFXE_StringAlloc(pDest, defValueLen + srcLen);

        if (retVal == IFX_SUCCESS)
        {
            replaceString(IFXE_StringDataWritable(pDest),
                defValueLen + srcLen + 1,
                szDefValue, srcData);
        }
    }

    if (!found)
    {
        IFXE_StringCopy(pDest, pSrc, IFX_TRUE);
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Maps an int to a string
    pMap - The mapping to use
    sizeMap - The size of pMap
    szDefValue - Default value if no map found
    pDest - Destination string
    pSrc - Source int
    Returns - Success of operation
*/
IFX_RETURN_STATUS IFXE_MapInt(
        const IFXI_INT_MAP      pMap[],
        IFX_INT32               sizeMap,
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        IFX_INT32               src)
{
    IFX_RETURN_STATUS retVal = IFX_SUCCESS;

    if (!szDefValue)
    {
        retVal = IFX_ERROR;
    }

    else
    {
        IFX_BOOL found = IFX_FALSE;

        if (pMap)
        {
            int mapIndex = 0;
            for(; mapIndex < sizeMap && !found; mapIndex++)
            {
                if (src >= pMap[mapIndex].from && src <= pMap[mapIndex].to)
                {
                    const IFX_WCHAR* mappedValue = pMap[mapIndex].value;
                    int destLen = lc_wcslen(mappedValue) + 21;
                    found = IFX_TRUE;
                    retVal = IFXE_StringAlloc(pDest, destLen);

                    if (retVal == IFX_SUCCESS)
                    {
                        replaceInt(IFXE_StringDataWritable(pDest),
                            destLen + 1, mappedValue, src);
                    }
                }
            }
        }

        if (!found)
        {
            int destLen = lc_wcslen(szDefValue) + 21;
            retVal = IFXE_StringAlloc(pDest, destLen);

            if (retVal == IFX_SUCCESS)
            {
                replaceInt(IFXE_StringDataWritable(pDest), destLen + 1,
                    szDefValue, src);
            }
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Maps a float to a string
    pMap - The mapping to use
    sizeMap - The size of pMap
    szDefValue - Default value if no map found
    pDest - Destination string
    pSrc - Source float
    Returns - Success of operation
*/
IFX_RETURN_STATUS IFXE_MapFloat(
        const IFXI_FLOAT_MAP    pMap[],
        IFX_INT32               sizeMap,
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        float                   src)
{
    IFX_RETURN_STATUS retVal = IFX_SUCCESS;

    if (!szDefValue)
    {
        retVal = IFX_ERROR;
    }

    else
    {
        IFX_BOOL found = IFX_FALSE;

        if (pMap)
        {
            int mapIndex = 0;
            for(; mapIndex < sizeMap && !found; mapIndex++)
            {
                if (src >= pMap[mapIndex].from && src <= pMap[mapIndex].to)
                {
                    const IFX_WCHAR* mappedValue = pMap[mapIndex].value;
                    int destLen = lc_wcslen(mappedValue) + 21;
                    found = IFX_TRUE;
                    retVal = IFXE_StringAlloc(pDest, destLen);

                    if (retVal == IFX_SUCCESS)
                    {
                        replaceFloat(IFXE_StringDataWritable(pDest),
                            destLen + 1, mappedValue, src);
                    }
                }
            }
        }

        if (!found)
        {
            int destLen = lc_wcslen(szDefValue) + 21;
            retVal = IFXE_StringAlloc(pDest, destLen);

            if (retVal == IFX_SUCCESS)
            {
                replaceFloat(IFXE_StringDataWritable(pDest), destLen + 1,
                    szDefValue, src);
            }
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Maps a bool to a string
    pMap - The mapping to use
    sizeMap - The size of pMap
    szDefValue - Default value if no map found
    pDest - Destination string
    pSrc - Source bool
    Returns - Success of operation
*/
IFX_RETURN_STATUS IFXE_MapBool(
        const IFXI_BOOL_MAP     pMap[],
        IFX_INT32               sizeMap,
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        IFX_BOOL                src)
{
    IFX_BOOL found = IFX_FALSE;

    if (pMap)
    {
        int mapIndex = 0;
        for(; mapIndex < sizeMap && !found; mapIndex++)
        {
            if (src == pMap[mapIndex].input)
            {
                IFXE_StringAssignConst(pDest, pMap[mapIndex].value);
                found = IFX_TRUE;
            }
        }
    }

    if (!found)
    {
        if (szDefValue)
        {
            IFXE_StringAssignConst(pDest, szDefValue);
        }
        
        else
        {
            IFXE_StringAssignConst(pDest, src ? L"true" : L"false");
        }
    }

    return IFX_SUCCESS;
}

/*-------------------------------------------------------------------------*//**
    Maps a time to a string
    szDefValue - Defines format of output (using strftime-like tokens)
    pDest - Destination string
    pSrc - Source time
    Returns - Success of operation
*/
IFX_RETURN_STATUS IL_MapTime(
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        IFX_TIME                pSrc)
{
    IFX_RETURN_STATUS retVal = IFX_SUCCESS;

    if (!szDefValue)
    {
        retVal = IFX_ERROR;
    }

    else
    {
        int destLen = lc_wcslen(szDefValue) + 100;
        retVal = IFXE_StringAlloc(pDest, destLen);

        if (retVal == IFX_SUCCESS)
        {
            (void)lc_wcsftime(IFXE_StringDataWritable(pDest), destLen + 1,
                szDefValue, lc_gmtime(&pSrc));
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Maps a time to a string
    szDefValue - Defines format of output (using strftime-like tokens)
    pDest - Destination string
    pSrc - Source time
    Returns - Success of operation
*/
IFX_RETURN_STATUS IFXE_MapTime(
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        IFX_TIME                pSrc)
{
    IFX_RETURN_STATUS retVal = IFX_SUCCESS;

    if (!szDefValue)
    {
        retVal = IFX_ERROR;
    }

    else
    {
        int destLen = lc_wcslen(szDefValue) + 100;
        retVal = IFXE_StringAlloc(pDest, destLen);

        if (retVal == IFX_SUCCESS)
        {
            (void)lc_wcsftime(IFXE_StringDataWritable(pDest), destLen + 1,
                szDefValue, lc_gmtime(&pSrc));
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Gets a menu-related string from a module
    sizeStrFn - String size function to call
    getStrFn - String data function to call
    moduleSession - Session to pass to module
    menuHandle - Menu handle to pass to module
    dest - Output string
    Returns - Success of operation
*/
IFX_RETURN_STATUS IFXE_GetMenuString(
        LPFN_IFXM_GetMenuPropertyStringSize     sizeStrFn,
        LPFN_IFXM_GetMenuPropertyStringData     getStrFn,
        IFX_HMODULE                             hModule,
        IFX_HMENU                               hMenu,
        IFXI_STRING                            *pDest)
{
    IFX_RETURN_STATUS retVal = IFX_SUCCESS;
    IFX_INT32 fieldSize = 0;
    IFX_HREQUEST request = NULL;

    /* Check input parameters are valid */
    if (!sizeStrFn || !getStrFn)
    {
        return IFX_ERROR;
    }

    retVal = sizeStrFn(hModule, &request, hMenu, &fieldSize);

    if (retVal == IFX_SUCCESS)
    {
        retVal = IFXE_StringAlloc(pDest, fieldSize);

        if (retVal == IFX_SUCCESS)
        {
            retVal = getStrFn(hModule, request, IFXE_StringDataWritable(pDest));
        }

        else
        {
            /* Allow module to clean up */
            (void)getStrFn(hModule, request, NULL);
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Gets a menu item-related string from a module
    sizeStrFn - String size function to call
    getStrFn - String data function to call
    moduleSession - Session to pass to module
    menuHandle - Menu handle to pass to module
    item - Item number
    dest - Output string
    Returns - Success of operation
*/
IFX_RETURN_STATUS IFXE_GetMenuItemString(
        LPFN_IFXM_GetMenuFieldStringSize    sizeStrFn,
        LPFN_IFXM_GetMenuFieldStringData    getStrFn,
        IFX_HMODULE                         hModule,
        IFX_HMENU                           hMenu,
        IFX_INT32                           item,
        IFXI_STRING                        *pDest)
{
    IFX_RETURN_STATUS retVal = IFX_SUCCESS;
    IFX_INT32 fieldSize = 0;
    IFX_HREQUEST request = NULL;

    /* Check input parameters are valid */
    if (!sizeStrFn || !getStrFn)
    {
        return IFX_ERROR;
    }

    retVal = sizeStrFn(hModule, &request, hMenu, item, &fieldSize);

    if (retVal == IFX_SUCCESS)
    {
        retVal = IFXE_StringAlloc(pDest, fieldSize);

        if (retVal == IFX_SUCCESS)
        {
            retVal = getStrFn(hModule, request, IFXE_StringDataWritable(pDest));
        }

        else
        {
            /* Allow module to clean up */
            (void)getStrFn(hModule, request, NULL);
        }
    }

    return retVal;
}

/**********************************************************
 * String fns
 */

/*-------------------------------------------------------------------------*//**
    Dynamically allocate memory for a string
    str - String to allocate memory for
    length - maximum length of string, not including zero-terminator
    returns - IFX_SUCCESS if successful, IFX_ERROR otherwise
*/
IFX_RETURN_STATUS IFXE_StringAlloc(
        IFXI_STRING          *pStr,
        IFX_INT32             length)
{
    IFX_RETURN_STATUS retVal = IFX_ERROR;

    if (pStr)
    {
        IFXE_StringFree(pStr);

        pStr->data = (IFX_WCHAR*)IFXE_AllocateMemoryUnsafe((length + 1) * sizeof(IFX_WCHAR));
        if (pStr->data)
        {
            pStr->ownsData = IFX_TRUE;

            retVal = IFX_SUCCESS;
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Access the string data
    str - String to access
    returns - String data in wide-char format, or NULL if no data assigned
*/
const IFX_WCHAR* IFXE_StringData(
        const IFXI_STRING    *pStr)
{
    const IFX_WCHAR *retVal = NULL;

    if (pStr)
    {
        if (pStr->data)
        {
            retVal = pStr->data;
        }

        else
        {
            retVal = pStr->constData;
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Access the string data in order to write to it
    str - String to access
    returns - String data in wide-char format, or NULL if no writable data assigned
*/
IFX_WCHAR *IFXE_StringDataWritable(
        const IFXI_STRING    *pStr)
{
    IFX_WCHAR *retVal = NULL;

    if (pStr)
    {
        if (pStr->data && pStr->ownsData)
        {
            retVal = pStr->data;
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Frees resources owned by a string
    str - String to free
*/
void IFXE_StringFree(
        IFXI_STRING          *pStr)
{
    if (pStr)
    {
        if (pStr->ownsData && pStr->data)
        {
            IFXE_FreeMemoryUnsafe(pStr->data);
        }

        IFXE_StringInit(pStr);
    }
}

/*-------------------------------------------------------------------------*//**
    Initialize a string to default, empty values
    str - string to initialize
*/
void IFXE_StringInit(
        IFXI_STRING          *pStr)
{
    if (pStr)
    {
        pStr->data = NULL;
        pStr->ownsData = IFX_FALSE;
        pStr->constData = NULL;
    }
}

/*-------------------------------------------------------------------------*//**
    Assign string data to the string
    dest - Destination string
    src - Source characters, zero terminated
    transferOwnership - Whether the string will take ownership the data. If
        true, the data will be deallocated on calling IFXE_StringFree()
*/
void IFXE_StringAssign(
        IFXI_STRING        *pDest,
        IFX_WCHAR          *szSrc,
        IFX_BOOL            transferOwnership)
{
    if (pDest)
    {
        IFXE_StringFree(pDest);

        pDest->data = szSrc;
        pDest->ownsData = transferOwnership;
    }
}

/*-------------------------------------------------------------------------*//**
    Assign constant data to the string
    dest - Destination string
    src - Source characters, zero terminated
*/
void IFXE_StringAssignConst(
        IFXI_STRING          *pDest,
        const IFX_WCHAR      *szSrc)
{
    if (pDest)
    {
        IFXE_StringFree(pDest);

        pDest->constData = szSrc;
    }
}

/*-------------------------------------------------------------------------*//**
    Copy one string to another. No new memory is allocated.
    dest - Destination string
    src - Source string
    transferOwnership - whether ownership of the data should be transferred from
        src to dest
*/
void IFXE_StringCopy(
        IFXI_STRING          *pDest,
        IFXI_STRING          *pSrc,
        IFX_BOOL             transferOwnership)
{
    if (pDest)
    {
        IFXE_StringFree(pDest);

        if (pSrc)
        {
            pDest->data = pSrc->data;
            pDest->constData = pSrc->constData;

            if (transferOwnership)
            {
                pDest->ownsData = pSrc->ownsData;
                pSrc->ownsData = IFX_FALSE;
            }

            else
            {
                pDest->ownsData = IFX_FALSE;
            }
        }
    }
}

/*-------------------------------------------------------------------------*//**
    Returns the IL_Module value associated with the given IL_HandleType
*/
IFX_INT32 IFXE_GetModule(IFX_INT32 handle)
{
    return handle >> 16;
}



IFX_RETURN_STATUS IFXE_MapGlobalField(
    LPFN_IFXM_GetGlobalFieldStringSize  sizeStrFn,
    LPFN_IFXM_GetGlobalFieldStringData  getStrFn,
    LPFN_IFXM_GetGlobalFieldIntData     getIntFn,
    LPFN_IFXM_GetGlobalFieldFloatData   getFloatFn,
    LPFN_IFXM_GetGlobalFieldTimeData    getTimeFn,
    LPFN_IFXM_GetGlobalFieldBoolData    getBoolFn,
    const IFXI_STRING_MAP              *pStringMap,
    const IFXI_INT_MAP                 *pIntMap,
    const IFXI_FLOAT_MAP               *pFloatMap,
    const IFXI_BOOL_MAP                *pBoolMap,
    IFX_INT32                           mapSize,
    const IFX_WCHAR                    *szDefValue,
    IFXI_STRING                        *pDest,
    IFX_HMODULE                         hModule)
{
    IFX_RETURN_STATUS retVal = IFX_ERROR;

    if (sizeStrFn && getStrFn)
    {
        IFX_INT32 fieldSize = 0;
        IFX_HREQUEST request = NULL;

        retVal = sizeStrFn(hModule, &request, &fieldSize);

        if (retVal == IFX_SUCCESS)
        {
            IFXI_STRING fieldStr;
            IFXE_StringInit(&fieldStr);

            retVal = IFXE_StringAlloc(&fieldStr, fieldSize);

            if (retVal == IFX_SUCCESS)
            {
                retVal = getStrFn(hModule, request,
                    IFXE_StringDataWritable(&fieldStr));
                if (retVal == IFX_SUCCESS)
                {
                    retVal = IL_MapString(pStringMap, mapSize, szDefValue,
                        pDest, &fieldStr);
                }

                IFXE_StringFree(&fieldStr);
            }

            else
            {
                /* Allow module to clean up */
                (void)getStrFn(hModule, request, NULL);
            }
        }
    }

    else if (getIntFn)
    {
        IFX_INT32 fieldInt;
        retVal = getIntFn(hModule, &fieldInt);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IFXE_MapInt(pIntMap, mapSize, szDefValue, pDest,
                fieldInt);
        }
    }

    else if (getFloatFn)
    {
        float fieldFloat;
        retVal = getFloatFn(hModule, &fieldFloat);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IFXE_MapFloat(pFloatMap, mapSize, szDefValue, pDest,
                fieldFloat);
        }
    }

    else if (getTimeFn)
    {
        IFX_TIME fieldTime;
        retVal = getTimeFn(hModule, &fieldTime);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IL_MapTime(szDefValue, pDest, fieldTime);
        }
    }

    else if (getBoolFn)
    {
        IFX_BOOL fieldBool;
        retVal = getBoolFn(hModule, &fieldBool);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IFXE_MapBool(pBoolMap, mapSize, szDefValue, pDest,
                fieldBool);
        }
    }

    else
    {
        /* Not found - do nothing */
    }

    return retVal;
}

IFX_RETURN_STATUS IFXE_MapMenuField(
    LPFN_IFXM_GetMenuFieldStringSize    sizeStrFn,
    LPFN_IFXM_GetMenuFieldStringData    getStrFn,
    LPFN_IFXM_GetMenuFieldIntData       getIntFn,
    LPFN_IFXM_GetMenuFieldFloatData     getFloatFn,
    LPFN_IFXM_GetMenuFieldTimeData      getTimeFn,
    LPFN_IFXM_GetMenuFieldBoolData      getBoolFn,
    const IFXI_STRING_MAP              *pStringMap,
    const IFXI_INT_MAP                 *pIntMap,
    const IFXI_FLOAT_MAP               *pFloatMap,
    const IFXI_BOOL_MAP                *pBoolMap,
    IFX_INT32                           mapSize,
    const IFX_WCHAR                    *szDefValue,
    IFXI_STRING                        *pDest,
    IFX_HMODULE                         hModule,
    IFXI_MODULE_MENU_INFO              *pMenuInfo,
    IFX_INT32                           item)
{
    IFX_RETURN_STATUS retVal = IFX_ERROR;

    /* Check input parameters are valid */
    if (!pMenuInfo)
    {
        return IFX_ERROR;
    }

    if (sizeStrFn && getStrFn)
    {
        IFX_INT32 fieldSize=0;
        IFX_HREQUEST request = NULL;

        retVal = sizeStrFn(hModule, &request, pMenuInfo->menuHandle,
            item, &fieldSize);

        if (retVal == IFX_SUCCESS)
        {
            IFXI_STRING fieldStr;
            IFXE_StringInit(&fieldStr);

            retVal = IFXE_StringAlloc(&fieldStr, fieldSize);

            if (retVal == IFX_SUCCESS)
            {
                retVal = getStrFn(hModule, request,
                    IFXE_StringDataWritable(&fieldStr));

                if (retVal == IFX_SUCCESS)
                {
                    retVal = IL_MapString(pStringMap, mapSize, szDefValue,
                        pDest, &fieldStr);
                }

                IFXE_StringFree(&fieldStr);
            }

            else
            {
                /* Allow module to clean up */
                (void)getStrFn(hModule, request, NULL);
            }
        }
    }

    else if (getIntFn)
    {
        IFX_INT32 fieldInt;
        retVal = getIntFn(hModule, pMenuInfo->menuHandle, item,
            &fieldInt);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IFXE_MapInt(pIntMap, mapSize, szDefValue, pDest,
                fieldInt);
        }
    }

    else if (getFloatFn)
    {
        float fieldFloat;
        retVal = getFloatFn(hModule, pMenuInfo->menuHandle, item,
            &fieldFloat);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IFXE_MapFloat(pFloatMap, mapSize, szDefValue, pDest,
                fieldFloat);
        }
    }

    else if (getTimeFn)
    {
        IFX_TIME fieldTime;
        retVal = getTimeFn(hModule, pMenuInfo->menuHandle, item,
            &fieldTime);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IL_MapTime(szDefValue, pDest, fieldTime);
        }
    }

    else if (getBoolFn)
    {
        IFX_BOOL fieldBool;
        retVal = getBoolFn(hModule, pMenuInfo->menuHandle, item,
            &fieldBool);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IFXE_MapBool(pBoolMap, mapSize, szDefValue, pDest,
                fieldBool);
        }
    }

    else
    {
        /* Not found - do nothing */
    }

    return retVal;
}

IFX_RETURN_STATUS IFXE_MapElementField(
    LPFN_IFXM_GetElementFieldStringSize     sizeStrFn,
    LPFN_IFXM_GetElementFieldStringData     getStrFn,
    LPFN_IFXM_GetElementFieldIntData        getIntFn,
    LPFN_IFXM_GetElementFieldFloatData      getFloatFn,
    LPFN_IFXM_GetElementFieldTimeData       getTimeFn,
    LPFN_IFXM_GetElementFieldBoolData       getBoolFn,
    const IFXI_STRING_MAP                  *pStringMap,
    const IFXI_INT_MAP                     *pIntMap,
    const IFXI_FLOAT_MAP                   *pFloatMap,
    const IFXI_BOOL_MAP                    *pBoolMap,
    IFX_INT32                               mapSize,
    const IFX_WCHAR                        *szDefValue,
    IFXI_STRING                            *pDest,
    IFX_HMODULE                             hModule,
    IFXI_MODULE_ELEMENT_INFO               *pElementInfo,
    IFXI_FIELDDATA_TYPE                     fieldType,
    const IFX_WCHAR                        *szField)
{
    IFX_RETURN_STATUS retVal = IFX_ERROR;

    /* Check input parameters are valid */
    if (!pElementInfo)
    {
        return IFX_ERROR;
    }

    if (IFXI_FIELDDATA_STRING == fieldType && sizeStrFn && getStrFn)
    {
        IFX_INT32 fieldSize;
        IFX_HREQUEST request = NULL;

        retVal = sizeStrFn(hModule, &request,
            pElementInfo->elementHandle, szField, &fieldSize);

        if (retVal == IFX_SUCCESS)
        {
            IFXI_STRING fieldStr;
            IFXE_StringInit(&fieldStr);

            retVal = IFXE_StringAlloc(&fieldStr, fieldSize);

            if (retVal == IFX_SUCCESS)
            {
                retVal = getStrFn(hModule, request,
                    IFXE_StringDataWritable(&fieldStr));

                if (retVal == IFX_SUCCESS)
                {
                    retVal = IL_MapString(pStringMap, mapSize, szDefValue,
                        pDest, &fieldStr);
                }

                IFXE_StringFree(&fieldStr);
            }

            else
            {
                /* Allow module to clean up */
                (void)getStrFn(hModule, request, NULL);
            }
        }
    }

    else if (IFXI_FIELDDATA_INT == fieldType && getIntFn)
    {
        IFX_INT32 fieldInt;
        retVal = getIntFn(hModule, pElementInfo->elementHandle,
            szField, &fieldInt);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IFXE_MapInt(pIntMap, mapSize, szDefValue, pDest,
                fieldInt);
        }
    }

    else if (IFXI_FIELDDATA_FLOAT == fieldType && getFloatFn)
    {
        float fieldFloat;
        retVal = getFloatFn(hModule, pElementInfo->elementHandle,
            szField, &fieldFloat);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IFXE_MapFloat(pFloatMap, mapSize, szDefValue, pDest,
                fieldFloat);
        }
    }

    else if (IFXI_FIELDDATA_TIME == fieldType && getTimeFn)
    {
        IFX_TIME fieldTime;
        retVal = getTimeFn(hModule, pElementInfo->elementHandle,
            szField, &fieldTime);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IL_MapTime(szDefValue, pDest, fieldTime);
        }
    }

    else if (IFXI_FIELDDATA_BOOL == fieldType && getBoolFn)
    {
        IFX_BOOL fieldBool;
        retVal = getBoolFn(hModule, pElementInfo->elementHandle,
            szField, &fieldBool);

        if (retVal == IFX_SUCCESS)
        {
            retVal = IFXE_MapBool(pBoolMap, mapSize, szDefValue, pDest,
                fieldBool);
        }
    }

    else
    {
        /* Not found - do nothing */
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Special-case version of swprintf
    Will replace the first instance of %d in format with string version of num
    No other % modifiers will be altered, and output as-is
*/
static int replaceInt(IFX_WCHAR *strDest, size_t maxsize, const IFX_WCHAR *format, int num)
{
    const IFX_WCHAR *formatPtr = format;
    IFX_WCHAR *destPtr = strDest;
    IFX_WCHAR *maxDestPtr = strDest + maxsize;
    int replacementDone = 0; /* Flag so only the first %d is replaced */

    int retVal = -1; /* default return value is error */

    /* Check for sensible input */
    if (strDest && format)
    {
        /* Loop over the format string, writing to the destination as we go */
        while (*formatPtr != '\0' && destPtr < maxDestPtr)
        {
            if (!replacementDone && *formatPtr == '%') /* Format code found*/
            {
                /* Examine format code */
                formatPtr++;
                if (*formatPtr == 'd')
                {
                    char numStr[MAX_INT_STRING_LEN];
                    char* numPtr = numStr;

                    lc_itoa(num, numStr, 10);

                    /* Copy (narrow) itoa output to (wide) destination */
                    while (*numPtr != '\0' && destPtr < maxDestPtr)
                    {
                        // We can assume the input is ASCII
                        *destPtr++ = *numPtr++;
                    }

                    /* Move to next format char */
                    formatPtr++;

                    replacementDone = 1;
                }
                else
                {
                    /* If not a %d, output the % as normal */
                    *destPtr++ = '%';
                }
            }
            else
            {
                /* Copy single char */
                *destPtr++ = *formatPtr++;
            }
        }

        if (destPtr < maxDestPtr)
        {
            /* Terminate string, calculate return value */
            *destPtr = '\0';
            retVal = destPtr - strDest;
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Special-case version of swprintf
    Will replace the first instance of %f in format with string version of num
    No other % modifiers will be altered, and output as-is
*/
static int replaceFloat(IFX_WCHAR *strDest, size_t maxsize, const IFX_WCHAR *format, float num)
{
    const IFX_WCHAR *formatPtr = format;
    IFX_WCHAR *destPtr = strDest;
    IFX_WCHAR *maxDestPtr = strDest + maxsize;
    int replacementDone = 0; /* Flag so only the first %f is replaced */

    int retVal = -1; /* default return value is error */

    /* Check for sensible input */
    if (strDest && format)
    {
        /* Loop over the format string, writing to the destination as we go */
        while (*formatPtr != '\0' && destPtr < maxDestPtr)
        {
            if (!replacementDone && *formatPtr == '%') /* Format code found*/
            {
                /* Examine format code */
                formatPtr++;
                if (*formatPtr == 'f')
                {
                    char numStr[MAX_FLOAT_STRING_LEN];
                    char* numPtr = numStr;

                    lc_sprintf(numStr, "%f", num);

                    /* Copy (narrow) itoa output to (wide) destination */
                    while (*numPtr != '\0' && destPtr < maxDestPtr)
                    {
                        // We can assume the input is ASCII
                        *destPtr++ = *numPtr++;
                    }

                    /* Move to next format char */
                    formatPtr++;

                    replacementDone = 1;
                }
                else
                {
                    /* If not a %f, output the % as normal */
                    *destPtr++ = '%';
                }
            }
            else
            {
                /* Copy single char */
                *destPtr++ = *formatPtr++;
            }
        }

        if (destPtr < maxDestPtr)
        {
            /* Terminate string, calculate return value */
            *destPtr = '\0';
            retVal = destPtr - strDest;
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
    Special-case version of swprintf
    Will replace the first instance of %s in format with replacementStr
    No other % modifiers will be altered, and output as-is
*/
static int replaceString(IFX_WCHAR *strDest, size_t maxsize, const IFX_WCHAR *format, const IFX_WCHAR *replacementStr)
{
    const IFX_WCHAR *formatPtr = format;
    IFX_WCHAR *destPtr = strDest;
    IFX_WCHAR *maxDestPtr = strDest + maxsize;
    int replacementDone = 0; /* Flag so only the first %s is replaced */

    int retVal = -1; /* default return value is error */

    /* Check for sensible input */
    if (strDest && format && replacementStr)
    {
        /* Loop over the format string, writing to the destination as we go */
        while (*formatPtr != '\0' && destPtr < maxDestPtr)
        {
            if (!replacementDone && *formatPtr == '%') /* Format code found*/
            {
                /* Examine format code */
                formatPtr++;
                if (*formatPtr == 's')
                {
                    const IFX_WCHAR* strPtr = replacementStr;

                    /* Copy the replacement string into the output */
                    while (*strPtr != '\0' && destPtr < maxDestPtr)
                    {
                        *destPtr++ = *strPtr++;
                    }

                    /* Move to next format char */
                    formatPtr++;

                    replacementDone = 1;
                }
                else
                {
                    /* If not a %s, output the % as normal */
                    *destPtr++ = '%';
                }
            }
            else
            {
                /* Copy single char */
                *destPtr++ = *formatPtr++;
            }
        }

        if (destPtr < maxDestPtr)
        {
            /* Terminate string, calculate return value */
            *destPtr = '\0';
            retVal = destPtr - strDest;
        }
    }

    return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void* IFXE_AllocateMemoryUnsafe(size_t size)
{
    return LcAllocateMemoryExternal(size);
}

/*-------------------------------------------------------------------------*//**
*/
void IFXE_FreeMemoryUnsafe(void* memory)
{
    LcDeallocateMemoryExternal(memory);
}
