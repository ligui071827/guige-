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
#ifndef NdhsCExpressionFieldH
#define NdhsCExpressionFieldH

#include "inflexionui/engine/inc/NdhsCExpression.h"
#include "inflexionui/engine/inc/NdhsCField.h"

/*-------------------------------------------------------------------------*//**
	NdhsCExpressionField is a sub class of NdhsCExpression.
	It has field.
*/
class NdhsCExpressionField : public NdhsCExpression, public NdhsCField::IObserver
{
private:
	NdhsCField*					m_field;
	LcTmString					m_fieldName;
	bool						m_createdWithContext;

protected:
								NdhsCExpressionField()
								: 	NdhsCExpression(true)
								{}
	void						construct(const LcTmString& fieldName, NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem *item);

public:
	static LcTaOwner<NdhsCExpressionField>  create(const LcTmString& fieldName, NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem * item);

	virtual LcTaString			getValueString();

	// Evaluate the expression
	virtual ENdhsExpressionType	evaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item);

	virtual						~NdhsCExpressionField();

	// NdhsCField::IObserver callback
	// Expression can only be dirtied if originally created with a context, otherwise shared
	virtual void				fieldValueUpdated(NdhsCField* field)    { if (m_createdWithContext) expressionDirty(this); }
	virtual void 				fieldDirty(NdhsCField* field) 			{ if (m_createdWithContext) expressionDirty(this); }
	virtual void				fieldDestroyed(NdhsCField* field);

	virtual bool				isLvalue()								{ return m_createdWithContext; }
	virtual NdhsCField*			getField(NdhsIFieldContext* context=NULL,
															int slotnum=-1,
															NdhsCMenuItem* item=NULL);

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			showExpression(bool showValues) const;
	virtual void 				gatherErrorReports(LcTmString& errorOutput) const;
#endif // def NDHS_TRACE_ENABLED

#ifdef IFX_SERIALIZATION
	static	NdhsCExpressionField*	loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual	SerializeHandle			serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual	void					deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
			ISerializeable *		getSerializeAble(int &type);
#endif /* IFX_SERIALIZATION */
};

#endif // NdhsCExpressionFieldH
