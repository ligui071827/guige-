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


#include <math.h>

/*-------------------------------------------------------------------------*//**
	General base class for unary (1-parameter) functions
	Parameters will be subject to normal casting rules, based on type support.
*/
class NdhsCExpressionGeneralUnaryFunction : public NdhsCExpressionFunction
{
private:
	bool 						m_supportsBools;
	bool 						m_supportsInts;
	bool 						m_supportsScalars;
	bool 						m_supportsStrings;

protected:
								NdhsCExpressionGeneralUnaryFunction(bool supportsBools, bool supportsInts, bool supportsScalars, bool supportsStrings)
								:	m_supportsBools(supportsBools),
									m_supportsInts(supportsInts),
									m_supportsScalars(supportsScalars),
									m_supportsStrings(supportsStrings)
									{}

	virtual unsigned int		getNumParameters() const 					{ return 1; }

	virtual void				doEvaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item);

	virtual void				doEvaluateBool(bool param)					{ setValueError(ENdhsExpressionErrorInternal); }
	virtual void				doEvaluateInt(int param)					{ setValueError(ENdhsExpressionErrorInternal); }
	virtual void				doEvaluateScalar(LcTScalar param)			{ setValueError(ENdhsExpressionErrorInternal); }
	virtual void				doEvaluateString(const LcTmString& param)	{ setValueError(ENdhsExpressionErrorInternal); }

public:

	virtual						~NdhsCExpressionGeneralUnaryFunction()	{}
};

/*-------------------------------------------------------------------------*//**
	General base class for binary (2-parameter) functions.
	Parameters will be subject to normal casting rules, based on type support.
	Both parameters must be of same type, or castable to the same type.
*/
class NdhsCExpressionGeneralBinaryFunction : public NdhsCExpressionFunction
{
private:
	bool 						m_supportsBools;
	bool 						m_supportsInts;
	bool 						m_supportsScalars;
	bool 						m_supportsStrings;

protected:
								NdhsCExpressionGeneralBinaryFunction(bool supportsBools, bool supportsInts, bool supportsScalars, bool supportsStrings)
								:	m_supportsBools(supportsBools),
									m_supportsInts(supportsInts),
									m_supportsScalars(supportsScalars),
									m_supportsStrings(supportsStrings)
									{}

	virtual unsigned int		getNumParameters() const 					{ return 2; }

	virtual void				doEvaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item);

	virtual void				doEvaluateBool(bool leftParam, bool rightParam)								{ setValueError(ENdhsExpressionErrorInternal); }
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueError(ENdhsExpressionErrorInternal); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueError(ENdhsExpressionErrorInternal); }
	virtual void				doEvaluateString(const LcTmString& leftParam, const LcTmString& rightParam)	{ setValueError(ENdhsExpressionErrorInternal); }

	// Lazy evaluation support: return true to stop further evaluation
	virtual bool				stopEvaluationOnBool(bool leftValue)										{ return false; }

public:

	virtual						~NdhsCExpressionGeneralBinaryFunction()	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function IF, ?:
*/
class NdhsCExpressionFnIf : public NdhsCExpressionFunction
{
protected:
	virtual unsigned int		getNumParameters() const			{ return 3; }
	virtual void				doEvaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item);

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const 					{ return "?:"; } // This function a special case, and we override showExpression()
	virtual bool				isInfix() const						{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeIf;}

public:
	virtual void 				expressionDirty(NdhsCExpression* expr);

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString 			showExpression(bool showValues) const;
#endif // def NDHS_TRACE_ENABLED
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function unary -
*/
class NdhsCExpressionFnUnaryMinus : public NdhsCExpressionGeneralUnaryFunction
{
protected:
	virtual void				doEvaluateInt(int param)					{ setValueInt(-param); }
	virtual void				doEvaluateScalar(LcTScalar param)			{ setValueScalar(-param); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const								{ return "-"; }
	virtual bool				isInfix() const								{ return true; }
#endif // def NDHS_TRACE_ENABLED
		virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeUnaryMinus;}

public:
								NdhsCExpressionFnUnaryMinus() : NdhsCExpressionGeneralUnaryFunction(false, true, true, false)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function NOT, !
*/
class NdhsCExpressionFnNot : public NdhsCExpressionGeneralUnaryFunction
{
protected:
	virtual void				doEvaluateBool(bool param)					{ setValueBool(!param); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const								{ return "!"; }
	virtual bool				isInfix() const								{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeNot;}
public:
								NdhsCExpressionFnNot() : NdhsCExpressionGeneralUnaryFunction(true, false, false, false)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function FLOOR
*/
class NdhsCExpressionFnFloor : public NdhsCExpressionGeneralUnaryFunction
{
protected:
	virtual void				doEvaluateInt(int param)					{ setValueScalar((LcTScalar)param); }
	virtual void				doEvaluateScalar(LcTScalar param)			{ setValueScalar(lc_floor(param)); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const								{ return "FLOOR"; }
	virtual bool				isInfix() const								{ return false; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeFloor;}

public:
								NdhsCExpressionFnFloor() : NdhsCExpressionGeneralUnaryFunction(false, true, true, false)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function CEIL
*/
class NdhsCExpressionFnCeil : public NdhsCExpressionGeneralUnaryFunction
{
protected:
	virtual void				doEvaluateInt(int param)					{ setValueScalar((LcTScalar)param); }
	virtual void				doEvaluateScalar(LcTScalar param)			{ setValueScalar(lc_ceil(param)); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const								{ return "CEIL"; }
	virtual bool				isInfix() const								{ return false; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeCeil;}

public:
								NdhsCExpressionFnCeil() : NdhsCExpressionGeneralUnaryFunction(false, true, true, false)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function ==
*/
class NdhsCExpressionFnEqual : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateBool(bool leftParam, bool rightParam)								{ setValueBool(leftParam == rightParam); }
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueBool(leftParam == rightParam); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueBool(leftParam == rightParam); }
	virtual void				doEvaluateString(const LcTmString& leftParam, const LcTmString& rightParam)	{ setValueBool(leftParam.compareNoCase(rightParam) == 0); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "=="; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeEqual;}
public:
								NdhsCExpressionFnEqual() : NdhsCExpressionGeneralBinaryFunction(true, true, true, true)	{}

	virtual ENdhsPageState		getPageState();
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function !=
*/
class NdhsCExpressionFnNotEqual : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateBool(bool leftParam, bool rightParam)								{ setValueBool(leftParam != rightParam); }
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueBool(leftParam != rightParam); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueBool(leftParam != rightParam); }
	virtual void				doEvaluateString(const LcTmString& leftParam, const LcTmString& rightParam)	{ setValueBool(leftParam.compareNoCase(rightParam) != 0); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "!="; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeNotEqual;}

public:
								NdhsCExpressionFnNotEqual() : NdhsCExpressionGeneralBinaryFunction(true, true, true, true)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function <
*/
class NdhsCExpressionFnLessThan : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueBool(leftParam < rightParam); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueBool(leftParam < rightParam); }
	virtual void				doEvaluateString(const LcTmString& leftParam, const LcTmString& rightParam)	{ setValueBool(leftParam.compareNoCase(rightParam) < 0); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "<"; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeLessThan;}

public:
								NdhsCExpressionFnLessThan() : NdhsCExpressionGeneralBinaryFunction(false, true, true, true)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function <=
*/
class NdhsCExpressionFnLessThanEqual : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueBool(leftParam <= rightParam); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueBool(leftParam <= rightParam); }
	virtual void				doEvaluateString(const LcTmString& leftParam, const LcTmString& rightParam)	{ setValueBool(leftParam.compareNoCase(rightParam) <= 0); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "<="; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeLessThanEqual;}

public:
								NdhsCExpressionFnLessThanEqual() : NdhsCExpressionGeneralBinaryFunction(false, true, true, true)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function >
*/
class NdhsCExpressionFnGreaterThan : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueBool(leftParam > rightParam); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueBool(leftParam > rightParam); }
	virtual void				doEvaluateString(const LcTmString& leftParam, const LcTmString& rightParam)	{ setValueBool(leftParam.compareNoCase(rightParam) > 0); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return ">"; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeGreaterThan;}

public:
								NdhsCExpressionFnGreaterThan() : NdhsCExpressionGeneralBinaryFunction(false, true, true, true)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function >=
*/
class NdhsCExpressionFnGreaterThanEqual : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueBool(leftParam >= rightParam); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueBool(leftParam >= rightParam); }
	virtual void				doEvaluateString(const LcTmString& leftParam, const LcTmString& rightParam)	{ setValueBool(leftParam.compareNoCase(rightParam) >= 0); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return ">="; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeGreaterThanEqual;}

public:
								NdhsCExpressionFnGreaterThanEqual() : NdhsCExpressionGeneralBinaryFunction(false, true, true, true)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function +
*/
class NdhsCExpressionFnAdd : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueInt(leftParam + rightParam); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueScalar(leftParam + rightParam); }
	virtual void				doEvaluateString(const LcTmString& leftParam, const LcTmString& rightParam)	{ setValueString(leftParam + rightParam); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "+"; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeAdd;}

public:
								NdhsCExpressionFnAdd() : NdhsCExpressionGeneralBinaryFunction(false, true, true, true)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function -
*/
class NdhsCExpressionFnSubtract : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueInt(leftParam - rightParam); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueScalar(leftParam - rightParam); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "-"; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeSubtract;}

public:
								NdhsCExpressionFnSubtract() : NdhsCExpressionGeneralBinaryFunction(false, true, true, false)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function *
*/
class NdhsCExpressionFnMultiply : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueInt(leftParam * rightParam); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueScalar(leftParam * rightParam); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "*"; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeMultiply;}

public:
								NdhsCExpressionFnMultiply() : NdhsCExpressionGeneralBinaryFunction(false, true, true, false)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function /
*/
class NdhsCExpressionFnDivide : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ rightParam == 0 ? setValueError(ENdhsExpressionErrorDivideByZero) : setValueScalar(leftParam / rightParam); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "/"; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeDivide;}

public:
								NdhsCExpressionFnDivide() : NdhsCExpressionGeneralBinaryFunction(false, false, true, false)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function POW
*/
class NdhsCExpressionFnExponent : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueScalar(lc_pow(leftParam, rightParam)); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "POW"; }
	virtual bool				isInfix() const																{ return false; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypePow;}

public:
								NdhsCExpressionFnExponent() : NdhsCExpressionGeneralBinaryFunction(false, false, true, false)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function AND, &&
*/
class NdhsCExpressionFnAnd : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateBool(bool leftParam, bool rightParam)								{ setValueBool(leftParam && rightParam); }
	virtual bool				stopEvaluationOnBool(bool leftValue)										{ if (!leftValue) setValueBool(false); return !leftValue; }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "&&"; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeAnd;}

public:
								NdhsCExpressionFnAnd() : NdhsCExpressionGeneralBinaryFunction(true, false, false, false)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function toString
*/
class NdhsCExpressionFnToString : public NdhsCExpressionFunction
{
protected:
	virtual unsigned int		getNumParameters() const			{ return 2; }
	virtual bool				doEvaluateStringFromScalar(LcTScalar leftParam, LcTaString format);
	virtual bool				doEvaluateStringFromInt(int param, LcTaString format);
	virtual bool 				doEvaluateStringFromTime(IFX_TIME leftParam, LcTaString format);
	virtual void				doEvaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item);
	ENdhsExpressionType			formatForExpressionType(LcTaString &format);

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "&&"; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeToString;}

public:
	NdhsCExpressionFnToString() : NdhsCExpressionFunction()	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function OR, ||
*/
class NdhsCExpressionFnOr : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateBool(bool leftParam, bool rightParam)								{ setValueBool(leftParam || rightParam); }
	virtual bool				stopEvaluationOnBool(bool leftValue)										{ if (leftValue) setValueBool(true); return leftValue; }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "||"; }
	virtual bool				isInfix() const																{ return true; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeOr;}

public:
								NdhsCExpressionFnOr() : NdhsCExpressionGeneralBinaryFunction(true, false, false, false)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function MIN
*/
class NdhsCExpressionFnMin : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueInt(min(leftParam, rightParam)); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueScalar(min(leftParam, rightParam)); }
	virtual void				doEvaluateString(const LcTmString& leftParam, const LcTmString& rightParam)	{ setValueString(min(leftParam, rightParam)); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "MIN"; }
	virtual bool				isInfix() const																{ return false; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeMin;}

public:
								NdhsCExpressionFnMin() : NdhsCExpressionGeneralBinaryFunction(false, true, true, true)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function MAX
*/
class NdhsCExpressionFnMax : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ setValueInt(max(leftParam, rightParam)); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ setValueScalar(max(leftParam, rightParam)); }
	virtual void				doEvaluateString(const LcTmString& leftParam, const LcTmString& rightParam)	{ setValueString(max(leftParam, rightParam)); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "MAX"; }
	virtual bool				isInfix() const																{ return false; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeMax;}

public:
								NdhsCExpressionFnMax() : NdhsCExpressionGeneralBinaryFunction(false, true, true, true)	{}
};

/*-------------------------------------------------------------------------*//**
	Implementation of expression function MOD, %
*/
class NdhsCExpressionFnMod : public NdhsCExpressionGeneralBinaryFunction
{
protected:
	virtual void				doEvaluateInt(int leftParam, int rightParam)								{ rightParam == 0 ? setValueError(ENdhsExpressionErrorModByZero) : setValueInt(leftParam % rightParam); }
	virtual void				doEvaluateScalar(LcTScalar leftParam, LcTScalar rightParam)					{ rightParam == 0 ? setValueError(ENdhsExpressionErrorModByZero) : setValueScalar(lc_fmod(leftParam, rightParam)); }

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString			getName() const																{ return "MOD"; }
	virtual bool				isInfix() const																{ return false; }
#endif // def NDHS_TRACE_ENABLED
	virtual ENdhsExpressionFunctionType getFunctionType(){ return ENdhsExpressionFunctionTypeMod;}

public:
								NdhsCExpressionFnMod() : NdhsCExpressionGeneralBinaryFunction(false, true, true, false)	{}
};


/*-------------------------------------------------------------------------*//**
*/
template <class FnClass>
LcTaOwner<NdhsCExpressionFunction> NdhsCExpressionFunction::create(LcTmOwnerArray<NdhsCExpression>& expressionStack, LcTmString* errorOutput, int argc)
{
	LcTaOwner<NdhsCExpressionFunction> ref;
	ref.set(new FnClass());

	ref->setArgumentCount(argc);
	// NB construct may fail!
	if (!ref->construct(expressionStack, errorOutput))
	{
		// Construct failed
		ref.destroy();
	}

	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpressionFunction>	NdhsCExpressionFunction::create(CExprToken* tok, LcTmOwnerArray<NdhsCExpression>& expressionStack, LcTmString* errorOutput)
{
	LcTaOwner<NdhsCExpressionFunction> retVal;

	if (tok->getOperator() != ENdhsTokenOperatorNone)
	{
		switch (tok->getOperator())
		{
			case ENdhsTokenOperatorEqual:				retVal = create<NdhsCExpressionFnEqual>(expressionStack, errorOutput);				break;
			case ENdhsTokenOperatorNotEqual:			retVal = create<NdhsCExpressionFnNotEqual>(expressionStack, errorOutput);			break;
			case ENdhsTokenOperatorLessThan:			retVal = create<NdhsCExpressionFnLessThan>(expressionStack, errorOutput);			break;
			case ENdhsTokenOperatorLessThanEqual:		retVal = create<NdhsCExpressionFnLessThanEqual>(expressionStack, errorOutput);		break;
			case ENdhsTokenOperatorGreaterThan:			retVal = create<NdhsCExpressionFnGreaterThan>(expressionStack, errorOutput);		break;
			case ENdhsTokenOperatorGreaterThanEqual:	retVal = create<NdhsCExpressionFnGreaterThanEqual>(expressionStack, errorOutput);	break;
			case ENdhsTokenOperatorAdd:					retVal = create<NdhsCExpressionFnAdd>(expressionStack, errorOutput);				break;
			case ENdhsTokenOperatorSubtract:			retVal = create<NdhsCExpressionFnSubtract>(expressionStack, errorOutput);			break;
			case ENdhsTokenOperatorMultiply:			retVal = create<NdhsCExpressionFnMultiply>(expressionStack, errorOutput);			break;
			case ENdhsTokenOperatorDivide:				retVal = create<NdhsCExpressionFnDivide>(expressionStack, errorOutput);				break;
			case ENdhsTokenOperatorModulus:				retVal = create<NdhsCExpressionFnMod>(expressionStack, errorOutput);				break;
			case ENdhsTokenOperatorUnaryMinus:			retVal = create<NdhsCExpressionFnUnaryMinus>(expressionStack, errorOutput);  		break;
			case ENdhsTokenOperatorAnd:					retVal = create<NdhsCExpressionFnAnd>(expressionStack, errorOutput);				break;
			case ENdhsTokenOperatorOr:					retVal = create<NdhsCExpressionFnOr>(expressionStack, errorOutput);					break;
			case ENdhsTokenOperatorNot:					retVal = create<NdhsCExpressionFnNot>(expressionStack, errorOutput);				break;
			case ENdhsTokenOperatorIf:					retVal = create<NdhsCExpressionFnIf>(expressionStack, errorOutput);					break;
			default:
				break;
		}
	}
	else
	{
		switch (tok->getFnType())
		{
			case ENdhsExprTokenFnAnd:					retVal = create<NdhsCExpressionFnAnd>(expressionStack, errorOutput);				break;
			case ENdhsExprTokenFnOr:					retVal = create<NdhsCExpressionFnOr>(expressionStack, errorOutput);					break;
			case ENdhsExprTokenFnNot:					retVal = create<NdhsCExpressionFnNot>(expressionStack, errorOutput);				break;
			case ENdhsExprTokenFnIf:					retVal = create<NdhsCExpressionFnIf>(expressionStack, errorOutput);					break;
			case ENdhsExprTokenFnToString:				retVal = create<NdhsCExpressionFnToString>(expressionStack, errorOutput,
																					((CExprTokenFunctionName*)tok)->getArgCount());			break;
			case ENdhsExprTokenFnMin:					retVal = create<NdhsCExpressionFnMin>(expressionStack, errorOutput);				break;
			case ENdhsExprTokenFnMax:					retVal = create<NdhsCExpressionFnMax>(expressionStack, errorOutput);				break;
			case ENdhsExprTokenFnMod:					retVal = create<NdhsCExpressionFnMod>(expressionStack, errorOutput);				break;
			case ENdhsExprTokenFnPow:					retVal = create<NdhsCExpressionFnExponent>(expressionStack, errorOutput);			break;
			case ENdhsExprTokenFnFloor:					retVal = create<NdhsCExpressionFnFloor>(expressionStack, errorOutput);				break;
			case ENdhsExprTokenFnCeil:					retVal = create<NdhsCExpressionFnCeil>(expressionStack, errorOutput);				break;
			default:
				break;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCExpressionFunction::construct(LcTmOwnerArray<NdhsCExpression>& expressionStack, LcTmString* errorOutput)
{
	bool success = false;

	if (expressionStack.size() >= getNumParameters())
	{
		LcTaAuto< LcTmOwnerArray<NdhsCExpression> > tempStack;

		for (int i = getNumParameters(); i > 0; i--)
		{
			tempStack.push_back(expressionStack.release_back());
		}

		for (int j = getNumParameters(); j > 0; j--)
		{
			m_parameters.push_back(tempStack.release_back());
			m_parameters.back()->setObserver(this);
		}

		success = true;
	}
	else
	{
		reportError("Wrong number of parameters in expression", errorOutput);
	}

	return success;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCExpression::ENdhsExpressionType NdhsCExpressionFunction::evaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item)
{
	if (fieldContext)
	{
		doEvaluate(fieldContext, slotNum, item);

		if (isDirty())
		{
			setDirty(false);
		}
	}
	else
	{
		if (isDirty())
		{
			doEvaluate(NULL, -1, NULL);
			setDirty(false);
		}
	}

	return expressionType();
}

#ifdef NDHS_TRACE_ENABLED
/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCExpressionFunction::showExpression(bool showValues) const
{
	LcTaString retVal;
	int numParams = getNumParameters();

	if (isInfix() && numParams == 1)
	{
		retVal = getName() + m_parameters[0]->showExpression(showValues);
	}
	else if (isInfix() && numParams == 2)
	{
		retVal = "(" + m_parameters[0]->showExpression(showValues) + " " + getName() + " " + m_parameters[1]->showExpression(showValues) + ")";
	}
	else
	{
		retVal = getName() + "(";
		int currentParam = 0;

		while (currentParam < numParams)
		{
			retVal += m_parameters[currentParam]->showExpression(showValues);
			currentParam++;

			if (currentParam < numParams - 1)
			{
				retVal += ", ";
			}
		}

		retVal += ")";
	}

	return retVal;
}
#endif // def NDHS_TRACE_ENABLED

#ifdef NDHS_TRACE_ENABLED
/*-------------------------------------------------------------------------*//**
*/
void NdhsCExpressionFunction::gatherErrorReports(LcTmString& errorOutput) const
{
	// Find parameter errors first
	LcTmOwnerArray<NdhsCExpression>::const_iterator it = m_parameters.begin();

	while (it != m_parameters.end())
	{
		(*it)->gatherErrorReports(errorOutput);
		it++;
	}

	LcTaString errorDescription;
	switch (errorType())
	{
		case ENdhsExpressionErrorDivideByZero:
		{
			errorDescription = "Divide by zero";
		}
		break;

		case ENdhsExpressionErrorModByZero:
		{
			errorDescription = "Mod by zero";
		}
		break;

		case ENdhsExpressionErrorTypeMismatch:
		{
			if (isInfix())
			{
				errorDescription = "Type mismatch in operator \"" + getName() + "\"";
			}
			else
			{
				errorDescription = "Type mismatch in function \"" + getName() + "\"";
			}
		}
		break;

		default:
			break;
	}

	if (!errorDescription.isEmpty())
	{
		if (errorOutput.isEmpty())
		{
			errorOutput = errorDescription;
		}
		else
		{
			errorOutput += "; " + errorDescription;
		}
	}
}
#endif // def NDHS_TRACE_ENABLED

/*-------------------------------------------------------------------------*//**
*/
#ifdef IFX_SERIALIZATION

NdhsCExpressionFunction* NdhsCExpressionFunction::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	NdhsCExpressionFunction*obj=NULL;
	SerializeHandle parentHandle=-1;

	ENdhsExpressionFunctionType classType;
	DESERIALIZE(classType,serializeMaster,cPtr)
	DESERIALIZE(parentHandle,serializeMaster,cPtr)
	DESERIALIZE(classType,serializeMaster,cPtr)
	switch(classType)
	{
		case  ENdhsExpressionFunctionTypeEqual:				obj = new NdhsCExpressionFnEqual();				break;
		case  ENdhsExpressionFunctionTypeNotEqual:			obj = new NdhsCExpressionFnNotEqual();			break;
		case  ENdhsExpressionFunctionTypeLessThan:			obj = new NdhsCExpressionFnLessThan();			break;
		case  ENdhsExpressionFunctionTypeLessThanEqual:		obj = new NdhsCExpressionFnLessThanEqual();		break;
		case  ENdhsExpressionFunctionTypeGreaterThan:		obj = new NdhsCExpressionFnGreaterThan();		break;
		case  ENdhsExpressionFunctionTypeGreaterThanEqual:	obj = new NdhsCExpressionFnGreaterThanEqual();	break;
		case  ENdhsExpressionFunctionTypeAdd:				obj = new NdhsCExpressionFnAdd();				break;
		case  ENdhsExpressionFunctionTypeSubtract:			obj = new NdhsCExpressionFnSubtract();			break;
		case  ENdhsExpressionFunctionTypeMultiply:			obj = new NdhsCExpressionFnMultiply();			break;
		case  ENdhsExpressionFunctionTypeDivide:			obj = new NdhsCExpressionFnDivide();			break;
		case  ENdhsExpressionFunctionTypeMod:				obj = new NdhsCExpressionFnMod();				break;
		case  ENdhsExpressionFunctionTypeUnaryMinus:		obj = new NdhsCExpressionFnUnaryMinus();  		break;
		case  ENdhsExpressionFunctionTypeAnd:				obj = new NdhsCExpressionFnAnd();				break;
		case  ENdhsExpressionFunctionTypeOr:				obj = new NdhsCExpressionFnOr();				break;
		case  ENdhsExpressionFunctionTypeNot:				obj = new NdhsCExpressionFnNot();				break;
		case  ENdhsExpressionFunctionTypeIf:				obj = new NdhsCExpressionFnIf();				break;
		case  ENdhsExpressionFunctionTypeMin:				obj = new NdhsCExpressionFnMin();				break;
		case  ENdhsExpressionFunctionTypeMax:				obj = new NdhsCExpressionFnMax();				break;
		case  ENdhsExpressionFunctionTypePow:				obj = new NdhsCExpressionFnExponent();			break;
		case  ENdhsExpressionFunctionTypeFloor:				obj = new NdhsCExpressionFnFloor();				break;
		case  ENdhsExpressionFunctionTypeCeil:				obj = new NdhsCExpressionFnCeil();				break;
		case  ENdhsExpressionFunctionTypeToString:			obj = new NdhsCExpressionFnToString();			break;
		default:;
	}
	obj->deSerialize(handle,serializeMaster);
	serializeMaster->setPointer(handle,obj);
	return obj;
}

SerializeHandle	NdhsCExpressionFunction::serialize(LcCSerializeMaster *serializeMaster,bool force)
{
	SerializeHandle handle=-1;
	if(!force)
	{
		handle=serializeMaster->getHandle(this);
		if(handle!=-1 && serializeMaster->isSerialized(handle))
		{
			return handle;
		}
		else if(handle==-1)
		{
			handle=serializeMaster->newHandle(this);
		}
	}
	else
	{
		handle=serializeMaster->newHandle(this);
	}

	int outputSize = sizeof(NdhsCExpressionFunction)+sizeof(IFX_INT32)*3;
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;
	SerializeHandle parentHandle= NdhsCExpression::serialize(serializeMaster,true);
	EExpressionClassType classType=EExpressionClassTypeFunction;
	SERIALIZE(classType,serializeMaster,cPtr)
	SERIALIZE(parentHandle,serializeMaster,cPtr)
	ENdhsExpressionFunctionType typ=getFunctionType();
	SERIALIZE(typ,serializeMaster,cPtr)
	SERIALIZE_Array(m_parameters,serializeMaster,cPtr)
	SERIALIZE(m_argc,serializeMaster,cPtr)

	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}
void	NdhsCExpressionFunction::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	SerializeHandle parentHandle=-1;
	EExpressionClassType classType;
	DESERIALIZE(classType,serializeMaster,cPtr)
	DESERIALIZE(parentHandle,serializeMaster,cPtr)
	NdhsCExpression::deSerialize(parentHandle,serializeMaster);
	DESERIALIZE(classType,serializeMaster,cPtr)
	DESERIALIZE_Array(m_parameters,serializeMaster,cPtr);
	DESERIALIZE(m_argc,serializeMaster,cPtr)
}
#endif /* IFX_SERIALIZATION */

void NdhsCExpressionGeneralBinaryFunction::doEvaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item)
{
	NdhsCExpression* leftParam = m_parameters[0];
	NdhsCExpression* rightParam = m_parameters[1];

	if (leftParam && rightParam)
	{
		ENdhsExpressionType leftType = leftParam->evaluate(fieldContext, slotNum, item);
		bool stopEvaluation = false;

		// Support for lazy evaluation - currently only bools supported
		if (ENdhsExpressionTypeBool == leftType)
		{
			stopEvaluation = stopEvaluationOnBool(leftParam->getValueBool());
		}
		else if (ENdhsExpressionTypeError == leftType)
		{
			setValueError(ENdhsExpressionErrorPropagated);
			stopEvaluation = true;
		}

		if (!stopEvaluation)
		{
			ENdhsExpressionType rightType = rightParam->evaluate(fieldContext, slotNum, item);
			bool canConvertRightTypeToString = (rightType == ENdhsExpressionTypeBool   || rightType == ENdhsExpressionTypeInt
											 || rightType == ENdhsExpressionTypeScalar || rightType == ENdhsExpressionTypeString);

			if (ENdhsExpressionTypeError == rightType)
			{
				setValueError(ENdhsExpressionErrorPropagated);
			}
			else
			{
				switch (leftType)
				{
					case ENdhsExpressionTypeBool:
					{
						if (m_supportsBools && rightType == ENdhsExpressionTypeBool)
						{
							doEvaluateBool(leftParam->getValueBool(), rightParam->getValueBool());
						}
						else if (m_supportsStrings && canConvertRightTypeToString)
						{
							doEvaluateString(leftParam->getValueString(), rightParam->getValueString());
						}
						else
						{
							setValueError(ENdhsExpressionErrorTypeMismatch);
						}
					}
					break;

					case ENdhsExpressionTypeInt:
					{
						if (m_supportsInts && rightType == ENdhsExpressionTypeInt)
						{
							doEvaluateInt(leftParam->getValueInt(), rightParam->getValueInt());
						}
						else if (m_supportsInts && rightType == ENdhsExpressionTypeScalar)
						{
							if (m_supportsScalars)
							{
								doEvaluateScalar(leftParam->getValueScalar(), rightParam->getValueScalar());
							}
							else
							{
								doEvaluateInt(leftParam->getValueInt(), rightParam->getValueInt());
							}
						}
						else if (m_supportsScalars && (rightType == ENdhsExpressionTypeInt || rightType == ENdhsExpressionTypeScalar))
						{
							doEvaluateScalar(leftParam->getValueScalar(), rightParam->getValueScalar());
						}
						else if (m_supportsStrings && canConvertRightTypeToString)
						{
							doEvaluateString(leftParam->getValueString(), rightParam->getValueString());
						}
						else
						{
							setValueError(ENdhsExpressionErrorTypeMismatch);
						}
					}
					break;

					case ENdhsExpressionTypeScalar:
					{
						if (m_supportsScalars && (rightType == ENdhsExpressionTypeInt || rightType == ENdhsExpressionTypeScalar))
						{
							doEvaluateScalar(leftParam->getValueScalar(), rightParam->getValueScalar());
						}
						else if (m_supportsInts && (rightType == ENdhsExpressionTypeInt || rightType == ENdhsExpressionTypeScalar))
						{
							doEvaluateInt(leftParam->getValueInt(), rightParam->getValueInt());
						}
						else if (m_supportsStrings && canConvertRightTypeToString)
						{
							doEvaluateString(leftParam->getValueString(), rightParam->getValueString());
						}
						else
						{
							setValueError(ENdhsExpressionErrorTypeMismatch);
						}
					}
					break;

					case ENdhsExpressionTypeString:
					{
						if (m_supportsStrings && canConvertRightTypeToString)
						{
							doEvaluateString(leftParam->getValueString(), rightParam->getValueString());
						}
						else
						{
							setValueError(ENdhsExpressionErrorTypeMismatch);
						}
					}
					break;

					default:
					{
						setValueError(ENdhsExpressionErrorInternal);
					}
					break;
				}
			}
		}
	}
	else
	{
		setValueError(ENdhsExpressionErrorInternal);
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCExpressionGeneralUnaryFunction::doEvaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item)
{
	NdhsCExpression* param = m_parameters[0];

	if (param)
	{
		ENdhsExpressionType paramType = param->evaluate(fieldContext, slotNum, item);

		switch (paramType)
		{
			case ENdhsExpressionTypeBool:
			{
				if (m_supportsBools)
				{
					doEvaluateBool(param->getValueBool());
				}
				else
				{
					setValueError(ENdhsExpressionErrorTypeMismatch);
				}
			}
			break;

			case ENdhsExpressionTypeInt:
			{
				if (m_supportsInts)
				{
					doEvaluateInt(param->getValueInt());
				}
				else if (m_supportsScalars)
				{
					doEvaluateScalar(param->getValueScalar());
				}
				else
				{
					setValueError(ENdhsExpressionErrorTypeMismatch);
				}
			}
			break;

			case ENdhsExpressionTypeScalar:
			{
				if (m_supportsScalars)
				{
					doEvaluateScalar(param->getValueScalar());
				}
				else if (m_supportsInts)
				{
					doEvaluateInt(param->getValueInt());
				}
				else
				{
					setValueError(ENdhsExpressionErrorTypeMismatch);
				}
			}
			break;

			case ENdhsExpressionTypeString:
			{
				if (m_supportsStrings)
				{
					doEvaluateString(param->getValueString());
				}
				else
				{
					setValueError(ENdhsExpressionErrorTypeMismatch);
				}
			}
			break;

			case ENdhsExpressionTypeError:
			{
				setValueError(ENdhsExpressionErrorPropagated);
			}
			break;

			default:
			{
				setValueError(ENdhsExpressionErrorInternal);
			}
			break;
		}
	}
	else
	{
		setValueError(ENdhsExpressionErrorInternal);
	}
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCExpressionFnToString::doEvaluateStringFromInt(int param, LcTaString format)
{
	char arr[256] = {0};
	if(lc_snprintf(arr, 255, format.bufUtf8(), param) >= 0)
	{
		setValueString(LcTaString("").fromBufUtf8(arr,lc_strlen(arr)));
		return true;
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCExpressionFnToString::doEvaluateStringFromScalar(LcTScalar leftParam, LcTaString format)
{
	char arr[256] = {0};
	if(lc_snprintf(arr, 255, format.bufUtf8(), leftParam) >= 0)
	{
		setValueString(LcTaString("").fromBufUtf8(arr,lc_strlen(arr)));
		return true;
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCExpressionFnToString::doEvaluateStringFromTime(IFX_TIME leftParam, LcTaString format)
{
	LcTaString str = "";
	str.fromTime(leftParam,format);
	if(!str.isEmpty())
	{
		setValueString(str);
		return true;
	}
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCExpression::ENdhsExpressionType NdhsCExpressionFnToString::formatForExpressionType(LcTaString &format)
{
	if(format.find("time(",0) == 0)
	{
		format=format.subString(5,format.length()-6);
		return ENdhsExpressionTypeTime;
	}
	else if(format.find("(",0) == 0)
	{
		format=format.subString(1,format.length()-2);
		return ENdhsExpressionTypeInt;
	}

	return ENdhsExpressionTypeUnset;
}
/*-------------------------------------------------------------------------*//**
*/
void NdhsCExpressionFnToString::doEvaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item)
{
	if(m_parameters.size()!=2)
	{
		setValueError(ENdhsExpressionErrorInternal);
		return;
	}

	NdhsCExpression* fieldorExpression = m_parameters[0];
	NdhsCExpression* secondParam = NULL;

	LcTaString format = "";

	secondParam = m_parameters[1];

	if (fieldorExpression)
	{
		fieldorExpression->evaluate(fieldContext, slotNum, item);
		ENdhsExpressionType type= fieldorExpression->expressionType();
		bool result = true;
		if (secondParam)
		{
			secondParam->evaluate(fieldContext, slotNum, item);
			ENdhsExpressionType secondParamType= secondParam->expressionType();
			switch (secondParamType)
			{
				case ENdhsExpressionTypeString:
				{
					format=secondParam->getValueString();
				}
				break;
				case ENdhsExpressionTypeError:
				{
					setValueError(ENdhsExpressionErrorTypeMismatch);
					return;
				}
				break;
				default:
				{
					setValueError(ENdhsExpressionErrorPropagated);
					return;
				}
			}
		}
		ENdhsExpressionType formatType = formatForExpressionType(format);

		result = (formatType == ENdhsExpressionTypeUnset)
						|| (formatType == type);

		if (formatType == ENdhsExpressionTypeTime && type == ENdhsExpressionTypeInt)
			type = ENdhsExpressionTypeTime;

		// if it is int or float we have to verify the format. Time should be ok.
		if (type == ENdhsExpressionTypeInt || type == ENdhsExpressionTypeScalar)
		{
			if (format.isEmpty())
			{
				setValueError(ENdhsExpressionErrorInternal);
				return;
			}
			else
			{
				if (format.findLastChar('%') != 0)
				{
					setValueError(ENdhsExpressionErrorInternal);
					return;
				}

				const char* frmt = format.bufUtf8();
				int len = format.length();
				LcTWChar ch;

				for (int i = 1; i < len; ++i)
				{
					ch = frmt[i];
					if (lc_isalpha(ch))
					{
						if (i != (len - 1))
						{
							setValueError(ENdhsExpressionErrorInternal);
							return;
						}
						break;
					}
				}

				switch (ch)
				{
				case 'd':
				case 'e':
				case 'E':
				case 'f':
				case 'g':
				case 'G':
				case 'o':
				case 'i':
				case 'x':
				case 'X':
					break;
				default:
					setValueError(ENdhsExpressionErrorInternal);
					return;
				}
			}
		}

		switch (type)
		{
			case ENdhsExpressionTypeInt:
			{
				result = (formatType == ENdhsExpressionTypeInt || formatType == ENdhsExpressionTypeUnset)
						&& doEvaluateStringFromInt(fieldorExpression->getValueInt(),format);
			}
			break;
			case ENdhsExpressionTypeScalar:
			{
				result = (formatType == ENdhsExpressionTypeInt || formatType == ENdhsExpressionTypeUnset)
								&& doEvaluateStringFromScalar(fieldorExpression->getValueScalar(),format);
			}
			break;
			case ENdhsExpressionTypeTime:
			{
				result = (formatType == ENdhsExpressionTypeTime || formatType == ENdhsExpressionTypeUnset)
								&& doEvaluateStringFromTime(fieldorExpression->getValueTime(),format);
			}
			break;
			case ENdhsExpressionTypeBool:
			case ENdhsExpressionTypeString:
			{
				result = false;
			}
			break;
			case ENdhsExpressionTypeError:
			{
				setValueError(ENdhsExpressionErrorTypeMismatch);
			}
			break;
			default:
			{
				setValueError(ENdhsExpressionErrorPropagated);
			}
		}

		if(!result)
		{
			setValueString(fieldorExpression->getValueString());
		}
	}
	else
	{
		setValueError(ENdhsExpressionErrorInternal);
	}
}

void NdhsCExpressionFnIf::doEvaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item)
{
	NdhsCExpression* cond = m_parameters[0];
	NdhsCExpression* valIfTrue = m_parameters[1];
	NdhsCExpression* valIfFalse = m_parameters[2];

	if (cond && valIfTrue && valIfFalse)
	{
		ENdhsExpressionType condType = cond->evaluate(fieldContext, slotNum, item);

		if (condType == ENdhsExpressionTypeBool)
		{
			NdhsCExpression* val;

			if (cond->getValueBool())
			{
				val = valIfTrue;
			}
			else
			{
				val = valIfFalse;
			}

			switch (val->evaluate(fieldContext, slotNum, item))
			{
				case ENdhsExpressionTypeBool:
				{
					setValueBool(val->getValueBool());
				}
				break;

				case ENdhsExpressionTypeInt:
				{
					setValueInt(val->getValueInt());
				}
				break;

				case ENdhsExpressionTypeScalar:
				{
					setValueScalar(val->getValueScalar());
				}
				break;

				case ENdhsExpressionTypeString:
				{
					setValueString(val->getValueString());
				}
				break;

				case ENdhsExpressionTypeError:
				{
					setValueError(ENdhsExpressionErrorPropagated);
				}
				break;

				default:
				{
					setValueError(ENdhsExpressionErrorInternal);
				}
				break;
			}
		}
		else if (condType == ENdhsExpressionTypeError)
		{
			setValueError(ENdhsExpressionErrorPropagated);
		}
		else
		{
			setValueError(ENdhsExpressionErrorTypeMismatch);
		}
	}
	else
	{
		setValueError(ENdhsExpressionErrorInternal);
	}
}

#ifdef NDHS_TRACE_ENABLED
/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCExpressionFnIf::showExpression(bool showValues) const
{
	return "(" + m_parameters[0]->showExpression(showValues) + " ? " + m_parameters[1]->showExpression(showValues) + " : " + m_parameters[2]->showExpression(showValues) + ")";
}
#endif // def NDHS_TRACE_ENABLED

/*-------------------------------------------------------------------------*//**
*/
void NdhsCExpressionFnIf::expressionDirty(NdhsCExpression* expr)
{
	if (!isDirty())
	{
		NdhsCExpression* cond = m_parameters[0];

		// Only dirty the exression if the condition or current result is dirtied
		if (expr == cond)
		{
			setDirty(true);
		}
		else
		{
			NdhsCExpression* resultExpr = NULL;

			if (cond->getValueBool())
			{
				resultExpr = m_parameters[1];
			}
			else
			{
				resultExpr = m_parameters[2];
			}

			if (expr == resultExpr)
			{
				setDirty(true);
			}
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
ENdhsPageState NdhsCExpressionFunction::getPageState()
{
	ENdhsPageState retVal = ENdhsPageStateNone;
	int numParams = getNumParameters();
	int currentParam = 0;

	while (currentParam < numParams && retVal == ENdhsPageStateNone)
	{
		ENdhsPageState pageState = m_parameters[currentParam]->getPageState();

		if (pageState != ENdhsPageStateNone)
		{
			retVal = pageState;
		}

		currentParam++;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
ENdhsPageState NdhsCExpressionFnEqual::getPageState()
{
	ENdhsPageState retVal = ENdhsPageStateNone;

	NdhsCField* leftField = m_parameters[0]->getField();
	NdhsCField* rightField = m_parameters[1]->getField();
	LcTaString pageStateName;

	if (leftField)
	{
		if (leftField->getFieldName().compareNoCase("_pageState") == 0)
		{
			pageStateName = m_parameters[1]->getValueString();
		}
	}

	if (pageStateName.isEmpty() && rightField)
	{
		if (rightField->getFieldName().compareNoCase("_pageState") == 0)
		{
			pageStateName = m_parameters[0]->getValueString();
		}
	}

	if (!pageStateName.isEmpty())
	{
		if (pageStateName.compareNoCase("open") == 0)
			retVal = ENdhsPageStateOpen;
		else if (pageStateName.compareNoCase("close") == 0)
			retVal = ENdhsPageStateClose;
		else if (pageStateName.compareNoCase("interactive") == 0)
			retVal = ENdhsPageStateInteractive;
		else if (pageStateName.compareNoCase("hide") == 0)
			retVal = ENdhsPageStateHide;
		else if (pageStateName.compareNoCase("selected") == 0)
			retVal = ENdhsPageStateSelected;
		else if (pageStateName.compareNoCase("show") == 0)
			retVal = ENdhsPageStateShow;
		else if (pageStateName.compareNoCase("launch") == 0)
			retVal = ENdhsPageStateLaunch;
	}

	return retVal;
}
