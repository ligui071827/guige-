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
#ifndef IFXUI_RTL_H
#define IFXUI_RTL_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <ctype.h>

#include "inflexionui/engine/ifxui_defs.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*
    Macros that configure the NDE runtime library usage.
*/

#if (defined(__CC_ARM) && (defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 200000))) || defined(__TMS470__)
    #define LC_USE_NDE_RTL_WIDE_CHAR
    typedef unsigned short IFX_WCHAR;
#elif defined (ANDROID)
    #define LC_USE_NDE_RTL_WIDE_CHAR
    typedef unsigned short IFX_WCHAR;
#elif (defined(__GNUC__) || defined(__GCC32__)) && !(defined(WIN32) || defined(_WIN32))
    typedef wchar_t IFX_WCHAR;
    #define LC_USE_NDE_RTL_WIDE_CHAR
#elif defined(_MCCCF) || defined(_CCCCF) || defined(_MCCPPC) || defined(__mips__)
    #define LC_USE_NDE_RTL_WIDE_CHAR
    typedef wchar_t IFX_WCHAR;
#elif !(defined(WIN32) || defined(_WIN32))
    #include <wchar.h>
    typedef wchar_t IFX_WCHAR;
#else
    typedef wchar_t IFX_WCHAR;
#endif

/* The standard library time() function does not work on target */
#if (defined(__CC_ARM) || defined(__TMS470__) || defined(_MRI))
    #define LC_USE_NDE_RTL_TIME
#endif

/* These function differ slightly on these platforms. */
#if defined(WIN32) || defined(_WIN32)
    #define lc_strcmpi      _stricmp
    #define lc_strnicmp     _strnicmp
    #define lc_wcscmpi      _wcsicmp
    #define lc_strdup       _strdup
    #define lc_itoa         _itoa
    #define lc_strstr       strstr

#else
    #if defined(__SYMBIAN32__)
        #define lc_strcmpi      strcasecmp
        #define lc_strnicmp     strncasecmp
        #define lc_wcscmpi      wcscasecmp
    #else
        extern int              rtl_strcmpi(const char*,    const char*);
        extern int              rtl_wcscmpi(const IFX_WCHAR*, const IFX_WCHAR*);
        extern int              rtl_strnicmp(const char* p1, const char* p2, int count);
        #define lc_strcmpi      rtl_strcmpi
        #define lc_strnicmp     rtl_strnicmp
        #define lc_wcscmpi      rtl_wcscmpi
    #endif
    #define lc_strdup       strdup

    /* itoa */
    #if defined (__CC_ARM) || defined (_MRI) || defined(__GNUC__) || defined(__GCC32__)
        extern void rtl_itoa(int n, char *s, int base);
        #define lc_itoa         rtl_itoa
    #else
        #define lc_itoa         itoa
    #endif

    /* strstr. Check for the GCC 4.2.3 version. */
    #if ((__GNUC__ > 4) || (__GNUC__ == 4)) && \
        ((__GNUC_MINOR__ > 2) || (__GNUC_MINOR__ == 2)) && \
        ((__GNUC_PATCHLEVEL__ > 3) || (__GNUC_PATCHLEVEL__ == 3))
        extern  char *          rtl_strstr(const char *s1,  const char *s2);
        #define lc_strstr       rtl_strstr
    #else
        #define lc_strstr       strstr
    #endif
#endif

#if defined(_MSC_VER)
    #define lc_snprintf(a, b, ...)  _snprintf_s(a, b , b - 1, __VA_ARGS__)
#else
    #define lc_snprintf  snprintf

#endif

/* Time functions */
#if defined (LC_USE_NDE_RTL_TIME)
    extern time_t rtl_time(time_t *T);
    #define lc_time         rtl_time
#else
    #define lc_time  time
#endif

#define DEFAULT_TIME_FORMAT L"%H:%M %d/%m/%y"

/* Printf definition. */
#if defined(__CC_ARM) && (defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 200000))
    #define lc_printf
#else
    #define lc_printf       printf
#endif

/* These are our own versions of the wide char run time libraries for platforms
   without full wide char support */
#if defined(LC_USE_NDE_RTL_WIDE_CHAR)

    extern IFX_WCHAR*       rtl_wcstok(IFX_WCHAR*,        const IFX_WCHAR*,     IFX_WCHAR**);
    extern int              rtl_wcscmp(const IFX_WCHAR*,  const IFX_WCHAR*);
    extern int              rtl_wcsncmp(const IFX_WCHAR*, const IFX_WCHAR*,     size_t);
    extern int              rtl_wcslen(const IFX_WCHAR*);
    extern IFX_WCHAR*       rtl_wcscpy(IFX_WCHAR*,        const IFX_WCHAR*);
    extern IFX_WCHAR*       rtl_wcsncpy(IFX_WCHAR*,       const IFX_WCHAR*,     size_t);
    extern IFX_WCHAR*       rtl_wcscat(IFX_WCHAR *,       const IFX_WCHAR*);
    extern size_t           rtl_wcsftime(IFX_WCHAR *strDest, size_t maxsize, const IFX_WCHAR *format, const struct tm *timeptr);
    extern long             rtl_wcstol(const IFX_WCHAR *nptr,     IFX_WCHAR **endptr,   int base);
    extern unsigned long    rtl_wcstoul(const IFX_WCHAR *nptr,    IFX_WCHAR **endptr,   int base);
    extern double           rtl_wcstod(const IFX_WCHAR *nptr,    IFX_WCHAR **endptr);
    extern size_t           rtl_wcstombs(char *mbstr,     const IFX_WCHAR *wcstr,       size_t count);


    #define lc_wcstok       rtl_wcstok
    #define lc_wcscmp       rtl_wcscmp
    #define lc_wcsncmp      rtl_wcsncmp
    #define lc_wcslen       rtl_wcslen
    #define lc_wcscpy       rtl_wcscpy
    #define lc_wcsncpy      rtl_wcsncpy
    #define lc_wcscat       rtl_wcscat
    #define lc_wcsftime     rtl_wcsftime
    #define lc_wcstoul      rtl_wcstoul
    #define lc_wcstol       rtl_wcstol
    #define lc_wcstod       rtl_wcstod
    #define lc_wcstombs     rtl_wcstombs

#else
    #if defined(WIN32) || defined(_WIN32)
        #define lc_wcstok(a, b, c)          wcstok(a, b);
    #else
        #define lc_wcstok   wcstok
    #endif

    #define lc_wcscmp       wcscmp
    #define lc_wcslen       wcslen
    #define lc_wcscpy       wcscpy
    #define lc_wcsncpy      wcsncpy
    #define lc_wcscat       wcscat
    #define lc_wcstoul      wcstoul
    #define lc_wcstod       wcstod
    #define lc_wcstombs     wcstombs

    #if (defined(__CC_ARM)) || (defined(_MRI))
        extern size_t           rtl_wcsftime(IFX_WCHAR *strDest, size_t maxsize, const IFX_WCHAR *format, const struct tm *timeptr);
        #define lc_wcsftime     rtl_wcsftime
    #else
        #define lc_wcsftime     wcsftime
    #endif

    #if (defined(_MRI))
        extern long             rtl_wcstol(const IFX_WCHAR *nptr,     IFX_WCHAR **endptr,   int base);
        extern int              rtl_wcsncmp(const IFX_WCHAR*, const IFX_WCHAR*,     size_t);
        #define lc_wcstol       rtl_wcstol
        #define lc_wcsncmp      rtl_wcsncmp
    #else
        #define lc_wcstol       wcstol
        #define lc_wcsncmp      wcsncmp
    #endif
#endif


/* These are the standard definitions */
    #define lc_sprintf      sprintf
    #define lc_strncmp      strncmp
    #define lc_strcmp       strcmp
    #define lc_strcat       strcat
    #define lc_strncat      strncat
    #define lc_memset       memset
    #define lc_memcmp       memcmp
    #define lc_isalnum      isalnum
    #define lc_isspace      isspace
    #define lc_isdigit      isdigit
    #define lc_isalpha      isalpha
    #define lc_sscanf       sscanf
    #define lc_strtok       strtok
    #define lc_strchr       strchr
    #define lc_strcpy       strcpy
    #define lc_strlen       strlen
    #define lc_cos          cos
    #define lc_sin          sin
    #define lc_tan          tan
    #define lc_acos         acos
    #define lc_asin         asin
    #define lc_atan         atan
    #define lc_fabs         fabs
    #define lc_fmod         fmod
    #define lc_pow          pow
    #define lc_strncpy      strncpy
    #define lc_strrchr      strrchr
    #define lc_atoi         atoi
    #define lc_sqrt         sqrt
    #define lc_toupper      toupper
    #define lc_towupper     towupper
    #define lc_mbtowc       mbtowc
    #define lc_mbstowcs     mbstowcs
    #define lc_wctomb       wctomb
    #define lc_gmtime       gmtime
    #define lc_localtime    localtime
    #define lc_strtod       strtod
    #define lc_floor        floor
    #define lc_ceil         ceil
    #define lc_strcspn      strcspn

/*****************************************************************************
    These RTL functions are not enabled under Nucleus so should not appear
    in any generic code.

    If a different platform is using these they can be used in the platform
    specific classes.

    malloc

    fopen
    fclose
    fseek
    fscanf
    fsetpos
    ftell
    fread
    fgets
    fgetc
    fwrite
    fputs
    fputc
*/

#ifdef __cplusplus
}
#endif
#endif /* IFXUI_RTL_H */
