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
#ifndef LcCMutexH
#define LcCMutexH

#include "inflexionui/engine/inc/LcCBase.h"

/*-------------------------------------------------------------------------*//**
	Mutex interface
*/
class LcCMutex : public LcCBase
{
public:
	typedef enum
	{
		EInfinite = -1
	} ETimeoutType;

private:
	IFXP_MUTEX						m_mutex;

protected:
									LcCMutex();
	bool 							construct(const LcTmString& name);

public:
	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcCMutex> create(const LcTmString& name);
	LC_VIRTUAL						~LcCMutex();
	
	// Locking / unlocking
	LC_VIRTUAL 		bool 			lock(int timeout = EInfinite);
	LC_VIRTUAL		void 			unlock();
};

#endif // LcCMutexH
