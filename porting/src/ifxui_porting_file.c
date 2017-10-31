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
*       ifxui_porting_file.c
*
*   COMPONENT
*
*       Inflexion UI Porting Layer.
*
*   DESCRIPTION
*
*       Inflexion UI Engine Porting API File Implementation for Linux.
*
*   DATA STRUCTURES
*
*       None
*
*   FUNCTIONS
*
*       PL_File_Initialize
*       PL_File_Change_Context
*       PL_File_Shutdown
*       IFXP_File_Open
*       IFXP_File_Read
*       IFXP_File_Seek
*       IFXP_File_Size
*       IFXP_File_Close
*       IFXP_File_Find_First
*       IFXP_File_Find_Next
*       IFXP_File_Find_Close
*       PL_File_Convert_Drive_Letter
*       IFXP_Get_Relative_Cwd
*       IFXP_Set_Relative_Cwd
*       IFXP_Dir_Create
*       IFXP_File_Create
*       IFXP_File_Write
*
*   DEPENDENCIES
*
*       ifxui_porting_linux.h
*
*************************************************************************/

/* Inflexion UI Engine includes. */
#include    "inflexionui/porting/inc/ifxui_porting_linux.h"

#define     PL_MAX_FILE_LENGTH              256

#ifdef      IFX_USE_PLATFORM_FILES

#include    <stdio.h>
#include    <dirent.h>
#include    <sys/stat.h>

#if defined (IFX_GENERATE_SCRIPTS) || defined(IFX_USE_SCRIPTS)
/* Can hold relative path of the currently running theme
 * if it is differen from default location. */
static char current_dir_path[PL_MAX_FILE_LENGTH] = {0};
#endif

typedef struct _PL_FILE_SEARCH_STRUCT
{
    struct dirent **namelist;
    int     num_entries;
    char    search_text[PL_MAX_FILE_LENGTH];
    char    prefix[PL_MAX_FILE_LENGTH];
} PL_FILE_SEARCH;

/*
 * These are the default directory search locations
 * They can be over-ridden with environment variables:
 * IFX_ENV_A_DRIVE, IFX_ENV_B_DRIVE, IFX_ENV_C_DRIVE
 */
static char *ifx_a_drive = "/HASE01";
static char *ifx_b_drive = "/HASE01";
static char *ifx_c_drive = "/HASE01";
static char *ifx_writeable_drive = "";

char *PL_File_Convert_Writeable_FS(const char *engine_path, char *writeablePath);

/*************************************************************************
*   FUNCTION
*
*       PL_File_Initialize
*
*   DESCRIPTION
*
*       This function initializes the file module.
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
IFX_RETURN_STATUS   PL_File_Initialize(void)
{
    char *env;

    env = getenv("IFX_ENV_A_DRIVE");
    if (env != NULL)
    {
        ifx_a_drive = env;
    }

    env = getenv("IFX_ENV_B_DRIVE");
    if (env != NULL)
    {
        ifx_b_drive = env;
    }

    env = getenv("IFX_ENV_C_DRIVE");
    if (env != NULL)
    {
        ifx_c_drive = env;
    }

    env = getenv("IFX_ENV_WRITEABLE_DRIVE");
    if (env != NULL)
    {
        ifx_writeable_drive = env;
    }

    return IFX_SUCCESS;
}

/*************************************************************************
*   FUNCTION
*
*       PL_File_Change_Context
*
*   DESCRIPTION
*
*       This function performs any task context sensitive initialization.
*       The context of this function call is the UI Engine task.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   PL_File_Change_Context(void)
{
    return IFX_SUCCESS;
}

/*************************************************************************
*   FUNCTION
*
*       PL_File_Shutdown
*
*   DESCRIPTION
*
*       This function shuts down the file module.
*
*   INPUTS
*
*       None.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*************************************************************************/
IFX_RETURN_STATUS   PL_File_Shutdown(void)
{
    return IFX_SUCCESS;
}


/*************************************************************************
*   FUNCTION
*
*       IFXP_File_Open
*
*   DESCRIPTION
*
*       Open a file with the specified path and filename
*
*   INPUTS
*
*       handle          - Pointer to file handle to be populated.
*
*       file_name       - Filename to open.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_File_Open(IFXP_FILE* handle, const char* file_name)
{
    char  local_file_name[PL_MAX_FILE_LENGTH] = {0};
    char *drive_path = PL_File_Convert_Drive_Letter(file_name);
    IFX_RETURN_STATUS ifxp_status = IFX_ERROR;
    unsigned int drive_path_len = 0;
    unsigned int file_name_len = 0;

    if ((handle != NULL) && ((drive_path != NULL) || (file_name[0] =='/')))
    {
        FILE *local_handle;

        if (file_name[0] =='/')
        {
            /* Special case to allow access to absolute file paths */
            if (lc_strlen(file_name) <= PL_MAX_FILE_LENGTH)
                lc_strcpy(local_file_name, file_name);
            else
                return ifxp_status;
        }
        else
        {
            drive_path_len = lc_strlen(drive_path);
            file_name_len =  lc_strlen(file_name);

            if ((file_name_len + drive_path_len) > PL_MAX_FILE_LENGTH)
                return  ifxp_status;

            /* Replace the Engine drive letter with the platform specific
            drive letter + path. */
            lc_strcpy(local_file_name, drive_path);

#if defined (IFX_GENERATE_SCRIPTS) || defined(IFX_USE_SCRIPTS)
            // In case of ATF, we append current_dir_path also so need to take it into account also
            if ((file_name_len + drive_path_len + lc_strlen(current_dir_path)) > PL_MAX_FILE_LENGTH)
                return  ifxp_status;

            lc_strcpy(local_file_name + lc_strlen(local_file_name), current_dir_path);
#endif

            lc_strcpy(local_file_name + lc_strlen(local_file_name), &file_name[2]);
        }

        local_handle = fopen(local_file_name, "rb");
        *handle = (IFXP_FILE)local_handle;
        if (local_handle)
        {
            ifxp_status = IFX_SUCCESS;
        }
        else
        {
            ifxp_status = IFX_ERROR;
        }
    }

    if(ifxp_status == IFX_ERROR && (handle) && (file_name[0] !='/'))
    {
        // Try loading from the write able fs
        if(PL_File_Convert_Writeable_FS(file_name, local_file_name) != NULL)
        {
            if(lc_strlen(local_file_name) > 0)
            {
                FILE *local_handle;

                local_handle = fopen(local_file_name, "rb");
                *handle = (IFXP_FILE)local_handle;
                if (local_handle)
                {
                    ifxp_status = IFX_SUCCESS;
                }
                else
                {
                    ifxp_status = IFX_ERROR;
                }
            }
        }
    }

    return ifxp_status;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_File_Read
*
*   DESCRIPTION
*
*       Reads the specified number of bytes from an open file.
*
*   INPUTS
*
*       handle          - Handle to previously opened file.
*
*       addr            - Address of the buffer to read data to.
*
*       size            - Number of bytes to attempt to read.
*
*       bytes_read      - Pointer to variable to receive the number of bytes read.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_File_Read(IFXP_FILE handle, void *addr,
                                   IFX_UINT32 size, IFX_UINT32 *bytes_read)
{
    int ret_val = -1;

    if (!bytes_read)
    {
        return IFX_ERROR;
    }

    ret_val = fread(addr, 1, size, (FILE*)handle);

    if (ret_val >= 0)
    {
        *bytes_read = ret_val;
        return IFX_SUCCESS;
    }

    return IFX_ERROR;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_File_Seek
*
*   DESCRIPTION
*
*       Seek to the specified location within a file.
*
*   INPUTS
*
*       handle          Handle to previously opened file.
*
*       offset          Offset from the specified location to seek to.
*
*       seek_mode       - Specifies where to seek from.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_File_Seek(IFXP_FILE handle, IFX_INT32 offset,
                                     IFX_SEEK seek_mode)
{
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;
    int mode;

    FILE* file = (FILE*)handle;
    if (file)
    {
        mode = SEEK_SET;
        if (IFX_SEEK_CUR == seek_mode)
            mode = SEEK_CUR;
        else if (IFX_SEEK_END == seek_mode)
            mode = SEEK_END;

        if (0 == fseek(file, offset, mode))
        {
            ifxp_status = IFX_SUCCESS;
        }
    }

    return ifxp_status;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_File_Size
*
*   DESCRIPTION
*
*       Returns the size of the specified file
*
*   INPUTS
*
*       handle          - Handle to previously opened file.
*
*       file_size       - Pointer to variable to receive the file size.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_File_Size(IFXP_FILE handle,
                                   IFX_UINT32 *file_size)
{
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;
    int ret_val = -1;
    int pos;

    if (!file_size)
    {
        return IFX_ERROR;
    }

    *file_size = 0;

    if (handle)
    {
        /* Save the current file location*/
        pos = ftell((FILE*)handle);

        /* Seek to the end of the file */
        IFXP_File_Seek(handle, 0, IFX_SEEK_END);

        /* Retrieve the end location */
        ret_val = ftell((FILE*)handle);

        /* Restore the original location */
        IFXP_File_Seek(handle, pos, IFX_SEEK_SET);
    }

    if (ret_val >= 0)
    {
        *file_size = ret_val;
        return IFX_SUCCESS;
    }

    return ifxp_status;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_File_Close
*
*   DESCRIPTION
*
*       Closes an open file.
*
*   INPUTS
*
*       handle          - Open file handle.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_File_Close(IFXP_FILE handle)
{
    if (handle)
    {
        fclose((FILE*)handle);
    }

    return IFX_SUCCESS;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_File_Find_First
*
* DESCRIPTION
*
*       Find the first occurrence of a filename containing the
*       specified text.
*
* INPUTS
*
*       handle          Pointer to a file handle pointer.
*
*       info_ptr        - Pointer to a void pointer used to hold search context.
*
*       search_test     - The text to search for.
*
* OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_File_Find_First(IFXP_FILE* handle,
                                         IFXP_SEARCH* info_ptr,
                                         char* search_text)
{
    IFX_RETURN_STATUS ifxp_status = IFX_ERROR;
    char local_search_text[PL_MAX_FILE_LENGTH];
    char *drivePath = PL_File_Convert_Drive_Letter(search_text);
    PL_FILE_SEARCH *search;
    struct stat sbuf; /* Structure for FILE statistics. */
    unsigned int drive_path_len = 0;
    unsigned int search_text_len = 0;

    if (!drivePath || !search_text)
        return ifxp_status;

    drive_path_len = lc_strlen(drivePath);
    search_text_len = lc_strlen(search_text);

    if (!handle || !info_ptr || ( drive_path_len + search_text_len) > PL_MAX_FILE_LENGTH)
    {
        return ifxp_status;
    }

    /* Default to NULL */
    info_ptr->internal = NULL;
    *handle = NULL;

    /* Replace the Engine drive letter with the platform specific
       drive letter + path.
    */
    lc_strcpy(local_search_text, drivePath);

#if defined (IFX_GENERATE_SCRIPTS) || defined(IFX_USE_SCRIPTS)

    if (( drive_path_len + search_text_len + lc_strlen(current_dir_path)) > PL_MAX_FILE_LENGTH)
    {
        return ifxp_status;
    }

    lc_strcpy(local_search_text + lc_strlen(local_search_text), current_dir_path);
#endif

    lc_strcpy(local_search_text + lc_strlen(local_search_text), &search_text[2]);

    /* Done as code block to prevent use of temporary variables */
    {
        char *dir  = local_search_text;
        char *file = local_search_text;
        char *ptr  = local_search_text;

        /* Search for the last directory separator */
        do
        {
            file = ptr;
            ptr = strstr(ptr+1, "/");
        } while (ptr);

        if (file != local_search_text)
        {
            file++;
        }
        else
        {
            /* No search pattern specified */
            return IFX_ERROR;
        }

        /* Allocate the search block */
        (void)IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL, sizeof(PL_FILE_SEARCH), (void**)&search);
        if (!search)
            return IFX_ERROR;

        /* Copy the search pattern without dir prefix */
        strncpy(search->search_text, file, PL_MAX_FILE_LENGTH);

        /* Copy the directory prefix (This will clobber the search text) */
        *file = 0;
        strncpy(search->prefix, dir, PL_MAX_FILE_LENGTH);
    }

    search->num_entries = scandir(search->prefix, &search->namelist, 0, (void *)alphasort);

    {
        char *ptr;
        ptr = strstr(search->search_text, "*");
        if (ptr)
            *ptr = 0;
    }

    /* Look for the first occurrence of file */
    while (search->num_entries-- > 0)
    {
        char *file_name = search->namelist[search->num_entries]->d_name;

        if (strstr(file_name, search->search_text))
        {
            lc_strcpy(local_search_text, search->prefix);
            lc_strcpy(local_search_text + lc_strlen(local_search_text), file_name);
            lc_strcpy(info_ptr->fileName, file_name);


            /* Check to see whether the filename is DIR or FILE. */
            stat(local_search_text, &sbuf);
            if (S_ISREG(sbuf.st_mode))
            {
                info_ptr->info = IFXP_INFO_FILE;
            }
            else
            {
                info_ptr->info = IFXP_INFO_DIRECTORY;
            }

            ifxp_status = IFX_SUCCESS;
        }

    free(search->namelist[search->num_entries]);

    if (ifxp_status == IFX_SUCCESS)
        break;

    }

    if (ifxp_status == IFX_SUCCESS)
    {
        info_ptr->internal = (void*)search;
        *handle = (IFXP_FILE)1;
    }
    else
    {
        if (search->num_entries <= 0)
        {
            IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, search);
            search = NULL;
        }
    }

    return ifxp_status;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_File_Find_Next
*
*   DESCRIPTION
*
*       Find the next occurrence of a filename searched for with
*       IFXP_File_Find_First.
*
*   INPUTS
*
*       handle          - Pointer to a file handle.
*
*       info_ptr        - Pointer to search context.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_File_Find_Next(IFXP_FILE handle,
                                        IFXP_SEARCH* info_ptr)
{
    IFX_RETURN_STATUS status = IFX_ERROR;
    char local_search_text[PL_MAX_FILE_LENGTH];
    PL_FILE_SEARCH *search;
    struct stat sbuf; /* Structure for FILE statistics. */

    if (!handle || !info_ptr)
    {
        return status;
    }

    search = (PL_FILE_SEARCH*)info_ptr->internal;

    /* Look for the next occurrence of file */
    while (search->num_entries-- > 0)
    {
        char *file_name = search->namelist[search->num_entries]->d_name;

        if (strstr(file_name, search->search_text))
        {
            lc_strcpy(local_search_text, search->prefix);
            lc_strcpy(local_search_text + lc_strlen(local_search_text), file_name);
            lc_strcpy(info_ptr->fileName, file_name);

            /* Check to see whether the filename is DIR or FILE. */
            stat(local_search_text, &sbuf);
            if (S_ISREG(sbuf.st_mode))
            {
                info_ptr->info = IFXP_INFO_FILE;
            }
            else
            {
                info_ptr->info = IFXP_INFO_DIRECTORY;
            }

            status = IFX_SUCCESS;
            break;
        }
        else
        {
            free(search->namelist[search->num_entries]);
        }
    }

    return status;
}


/*************************************************************************
*   FUNCTION
*
*       IFXP_File_Find_Close
*
*   DESCRIPTION
*
*       Closes the handle and info_ptr created with IFXP_File_Find_First.
*
*   INPUTS
*
*       handle          - Pointer to a file handle.
*
*       info_ptr        - Pointer to search context.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_File_Find_Close(IFXP_FILE handle,
                                         IFXP_SEARCH* info_ptr)
{
    IFX_RETURN_STATUS ifxp_status = IFX_ERROR;
    PL_FILE_SEARCH *search;

    if (!info_ptr)
    {
        return ifxp_status;
    }

    search = (PL_FILE_SEARCH*)info_ptr->internal;

    if (search)
    {
        /* Delete all remaining entries */
        while (search->num_entries-- > 0)
        {
            free(search->namelist[search->num_entries]);
        }

        free(search->namelist);
        IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, search);

        info_ptr->internal = NULL;

        ifxp_status = IFX_SUCCESS;
    }

    /* We need to return success to work around an Engine BUG */
    ifxp_status = IFX_SUCCESS;

    return ifxp_status;
}

/*************************************************************************
*   FUNCTION
*
*       PL_File_Convert_Writeable_FS
*
*   DESCRIPTION
*
*       Converts the drive letter.
*
*   INPUTS
*
*       engine_path     - Pointer to string containing file path.
*
*   OUTPUTS
*
*       char*                 - Pointer to the string containing path to a writeable fs simillar to the file path.
*       writeablePath   - Pointer to string to copy path to.
*
*************************************************************************/
char *PL_File_Convert_Writeable_FS(const char *engine_path, char *writeablePath)
{
    char *temp_ptr = NULL;

    if(!engine_path || !writeablePath)
        return temp_ptr;

    if (lc_strlen(engine_path) < 3)
    {
        temp_ptr = NULL;
    }
    else if (engine_path[1] != ':' || engine_path[0] < 'A' ||
              engine_path[0] > 'Z')
    {
        temp_ptr = NULL;
    }
    else
    {
        int pathLen = 0;
        temp_ptr = writeablePath;

        if (lc_strlen(ifx_writeable_drive) != 0)
        {
            lc_strncpy(writeablePath, ifx_writeable_drive, PL_MAX_FILE_LENGTH);
            pathLen =  lc_strlen(writeablePath);
            lc_strncpy(writeablePath + pathLen, "/", (PL_MAX_FILE_LENGTH - pathLen));
        }
        else
        {
            // if no writable is defined... we will try in current directory
            lc_strncpy(writeablePath, ifx_c_drive, PL_MAX_FILE_LENGTH);
            pathLen =  lc_strlen(writeablePath);
            lc_strncpy(writeablePath + pathLen, "/writable/", (PL_MAX_FILE_LENGTH - pathLen));
        }

        pathLen =  lc_strlen(writeablePath);
        writeablePath += pathLen;
        lc_strncpy(writeablePath, engine_path, (PL_MAX_FILE_LENGTH - pathLen));
        writeablePath[1] = '_';
    }

    return temp_ptr;
}

/*************************************************************************
*   FUNCTION
*
*       PL_File_Convert_Drive_Letter
*
*   DESCRIPTION
*
*       Converts the drive letter.
*
*   INPUTS
*
*       engine_path     - Pointer to string containing file path.
*
*   OUTPUTS
*
*       char*           - Pointer to the string containing current driver letter.
*
*************************************************************************/
char *PL_File_Convert_Drive_Letter(const char *engine_path)
{
    char *temp_ptr = NULL;
    /* Perform mapping of Engine drive letters to platform paths. */

    /* Note that when ROM files are enabled, the drive Z is reserved
       and not available here. */

    if (engine_path)
    {
      if (lc_strlen(engine_path) < 3)
      {
          temp_ptr = NULL;
      }
      else if (engine_path[1] != ':' || engine_path[0] < 'A' ||
                engine_path[0] > 'Z')
      {
          temp_ptr = NULL;
      }
      else if (engine_path[0] == 'A')
      {
          temp_ptr = ifx_a_drive;
      }
      else if (engine_path[0] == 'B')
      {
          temp_ptr = ifx_b_drive;
      }
      else if (engine_path[0] == 'C')
      {
          temp_ptr = ifx_c_drive;
      }
      else
      {
          temp_ptr = NULL;
      }
    }

    return temp_ptr;
}

#if defined (IFX_GENERATE_SCRIPTS) || defined(IFX_USE_SCRIPTS)

/*************************************************************************
*   FUNCTION
*
*       IFXP_Get_Relative_Cwd
*
*   DESCRIPTION
*
*       Returns the current relative directory.
*
*   INPUTS
*
*       current_dir       contains the current relative directory path
*
* OUTPUTS
*
*   OUTPUTS
*
*       IFX_SUCCESS
*
*************************************************************************/
IFX_RETURN_STATUS IFXP_Get_Relative_Cwd (char *current_dir)
{
    lc_strcpy(current_dir, current_dir_path);
    return IFX_SUCCESS;
}


/*************************************************************************
*   FUNCTION
*
*       IFXP_Set_Relative_Cwd
*
*   DESCRIPTION
*
*       Sets the current relative working directory.
*
*   INPUTS
*
*       new_dir     contain the new relative directory path
*
* OUTPUTS
*
*   OUTPUTS
*
*       IFX_SUCCESS
*
*************************************************************************/
IFX_RETURN_STATUS IFXP_Set_Relative_Cwd (const char *new_dir)
{
    lc_strcpy(current_dir_path, new_dir);
    return IFX_SUCCESS;
}

#endif /* defined (IFX_GENERATE_SCRIPTS) || defined(IFX_USE_SCRIPTS) */

/*************************************************************************
*   FUNCTION
*
*       IFXP_Dir_Create
*
*   DESCRIPTION
*
*       Creates a directory.
*
*   INPUTS
*
*       new_dir          contains the path of  directory to create
*
* OUTPUTS
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS IFXP_Dir_Create (const char *new_dir)
{

    IFX_RETURN_STATUS ifxp_status = IFX_ERROR;

    char  local_dir_name[PL_MAX_FILE_LENGTH];
    char *drive_path = PL_File_Convert_Drive_Letter(new_dir);
    char *local_dir_temp = NULL;
    char *local_dir_temp2 = NULL;
    int   status = 0;

    if (new_dir == NULL)
        return IFX_ERROR;

    if (drive_path || (new_dir[0] =='/') || (new_dir[0] =='.'))
    {
        int pathLen = drive_path ? lc_strlen(drive_path) : 0;
        pathLen += lc_strlen(new_dir);
        if (pathLen > PL_MAX_FILE_LENGTH)
            return  ifxp_status;

        if ((new_dir[0] =='/')
            || (new_dir[0] =='.'))
        {
            /* Special case to allow access to absolute file paths or current directory*/
            lc_strcpy(local_dir_name, new_dir);
            drive_path = "";
        }
        else
        {
            /* Replace the Engine drive letter with the platform specific
               drive letter + path. */
            lc_strcpy(local_dir_name, drive_path);
            lc_strcpy(local_dir_name + lc_strlen(local_dir_name), &new_dir[2]);
        }

        status = mkdir(local_dir_name,S_IRWXU | S_IRWXG | S_IRWXO);

        if(status == 0 || errno == EEXIST)
        ifxp_status = IFX_SUCCESS;

        if(ifxp_status != IFX_SUCCESS)
        {
            // If it is a sub-directory of a directory that does not exist

            local_dir_temp  = local_dir_name;
            local_dir_temp2 = local_dir_name + lc_strlen(drive_path);

            ifxp_status = IFX_SUCCESS;

            while(local_dir_temp2 != NULL)
            {
                ++local_dir_temp2;
                local_dir_temp2 = lc_strchr(local_dir_temp2, '/');
                if(local_dir_temp2 != NULL)
                {
                    *local_dir_temp2 = 0;
                }

                status = mkdir(local_dir_temp,S_IRWXU | S_IRWXG | S_IRWXO);

                if(local_dir_temp2 != NULL)
                {
                    *local_dir_temp2 = '/';
                }

                if(status != 0 && errno != EEXIST)
                    ifxp_status = IFX_ERROR;
            }
        }
    }
    return ifxp_status;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_File_Create
*
*   DESCRIPTION
*
*       Create a file with the specified filename
*
*   INPUTS
*
*       handle          - Pointer to file handle to be populated.
*
*       file_name       - Filename including path on which to create file.
*
*   OUTPUTS
*
*       IFX_SUCCESS     - On success.
*       IFX_ERROR       - On failure.
*
*************************************************************************/
IFX_RETURN_STATUS IFXP_File_Create (IFXP_FILE *handle, const char *file_name)
{
    IFX_RETURN_STATUS ifxp_status = IFX_ERROR;

    char  local_file_name[PL_MAX_FILE_LENGTH];
    char *drive_path = PL_File_Convert_Drive_Letter(file_name);

    if ((handle) && (drive_path) && (file_name))
    {
        if ((lc_strlen(file_name) + lc_strlen(drive_path)) > PL_MAX_FILE_LENGTH)
            return ifxp_status;

        /* Replace the Engine drive letter with the platform specific
           drive letter + path. */

        lc_strcpy(local_file_name, drive_path);
        lc_strcpy(local_file_name + lc_strlen(local_file_name), &file_name[2]);

        if(lc_strlen(local_file_name) > 0)
        {
            FILE *local_handle;
            local_handle = fopen(local_file_name, "wb");
            *handle = (IFXP_FILE)local_handle;
            if (local_handle)
            {
                ifxp_status = IFX_SUCCESS;
            }
            else
            {
                ifxp_status = IFX_ERROR;
            }
        }
    }

    if(ifxp_status == IFX_ERROR && (handle))
    {
        // Try saving to the write able fs
        if((PL_File_Convert_Writeable_FS(file_name,local_file_name)))
        {
            char *saveDir = lc_strrchr(local_file_name, '/');

            if(saveDir)
            {
                // terminate string at the last directory separator and create directory to save the file in.
                *saveDir = 0;
                IFXP_Dir_Create(local_file_name);
                // restore the directory separator, now we can use the file path to create the file..
                *saveDir = '/';
            }

            if(lc_strlen(local_file_name) > 0)
            {
                FILE *local_handle;

                local_handle = fopen(local_file_name, "wb");
                *handle = (IFXP_FILE)local_handle;
                if (local_handle)
                {
                    ifxp_status = IFX_SUCCESS;
                }
                else
                {
                    ifxp_status = IFX_ERROR;
                }
            }
        }
    }

    return ifxp_status;
}

/*************************************************************************
*   FUNCTION
*
*       IFXP_File_Write
*
*   DESCRIPTION
*
*       Write the specified number of bytes to an open file.
*
*   INPUTS
*
*       handle                Handle to previously opened file.
*
*       buffer                Contain the data to be written into file.
*
*       size                  Size of the buffer.
*
*       each_element_size   - Size in bytes of each element to be written.
*
*   OUTPUTS
*
*       bytes_wrote           Returns the bytes wrote into file.
*       IFX_SUCCESS           On success.
*       IFX_ERROR             On failure.
*
*************************************************************************/
IFX_RETURN_STATUS  IFXP_File_Write(IFXP_FILE handle, void *addr, IFX_UINT32 size, IFX_UINT32 *bytes_written)
{
    IFX_RETURN_STATUS ifxp_status = IFX_ERROR;
    int ret_val = -1;

    if (addr && (size > 0) && bytes_written)
    {
        ret_val = fwrite(addr, 1, size, handle);
    }

    if (ret_val >= 0)
    {
        *bytes_written = ret_val;
        ifxp_status = IFX_SUCCESS;
    }

    return ifxp_status;
}

#endif      /* IFX_USE_PLATFORM_FILES */


