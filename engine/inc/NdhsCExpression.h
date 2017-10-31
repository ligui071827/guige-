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
#ifndef NdhsCExpressionH
#define NdhsCExpressionH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcTString.h"
#include "inflexionui/engine/inc/LcCSerializeMaster.h"
#include "inflexionui/engine/inc/NdhsIFieldContext.h"
#include "inflexionui/engine/inc/NdhsIExpressionObserver.h"

/*-------------------------------------------------------------------------*//**
	NdhsCExpression is a base class of expression.
*/
class NdhsCExpression : public LcCBase, public NdhsIExpressionObserver, public ISerializeable
{
public:
	// Type of expression
	typedef enum
	{
		ENdhsExpressionTypeUnset,
		ENdhsExpressionTypeError,
		ENdhsExpressionTypeBool,
		ENdhsExpressionTypeInt,
		ENdhsExpressionTypeScalar,
		ENdhsExpressionTypeString,
		ENdhsExpressionTypeTime
	} ENdhsExpressionType;

	typedef enum
	{
		EExpressionClassTypeBase,
		EExpressionClassTypeField,
		EExpressionClassTypeFunction
	} EExpressionClassType;

protected:
	// Error codes used for debugging
	typedef enum
	{
		ENdhsExpressionErrorNone,
		ENdhsExpressionErrorInternal,
		ENdhsExpressionErrorPropagated,
		ENdhsExpressionErrorUnknownField,
		ENdhsExpressionErrorDivideByZero,
		ENdhsExpressionErrorModByZero,
		ENdhsExpressionErrorTypeMismatch,
		ENdhsExpressionErrorFieldHasError
	} ENdhsExpressionError;

private:
	ENdhsExpressionType			m_valueType;	// Base type of expression's current value.
	bool						m_dirty;
	LcTmString					m_valueString;

	union
	{
		bool						m_valueBool;
		int							m_valueInt;
		LcTScalar					m_valueScalar;
		IFX_TIME					m_valueTime;
	};

	NdhsIExpressionObserver*		m_observer;

	ENdhsExpressionError			m_errorType;

protected:
								NdhsCExpression(){}
								NdhsCExpression(bool dirty)
								: 	m_valueType(ENdhsExpressionTypeUnset),
									m_dirty(dirty),
									m_observer(NULL)
								{}

	void						setValueBool(bool boolValue)				{ m_valueBool = boolValue; 		m_valueType = ENdhsExpressionTypeBool; 		m_errorType = ENdhsExpressionErrorNone; }
	void						setValueInt(int intValue)					{ m_valueInt = intValue; 		m_valueType = ENdhsExpressionTypeInt; 		m_errorType = ENdhsExpressionErrorNone; }
	void						setValueTime(IFX_TIME timeValue)			{ m_valueTime = timeValue; 		m_valueType = ENdhsExpressionTypeTime; 		m_errorType = ENdhsExpressionErrorNone; }
	void						setValueScalar(LcTScalar scalarValue)		{ m_valueScalar = scalarValue; 	m_valueType = ENdhsExpressionTypeScalar; 	m_errorType = ENdhsExpressionErrorNone; }
	void						setValueString(const LcTmString& strValue)	{ m_valueString = strValue; 	m_valueType = ENdhsExpressionTypeString; 	m_errorType = ENdhsExpressionErrorNone; }
	void						setValueError(ENdhsExpressionError reason)	{ m_valueInt = 0; 				m_valueType = ENdhsExpressionTypeError;		m_errorType = reason; }

	void						setDirty(bool dirty = true);

	typedef enum {
		ENdhsPrecedenceOperatorNone,		// Lowest precedence
		ENdhsPrecedenceOperatorArgumentSeparator,
		ENdhsPrecedenceOperatorIf,
		ENdhsPrecedenceOperatorElse = ENdhsPrecedenceOperatorIf,
		ENdhsPrecedenceOperatorOr,
		ENdhsPrecedenceOperatorAnd,
		ENdhsPrecedenceOperatorEqual,
		ENdhsPrecedenceOperatorNotEqual = ENdhsPrecedenceOperatorEqual,
		ENdhsPrecedenceOperatorLessThan,
		ENdhsPrecedenceOperatorLessThanEqual = ENdhsPrecedenceOperatorLessThan,
		ENdhsPrecedenceOperatorGreaterThan = ENdhsPrecedenceOperatorLessThan,
		ENdhsPrecedenceOperatorGreaterThanEqual = ENdhsPrecedenceOperatorLessThan,
		ENdhsPrecedenceOperatorAdd,
		ENdhsPrecedenceOperatorSubtract = ENdhsPrecedenceOperatorAdd,
		ENdhsPrecedenceOperatorMultiply,
		ENdhsPrecedenceOperatorDivide = ENdhsPrecedenceOperatorMultiply,
		ENdhsPrecedenceOperatorModulus = ENdhsPrecedenceOperatorMultiply,
		ENdhsPrecedenceOperatorUnaryMinus,
		ENdhsPrecedenceOperatorUnaryPlus = ENdhsPrecedenceOperatorUnaryMinus,
		ENdhsPrecedenceOperatorNot = ENdhsPrecedenceOperatorUnaryMinus,
		ENdhsPrecedenceOperatorLeftParenthesis,
		ENdhsPrecedenceOperatorRightParenthesis = ENdhsPrecedenceOperatorLeftParenthesis // Highest precedence
	} ENdhsPrecedenceOperator;

	typedef enum {
		ENdhsTokenOperatorNone,
		ENdhsTokenOperatorEqual,
		ENdhsTokenOperatorNotEqual,
		ENdhsTokenOperatorLessThan,
		ENdhsTokenOperatorLessThanEqual,
		ENdhsTokenOperatorGreaterThan,
		ENdhsTokenOperatorGreaterThanEqual,
		ENdhsTokenOperatorAdd,
		ENdhsTokenOperatorSubtract,
		ENdhsTokenOperatorMultiply,
		ENdhsTokenOperatorDivide,
		ENdhsTokenOperatorUnaryMinus,
		ENdhsTokenOperatorUnaryPlus,
		ENdhsTokenOperatorLeftParenthesis,
		ENdhsTokenOperatorRightParenthesis,
		ENdhsTokenOperatorArgumentSeparator,
		ENdhsTokenOperatorIf,
		ENdhsTokenOperatorElse,
		ENdhsTokenOperatorAnd,
		ENdhsTokenOperatorOr,
		ENdhsTokenOperatorNot,
		ENdhsTokenOperatorModulus
	} ENdhsTokenOperator;

	static ENdhsPrecedenceOperator getPrecedence(ENdhsTokenOperator op);

	typedef enum {
		ENdhsExprTokenTypeNone,
		ENdhsExprTokenTypeOperator,
		ENdhsExprTokenTypeFieldName,
		ENdhsExprTokenTypeFunctionName,
		ENdhsExprTokenTypeLiteralString,
		ENdhsExprTokenTypeLiteralInt,
		ENdhsExprTokenTypeLiteralScalar,
		ENdhsExprTokenTypeLiteralBool
	} ENdhsExprTokenType;

	typedef enum {
		ENdhsExprTokenFnNone,
		ENdhsExprTokenFnAnd,
		ENdhsExprTokenFnOr,
		ENdhsExprTokenFnNot,
		ENdhsExprTokenFnIf,
		ENdhsExprTokenFnToString,
		ENdhsExprTokenFnMin,
		ENdhsExprTokenFnMax,
		ENdhsExprTokenFnMod,
		ENdhsExprTokenFnPow,
		ENdhsExprTokenFnFloor,
		ENdhsExprTokenFnCeil
	} ENdhsExprTokenFunction;


	typedef enum {
		ENdhsExpressionFunctionTypeNone,
		ENdhsExpressionFunctionTypeAnd,
		ENdhsExpressionFunctionTypeOr,
		ENdhsExpressionFunctionTypeNot,
		ENdhsExpressionFunctionTypeIf,
		ENdhsExpressionFunctionTypeToString,
		ENdhsExpressionFunctionTypeMin,
		ENdhsExpressionFunctionTypeMax,
		ENdhsExpressionFunctionTypeMod,
		ENdhsExpressionFunctionTypePow,
		ENdhsExpressionFunctionTypeFloor,
		ENdhsExpressionFunctionTypeCeil,
		ENdhsExpressionFunctionTypeAdd,
		ENdhsExpressionFunctionTypeSubtract,
		ENdhsExpressionFunctionTypeMultiply,
		ENdhsExpressionFunctionTypeDivide,
		ENdhsExpressionFunctionTypeUnaryMinus,
		ENdhsExpressionFunctionTypeUnaryPlus,
		ENdhsExpressionFunctionTypeEqual,
		ENdhsExpressionFunctionTypeNotEqual,
		ENdhsExpressionFunctionTypeLessThan,
		ENdhsExpressionFunctionTypeLessThanEqual,
		ENdhsExpressionFunctionTypeGreaterThan,
		ENdhsExpressionFunctionTypeGreaterThanEqual,
	} ENdhsExpressionFunctionType;

	class CExprToken : public LcCBase
	{
	protected:
		CExprToken()	{}

	public:
		virtual LcTaString 				getToken()		{ return ""; }
		virtual ENdhsPrecedenceOperator getPrecedence()	{ return ENdhsPrecedenceOperatorNone; }
		virtual ENdhsExprTokenType		getType()		{ return ENdhsExprTokenTypeNone; }
		virtual ENdhsExprTokenFunction	getFnType()		{ return ENdhsExprTokenFnNone; }
		virtual ENdhsTokenOperator		getOperator()	{ return ENdhsTokenOperatorNone; }
		virtual bool					isRightAssoc()	{ return false; }
		virtual void					setArgCount(int numArgs)	{ LC_UNUSED(numArgs); }
		virtual int						getArgCount()				{ return 0; }

		virtual					~CExprToken()	{}
	};

	class CExprTokenOperator : public CExprToken
	{
	private:
		ENdhsPrecedenceOperator	m_precedence;
		ENdhsTokenOperator		m_operator;

	protected:
		CExprTokenOperator(ENdhsTokenOperator op) : m_operator(op) { m_precedence = NdhsCExpression::getPrecedence(m_operator); }

	public:
		virtual LcTaString 				getToken();
		virtual ENdhsPrecedenceOperator getPrecedence()	{ return m_precedence; }
		virtual ENdhsExprTokenType		getType()		{ return ENdhsExprTokenTypeOperator; }
		virtual bool					isRightAssoc()	{ return (m_operator == ENdhsTokenOperatorUnaryMinus)
																|| (m_operator == ENdhsTokenOperatorNot)
																|| (m_operator == ENdhsTokenOperatorElse)
																|| (m_operator == ENdhsTokenOperatorIf); }
		virtual ENdhsTokenOperator		getOperator()	{ return m_operator; }

		static LcTaOwner<CExprTokenOperator>	create(ENdhsTokenOperator op);
		virtual					~CExprTokenOperator()	{}
	};

	class CExprTokenFieldName : public CExprToken
	{
	private:
		LcTmString				m_fieldName;

	protected:
		CExprTokenFieldName(const LcTmString& fieldName) : m_fieldName(fieldName) {}

	public:
		virtual LcTaString 				getToken()		{ return m_fieldName; }
		virtual ENdhsExprTokenType		getType()		{ return ENdhsExprTokenTypeFieldName; }

		static LcTaOwner<CExprTokenFieldName>	create(const LcTmString& fieldName);
		virtual					~CExprTokenFieldName()	{}
	};

	class CExprTokenFunctionName : public CExprToken
	{
	private:
		ENdhsExprTokenFunction	m_fnType;
		int						m_numArgs;

	protected:
		CExprTokenFunctionName(ENdhsExprTokenFunction fnType) : m_fnType(fnType), m_numArgs(0) {}

	public:
		virtual LcTaString 				getToken();
		virtual ENdhsExprTokenType		getType()		{ return ENdhsExprTokenTypeFunctionName; }
		virtual ENdhsExprTokenFunction	getFnType()		{ return m_fnType; }
		virtual void					setArgCount(int numArgs)	{ m_numArgs = numArgs; }
		virtual int						getArgCount()				{ return m_numArgs; }

		static LcTaOwner<CExprTokenFunctionName>	create(ENdhsExprTokenFunction fnType);
		virtual					~CExprTokenFunctionName()	{}
	};

	class CExprTokenLiteralString : public CExprToken
	{
	private:
		LcTmString				m_string;

	protected:
		CExprTokenLiteralString(const LcTmString& string) : m_string(string) {}

	public:
		virtual LcTaString 				getToken()		{ return m_string; }
		virtual ENdhsExprTokenType		getType()		{ return ENdhsExprTokenTypeLiteralString; }

		static LcTaOwner<CExprTokenLiteralString>	create(const LcTmString& string);
		virtual					~CExprTokenLiteralString()	{}
	};

	class CExprTokenLiteralInt : public CExprToken
	{
	private:
		unsigned int			m_val;

	protected:
		CExprTokenLiteralInt(unsigned int val) : m_val(val) {}

	public:
		virtual LcTaString 				getToken()		{ return LcTaString().fromInt(m_val); }
		virtual ENdhsExprTokenType		getType()		{ return ENdhsExprTokenTypeLiteralInt; }

		int								getVal()		{ return m_val; }

		static LcTaOwner<CExprTokenLiteralInt>	create(unsigned int val);
		virtual					~CExprTokenLiteralInt()	{}
	};

	class CExprTokenLiteralScalar : public CExprToken
	{
	private:
		LcTScalar				m_val;

	protected:
		CExprTokenLiteralScalar(LcTScalar val) : m_val(val) {}

	public:
		virtual LcTaString 				getToken()		{ return LcTaString().fromScalar(m_val); }
		virtual ENdhsExprTokenType		getType()		{ return ENdhsExprTokenTypeLiteralScalar; }

		LcTScalar						getVal()		{ return m_val; }

		static LcTaOwner<CExprTokenLiteralScalar>	create(LcTScalar val);
		virtual					~CExprTokenLiteralScalar()	{}
	};

	class CExprTokenLiteralBool : public CExprToken
	{
	private:
		bool					m_val;

	protected:
		CExprTokenLiteralBool(bool val) : m_val(val) {}

	public:
		virtual LcTaString 				getToken()		{ return m_val ? "true" : "false"; }
		virtual ENdhsExprTokenType		getType()		{ return ENdhsExprTokenTypeLiteralBool; }

		bool							getVal()		{ return m_val; }

		static LcTaOwner<CExprTokenLiteralBool>	create(bool val);
		virtual					~CExprTokenLiteralBool()	{}
	};

	static LcTaOwner<NdhsCExpression>	createLiteralInt(int val);
	static LcTaOwner<NdhsCExpression>	createLiteralScalar(LcTScalar val);
	static LcTaOwner<NdhsCExpression>	createLiteralBool(bool val);
	static LcTaOwner<NdhsCExpression>	createLiteralString(const LcTmString& val);

	static void							reportError(const LcTmString& errorName, LcTmString* errorOutput);

	ENdhsExpressionError				errorType()	const 										{ return m_errorType; }

public:


	class CExprSkeleton : public LcCBase
	{
	private:
		LcTmOwnerArray<CExprToken>		m_rpnTokenStream;
		bool							m_badExpression;
		LcTmString						m_infixExpression;
		LcTmOwner<NdhsCExpression> 		m_contextFreeExpression; // lazy construction

	protected:
										CExprSkeleton()	: m_badExpression(false) {}
		bool							construct(const LcTmString& infixExpr, LcTmString* errorOutput);

	public:
		// NB The create functions may return a NULL owner object in case of syntax errors in the expression
		static LcTaOwner<CExprSkeleton>	create(const LcTmString& infixExpr, LcTmString* errorOutput = NULL);
		LcTaOwner<NdhsCExpression>		createExpression(NdhsIFieldContext* fieldContext, LcTmString* errorOutput = NULL)			{ return createExpression(fieldContext, -1, NULL, errorOutput); }
		LcTaOwner<NdhsCExpression> 		createExpression(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem * item, LcTmString* errorOutput = NULL);
		NdhsCExpression*				getContextFreeExpression(LcTmString* errorOutput = NULL);

		LcTaString						toString();
		bool							isEmpty()				{ return m_rpnTokenStream.size() == 0; }
		bool							isBad()					{ return m_badExpression; }

		virtual ~CExprSkeleton() {}
	};

	// Evaluate the expression
	ENdhsExpressionType					evaluate()									{ return evaluate(NULL, -1, NULL); }
	virtual ENdhsExpressionType			evaluate(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem* item) 	{ return m_valueType; }

	// Get expression type
	ENdhsExpressionType					expressionType()					{ return m_valueType; }

	// Get current value of expression
	virtual bool						getValueBool();
	virtual int							getValueInt();
	virtual IFX_TIME					getValueTime();
	virtual LcTScalar					getValueScalar();
	virtual LcTaString					getValueString();
	virtual bool						isLvalue()							{ return false;}
			bool						isError()							{ return ENdhsExpressionTypeError == m_valueType;}
			bool						isBool()							{ return ENdhsExpressionTypeBool == m_valueType;}
			bool						isNumeric()							{ return (ENdhsExpressionTypeInt == m_valueType || ENdhsExpressionTypeScalar == m_valueType);}
	virtual NdhsCField*					getField(NdhsIFieldContext* context=NULL,
														int slotnum=-1,
														NdhsCMenuItem* item=NULL){ return NULL; }
	bool								isDirty()							{ return m_dirty; }

	/* Observer support */
	void								setObserver(NdhsIExpressionObserver * obs) 		{ LC_ASSERT(!m_observer); m_observer = obs; }
	void								removeObserver()								{ m_observer = NULL; }

	virtual void 						expressionDirty(NdhsCExpression* expr)			{ setDirty(); }
	virtual void 						expressionDestroyed(NdhsCExpression* expr) 		{ }

	void								errorDiagnostics(const LcTmString& context, bool fatal);

#ifdef NDHS_TRACE_ENABLED
	virtual LcTaString					showExpression(bool showValues) const;
	virtual void						gatherErrorReports(LcTmString& errorOutput) const;
#endif

	virtual ENdhsPageState				getPageState()									{ return ENdhsPageStateNone; }

	virtual								~NdhsCExpression();

#ifdef IFX_SERIALIZATION
	static		NdhsCExpression*		loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	virtual		SerializeHandle			serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	virtual		void					deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	ISerializeable*						getSerializeAble(int &type){type=4; return this;}
				bool					isMenuItemChild();
#endif /* IFX_SERIALIZATION */
};

#endif // NdhsCExpressionH
