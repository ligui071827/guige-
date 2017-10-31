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

#ifndef IFXUI_MODULE_INTEGRATION_H
#define IFXUI_MODULE_INTEGRATION_H

#include "inflexionui/engine/ifxui_defs.h"
#include "ifxui_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QUICKSTART_INF {\
						\
	return IFX_SUCCESS;\
}
/* Define any 'weak reference' macro available to loosen the linkage between engine
   and integration layer */
#if !defined(IFX_WEAK_REF)
    #define IFX_WEAK_REF(X) X
#endif

/* Function Prototypes  - weak referenced as implemented in the integration layer */
IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_BufferCreateString(\
        IFX_HMODULEID       hModuleId,\
        IFX_HBUFFERID      *phBufferId,\
        IFX_HMENUID         hMenuId,\
        IFX_INT32           item,\
        const IFX_WCHAR    *szField,
        IFX_WCHAR         **pBuffer,\
        IFX_INT32           maxLength)\
);

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_BufferSetFocus(\
        IFX_HMODULEID       hModuleId,\
        IFX_HBUFFERID       hBufferId)\
);

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_BufferUnsetFocus(\
        IFX_HMODULEID       hModuleId,\
        IFX_HBUFFERID       hBufferId)\
);

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_BufferDestroy(\
        IFX_HMODULEID       hModuleId,\
        IFX_HBUFFERID       hBufferId)\
);

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_RequestFieldRefresh(\
        IFX_HMODULEID       hModuleId,\
        IFX_HMENUID         hMenuId,\
        IFX_INT32           item,\
        const IFX_WCHAR    *szField)\
);

/* Function Prototypes  - implemented directly in the engine */
IFX_RETURN_STATUS IFXI_RequestExecuteLink(
        IFX_HMODULEID       hModuleId,
        const IFX_WCHAR    *szUri);

IFX_RETURN_STATUS IFXI_RequestMenuRefresh(
        IFX_HMODULEID       hModuleId,
        IFX_HMENUID         hMenuId);

IFX_RETURN_STATUS IFXI_RequestQueryActiveItem(
        IFX_HMODULEID       hModuleId,
        IFX_HMENUID         hMenuId,
        IFX_INT32          *pItem);

IFX_RETURN_STATUS IFXI_RequestSetActiveItem(
        IFX_HMODULEID       hModuleId,
        IFX_HMENUID         hMenuId,
        IFX_INT32           item);

IFX_RETURN_STATUS IFXI_RequestRefreshBufferedElement(
        IFX_HMODULEID       hModuleId,
        IFX_HELEMENTID      hElementId);

IFX_RETURN_STATUS IFXI_RequestResizeBufferedElementBuffer(
        IFX_HMODULEID       hModuleId,
        IFX_HELEMENTID      hElementId,
        IFX_INT32           x,
        IFX_INT32           y);

IFX_RETURN_STATUS IFXI_RequestResourceSearch(
        IFX_HMODULEID       hModuleId,
        IFX_HMENUID         hMenuId,
        const IFX_WCHAR    *szResource,
        IFX_WCHAR          *szFullPath);

IFX_RETURN_STATUS IFXI_RequestFullScreenStart(
        IFX_HMODULEID       hModuleId,
        IFX_HELEMENTID      hElementId);

IFX_RETURN_STATUS IFXI_RequestFullScreenStop(
        IFX_HMODULEID       hModuleId,
        IFX_HELEMENTID      hElementId);

IFX_RETURN_STATUS IFXI_RequestTriggerKey(
        IFX_HMODULEID       hModuleId,
        IFX_INT32           scancode);

IFX_RETURN_STATUS IFXI_RequestCloseMenu(
        IFX_HMODULEID       hModuleId,
        IFX_HMENUID         hMenuId);

#ifdef __cplusplus
}
#endif

#endif /* !def IFXUI_MODULE_INTEGRATION_H */

