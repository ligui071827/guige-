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

#ifndef NdhsCExclusivityH
#define NdhsCExclusivityH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTMessage.h"

class LcCMutex;

/*-------------------------------------------------------------------------*//**
*/
class NdhsCExclusivity : public LcCBase, public LcTMessage::IHandler
{
public:
	// Interface for status change callback
	class IExclusivityHandler
	{
	public:
		virtual IFX_RETURN_STATUS		exclusivityStatusChange(
														IFX_HEXCLUSIVITY 		hExclusivity,
														IFX_EXCLUSIVITY_STATUS 	status) = 0;
	};

private:
	// Private structure to hold details of each request
	class TRequestInfo
	{
	public:
		IFX_UINT32						m_priority;
		IFX_HEXCLUSIVITY				m_exclusivityHandle;
		LcTMessage						m_message;

										TRequestInfo() : m_message(0,0) {m_priority = 0; m_exclusivityHandle=0;}
	};

private:
	IExclusivityHandler* 				m_handler;
	LcCSpace*							m_space;
	int									m_numModules;

	LcTmOwner<LcCMutex>					m_requestListMutex;

	LcTmAlloc<TRequestInfo>				m_requestList;

	TRequestInfo*						m_exclusivityOwner;
	IFX_HEXCLUSIVITY					m_exclusivityOwnerHandle;

protected:
										NdhsCExclusivity(IExclusivityHandler* pHandler,
														LcCSpace* pSpace);
	void 								construct(int numModules);

public:

	// Create/destruct
	static LcTaOwner<NdhsCExclusivity> 	create(IExclusivityHandler* pHandler,
														LcCSpace* pSpace,
														int numModules);
	virtual								~NdhsCExclusivity();

	IFX_RETURN_STATUS					requestExclusivity(
														IFX_HEXCLUSIVITY 	hExclusivity,
														IFX_UINT32			priority,
														IFX_UINT32			timeout);

	IFX_RETURN_STATUS					releaseExclusivity(
														IFX_HEXCLUSIVITY 	hExclusivity);

	IFX_RETURN_STATUS					exclusivityPermitted(
														IFX_HEXCLUSIVITY 	hExclusivity,
														IFX_INT32*			pResult);

	// Implements LcTMessage::IHandler
	virtual void 						onMessage(int iID, int iParam);
};

#endif /* NdhsCExclusivityH */
