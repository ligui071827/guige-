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


#include "inflexionui/engine/inc/NdhsCExclusivity.h"

/*-------------------------------------------------------------------------*//**
	Creation
*/
LcTaOwner<NdhsCExclusivity> NdhsCExclusivity::create(IExclusivityHandler* pHandler,
													LcCSpace* pSpace,
													int numModules)
{
	LcTaOwner<NdhsCExclusivity> ref;
	ref.set(new NdhsCExclusivity(pHandler, pSpace));
	ref->construct(numModules);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Construction
*/
void NdhsCExclusivity::construct(int numModules)
{
	LcTaOwner<LcCMutex> newMutex = m_space->createMutex(LcTaString("ExclMap"));
	m_requestListMutex = newMutex;
	m_numModules = 0;

	if (numModules > 0)
	{
		m_requestList.alloc(numModules);
		m_numModules = numModules;
	}
}

/*-------------------------------------------------------------------------*//**
	Constructor
*/
NdhsCExclusivity::NdhsCExclusivity(IExclusivityHandler* pHandler, LcCSpace* pSpace)
:	m_handler(pHandler),
	m_space(pSpace)
{
}

/*-------------------------------------------------------------------------*//**
	Cleanup on destruction
*/
NdhsCExclusivity::~NdhsCExclusivity()
{
	m_requestListMutex.destroy();

	if (m_requestList)
	{
		int i;
		for (i=0; i<m_numModules; i++)
		{
			if(m_requestList[i].m_message.isScheduled())
				m_requestList[i].m_message.cancel();
		}

		m_requestList.free();
	}
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS NdhsCExclusivity::requestExclusivity(
		IFX_HEXCLUSIVITY 	hExclusivity,
		IFX_UINT32 			priority,
		IFX_UINT32 			timeout)
{
	IFX_RETURN_STATUS retVal = IFX_ERROR;

	if (m_requestListMutex->lock(LcCMutex::EInfinite))
	{
		bool requestExists = false;
		int freeSlot = -1;

		// Try to find any existing requests with the same handle
		for(int i=0; i<m_numModules; i++)
		{
			if(m_requestList[i].m_exclusivityHandle == hExclusivity)
			{
				requestExists = true;
			}
			else if((freeSlot == -1) && (m_requestList[i].m_exclusivityHandle == NULL) )
			{
				freeSlot = i;
			}
		}

		// Treat as error if already requested
		if (!requestExists && freeSlot != -1)
		{
			bool 					requester1StatusChange 		= false;
			IFX_HEXCLUSIVITY 		requester1ExclusivityHandle = NULL;
			IFX_EXCLUSIVITY_STATUS	requester1NewStatus 		= IFX_EXCLUSIVITY_STATUS_ERROR;

			bool 					requester2StatusChange 		= false;
			IFX_HEXCLUSIVITY 		requester2ExclusivityHandle	= NULL;
			IFX_EXCLUSIVITY_STATUS	requester2NewStatus 		= IFX_EXCLUSIVITY_STATUS_ERROR;

			if (m_exclusivityOwner)
			{
				// Something already has exclusivity
				if (priority < m_exclusivityOwner->m_priority)
				{
					// New request has higher priority: currently exclusive request loses exclusivity

					// Tell the currently exclusive owner it has had its exclusivity suspended
					requester1StatusChange = true;
					requester1ExclusivityHandle = m_exclusivityOwner->m_exclusivityHandle;
					requester1NewStatus = IFX_EXCLUSIVITY_STATUS_SUSPENDED;

					// New request gains exclusivity
					m_requestList[freeSlot].m_priority = priority;
					m_requestList[freeSlot].m_exclusivityHandle = hExclusivity;
					m_requestList[freeSlot].m_message = LcTMessage(0,0);
					m_exclusivityOwner = &m_requestList[freeSlot];

					if (timeout != 0)
					{
						// If a timeout was specified, give the result back as a status change
						requester2StatusChange = true;
						requester2ExclusivityHandle = hExclusivity;
						requester2NewStatus = IFX_EXCLUSIVITY_STATUS_GRANTED;
					}

					retVal = IFX_SUCCESS;
				}
				else
				{
					// New request has lower priority
					if (timeout == 0)
					{
						// If no timeout, request has failed
						retVal = IFX_ERROR_EXCLUSIVITY;
					}
					else
					{
						m_requestList[freeSlot].m_priority = priority;
						m_requestList[freeSlot].m_exclusivityHandle = hExclusivity;
						m_requestList[freeSlot].m_message = LcTMessage(this,(int)hExclusivity);

						// Set up a message to callback after the timeout
						m_requestList[freeSlot].m_message.schedule(m_space->getTimer(), 0, timeout);

						retVal = IFX_SUCCESS;
					}
				}
			}
			else
			{
				// New request gains exclusivity straight away
				m_requestList[freeSlot].m_priority = priority;
				m_requestList[freeSlot].m_exclusivityHandle = hExclusivity;
				m_requestList[freeSlot].m_message = LcTMessage(0,0);
				m_exclusivityOwner = &m_requestList[freeSlot];

				if (timeout != 0)
				{
					// If a timeout was specified, give the result back as a status change
					requester1StatusChange = true;
					requester1ExclusivityHandle = hExclusivity;
					requester1NewStatus = IFX_EXCLUSIVITY_STATUS_GRANTED;
				}

				retVal = IFX_SUCCESS;
			}

			// Update m_exclusivityOwnerHandle
			if (m_exclusivityOwner)
			{
				m_exclusivityOwnerHandle = m_exclusivityOwner->m_exclusivityHandle;
			}

			// Only inform the requesters of any status change after our internal state changes are complete
			if (requester1StatusChange)
			{
				m_handler->exclusivityStatusChange(requester1ExclusivityHandle, requester1NewStatus);
			}

			if (requester2StatusChange)
			{
				m_handler->exclusivityStatusChange(requester2ExclusivityHandle, requester2NewStatus);
			}
		}

		m_requestListMutex->unlock();
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS NdhsCExclusivity::releaseExclusivity(
		IFX_HEXCLUSIVITY 	hExclusivity)
{
	IFX_RETURN_STATUS retVal = IFX_ERROR;

	if (m_requestListMutex->lock(LcCMutex::EInfinite))
	{
		// Find the request that the handle refers to
		int foundReqIndex = -1;
		int i;
		for(i=0; i<m_numModules; i++)
		{
			if(m_requestList[i].m_exclusivityHandle == hExclusivity)
			{
				foundReqIndex = i;
				break;
			}
		}

		// (If not found, treat as an error
		if (foundReqIndex != -1)
		{
			bool 					requester1StatusChange 		= false;
			IFX_HEXCLUSIVITY 		requester1ExclusivityHandle = NULL;
			IFX_EXCLUSIVITY_STATUS	requester1NewStatus 		= IFX_EXCLUSIVITY_STATUS_ERROR;

			if (m_exclusivityOwner)
			{
				if (hExclusivity == m_exclusivityOwner->m_exclusivityHandle)
				{
					// The request being released currently has exclusivity
					int highestPriorityRequest = -1;

					// Find next highest priority request
					for(i=0; i<m_numModules; i++)
					{
						if ( (m_requestList[i].m_exclusivityHandle != NULL) && (i != foundReqIndex))
						{
							if (highestPriorityRequest != -1)
							{
								if (m_requestList[i].m_priority < m_requestList[highestPriorityRequest].m_priority)
								{
									highestPriorityRequest = i;
								}
							}
							else
							{
								highestPriorityRequest = i;
							}
						}
					}

					if (highestPriorityRequest != -1)
					{
						// A request has been found: set it as the new exclusivity holder
						m_exclusivityOwner = &m_requestList[highestPriorityRequest];

						// Inform the requester of status change
						requester1StatusChange = true;
						requester1ExclusivityHandle = m_requestList[highestPriorityRequest].m_exclusivityHandle;
						requester1NewStatus = IFX_EXCLUSIVITY_STATUS_RESUMED; // may be modified below

						// Cancel scheduled message, if any
						if (m_requestList[highestPriorityRequest].m_message.isScheduled())
						{
							m_requestList[highestPriorityRequest].m_message.cancel();

							// If there was a message scheduled, then this is the first time
							// exclusivity has been granted: different status message needed
							requester1NewStatus = IFX_EXCLUSIVITY_STATUS_GRANTED;
						}
					}
					else
					{
						// No more requests waiting
						m_exclusivityOwner = NULL;
					}
				}
			}

			// Remove request from list
			m_requestList[foundReqIndex].m_priority = 0;
			m_requestList[foundReqIndex].m_exclusivityHandle = 0;
			if (m_requestList[foundReqIndex].m_message.isScheduled())
			{
				m_requestList[foundReqIndex].m_message.cancel();
			}
			m_requestList[foundReqIndex].m_message = LcTMessage(0,0);

			// Update m_exclusivityOwnerHandle
			if (m_exclusivityOwner)
			{
				m_exclusivityOwnerHandle = m_exclusivityOwner->m_exclusivityHandle;
			}

			// Only inform requesters of any status change after our internal state changes are complete
			if (requester1StatusChange)
			{
				m_handler->exclusivityStatusChange(requester1ExclusivityHandle, requester1NewStatus);
			}
		}

		m_requestListMutex->unlock();
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_RETURN_STATUS NdhsCExclusivity::exclusivityPermitted(
		IFX_HEXCLUSIVITY 	hExclusivity,
		IFX_INT32*				pResult)
{
	IFX_RETURN_STATUS retVal = IFX_ERROR;

	if (pResult)
	{
		// 2 cases: if either the handle has exclusivity, or nothing has exclusivity,
		// then permission is granted
		if (m_exclusivityOwnerHandle == hExclusivity || !m_exclusivityOwner)
		{
			*pResult = 1;
		}
		else
		{
			*pResult = 0;
		}

		retVal = IFX_SUCCESS;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCExclusivity::onMessage(int iID, int iParam)
{
	// Called when a timeout timer has elapsed

	volatile int memError = 0;

	LC_UNUSED(iParam)

	// Prepare cleanup frame
	LC_CLEANUP_PUSH_FRAME(memError);

	if (memError == 0)
	{
		if (m_requestListMutex->lock(LcCMutex::EInfinite))
		{
			IFX_HEXCLUSIVITY requestExclusivityHandle = (IFX_HEXCLUSIVITY)iID;

			// Find the corresponding request
			int foundRequestIndex = -1;
			int i;

			for (i=0; i<m_numModules; i++)
			{
				if(m_requestList[i].m_exclusivityHandle == requestExclusivityHandle)
				{
					foundRequestIndex = i;
				}
			}

			if (foundRequestIndex != -1)
			{
				// Remove request from list
				m_requestList[foundRequestIndex].m_priority = 0;
				m_requestList[foundRequestIndex].m_exclusivityHandle = 0;
				m_requestList[foundRequestIndex].m_message = LcTMessage(0,0);

				// Inform requester that exclusivity has been denied
				m_handler->exclusivityStatusChange(requestExclusivityHandle, IFX_EXCLUSIVITY_STATUS_DENIED);
			}

			m_requestListMutex->unlock();
		}
	}
	else
	{
		// Make sure mutex is released, and re-throw
		m_requestListMutex->unlock();
		LC_CLEANUP_THROW(memError);
	}

	// Finished with cleanup stack
	LC_CLEANUP_POP_FRAME(memError);
}
