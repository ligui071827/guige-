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

/* This also defines the platform */
#include "inflexionui/engine/ifxui_defs.h"
#include "inflexionui/engine/ifxui_rtl.h"

#include "inflexionui/engine/ifxui_module_integration.h"
#include "inflexionui/modules/simpleedit/ifxui_simpleedit.h"
#include "inc/simpleedit_internals.h"
#include "inc/simpleedit_multitap.h"
#include "inc/simpleedit_platform.h"

/* String Literal of available styles */
const IFX_WCHAR* S_PW_STRING                            = L"password";
const IFX_WCHAR* S_TEXT_STRING                          = L"text";
const IFX_WCHAR* S_PHONE_STRING                         = L"phone";
const IFX_WCHAR* S_NUMERIC_STRING                       = L"numeric";

/* String Literal of input modes supported */
const IFX_WCHAR* IM_AUTOCAPWORD_STRING                  = L"autoCapWord";
const IFX_WCHAR* IM_AUTOCAPSENTENCE_STRING              = L"autoCapSentence";
const IFX_WCHAR* IM_LOWERCASE_STRING                    = L"lowercase";
const IFX_WCHAR* IM_UPPERCASE_STRING                    = L"uppercase";
const IFX_WCHAR* IM_NUMERIC_STRING                      = L"numeric";

const IFX_WCHAR* FIELD_INPUTMODE                        = L"simpleEditInputMode";

const IFX_WCHAR  PASSWORD_HIDE_CHAR                     = L'*';

#define MAX_INPUT_MODES                     5
#define SIMPLE_EDIT_LINK_COUNT              1

const IFX_INT32 INPUT_MODE_ARRAY[MAX_INPUT_MODES] = {   SIMPLE_EDIT_INPUT_AUTOCAPWORD,
                                                        SIMPLE_EDIT_INPUT_AUTOCAPSENTENCE,
                                                        SIMPLE_EDIT_INPUT_LOWERCASE,
                                                        SIMPLE_EDIT_INPUT_UPPERCASE,
                                                        SIMPLE_EDIT_INPUT_NUMERIC};

/**********************************
* INTERNAL FUNCTIONS             *
*********************************/

static IFX_RETURN_STATUS    SimpleEdit_GetDefaultInputMode( SIMPLE_EDIT*        pElement,
                                                            IFX_INT32*          recommendedInputMode);

static IFX_RETURN_STATUS    SimpleEdit_ValidateInputMode(   SIMPLE_EDIT*        pElement,
                                                            IFX_INT32           givenInputMode,
                                                            IFX_INT32*          recommendedInputMode);

static IFX_RETURN_STATUS    SimpleEdit_processStyle(        SIMPLE_EDIT*        pElement,
                                                            const IFX_WCHAR*    szValue);

static IFX_RETURN_STATUS    SimpleEdit_processInputMode(    SIMPLE_EDIT*        pElement,
                                                            const IFX_WCHAR*    szValue);

static IFX_RETURN_STATUS    SimpleEdit_FillMissingInfo(     SIMPLE_EDIT*        pElement);

/**********************************
* SimpleEdit API Functions       *
*********************************/

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_Initialize(
        IFX_HMODULEID       hModuleId,
        IFX_HMODULE        *phModule)
{
    SIMPLE_EDIT_SESSION* pLocalSession;

    SimpleEdit_AllocateMemoryUnsafe(sizeof(SIMPLE_EDIT_SESSION), &pLocalSession);
    if (pLocalSession != NULL)
    {
        pLocalSession->hModID       = hModuleId;
        pLocalSession->lastKey      = CONTROL_KEY;
        pLocalSession->pCurrentElement = NULL;
        *phModule = (IFX_HMODULE)pLocalSession;

        return IFX_SUCCESS;
    }

    return IFX_ERROR;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_ShutDown(
        IFX_HMODULE         hModule)
{
    SIMPLE_EDIT_SESSION* pLocalSession = (SIMPLE_EDIT_SESSION*)hModule;

    if (pLocalSession != NULL)
    {
        SimpleEdit_FreeMemoryUnsafe(pLocalSession);
    }

    return IFX_SUCCESS;
}

/* Menus */
/* Field Data */
IFX_RETURN_STATUS IFXM_ifxuisimpleedit_GetFieldStringSize_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HREQUEST       *phRequest,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        IFX_INT32          *pSize)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    SIMPLE_EDIT_SESSION* pLocalSession = (SIMPLE_EDIT_SESSION*)hModule;
    SIMPLE_EDIT* pLocalElement = (SIMPLE_EDIT*)hElement;

    if (pLocalSession && pLocalElement)
    {
        if (lc_wcscmpi(pLocalElement->szFieldName, szField) == 0)
        {
            IFX_WCHAR* strBuffer = NULL;

            *pSize = (lc_wcslen(pLocalElement->pBuffer) + 1) * sizeof(IFX_WCHAR);

            if (!(pLocalElement->style & SIMPLE_EDIT_STYLE_PASSWORD))
            {
                *pSize += pLocalElement->specialChars * sizeof(IFX_WCHAR);
            }

            SimpleEdit_AllocateMemoryUnsafe(*pSize, &strBuffer);
            if (strBuffer != NULL)
            {
                if (pLocalElement->style & SIMPLE_EDIT_STYLE_PASSWORD)
                {
                    IFX_INT32 pwdLen = lc_wcslen(pLocalElement->pBuffer);
                    IFX_INT32 i = 0;

                    for (; i < pwdLen; i++)
                    {
                        strBuffer[i] = PASSWORD_HIDE_CHAR;
                    }

                    strBuffer[pwdLen] = '\0';
                }
                else
                {
                    IFX_WCHAR* dst = strBuffer;
                    IFX_WCHAR* src = pLocalElement->pBuffer;

                    while(*src != 0)
                    {
                        if (SimpleEdit_CheckSpecialChar(*src) == IFX_TRUE)
                        {
                            *dst++ = '$';
                        }

                        *dst++ = *src++;
                    }

                    *dst = '\0';
                }

                *phRequest = strBuffer;
                status = IFX_SUCCESS;
            }
            else
            {
                status = IFX_ERROR_MEMORY;
            }
        }
    }

    return status;
}


IFX_RETURN_STATUS IFXM_ifxuisimpleedit_GetFieldIntData_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        IFX_INT32          *pData)
{
    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_GetFieldFloatData_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        float              *pData)
{
    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_GetFieldTimeData_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        IFX_TIME           *pData)
{
    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_GetFieldBoolData_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        const IFX_WCHAR    *szField,
        IFX_BOOL           *pData)
{
    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_GetFieldIntData_simpleEditInputMode(
        IFX_HMODULE         hModule,
        IFX_INT32          *pData)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    SIMPLE_EDIT_SESSION* pLocalSession = (SIMPLE_EDIT_SESSION*)hModule;

    if (pLocalSession)
    {
        if (pLocalSession->pCurrentElement)
        {
            *pData = pLocalSession->pCurrentElement->inputMode;
        }
        else
        {
            *pData = 0;
        }

        status = IFX_SUCCESS;
    }

    return status;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_GetFieldStringData(
        IFX_HMODULE         hModule,
        IFX_HREQUEST        hRequest,
        IFX_WCHAR          *szData)
{
    lc_wcscpy(szData, (IFX_WCHAR*)hRequest);
    SimpleEdit_FreeMemoryUnsafe((IFX_WCHAR*)hRequest);

    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_ExecuteLink_cycleInputMode(
        IFX_HMODULE         hModule)
{
    SIMPLE_EDIT_SESSION* pLocalSession = (SIMPLE_EDIT_SESSION*)hModule;

    if (pLocalSession)
    {
        if (pLocalSession->pCurrentElement)
        {
            /* Cycle the input mode on the focused element */
            SimpleEdit_CycleInputMode(pLocalSession, pLocalSession->pCurrentElement);
        }
    }

    return IFX_SUCCESS;
}


IFX_RETURN_STATUS IFXM_ifxuisimpleedit_ExecuteLink_showOSK(
        IFX_HMODULE         hModule){

    return IFX_ERROR;

}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_ExecuteLink_hideOSK(
        IFX_HMODULE         hModule){

    return IFX_ERROR;

}


/* Element functions */
IFX_RETURN_STATUS IFXM_ifxuisimpleedit_CreateElement_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT       *phElement,
        IFX_HMENUID         hMenuId,
        IFX_HMENU           hMenu,
        IFX_INT32           item,
        const IFX_WCHAR    *body,
        const IFX_WCHAR    *paramStyle,
        IFX_INT32           paramMaxLength,
        const IFX_WCHAR    *paramInputMode,
        IFX_INT32           paramCaretPosition)
{
    SIMPLE_EDIT_SESSION* pLocalSession = (SIMPLE_EDIT_SESSION*)hModule;
    SIMPLE_EDIT *edit;
    IFX_INT32 len = 0;
    IFX_WCHAR* src = NULL;

    if (!body)
    {
        return IFX_ERROR;
    }

    SimpleEdit_AllocateMemoryUnsafe(sizeof(SIMPLE_EDIT), &edit);
    if (edit != NULL)
    {
        /* Set the item value in the data structure */
        edit->item              = item;
        edit->hMenuId           = hMenuId;
        edit->inputMode         = 0;
        edit->style             = 0;
        edit->szFieldName       = NULL;
        edit->specialChars      = 0;

        edit->maxLength = paramMaxLength;
        // Boundary check
        if (edit->maxLength < 0)
        {
            edit->maxLength = 0;
        }

        edit->caretPosition = paramCaretPosition;
        // Boundary check
        if (edit->caretPosition < -1)
        {
            edit->caretPosition = -1;
        }
        else if (edit->caretPosition > edit->maxLength)
        {
            edit->caretPosition = edit->maxLength;
        }

        SimpleEdit_processInputMode(edit, paramInputMode);
        SimpleEdit_processStyle(edit, paramStyle);

        // Store the field name
        SimpleEdit_AllocateMemoryUnsafe((lc_wcslen(body) + 1) * sizeof(IFX_WCHAR), &(edit->szFieldName));
        if (edit->szFieldName == NULL)
        {
            SimpleEdit_FreeMemoryUnsafe(edit);

            return IFX_ERROR;
        }
        lc_wcscpy(edit->szFieldName, body);


        /* Get the string buffer to edit from the owner module via the IFXM API*/
        if (IFX_SUCCESS != IFXI_BufferCreateString(pLocalSession->hModID, &(edit->bufferID), hMenuId, item, body,  &(edit->pBuffer), paramMaxLength))
        {
            SimpleEdit_FreeMemoryUnsafe(edit->szFieldName);
            SimpleEdit_FreeMemoryUnsafe(edit);

            return IFX_ERROR;
        }

        src = edit->pBuffer;
        edit->specialChars = 0;

        while(*src++ != 0)
        {
            if (SimpleEdit_CheckSpecialChar(*src) == IFX_TRUE)
            {
                edit->specialChars++;
            }
        }

        len = lc_wcslen(edit->pBuffer);
        if (edit->caretPosition == -1 || len < edit->caretPosition )
        {
            /* Update the caret position just in case there is a default string */
            edit->caretPosition = len;
        }

        SimpleEdit_FillMissingInfo(edit);

        /* Store the handle in the out variable */
        *phElement = (IFX_HELEMENT*)edit;

        return IFX_SUCCESS;
    }

    return IFX_ERROR;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_DestroyElement_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement)
{
    SIMPLE_EDIT_SESSION* pLocalSession = (SIMPLE_EDIT_SESSION*)hModule;
    SIMPLE_EDIT* pElementSession = (SIMPLE_EDIT*)hElement;

    if (pElementSession)
    {
        if (pLocalSession)
        {
            /* Ask for the string buffer to be destroyed */
            IFXI_BufferDestroy(pLocalSession->hModID, pElementSession->bufferID);
        }

        SimpleEdit_FreeMemoryUnsafe(pElementSession->szFieldName);

        SimpleEdit_FreeMemoryUnsafe(pElementSession);
    }

    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_ActivateElement_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement)
{
    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_DeactivateElement_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement)
{
    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_SetElementFocus_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement)
{
    SIMPLE_EDIT_SESSION* pLocalSession = (SIMPLE_EDIT_SESSION*)hModule;
    SIMPLE_EDIT* pElementSession = (SIMPLE_EDIT*)hElement;

    IFX_HMODULEID modId = NULL;
    IFX_HBUFFERID bufId = NULL;

    if (pLocalSession)
    {
        IFX_INT32 inputModeChanged = IFX_TRUE;

        if (pElementSession && pLocalSession->pCurrentElement)
        {
            if (pElementSession->inputMode == pLocalSession->pCurrentElement->inputMode)
                inputModeChanged = IFX_FALSE;
        }

        pLocalSession->pCurrentElement = pElementSession;

        if (inputModeChanged)
        {
            /* Let the Engine indicator know the input mode has changed */
            (void)IFXI_RequestFieldRefresh(pLocalSession->hModID, NULL, -1, FIELD_INPUTMODE);
        }
    }

    if (pLocalSession)
    {
        modId = pLocalSession->hModID;
    }

    if (pElementSession)
    {
        bufId = pElementSession->bufferID;
    }

    /* Inform the owner module */
    return IFXI_BufferSetFocus(modId, bufId);
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_UnsetElementFocus_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement)
{
    SIMPLE_EDIT_SESSION* pLocalSession = (SIMPLE_EDIT_SESSION*)hModule;
    SIMPLE_EDIT* pElementSession = (SIMPLE_EDIT*)hElement;
    IFX_HMODULEID modId = NULL;
    IFX_HBUFFERID bufId = NULL;

    if (pLocalSession)
    {
        pLocalSession->pCurrentElement = NULL;
        modId = pLocalSession->hModID;
    }

    if (pElementSession)
    {
        bufId = pElementSession->bufferID;
    }

    /* Inform the owner module */
    return IFXI_BufferUnsetFocus(modId, bufId);
}


IFX_RETURN_STATUS IFXM_ifxuisimpleedit_ProcessElementKeyUpEvent_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        IFX_INT32           key)
{
    return IFX_SUCCESS;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_ProcessElementKeyDownEvent_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        IFX_INT32           key,
        IFX_INT32          *pConsumed)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    SIMPLE_EDIT_SESSION* pLocalSession  = (SIMPLE_EDIT_SESSION*)hModule;
    SIMPLE_EDIT* pEditSession           = (SIMPLE_EDIT*)hElement;
    IFX_INT32 callback                  = IFX_TRUE;

    if (pLocalSession && pEditSession)
    {
        /*  To support different key layout systems,
            simply call you own process element event
            function here.  You must create a new source
            file based on simpleEditMultiTap.c with the
            logic for your key layout                   */

        status = SimpleEdit_ProcessElementMultiTapKeyEvent(pLocalSession, pEditSession, key, pConsumed, &callback);

        if (status == IFX_SUCCESS && callback == IFX_TRUE)
        {
            (void)IFXI_RequestFieldRefresh(pLocalSession->hModID,
                                        pEditSession->hMenuId,
                                        pEditSession->item,
                                        pEditSession->szFieldName);
        }
    }

    return status;
}

IFX_RETURN_STATUS IFXM_ifxuisimpleedit_GetElementCaretPosition_simpleEdit(
        IFX_HMODULE         hModule,
        IFX_HELEMENT        hElement,
        IFX_INT32          *pIndex)
{
    SIMPLE_EDIT* pElementSession = (SIMPLE_EDIT*)hElement;

    if (pElementSession)
    {
        *pIndex = pElementSession->caretPosition;

        return IFX_SUCCESS;
    }

    return IFX_ERROR;
}

//////////////////////////////////////
//Internal SimpleEdit Functions
//////////////////////////////////////

/*-------------------------------------------------------------------------*//**
    Depending upon the style of the Simple_edit this
    function gives a default input mode for the text box
*/
static IFX_RETURN_STATUS    SimpleEdit_GetDefaultInputMode( SIMPLE_EDIT*    pElement,
                                                            IFX_INT32       *recommendedInputMode)
{
    if (pElement->style & SIMPLE_EDIT_STYLE_TEXT)
    {
        *recommendedInputMode = SIMPLE_EDIT_INPUT_AUTOCAPSENTENCE;
    }
    else if (pElement->style & SIMPLE_EDIT_STYLE_PHONE)
    {
        *recommendedInputMode = SIMPLE_EDIT_INPUT_NUMERIC;
    }
    else if (pElement->style & SIMPLE_EDIT_STYLE_NUMERIC)
    {
        *recommendedInputMode = SIMPLE_EDIT_INPUT_NUMERIC;
    }
    else if (pElement->style & SIMPLE_EDIT_STYLE_PASSWORD)
    {
        *recommendedInputMode = SIMPLE_EDIT_INPUT_NUMERIC;
    }

    return IFX_SUCCESS;
}

/*-------------------------------------------------------------------------*//**
    Validates the input mode provided by the user against the style of the
    simple edit. this function also gives the recommended input in case of an error
    if there is no error then the recommendation is equal to the given mode
*/
static IFX_RETURN_STATUS    SimpleEdit_ValidateInputMode(   SIMPLE_EDIT*    pElement,
                                                            IFX_INT32       givenInputMode,
                                                            IFX_INT32       *recommendedInputMode)
{
    IFX_INT32 style = pElement->style;
    IFX_RETURN_STATUS status = IFX_ERROR;

    *recommendedInputMode = givenInputMode;

    if (givenInputMode & SIMPLE_EDIT_INPUT_AUTOCAPWORD)
    {
        /* Auto cap word case is for text only */
        if (style & SIMPLE_EDIT_STYLE_TEXT)
        {
            status = IFX_SUCCESS;
        }
    }
    else if (givenInputMode & SIMPLE_EDIT_INPUT_AUTOCAPSENTENCE)
    {
        /* Auto cap sentence case is for text only */
        if (style & SIMPLE_EDIT_STYLE_TEXT)
        {
            status = IFX_SUCCESS;
        }
    }
    else if (givenInputMode & SIMPLE_EDIT_INPUT_LOWERCASE)
    {
        /* Lower case is for text only */
        if (style & SIMPLE_EDIT_STYLE_TEXT)
        {
            status = IFX_SUCCESS;
        }
    }
    else if (givenInputMode & SIMPLE_EDIT_INPUT_UPPERCASE)
    {
        /* Upper case is for text only */
        if (style & SIMPLE_EDIT_STYLE_TEXT)
        {
            status = IFX_SUCCESS;
        }
    }
    else if (givenInputMode & SIMPLE_EDIT_INPUT_NUMERIC)
    {
        /* Numeric can be used in any style */
        status = IFX_SUCCESS;
    }

    /* If current input mode is not valid then tell the caller
       that current input mode is wrong and recommend default input mode */
    if (status == IFX_ERROR)
    {
        SimpleEdit_GetDefaultInputMode(pElement,recommendedInputMode);
    }

    return status;
}

/*-------------------------------------------------------------------------*//**
    Initialize the Simple_edit depending upon the value against style key in URL
*/
static IFX_RETURN_STATUS SimpleEdit_processStyle(   SIMPLE_EDIT*        pElement,
                                                    const IFX_WCHAR*    szValue)
{
#if !(defined(WIN32) || defined(_WIN32)) || defined(LC_USE_NDE_RTL_WIDE_CHAR)
    IFX_WCHAR* szState          = NULL;
#endif
    IFX_WCHAR delimiter[]       = L"|";
    IFX_RETURN_STATUS status    = IFX_SUCCESS;
    IFX_WCHAR* szKey            = NULL;
    IFX_WCHAR* szValueCopy      = NULL;

    if (szValue == NULL)
        return IFX_SUCCESS;

    // Make a writable copy of szValue
    SimpleEdit_AllocateMemoryUnsafe((lc_wcslen(szValue) + 1) * sizeof(IFX_WCHAR), &szValueCopy);
    if (szValueCopy == NULL)
    {
        return IFX_ERROR;
    }
    lc_wcscpy(szValueCopy, szValue);

    szKey = (IFX_WCHAR*)lc_wcstok(szValueCopy, delimiter, &szState);
    while (szKey != NULL)
    {
        if (lc_wcscmpi(szKey, S_PW_STRING) == 0)
        {
            pElement->style |= SIMPLE_EDIT_STYLE_PASSWORD;
        }
        else if (lc_wcscmpi(szKey, S_TEXT_STRING) == 0)
        {
            pElement->style |= SIMPLE_EDIT_STYLE_TEXT;
        }
        else if (lc_wcscmpi(szKey, S_PHONE_STRING) == 0)
        {
            pElement->style |= SIMPLE_EDIT_STYLE_PHONE;
        }
        else if (lc_wcscmpi(szKey, S_NUMERIC_STRING) == 0)
        {
            pElement->style |= SIMPLE_EDIT_STYLE_NUMERIC;
        }
        else
        {
            status = IFX_ERROR;
        }

        szKey = (IFX_WCHAR*)lc_wcstok(NULL, delimiter, &szState);
    }

    SimpleEdit_FreeMemoryUnsafe(szValueCopy);

    return status;
}

/*-------------------------------------------------------------------------*//**
    Initialize the Simple_edit depending upon the value against input mode key in URL
*/
static IFX_RETURN_STATUS SimpleEdit_processInputMode(   SIMPLE_EDIT*        pElement,
                                                        const IFX_WCHAR*    szValue)
{
    IFX_INT32 givenMode = 0;
    IFX_INT32 recommendedMode;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (szValue == NULL)
        return status;

    if (lc_wcscmpi(szValue, IM_AUTOCAPWORD_STRING) == 0)
    {
        givenMode = SIMPLE_EDIT_INPUT_AUTOCAPWORD;
    }
    else if (lc_wcscmpi( szValue, IM_AUTOCAPSENTENCE_STRING) == 0)
    {
        givenMode = SIMPLE_EDIT_INPUT_AUTOCAPSENTENCE;
    }
    else if (lc_wcscmpi( szValue, IM_LOWERCASE_STRING) == 0)
    {
        givenMode = SIMPLE_EDIT_INPUT_LOWERCASE;
    }
    else if (lc_wcscmpi( szValue, IM_UPPERCASE_STRING) == 0)
    {
        givenMode = SIMPLE_EDIT_INPUT_UPPERCASE;
    }
    else if (lc_wcscmpi( szValue, IM_NUMERIC_STRING) == 0)
    {
        givenMode = SIMPLE_EDIT_INPUT_NUMERIC;
    }
    else
    {
        SimpleEdit_GetDefaultInputMode(pElement, &recommendedMode);

        status = IFX_ERROR;
    }

    /* This call will verify if the user given inputmode is valid or not.
       If the input mode is invalid the recommended has the default mode */
    SimpleEdit_ValidateInputMode(pElement, givenMode, &recommendedMode);

    pElement->inputMode = recommendedMode;

    return status;
}

/*-------------------------------------------------------------------------*//**
    This function verifies if the pElement is completely initialized.
    It also fills in the missing information
*/
static IFX_RETURN_STATUS    SimpleEdit_FillMissingInfo(SIMPLE_EDIT*     pElement)
{
    IFX_INT32 style = pElement->style;

    /* If text style is neither of text, numeric, and phone
       then select the text as default */
    if (!(style & SIMPLE_EDIT_STYLE_PHONE
        || style & SIMPLE_EDIT_STYLE_TEXT
        || style & SIMPLE_EDIT_STYLE_NUMERIC))
    {
        style |= SIMPLE_EDIT_STYLE_TEXT;
    }

    /*we cannot have phone style and password style simultaneously*/
    if (style & SIMPLE_EDIT_STYLE_PHONE
        && style & SIMPLE_EDIT_STYLE_PASSWORD )
    {
        /* Using xor to unset the phone style */
        style ^= SIMPLE_EDIT_STYLE_PHONE;

        /* Setting the mode to text */
        style |= SIMPLE_EDIT_STYLE_TEXT;
    }

    pElement->style = style;

    /* Validate to confirm that input mode does not conflict with updated style */
    SimpleEdit_ValidateInputMode(pElement, pElement->inputMode,
                                &(pElement->inputMode));

    return IFX_SUCCESS;
}

/*-------------------------------------------------------------------------*//**
    Based on the style and inputmode this function selects next legal inputmode
*/
IFX_RETURN_STATUS   SimpleEdit_CycleInputMode(SIMPLE_EDIT_SESSION*  pSession,
                                              SIMPLE_EDIT*          pElement)
{
    IFX_INT32 givenInputMode            = pElement->inputMode;
    IFX_INT32 recommendedInputMode  = 0;
    IFX_INT32 i = 0;
    IFX_INT32 j = 0;

    for (; i < MAX_INPUT_MODES; i++)
    {
        if (givenInputMode & INPUT_MODE_ARRAY[i])
        {
            break;
        }
    }

    /* To see if the previous input mode was valid or not */
    if (i < MAX_INPUT_MODES)
    {
        for (j = 0; j < MAX_INPUT_MODES; j++)
        {
            i = (i + 1) % MAX_INPUT_MODES;

            if (IFX_SUCCESS == SimpleEdit_ValidateInputMode(pElement, INPUT_MODE_ARRAY[i],
                                                            &recommendedInputMode))
            {
                break;
            }
        }

        /* There was no valid inputmode. get the default */
        if (j >= MAX_INPUT_MODES)
        {
            SimpleEdit_GetDefaultInputMode(pElement, &recommendedInputMode);
        }
    }
    else
    {
        /* Already used input mode was invalid. use default now */
        SimpleEdit_GetDefaultInputMode(pElement, &recommendedInputMode);
    }

    pElement->inputMode = recommendedInputMode;

    if (givenInputMode != recommendedInputMode)
    {
        /* Let the Engine indicator know the input mode has changed */
        IFXI_RequestFieldRefresh(pSession->hModID, NULL, -1, FIELD_INPUTMODE);
    }

    return IFX_SUCCESS;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_INT32 SimpleEdit_CheckSpecialChar(IFX_INT32 key)
{
    switch(key)
    {
        case '$':
        case '{':
        case '}':
        case '[':
        case ']':
        {
            return IFX_TRUE;
        }
    }

    return IFX_FALSE;
}
