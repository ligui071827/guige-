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
#ifndef NdhsCExpressionFunctionH
#define NdhsCExpressionFunctionH

#include "inflexionui/engine/inc/NdhsCExpression.h"

/*-------------------------------------------------------------------------*//**
	Base class for functional expression nodes
*/
class NdhsCExpressionFunction : public NdhsCExpression
{
protected:
	LcTmOwnerArray<NdhsCExpression>	m_parameters;
	int 							m_argc;

								NdhsCExpressionFunction() : NdhsCExpression(true)		{}

	template <class FnClass>
	static LcTaOwner<NdhsCExpressionFunction>	create(LcTmOwnerArray<NdhsCExpression>& expressionStack, LcTmString* errorOutput, int argc=0);

	bool						construct(LcTmOwnerArray<NdhsCExpression>& expressionStack, LcTmString* errorOutput);

	virtual 		void		setArgumentCount(int argc) {m_argc = argc;}

	virtual unsigned int		getNumParameters() const = 0;
	virtual void				doEvaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item) = 0;
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeNone;}

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const = 0;
	virtual bool				isInfix() const = 0;
#endif // def NDHS_TRACE_ENABLED

public:
	static LcTaOwner<NdhsCExpressionFunction>	create(CExprToken* tok, LcTmOwnerArray<NdhsCExpression>& expressionStack, LcTmString* errorOutput);

	// Evaluate the expression
	virtual ENdhsExpressionType	evaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item);

	virtual ENdhsPageState		getPageState();

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			showExpression(bool showValues) const;
	virtual void 				gatherErrorReports(LcTmString& errorOutput) const;
#endif // def NDHS_TRACE_ENABLED

	virtual						~NdhsCExpressionFunction()	{}

#ifdef IFX_SERIALIZATION
	static NdhsCExpressionFunction*	loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual		SerializeHandle		serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual		void				deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
#endif /* IFX_SERIALIZATION */
};

#endif // NdhsCExpressionFunctionH
