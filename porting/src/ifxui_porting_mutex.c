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
*       ifxui_porting_mutex.c
*
* COMPONENT
*
*       Inflexion UI Porting Layer.
*
* DESCRIPTION
*
*       Inflexion UI Engine Porting API Mutex Implementation for GrafixRS
*
* DATA STRUCTURES
*
*       None
*
* FUNCTIONS
*
*       IFXP_Mutex_Create                   Create mutex.
*
*       IFXP_Mutex_Destroy                  Delete a mutex.
*
*       IFXP_Mutex_Lock                     Lock mutex.
*
*       IFXP_Mutex_Unlock                   Unlock mutex.
*
* DEPENDENCIES
*
*       ifxui_porting_linux.h               Main include for Linux
*                                           porting layer.
*
*************************************************************************/

/* Inflexion UI Engine includes. */
#include    "inflexionui/porting/inc/ifxui_porting_linux.h"

#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/ipc.h>
#include    <sys/sem.h>

#define     PL_MUTEX_INTERNAL_KEY           0x1234

/* Structure to hold information about mutex. */
typedef struct _IFXP_INTERNAL_MUTEX_STRUCT
{
    unsigned int    m_mutex_id;
    unsigned int    m_owning_task;

} IFXP_INTERNAL_MUTEX;

typedef union _semun_union
{
      int val;
      struct semid_ds *buf;
      ushort * array;
} PL_MUTEX_SEMUN;

/*************************************************************************
* FUNCTION
*
*       IFXP_Mutex_Create
*
* DESCRIPTION
*
*       Creates a mutex semaphore.
*
* INPUTS
*
*       mutex                               Mutex to be constructed.
*
*       name                                Name for the mutex.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Mutex_Create(IFXP_MUTEX *mutex, const IFX_WCHAR *name)
{
    IFXP_INTERNAL_MUTEX *internal_mutex = NULL;
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;
    PL_MUTEX_SEMUN argument;
    struct sembuf operations[1];
    int     mutex_key;
    static int mutex_count = 0;

    ifxp_status = IFXP_Mem_Allocate(IFXP_MEM_EXTERNAL, sizeof(IFXP_INTERNAL_MUTEX), (void**)&internal_mutex);

    /* Check to see if the previous operation was successful. */
    if (ifxp_status == IFX_SUCCESS)
    {
        /* Store the internal mutex for later use. */

        *mutex = (IFXP_MUTEX)internal_mutex;
        argument.val = 0;

        /* Generate key index. */
        mutex_count++;
        mutex_key = (PL_MUTEX_INTERNAL_KEY + (mutex_count * 10));

        /* Create the semaphore with external key KEY if it doesn't already
           exists. Give permissions to the world. */

        internal_mutex->m_mutex_id = semget(mutex_key, 1, 0666 | IPC_CREAT);

        if (internal_mutex->m_mutex_id < 0)
        {
            ifxp_status = IFX_ERROR;
        }
    }

    /* Check to see if the previous operation was successful. */
    if (ifxp_status == IFX_SUCCESS)
    {

        /* Set the value of the semaphore. */

        if( semctl(internal_mutex->m_mutex_id, 0, SETVAL, argument) < 0)
        {
           ifxp_status = IFX_ERROR;
        }
    }

    /* Check to see if the previous operation was successful. */
    if (ifxp_status == IFX_SUCCESS)
    {

        /* Set up the sembuf structure. */

        /* Semaphore number. */
        operations[0].sem_num = 0;

        /* Which operation? Subtract 1 from semaphore value. */
        operations[0].sem_op = 1;

        /* Set the flag for waiting. */
        operations[0].sem_flg = 0;

        /* Do operation. */

        if (semop(internal_mutex->m_mutex_id, operations, 1) != 0)
        {
            ifxp_status = IFX_ERROR;
        }
    }

    return ifxp_status;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Mutex_Destroy
*
* DESCRIPTION
*
*       This function closes mutex.
*
* INPUTS
*
*       mutex                               Mutex to be closed.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Mutex_Destroy(IFXP_MUTEX mutex)
{
    IFXP_INTERNAL_MUTEX *internal_mutex = (IFXP_INTERNAL_MUTEX *)mutex;
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;
    PL_MUTEX_SEMUN argument;

    argument.val = 0;
    semctl(internal_mutex->m_mutex_id, 0, IPC_RMID, argument);

    if (ifxp_status ==IFX_SUCCESS)
    {
        ifxp_status = IFXP_Mem_Deallocate(IFXP_MEM_EXTERNAL, internal_mutex);
        mutex = NULL;
    }
    else
    {
        ifxp_status = IFX_ERROR;
    }

    return ifxp_status;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Mutex_Lock
*
* DESCRIPTION
*
*       Attempts to gain control of a mutex.
*
* INPUTS
*
*       mutex                               Mutex to be locked.
*
*       timeout                             Max time in milliseconds to
*                                           wait for the semaphore,
*                                           defaults to INFINITE
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Mutex_Lock(IFXP_MUTEX mutex, IFX_UINT32 timeout)
{
    IFXP_INTERNAL_MUTEX *internal_mutex = (IFXP_INTERNAL_MUTEX*)mutex;
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;
    struct  sembuf operations[1];
    int     retval;

    if (ifxp_status == IFX_SUCCESS)
    {

        /* Semaphore number. */
        operations[0].sem_num = 0;

        /* Lock operation. */
        operations[0].sem_op = -1;

        /* Set the waiting flag. */
        operations[0].sem_flg = 0;

        /* Do operation. */
        retval = semop(internal_mutex->m_mutex_id, operations, 1);
        if(retval != 0)
        {
            ifxp_status = IFX_ERROR;
        }
    }

    return ifxp_status;
}

/*************************************************************************
* FUNCTION
*
*       IFXP_Mutex_Lock
*
* DESCRIPTION
*
*       Releases a previously obtained mutex.
*
* INPUTS
*
*       mutex                               Mutex to be unlocked.
*
* OUTPUTS
*
*       IFX_SUCCESS                         On success.
*
*       IFX_ERROR                           On failure.
*
*************************************************************************/
IFX_RETURN_STATUS   IFXP_Mutex_Unlock(IFXP_MUTEX mutex)
{
    IFXP_INTERNAL_MUTEX *internal_mutex = (IFXP_INTERNAL_MUTEX*)mutex;
    IFX_RETURN_STATUS ifxp_status = IFX_SUCCESS;
    struct  sembuf operations[1];
    int     retval;

    if (ifxp_status == IFX_SUCCESS)
    {
        /* Semaphore number. */
        operations[0].sem_num = 0;

        /* Unlock operation. */
        operations[0].sem_op = 1;

        /* Clear the waiting flag. */
        operations[0].sem_flg = 0;

        /* So do the operation! */
        retval = semop(internal_mutex->m_mutex_id, operations, 1);
        if(retval != 0)
        {
            ifxp_status = IFX_ERROR;
        }
    }

    return ifxp_status;
}

