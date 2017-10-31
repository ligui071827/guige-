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
#ifndef NdhsIPathH
#define NdhsIPathH

/*-------------------------------------------------------------------------*//**
*/
class NdhsIPath
{

public:

    virtual void  getPlacement(LcTUnitScalar position, int mask, LcTPlacement& placement) = 0;

};

#endif // NdhsIPathH
