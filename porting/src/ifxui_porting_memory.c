/*************************************************************************
*
*            Copyright 2006 Mentor Graphics Corporation
*                         All Rights Reserved.
*
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS
* SUBJECT TO LICENSE TERMS.
*
**************************************************************************

**************************************************************************
* FILE NAME
*
*       ifxui_porting_memory.c
*
* COMPONENT
*
*       Inflexion UI Porting Layer.
*
* DESCRIPTION
*
*       Inflexion UI Engine Porting API Memory Implementation for GrafixRS
*
* DATA STRUCTURES
*
*       None
*
* FUNCTIONS
*
*       PL_Memory_Initialize                Initializes the porting layer.
*
*       PL_Memory_Shutdown                  Shuts down the porting layer.
*
*       IFXP_Mem_Allocate                   Allocates memory.
*
*       IFXP_Mem_Deallocate                 Deallocates memory.
*
*       IFXP_Mem_Compress                   Compress memory fragmentation.
*
* DEPENDENCIES
*
*       ifxui_porting_linux.h               Main include for Linux
*                                           porting layer.
*
*************************************************************************/

/* Inflexion UI Engine includes. */
#include    "inflexionui/porting/inc/ifxui_porting_linux.h"
#include    "inflexionui/engine/ifxui_engine_porting.h"

/*************************************************************************
* FUNCTION
*
*       PL_Memory_Initialize
*
* DESCRIPTION
*
*       This function initializes the memory related porting of the
*       porting layer.
*
* INPUTS
*
*      *param                               Implementation specific pointer.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   PL_Memory_Initialize(void)
{
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;

    return ifxp_status;
}

/*************************************************************************
* FUNCTION
*
*       PL_Memory_Shutdown
*
* DESCRIPTION
*
*       This function shuts down the memory related porting layer.
*
* INPUTS
*
*       None
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   PL_Memory_Shutdown(void)
{
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;

    return ifxp_status;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Mem_Allocate
*
* DESCRIPTION
*
*       This function allocates memory from the specified dynamic memory
*       pool.
*
* INPUTS
*
*       usage                               Memory pool selection parameter.
*
*       size                                Size of memory block
*
*     **ptr                                 Pointer to the allocated memory
*                                           block.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Mem_Allocate(IFXP_MEM_USE usage,
                                      IFX_UINT32 size, void **ptr)
{
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;
    IFX_UINT32 *temp_ptr = NULL;

#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE
    temp_ptr = (IFX_UINT32 *)malloc(size + sizeof(IFX_UINT32));
#else
    temp_ptr = (IFX_UINT32 *)malloc(size);
#endif      /* IFX_ENABLE_BENCHMARKING_HEAP_USAGE */

    if (temp_ptr == NULL)
    {
        ifxp_status = IFX_ERROR;
    }
    else
    {
        *ptr = temp_ptr;
        if(size>=4)
       		temp_ptr[(int)(size/4)-1] = 0;
//   	  memset(temp_ptr,0,size);
    }

#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE

    if (ifxp_status == IFX_SUCCESS)
    {
        *ptr = temp_ptr+1;
        *temp_ptr = size;

        switch (usage)
        {
            case IFXP_MEM_ENGINE:
                pl_engine_heap_usage += size;
                if (pl_engine_heap_usage > pl_engine_heap_usage_max) pl_engine_heap_usage_max = pl_engine_heap_usage;
            break;

            case IFXP_MEM_EXTERNAL:
                pl_generic_heap_usage += size;
                if (pl_generic_heap_usage > pl_generic_heap_usage_max) pl_generic_heap_usage_max = pl_generic_heap_usage;
            break;

            default:
            break;
        }
    }

#else

    /* Suppress compiler warning. */
    (void) usage;

#endif      /* IFX_ENABLE_BENCHMARKING_HEAP_USAGE */

    return  ifxp_status;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Mem_Deallocate
*
* DESCRIPTION
*
*       This function deallocates a previously allocated memory block.
*
* INPUTS
*
*       usage                               Memory pool selection parameter.
*
*      *ptr                                 Pointer to the allocated memory
*                                           block.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Mem_Deallocate(IFXP_MEM_USE usage, void *ptr)
{
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;

    /* Check the validity of input pointer. */

    if (ptr)
    {
        IFX_UINT32   *free_ptr;

       free_ptr = ptr;

#ifdef      IFX_ENABLE_BENCHMARKING_HEAP_USAGE

        free_ptr--;

        switch (usage)
        {
            case IFXP_MEM_ENGINE:
                pl_engine_heap_usage -= *free_ptr;
            break;

            case IFXP_MEM_EXTERNAL:
                pl_generic_heap_usage -= *free_ptr;
            break;

            default:
            break;
        }

#else

        /* Suppress compiler warning. */
        (void) usage;

#endif      /* IFX_ENABLE_BENCHMARKING_HEAP_USAGE */

        free(free_ptr);

    }
    else
    {
        ifxp_status = IFX_ERROR;
    }

    return  ifxp_status;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Mem_Compress
*
* DESCRIPTION
*
*       This function compresses heap.
*
* INPUTS
*
*       None
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Mem_Compress(void)
{
    return IFX_SUCCESS;
}

