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
#ifndef IFXUI_INTEGRATION_H
#define IFXUI_INTEGRATION_H

#include "inflexionui/engine/ifxui_defs.h"
#include "ifxui_config.h"

/* Define any 'weak reference' macro available to loosen the linkage between engine
   and integration layer */
#if !defined(IFX_WEAK_REF)
    #define IFX_WEAK_REF(X) X
#endif

/*
    IFXI type definitions

    These are to be used within Engine / Integration layer, only.

*/

/* Field data types */
typedef enum
{
    IFXI_FIELDDATA_INT,
    IFXI_FIELDDATA_STRING,
    IFXI_FIELDDATA_TIME,
    IFXI_FIELDDATA_FLOAT,
    IFXI_FIELDDATA_BOOL
} IFXI_FIELDDATA_TYPE;


/* Field mode */
typedef enum
{
    IFXI_FIELD_MODE_INPUT,
    IFXI_FIELD_MODE_OUTPUT,
    IFXI_FIELD_MODE_INPUT_OUTPUT
} IFXI_FIELD_MODE;

/* Field scope */
typedef enum
{
    IFXI_FIELD_SCOPE_ELEMENT,
    IFXI_FIELD_SCOPE_ITEM,
    IFXI_FIELD_SCOPE_MENU,
    IFXI_FIELD_SCOPE_GLOBAL,
    IFXI_FIELD_SCOPE_LOCAL
} IFXI_FIELD_SCOPE;

typedef union tVariant_Type
{
    IFX_TIME  t;
    IFX_INT32 i;
    float     f;
    IFX_BOOL  b;
} IFXI_VARIANT_TYPE;


/*
    IFXI_API Internal Definitions.

    These are used by the internal plug-in interface only.
*/

/* IFXI_API dll entry point function names (for GetProcAddress) */
#define IFXI_FUNCNAME_INITIALIZE                        "IFXI_Initialize"
#define IFXI_FUNCNAME_SHUTDOWN                          "IFXI_ShutDown"
#define IFXI_FUNCNAME_VALIDATEGUID                      "IFXI_ValidateGUID"
#define IFXI_FUNCNAME_GETLINKTYPECOUNT                  "IFXI_GetLinkTypeCount"
#define IFXI_FUNCNAME_GETLINKTYPEDATA                   "IFXI_GetLinkTypeData"
#define IFXI_FUNCNAME_OPENMENU                          "IFXI_OpenMenu"
#define IFXI_FUNCNAME_CLOSEMENU                         "IFXI_CloseMenu"
#define IFXI_FUNCNAME_GETITEMCOUNT                      "IFXI_GetItemCount"
#define IFXI_FUNCNAME_GETFIELDSIZE                      "IFXI_GetFieldSize"
#define IFXI_FUNCNAME_GETFIELDDATA                      "IFXI_GetFieldData"
#define IFXI_FUNCNAME_EXECUTELINK                       "IFXI_ExecuteLink"
#define IFXI_FUNCNAME_CREATEELEMENT                     "IFXI_CreateElement"
#define IFXI_FUNCNAME_DESTROYELEMENT                    "IFXI_DestroyElement"
#define IFXI_FUNCNAME_ACTIVATEELEMENT                   "IFXI_ActivateElement"
#define IFXI_FUNCNAME_DEACTIVATEELEMENT                 "IFXI_DeactivateElement"
#define IFXI_FUNCNAME_SETELEMENTFOCUS                   "IFXI_SetElementFocus"
#define IFXI_FUNCNAME_UNSETELEMENTFOCUS                 "IFXI_UnsetElementFocus"
#define IFXI_FUNCNAME_PROCESSELEMENTKEYDOWNEVENT        "IFXI_ProcessElementKeyDownEvent"
#define IFXI_FUNCNAME_PROCESSELEMENTKEYUPEVENT          "IFXI_ProcessElementKeyUpEvent"
#define IFXI_FUNCNAME_GETELEMENTCARETPOSITION           "IFXI_GetElementCaretPosition"
#define IFXI_FUNCNAME_POSITIONELEMENT                   "IFXI_PositionElement"
#define IFXI_FUNCNAME_PROCESSELEMENTSTYLUSDOWNEVENT     "IFXI_ProcessElementStylusDownEvent"
#define IFXI_FUNCNAME_PAINTELEMENT                      "IFXI_PaintElement"
#define IFXI_FUNCNAME_CHANGEELEMENTMODE                 "IFXI_ChangeElementMode"
#define IFXI_FUNCNAME_EXCLUSIVITYSTATUSCHANGE           "IFXI_ExclusivityStatusChange"
#define IFXI_FUNCNAME_GETFIRSTACTIVEITEM                "IFXI_GetFirstActiveItem"
#define IFXI_FUNCNAME_SETACTIVEITEM                     "IFXI_SetActiveItem"
#define IFXI_FUNCNAME_GETEXCLUSIVITYCOUNT               "IFXI_GetExclusivityCount"
#define IFXI_FUNCNAME_GETFIELDINFO                      "IFXI_GetFieldInfo"
#define IFXI_FUNCNAME_GETFIELDRAW                       "IFXI_GetFieldRaw"
#define IFXI_FUNCNAME_SETFIELDRAW                       "IFXI_SetFieldRaw"
#define IFXI_FUNCNAME_GETFIELDSIZEFROMRAW               "IFXI_GetFieldSizeFromRaw"

/*
    IFXI_API Function Definitions.

    These (except for callback) are to be implemented by the plug-in creator.
*/

/*  Library services to plug-ins - implemented by the library, and may be called by plug-in */

#if defined(__cplusplus)
extern "C" {
#endif

IFX_RETURN_STATUS IFXE_Callback(
    IFX_HUI                 hIfx,
    IFX_CALLBACK_CODE       nCode,
    IFX_HELEMENT            hElement,
    IFX_HMENU               hMenu,
    IFX_INT32               item,
    const void             *pInput,
    void                   *pOutput);

IFX_RETURN_STATUS IFXE_RequestExclusivity(
    IFX_HUI                 hIfx,
    IFX_HEXCLUSIVITY        hExclusivity,
    IFX_UINT32              priority,
    IFX_UINT32              timeout);

IFX_RETURN_STATUS IFXE_ExclusivityPermitted(
    IFX_HUI                 hIfx,
    IFX_HEXCLUSIVITY        hExclusivity,
    IFX_INT32              *pResult);

IFX_RETURN_STATUS IFXE_ReleaseExclusivity(
    IFX_HUI                 hIfx,
    IFX_HEXCLUSIVITY        hExclusivity);

/* Plug-in initialization calls. */

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_Initialize(IFX_HUI hIfx, IFX_HIL *phSession));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ShutDown(IFX_HIL hSession));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ValidateGUID(IFX_HIL hSession, const IFX_WCHAR *szGUID, IFX_INT32 *pValid));

/* Plug-in registration calls. */

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_GetLinkTypeCount(IFX_HIL hSession, IFX_INT32 *pCount));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_GetLinkTypeData(IFX_HIL hSession, IFX_INT32 item, IFX_WCHAR *szLinkName, IFX_LINK_MODE *pLinkMode));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_GetExclusivityCount(IFX_HIL hSession, IFX_INT32 *pCount));

/* Menu plug-in API calls. */

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_OpenMenu(IFX_HIL hSession, IFX_HMENU* phMenu, const IFX_WCHAR *szLink));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_CloseMenu(IFX_HIL hSession, IFX_HMENU hMenu));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_GetItemCount(IFX_HIL hSession, IFX_HMENU hMenu, IFX_INT32 *pCount));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_GetFirstActiveItem(IFX_HIL hSession, IFX_HMENU hMenu, IFX_INT32 *pItem));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_SetActiveItem(IFX_HIL hSession, IFX_HMENU hMenu, IFX_INT32 itemIndex));

/* Field data API calls. */

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_GetFieldSize(IFX_HIL hSession, IFX_FIELD_TYPE type, IFX_HELEMENT hElement, IFX_HMENU hMenu, IFX_INT32 item, const IFX_WCHAR *szField, IFXI_FIELD_SCOPE *pScope, IFX_INT32 *pSize));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_GetFieldInfo(IFX_HIL hSession, IFX_HMENU hMenu, IFX_INT32 item, const IFX_WCHAR *szField, IFXI_FIELD_MODE *pMode, IFXI_FIELD_SCOPE *pScope, IFXI_FIELDDATA_TYPE *pFieldType));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_GetFieldRaw(IFX_HIL hSession, IFX_FIELD_TYPE type, IFX_HELEMENT hElement, IFX_HMENU hMenu, IFX_INT32 item, const IFX_WCHAR *szField, IFXI_FIELD_SCOPE *pScope, IFXI_VARIANT_TYPE *pValue));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_SetFieldRaw(IFX_HIL hSession, IFX_FIELD_TYPE type, IFX_HELEMENT hElement, IFX_HMENU hMenu, IFX_INT32 item, const IFX_WCHAR *szField, IFXI_VARIANT_TYPE value, IFX_INT32 final));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_GetFieldSizeFromRaw(IFX_HIL hSession, IFX_HMENU hMenu, IFX_INT32 item, const IFX_WCHAR *szField, IFXI_VARIANT_TYPE value, IFX_INT32 *pSize));

/* Link plug-in API calls. */

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ExecuteLink(IFX_HIL hSession, const IFX_WCHAR *szLink));

/* Graphical and event handler element API calls */

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_CreateElement( IFX_HIL hSession, IFX_HELEMENT *phElement, IFX_HMENU hMenu, IFX_INT32 item, const IFX_WCHAR *szLink,void* pProperty));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_DestroyElement(IFX_HIL hSession, IFX_HELEMENT hElement));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ActivateElement(IFX_HIL hSession, IFX_HELEMENT hElement));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_DeactivateElement(IFX_HIL hSession, IFX_HELEMENT hElement));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_SetElementFocus(IFX_HIL hSession, IFX_HELEMENT hElement));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_UnsetElementFocus(IFX_HIL hSession, IFX_HELEMENT hElement));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ProcessElementKeyDownEvent(IFX_HIL hSession, IFX_HELEMENT hElement, IFX_INT32 key, IFX_INT32 *pConsumed));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ProcessElementKeyUpEvent(IFX_HIL hSession, IFX_HELEMENT hElement, IFX_INT32 key));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_GetElementCaretPosition(IFX_HIL hSession, IFX_HELEMENT hElement, IFX_INT32 *pIndex));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_PositionElement(IFX_HIL hSession, IFX_HELEMENT hElement, IFX_POSITION *pPosition));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ProcessElementStylusDownEvent(IFX_HIL hSession, IFX_HELEMENT hElement, IFX_INT32 x,IFX_INT32 y, IFX_INT32 *pConsumed));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ProcessElementStylusUpEvent(IFX_HIL hSession, IFX_HELEMENT hElement, IFX_INT32 x, IFX_INT32 y));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ProcessElementStylusDragEvent(IFX_HIL hSession, IFX_HELEMENT hElement, IFX_INT32 x, IFX_INT32 y));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ProcessElementStylusCancelEvent(IFX_HIL hSession, IFX_HELEMENT hElement));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_PaintElement(IFX_HIL hSession, IFX_HELEMENT hElement));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ChangeElementMode(IFX_HIL hSession, IFX_HELEMENT hElement, IFX_ELEMENT_MODE newMode, void *pContext));

IFX_WEAK_REF(IFX_RETURN_STATUS IFXI_ExclusivityStatusChange(IFX_HIL hSession, IFX_HEXCLUSIVITY hExclusivity, IFX_EXCLUSIVITY_STATUS status));

IFX_RETURN_STATUS IFXI_GetFieldData(IFX_HIL hSession, IFX_FIELD_TYPE type, IFX_HELEMENT hElement, IFX_HMENU hMenu, IFX_INT32 item, const IFX_WCHAR *szField, IFX_WCHAR *pszData);
#if defined(__cplusplus)
}
#endif

/*
    IFXI_API dll function entry type definitions
*/
#if defined(__cplusplus)
extern "C" {
#endif

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_INITIALIZE)                (
    IFX_HUI                 hIfx,
    IFX_HIL                *phSession);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_SHUTDOWN)                  (
    IFX_HIL                 hSession);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_VALIDATEGUID)              (
    IFX_HIL                 hSession,
    const IFX_WCHAR        *szGUID,
    IFX_INT32              *pValid);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_GETLINKTYPECOUNT)          (
    IFX_HIL                 hSession,
    IFX_INT32              *pCount);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_GETLINKTYPEDATA)           (
    IFX_HIL                 hSession,
    IFX_INT32               item,
    IFX_WCHAR              *szLinkName,
    IFX_LINK_MODE          *pLinkMode);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_GETEXCLUSIVITYCOUNT)       (
    IFX_HIL                 hSession,
    IFX_INT32              *pCount);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_OPENMENU)                  (
    IFX_HIL                 hSession,
    IFX_HMENU              *phMenu,
    const IFX_WCHAR        *szLink);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_CLOSEMENU)                 (
    IFX_HIL                 hSession,
    IFX_HMENU               hMenu);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_GETITEMCOUNT)              (
    IFX_HIL                 hSession,
    IFX_HMENU               hMenu,
    IFX_INT32*              pCount);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_GETFIRSTACTIVEITEM)        (
    IFX_HIL                 hSession,
    IFX_HMENU               hMenu,
    IFX_INT32              *pItem);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_SETACTIVEITEM)        (
    IFX_HIL                 hSession,
    IFX_HMENU               hMenu,
    IFX_INT32               itemIndex);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_GETFIELDSIZE)              (
    IFX_HIL                 hSession,
    IFX_FIELD_TYPE          type,
    IFX_HELEMENT            hElement,
    IFX_HMENU               hMenu,
    IFX_INT32               item,
    const IFX_WCHAR        *szField,
    IFXI_FIELD_SCOPE       *pScope,
    IFX_INT32              *pSize);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_GETFIELDDATA)              (
    IFX_HIL                 hSession,
    IFX_FIELD_TYPE          type,
    IFX_HELEMENT            hElement,
    IFX_HMENU               hMenu,
    IFX_INT32               item,
    const IFX_WCHAR        *szField,
    IFX_WCHAR              *pszData);

typedef IFX_RETURN_STATUS (* LPFN_IFXI_GETFIELDINFO)                (
    IFX_HIL             hSession,
    IFX_HMENU           hMenu,
    IFX_INT32           item,
    const IFX_WCHAR    *szField,
    IFXI_FIELD_MODE    *pMode,
    IFXI_FIELD_SCOPE   *pScope,
    IFXI_FIELDDATA_TYPE *pFieldType);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_GETFIELDRAW)               (
    IFX_HIL                 hSession,
    IFX_FIELD_TYPE          type,
    IFX_HELEMENT            hElement,
    IFX_HMENU               hMenu,
    IFX_INT32               item,
    const IFX_WCHAR        *szField,
    IFXI_FIELD_SCOPE       *pScope,
    IFXI_VARIANT_TYPE      *pValue);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_SETFIELDRAW)               (
    IFX_HIL                 hSession,
    IFX_FIELD_TYPE          type,
    IFX_HELEMENT            hElement,
    IFX_HMENU               hMenu,
    IFX_INT32               item,
    const IFX_WCHAR        *szField,
    IFXI_VARIANT_TYPE       value,
    IFX_INT32               final);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_GETFIELDSIZEFROMRAW)       (
    IFX_HIL                 hSession,
    IFX_HMENU               hMenu,
    IFX_INT32               item,
    const IFX_WCHAR        *szField,
    IFXI_VARIANT_TYPE       value,
    IFX_INT32              *pSize);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_EXECUTELINK)               (
    IFX_HIL                 hSession,
    const IFX_WCHAR        *szLink);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_CREATEELEMENT)             (
    IFX_HIL                 hSession,
    IFX_HELEMENT           *phElement,
    IFX_HMENU               hMenu,
    IFX_INT32               item,
    const IFX_WCHAR        *szLink,
    void*                   pProperty);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_DESTROYELEMENT)            (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_ACTIVATEELEMENT)           (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_DEACTIVATEELEMENT)         (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_SETELEMENTFOCUS)           (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement);

typedef IFX_RETURN_STATUS  (* LPFN_IFXI_UNSETELEMENTFOCUS)          (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_PROCESSELEMENTKEYDOWNEVENT)    (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement,
    IFX_INT32               key,
    IFX_INT32              *pConsumed);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_PROCESSELEMENTKEYUPEVENT)  (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement,
    IFX_INT32               key);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_GETELEMENTCARETPOSITION)   (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement,
    IFX_INT32              *pIndex);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_POSITIONELEMENT)           (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement,
    IFX_POSITION           *pPosition);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_PAINTELEMENT)              (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_PROCESSELEMENTSTYLUSDOWNEVENT)         (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement,
    IFX_INT32               x,
    IFX_INT32               y,
    IFX_INT32              *pConsumed);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_PROCESSELEMENTSTYLUSUPEVENT)         (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement,
    IFX_INT32               x,
    IFX_INT32               y);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_PROCESSELEMENTSTYLUSDRAGEVENT)         (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement,
    IFX_INT32               x,
    IFX_INT32               y);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_PROCESSELEMENTSTYLUSCANCELEVENT)         (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_CHANGEELEMENTMODE)         (
    IFX_HIL                 hSession,
    IFX_HELEMENT            hElement,
    IFX_ELEMENT_MODE        newMode,
    void                   *pContext);

typedef IFX_RETURN_STATUS   (* LPFN_IFXI_EXCLUSIVITYSTATUSCHANGE)   (
    IFX_HIL                 hSession,
    IFX_HEXCLUSIVITY        hExclusivity,
    IFX_EXCLUSIVITY_STATUS  status);


#if defined(__cplusplus)
}
#endif

struct tagIfxiModuleInterface {
    LPFN_IFXI_INITIALIZE                    pIfxiInitialise;
    LPFN_IFXI_SHUTDOWN                      pIfxiShutdown;
    LPFN_IFXI_VALIDATEGUID                  pIfxiValidateGUID;
    LPFN_IFXI_GETLINKTYPECOUNT              pIfxiGetLinkTypeCount;
    LPFN_IFXI_GETLINKTYPEDATA               pIfxiGetLinkTypeData;
    LPFN_IFXI_GETEXCLUSIVITYCOUNT           pIfxiGetExclusivityCount;
    LPFN_IFXI_OPENMENU                      pIfxiOpenMenu;
    LPFN_IFXI_CLOSEMENU                     pIfxiCloseMenu;
    LPFN_IFXI_GETITEMCOUNT                  pIfxiGetItemCount;
    LPFN_IFXI_GETFIRSTACTIVEITEM            pIfxiGetFirstActiveItem;
    LPFN_IFXI_SETACTIVEITEM                 pIfxiSetActiveItem;
    LPFN_IFXI_GETFIELDSIZE                  pIfxiGetFieldSize;
    LPFN_IFXI_GETFIELDDATA                  pIfxiGetFieldData;
    LPFN_IFXI_GETFIELDINFO                  pIfxiGetFieldInfo;
    LPFN_IFXI_GETFIELDRAW                   pIfxiGetFieldRaw;
    LPFN_IFXI_SETFIELDRAW                   pIfxiSetFieldRaw;
    LPFN_IFXI_GETFIELDSIZEFROMRAW           pIfxiGetFieldSizeFromRaw;
    LPFN_IFXI_EXECUTELINK                   pIfxiExecuteLink;
    LPFN_IFXI_CREATEELEMENT                 pIfxiCreateElement;
    LPFN_IFXI_DESTROYELEMENT                pIfxiDestroyElement;
    LPFN_IFXI_ACTIVATEELEMENT               pIfxiActivateElement;
    LPFN_IFXI_DEACTIVATEELEMENT             pIfxiDeactivateElement;
    LPFN_IFXI_SETELEMENTFOCUS               pIfxiSetElementFocus;
    LPFN_IFXI_UNSETELEMENTFOCUS             pIfxiUnsetElementFocus;
    LPFN_IFXI_PROCESSELEMENTKEYDOWNEVENT    pIfxiProcessElementKeyDownEvent;
    LPFN_IFXI_PROCESSELEMENTKEYUPEVENT      pIfxiProcessElementKeyUpEvent;
    LPFN_IFXI_GETELEMENTCARETPOSITION       pIfxiGetElementCaretPosition;
    LPFN_IFXI_POSITIONELEMENT               pIfxiPositionElement;
    LPFN_IFXI_PROCESSELEMENTSTYLUSDOWNEVENT pIfxiProcessElementStylusDownEvent;
    LPFN_IFXI_PROCESSELEMENTSTYLUSUPEVENT   pIfxiProcessElementStylusUpEvent;
    LPFN_IFXI_PROCESSELEMENTSTYLUSDRAGEVENT pIfxiProcessElementStylusDragEvent;
    LPFN_IFXI_PROCESSELEMENTSTYLUSCANCELEVENT pIfxiProcessElementStylusCancelEvent;
    LPFN_IFXI_PAINTELEMENT                  pIfxiPaintElement;
    LPFN_IFXI_CHANGEELEMENTMODE             pIfxiChangeElementMode;
    LPFN_IFXI_EXCLUSIVITYSTATUSCHANGE       pIfxiExclusivityStatusChange;
};
typedef struct tagIfxiModuleInterface IFXI_MODULE_INTERFACE;


/**********************************
* INTEGRATION LAYER INTERNALS     *
**********************************/


/* TYPES */

/* Function typedefs for GetField IFXM calls */

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetElementFieldStringSize)(
        IFX_HMODULE         hModule,
        IFX_HREQUEST       *phRequest,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        IFX_INT32          *pSize);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetElementFieldStringData)(
        IFX_HMODULE         hModule,
        IFX_HREQUEST        phRequest,
        IFX_WCHAR          *pStr);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetElementFieldIntData)(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        IFX_INT32          *pInt);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetElementFieldFloatData)(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        float              *pFloat);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetElementFieldTimeData)(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        IFX_TIME           *pTime);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetElementFieldBoolData)(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        IFX_BOOL           *pBool);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetMenuPropertyStringSize)(
        IFX_HMODULE         hModule,
        IFX_HREQUEST       *phRequest,
        IFX_HMENU           hMenu,
        IFX_INT32          *pSize);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetMenuPropertyStringData)(
        IFX_HMODULE         hModule,
        IFX_HREQUEST        hRequest,
        IFX_WCHAR          *pStr);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetMenuFieldStringSize)(
        IFX_HMODULE         hModule,
        IFX_HREQUEST       *phRequest,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        IFX_INT32          *pSize);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetMenuFieldStringData)(
        IFX_HMODULE         hModule,
        IFX_HREQUEST        hRequest,
        IFX_WCHAR          *pStr);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetMenuFieldIntData)(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        IFX_INT32          *pInt);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetMenuFieldFloatData)(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        float              *pFloat);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetMenuFieldTimeData)(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        IFX_TIME           *pTime);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetMenuFieldBoolData)(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        IFX_BOOL           *pBool);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetGlobalFieldStringSize)(
        IFX_HMODULE         hModule,
        IFX_HREQUEST       *phRequest,
        IFX_INT32          *pSize);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetGlobalFieldStringData)(
        IFX_HMODULE         hModule,
        IFX_HREQUEST        hRequest,
        IFX_WCHAR          *pStr);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetGlobalFieldIntData)(
        IFX_HMODULE         hModule,
        IFX_INT32          *pInt);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetGlobalFieldFloatData)(
        IFX_HMODULE         hModule,
        float              *pFloat);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetGlobalFieldTimeData)(
        IFX_HMODULE         hModule,
        IFX_TIME           *pTime);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_GetGlobalFieldBoolData)(
        IFX_HMODULE         hModule,
        IFX_BOOL           *pBool);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_SetElementFieldIntData)(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        IFX_INT32           value,
        IFX_INT32           final);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_SetElementFieldFloatData)(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        float               value,
        IFX_INT32           final);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_SetElementFieldBoolData)(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        IFX_BOOL            value);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_SetMenuFieldIntData)(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        IFX_INT32           value,
        IFX_INT32           final);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_SetMenuFieldFloatData)(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        float               value,
        IFX_INT32           final);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_SetMenuFieldBoolData)(
        IFX_HMODULE         hModule,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        IFX_BOOL            value);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_SetGlobalFieldIntData)(
        IFX_HMODULE         hModule,
        IFX_INT32           value,
        IFX_INT32           final);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_SetGlobalFieldFloatData)(
        IFX_HMODULE         hModule,
        float               value,
        IFX_INT32           final);

typedef IFX_RETURN_STATUS (*LPFN_IFXM_SetGlobalFieldBoolData)(
        IFX_HMODULE         hModule,
        IFX_BOOL            value);


/**********************************
* STRUCTURES                      *
**********************************/

/* Internal string */
typedef struct
{
    IFX_WCHAR *data;            /* Dynamically allocated string data
                                   - at most one of data and constData may
                                   be non-NULL at any time              */
    IFX_BOOL ownsData;           /* Whether the structure owns
                                   the dynamic data                     */
    const IFX_WCHAR *constData; /* Constant string data
                                   - at most one of data and constData may
                                   be non-NULL at any time              */
} IFXI_STRING;

/* String-to-string map */
typedef struct
{
    const IFX_WCHAR *value;     /* Output value                         */
    const IFX_WCHAR *input;     /* Input value                          */
} IFXI_STRING_MAP;

/* Int-to-string map */
typedef struct
{
    const IFX_WCHAR *value;     /* Output value                         */
    IFX_INT32 from;                 /* Lowest input                         */
    IFX_INT32 to;                   /* Highest input                        */
} IFXI_INT_MAP;

/* Float-to-string map */
typedef struct
{
    const IFX_WCHAR *value;     /* Output value                         */
    float from;                 /* Lowest input                         */
    float to;                   /* Highest input                        */
} IFXI_FLOAT_MAP;

/* Bool-to-string map */
typedef struct
{
    const IFX_WCHAR *value;     /* Output value                         */
    IFX_BOOL input;             /* Input value                          */
} IFXI_BOOL_MAP;

/* Menu info */
typedef struct
{
    IFX_INT32 menuType;             /* Type of menu (module, prefix)        */
    IFX_HMENU menuHandle;       /* Module-chosen handle                 */
} IFXI_MODULE_MENU_INFO;

/* Element info */
typedef struct
{
    IFX_INT32 elementType;          /* Type of element (module, prefix)     */
    IFX_HELEMENT elementHandle; /* Module-chosen handle                 */
} IFXI_MODULE_ELEMENT_INFO;

/* Buffer info */
typedef struct
{
    IFX_INT32 bufferType;           /* Type of buffer (Module, field)       */
    IFX_HBUFFER bufferHandle;   /* Module-chosen handle                 */
} IFXI_MODULE_BUFFER_INFO;

/* Forward declaration of IFXI_SESSION */
struct tagIFXI_SESSION;
typedef struct tagIFXI_SESSION IFXI_SESSION;

/* Module session struct */
struct tagIFXI_MODULE_SESSION
{
       IFX_INT32 module;            /* Module number                        */
       IFXI_SESSION *ils;         /* Ptr to owning IFXI_SESSION             */
};
typedef struct tagIFXI_MODULE_SESSION IFXI_MODULE_SESSION;


/* URI helpers */
IFX_WCHAR* IFXE_GetUriStringParam(
        IFX_HUTILITY        hSession,
        const IFX_WCHAR    *szParam,
        const IFX_WCHAR    *pDefaultValue);
IFX_WCHAR* IFXE_GetUriBody(
        IFX_HUTILITY        hSession,
        const IFX_WCHAR    *pDefaultValue);

/* Mapping functions */
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
        IFX_HMODULE                         hModule);

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
        IFX_INT32                           item);

IFX_RETURN_STATUS IFXE_MapElementField(
        LPFN_IFXM_GetElementFieldStringSize sizeStrFn,
        LPFN_IFXM_GetElementFieldStringData getStrFn,
        LPFN_IFXM_GetElementFieldIntData    getIntFn,
        LPFN_IFXM_GetElementFieldFloatData  getFloatFn,
        LPFN_IFXM_GetElementFieldTimeData   getTimeFn,
        LPFN_IFXM_GetElementFieldBoolData   getBoolFn,
        const IFXI_STRING_MAP              *pStringMap,
        const IFXI_INT_MAP                 *pIntMap,
        const IFXI_FLOAT_MAP               *pFloatMap,
        const IFXI_BOOL_MAP                *pBoolMap,
        IFX_INT32                           mapSize,
        const IFX_WCHAR                    *szDefValue,
        IFXI_STRING                        *pDest,
        IFX_HMODULE                         hModule,
        IFXI_MODULE_ELEMENT_INFO           *pElementInfo,
        IFXI_FIELDDATA_TYPE                 fieldType,
        const IFX_WCHAR                    *szField);

IFX_RETURN_STATUS IFXE_MapInt(
        const IFXI_INT_MAP      pMap[],
        IFX_INT32               sizeMap,
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        IFX_INT32               src);

IFX_RETURN_STATUS IFXE_MapFloat(
        const IFXI_FLOAT_MAP    pMap[],
        IFX_INT32               sizeMap,
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        float                   src);

IFX_RETURN_STATUS IFXE_MapBool(
        const IFXI_BOOL_MAP     pMap[],
        IFX_INT32               sizeMap,
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        IFX_BOOL                src);

IFX_RETURN_STATUS IFXE_MapTime(
        const IFX_WCHAR        *szDefValue,
        IFXI_STRING            *pDest,
        IFX_TIME                pSrc);

/* String-getters */
IFX_RETURN_STATUS IFXE_GetMenuString(
        LPFN_IFXM_GetMenuPropertyStringSize sizeStrFn,
        LPFN_IFXM_GetMenuPropertyStringData getStrFn,
        IFX_HMODULE                         hModule,
        IFX_HMENU                           hMenu,
        IFXI_STRING                        *pDest);
IFX_RETURN_STATUS IFXE_GetMenuItemString(
        LPFN_IFXM_GetMenuFieldStringSize    sizeStrFn,
        LPFN_IFXM_GetMenuFieldStringData    getStrFn,
        IFX_HMODULE                         hModule,
        IFX_HMENU                           hMenu,
        IFX_INT32                           item,
        IFXI_STRING                        *pDest);

/* General string functions*/
IFX_RETURN_STATUS   IFXE_StringAlloc(IFXI_STRING *pStr, IFX_INT32 length);
void                IFXE_StringFree(IFXI_STRING *pStr);
void                IFXE_StringInit(IFXI_STRING *pStr);
void                IFXE_StringAssign(IFXI_STRING *pDest, IFX_WCHAR *szSrc, IFX_BOOL transferOwnership);
void                IFXE_StringAssignConst(IFXI_STRING *pDest, const IFX_WCHAR *szSrc);
void                IFXE_StringCopy(IFXI_STRING *pDest, IFXI_STRING *pSrc, IFX_BOOL transferOwnership);
const IFX_WCHAR    *IFXE_StringData(const IFXI_STRING *pStr);
IFX_WCHAR          *IFXE_StringDataWritable(const IFXI_STRING *pStr);

/* IFXI_SESSION accessors */

IFX_WEAK_REF(IFX_HUI IFXI_HuiFromSession(IFXI_SESSION* pSession));
IFX_WEAK_REF(IFXI_STRING* IFXI_CacheStrFromSession(IFXI_SESSION* pSession));

/* General utilities */
IFXI_SESSION*           IFXE_SessionFromHIL         (IFX_HIL hil);
IFXI_SESSION*           IFXE_SessionFromHModuleID   (IFX_HMODULEID hModuleId);
IFX_INT32               IFXE_GetModule              (IFX_INT32 handle);

#if defined(__cplusplus)
extern "C" {
#endif

/* Memory allocation */
void*                   IFXE_AllocateMemoryUnsafe(size_t size);
void                    IFXE_FreeMemoryUnsafe(void* memory);

#if defined(__cplusplus)
}
#endif


#endif /* IFXUI_INTEGRATION_H */
