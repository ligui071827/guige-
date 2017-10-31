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

/*-------------------------------------------------------------------------*//**
    This file contains the rtl string functions that are missing from some
    compiler implementations. They are mainly wide char related.

    The header is ifxui_rtl.h
*/
#include "inflexionui/engine/ifxui_defs.h"
#include "inflexionui/engine/ifxui_porting.h"
#include "inflexionui/engine/ifxui_rtl.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#if (defined(_MRI) || defined (LC_USE_NDE_RTL_WIDE_CHAR) )
int rtl_iswdigit(IFX_WCHAR digit);
int rtl_iswxdigit(IFX_WCHAR digit);
int rtl_iswlower(IFX_WCHAR C);
IFX_WCHAR rtl_towupper(IFX_WCHAR C);
long rtl_wcstol(const IFX_WCHAR *nptr, IFX_WCHAR **endptr, int base);
int rtl_wcsncmp(const IFX_WCHAR* p1, const IFX_WCHAR* p2, size_t length);
#endif /* #if (defined(_MRI) || defined (LC_USE_NDE_RTL_WIDE_CHAR) ) */

/*-------------------------------------------------------------------------*//**
    rtl_strcmpi
*/
int rtl_strcmpi(const char* p1, const char* p2)
{
    /* Find first position that differs or terminates (case insensitive) */
    while (*p1 && *p2 && lc_toupper((int)*p1) == lc_toupper((int)*p2))
        ++p1, ++p2;

    /* Return signed difference */
    return (int)(lc_toupper((int)*p1)) - (int)(lc_toupper((int)*p2));
}

/*-------------------------------------------------------------------------*//**
    rtl_strnicmp
*/
int rtl_strnicmp(const char* p1, const char* p2, int count)
{
    int result = 0;

    // If count 0, or paramerts invalid, no need to go further
    if (count == 0 || !p1 || !p2)
        return result;

    do
    {
        // Go through character by character and see if the substring is equal
        result = lc_toupper((int)*p1) - lc_toupper((int)*p2);
    } while(--count > 0 && *p1++ && *p2++ && result == 0);

    /* Return result */
    return result;
}

/*-------------------------------------------------------------------------*//**
    rtl_wcscmpi
*/
int rtl_wcscmpi(const IFX_WCHAR* p1, const IFX_WCHAR* p2)
{
    /* Find first position that differs or terminates (case insensitive) */
    while (*p1 && *p2 && lc_toupper(*p1) == lc_toupper(*p2))
        ++p1, ++p2;

    /* Return signed difference */
    return (int)(lc_toupper(*p1)) - (int)(lc_toupper(*p2));
}

#ifdef LC_USE_NDE_RTL_WIDE_CHAR

/*-------------------------------------------------------------------------*//**
    rtl_wcstok
*/
IFX_WCHAR* rtl_wcstok(IFX_WCHAR* wcs, const IFX_WCHAR* delim, IFX_WCHAR** ptr)
{
    const IFX_WCHAR* pDelim = delim;
    IFX_WCHAR* startPos = NULL;
    IFX_WCHAR* strPos = NULL;

    if (delim == NULL || ptr == NULL)
        return NULL;

    /* Decide on the string start position */
    if (wcs)
    {
        startPos = wcs;

        /* If this is a new string avoid the delimiter at the start. */
        while (*pDelim != '\0')
        {
            if (*startPos == *pDelim)
            {
                ++startPos;

                // Reset the delimiter check to check the next letter.
                pDelim = delim;
                break;
            }
            ++pDelim;
        }
    }
    else if (*ptr)
        startPos = *ptr;
    else
        return NULL;

    // Check that the start character is not already the end of the string
    if (*startPos == '\0')
    {
        *ptr = NULL;
        return NULL;
    }

    /* Find the next Token */
    strPos = startPos;
    while(*strPos != '\0')
    {
        /* Check if any of the delimiters match */
        pDelim = delim;
        while (*pDelim != '\0')
        {
            if (*strPos == *pDelim)
            {
                *strPos = '\0';
                *ptr = strPos + 1;
                return startPos;
            }
            ++pDelim;
        }
        ++strPos;
    }

    *ptr = NULL;
    return startPos;
}

/*-------------------------------------------------------------------------*//**
    rtl_wcscmp
*/
int rtl_wcscmp(const IFX_WCHAR* p1, const IFX_WCHAR* p2)
{
    /* Find first position within length that differs or terminates (case sensitive) */
    while (*p1 && *p2 && (*p1 == *p2))
        ++p1, ++p2;

    /* Return signed difference */
    return (int)(*p1) - (int)(*p2);
}

/*-------------------------------------------------------------------------*//**
    rtl_wcslen
*/
int rtl_wcslen(const IFX_WCHAR* str)
{
    int count = 0;

    if(str == NULL)
        return 0;

    /* Count the characters */
    while(*str != '\0')
        ++str, ++count;

    return count;
}

/*-------------------------------------------------------------------------*//**
    rtl_wcscpy
*/
IFX_WCHAR* rtl_wcscpy(IFX_WCHAR* dest, const IFX_WCHAR* src)
{
    IFX_WCHAR* pDest = dest;

    if (src == NULL || dest == NULL)
        return dest;

    /* Copy the main string */
    while(*src != '\0')
    {
        *pDest = *src;
        ++src;
        ++pDest;
    }

    /* Set the terminating null character */
    *pDest = '\0';

    return dest;
}

/*-------------------------------------------------------------------------*//**
    rtl_wcsncpy
*/
IFX_WCHAR* rtl_wcsncpy(IFX_WCHAR* dest, const IFX_WCHAR* src, size_t count)
{
    int i = 0;
    IFX_WCHAR* pDest = dest;

    if (src == NULL || dest == NULL)
        return dest;

    /* Copy the required number of characters, padding with '\0' */
    for(i = 0; i < count; i++)
    {
        if(*src != '\0')
        {
            *pDest = *src;
            ++src;
        }
        else
        {
            *pDest = '\0';
        }
        ++pDest;
    }

    return dest;
}

/*-------------------------------------------------------------------------*//**
    rtl_wcscat
*/
IFX_WCHAR *rtl_wcscat(IFX_WCHAR *s1, const IFX_WCHAR *s2)
{
    IFX_WCHAR *pDest = s1;
    pDest += rtl_wcslen(s1);
    rtl_wcscpy(pDest, s2);
    return s1;
}


/*-------------------------------------------------------------------------*//**
    rtl_wcstoul
*/
unsigned long rtl_wcstoul(const IFX_WCHAR *nptr, IFX_WCHAR **endptr, int base)
{
    long result = 0;
    unsigned long value;

    if ( *nptr == L'-' )
    {
        *endptr = (IFX_WCHAR *)nptr;
        return result;
    }

    if (!base)
    {
        base = 10;
        if (*nptr == L'0')
        {
            base = 8;
            nptr++;
            if ((*nptr == L'x') && rtl_iswxdigit(nptr[1]))
            {
                nptr++;
                base = 16;
            }
        }
    }

    do
    {
        if (!rtl_iswxdigit(*nptr))
        {
            break;
        }

        if (rtl_iswdigit(*nptr))
        {
            value = *nptr - L'0';
        }

        else
        {
            if (rtl_iswlower(*nptr))
            {
                rtl_towupper(*nptr);
            }

            value = *nptr - L'A' + 10;
        }

        if (value < base)
        {
            result = result*base + value;
            nptr++;
        }

        else
        {
            break;
        }
    } while(1);

    if (endptr)
    {
        *endptr = (IFX_WCHAR *)nptr;
    }

    return result;
}

/*-------------------------------------------------------------------------*//**
    rtl_wcstod

    [+|-]DIGITS[.][DIGITS][(e|E)[+|-]DIGITS]

*/
double rtl_wcstod(const IFX_WCHAR *nptr, IFX_WCHAR **endptr)
{
    char charArray[256];
    char *ptr;
    double result;
    int i;

    /* Convert the wide char into char for strtod */
    for (i=0; (i<lc_wcslen(nptr)) && (i<255); i++)
    {
        charArray[i] = (char)nptr[i];
    }
    charArray[i] = 0;

    result = lc_strtod(charArray, &ptr);

    if (endptr)
    {
        *endptr = (IFX_WCHAR*)nptr + (ptr - charArray);
    }

    return result;
}

/*-------------------------------------------------------------------------*//**
    rtl_wcstombs

    If the destination string is empty then this function returns the count.
*/
size_t rtl_wcstombs(char *dest, const IFX_WCHAR *src, size_t count)
{
    int wideCharLen = 0;
    int result = 0;
    int i = 0;

    if (src && (count > 0))
    {
        wideCharLen = lc_wcslen(src);

        /* Convert the wide char into char. */
        for (i = 0; (i < wideCharLen) && (i < count); i++)
        {
            if (dest)
                dest[i] = (char)src[i];

            ++result;
        }

        if (dest)
            dest[i] = 0;
    }

    return result;
}

#endif /* LC_USE_NDE_RTL_WIDE_CHAR */

#if (defined(__CC_ARM) || defined(_MRI) || defined (LC_USE_NDE_RTL_WIDE_CHAR) )
/*-------------------------------------------------------------------------*//**
    rtl_wcsftime
*/
size_t rtl_wcsftime(IFX_WCHAR *strDest, size_t maxsize, const IFX_WCHAR *format, const struct tm *timeptr)
{
    char timeFormat[3] = "% ";  /* input for strftime */
    char timeOutput[100];       /* output for strftime */

    const IFX_WCHAR *formatPtr = format;
    IFX_WCHAR *destPtr = strDest;
    IFX_WCHAR *maxDestPtr = strDest + maxsize;

    size_t retVal = 0; /* default return value is error */

    /* Check for sensible input */
    if (strDest && format && timeptr)
    {
        /* Loop over the format string, writing to the destination as we go */
        while (*formatPtr != '\0' && destPtr < maxDestPtr)
        {
            if (*formatPtr == '%') /* Format code found: pass to strftime */
            {
                /* Examine format code */
                formatPtr++;
                if (*formatPtr != '\0')
                {
                    char* timePtr = timeOutput;

                    /* Pass single format code to strftime */
                    timeFormat[1] = (char)*formatPtr;

                    /* (If strftime fails, don't output anything) */
                    if (0 != strftime(timeOutput, 100, timeFormat, timeptr))
                    {
                        /* Copy (narrow) strftime output to (wide) destination */
                        while (*timePtr != '\0' && destPtr < maxDestPtr)
                        {
                            *destPtr++ = *timePtr++;
                        }
                    }

                    /* Move to next format char */
                    formatPtr++;
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
#endif /* #if (defined(__CC_ARM) || defined(_MRI) || defined (LC_USE_NDE_RTL_WIDE_CHAR) ) */

#if (defined(_MRI) || defined (LC_USE_NDE_RTL_WIDE_CHAR) )
/*-------------------------------------------------------------------------*//**
    rtl_iswdigit
*/
int rtl_iswdigit(IFX_WCHAR digit)
{
    if ( (digit >= L'0') && (digit <= L'9') )
    {
        return 1;
    }
    return 0;
}

/*-------------------------------------------------------------------------*//**
    rtl_iswxdigit
*/
int rtl_iswxdigit(IFX_WCHAR digit)
{
    if ( (digit >= L'A') && (digit <= L'F') )
    {
        return 1;
    }
    if ( (digit >= L'a') && (digit <= L'f') )
    {
        return 1;
    }
    return rtl_iswdigit(digit);
}

/*-------------------------------------------------------------------------*//**
    rtl_iswlower
*/
int rtl_iswlower(IFX_WCHAR C)
{
    if ( (C >= L'a') && (C <= L'z') )
    {
        return 1;
    }
    return 0;
}

/*-------------------------------------------------------------------------*//**
    rtl_towupper
*/
IFX_WCHAR rtl_towupper(IFX_WCHAR C)
{
    if ( (C >= L'a') && (C <= L'z') )
    {
        return (C - L'a' + L'A');
    }
    return C;
}

/*-------------------------------------------------------------------------*//**
    rtl_wcstol
*/
long rtl_wcstol(const IFX_WCHAR *nptr, IFX_WCHAR **endptr, int base)
{
    long result = 0;
    long value;
    int sign = 1;

    if ( *nptr == L'-' )
    {
        sign = -1;
        nptr++;
    }

    if (!base)
    {
        base = 10;
        if (*nptr == L'0')
        {
            base = 8;
            nptr++;
            if ((*nptr == L'x') && rtl_iswxdigit(nptr[1]))
            {
                nptr++;
                base = 16;
            }
        }
    }

    do
    {
        if (!rtl_iswxdigit(*nptr))
        {
            break;
        }

        if (rtl_iswdigit(*nptr))
        {
            value = *nptr - L'0';
        }

        else
        {
            if (rtl_iswlower(*nptr))
            {
                rtl_towupper(*nptr);
            }

            value = *nptr - L'A' + 10;
        }

        if (value < base)
        {
            result = result*base + value;
            nptr++;
        }

        else
        {
            break;
        }
    } while(1);

    if (endptr)
    {
        *endptr = (IFX_WCHAR *)nptr;
    }

    return (result * sign);
}

/*-------------------------------------------------------------------------*//**
    rtl_wcsncmp
*/
int rtl_wcsncmp(const IFX_WCHAR* p1, const IFX_WCHAR* p2, size_t length)
{
    if (length == 0)
    {
        return -1;
    }
    /* Find first position within length that differs or terminates (case sensitive) */
    while (*p1 && *p2 && (*p1 == *p2) && (length--))
        ++p1, ++p2;

    if (length != 0)
    {
        /* Return signed difference */
        return (int)(*p1) - (int)(*p2);
    }

    else
    {
        return 0;
    }
}
#endif /* #if (defined(_MRI) || defined (LC_USE_NDE_RTL_WIDE_CHAR) ) */

#if defined (__CC_ARM) || defined (_MRI) || defined(__GNUC__) || defined(__GCC32__)
/*-------------------------------------------------------------------------*//**
    rtl_itoa
*/
void rtl_itoa(int n, char *s, int base)
{
    switch(base)
    {
        case 8:
            sprintf(s, "%o", n);
        break;

        case 16:
            sprintf(s, "%X", n);
        break;

        case 10:
        default:
            sprintf(s, "%d", n);
        break;
    }
}
#endif

#if ((__GNUC__ > 4) || (__GNUC__ == 4)) && \
    ((__GNUC_MINOR__ > 2) || (__GNUC_MINOR__ == 2)) && \
    ((__GNUC_PATCHLEVEL__ > 3) || (__GNUC_PATCHLEVEL__ == 3))
/*-------------------------------------------------------------------------*//**
    rtl_strstr
*/
char *rtl_strstr(const char *s1,const char *s2)
{

    size_t len2;

    len2 = strlen(s2);

    if(!len2)
    {

        return( (char *)s1);
    }

    for(; *s1; ++s1)
    {

        if(*s1 == *s2 && strncmp(s1, s2, len2) == 0)
        {

            return( (char *)s1);
        }
    }

    return(0);

}
#endif

#if defined (LC_USE_NDE_RTL_TIME)
/*-------------------------------------------------------------------------*//**
    rtl_time
*/
time_t rtl_time(time_t *T)
{
    IFX_UINT32 timeUpper;
    IFX_UINT32 timeLower;
    IFXP_Timer_Get_Current_Time(&timeUpper, &timeLower);
    if (T != NULL)
    {
        *T = (time_t)timeUpper*UINT_MAX + (time_t)timeLower;
    }
    return (time_t)timeUpper*UINT_MAX + (time_t)timeLower;
}
#endif

#if defined(__mips__)
/*to satisfy MIPS GNU linker*/
char    *getenv (const char * c)
{
    return 0;
}
#endif
