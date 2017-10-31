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

#ifndef SimpleEditMultiTapH
#define SimpleEditMultiTapH

#ifdef __cplusplus
extern "C" {
#endif

IFX_RETURN_STATUS	SimpleEdit_ProcessElementMultiTapKeyEvent(	SIMPLE_EDIT_SESSION* 	pSession,
																SIMPLE_EDIT*			pElement, 
																IFX_INT32				key,
																IFX_INT32*				pConsumed,
																IFX_INT32*				pCallback);

#ifdef __cplusplus
}
#endif
#endif /* SimpleEditMultiTapH */
