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

#ifndef IFXUI_CATCHALL_H
#define IFXUI_CATCHALL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "inflexionui/engine/ifxui_module_integration.h"

IFX_RETURN_STATUS IFXM_Initialize(
        IFX_HMODULEID       hModuleId,
        IFX_HMODULE        *phModule);

IFX_RETURN_STATUS IFXM_ShutDown(
        IFX_HMODULE         hModule);

IFX_RETURN_STATUS IFXM_GetLinkTypeCount(
        IFX_HMODULE         hModule,
        IFX_INT32          *pCount);

IFX_RETURN_STATUS IFXM_GetLinkTypeData(
        IFX_HMODULE         hModule,
        IFX_INT32               item,
        IFX_WCHAR          *szLinkType,
        IFX_LINK_MODE      *pLinkMode);

IFX_RETURN_STATUS IFXM_OpenMenu(
        IFX_HMODULE         hModule,
        IFX_HMENUID         hMenuId,
        IFX_HMENU          *phMenu,
        const IFX_WCHAR    *szLink);

IFX_RETURN_STATUS IFXM_CloseMenu(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu);

IFX_RETURN_STATUS IFXM_GetItemCount(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_INT32          *pCount);

IFX_RETURN_STATUS IFXM_GetFirstActiveItem(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_INT32          *pItem);

IFX_RETURN_STATUS IFXM_SetActiveItem(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_INT32           itemIndex);

IFX_RETURN_STATUS IFXM_ExecuteLink(
        IFX_HMODULE         hModule,
        const IFX_WCHAR    *szLink);

IFX_RETURN_STATUS IFXM_CreateElement(
        IFX_HMODULE         hModule,
        IFX_HELEMENTID      hElementId,
        IFX_HELEMENT       *phElement,
        IFX_HMENUID         hMenuId,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        IFX_ELEMENT_PROPERTY *pProperty,
        const IFX_WCHAR    *szLink);

IFX_RETURN_STATUS IFXM_DestroyElement(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement);

IFX_RETURN_STATUS IFXM_ActivateElement(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement);

IFX_RETURN_STATUS IFXM_DeactivateElement(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement);

IFX_RETURN_STATUS IFXM_SetElementFocus(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement);

IFX_RETURN_STATUS IFXM_UnsetElementFocus(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement);

IFX_RETURN_STATUS IFXM_ProcessElementKeyUpEvent(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        IFX_INT32           key);

IFX_RETURN_STATUS IFXM_ProcessElementKeyDownEvent(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        IFX_INT32           key,
        IFX_INT32          *pConsumed);

IFX_RETURN_STATUS IFXM_ProcessElementStylusDragEvent(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        IFX_INT32           x,
        IFX_INT32           y);

IFX_RETURN_STATUS IFXM_ProcessElementStylusUpEvent(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        IFX_INT32           x,
        IFX_INT32           y);

IFX_RETURN_STATUS IFXM_ProcessElementStylusDownEvent(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        IFX_INT32           x,
        IFX_INT32           y,
        IFX_INT32          *pConsumed);

IFX_RETURN_STATUS IFXM_ProcessElementStylusCancelEvent(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement);

IFX_RETURN_STATUS IFXM_GetElementCaretPosition(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        IFX_INT32          *pIndex);

IFX_RETURN_STATUS IFXM_PositionElement(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        IFX_POSITION       *pPosition);

IFX_RETURN_STATUS IFXM_PaintElement(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement);

IFX_RETURN_STATUS IFXM_ChangeElementMode(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        IFX_ELEMENT_MODE    newMode,
        void*               pContext);

IFX_RETURN_STATUS IFXM_BufferCreateString(
        IFX_HMODULE         hModule,
        IFX_HBUFFER        *phBuffer,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        const IFX_WCHAR    *szField,
        IFX_WCHAR         **pBuffer,
        IFX_INT32           maxLength);

IFX_RETURN_STATUS IFXM_BufferDestroy(
        IFX_HMODULE         hModule,
        IFX_HBUFFER         hBuffer);

IFX_RETURN_STATUS IFXM_BufferSetFocus(
        IFX_HMODULE         hModule,
        IFX_HBUFFER         hBuffer);

IFX_RETURN_STATUS IFXM_BufferUnsetFocus(
        IFX_HMODULE         hModule,
        IFX_HBUFFER         hBuffer);

IFX_RETURN_STATUS IFXM_GetFieldStringSize(
        IFX_HMODULE         hModule,
        IFX_HREQUEST       *phRequest,
        IFX_HELEMENT        hElement,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        const IFX_WCHAR    *szField,
        IFX_INT32          *pSize);

IFX_RETURN_STATUS IFXM_GetFieldStringData(
        IFX_HMODULE         hModule,
        IFX_HREQUEST        hRequest,
        IFX_WCHAR          *szData);

IFX_RETURN_STATUS IFXM_GetMenuNameSize(
        IFX_HMODULE         hModule,
        IFX_HREQUEST       *phRequest,
        IFX_HMENU           hMenu,
        IFX_INT32          *pSize);

IFX_RETURN_STATUS IFXM_GetMenuNameData(
        IFX_HMODULE         hModule,
        IFX_HREQUEST        hRequest,
        IFX_WCHAR          *szData);

IFX_RETURN_STATUS IFXM_GetSortFieldSize(
        IFX_HMODULE         hModule,
        IFX_HREQUEST       *phRequest,
        IFX_HMENU           hMenu,
        IFX_INT32          *pSize);

IFX_RETURN_STATUS IFXM_GetSortFieldData(
        IFX_HMODULE         hModule,
        IFX_HREQUEST        hRequest,
        IFX_WCHAR          *szData);

IFX_RETURN_STATUS IFXM_GetSortDirection(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_SORT_DIRECTION *pDirection);

IFX_RETURN_STATUS IFXM_GetSortType(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_SORT_TYPE      *pType);

IFX_RETURN_STATUS IFXM_GetLoadOnDemand(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_INT32          *pLoadOnDemand);

IFX_RETURN_STATUS IFXM_GetPlaceholderSize(
        IFX_HMODULE         hModule,
        IFX_HREQUEST       *phRequest,
        IFX_HMENU           hMenu,
        IFX_INT32          *pSize);

IFX_RETURN_STATUS IFXM_GetPlaceholderData(
        IFX_HMODULE         hModule,
        IFX_HREQUEST        hRequest,
        IFX_WCHAR          *szData);

IFX_RETURN_STATUS IFXM_GetItemLinkSize(
        IFX_HMODULE         hModule,
        IFX_HREQUEST       *phRequest,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        IFX_INT32          *pSize);

IFX_RETURN_STATUS IFXM_GetItemLinkData(
        IFX_HMODULE         hModule,
        IFX_HREQUEST        hRequest,
        IFX_WCHAR          *szData);

IFX_RETURN_STATUS IFXM_ExclusivityStatusChange(
        IFX_HMODULE         hModule,
        IFX_EXCLUSIVITY_STATUS status);

IFX_RETURN_STATUS IFXI_RequestExclusivity(
        IFX_HMODULEID       hModuleId,
        IFX_INT32           priority,
        IFX_INT32           timeout);

IFX_RETURN_STATUS IFXI_ReleaseExclusivity(
        IFX_HMODULEID       hModuleId);

#ifdef __cplusplus
}
#endif
#endif /* IFXUI_CATCHALL_H */
