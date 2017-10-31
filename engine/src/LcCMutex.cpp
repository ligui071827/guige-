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

#include "inflexionui/engine/inc/LcAll.h"
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


/*-------------------------------------------------------------------------*//**
	Constructor
*/
LcCMutex::LcCMutex()
{
}

/*-------------------------------------------------------------------------*//**
	Destructor
*/
LcCMutex::~LcCMutex()
{
	IFXP_Mutex_Destroy(m_mutex);
}

/*-------------------------------------------------------------------------*//**
	Creates a mutex semaphore
	returns - Created mutex
*/
LcTaOwner<LcCMutex> LcCMutex::create(const LcTmString& name)
{
	LcTaOwner<LcCMutex> ref;
	ref.set(new LcCMutex());

	if (ref->construct(name) != true)
		ref.destroy();

	return ref;
}

/*-------------------------------------------------------------------------*//**
	Creates a mutex semaphore
*/
bool LcCMutex::construct(const LcTmString& name)
{
	if (IFX_SUCCESS == IFXP_Mutex_Create(&m_mutex, name.bufWChar()))
		return true;
	return false;
}

/*-------------------------------------------------------------------------*//**
	Attempts to gain control of a mutex
	timeout - Max time in milliseconds to wait for the semaphore, defaults to INFINITE
	returns - Whether the mutex has been obtained
*/
bool LcCMutex::lock(int timeout)
{
	IFX_RETURN_STATUS status = IFX_ERROR;
	
	switch (timeout)
	{
		case EInfinite:
			status = IFXP_Mutex_Lock(m_mutex, IFXP_MUTEX_INFINITE);
		break;
		
		default:
			status = IFXP_Mutex_Lock(m_mutex, timeout);
		break;
	}
		
	if (status == IFX_SUCCESS)
	{
		return true;
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
	Releases a previously obtained mutex
*/
void LcCMutex::unlock()
{
	IFXP_Mutex_Unlock(m_mutex);
}
