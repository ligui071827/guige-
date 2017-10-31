/*************************************************************************
*
*            Copyright 2006 Mentor Graphics Corporation
*                         All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
*************************************************************************/

/*************************************************************************
*   FILE NAME
*
*       ifxui_porting_font.c
*
*   COMPONENT
*
*       Inflexion UI Porting Layer.
*
*   DESCRIPTION
*
*       Inflexion UI Engine Porting API Font Implementation for FreeType.
*
*   DATA STRUCTURES
*
*       PL_FONT_SIZE_LIST
*       PL_FONT_GLYPH_CACHE
*       IFXP_FT_INTERNAL_FONT
*

*   FUNCTIONS
*
*       PL_Search_Font_Height
*       IFXP_Text_Open_Font
*       IFXP_Text_Close_Font
*       IFXP_Text_Get_Width
*       IFXP_Text_Get_Char_Height
*       IFXP_Text_Canvas_Create
*       IFXP_Text_Canvas_Destroy
*       PL_Read_Font_Data
*       Read_System_Font_Data
*       PL_Get_Font_File_Path

*   DEPENDENCIES
*
*       ifxui_porting_linux.h
*       ft2build.h
*       freetype.h
*       ftcache.h
*       font_mplus_1c.h
*       ifxui_engine.h
*
*************************************************************************/

/* Inflexion UI Engine includes. */
#include    "inflexionui/porting/inc/ifxui_porting_linux.h"

#ifdef      IFX_USE_NATIVE_FONTS

#include    "ft2build.h"
#include    "freetype2/freetype.h"
#include    "freetype2/ftcache.h"
#include    "inflexionui/engine/ifxui_engine.h"

#include "hb.h"
#include "hb-ft.h"
#include "fribidi.h"
#include "hb-ucdn.h"

/* Default defines related to fonts. */

// Our font name also include its path when passed from Engine
#define     MAX_FONT_NAME_LENGTH            256
#define     FT_MAX_CACHE_SIZE               (128 * 1024)
#define     FONT_SYSTEM_PATH                "/usr/share/fonts"
#define     PL_MAX_FILE_LENGTH              MAX_FONT_NAME_LENGTH

/* Define for number of loop iterations for searching of
   appropriate font height. */
#define     PL_FT_MAX_TRIES_HEIGHT          5

/* Data Structure for caching glyphs. */
typedef struct _PL_FONT_GLYPH_CACHE_LIST_STRUCT PL_FONT_GLYPH_CACHE;

struct _PL_FONT_GLYPH_CACHE_LIST_STRUCT
{
    IFX_UINT32              ft_int_cache_ref_count;     /* Reference counter. */
    FT_UInt                 ft_int_cache_glyph_index;   /* Glyph index. */
    FT_Glyph                ft_int_cache_glyph;         /* Cached glyph. */
    PL_FONT_GLYPH_CACHE    *ft_next_cache_node;         /* Pointer to next node. */
};

/* Data Structure for managing font heights. */

typedef struct _IFXP_FT_INTERNAL_FONT_SIZE_LIST_STRUCT PL_FONT_SIZE_LIST;

struct _IFXP_FT_INTERNAL_FONT_SIZE_LIST_STRUCT
{
    int                 ft_char_height;             /* Character height in pixels. */
    int                 ft_char_width;              /* Character width in pixels. */
    int                 ft_baseline;                /* FT baseline. */
    int                 ft_internal_char_height;    /* Internal FT height. */
    int                 ft_internal_char_width;     /* Internal FT width. */
    PL_FONT_GLYPH_CACHE *ft_cache_list_ptr;
    PL_FONT_SIZE_LIST   *ft_next_node;
};

typedef struct _IFXP_FT_INTERNAL_TEXT_RUN IFXP_FT_INTERNAL_TEXT_RUN;
typedef struct _IFXP_FT_INTERNAL_FONT_STRUCT IFXP_FT_INTERNAL_FONT;

struct _IFXP_FT_INTERNAL_TEXT_RUN
{
    IFXP_FT_INTERNAL_FONT *fallbackFont;
    hb_buffer_t *buffer;
    IFXP_FT_INTERNAL_TEXT_RUN *nextRun;
    IFXP_FT_INTERNAL_TEXT_RUN *prevRun;
    IFX_UINT32 sub_text_start;
};

struct _IFXP_FT_INTERNAL_FONT_STRUCT
{
    FT_Library      ft_library;         /* Handle to FT library. */
    FT_Face         ft_face;            /* Handle to FT face object. */
    FT_GlyphSlot    ft_slot;            /* Handle to FT slot object. */

    hb_font_t       *hb_font;
    hb_unicode_funcs_t *hb_unicode_funcs;

    int             ft_font_buffer_size;/* Size of font buffer. */
    unsigned char  *ft_font_buffer;     /* Pointer to font buffer. */
    int             ft_is_file_font;    /* Pointer to font buffer. */

    unsigned int    ft_used_cache_size;/* Pointer to font buffer. */

    PL_FONT_SIZE_LIST  *ft_current_height_list_ptr;
    PL_FONT_SIZE_LIST  *ft_font_height_list_ptr;
    int                 ft_font_height_list_cnt;

    IFXP_FT_INTERNAL_TEXT_RUN *current_text_runs;
    unsigned int    rtlOut;
} ;

#define     DEFAULT_DRIVE_FOR_FT            "C:"

static IFX_RETURN_STATUS   PL_Get_Font_File_Path(char *file_path,
                                                 IFXE_FILE *handle,
                                                 const IFX_WCHAR *font_name);
static IFX_RETURN_STATUS   PL_Read_Font_Data(IFXP_FT_INTERNAL_FONT *internal_font,
                                             const IFX_WCHAR *font_name);

static void memcpyWithAlpha(unsigned char * dst, unsigned char * src, int size);

static void destroyTextRuns(IFXP_FT_INTERNAL_TEXT_RUN *textRun);

static void cacheRuns(IFXP_FT_INTERNAL_FONT *internal_font, IFXP_FT_INTERNAL_TEXT_RUN *text_run);

static IFX_WCHAR * getNextRun(IFX_WCHAR *text,
                                    int *index,
                                    int *prev_index,
                                    IFX_WCHAR *prev_char,
                                    int *len_substr,
                                    int num_chars,
                                    int base_direction,
                                    int *current_direction_ptr,
                                    int8_t embedding_level_list[]);


/*************************************************************************
*************************************************************************/
static int compareScripts(hb_script_t* currentScript, IFX_WCHAR unicode)
{
    hb_script_t s = ucdn_script_translate[ucdn_get_script(unicode)];

    if (s == HB_SCRIPT_INVALID || s == HB_SCRIPT_UNKNOWN || s == HB_SCRIPT_COMMON || s == HB_SCRIPT_INHERITED)
    {
        return 0;
    }
    else if (*currentScript == HB_SCRIPT_INVALID || *currentScript == HB_SCRIPT_UNKNOWN || *currentScript == HB_SCRIPT_COMMON || *currentScript == HB_SCRIPT_INHERITED)
    {
        *currentScript = s;
        return 0;
    }
    else if (s != *currentScript)
    {
        return 1;
    }
    return 0;
}

/*************************************************************************
*************************************************************************/
static int isCommonScript(IFX_WCHAR unicode)
{
    hb_script_t s = ucdn_script_translate[ucdn_get_script(unicode)];

    if (s == HB_SCRIPT_INVALID || s == HB_SCRIPT_UNKNOWN || s == HB_SCRIPT_COMMON || s == HB_SCRIPT_INHERITED)
    {
        return 1;
    }
    return 0;
}

#define HB_SCRIPT_UNICODE_CMP(t, n, s) compareScripts(s, t[n])

#define HB_SCRIPT_UNICODE(t, n) ucdn_script_translate[ucdn_get_script(t[n])]

#define IFX_SCRIPT_COMMON(t, n) isCommonScript(t[n])

#ifdef IFXP_FALLBACK_FONT_SUPPORT

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

static const IFX_WCHAR* gFallBackFonts[] = {
    L"DroidSansArabic.ttf",
    L"DroidSans.ttf",
    L"DroidSansFallback.ttf"
};

typedef struct _IFXP_INTERNAL_FALL_BACK_FONT_STRUCT
{
    IFXP_FONT  fallback_font_handle;
}IFXP_INTERNAL_FALL_BACK_FONT;

IFXP_INTERNAL_FALL_BACK_FONT  fallback_font_handle_list[ARRAY_COUNT(gFallBackFonts)];

#endif

/*************************************************************************
*   FUNCTION
*
*       PL_Font_Initialize
*
*   DESCRIPTION
*
*       This function initializes the font module.
*
*   INPUTS
*
*      None.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS PL_Font_Initialize()
{
#ifdef IFXP_FALLBACK_FONT_SUPPORT
    IFX_RETURN_STATUS status;
    IFXP_FONT font_handle = NULL;
    int i;
    for (i = 0; i < ARRAY_COUNT(gFallBackFonts); i++ )
    {
        font_handle = NULL;
        status = IFXP_Text_Open_Font(&font_handle,
                                      IFXP_FONT_FACE_NORMAL, /* we just want normal face */
                                      gFallBackFonts[i]);
        if (status == IFX_SUCCESS)
        {
            fallback_font_handle_list[i].fallback_font_handle = font_handle;
        }
        else
        {
            fallback_font_handle_list[i].fallback_font_handle = NULL;
        }
    }
#endif
    return IFX_SUCCESS;
}

/*************************************************************************
*   FUNCTION
*
*       PL_Font_Shutdown
*
*   DESCRIPTION
*
*       This function shutdown the font module.
*
*   INPUTS
*
*      None.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS PL_Font_Shutdown(void)
{
#ifdef IFXP_FALLBACK_FONT_SUPPORT
    int i;
    for (i = 0; i < ARRAY_COUNT(fallback_font_handle_list); i++ )
    {
        if (fallback_font_handle_list[i].fallback_font_handle)
        {
            IFXP_Text_Close_Font(fallback_font_handle_list[i].fallback_font_handle);
            fallback_font_handle_list[i].fallback_font_handle = NULL;
        }
    }
#endif
    return IFX_SUCCESS;
}

#ifdef IFXP_FALLBACK_FONT_SUPPORT
/*************************************************************************
*   FUNCTION
*
*       checkFallbackFonts
*
*   DESCRIPTION
*
*       Consult the list of fallback fonts in the case of a glyph not
*       supported by the primary font.  Given the character causing the,
*       problem, try and find a font that can fill the gap.
*       Note that we need to go through the shaping process to cope with
*       arabic et al.
*
*   INPUTS
*
*      text             - string containing character being being displayed.
*      glyph_index      - if font found, this will be the glyph index
*      glyph_advance    - if font found, this will be the glyph advance
*
*   OUTPUTS
*
*       Pointer to IFXP_FT_INTERNAL_FONT object for relevant fallback
*       font, or NULL if glyph not found.
*
*************************************************************************/
IFXP_FT_INTERNAL_FONT* checkFallbackFonts(IFX_WCHAR* text,
                                          FT_UInt* glyph_index,
                                          FT_UInt* glyph_advance,
                                          hb_direction_t direction,
                                          IFX_INT32 *baseLine)
{
    int count = 0;
    int done = 0;
    IFXP_FT_INTERNAL_FONT* font = NULL;
    unsigned int numChars;
    IFX_RETURN_STATUS status = IFX_SUCCESS;
    hb_buffer_t *buff = hb_buffer_create();

    if (glyph_index == NULL || glyph_advance == NULL || text == NULL)
        return NULL;

    numChars = lc_wcslen(text);

    for (count = 0; count < ARRAY_COUNT(fallback_font_handle_list) && (done == 0); count++)
    {
        IFXP_FT_INTERNAL_FONT *font_handle = (IFXP_FT_INTERNAL_FONT*) (fallback_font_handle_list[count].fallback_font_handle);
        if (font_handle)
        {
            /* Check to see if there are any errors. */
            if (status == IFX_SUCCESS)
            {
                hb_buffer_reset(buff);
                hb_buffer_set_unicode_funcs(buff, font_handle->hb_unicode_funcs);
                hb_buffer_set_direction(buff, direction);

                hb_buffer_add_utf32(buff, text, -1, 0, -1);

                hb_shape(font_handle->hb_font, buff, NULL, 0);

                numChars = hb_buffer_get_length(buff);
                hb_glyph_info_t *glyphInfos = hb_buffer_get_glyph_infos(buff, &numChars);
                hb_glyph_position_t *glyphPos = hb_buffer_get_glyph_positions(buff, &numChars);

                if (numChars > 0)
                {
                    if (glyphInfos[0].codepoint != 0)
                    {
                        *glyph_index = glyphInfos[0].codepoint;
                        *glyph_advance = glyphPos[0].x_advance;
                        font = font_handle;
                        *baseLine = font->ft_current_height_list_ptr->ft_baseline;
                        done = 1;
                    }
                }
            }
        }
    }
    hb_buffer_destroy(buff);

    return font;
}

void setFallbackFontsHeight(IFX_UINT32 font_height)
{
    int count = 0;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    for (count = 0; count < ARRAY_COUNT(fallback_font_handle_list); count++)
    {
        IFXP_FT_INTERNAL_FONT *font_handle = (IFXP_FT_INTERNAL_FONT*) (fallback_font_handle_list[count].fallback_font_handle);
        if (font_handle)
        {
            // Adjust font height
            status = IFXP_Text_Get_Char_Height(font_handle, &font_height);
        }
    }
}

IFXP_FT_INTERNAL_FONT *getNextFallbackFont (int *fallBackIndex)
{
    int count = *fallBackIndex;
    IFXP_FT_INTERNAL_FONT *font_handle = NULL;

    count = count < 0 ? 0 : count;
    *fallBackIndex = -1;
    for (; count < ARRAY_COUNT(fallback_font_handle_list); count++)
    {
        font_handle = (IFXP_FT_INTERNAL_FONT*) (fallback_font_handle_list[count].fallback_font_handle);
        if (font_handle)
        {
            *fallBackIndex = count + 1;
            return font_handle;
        }
    }
    return NULL;
}

#endif

/*************************************************************************
* FUNCTION
*
*       PL_Search_Font_Height
*
* DESCRIPTION
*
*       This function is used to search the height of font.
*
* INPUTS
*
*       ht_node_ptr     - Pointer to the font node.
*
*       req_height      - Font height value.
*
* OUTPUTS
*
*       ht_node_ptr     - Address of the font node.
*
*************************************************************************/
PL_FONT_SIZE_LIST *PL_Search_Font_Height(PL_FONT_SIZE_LIST *ht_node_ptr,
                                         unsigned int req_height)
{
    while(ht_node_ptr != NULL)
    {
        if ((unsigned int)ht_node_ptr->ft_char_height == req_height)
        {
            break;
        }
        else
        {
            ht_node_ptr = ht_node_ptr->ft_next_node;
        }
    }

    return  ht_node_ptr;
}

/************************************************************************
* FUNCTION
*
*       PL_Search_Char_in_Cache
*
* DESCRIPTION
*
*       Searches cached glyph node in the given list.
*
* INPUTS
*
*       cache_node_ptr                      Pointer to font size list node.
*
*       glyph_index                         Required character code.
*
* OUTPUTS
*
*       PL_FONT_GLYPH_CACHE*                Pointer to font glyph cache node.
*
*************************************************************************/
PL_FONT_GLYPH_CACHE *PL_Search_Char_in_Cache(PL_FONT_GLYPH_CACHE *cache_node_ptr,
                                             FT_UInt glyph_index)
{
    while (cache_node_ptr != NULL)
    {
        if (cache_node_ptr->ft_int_cache_glyph_index == glyph_index)
        {
            /* Increment the reference counter. */
            cache_node_ptr->ft_int_cache_ref_count++;
            break;
        }
        else
        {
            cache_node_ptr = cache_node_ptr->ft_next_cache_node;
        }
    }

    return  cache_node_ptr;
}

int compareFontFace(int a,int b)
{
    if(a==b || a==0)
        return 0;
    if((a==3 && a>b) || (a>0 && b==0) || (a==1 && b==2))
        return 1;
    return 0;
}


/*************************************************************************
*   FUNCTION
*
*       getGlyph
*
*   DESCRIPTION
*
*       This function searches the provided font cache for the glyph
*       matching the glyph index.  If it is not found in the cache, we
*       try to load it from the font and add it to the cache.
*
*   INPUTS
*
*      font             - font to use to find glyph.
*      glyph_index      - font table index for required glyph
*
*   OUTPUTS
*
*       Pointer to glyph object if found, NULL otherwise
*
*************************************************************************/
FT_BitmapGlyph getGlyph(IFXP_FT_INTERNAL_FONT* font, FT_UInt glyph_index)
{
    FT_BitmapGlyph glyph_bitmap = NULL;
    int glyph_not_found = 0;
    PL_FONT_GLYPH_CACHE* temp_cache_list_node = NULL;
    IFX_RETURN_STATUS status = IFX_SUCCESS;

    if (font == NULL)
        return NULL;

    // Check if glyph is in the cache already
    temp_cache_list_node = PL_Search_Char_in_Cache(font->ft_current_height_list_ptr->ft_cache_list_ptr, glyph_index);

    if (temp_cache_list_node == NULL)
    {
        /* Glyph not in cache, check font */

        /* Load glyph image into the slot. */
        if (FT_Load_Glyph(font->ft_face, glyph_index, FT_LOAD_RENDER))
        {
            // Glyph not found...
            glyph_not_found = 1;
        }

        if (glyph_not_found == 0)
        {
            /* Allocate new cache node. */
            status = IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,
                                        sizeof(PL_FONT_GLYPH_CACHE),
                                        (void**)&temp_cache_list_node);

            if (status == IFX_SUCCESS)
            {
                /* Clear allocated memory. */
                memset(temp_cache_list_node, 0, sizeof(PL_FONT_GLYPH_CACHE));

                /* Extract Glyph from the slot. */
                if (FT_Get_Glyph(font->ft_face->glyph, &temp_cache_list_node->ft_int_cache_glyph) == 0)
                {
                    glyph_bitmap = (FT_BitmapGlyph)temp_cache_list_node->ft_int_cache_glyph;

                    /* Add to the cache usage size. */
                    font->ft_used_cache_size += sizeof(PL_FONT_GLYPH_CACHE);

                    /* Set the node parameters. */
                    temp_cache_list_node->ft_int_cache_glyph_index = glyph_index;

                    /* Get the size of allocations and add to cache size counter. */
                    font->ft_used_cache_size += sizeof(FT_GlyphRec) + (glyph_bitmap->bitmap.rows * glyph_bitmap->bitmap.width);

                    /* Add cache node to the cache node list. */
                    if (font->ft_current_height_list_ptr->ft_cache_list_ptr == NULL)
                    {
                        temp_cache_list_node->ft_next_cache_node = NULL;
                        font->ft_current_height_list_ptr->ft_cache_list_ptr = temp_cache_list_node;
                    }
                    else
                    {
                        temp_cache_list_node->ft_next_cache_node = font->ft_current_height_list_ptr->ft_cache_list_ptr;
                        font->ft_current_height_list_ptr->ft_cache_list_ptr = temp_cache_list_node;
                    }
                }
                else
                {
                    IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, temp_cache_list_node);
                }
            }
        }
    }
    else
    {
        glyph_bitmap = (FT_BitmapGlyph)temp_cache_list_node->ft_int_cache_glyph;
    }

    return glyph_bitmap;
}

/************************************************************************
* FUNCTION
*
*       FONT_selectFace
*
* DESCRIPTION
*
*       Select the appropriate face if the font has multiple faces. If the selected
*       style is not found it selects the face in Regular, Bold, Italic, BoldItalic order.
*
* INPUTS
*
*       internal_font                      Internal font structure.
*
*       style                              font face style.
*
* OUTPUTS
*
*       None.
*
*************************************************************************/
void FONT_selectFace(IFXP_FT_INTERNAL_FONT *internal_font,
                                             IFXP_FONT_FACE face_param)
{
    int i,numberOfFaces;
    FT_Long styleFlagsRequired=0;
    int lowestFaceIndex=0;
    FT_Long lowestFaceFlags=0;
    if(internal_font==NULL || internal_font->ft_face==NULL)
        return;

    if(internal_font->ft_face->num_faces==1) //only one face
        return;

    numberOfFaces=internal_font->ft_face->num_faces;

    switch(face_param)
    {
        case IFXP_FONT_FACE_BOLD:
            styleFlagsRequired = FT_STYLE_FLAG_BOLD;
        break;
        case IFXP_FONT_FACE_ITALIC:
            styleFlagsRequired = FT_STYLE_FLAG_ITALIC;
        break;
        case IFXP_FONT_FACE_BOLDITALIC:
            styleFlagsRequired = FT_STYLE_FLAG_BOLD | FT_STYLE_FLAG_ITALIC;
        break;
        case IFXP_FONT_FACE_NORMAL:
            styleFlagsRequired=0;
    }

    if(internal_font->ft_face->style_flags == styleFlagsRequired)
    {
        return;
    }
    else
    {
        lowestFaceIndex=0;
        lowestFaceFlags=internal_font->ft_face->style_flags;
    }

    for(i=1;i<numberOfFaces;++i)
    {
         /* Opens the font from the given font buffer. */
        FT_New_Memory_Face(internal_font->ft_library,
                                   internal_font->ft_font_buffer,
                                   internal_font->ft_font_buffer_size,
                                   i,
                                   &internal_font->ft_face );

        if(internal_font->ft_face->style_flags == styleFlagsRequired)
        {
            return;
        }
        else
        {
            if(compareFontFace(internal_font->ft_face->style_flags,lowestFaceFlags)==1)
            {
                lowestFaceIndex=i;
                lowestFaceFlags=internal_font->ft_face->style_flags;
            }
        }
    }
    FT_New_Memory_Face(internal_font->ft_library,
                                   internal_font->ft_font_buffer,
                                   internal_font->ft_font_buffer_size,
                                   lowestFaceIndex,
                                   &internal_font->ft_face );
}

/************************************************************************
* FUNCTION
*
*       PL_Clean_Glyph_Cache
*
* DESCRIPTION
*
*       Cleans the internal glyph cache.
*
* INPUTS
*
*       font_handle                         Pointer to handle to be
*                                           populated with the opened font.
*
* OUTPUTS
*
*       None.
*
*************************************************************************/
void    PL_Clean_Glyph_Cache(IFXP_FONT font_handle)
{
    IFXP_FT_INTERNAL_FONT  *internal_font = (IFXP_FT_INTERNAL_FONT*)font_handle;
    PL_FONT_SIZE_LIST      *temp_ft_ht_node;
    PL_FONT_GLYPH_CACHE    *node_to_del;
    PL_FONT_GLYPH_CACHE    *prev_node = NULL;
    PL_FONT_GLYPH_CACHE    *curr_node = NULL;
    FT_BitmapGlyph          glyph_bitmap;

    /* Loop until the cache is below threshold. */
    while (internal_font->ft_used_cache_size > FT_MAX_CACHE_SIZE)
    {
        temp_ft_ht_node = internal_font->ft_font_height_list_ptr;

        /* Loop through the complete font height list. */
        while (temp_ft_ht_node != NULL)
        {
            prev_node = NULL;
            node_to_del = NULL;
            curr_node = temp_ft_ht_node->ft_cache_list_ptr;

            /* Loop through the complete glyph cache nodes list. */
            while (curr_node != NULL)
            {
                if (curr_node->ft_int_cache_ref_count == 0)
                {
                    node_to_del = curr_node;

                    /* Manage nodes. */

                    if (prev_node != NULL)
                    {
                        prev_node->ft_next_cache_node = curr_node->ft_next_cache_node;
                    }
                    if (curr_node == temp_ft_ht_node->ft_cache_list_ptr)
                        temp_ft_ht_node->ft_cache_list_ptr =
                        temp_ft_ht_node->ft_cache_list_ptr->ft_next_cache_node;

                    curr_node = curr_node->ft_next_cache_node;


                    /* Now delete the node. */

                    /* First, decrement the cache usage size. */

                    glyph_bitmap = (FT_BitmapGlyph)node_to_del->ft_int_cache_glyph;
                    internal_font->ft_used_cache_size -= sizeof(FT_GlyphRec) +
                               (glyph_bitmap->bitmap.rows * glyph_bitmap->bitmap.width);
                    internal_font->ft_used_cache_size -= sizeof(PL_FONT_GLYPH_CACHE);

                    FT_Done_Glyph(node_to_del->ft_int_cache_glyph);
                    IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, node_to_del);
                    node_to_del = NULL;
                }
                else
                {
                    /* Decrement the reference counter, so that the node is
                       cleared next time. */
                    curr_node->ft_int_cache_ref_count--;

                    /* Manage nodes. */

                    prev_node = curr_node;
                    curr_node = curr_node->ft_next_cache_node;
                }
            }

            /* Make sure that previous node's next is set to NULL. */
            if (prev_node != NULL)
                prev_node->ft_next_cache_node = NULL;

            temp_ft_ht_node = temp_ft_ht_node->ft_next_node;
        }
    }

    return;
}

/************************************************************************
*   FUNCTION
*
*       IFXP_Text_Open_Font
*
*   DESCRIPTION
*
*       Opens and sets up the font based on the supplied parameters
*
*   INPUTS
*
*       font_handle     - Pointer to handle to be populated with the opened font.
*
*       face_param      - The typeface of the font (see IFXP_FONT_FACE).
*
*       font_name       - The name of the requested font.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Text_Open_Font(IFXP_FONT *font_handle,
                                        IFXP_FONT_FACE face_param,
                                        const IFX_WCHAR *font_name)
{
    int error;
    IFXP_FT_INTERNAL_FONT *internal_font;
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;

    if (!font_name || lc_wcslen(font_name) > MAX_FONT_NAME_LENGTH)
       return IFX_ERROR;

    if (font_handle == NULL)
       return IFX_ERROR;

    ifxp_status = IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,
                                    sizeof(IFXP_FT_INTERNAL_FONT),
                                    (void**)&internal_font);

    if (ifxp_status != IFX_SUCCESS)
    {
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, internal_font);

        return IFX_ERROR;
    }

    /* Clear the allocated memory block. */
    memset(internal_font, 0 , sizeof(IFXP_FT_INTERNAL_FONT));

    /* Clear the height list related variables. */

    internal_font->ft_current_height_list_ptr = NULL;
    internal_font->ft_font_height_list_ptr = NULL;
    internal_font->ft_font_height_list_cnt = 0;

    /* Initialize the FreeType font engine library. */
    error = FT_Init_FreeType(&internal_font->ft_library);

    if (error)
    {
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, internal_font);

        return (IFX_ERROR);
    }

    if (IFX_SUCCESS == PL_Read_Font_Data(internal_font, font_name))
    {
 printf("%s %d %ls	IFX_SUCCESS\n",__func__,__LINE__,font_name);
        internal_font->ft_is_file_font = IFX_TRUE;
    }
    else
    {
 printf("%s %d %ls 	IFX_ERROR\n",__func__,__LINE__,font_name);
        /* Close FreeType font library handle. */
        FT_Done_FreeType(internal_font->ft_library);

        /* De allocate the memory consumed by font internal structure. */
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, internal_font);

        return (IFX_ERROR);
    }

    /* Opens the font from the given font buffer. */
    error = FT_New_Memory_Face(internal_font->ft_library,
                               internal_font->ft_font_buffer,
                               internal_font->ft_font_buffer_size,
                               0,
                               &internal_font->ft_face );

    if (error)
    {
        /* Close FreeType font library handle. */
        FT_Done_FreeType(internal_font->ft_library);

        /* De allocate the memory consumed by font internal structure. */
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, internal_font);

        return (IFX_ERROR);
    }

    FONT_selectFace(internal_font,face_param);

    /* Get FreeType font slot pointer, to be used very frequently.  */
    internal_font->ft_slot = internal_font->ft_face->glyph;

    internal_font->hb_font = hb_ft_font_create(internal_font->ft_face, 0);
    internal_font->hb_unicode_funcs = hb_ucdn_make_unicode_funcs();

    *font_handle = internal_font;

    return (IFX_SUCCESS);
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_Text_Close_Font
*
*   DESCRIPTION
*
*       Closes the font.
*
*   INPUTS
*
*       font_handle     - Font handle.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Text_Close_Font(IFXP_FONT font_handle)
{
    IFXP_FT_INTERNAL_FONT   *internal_font = (IFXP_FT_INTERNAL_FONT*) font_handle;
    PL_FONT_SIZE_LIST       *temp_ft_ht_node;
    PL_FONT_GLYPH_CACHE     *temp_cache_list_node;

    if (internal_font)
    {
        if (internal_font->current_text_runs != NULL)
        {
            destroyTextRuns (internal_font->current_text_runs);
            internal_font->current_text_runs = NULL;
        }

        if ((internal_font->ft_font_buffer != NULL) &&
            (internal_font->ft_is_file_font == IFX_TRUE))
        {
            IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, internal_font->ft_font_buffer);
        }

        while (internal_font->ft_font_height_list_ptr)
        {
            temp_ft_ht_node = internal_font->ft_font_height_list_ptr;

            while (temp_ft_ht_node->ft_cache_list_ptr)
            {
                temp_cache_list_node = temp_ft_ht_node->ft_cache_list_ptr;
                temp_ft_ht_node->ft_cache_list_ptr = temp_cache_list_node->ft_next_cache_node;
                FT_Done_Glyph(temp_cache_list_node->ft_int_cache_glyph);
                IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, temp_cache_list_node);
            }

            internal_font->ft_font_height_list_ptr = temp_ft_ht_node->ft_next_node;
            IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, temp_ft_ht_node);
        }

        hb_unicode_funcs_destroy(internal_font->hb_unicode_funcs);
        hb_font_destroy(internal_font->hb_font);

        /* Close font face handle. */
        FT_Done_Face( internal_font->ft_face );

        /* Close FreeType font library handle. */
        FT_Done_FreeType(internal_font->ft_library);

        /* De allocate the memory consumed by font internal structure. */
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, internal_font);
    }

    return IFX_SUCCESS;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_Text_Get_Width
*
*   DESCRIPTION
*
*       Returns the width of the text in pixels.
*
*   INPUTS
*
*       font_handle     - Font Handle.
*
*       text            - Text to calculate width with.
*
*       in_length       - Length of the text string (in chars) to use for
*                         calculation.
*
*       out_length      - Pointer to variable to hold the width in pixels
*                         of the text string.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Text_Get_Width(IFXP_FONT font_handle,
                                        IFX_WCHAR *text,
                                        IFX_UINT32 in_length,
                                        IFX_UINT32 *out_length,
                                        IFX_INT32 *rtlOut)
{
    IFXP_FT_INTERNAL_FONT  *internal_font = (IFXP_FT_INTERNAL_FONT*)font_handle;
    IFXP_FT_INTERNAL_FONT  *current_font;
    IFXP_FT_INTERNAL_FONT  *best_font;
    unsigned int            numChars = lc_wcslen(text);
    unsigned int            count;
    unsigned int            height = internal_font->ft_current_height_list_ptr->ft_char_height;
    unsigned int            pen_x = 0;
    unsigned int            cached_width = 0;
             int            leftEdge = 0;
             int            rightEdge = 0;
             int            bitmapRightEdge = 0;
             int            bitmapLeftEdge = 0;
             int            fallbackFontIndex = -1;
    FT_UInt                 glyph_index;
    int                     error = 0;
    FT_BitmapGlyph          glyph_bitmap;
    FT_UInt                 glyph_advance = 0;

    if (out_length && internal_font)
    {
        *out_length = 0;

        if (!internal_font->hb_font)
            return IFX_ERROR;

        if (internal_font->current_text_runs != NULL)
        {
            destroyTextRuns (internal_font->current_text_runs);
            internal_font->current_text_runs = NULL;
        }

#ifdef IFXP_FALLBACK_FONT_SUPPORT
        // set height of the fallbackfonts
        setFallbackFontsHeight(height);
#endif

        if (numChars > 0)
        {
            hb_buffer_t *buff = hb_buffer_create();

            FriBidiChar *src_str = (FriBidiChar *)text;
            FriBidiChar *visual_str = NULL;
            int         *position_L_to_V_list = NULL;
            int         *position_V_to_L_list = NULL;
            int8_t      *embedding_level_list = NULL;
            FriBidiCharType base_type = FRIBIDI_TYPE_ON;
            int base_direction = 0;
            int index = 0;
            IFX_WCHAR prev_char;
            int prev_index = -1;
            int len_substr = 0;
            unsigned int buff_len = 0;
            int current_run_direction = 0;
            IFX_WCHAR *sub_text = NULL;
            int first_char = 1;
            int errorCount = 0;
            hb_glyph_info_t *glyphInfos = NULL;
            hb_glyph_position_t *glyphPos = NULL;

            if (src_str == NULL)
                return IFX_ERROR;

            IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL, numChars * sizeof(int8_t),
                                    (void**)&embedding_level_list);

            if (embedding_level_list == NULL)
            {
                IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, src_str);
                return IFX_ERROR;
            }

            switch (*rtlOut)
            {
            case 0:
                base_type = FRIBIDI_TYPE_L;
            break;

            case 1:
                base_type = FRIBIDI_TYPE_R;
            }

            fribidi_log2vis(/* input */
                    src_str,
                    numChars,
                    &base_type,
                    /* output */
                    visual_str,
                    position_L_to_V_list,
                    position_V_to_L_list,
                    (FriBidiLevel*)embedding_level_list);


            *rtlOut = 0;
            if (base_type == FRIBIDI_TYPE_R)
            {
                internal_font->rtlOut = base_direction = 1;
                index = numChars - 1;
                *rtlOut = 1;
            }

            current_font = internal_font;
            best_font = NULL;

            while(error != 0 || (index >= 0 && index < (int)numChars))
            {
                if (!error)
                {
                    sub_text = getNextRun(text,
                                       &index,
                                       &prev_index,
                                       &prev_char,
                                       &len_substr,
                                       numChars,
                                       base_direction,
                                       &current_run_direction,
                                       embedding_level_list);

                    current_font = internal_font;
                }

                if (sub_text == NULL)
                {
                    error = 0;
                    continue;
                }

                if (buff == NULL)
                    buff = hb_buffer_create();

                hb_buffer_reset(buff);
                hb_buffer_set_unicode_funcs(buff, current_font->hb_unicode_funcs);
                hb_buffer_set_direction(buff, (current_run_direction == 0)? HB_DIRECTION_LTR:HB_DIRECTION_RTL);

                hb_buffer_add_utf32(buff, sub_text,-1 , 0, -1);

                // Do the glyph substitution, or our calculated buffer size will not match with the
                // buffer size actually used.  We don't need to do the bi-directional check here,
                // as that won't affect overall length
                hb_shape(current_font->hb_font, buff, NULL, 0);

                buff_len = hb_buffer_get_length(buff);
                glyphInfos = hb_buffer_get_glyph_infos(buff, &buff_len);
                glyphPos = hb_buffer_get_glyph_positions(buff, &buff_len);

#ifdef IFXP_FALLBACK_FONT_SUPPORT
                // quickly check the glyph index
                count = 0;
                error = 0;
                for (;count < buff_len && fallbackFontIndex != 999; ++count)
                    error += (glyphInfos[count].codepoint == 0 && IFX_SCRIPT_COMMON(sub_text, glyphInfos[count].cluster) == 0) ? 1 : 0;

                if (error != 0 && fallbackFontIndex != 999)
                {
                    IFXP_FT_INTERNAL_FONT *tempFont = getNextFallbackFont (&fallbackFontIndex);

                    if (best_font == NULL
                       || errorCount > error)
                    {
                        best_font = current_font;
                        errorCount = error;
                    }
                    if (tempFont != NULL)
                    {
                        current_font = tempFont;
                        continue;
                    }
                    else if (current_font != best_font)
                    {
                        current_font = best_font;
                        fallbackFontIndex = 999;
                        continue;
                    }
                }
#endif
                /* Calculate the width of the string character by character. */
                for (count = 0;count < buff_len; ++count)
                {
                    glyph_index = glyphInfos[count].codepoint;

                    // check for the '0 width' bit - if found, we don't draw this glyph
                    if ((glyphInfos[count].var1.u8[0] & 0x80) != 0)
                        continue;

                    if (glyph_index == 0)
                    {
#ifdef IFXP_FALLBACK_FONT_SUPPORT
                        IFX_INT32 baseline;
                        IFXP_FT_INTERNAL_FONT *temp_font;
                        IFX_WCHAR missing_char[2] = {0};
                        missing_char[0] = sub_text[glyphInfos[count].cluster];

                        temp_font = checkFallbackFonts(missing_char,
                                           &glyph_index,
                                           &glyph_advance,
                                           hb_buffer_get_direction(buff),
                                           &baseline);

                        glyph_bitmap = getGlyph(temp_font, glyph_index);
#else
                        glyph_bitmap = NULL;
#endif
                    }
                    else
                    {
                        // Look up the glyph bitmap from the font cache (will add the bitmap to the font
                        // cache if not already there).  Will return 'NULL' if the glyph can't be found
                        glyph_bitmap = getGlyph(current_font, glyph_index);
                    }

                    if (!glyph_bitmap)
                        continue;

                    glyph_advance = (glyph_bitmap->root.advance.x / 1024);

                    // For each character, we need to work out where the pen is, then check where the glyph bitmap will be,
                    // taking into account any horizontal bearing.  Note that pen coordinates are in 1/64th of a pixel
                    bitmapLeftEdge = (pen_x >> 6) + glyph_bitmap->left + (glyphPos[count].x_offset >> 6);
                    bitmapRightEdge = bitmapLeftEdge + glyph_bitmap->bitmap.width;

                    // cache the width of NSM characters
                    if (glyph_advance == 0 && cached_width < (unsigned int)glyph_bitmap->bitmap.width)
                        cached_width = glyph_bitmap->bitmap.width;
                    else if ((unsigned int)glyph_bitmap->bitmap.width < cached_width)
                    {
                        bitmapRightEdge = bitmapLeftEdge + cached_width;
                    }
                    else
                    {
                        cached_width = 0;
                    }

                    // To avoid issues where the last glyph is right up against the right edge of the buffer,
                    // or where the glyph width is 0 (like in the case of the space character), use the advance
                    // width if that is greater than the 'left bearing + glyph width'
                    if (bitmapRightEdge < (int)(pen_x + glyph_advance)>> 6)
                        bitmapRightEdge = (pen_x + glyph_advance)>> 6;

                    // Keep track of the overall string bounding box edges and update as required
                    if (bitmapLeftEdge < leftEdge)
                        leftEdge = bitmapLeftEdge;

                    // Only update right edge if it is not an nsm character
                    if ((glyph_advance != 0) && bitmapRightEdge > rightEdge)
                        rightEdge = bitmapRightEdge;

                    // Now update the pen position using the glyph advance
                    pen_x += glyph_advance;
                }
                error = 0;
                fallbackFontIndex = -1;
                errorCount = 0;
                best_font = NULL;

                if (buff)
                {
                    IFXP_FT_INTERNAL_TEXT_RUN *run = NULL;

                    if (IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL, sizeof (IFXP_FT_INTERNAL_TEXT_RUN), (void **)&run) == IFX_SUCCESS)
                    {
                        run->buffer = buff;
                        run->fallbackFont = current_font;
                        run->nextRun = NULL;
                        run->sub_text_start = sub_text - text;

                        cacheRuns(internal_font, run);
                        buff = NULL;
                    }
                }
            }

            if (prev_index != -1)
            {
                text[prev_index] = prev_char;
            }

            // Finally, the text width is the difference between bounding box edges
            *out_length = rightEdge - leftEdge;
            IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, embedding_level_list);

            if (buff != NULL)
                hb_buffer_destroy(buff);
        }
    }
    else
    {
        return IFX_ERROR;
    }

    return IFX_SUCCESS;
}

static void destroyTextRuns(IFXP_FT_INTERNAL_TEXT_RUN *textRun)
{
    IFXP_FT_INTERNAL_TEXT_RUN *currentRun = textRun;
    textRun->prevRun->nextRun = NULL;

    while (currentRun != NULL)
    {
        currentRun = currentRun->nextRun;

        hb_buffer_destroy(textRun->buffer);
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, textRun);
        textRun = currentRun;
    }
}

static void cacheRuns(IFXP_FT_INTERNAL_FONT *internal_font, IFXP_FT_INTERNAL_TEXT_RUN *text_run)
{
    if (internal_font->current_text_runs == NULL)
    {
        internal_font->current_text_runs = text_run;
        internal_font->current_text_runs->nextRun = text_run;
        internal_font->current_text_runs->prevRun = text_run;
    }
    else
    {
        text_run->nextRun = internal_font->current_text_runs;
        text_run->prevRun = internal_font->current_text_runs->prevRun;
        text_run->prevRun->nextRun = text_run;
        internal_font->current_text_runs->prevRun = text_run;
    }
}

/*************************************************************************
*   FUNCTION
*
*        IFXP_Text_Get_Char_Height
*
*   DESCRIPTION
*
*        Returns the height of a character in pixels.
*
*   INPUTS
*
*        font_handle        - Font Handle.
*
*        height             - Pointer to variable to be populated with the
*                             character height.
*
*   OUTPUTS
*
*        IFX_SUCCESS        - On success.
*
*        IFX_ERROR          - On Failure.
*
*************************************************************************/
IFX_RETURN_STATUS IFXP_Text_Get_Char_Height (IFXP_FONT font_handle,
                                             IFX_UINT32 *height)
{
    IFXP_FT_INTERNAL_FONT  *internal_font = (IFXP_FT_INTERNAL_FONT*)font_handle;
    IFX_RETURN_STATUS   ifxp_status = IFX_SUCCESS;
    IFX_UINT32          temp_font_height;
    int                 error = 0;
    int                 max_tries = PL_FT_MAX_TRIES_HEIGHT;
    int                 is_search_required = 0;
    unsigned char       going_down=0;
    float               scaleFactor = 0;
    unsigned int        calc_height = 0;

    PL_FONT_SIZE_LIST  *temp_ft_ht_node;

    if ((internal_font == NULL) || (height == NULL))
    {
        return IFX_ERROR;
    }

    if (internal_font->ft_font_height_list_ptr == NULL)
    {
        ifxp_status = IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,
                                        sizeof(PL_FONT_SIZE_LIST),
                                        (void**)&temp_ft_ht_node);

        if (temp_ft_ht_node == NULL)
        {
            return IFX_ERROR;
        }

        /* Clear allocated memory. */
        memset(temp_ft_ht_node, 0, sizeof(PL_FONT_SIZE_LIST));

        /* Set fields to NULL. */

        temp_ft_ht_node->ft_next_node = NULL;
        temp_ft_ht_node->ft_cache_list_ptr = NULL;


        internal_font->ft_font_height_list_cnt++;
        internal_font->ft_font_height_list_ptr = temp_ft_ht_node;

        is_search_required = 1;
    }
    else
    {
        /* Search for node with given font size. */
        temp_ft_ht_node = PL_Search_Font_Height(internal_font->ft_font_height_list_ptr, *height);
        if (temp_ft_ht_node == NULL)
        {
            /* Here we need to allocate the node. */
            ifxp_status = IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,
                                            sizeof(PL_FONT_SIZE_LIST),
                                            (void**)&temp_ft_ht_node);

            if (ifxp_status != IFX_SUCCESS)
            {
                return IFX_ERROR;
            }

            /* Clear allocated memory. */
            memset(temp_ft_ht_node, 0, sizeof(PL_FONT_SIZE_LIST));

            internal_font->ft_font_height_list_cnt++;

            /* Add the newly created node to chain. */
            temp_ft_ht_node->ft_cache_list_ptr = NULL;
            temp_ft_ht_node->ft_next_node = internal_font->ft_font_height_list_ptr;
            internal_font->ft_font_height_list_ptr = temp_ft_ht_node;
            is_search_required = 1;
        }
        else
        {
            /* Nothing to do. Just use the temp_ft_ht_node to look for the required parameters. */

            is_search_required = 0;
        }
    }

    if (is_search_required == 1)
    {
        /* Its a newly created node and requires search for suitable height.*/

        temp_font_height = *height;

        /* scale factor is the ration of the bbox height and the EMBox, we use this to calculate the
           the bbox of the current size. */
        scaleFactor = (internal_font->ft_face->bbox.yMax - internal_font->ft_face->bbox.yMin)
                                                    / ((float)(*(internal_font->ft_face)).units_per_EM);
        /* Set the pixel sizes for the given font. */
        error = FT_Set_Char_Size(internal_font->ft_face,
                                   temp_font_height *64,
                                   temp_font_height*64,
                                   0,
                                   0);

        /* calculate the actual height using the scale factor and the pixel per EM,
           the current size height in font units */
        calc_height = (int)(scaleFactor * (internal_font->ft_face->size->metrics).y_ppem + 1.5f);
        if (*height != calc_height)
        {
            /* Guess the height that will fit */
            temp_font_height = (int)((temp_font_height / scaleFactor) + 1.5);

            error = FT_Set_Char_Size(internal_font->ft_face,
                                   temp_font_height*64,
                                   temp_font_height*64,
                                   0,
                                   0);

            while ((max_tries > 0) && (error == 0))
            {
                calc_height = (int)(scaleFactor * (internal_font->ft_face->size->metrics).y_ppem + 1.5);

                if (*height == calc_height)
                {
                    /* Do Nothing. */
                    break;

                }
                else if (*height < calc_height)
                {
                    temp_font_height = temp_font_height - 1;

                    /* Set the pixel sizes for the given font. */
                    error = FT_Set_Char_Size(internal_font->ft_face,
                                   temp_font_height*64,
                                   temp_font_height*64,
                                   0,
                                   0);
                    going_down = 1;
                }
                else
                {
                    if (going_down == 1)
                    {
                        /* Break the loop. As we need to fit the font in the given size. */
                        break;
                    }

                    temp_font_height = temp_font_height + 1;

                    /* Set the pixel sizes for the given font. */
                    error = FT_Set_Char_Size(internal_font->ft_face,
                                   temp_font_height*64,
                                   temp_font_height*64,
                                   0,
                                   0);
                }

                max_tries--;
            }

            /* Check to see if there are any errors. */
            if (error)
            {
                /* Close font face handle. */
                FT_Done_Face( internal_font->ft_face );


                /* Close FreeType font library handle. */
                FT_Done_FreeType(internal_font->ft_library);

                /* De allocate the memory consumed by font internal structure. */
                IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, internal_font);

                return (IFX_ERROR);
            }
        }

        internal_font->ft_font_height_list_ptr->ft_char_height = *height;
        internal_font->ft_font_height_list_ptr->ft_char_width = *height;
        internal_font->ft_font_height_list_ptr->ft_internal_char_height = temp_font_height;
        internal_font->ft_font_height_list_ptr->ft_internal_char_width = temp_font_height;

        /* Calculate the ratio of the bbox y minimum to the EMBox height, use this ratio to calculate
           the baseline. */
        internal_font->ft_font_height_list_ptr->ft_baseline = (int)(*height + (internal_font->ft_face->bbox.yMin
                                                                * (internal_font->ft_face->size->metrics).y_ppem)/((float)(*(internal_font->ft_face)).units_per_EM));

        /* Set the current pointer. This will enable getting the font height at
            canvas creation time. */
        internal_font->ft_current_height_list_ptr = internal_font->ft_font_height_list_ptr;

    }
    else
    {
        /* Set the pixel sizes for the given font. */
        error = FT_Set_Char_Size(internal_font->ft_face,
                                   temp_ft_ht_node->ft_internal_char_height*64,
                                   temp_ft_ht_node->ft_internal_char_height*64,
                                   0,
                                   0);

        /* Check to see if there are any errors. */
        if (error)
        {
            /* Close font face handle. */
            FT_Done_Face( internal_font->ft_face );


            /* Close FreeType font library handle. */
            FT_Done_FreeType(internal_font->ft_library);

            /* De allocate the memory consumed by font internal structure. */
            IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, internal_font);

            return (IFX_ERROR);
        }
    }

    /* Set the current pointer. This will enable getting the font height at
        canvas creation time. */
        internal_font->ft_current_height_list_ptr = temp_ft_ht_node;

        hb_font_set_scale (internal_font->hb_font,
             (int)((uint64_t) internal_font->ft_face->size->metrics.x_scale * (uint64_t) internal_font->ft_face->units_per_EM) >> 16,
             (int)((uint64_t) internal_font->ft_face->size->metrics.y_scale * (uint64_t) internal_font->ft_face->units_per_EM) >> 16);

        hb_font_set_ppem (internal_font->hb_font,
            internal_font->ft_face->size->metrics.x_ppem,
            internal_font->ft_face->size->metrics.y_ppem);

    return (IFX_SUCCESS);
}

/*************************************************************************
* FUNCTION
*
*       memcpyWithAlpha
*
* DESCRIPTION
*
*       Helper function for text rendering.
*
* INPUTS
*
*       dst      - Pointer destination buffer.
*
*       src      - Pointer to source buffer.
*
*       size     - number of bytes to copy
*
* OUTPUTS
*
*       ht_node_ptr     - Address of the font node.
*
*************************************************************************/
static void memcpyWithAlpha(unsigned char * dst, unsigned char * src, int size)
{
    unsigned char* endDst = dst + size;
    while (dst < endDst)
    {
        if(*dst <= *src)
            *dst = *src;
        ++dst;
        ++src;
    }
}

/*************************************************************************
*************************************************************************/
static IFX_WCHAR * getNextRun(IFX_WCHAR *text,
                                    int *index,
                                    int *prev_index,
                                    IFX_WCHAR *prev_char,
                                    int *len_substr,
                                    int num_chars,
                                    int base_direction,
                                    int *current_direction_ptr,
                                    int8_t embedding_level_list[])
{
    IFX_WCHAR * sub_text = NULL;
    IFX_WCHAR pchar = *prev_char;
    int i = *index;
    int pindex = *prev_index;
    int current_direction = embedding_level_list[i] % 2;
    hb_script_t current_script;

    if (pindex != -1)
    {
        text[pindex] = pchar;
    }

    *current_direction_ptr = current_direction;
    current_script = HB_SCRIPT_UNICODE(text, i);

    if (base_direction == 0)
    {
        for (;i < num_chars;++i)
        {
            if ((embedding_level_list[i] % 2) != current_direction
                    || HB_SCRIPT_UNICODE_CMP(text, i, &current_script) != 0)
            {
                *len_substr = i - *index;
                sub_text = &text[*index];
                *index = i;

                if (i < num_chars)
                {
                    pchar = text[i];
                    text[i] = 0;
                    pindex = i;
                }
                break;
            }
        }

        if (i == num_chars
                && (embedding_level_list[i - 1] % 2) == current_direction
                && HB_SCRIPT_UNICODE_CMP(text, i - 1, &current_script) == 0)
        {
            *len_substr = i - *index;
            sub_text = &text[*index];
            *index = i;
            --i;
        }
    }
    else
    {
        for (;i >=0 ;--i)
        {
            if ((embedding_level_list[i] % 2) != current_direction
                    || HB_SCRIPT_UNICODE_CMP(text, i, &current_script) != 0)
            {
                int i0 = *index + 1;
                *len_substr = *index - i;
                *index = i;
                ++i;

                sub_text = &text[i];

                if (i0 < num_chars)
                {
                    pchar = text[i0];
                    text[i0] = 0;
                    pindex = i0;
                }
                break;
            }
        }

        if (i == -1
                && (embedding_level_list[i + 1] % 2) == current_direction
                && HB_SCRIPT_UNICODE_CMP(text, i + 1, &current_script) == 0)
        {
            int i0 = *index + 1;
            *len_substr = *index + 1;
            *index = i;
            ++i;

            sub_text = &text[i];

            if (i0 < num_chars)
            {
                pchar = text[i0];
                text[i0] = 0;
                pindex = i0;
            }
        }
    }

    *prev_char = pchar;
    *prev_index = pindex;
    return sub_text;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_Text_Canvas_Create
*
*   DESCRIPTION
*
*       Creates an 8-bit gray scale buffer and renders the specified text
*       to it using the face and alignment provided.
*
*   INPUTS
*
*        text_canvas        - Pointer to text_canvas pointer to store
*                             created canvas.
*
*        font_handle        - Font Handle.
*
*        text               - Text to be rendered.
*
*   OUTPUTS
*
*       IFX_SUCCESS         - On success.
*       IFX_ERROR           - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Text_Canvas_Create(IFXP_TEXT** text_canvas,
                                            IFXP_FONT font_handle,
                                            IFX_WCHAR *text,
                                            IFX_INT32 *rtlOut)
{
    IFXP_FT_INTERNAL_FONT  *internal_font = (IFXP_FT_INTERNAL_FONT*)font_handle;
    IFXP_TEXT              *local_text_canvas;
    IFX_UINT32              count;
    IFX_INT32               baseLine = 0;
    IFX_UINT32              width = 0, pen_x = 0;
    unsigned char          *dst_ptr, *src_ptr;
    FT_UInt                 glyph_index;
    FT_BitmapGlyph          glyph_bitmap;
    FT_UInt                 glyph_advance = 0;
    FT_Bitmap*              source_bitmap;
    IFXP_FT_INTERNAL_FONT  *current_font;

    /* Check the validity of internal font structure. */
    if (!internal_font)
    {
        return (IFX_ERROR);
    }

    if (IFX_SUCCESS != IFXP_Text_Get_Width(font_handle, text, lc_wcslen(text), &width, rtlOut))
    {
        return (IFX_ERROR);
    }

    /* Allocate memory for the text Canvas. */
    if (IFX_SUCCESS != IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,
                                         sizeof(IFXP_TEXT),
                                         (void**)&local_text_canvas))
    {
        return (IFX_ERROR);
    }

    local_text_canvas->internal = NULL;

    /* In the zero-length text case, make sure there is room for the caret */
    if (width == 0)
    {
        width = 1;
    }

    /* Make a copy of the width and height values. */
    local_text_canvas->width = width;
    local_text_canvas->height = internal_font->ft_current_height_list_ptr->ft_char_height;
    baseLine = internal_font->ft_current_height_list_ptr->ft_baseline;

    /* Allocate memory for the alpha buffer. */
    if (IFX_SUCCESS != IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,
                                         (width * internal_font->ft_current_height_list_ptr->ft_char_height),
                                         (void**)&(local_text_canvas->alphaBuffer)))
    {
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, local_text_canvas);

        return (IFX_ERROR);
    }

    /* Clear the newly allocated alpha buffer. */
    memset(local_text_canvas->alphaBuffer, 0, width*internal_font->ft_current_height_list_ptr->ft_char_height);

    /* Render and copy each char out of the glyph buffer. */

    if (internal_font->current_text_runs != NULL)
    {
        hb_buffer_t *buff = NULL;
        unsigned int buff_len = 0;
        IFXP_FT_INTERNAL_TEXT_RUN *current_run = internal_font->current_text_runs;
        int first_char = 1;
        current_run->prevRun->nextRun = NULL;
        current_font = internal_font;

        while(current_run != NULL)
        {
            buff = current_run->buffer;

#ifdef IFXP_FALLBACK_FONT_SUPPORT
            current_font = current_run->fallbackFont;
#endif
            if (buff != NULL
                    && current_font != NULL)
            {
                hb_glyph_info_t *glyphInfos = NULL;
                hb_glyph_position_t *glyphPos = NULL;

                buff_len = hb_buffer_get_length(buff);
                glyphInfos = hb_buffer_get_glyph_infos(buff, &buff_len);
                glyphPos = hb_buffer_get_glyph_positions(buff, &buff_len);

                count = 0;
                for (;count < buff_len; ++count)
                {
                    int src_y;
                    unsigned int dst_y;

                    glyph_index = glyphInfos[count].codepoint;

                    // check for the '0 width' bit - if found, we don't draw this glyph
                    if ((glyphInfos[count].var1.u8[0] & 0x80) != 0)
                        continue;

                    if (glyph_index == 0)
                    {
#ifdef IFXP_FALLBACK_FONT_SUPPORT
                        IFXP_FT_INTERNAL_FONT *temp_font;
                        IFX_WCHAR missing_char[2] = {0};
                        missing_char[0] = text[glyphInfos[count].cluster + current_run->sub_text_start];

                        temp_font = checkFallbackFonts(missing_char,
                                          &glyph_index,
                                          &glyph_advance,
                                          hb_buffer_get_direction(buff),
                                          &baseLine);

                        glyph_bitmap = getGlyph(temp_font, glyph_index);
#else
                        glyph_bitmap = NULL;
#endif
                    }
                    else
                    {
                         // Look up the glyph bitmap from the font cache (will add the bitmap to the font
                         // cache if not already there).  Will return 'NULL' if the glyph can't be found
                         glyph_bitmap = getGlyph(current_font, glyph_index);
                    }

                    if (!glyph_bitmap)
                         continue;

                    glyph_advance = (glyph_bitmap->root.advance.x / 1024);

                    /* Extract bitmap. */
                    source_bitmap = &glyph_bitmap->bitmap;

                    /* Now copy the bitmap to Inflexion UI buffer. */

                    src_ptr = source_bitmap->buffer;

                    if ((first_char == 1) && ((glyph_bitmap->left + (glyphPos[count].x_offset >> 6)) < 0))
                    {
                        // First character has a negative X bearing, so buffer start
                        // is not at string 'origin'.
                        // Move pen inside buffer to 'origin'...
                        pen_x = (-glyph_bitmap->left - (glyphPos[count].x_offset >> 6))*64;
                    }

                    // Find offset into first raster given current pen value
                    dst_ptr = (unsigned char *)local_text_canvas->alphaBuffer + (pen_x >> 6);

                    // Now we need to move to the correct raster for top left corner of glyph
                    // if there was no X bearing

                    baseLine = current_font->ft_current_height_list_ptr->ft_baseline;
                    if(baseLine < glyph_bitmap->top)
                    {
                        dst_y = 0;
                    }
                    else
                    {
                        dst_y = baseLine - glyph_bitmap->top;
                    }

                    if ((glyphPos[count].y_offset < 0) || ((unsigned int)(glyphPos[count].y_offset / 64) <= dst_y))
                    {
                        dst_y = dst_y - (glyphPos[count].y_offset >> 6);
                    }
                    else
                    {
                        dst_y = 0;
                    }

                    dst_ptr +=  dst_y * local_text_canvas->width + (glyphPos[count].x_offset >> 6);

                    // Now apply X bearing to find where glyph starts
                    dst_ptr = dst_ptr + glyph_bitmap->left;

                    // Copy glyph into buffer...
                    for (src_y=0;
                         src_y < source_bitmap->rows && dst_y < local_text_canvas->height;
                         src_y++, dst_y++)
                    {
                        memcpyWithAlpha(dst_ptr, src_ptr, source_bitmap->width);
                        src_ptr += (source_bitmap->width);
                        dst_ptr += local_text_canvas->width;
                    }

                    /* Increment pen position by glyph advance value*/
                    pen_x += glyph_advance;

                    first_char = 0;
                }
            }
            current_run = current_run->nextRun;
        }
        destroyTextRuns(internal_font->current_text_runs);
        internal_font->current_text_runs = NULL;
    }

    *text_canvas = local_text_canvas;

    /* In case the cache size is exceeding limit, then clean cache
        before processing input string. */
    if (internal_font->ft_used_cache_size > FT_MAX_CACHE_SIZE)
        PL_Clean_Glyph_Cache(font_handle);

    return IFX_SUCCESS;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_Text_Canvas_Destroy
*
*   DESCRIPTION
*
*       Destroys a previously created text canvas.
*
*   INPUTS
*
*       canvas          - Text canvas created with IFXP_Text_Canvas_Create.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Text_Canvas_Destroy(IFXP_TEXT *canvas)
{
    if (canvas)
    {
        if (canvas->alphaBuffer)
        {
            IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, canvas->alphaBuffer);
        }

        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, (void*)canvas);

        return (IFX_SUCCESS);
    }

    return (IFX_ERROR);
}

/*************************************************************************
*   FUNCTION
*
*       PL_Get_Font_File_Path
*
*   DESCRIPTION
*
*       Extracts the font file name and path against the given font name.
*
*   INPUTS
*
*       file_path       - Pointer to string containing file path.
*
*       handle          - Pointer to return the opened file buffer.
*
*       font_name       - Pointer to string containing file name.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
static IFX_RETURN_STATUS   PL_Get_Font_File_Path(char *file_path,
                                                 IFXE_FILE *handle,
                                                 const IFX_WCHAR *font_name)
{
    char                path[3] = {0};
    char                pkgDriveList[] = IFX_PACKAGE_DRIVES;
    char               *drivePtr;
    IFX_RETURN_STATUS   ifxp_status = IFX_ERROR;
    char                mbFontName[MAX_FONT_NAME_LENGTH];
    IFXE_FILE           localHandle = NULL;
    IFX_WCHAR * ignoreState=NULL;
    IFX_WCHAR * userPathFound=NULL;
    IFX_WCHAR * delimeter=L"/";
    IFX_WCHAR originalFileName[MAX_FONT_NAME_LENGTH];
    IFX_WCHAR * onlyFileName;
    int i=0;
    int useSystemPath=1;

    /* Create a multi-byte version of the name. */
    lc_wcscpy(originalFileName,font_name);
    lc_wcstombs(mbFontName, font_name, MAX_FONT_NAME_LENGTH);
    onlyFileName=originalFileName;

    if (mbFontName[0] != '\0' && mbFontName[1] == ':')
    {
        /* Full path with drive given - try opening it */
        if (IFX_SUCCESS == IFXE_File_Open(&localHandle, mbFontName))
        {
            ifxp_status = IFX_SUCCESS;

            lc_strcpy(file_path, mbFontName);

            if (handle)
                *handle = localHandle;
            else
                IFXE_File_Close(localHandle);
        }

    }

    if (ifxp_status != IFX_SUCCESS)
    {
        userPathFound= lc_wcstok(originalFileName,delimeter,&ignoreState);
        userPathFound= lc_wcstok(NULL,delimeter,&ignoreState);
        if(userPathFound==NULL)
        {
            lc_wcscpy(originalFileName,font_name);
            userPathFound= lc_wcstok(originalFileName,L"\\",&ignoreState);
            userPathFound= lc_wcstok(NULL,L"\\",&ignoreState);
            if(userPathFound!=NULL)
                delimeter=L"\\";
        }
        if(userPathFound!=NULL)
            useSystemPath=0;

        lc_wcscpy(originalFileName,font_name);

        for(i=0;i<2;++i)
        {
            switch(useSystemPath)
            {
                case 0:
                {
                /* Cycle through each of the drives in IFX_PACKAGE_DRIVES until the font is
                   found.
                */
                    for (drivePtr = pkgDriveList; strlen(drivePtr) && (ifxp_status == IFX_ERROR);
                         (drivePtr[1] == ',') ? drivePtr += 2 : drivePtr++ )
                    {
                        if (*drivePtr >= 'A' && *drivePtr <= 'Z')
                        {
                            path[0] = *drivePtr;
                            path[1] = ':';
                            path[2] = 0;

                            /* Try the font with the various extensions until it is found.
                               If it is not found then use the default. */
                            lc_sprintf(file_path, "%s%s%s", path, IFX_PLAT_DIR_SEP, mbFontName);

                            if (IFX_SUCCESS == IFXE_File_Open(&localHandle, file_path))
                            {
                                ifxp_status = IFX_SUCCESS;
                                if (handle)
                                    *handle = localHandle;
                                else
                                    IFXE_File_Close(localHandle);
                            }
                        }
                    }
                    if(ifxp_status==IFX_ERROR)
                    {
                        lc_wcscpy(originalFileName,font_name);
                        userPathFound=originalFileName;
                        userPathFound= lc_wcstok(userPathFound,delimeter,&ignoreState);
                        while(userPathFound!=NULL)
                        {
                            onlyFileName=userPathFound;
                            userPathFound= lc_wcstok(NULL,delimeter,&ignoreState);
                        }
                        useSystemPath=1;
                    }
                    else
                        i=2;
                }
                break;
                case 1:
                {
                /* Searching in system. */

                    lc_wcstombs(mbFontName, onlyFileName, MAX_FONT_NAME_LENGTH);

                    lc_sprintf(file_path, "%s%s%s", FONT_SYSTEM_PATH, IFX_PLAT_DIR_SEP, mbFontName);

                    if (IFX_SUCCESS == IFXE_File_Open(&localHandle, file_path))
                    {
                        ifxp_status = IFX_SUCCESS;
                        if (handle)
                            *handle = localHandle;
                        else
                            IFXE_File_Close(localHandle);
                        i=2;
                    }
                    else
                        useSystemPath=0;
                }
            }
        }
    }

    return (ifxp_status);
}

/*************************************************************************
*   FUNCTION
*
*       PL_Read_Font_Data
*
*   DESCRIPTION
*
*       Opens and read the font file and fill the font buffer with the
*       font file data.
*
*   INPUTS
*
*       internal_font       - Pointer to internal font data structure.
*
*       font_name           - Pointer to string containing file name.
*
*   OUTPUTS
*
*       IFX_SUCCESS         - On success.
*
*       IFX_ERROR           - On failure.
*
*************************************************************************/
static IFX_RETURN_STATUS    PL_Read_Font_Data(IFXP_FT_INTERNAL_FONT *internal_font,
                                              const IFX_WCHAR *font_name)
{
    IFXE_FILE    font_file_descripter = NULL;
    void        *pointer;
    IFX_UINT32   bytes_read = 0;
    IFX_UINT32   file_size;
    char         file_path [PL_MAX_FILE_LENGTH];


    if (IFX_SUCCESS != PL_Get_Font_File_Path(file_path,
                                             &font_file_descripter,
                                             (const IFX_WCHAR *)font_name))
    {
        return (IFX_ERROR);
    }

    if (IFX_SUCCESS != IFXE_File_Size(font_file_descripter, &file_size))
    {
        IFXE_File_Close(font_file_descripter);

        return (IFX_ERROR);
    }

    if (IFX_SUCCESS != IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL,
                                         file_size,
                                         (void **)&pointer))
    {

        IFXE_File_Close(font_file_descripter);

        return IFX_ERROR;
    }
    else
    {
        internal_font->ft_font_buffer = (unsigned char *) pointer;
    }

    if (IFX_SUCCESS != IFXE_File_Read(font_file_descripter,
                                      internal_font->ft_font_buffer,
                                      file_size,
                                      &bytes_read))
    {
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, internal_font->ft_font_buffer);
        internal_font->ft_font_buffer = NULL;

        IFXE_File_Close(font_file_descripter);

        return (IFX_ERROR);
    }
    else
    {
        internal_font->ft_font_buffer_size = bytes_read;
        internal_font->ft_is_file_font = IFX_TRUE;
    }

    IFXE_File_Close(font_file_descripter);
    font_file_descripter = NULL;

    return (IFX_SUCCESS);
}

#endif      /* IFX_USE_NATIVE_FONTS */

