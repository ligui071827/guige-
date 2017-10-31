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

#include "inflexionui/engine/inc/ifxui_integration.h"

#include <string.h>
#include "inflexionui/engine/ifxui_uriparser.h"

#define IFXU_URI_INT_BYTE_SIZE      15      /* The max string size of IFX_INT32
                                               representation */
#define IFXU_URI_FLOAT_BYTE_SIZE    30      /* The max string size of float
                                               representation */
#define IFXU_URI_BOOL_BYTE_SIZE     10      /* The max string size of IFX_BOOL
                                               representation */

#define URI_ESCAPE_CHARACTER    '&'
#define URI_LINKPREFIX          L"://"

typedef struct Param_t
{
    IFX_WCHAR*      szKey;                  /* Parameter key */
    IFX_WCHAR*      szVal;                  /* Parameter value */
    struct Param_t* next;                   /* Pointer to the next Param */
}   URI_Param;

typedef struct URI_t
{
    IFX_WCHAR*      szLinkPrefix;           /* Link Prefix */
    IFX_WCHAR*      szBody;                 /* Body of URI */
    URI_Param*      param;                  /* Parameters of the URI */
} URI_Parser;

typedef enum tokenType                      /* URI parts delimiters */
{
    TOKEN_LINKPREFIX    = ':',
    TOKEN_BODY          = '?',
    TOKEN_PARAM         = '=',
    TOKEN_PARAM_VALUE   = ';'
} TOKEN_TYPE;

/*********************************
* Internal function              *
*********************************/

static IFX_RETURN_STATUS createURI(URI_Parser **hParser);
static IFX_RETURN_STATUS lookupParam(URI_Parser *hParser, const IFX_WCHAR *key,
                                               URI_Param **param);
static IFX_RETURN_STATUS createParam(URI_Parser *parser, URI_Param **param);
static IFX_RETURN_STATUS findOrCreateParam(URI_Parser *hParser, const IFX_WCHAR *key,
                                           URI_Param **param);
static IFX_INT32 isValidURI(const IFX_WCHAR *uriStr);
static IFX_RETURN_STATUS destroyParam(URI_Parser *parser, URI_Param *param);
static IFX_INT32 isSpecialChar(IFX_INT32 ch);
static IFX_INT32 escapeCharLen(const IFX_WCHAR *wcs, IFX_INT32 length);
static IFX_WCHAR* removeEscapeChar(IFX_WCHAR *dest, const IFX_WCHAR *src, IFX_INT32 length);
static IFX_WCHAR* appendEscapeChar(IFX_WCHAR *dest, const IFX_WCHAR *src);
static unsigned char checkLinkPrefix(const IFX_WCHAR    *szLinkPrefix);
static IFX_INT32 calcUriStrlen(const IFX_WCHAR *wcs);
static void asciiStringToWideCharString(IFX_WCHAR* wcStr, const char * asciiStr, int maxSize);
static IFX_BOOL strToBool(IFX_WCHAR* wcStr);

/*********************************
* External function              *
*********************************/

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_Initialize
*
* DESCRIPTION
*
*    This function creates a new IFX_HUTILITY and parses szLink URI.
*
* INPUTS
*
*    IFX_HUTILITY* hUtility - This is out pointer to the session object.
*
*    const IFX_WCHAR* szLink - This is URI string to parse out. This can be
*    null when constructing a new URI. URI can be complete or without link
*    prefix.
*
*    URI Format: <linkPrefix>://<linkBody>?<parameterName>=<parameterValue>;...
*    Example Complete URI - SimpleEdit://telephone?maxlength=10;style=numeric
*    Example sub URI - telephone?maxlength=10;style=numeric
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_Initialize(
                        IFX_HUTILITY       *phUtility,
                        const IFX_WCHAR    *szLink)
{
    IFX_RETURN_STATUS   status;
    URI_Parser* parser;

    status = createURI(&parser);

    if (status == IFX_SUCCESS)
    {
        if (szLink != NULL)
        {
            /* Check for a valid URI */
            if (!isValidURI(szLink))
            {
                status = IFXU_URI_ERROR_INVALID_URI;
            }
        }
    }

    if (status == IFX_SUCCESS)
    {
        if (szLink != NULL)
        {
            /* NULL check on input string has been done,
            and it has been checked for validity so we are safe to parse it */

            IFX_WCHAR** tokenDest           = NULL;
            const IFX_WCHAR* startPos       = szLink;
            const IFX_WCHAR* strPos         = szLink;
            URI_Param* currentParam         = 0;
            IFX_INT32 tokenSize                 = 0;
            IFX_INT32 size;
            TOKEN_TYPE expectedToken;

            expectedToken = TOKEN_LINKPREFIX;

            while(1)
            {
                /* if delimiter is found or EOL occurs */
                if ((*strPos == expectedToken) || (*strPos == '\0'))
                {
                    tokenSize = (IFX_INT32)(strPos - startPos);

                    if (expectedToken == TOKEN_PARAM)
                    {
                        currentParam = NULL;
                        status = createParam(parser, &currentParam);

                        /* Check for memory error */
                        if (status != IFX_SUCCESS)
                            break;

                        if (tokenSize > 0 && currentParam)
                        {
                            tokenDest = &(currentParam->szKey);
                        }

                        else
                        {
                            /*  invalid URI */
                            status = IFXU_URI_ERROR_INVALID_URI;
                            destroyParam(parser, currentParam);
                            break;
                        }

                        expectedToken = TOKEN_PARAM_VALUE;
                    }

                    else if (expectedToken == TOKEN_PARAM_VALUE)
                    {
                        if (tokenSize && currentParam)
                        {
                            tokenDest = &(currentParam->szVal);
                        }

                        expectedToken = TOKEN_PARAM;
                    }

                    else if (expectedToken == TOKEN_LINKPREFIX)
                    {
                        if (lc_wcsncmp(strPos, L"://", 3) == 0)
                        {
                            if (tokenSize)
                            {
                                tokenDest = &(parser->szLinkPrefix);
                            }

                            else
                            {
                                parser->szBody  = NULL;
                            }

                            strPos += 2;

                            expectedToken = TOKEN_BODY;
                        }
                        else
                        {
                            /* False alarm */
                            tokenSize = 0;
                        }
                    }

                    else if (expectedToken == TOKEN_BODY)
                    {
                        if (tokenSize)
                        {
                            tokenDest = &parser->szBody;
                        }

                        else
                        {
                            parser->szBody  = NULL;
                        }

                        /* Special case detection - URI has a body delimiter
                           but no parameters.  Advance the current position
                           to the Null terminator. */
                        if (lc_wcslen(strPos) == 1)
                            strPos++;

                        expectedToken = TOKEN_PARAM;
                    }

                    /* if token found then Allocate memory for the token
                       and copy it */
                    if (tokenSize > 0 && tokenDest)
                    {
                        size = escapeCharLen(startPos, tokenSize);
                        tokenSize-= size;

                        *tokenDest = IFXE_AllocateMemoryUnsafe(sizeof(IFX_WCHAR) * (tokenSize +1));
                        if (*tokenDest != NULL)
                        {
                            removeEscapeChar((*tokenDest), startPos, tokenSize);
                        }

                        else
                        {
                            status = IFX_ERROR_MEMORY;
                            break;
                        }

                        /* start of the next token */
                        startPos = strPos + 1;
                    }

                    /* If string is finished, then end the loop */
                    if (*strPos == '\0')
                        break;

                    /* start of the next token */
                    startPos = strPos + 1;
                }

                strPos++;

                /* Process for escape characters */
                while(*strPos == URI_ESCAPE_CHARACTER)
                {
                    strPos++;

                    if (*strPos != '\0')
                    {
                        strPos++;
                    }
                }
            }
        }

        (*phUtility) = parser;
    }

    /* Clean up resources if operation failed. */
    if (status != IFX_SUCCESS)
        IFXU_UriParser_Destroy(parser);

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    URI_Destroy
*
* DESCRIPTION
*
*    This function destroys and releases resources occupied by hUtility.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_Destroy(
                        IFX_HUTILITY        hUtility)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;
    URI_Parser* parser  = (URI_Parser*)hUtility;
    URI_Param*  param   = NULL;
    URI_Param*  temp    = NULL;

    if (parser)
    {
        /* Release memory for body and linkPrefix */
        if (parser->szBody)
        {
            IFXE_FreeMemoryUnsafe(parser->szBody);
        }

        if (parser->szLinkPrefix)
        {
            IFXE_FreeMemoryUnsafe(parser->szLinkPrefix);
        }

        /* Release memory of all the parameters */
        param = parser->param;
        while(param)
        {
            if (param->szKey)
            {
                IFXE_FreeMemoryUnsafe(param->szKey);
            }

            if (param->szVal)
            {
                IFXE_FreeMemoryUnsafe(param->szVal);
            }

            temp  = param;
            param = param->next;

            IFXE_FreeMemoryUnsafe(temp);
        }

        /* Release memory of parser structure */
        IFXE_FreeMemoryUnsafe(parser);
    }

    else
    {
        status = IFXU_URI_ERROR_INVALID_HANDLE;
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetParameterCount
*
* DESCRIPTION
*
*    This function calculates the paramter count of the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_INT32* count - Count of the parameters is returned through this variable.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetParameterCount(
                        IFX_HUTILITY hUtility,
                        IFX_INT32 *count)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;
    URI_Parser* parser  = (URI_Parser*)hUtility;
    URI_Param*  param   = NULL;
    *count = 0;

    if (parser)
    {
        /* Traverse the parameters list */
        param = parser->param;
        while(param)
        {
            param = param->next;
            *count += 1;
        }

        if (*count == 0)
        {
            status = IFXU_URI_ERROR_GENERAL;
        }
    }

    else
    {
        status = IFXU_URI_ERROR_INVALID_HANDLE;
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetParameterKeySize
*
* DESCRIPTION
*
*    This function calculates the size of the string value for the given
*    parameter index.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_INT32 paramIndex - Index of the parameter.
*
*    IFX_INT32* size - Size of the parameter's value is returned through this
*    variable.
*
*    const IFX_WCHAR* defaultValue - Default value of the parameter.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetParameterKeySize(
                        IFX_HUTILITY hUtility,
                        IFX_INT32 paramIndex,
                        IFX_INT32 *size,
                        const IFX_WCHAR *defaultValue)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;
    URI_Parser* parser  = (URI_Parser*)hUtility;
    URI_Param*  param   = NULL;
    IFX_BOOL found = IFX_FALSE;
    IFX_INT32 count = 0;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (size == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (parser)
    {
        if (status == IFX_SUCCESS)
        {
            *size = 1;
            param = parser->param;

            while(param)
            {
                if (param->szKey && count == paramIndex)
                {
                    *size += (IFX_INT32)calcUriStrlen(param->szKey) + 1;  /* key + delimiter '=' */
                    found = IFX_TRUE;
                    break;
                }
                else
                {
                    count++;
                }
                param = param->next;
            }

            if (found == IFX_FALSE && defaultValue)
            {
                *size = (IFX_INT32)lc_wcslen(defaultValue) + 1;
                status = IFX_SUCCESS;
            }
            else if (found == IFX_FALSE)
            {
                status = IFXU_URI_ERROR_INVALID_PARAM;
            }
        }
    }
    else
    {
        status = IFXU_URI_ERROR_INVALID_HANDLE;
    }
    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetParameterKeyData
*
* DESCRIPTION
*
*    This function gives back value of the given parameter (key).
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_INT32 paramIndex - Index of the parameter.
*
*    IFX_WCHAR* szValue - Pointer to the buffer to copy parameter value. This
*    should be larger enough to occupy parameter value and null terminator.
*
*    const IFX_WCHAR* defaultValue - If parameter is not found in parameter
*    list then defaultValue is returned.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetParameterKeyData(
                        IFX_HUTILITY hUtility,
                        IFX_INT32 paramIndex,
                        IFX_WCHAR *szValue,
                        const IFX_WCHAR *defaultValue)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    URI_Parser* parser  = (URI_Parser*)hUtility;
    URI_Param*  param   = NULL;
    IFX_INT32 count = 0;

    if (parser)
    {
        param = parser->param;
        while(param)
        {
            if (param->szKey)
            {
                if (count == paramIndex)
                {
                    status = IFX_SUCCESS;
                    break;
                }
                else
                {
                    count ++;
                }
            }
            param = param->next;
        }

        /* if parameter found return its value */
        if (status == IFX_SUCCESS && param)
        {
            lc_wcscpy(szValue, param->szKey);
        }

        else if (defaultValue)
        {
            lc_wcscpy(szValue, defaultValue);
            status = IFX_SUCCESS;
        }

        else
        {
            *szValue = 0;
            status = IFX_SUCCESS;
        }
    }

    else
    {
        status = IFXU_URI_ERROR_INVALID_HANDLE;
    }

    return status;

}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetParameterValueSize
*
* DESCRIPTION
*
*    This function calculates the size of the string value for the given
*    parameter index.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_INT32 paramIndex - Index of the parameter.
*
*    IFX_INT32* size - Size of the parameter's value is returned through this
*    variable.
*
*    const IFX_WCHAR* defaultValue - Default value of the parameter.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetParameterValueSize(
                        IFX_HUTILITY hUtility,
                        IFX_INT32 paramIndex,
                        IFX_INT32 *size,
                        const IFX_WCHAR *defaultValue)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;
    URI_Parser* parser  = (URI_Parser*)hUtility;
    URI_Param*  param   = NULL;
    IFX_BOOL found = IFX_FALSE;
    IFX_INT32 count = 0;

    if (parser)
    {
        if (size == NULL)
            status = IFXU_URI_ERROR_INVALID_PARAM;

        if (status == IFX_SUCCESS)
        {
            *size = 1;
            param = parser->param;
            while(param)
            {
                if (param->szVal && count == paramIndex)
                {
                    *size += (IFX_INT32)calcUriStrlen(param->szVal);
                    found = IFX_TRUE;
                    break;
                }
                else
                {
                    count ++;
                }
                param = param->next;
            }

            if (found == IFX_FALSE && defaultValue)
            {
                *size = (IFX_INT32)lc_wcslen(defaultValue) + 1;
                status = IFX_SUCCESS;
            }
            else if (found == IFX_FALSE)
            {
                 status = IFXU_URI_ERROR_INVALID_PARAM;
            }
        }
    }

    else
    {
        status = IFXU_URI_ERROR_INVALID_HANDLE;
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetParameterValueData
*
* DESCRIPTION
*
*    This function gives back value of the given parameter (key).
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_INT32 paramIndex - Index of the parameter.
*
*    IFX_WCHAR* szValue - Pointer to the buffer to copy parameter value. This
*    should be larger enough to occupy parameter value and null terminator.
*
*    const IFX_WCHAR* defaultValue - If parameter is not found in parameter
*    list then defaultValue is returned.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetParameterValueData(
                        IFX_HUTILITY hUtility,
                        IFX_INT32 paramIndex,
                        IFX_WCHAR *szValue,
                        const IFX_WCHAR *defaultValue)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    URI_Parser* parser  = (URI_Parser*)hUtility;
    URI_Param*  param   = NULL;
    IFX_INT32 count = 0;

    if (parser)
    {
        param = parser->param;
        while(param)
        {
            if (count == paramIndex)
            {
                status = IFX_SUCCESS;
                break;
            }
            else
            {
                count ++;
            }
            param = param->next;
        }

        /* if parameter found return its value */
        if (status == IFX_SUCCESS && param)
        {
            lc_wcscpy(szValue, param->szVal);
        }

        else if (defaultValue)
        {
            lc_wcscpy(szValue, defaultValue);
            status = IFX_SUCCESS;
        }

        else
        {
            *szValue = 0;
            status = IFX_SUCCESS;
        }
    }

    else
    {
        status = IFXU_URI_ERROR_INVALID_HANDLE;
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetLinkBodySize
*
* DESCRIPTION
*
*    This function calculates the size of <linkBody> token of the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_INT32* size - Size of the <linkBody> is returned through this variable.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetLinkBodySize(
                        IFX_HUTILITY        hUtility,
                        IFX_INT32          *size,
                        const IFX_WCHAR    *defaultValue)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (size == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        *size = 0;

        if (((URI_Parser*)hUtility)->szBody)
        {
            *size = (IFX_INT32)lc_wcslen(((URI_Parser*)hUtility)->szBody) + 1;
        }

        else if (defaultValue)
        {
            *size = (IFX_INT32)lc_wcslen(defaultValue) + 1;
        }

        else
        {
            *size = 1;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetLinkBodyStringData
*
* DESCRIPTION
*
*    This function give back <linkBody> token of the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_WCHAR* szLinkBody - Pointer to the buffer to copy <linkBody>. This
*    should be larger enough to contain <linkBody> and null terminator.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetLinkBodyStringData(
                        IFX_HUTILITY        hUtility,
                        IFX_WCHAR          *szLinkBody,
                        const IFX_WCHAR    *defaultValue)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (szLinkBody == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        *szLinkBody = 0;

        if (((URI_Parser*)hUtility)->szBody)
        {
            lc_wcscpy(szLinkBody, ((URI_Parser*)hUtility)->szBody);
        }

        else if (defaultValue)
        {
            lc_wcscpy(szLinkBody, defaultValue);
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetLinkPrefixSize
*
* DESCRIPTION
*
*    This function calculates the size of <LinkPrefix> token of the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_INT32* size - Size of the <LinkPrefix> is returned through this variable.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetLinkPrefixSize(
                        IFX_HUTILITY        hUtility,
                        IFX_INT32          *size)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (size == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        *size = 0;

        if (((URI_Parser*)hUtility)->szLinkPrefix)
        {
            *size = (IFX_INT32)lc_wcslen(((URI_Parser*)hUtility)->szLinkPrefix) + 1;
        }

        else
        {
            status = IFXU_URI_ERROR_NOT_FOUND;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetLinkPrefixData
*
* DESCRIPTION
*
*    This function give back <LinkPrefix> token of the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_WCHAR* szLinkPrefix - Pointer to the buffer to copy <LinkPrefix>. This
*    should be larger enough to contain <LinkPrefix> and null terminator.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetLinkPrefixData(
                        IFX_HUTILITY        hUtility,
                        IFX_WCHAR          *szLinkPrefix)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (szLinkPrefix == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        *szLinkPrefix = '\0';

        if (((URI_Parser*)hUtility)->szLinkPrefix)
        {
            lc_wcscpy(szLinkPrefix, ((URI_Parser*)hUtility)->szLinkPrefix);
        }

        else
        {
            status = IFXU_URI_ERROR_NOT_FOUND;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetLinkBodyInt
*
* DESCRIPTION
*
*    This function give back <linkBody> token of the URI as IFX_INT32 representation.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_INT32* pLinkBody - Pointer to the IFX_INT32 to copy <linkBody>.
*
*    IFX_INT32 defaultValue - defaultValue is return in case of missing value or
*    some error.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetLinkBodyInt(
                        IFX_HUTILITY        hUtility,
                        IFX_INT32          *pLinkBody,
                        IFX_INT32           defaultValue)
{
    IFX_INT32 size = 0;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (pLinkBody == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        *pLinkBody = defaultValue;

        status = IFXU_UriParser_GetLinkBodySize(hUtility, &size, NULL);
        if (status == IFX_SUCCESS && size <= IFXU_URI_INT_BYTE_SIZE && size > 1)
        {
            IFX_WCHAR data[IFXU_URI_INT_BYTE_SIZE];
            status = IFXU_UriParser_GetLinkBodyStringData(hUtility, &data[0],
                                                                         NULL);

            if (status == IFX_SUCCESS)
            {
                *pLinkBody = lc_wcstol(&data[0], NULL, 10);
            }
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetLinkBodyFloat
*
* DESCRIPTION
*
*    This function gives back float value of the given parameter.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    float* pLinkBody - Pointer to the float to copy body value into.
*
*    float defaultValue - If body is not found, then defaultValue is returned.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS IFXU_UriParser_GetLinkBodyFloat(
                        IFX_HUTILITY        hUtility,
                        float              *pLinkBody,
                        float               defaultValue)
{
    IFX_INT32 size = 0;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (pLinkBody == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        *pLinkBody = defaultValue;

        status = IFXU_UriParser_GetLinkBodySize(hUtility, &size, NULL);
        if (status == IFX_SUCCESS && size <= IFXU_URI_FLOAT_BYTE_SIZE && size > 1)
        {
            IFX_WCHAR data[IFXU_URI_FLOAT_BYTE_SIZE];
            status = IFXU_UriParser_GetLinkBodyStringData(hUtility, &data[0],
                                                                         NULL);

            if (status == IFX_SUCCESS)
            {
                *pLinkBody = (float)lc_wcstod(&data[0], NULL);
            }
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetLinkBodyTime
*
* DESCRIPTION
*
*    This function give back <linkBody> token of the URI as time representation.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_TIME* pLinkBody - Pointer to the IFX_TIME to copy <linkBody>.
*
*    IFX_TIME defaultValue - defaultValue is return in case of missing value or
*    some error.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS IFXU_UriParser_GetLinkBodyTime(
                        IFX_HUTILITY        hUtility,
                        IFX_TIME           *pLinkBody,
                        IFX_TIME            defaultValue)
{
    IFX_INT32 size = 0;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (pLinkBody == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        *pLinkBody = defaultValue;

        status = IFXU_UriParser_GetLinkBodySize(hUtility, &size, NULL);
        if (status == IFX_SUCCESS && size <= IFXU_URI_INT_BYTE_SIZE && size > 1)
        {
            IFX_WCHAR data[IFXU_URI_INT_BYTE_SIZE];
            status = IFXU_UriParser_GetLinkBodyStringData(hUtility, &data[0],
                                                                        NULL);

            if (status == IFX_SUCCESS)
            {
                *pLinkBody = lc_wcstol(&data[0], NULL, 10);
            }
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetLinkBodyBool
*
* DESCRIPTION
*
*    This function give back <linkBody> token of the URI as IFX_BOOL representation.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_BOOL* pLinkBody - Pointer to the IFX_BOOL to copy <linkBody>.
*
*    IFX_BOOL defaultValue - defaultValue is return in case of missing value or
*    some error.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetLinkBodyBool(
                        IFX_HUTILITY        hUtility,
                        IFX_BOOL           *pLinkBody,
                        IFX_BOOL            defaultValue)
{
    IFX_INT32 size = 0;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (pLinkBody == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        *pLinkBody = defaultValue;

        status = IFXU_UriParser_GetLinkBodySize(hUtility, &size, NULL);
        if (status == IFX_SUCCESS && size <= IFXU_URI_BOOL_BYTE_SIZE && size > 1)
        {
            IFX_WCHAR data[IFXU_URI_BOOL_BYTE_SIZE];
            status = IFXU_UriParser_GetLinkBodyStringData(hUtility, &data[0],
                                                                         NULL);

            if (status == IFX_SUCCESS)
            {
                *pLinkBody = strToBool(&data[0]);
            }
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetParamStringSize
*
* DESCRIPTION
*
*    This function calculates the size of the string value for the given
*    parameter.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szParam - Name of the parameter.
*
*    IFX_INT32* size - Size of the parameter's value is returned through this
*    variable.
*
*    const IFX_WCHAR* defaultValue - Default value of the parameter.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetParamStringSize(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_INT32          *size,
                        const IFX_WCHAR    *defaultValue)
{
    URI_Param* param  = NULL;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if ((szParam == NULL) || (size == NULL))
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        *size = 0;
        status = lookupParam((URI_Parser*)hUtility, szParam, &param);

        /* if parameter found calculate its size */
        if ((status == IFX_SUCCESS) && param && param->szVal)
        {
            *size = (IFX_INT32)lc_wcslen(param->szVal) + 1;
        }

        else if (defaultValue)
        {
            *size = (IFX_INT32)lc_wcslen(defaultValue) + 1;
            status = IFX_SUCCESS;
        }

        else
        {
            *size = 1;
            status = IFX_SUCCESS;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetParamStringData
*
* DESCRIPTION
*
*    This function gives back value of the given parameter.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szParam - Name of the parameter.
*
*    IFX_WCHAR* szValue - Pointer to the buffer to copy parameter value. This
*    should be larger enough to occupy parameter value and null terminator.
*
*    const IFX_WCHAR* defaultValue - If parameter is not found in parameter
*    list then defaultValue is returned.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetParamStringData(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_WCHAR          *szValue,
                        const IFX_WCHAR    *defaultValue)
{
    URI_Param* param  = NULL;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if ((szParam == NULL) || (szValue == NULL))
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        *szValue = 0;

        status = lookupParam((URI_Parser*)hUtility, szParam, &param);

        /* if parameter found return its value */
        if (status == IFX_SUCCESS && param && param->szVal)
        {
            lc_wcscpy(szValue, param->szVal);
        }

        else if (defaultValue)
        {
            lc_wcscpy(szValue, defaultValue);
            status = IFX_SUCCESS;
        }

        else
        {
            *szValue = 0;
            status = IFX_SUCCESS;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetParamInt
*
* DESCRIPTION
*
*    This function gives back integer value of the given parameter.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szParam - Name of the parameter.
*
*    IFX_INT32* value - Pointer to the long to copy parameter value.
*
*    IFX_INT32 defaultValue - If parameter is not found in parameter
*    list then defaultValue is returned.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS IFXU_UriParser_GetParamInt(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_INT32          *value,
                        IFX_INT32           defaultValue)
{
    URI_Param* param  = NULL;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if ((szParam == NULL) || (value == NULL))
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        status = lookupParam((URI_Parser*)hUtility, szParam, &param);

        /* if parameter found convert its value to IFX_INT32*/
        if (status == IFX_SUCCESS && param && param->szVal)
        {
            *value = (IFX_INT32)lc_wcstol(param->szVal, NULL, 10);
        }

        else
        {
            *value = defaultValue;
            status = IFX_SUCCESS;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetParamFloat
*
* DESCRIPTION
*
*    This function gives back float value of the given parameter.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szParam - Name of the parameter.
*
*    float* value - Pointer to the float to copy parameter value.
*
*    float defaultValue - If parameter is not found in parameter
*    list then defaultValue is returned.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS IFXU_UriParser_GetParamFloat(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        float              *value,
                        float               defaultValue)
{
    URI_Param* param  = NULL;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if ((szParam == NULL) || (value == NULL))
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        status = lookupParam((URI_Parser*)hUtility, szParam, &param);

        /* if parameter found convert its value to float*/
        if (status == IFX_SUCCESS && param && param->szVal)
        {
            *value = (float) lc_wcstod(param->szVal, NULL);
        }

        else
        {
            *value = defaultValue;
            status = IFX_SUCCESS;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetParamTime
*
* DESCRIPTION
*
*    This function gives back time value of the given parameter.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szParam - Name of the parameter.
*
*    IFX_TIME* value - Pointer to the IFX_TIME to copy parameter value.
*
*    IFX_TIME defaultValue - If parameter is not found in parameter
*    list then defaultValue is returned.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS IFXU_UriParser_GetParamTime(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_TIME           *value,
                        IFX_TIME            defaultValue)
{
    URI_Param *param  = NULL;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if ((szParam == NULL) || (value == NULL))
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        status = lookupParam((URI_Parser*)hUtility, szParam, &param);

        /* if parameter found convert its value to time*/
        if (status == IFX_SUCCESS && param && param->szVal)
        {
            *value = lc_wcstol(param->szVal, NULL, 10);
        }

        else
        {
            *value = defaultValue;
            status = IFX_SUCCESS;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_GetParamBool
*
* DESCRIPTION
*
*    This function gives back boolean value of the given parameter.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szParam - Name of the parameter.
*
*    IFX_BOOL* value - Pointer to the bool to copy parameter value.
*
*    IFX_BOOL defaultValue - If parameter is not found in parameter
*    list then defaultValue is returned.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS IFXU_UriParser_GetParamBool(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_BOOL           *value,
                        IFX_BOOL            defaultValue)
{
    URI_Param* param  = NULL;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if ((szParam == NULL) || (value == NULL))
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        status = lookupParam((URI_Parser*)hUtility, szParam, &param);

        /* if parameter found convert its value to IFX_BOOL*/
        if (status == IFX_SUCCESS && param && param->szVal)
        {
            *value = strToBool(param->szVal);
        }

        else
        {
            *value = defaultValue;
            status = IFX_SUCCESS;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_SetLinkPrefix
*
* DESCRIPTION
*
*    This function sets the <linkPrefix> of the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szLinkPrefix - <linkPrefix> value.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_SetLinkPrefix(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szLinkPrefix)
{
    URI_Parser *parser  = (URI_Parser *)hUtility;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (parser == NULL)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (szLinkPrefix == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    else if (checkLinkPrefix(szLinkPrefix) == IFX_FALSE)
        status = IFXU_URI_ERROR_INVALID_LINK_PREFIX;

    if (status == IFX_SUCCESS)
    {
        /* deallocate memory if body exists */
        if (parser->szLinkPrefix)
        {
            IFXE_FreeMemoryUnsafe(parser->szLinkPrefix);

            parser->szLinkPrefix = NULL;
        }


        parser->szLinkPrefix = IFXE_AllocateMemoryUnsafe((sizeof(IFX_WCHAR) * (lc_wcslen(szLinkPrefix) +1)));
        if (parser->szLinkPrefix == NULL)
        {
            status = IFX_ERROR_MEMORY;
        }

        else
        {
            lc_wcscpy(parser->szLinkPrefix, szLinkPrefix);
            status = IFX_SUCCESS;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_SetLinkBodyString
*
* DESCRIPTION
*
*    This function sets the <linkBody> of the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szLinkBody - <linkBody> value.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_SetLinkBodyString(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szLinkBody)
{
    URI_Parser *parser  = hUtility;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (parser == NULL)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (szLinkBody == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        /* deallocate memory if body exists */
        if (parser->szBody)
        {
            IFXE_FreeMemoryUnsafe(parser->szBody);

            parser->szBody = NULL;
        }

        parser->szBody = IFXE_AllocateMemoryUnsafe(sizeof(IFX_WCHAR) * (lc_wcslen(szLinkBody) +1));
        if (parser->szBody == NULL)
        {
            status = IFX_ERROR_MEMORY;
        }

        else
        {
            lc_wcscpy(parser->szBody, szLinkBody);
            status = IFX_SUCCESS;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_SetLinkBodyInt
*
* DESCRIPTION
*
*    This function sets the <linkBody> of the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_INT32 linkBody - <linkBody> value.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_SetLinkBodyInt(
                        IFX_HUTILITY        hUtility,
                        IFX_INT32           linkBody)
{
    IFX_WCHAR strVal[IFXU_URI_INT_BYTE_SIZE];
    char cstrVal[IFXU_URI_INT_BYTE_SIZE];
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (status == IFX_SUCCESS)
    {
        lc_itoa(linkBody, cstrVal, 10);

        asciiStringToWideCharString(strVal, cstrVal, IFXU_URI_INT_BYTE_SIZE);

        status = IFXU_UriParser_SetLinkBodyString(hUtility, &strVal[0]);
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_SetLinkBodyFloat
*
* DESCRIPTION
*
*    This function sets the <linkBody> of the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    float linkBody - <linkBody> value.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_SetLinkBodyFloat(
                        IFX_HUTILITY        hUtility,
                        float               linkBody)
{
    IFX_WCHAR strVal[IFXU_URI_FLOAT_BYTE_SIZE];
    char cstrVal[IFXU_URI_FLOAT_BYTE_SIZE];
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (status == IFX_SUCCESS)
    {
        lc_sprintf(cstrVal, "%f", linkBody);

        asciiStringToWideCharString(strVal, cstrVal, IFXU_URI_FLOAT_BYTE_SIZE);

        status = IFXU_UriParser_SetLinkBodyString(hUtility, &strVal[0]);
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_SetLinkBodyTime
*
* DESCRIPTION
*
*    This function sets the <linkBody> of the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_TIME linkBody - <linkBody> value.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_SetLinkBodyTime(
                        IFX_HUTILITY        hUtility,
                        IFX_TIME            linkBody)
{
    IFX_WCHAR strVal[IFXU_URI_INT_BYTE_SIZE];
    char cstrVal[IFXU_URI_INT_BYTE_SIZE];
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (status == IFX_SUCCESS)
    {
        lc_itoa((IFX_INT32)linkBody, cstrVal, 10);
        asciiStringToWideCharString(strVal, cstrVal, IFXU_URI_INT_BYTE_SIZE);

        status = IFXU_UriParser_SetLinkBodyString(hUtility, &strVal[0]);
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_SetLinkBodyBool
*
* DESCRIPTION
*
*    This function sets the <linkBody> of the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_BOOL linkBody - <linkBody> value.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_SetLinkBodyBool(
                        IFX_HUTILITY        hUtility,
                        IFX_BOOL            linkBody)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (status == IFX_SUCCESS)
    {
        status = IFXU_UriParser_SetLinkBodyString(hUtility, linkBody ? L"true" : L"false");
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_SetParamString
*
* DESCRIPTION
*
*    This function adds string parameter to the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szParam - Name of the parameter.
*
*    const IFX_WCHAR* szValue - Value of the parameter.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_SetParamString(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        const IFX_WCHAR    *szValue)
{
    URI_Parser *parser  = hUtility;
    URI_Param  *param   = NULL;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (parser == NULL)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if ( (szParam == NULL) || (szValue == NULL))
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        status = findOrCreateParam(parser, szParam, &param);

        if (status == IFX_SUCCESS)
        {
            /* Release memory if we are resetting parameter */
            if (param->szVal)
            {
                 IFXE_FreeMemoryUnsafe(param->szVal);
                 param->szVal = NULL;
            }

            /* Allocate memory for value and copy it */
            param->szVal = IFXE_AllocateMemoryUnsafe(sizeof(IFX_WCHAR) * (lc_wcslen(szValue) +1));
            if (param->szVal == NULL)
            {
                (void)destroyParam(parser, param);
                status = IFX_ERROR_MEMORY;
            }

            else
            {
                lc_wcscpy(param->szVal, szValue);
                status = IFX_SUCCESS;
            }
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_SetParamInt
*
* DESCRIPTION
*
*    This function adds integer parameter to the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szParam - Name of the parameter.
*
*    IFX_INT32 value - Value of the parameter.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS IFXU_UriParser_SetParamInt(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_INT32           value)
{
    IFX_WCHAR strVal[IFXU_URI_INT_BYTE_SIZE];
    char cstrVal[IFXU_URI_INT_BYTE_SIZE];
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (szParam == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        lc_itoa(value, cstrVal, 10);
        asciiStringToWideCharString(strVal, cstrVal, IFXU_URI_INT_BYTE_SIZE);

        status = IFXU_UriParser_SetParamString(hUtility, szParam, &strVal[0]);
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_SetParamFloat
*
* DESCRIPTION
*
*    This function adds float parameter to the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szParam - Name of the parameter.
*
*    float value - Value of the parameter.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS IFXU_UriParser_SetParamFloat(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        float               value)
{
    IFX_WCHAR strVal[IFXU_URI_FLOAT_BYTE_SIZE];
    char cstrVal[IFXU_URI_FLOAT_BYTE_SIZE];
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (szParam == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        lc_sprintf(cstrVal, "%f", value);
        asciiStringToWideCharString(strVal, cstrVal, IFXU_URI_FLOAT_BYTE_SIZE);

        status = IFXU_UriParser_SetParamString(hUtility, szParam, &strVal[0]);
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_SetParamTime
*
* DESCRIPTION
*
*    This function adds time parameter to the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szParam - Name of the parameter.
*
*    IFX_TIME value - Value of the parameter.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS IFXU_UriParser_SetParamTime(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_TIME            value)
{
    IFX_WCHAR strVal[IFXU_URI_INT_BYTE_SIZE];
    char cstrVal[IFXU_URI_INT_BYTE_SIZE];
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (szParam == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        lc_itoa((IFX_INT32)value, cstrVal, 10);
        asciiStringToWideCharString(strVal, cstrVal, IFXU_URI_INT_BYTE_SIZE);

        status = IFXU_UriParser_SetParamString(hUtility, szParam, &strVal[0]);
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    IFXU_UriParser_SetParamBool
*
* DESCRIPTION
*
*    This function adds boolean parameter to the URI.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    const IFX_WCHAR* szParam - Name of the parameter.
*
*    IFX_BOOL value - Value of the parameter.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS IFXU_UriParser_SetParamBool(
                        IFX_HUTILITY        hUtility,
                        const IFX_WCHAR    *szParam,
                        IFX_BOOL            value)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (hUtility == 0)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (szParam == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        status = IFXU_UriParser_SetParamString(hUtility, szParam, value ? L"true" : L"false");
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    URI_GetLinkStringSize
*
* DESCRIPTION
*
*    This function returns back the size of the final URI string.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_INT32 size - Contains size of the URI string.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetLinkStringSize(
                        IFX_HUTILITY        hUtility,
                        IFX_INT32          *size)
{
    URI_Parser *parser = hUtility;
    URI_Param  *param;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (parser == NULL)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (size == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        /* Initialize to 1 for the NULL terminator */
        *size = 1;

        if (parser->szLinkPrefix)
        {
            *size += (IFX_INT32)calcUriStrlen(parser->szLinkPrefix) + 3;  /* link + :// */
        }

        if (parser->szBody)
        {
            *size += (IFX_INT32)calcUriStrlen(parser->szBody);
        }

        /* process parameters */
        param = parser->param;

        if(param)
            (*size)++;      /* body delimiter '?' is present only if there is at least one parameter */

        while (param)
        {
            if (param->szKey)
            {
                *size += (IFX_INT32)calcUriStrlen(param->szKey) + 1;  /* key + delimiter '=' */

                if (param->szVal)
                {
                    *size += (IFX_INT32)calcUriStrlen(param->szVal);
                }

                if (param->next)
                    (*size)++; /* value delimiter ';' */
            }

            param = param->next;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    URI_GetLinkString
*
* DESCRIPTION
*
*    This function returns back the final URI string.
*
* INPUTS
*
*    IFX_HUTILITY hUtility - This is pointer to the session object.
*
*    IFX_WCHAR* szLinkString - Pointer to the buffer to copy URI string. This
*    should be larger enough to occupy URI string and the null terminator.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
IFX_RETURN_STATUS   IFXU_UriParser_GetLinkStringData(
                        IFX_HUTILITY        hUtility,
                        IFX_WCHAR          *szLinkString)
{
    URI_Parser* parser = hUtility;
    URI_Param*  param;
    IFX_WCHAR* strPos = szLinkString;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (parser == NULL)
        status = IFXU_URI_ERROR_INVALID_HANDLE;

    if (szLinkString == NULL)
        status = IFXU_URI_ERROR_INVALID_PARAM;

    if (status == IFX_SUCCESS)
    {
        if (parser->szLinkPrefix)
        {
            strPos = appendEscapeChar(strPos, parser->szLinkPrefix);
            /* Normal copy, we this to be copied as it is */
            lc_wcscpy(strPos, L"://");
            strPos += 3;
        }

        if (parser->szBody)
        {
            strPos = appendEscapeChar(strPos, parser->szBody);
        }

        /* process parameters */
        param = parser->param;

        if(param)
        {
            /* body delimiter '?' is only present if there is at least one parameter */
            lc_wcscpy(strPos, L"?");
            strPos++;
        }

        while (param)
        {
            if (param->szKey)
            {
                /* key + delimiter '=' */
                strPos = appendEscapeChar(strPos, param->szKey);
                lc_wcscpy(strPos, L"=");
                strPos++;

                if (param->szVal)
                {
                    strPos = appendEscapeChar(strPos, param->szVal);
                }

                /* separate each parameter with ';' */
                if (param->next)
                {
                    lc_wcscpy(strPos, L";"); /* value delimiter ';' */
                    strPos++;
                }
            }

            param = param->next;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    createURI
*
* DESCRIPTION
*
*    Internal helper function to create Parser structure.
*
* INPUTS
*
*    URI_Parser** hParser - Pointer to the out memory to return address of the
*    URI_Parser structure.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
static IFX_RETURN_STATUS createURI(
                        URI_Parser        **hParser)
{
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    /*  Allocate memory for parser structure    */
    *hParser = IFXE_AllocateMemoryUnsafe(sizeof(URI_Parser));
    if (*hParser == NULL)
    {
        status = IFX_ERROR_MEMORY;
    }

    else
    {
        /* Initialize struct with NULL */
        lc_memset((*hParser), 0, sizeof(URI_Parser));
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    findOrCreateParam
*
* DESCRIPTION
*
*    This function looks up a parameter and create new parameter if not found.
*    This function also allocates memory for key and sets it.
*
* INPUTS
*
*    const URI_Parser* hParser - Pointer to URI_Parser structure.
*
*    const IFX_WCHAR* key - Name of the parameter.
*
*    URI_Param** param - Pointer to the return address of the parameter if found
*    otherwise NULL.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully and new parameter is created.
*    Returns IFXU_URI_PARAM_EXIST if function completes successfully and parameter already
*    exists.
*    otherwise appropriate error code is returned.
*
*
****************************************************************************/
static IFX_RETURN_STATUS findOrCreateParam(URI_Parser *hParser, const IFX_WCHAR *key,
                          URI_Param **param)
{
    IFX_RETURN_STATUS result = lookupParam(hParser, key, param);

    if (result != IFX_SUCCESS)
    {
        result = createParam(hParser, param);

        /* Allocate memory for key and copy it */
        (*param)->szKey = IFXE_AllocateMemoryUnsafe(sizeof(IFX_WCHAR) * (lc_wcslen(key) + 1));
        if ((*param)->szKey == NULL)
        {
            result = IFX_ERROR_MEMORY;
            destroyParam(hParser, (*param));
        }

        else
        {
            lc_wcscpy((*param)->szKey, key);
            result = IFX_SUCCESS;
        }
    }

    return result;
}

/*****************************************************************************
* FUNCTION
*
*    createParam
*
* DESCRIPTION
*
*    Internal helper function to create Parameter structure and also adds to
*    the parameter's list.
*
* INPUTS
*
*    const URI_Parser* hParser - Pointer to URI_Parser structure.
*
*    URI_Param** param - Pointer to the out memory to return address of the
*    newly create parameter structure.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
static IFX_RETURN_STATUS createParam(
                        URI_Parser         *parser,
                        URI_Param         **param )
{
    IFX_RETURN_STATUS status = IFXU_URI_ERROR_GENERAL;
    URI_Param **p;

    if (parser)
    {
        p = &(parser->param);

        while ((*p))
            p = &((*p)->next);

        *p = IFXE_AllocateMemoryUnsafe(sizeof(URI_Param));
        if (*p == NULL)
        {
            status = IFX_ERROR_MEMORY;
        }

        else
        {
            (*p)->szKey = NULL;
            (*p)->szVal = NULL;
            (*p)->next  = NULL;
            (*param)    = (*p);

            status      = IFX_SUCCESS;
        }
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    destroyParam
*
* DESCRIPTION
*
*    Internal helper function to destroy and remove Parameter structure from
*    the parameter's list.
*
* INPUTS
*
*    const URI_Parser* hParser - Pointer to URI_Parser structure.
*
*    URI_Param* param - Pointer to the parameter structure.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if function completes successfully otherwise appropriate
*    Error code is returned.
*
*
****************************************************************************/
static IFX_RETURN_STATUS destroyParam(
                        URI_Parser         *parser,
                        URI_Param          *param)
{
    IFX_RETURN_STATUS status = IFXU_URI_ERROR_GENERAL;
    URI_Param **p;

    if (parser && param)
    {
        status = IFX_SUCCESS;
        p = &(parser->param);

        while ((*p) && (*p) != param)
            p = &(*p)->next;

        /* if found */
        if ((*p) == param)
        {
            (*p) = param->next;
        }

        if (param->szKey)
        {
            IFXE_FreeMemoryUnsafe(param->szKey);
        }

        if (param->szVal)
        {
            IFXE_FreeMemoryUnsafe(param->szVal);
        }

        IFXE_FreeMemoryUnsafe(param);
    }

    return status;
}

/*****************************************************************************
* FUNCTION
*
*    lookupParam
*
* DESCRIPTION
*
*    Internal helper function to lookup parameter from the parameter list.
*
* INPUTS
*
*    const URI_Parser* hParser - Pointer to URI_Parser structure.
*
*    const IFX_WCHAR* key - Name of the parameter.
*
*    URI_Param** param - Pointer to the return address of the parameter if found
*    otherwise NULL.
*
* OUTPUTS
*
*    Returns IFX_SUCCESS if parameter is found otherwise IFXU_URI_ERROR_NOT_FOUND
*    is returned.
*
*
****************************************************************************/
static IFX_RETURN_STATUS lookupParam(
                        URI_Parser         *hParser,
                        const IFX_WCHAR    *key,
                        URI_Param         **param)
{
    IFX_RETURN_STATUS result = IFXU_URI_ERROR_NOT_FOUND;
    URI_Param *p;

    if (hParser)
    {
        p = hParser->param;

        while (p)
        {
            if (lc_wcscmpi(key, p->szKey) == 0)
            {
                (*param)  = p;
                result = IFX_SUCCESS;
                break;
            }

            p = p->next;
        }
    }

    return result;
}

/*****************************************************************************
* FUNCTION
*
*    isValidURI
*
* DESCRIPTION
*
*    Internal helper function used to determine whether given URI String is
*    valid.
*
* INPUTS
*
*    const IFX_WCHAR* uriStr - URI string to be analyzed.
*
* OUTPUTS
*
*    Returns IFX_TRUE if uriStr is a valid URI otherwise IFX_FALSE.
*
*
****************************************************************************/
static IFX_INT32 isValidURI( const IFX_WCHAR *uriStr )
{
    const IFX_WCHAR *cp = uriStr;

    while (*cp)
    {
        if (!lc_wcsncmp(cp, URI_LINKPREFIX, 3))
        {
            // We have matched the link prefix.

            // Check that it isn't escaped.
            if ( (cp == uriStr) ||
                 (*(cp - 1) != URI_ESCAPE_CHARACTER) )
            {
                return IFX_TRUE;
            }
        }
        cp++;
    }

    return IFX_FALSE;
}

/*****************************************************************************
* FUNCTION
*
*    escapeCharLen
*
* DESCRIPTION
*
*    This function searches escape characters in the given string and returns
*    string length and number of escape characters found.
*
* INPUTS
*
*    const URI_WCHAR* wcs - This is the source string.
*
*    IFX_INT32 length - this is the length of the wcs string.
*
*
* OUTPUTS
*
*    returns string length.
*
* EXAMPLE
*    wcs = "Price$=100$$" returns  2
*
****************************************************************************/
static IFX_INT32 escapeCharLen(
                        const IFX_WCHAR    *wcs,
                        IFX_INT32           length)
{
    const IFX_WCHAR *eos = wcs;
    IFX_INT32 escapeLen = 0;

    while( length-- > 0 ){
        if(*eos++ == URI_ESCAPE_CHARACTER)
        {
            escapeLen++;

            if(*eos)
                eos++;                      /* skip escaped character */
        }
    }

    return escapeLen;
}


/*****************************************************************************
* FUNCTION
*
*    removeEscapeChar
*
* DESCRIPTION
*
*    Copies the string src into the dest removing all the application escape
*    characters.
*
* INPUTS
*
*    URI_WCHAR* dest - This would be the destination string buffer. This buffer
*    should be large enough to accommodate all characters + '\0'.
*
*    const URI_WCHAR* src - This is the source string.
*
*    IFX_INT32 length - This is the length of the src string
*
* OUTPUTS
*
*    pointer to the dest is returned.
*
* EXAMPLE
*    src = "Price$=100$$" results dest =  "Price=100$"
*
****************************************************************************/
static IFX_WCHAR* removeEscapeChar(
                        IFX_WCHAR          *dest,
                        const IFX_WCHAR    *src,
                        IFX_INT32           length)
{
    IFX_WCHAR* cp = dest;

    if(dest){
        while( length-- > 0 )
        {
            if(*src == URI_ESCAPE_CHARACTER)
                src++;

            *cp++ = *src++;
        }

        *cp = '\0';
    }

    return( dest );
}

static IFX_INT32 isSpecialChar(IFX_INT32 ch)
{
    switch(ch)
    {
        case ';':
        case '=':
        case '?':
        case '&':
        {
            return IFX_TRUE;
        }
    }

    return IFX_FALSE;
}
/*****************************************************************************
* FUNCTION
*
*    appendEscapeChar
*
* DESCRIPTION
*
*    Copies the string src into the dest guarding all the application special
*    characters with an escape character.
*
* INPUTS
*
*    URI_WCHAR* dest - This would be the destination string buffer.
*
*    const URI_WCHAR* - This is the source string.
*
* OUTPUTS
*
*    Returns pointer to the end of the string. Which can be used to append
*    more characters at the end of the string.
*
* EXAMPLE
*    src = "Price=100$" results dest = "Price$=100$$"
*
****************************************************************************/
static IFX_WCHAR* appendEscapeChar(
                        IFX_WCHAR          *dest,
                        const IFX_WCHAR    *src)
{
        IFX_WCHAR* cp = dest;

        while(1)
        {
            if(isSpecialChar(*src))
            *cp++  = URI_ESCAPE_CHARACTER;

            if(0 == (*cp++ = *src++))
                break;
        }

        return( cp == dest ? cp : cp - 1);
}

/*****************************************************************************
* FUNCTION
*
*    checkLinkPrefix
*
* DESCRIPTION
*
*    Looks through the string pointed to by szLinkPrefix checking that no
*    non alphanumeric characters are used.
*
* INPUTS
*
*    szLinkPrefix - String to check.
*
* OUTPUTS
*
*    IFX_TRUE    -   string contains no non-valid characters.
*    IFX_FALSE   -   string contains at least one non-valid character.
*
****************************************************************************/
static unsigned char checkLinkPrefix(
                        const IFX_WCHAR    *szLinkPrefix)
{
    const IFX_WCHAR* pCh = szLinkPrefix;

    while(*pCh != 0)
    {
        if( (*pCh >= '0' && *pCh <= '9') ||
            (*pCh >= 'A' && *pCh <= 'Z') ||
            (*pCh >= 'a' && *pCh <= 'z') ||
            (*pCh == '_'))
        {
             /* character passes test - move on to next character */
            pCh++;
        }
        else
        {
             /* test fails - bail out immediately */
            return IFX_FALSE;
        }
    }

    /* no more characters to test - we must be successful */
    return IFX_TRUE;
}

/*****************************************************************************
* FUNCTION
*
*    calcUriStrlen
*
* DESCRIPTION
*
*    This function calculates length of the input string + number of special
*    special characters. This function can be used to calculate buffer length
*    for URI output string.
*
* INPUTS
*
*    URI_WCHAR* wcs - Null terminated string.
*
* OUTPUTS
*
*    Returns length of the wcs string + number of special characters
*
* EXAMPLE
*    wcs = "Price=" returns 7
*
****************************************************************************/
static IFX_INT32 calcUriStrlen(const IFX_WCHAR *wcs)
{
    IFX_INT32 len = 0;
    const IFX_WCHAR* pCh = wcs;

    while(*pCh)
    {
        /* We need to append a escape character for special character*/
        if(isSpecialChar(*wcs))
            len++;

        len++;
        pCh++;
    }

    return len;

} /* End of calcUriStrlen function. */


/*****************************************************************************
* FUNCTION
*
*    asciiStringToWideCharString
*
* DESCRIPTION
*
*    Copies an Ascii string into a wide-char string
*
* INPUTS
*
*    const char * asciiStr - Null terminated Ascii string.
*    size_t maxSize        - Maximum size of output string
*
* OUTPUTS
*
*    IFX_WCHAR* wcStr      - Null terminated wide-char string.
*
****************************************************************************/
void asciiStringToWideCharString(IFX_WCHAR* wcStr, const char* asciiStr, int maxSize)
{
    const IFX_WCHAR* maxDestPtr = wcStr + maxSize - 1; /* leave room for NULL terminator */

    while (*asciiStr != '\0' && wcStr < maxDestPtr)
    {
        *wcStr++ = *asciiStr++;
    }
    *wcStr = '\0';
}

/*****************************************************************************
* FUNCTION
*
*    strToBool
*
* DESCRIPTION
*
*    Interprets a string as a bool
*
* INPUTS
*
*    IFX_WCHAR* wcStr - Null terminated input string.
*
* OUTPUTS
*
*    IFX_BOOL         - Boolean interpretation of input string
*
****************************************************************************/
IFX_BOOL strToBool(IFX_WCHAR* wcStr)
{
    return (lc_wcscmpi(wcStr, L"1") == 0)
        || (lc_wcscmpi(wcStr, L"true") == 0)
        || (lc_wcscmpi(wcStr, L"yes") == 0)
        || (lc_wcscmpi(wcStr, L"y"  ) == 0);
}

