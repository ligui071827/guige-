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
#ifndef NdhsIExpressionObserverH
#define NdhsIExpressionObserverH

/*-------------------------------------------------------------------------*//**
*/
class NdhsCExpression;

class NdhsIExpressionObserver
{
public:
	virtual void 					expressionDirty(NdhsCExpression* expr) = 0;
	virtual void 					expressionDestroyed(NdhsCExpression* expr) = 0; // Broadcast this from destructor
	virtual SerializeHandle			serialize(LcCSerializeMaster *serializeMaster,bool force=false){return -1;}
#ifdef IFX_SERIALIZATION
	virtual ISerializeable*			getSerializeAble(int &type) = 0;
	static  NdhsIExpressionObserver* loadState(int type,SerializeHandle h,LcCSerializeMaster * serializeMaster);
#endif /* IFX_SERIALIZATION */
};

#endif // NdhsIExpressionObserverH
