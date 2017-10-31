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


#include <ctype.h>

#define MAX_INT_VALUE "2147483647" // 0x7FFFFFFF

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression::CExprTokenOperator> NdhsCExpression::CExprTokenOperator::create(ENdhsTokenOperator op)
{
	LcTaOwner<CExprTokenOperator> ref;
	ref.set(new CExprTokenOperator(op));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression::CExprTokenFieldName> NdhsCExpression::CExprTokenFieldName::create(const LcTmString& fieldName)
{
	LcTaOwner<CExprTokenFieldName> ref;
	ref.set(new CExprTokenFieldName(fieldName));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression::CExprTokenFunctionName> NdhsCExpression::CExprTokenFunctionName::create(ENdhsExprTokenFunction fnType)
{
	LcTaOwner<CExprTokenFunctionName> ref;
	ref.set(new CExprTokenFunctionName(fnType));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression::CExprTokenLiteralString> NdhsCExpression::CExprTokenLiteralString::create(const LcTmString& string)
{
	LcTaOwner<CExprTokenLiteralString> ref;
	ref.set(new CExprTokenLiteralString(string));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression::CExprTokenLiteralInt> NdhsCExpression::CExprTokenLiteralInt::create(unsigned int val)
{
	LcTaOwner<CExprTokenLiteralInt> ref;
	ref.set(new CExprTokenLiteralInt(val));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression::CExprTokenLiteralScalar> NdhsCExpression::CExprTokenLiteralScalar::create(LcTScalar val)
{
	LcTaOwner<CExprTokenLiteralScalar> ref;
	ref.set(new CExprTokenLiteralScalar(val));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression::CExprTokenLiteralBool> NdhsCExpression::CExprTokenLiteralBool::create(bool val)
{
	LcTaOwner<CExprTokenLiteralBool> ref;
	ref.set(new CExprTokenLiteralBool(val));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCExpression::~NdhsCExpression()
{
	if (m_observer)
	{
		m_observer->expressionDestroyed(this);
	}
}

#ifdef IFX_SERIALIZATION
NdhsCExpression* NdhsCExpression::loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * ptr=serializeMaster->getOffset(handle);
	EExpressionClassType type=*((EExpressionClassType*)ptr);
	switch(type)
	{
		case EExpressionClassTypeBase:
		{
			NdhsCExpression*obj=new NdhsCExpression();
			obj->deSerialize(handle,serializeMaster);
			serializeMaster->setPointer(handle,obj);
			return obj;
		}

		case EExpressionClassTypeField:
		{
			return NdhsCExpressionField::loadState(handle,serializeMaster);
			break;
		}

		case EExpressionClassTypeFunction:
		{
			return NdhsCExpressionFunction::loadState(handle,serializeMaster);
			break;
		}
	}
	return NULL;
}

SerializeHandle	NdhsCExpression::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int outputSize = sizeof(NdhsCExpression)+sizeof(IFX_INT32);
	void * ptr=0;
	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;

	EExpressionClassType classType=EExpressionClassTypeBase;
	SERIALIZE(classType,serializeMaster,cPtr)
	SERIALIZE(m_valueType,serializeMaster,cPtr)
	SERIALIZE(m_dirty,serializeMaster,cPtr)
	SERIALIZE(m_valueInt,serializeMaster,cPtr)

	SerializeHandle h=-1;
	int observerType=-1;
	if(m_observer!=NULL)
	{
		h=serializeMaster->reserveHandle(m_observer->getSerializeAble(observerType));
	}
	SERIALIZE(h,serializeMaster,cPtr)
	SERIALIZE(observerType,serializeMaster,cPtr)
	SERIALIZE_String(m_valueString,serializeMaster,cPtr)
	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}

bool NdhsCExpression::isMenuItemChild()
{
	if(m_observer)
	{
		int type;
		ISerializeable *serializeAble=m_observer->getSerializeAble(type);
		return serializeAble==NULL || serializeAble->isMenuItemChild();
	}
	return false;
}

void NdhsCExpression::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);
	DESERIALIZE(m_valueType,serializeMaster,cPtr)// class type
	DESERIALIZE(m_valueType,serializeMaster,cPtr)
	DESERIALIZE(m_dirty,serializeMaster,cPtr)
	DESERIALIZE(m_valueInt,serializeMaster,cPtr)

	SerializeHandle h=-1;
	int observerType=-1;
	DESERIALIZE(h,serializeMaster,cPtr)
	DESERIALIZE(observerType,serializeMaster,cPtr)
	if(h!=-1)
	{
		m_observer=NdhsIExpressionObserver::loadState(observerType,h,serializeMaster);
	}
	else
	{
		m_observer=NULL;
	}
	DESERIALIZE_String(m_valueString,serializeMaster,cPtr)
}


NdhsIExpressionObserver* NdhsIExpressionObserver::loadState(int type,SerializeHandle h,LcCSerializeMaster* serializeMaster)
{
    ISerializeable* serializeable=NULL;
	switch(type)
	{
		case 0:
		{
			serializeable=serializeMaster->getPointer(h);
			if(serializeable==NULL)
			{
				return NdhsCElement::loadState(h,serializeMaster);
			}
			return (NdhsCElement*)serializeable;
		}
		break;
		case 2:
		{
			serializeable=serializeMaster->getPointer(h);
			if(serializeable==NULL)
			{
				return NdhsCExpressionField::loadState(h,serializeMaster);
			}
			return (NdhsCExpressionField*)serializeable;
		}
		break;
		case 3:
		{
			serializeable=serializeMaster->getPointer(h);
			if(serializeable==NULL)
			{
				return NdhsCField::loadState(h,serializeMaster);
			}
			return (NdhsCField*)serializeable;
		}
		break;
		case 4:
		{
			serializeable=serializeMaster->getPointer(h);
			if(serializeable==NULL)
			{
				return NdhsCExpression::loadState(h,serializeMaster);
			}
			return (NdhsCExpression*)serializeable;
		}
		break;
		case 5:
		{
			serializeable=serializeMaster->getPointer(h);
			if(serializeable==NULL)
			{
				return NdhsCMenuComponent::loadState(h,serializeMaster);
			}
			return (NdhsCMenuComponent*)serializeable;
		}
		break;
		default:
			return NULL;
	}
 }

#endif /* IFX_SERIALIZATION */
/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression::CExprSkeleton> NdhsCExpression::CExprSkeleton::create(const LcTmString& infixExpr, LcTmString* errorOutput)
{
	LcTaOwner<CExprSkeleton> ref;
	ref.set(new CExprSkeleton());

	ref->construct(infixExpr, errorOutput);

	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCExpression::CExprSkeleton::construct(const LcTmString& infixExpr, LcTmString* errorOutput)
{
	bool success = true;

	m_infixExpression = infixExpr;

	LcTaString maxIntStr(MAX_INT_VALUE);

	typedef LcTaAuto< LcTmOwnerArray<CExprToken> > TokenStack;
	TokenStack tokenStack;

	LcTaString token;

	ENdhsExprTokenType tokenType = ENdhsExprTokenTypeNone;
	ENdhsExprTokenType prevTokenType = ENdhsExprTokenTypeNone;

	ENdhsTokenOperator tokenOperatorType = ENdhsTokenOperatorNone;
	ENdhsTokenOperator prevTokenOperatorType = ENdhsTokenOperatorNone;

	// This stack keeps a count of the number of arguments in the currently parsed function calls
	// Require a stack for nested function calls
	LcTaArray<int> argCountStack;
	argCountStack.push_back(1);

	int i = 0;
	int len = infixExpr.length();

	bool errorFound = false;

	while (i < len && !errorFound)
	{
		if (lc_isspace((unsigned char)(infixExpr[i])) != 0)
		{
			// Ignore all whitespace.
			i++;
			continue;
		}

		token = "";
		tokenType = ENdhsExprTokenTypeNone;
		tokenOperatorType = ENdhsTokenOperatorNone;

		LcTWChar currentChar = infixExpr[i];

		if (currentChar == '\"' || currentChar == '\'')
		{
			// When a double or single quote mark is found, start parsing for constant string.
			bool stringFound = false;
			LcTWChar delimitChar = currentChar;

			for (i++; i < len && !stringFound; i++)
			{
				if (i + 1 < len && infixExpr[i] == '\\' && infixExpr[i + 1] == '\\')
				{
					// special handling for double backslash - store the first, the
					// second will be stored in a moment.
					token += infixExpr[i];
					i++;
				}
				else if (i + 1 < len && infixExpr[i] == '\\' && (infixExpr[i + 1] == '\'' || infixExpr[i + 1] == '\"'))
				{
					// Skip the initial escape char for quotes
					i++;
				}
				else if (infixExpr[i] == delimitChar)
				{
					stringFound = true;
				}

				if (!stringFound && i < len)
				{
					token += infixExpr[i];
				}
			}

			if(stringFound)
			{
				m_rpnTokenStream.push_back(CExprTokenLiteralString::create(token));
				tokenType = ENdhsExprTokenTypeLiteralString;
			}
			else
			{
				reportError("Expression contains incomplete string value", errorOutput);
				errorFound = true;
			}
		}
		else if (lc_isalpha(currentChar) || currentChar == '_')
		{
			// Start parsing for field name or function name
			bool nameFound = false;

			while(i < len && !nameFound)
			{
				if(!lc_isalnum(infixExpr[i]) && infixExpr[i] != '_')
				{
					nameFound = true;
				}
				else
				{
					token += infixExpr[i];
					i++;
				}
			}
			nameFound = true;

			bool isFunctionName = false;
			bool searchOver = false;
			int searchIdx = i;

			while (searchIdx < len && !searchOver)
			{
				if (lc_isspace(infixExpr[searchIdx]))
				{
					searchIdx++;
				}
				else if (infixExpr[searchIdx] == '(')
				{
					isFunctionName = true;
					searchOver = true;
				}
				else
				{
					searchOver = true;
				}
			}

			if(nameFound == true)
			{
				if (isFunctionName)
				{
					ENdhsExprTokenFunction fnType = ENdhsExprTokenFnNone;

					if (token.compareNoCase("AND") == 0)
					{
						fnType = ENdhsExprTokenFnAnd;
					}
					else if (token.compareNoCase("OR") == 0)
					{
						fnType = ENdhsExprTokenFnOr;
					}
					else if (token.compareNoCase("NOT") == 0)
					{
						fnType = ENdhsExprTokenFnNot;
					}
					else if (token.compareNoCase("IF") == 0)
					{
						fnType = ENdhsExprTokenFnIf;
					}
					else if (token.compareNoCase("FORMAT") == 0)
					{
						fnType = ENdhsExprTokenFnToString;
					}
					else if (token.compareNoCase("MIN") == 0)
					{
						fnType = ENdhsExprTokenFnMin;
					}
					else if (token.compareNoCase("MAX") == 0)
					{
						fnType = ENdhsExprTokenFnMax;
					}
					else if (token.compareNoCase("MOD") == 0)
					{
						fnType = ENdhsExprTokenFnMod;
					}
					else if (token.compareNoCase("POW") == 0)
					{
						fnType = ENdhsExprTokenFnPow;
					}
					else if (token.compareNoCase("FLOOR") == 0)
					{
						fnType = ENdhsExprTokenFnFloor;
					}
					else if (token.compareNoCase("CEIL") == 0)
					{
						fnType = ENdhsExprTokenFnCeil;
					}

					if (fnType != ENdhsExprTokenFnNone)
					{
						tokenStack.push_back(CExprTokenFunctionName::create(fnType));
						tokenType = ENdhsExprTokenTypeFunctionName;
					}
					else
					{
						reportError("Expression contains unknown function name", errorOutput);
						errorFound = true;
					}
				}
				else
				{
					// Cope with special names
					if (token.compareNoCase("false") == 0)
					{
						m_rpnTokenStream.push_back(CExprTokenLiteralBool::create(false));
						tokenType = ENdhsExprTokenTypeLiteralBool;
					}
					else if (token.compareNoCase("true") == 0)
					{
						m_rpnTokenStream.push_back(CExprTokenLiteralBool::create(true));
						tokenType = ENdhsExprTokenTypeLiteralBool;
					}
					else
					{
						m_rpnTokenStream.push_back(CExprTokenFieldName::create(token.toLower()));
						tokenType = ENdhsExprTokenTypeFieldName;
					}
				}
			}
		}
		else if (lc_isdigit(currentChar))
		{
			// When first digit of a number is found, start parsing whole number.
			bool numberFound = true;
			bool dotFound = false;
			bool expFound = false;
			bool finishedParsing = false;

			while(i < len && !finishedParsing)
			{
				if(infixExpr[i] >= '0' && infixExpr[i] <= '9')
				{
					token += infixExpr[i];
					i++;
					numberFound = true;
				}
				else if(infixExpr[i] == '.')
				{
					if(!dotFound && !expFound)
					{
						token += infixExpr[i];
						i++;
						dotFound = true;
						numberFound = false;
					}
					else
					{
						finishedParsing = true;
					}
				}
				else if(infixExpr[i] == 'e' || infixExpr[i] == 'E')
				{
					if(!expFound)
					{
						token += infixExpr[i];
						i++;
						expFound = true;
						numberFound = false;
						if(i < len && (infixExpr[i] == '+' || infixExpr[i] == '-'))
						{
							token += infixExpr[i];
							i++;
						}
					}
					else
					{
						finishedParsing = true;
					}
				}
				else
				{
					finishedParsing = true;
				}
			}

			if(numberFound == true)
			{
				// If integer value is greater than max int then treat it as a real value
				if (dotFound || expFound || (token.length() > maxIntStr.length())
						|| (token.length() == maxIntStr.length() && token > maxIntStr))
				{
					m_rpnTokenStream.push_back(CExprTokenLiteralScalar::create(token.toScalar()));
					tokenType = ENdhsExprTokenTypeLiteralScalar;
				}
				else
				{
					m_rpnTokenStream.push_back(CExprTokenLiteralInt::create(token.toInt()));
					tokenType = ENdhsExprTokenTypeLiteralInt;
				}
			}
			else
			{
				reportError("Expression contains malformed numeric constant", errorOutput);
				errorFound = true;
			}
		}
		else if (currentChar == '(')
		{
			token = '(';
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			tokenOperatorType = ENdhsTokenOperatorLeftParenthesis;

			tokenStack.push_back(CExprTokenOperator::create(tokenOperatorType));

			argCountStack.push_back(1);
		}
		else if (currentChar == ',' || currentChar == ')')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			if (currentChar == ')')
			{
				tokenOperatorType = ENdhsTokenOperatorRightParenthesis;
			}
			else
			{
				tokenOperatorType = ENdhsTokenOperatorArgumentSeparator;
			}

			// copy tokenStack -> rpnStack until left-parenthesis found
			bool leftParenthesisFound = false;
			bool finished = false;
			while(!tokenStack.empty() && leftParenthesisFound == false && !finished)
			{
				if(tokenStack.back()->getPrecedence() == ENdhsPrecedenceOperatorLeftParenthesis)
				{
					leftParenthesisFound = true;

					if (tokenOperatorType == ENdhsTokenOperatorRightParenthesis)
					{
						tokenStack.pop_back();
					}
				}
				else if(tokenStack.back()->getType() == ENdhsExprTokenTypeOperator)
				{
					m_rpnTokenStream.push_back(tokenStack.release_back());
				}
				else
				{
					finished = true;
				}
			}

			if(leftParenthesisFound == false)
			{
				if (tokenOperatorType == ENdhsTokenOperatorRightParenthesis)
				{
					reportError("Expression contains unmatched parentheses", errorOutput);
				}
				else
				{
					reportError("Expression contains unexpected comma", errorOutput);
				}

				errorFound = true;
			}
			else
			{
				if (tokenOperatorType == ENdhsTokenOperatorRightParenthesis)
				{
					// transfer top token iff it is a function name
					if (!tokenStack.empty())
					{
						if (tokenStack.back()->getType() == ENdhsExprTokenTypeFunctionName)
						{
							m_rpnTokenStream.push_back(tokenStack.release_back());

							m_rpnTokenStream.back()->setArgCount(argCountStack.back());
						}
					}

					argCountStack.pop_back();
				}
				else
				{
					// We had a comma: increment the current argument count
					int argCount = argCountStack.back() + 1;
					argCountStack.pop_back();
					argCountStack.push_back(argCount);
				}
			}
		}
		else if (currentChar == '>')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			tokenOperatorType = ENdhsTokenOperatorGreaterThan;

			// Check next character.
			if(i < len && infixExpr[i] == '=')
			{
				i++;
				tokenOperatorType = ENdhsTokenOperatorGreaterThanEqual;
			}
		}
		else if (currentChar == '<')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			tokenOperatorType = ENdhsTokenOperatorLessThan;

			// Check next character.
			if (i < len)
			{
				if (infixExpr[i] == '=')
				{
					i++;
					tokenOperatorType = ENdhsTokenOperatorLessThanEqual;
				}
			}
		}
		else if (currentChar == '*')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			tokenOperatorType = ENdhsTokenOperatorMultiply;
		}
		else if (currentChar == '/')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			tokenOperatorType = ENdhsTokenOperatorDivide;
		}
		else if (currentChar == '%')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			tokenOperatorType = ENdhsTokenOperatorModulus;
		}
		else if (currentChar == ':')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			tokenOperatorType = ENdhsTokenOperatorElse;

			// Check next character.
			if (i < len - 1)
			{
				if (infixExpr[i] == '/' && infixExpr[i+1] == '/')
				{
					// infix is probably a string literal link, certainly
					// not a valid expression, anyway.  Note that we don't
					// want to generate an error in this case, as we support
					// string literal data sources without the need for quotation
					// marks
					errorFound = true;
				}
			}
		}
		else if (currentChar == '?')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			tokenOperatorType = ENdhsTokenOperatorIf;
		}
		else if (currentChar == '=')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;

			// Check next character.
			if (i < len)
			{
				if (infixExpr[i] == '=')
				{
					i++;
					tokenOperatorType = ENdhsTokenOperatorEqual;
				}
			}
		}
		else if (currentChar == '&')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;

			// Check next character.
			if (i < len)
			{
				if (infixExpr[i] == '&')
				{
					i++;
					tokenOperatorType = ENdhsTokenOperatorAnd;
				}
			}
		}
		else if (currentChar == '|')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;

			// Check next character.
			if (i < len)
			{
				if (infixExpr[i] == '|')
				{
					i++;
					tokenOperatorType = ENdhsTokenOperatorOr;
				}
			}
		}
		else if (currentChar == '!')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			tokenOperatorType = ENdhsTokenOperatorNot;

			// Check next character.
			if (i < len)
			{
				if (infixExpr[i] == '=')
				{
					i++;
					tokenOperatorType = ENdhsTokenOperatorNotEqual;
				}
			}
		}
		else if (currentChar == '+')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			tokenOperatorType = ENdhsTokenOperatorAdd;

			if (prevTokenType == ENdhsExprTokenTypeNone ||
					(  ENdhsExprTokenTypeOperator         == prevTokenType
					&& ENdhsTokenOperatorRightParenthesis != prevTokenOperatorType))
			{
				tokenOperatorType = ENdhsTokenOperatorUnaryPlus;
			}
		}
		else if (currentChar == '-')
		{
			i++;

			tokenType = ENdhsExprTokenTypeOperator;
			tokenOperatorType = ENdhsTokenOperatorSubtract;

			if (prevTokenType == ENdhsExprTokenTypeNone ||
					(  ENdhsExprTokenTypeOperator         == prevTokenType
					&& ENdhsTokenOperatorRightParenthesis != prevTokenOperatorType))
			{
				tokenOperatorType = ENdhsTokenOperatorUnaryMinus;
			}
		}
		else
		{
			reportError("Expression contains unexpected input", errorOutput);
			errorFound = true;

			i++;
		}

		if (tokenType == ENdhsExprTokenTypeOperator && tokenOperatorType == ENdhsTokenOperatorNone)
		{
			reportError("Expression contains unknown operator", errorOutput);
			errorFound = true;
		}

		if(ENdhsExprTokenTypeOperator == tokenType
			&& ENdhsTokenOperatorLeftParenthesis != tokenOperatorType
			&& ENdhsTokenOperatorRightParenthesis != tokenOperatorType
			&& ENdhsTokenOperatorArgumentSeparator != tokenOperatorType
			&& ENdhsTokenOperatorUnaryPlus != tokenOperatorType)
		{
			ENdhsPrecedenceOperator tokenPrecedence = getPrecedence(tokenOperatorType);
			LcTaOwner<CExprToken> newOp = CExprTokenOperator::create(tokenOperatorType);
			bool finished = false;

			while(!tokenStack.empty() && !finished)
			{
				CExprToken* stackTop = tokenStack.back();
				ENdhsPrecedenceOperator stackTopPrec = stackTop->getPrecedence();
				bool stackTopRightAssoc = stackTop->isRightAssoc();

				if( stackTop->getType() == ENdhsExprTokenTypeOperator
					&& stackTopPrec != ENdhsPrecedenceOperatorLeftParenthesis
					&& (	(!stackTopRightAssoc && tokenPrecedence <= stackTopPrec)
						||	( stackTopRightAssoc && tokenPrecedence <  stackTopPrec)))
				{
					m_rpnTokenStream.push_back(tokenStack.release_back());
				}
				else if (tokenStack.size() >= 2
					&& stackTop->getOperator() == ENdhsTokenOperatorElse
					&& (*(tokenStack.end()-2))->getOperator() == ENdhsTokenOperatorIf
					&& tokenOperatorType == ENdhsTokenOperatorElse)
				{
					// Deals with the case where the operator stack looks like ? ? : :
					// e.g. from the expression a ? b ? c : d : e
					// which should be interpreted a ? (b ? c : d) : e
					m_rpnTokenStream.push_back(tokenStack.release_back());
					m_rpnTokenStream.push_back(tokenStack.release_back());
				}
				else
				{
					finished = true;
				}
			}

			tokenStack.push_back(newOp);
		}

		prevTokenType = tokenType;
		prevTokenOperatorType = tokenOperatorType;
	}// end of outer while

	// Read token (operators) from stack and output into the rpnExpr.
	while(tokenStack.size() > 0 && !errorFound)
	{
		if(tokenStack.back()->getType() == ENdhsExprTokenTypeOperator && tokenStack.back()->getPrecedence() != ENdhsPrecedenceOperatorLeftParenthesis)
		{
			m_rpnTokenStream.push_back(tokenStack.release_back());
		}
		else
		{
			if(tokenStack.back()->getPrecedence() == ENdhsPrecedenceOperatorLeftParenthesis)
			{
				reportError("Expression contains mismatched parentheses", errorOutput);
				errorFound = true;
			}
			else
			{
				reportError("Expression is malformed", errorOutput);
				errorFound = true;
			}
			break;
		}
	}

	if (errorFound)
	{
		m_badExpression = true;
		success = false;

		// Don't leave any garbage in the stream
		m_rpnTokenStream.clear();
	}

	return success;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCExpression::getValueBool()
{
	return m_valueType == ENdhsExpressionTypeBool ? m_valueBool : false;
}

/*-------------------------------------------------------------------------*//**
*/
int NdhsCExpression::getValueInt()
{
	int retVal = 0;

	if (m_valueType == ENdhsExpressionTypeInt)
	{
		retVal = m_valueInt;
	}
	else if (m_valueType == ENdhsExpressionTypeScalar)
	{
		retVal = (int)m_valueScalar;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
IFX_TIME NdhsCExpression::getValueTime()
{
	IFX_TIME retVal = 0;

	if (m_valueType == ENdhsExpressionTypeTime)
	{
		retVal = m_valueTime;
	}
	else if (m_valueType == ENdhsExpressionTypeInt)
	{
		retVal = (IFX_TIME) m_valueInt;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTScalar NdhsCExpression::getValueScalar()
{
	LcTScalar retVal = 0.0;

	if (m_valueType == ENdhsExpressionTypeScalar)
	{
		retVal = m_valueScalar;
	}
	else if (m_valueType == ENdhsExpressionTypeInt)
	{
		retVal = (LcTScalar)m_valueInt;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCExpression::getValueString()
{
	LcTaString retVal;

	if (m_valueType == ENdhsExpressionTypeString)
	{
		retVal = m_valueString;
	}
	else if (m_valueType == ENdhsExpressionTypeInt)
	{
		retVal.fromInt(m_valueInt);
	}
	else if (m_valueType == ENdhsExpressionTypeScalar)
	{
		retVal.fromScalar(m_valueScalar);
	}
	else if (m_valueType == ENdhsExpressionTypeBool)
	{
		retVal = m_valueBool ? "true" : "false";
	}
	else if (m_valueType == ENdhsExpressionTypeTime)
	{
		retVal.fromTime(m_valueTime);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	Helper function to get precedence for a token
*/
NdhsCExpression::ENdhsPrecedenceOperator NdhsCExpression::getPrecedence(ENdhsTokenOperator op)
{
	ENdhsPrecedenceOperator retVal =  ENdhsPrecedenceOperatorNone;

	switch (op)
	{
		case ENdhsTokenOperatorEqual: 				retVal = ENdhsPrecedenceOperatorEqual; break;
		case ENdhsTokenOperatorNotEqual: 			retVal = ENdhsPrecedenceOperatorNotEqual; break;
		case ENdhsTokenOperatorLessThan: 			retVal = ENdhsPrecedenceOperatorLessThan; break;
		case ENdhsTokenOperatorLessThanEqual: 		retVal = ENdhsPrecedenceOperatorLessThanEqual; break;
		case ENdhsTokenOperatorGreaterThan: 		retVal = ENdhsPrecedenceOperatorGreaterThan; break;
		case ENdhsTokenOperatorGreaterThanEqual:	retVal = ENdhsPrecedenceOperatorGreaterThanEqual; break;
		case ENdhsTokenOperatorAdd: 				retVal = ENdhsPrecedenceOperatorAdd; break;
		case ENdhsTokenOperatorSubtract: 			retVal = ENdhsPrecedenceOperatorSubtract; break;
		case ENdhsTokenOperatorMultiply: 			retVal = ENdhsPrecedenceOperatorMultiply; break;
		case ENdhsTokenOperatorDivide: 				retVal = ENdhsPrecedenceOperatorDivide; break;
		case ENdhsTokenOperatorUnaryMinus: 			retVal = ENdhsPrecedenceOperatorUnaryMinus; break;
		case ENdhsTokenOperatorUnaryPlus: 			retVal = ENdhsPrecedenceOperatorUnaryPlus; break;
		case ENdhsTokenOperatorLeftParenthesis: 	retVal = ENdhsPrecedenceOperatorLeftParenthesis; break;
		case ENdhsTokenOperatorRightParenthesis: 	retVal = ENdhsPrecedenceOperatorRightParenthesis; break;
		case ENdhsTokenOperatorArgumentSeparator:	retVal = ENdhsPrecedenceOperatorArgumentSeparator; break;
		case ENdhsTokenOperatorIf:					retVal = ENdhsPrecedenceOperatorIf; break;
		case ENdhsTokenOperatorElse:				retVal = ENdhsPrecedenceOperatorElse; break;
		case ENdhsTokenOperatorAnd:					retVal = ENdhsPrecedenceOperatorAnd; break;
		case ENdhsTokenOperatorOr:					retVal = ENdhsPrecedenceOperatorOr; break;
		case ENdhsTokenOperatorNot:					retVal = ENdhsPrecedenceOperatorNot; break;
		case ENdhsTokenOperatorModulus:				retVal = ENdhsPrecedenceOperatorModulus; break;
		default:									retVal = ENdhsPrecedenceOperatorNone; break;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCExpression::CExprTokenOperator::getToken()
{
	LcTaString retVal;

	switch (m_operator)
	{
		case ENdhsTokenOperatorEqual: 				retVal = "=="; break;
		case ENdhsTokenOperatorNotEqual: 			retVal = "!="; break;
		case ENdhsTokenOperatorLessThan: 			retVal = "<"; break;
		case ENdhsTokenOperatorLessThanEqual: 		retVal = "<="; break;
		case ENdhsTokenOperatorGreaterThan: 		retVal = ">"; break;
		case ENdhsTokenOperatorGreaterThanEqual:	retVal = ">="; break;
		case ENdhsTokenOperatorAdd: 				retVal = "+"; break;
		case ENdhsTokenOperatorSubtract: 			retVal = "-"; break;
		case ENdhsTokenOperatorMultiply: 			retVal = "*"; break;
		case ENdhsTokenOperatorDivide: 				retVal = "/"; break;
		case ENdhsTokenOperatorUnaryMinus: 			retVal = "~"; break;
		case ENdhsTokenOperatorUnaryPlus: 			retVal = "+"; break;
		case ENdhsTokenOperatorLeftParenthesis: 	retVal = "("; break;
		case ENdhsTokenOperatorRightParenthesis: 	retVal = ")"; break;
		case ENdhsTokenOperatorIf:					retVal = "?"; break;
		case ENdhsTokenOperatorElse:				retVal = ":"; break;
		case ENdhsTokenOperatorAnd:					retVal = "&&"; break;
		case ENdhsTokenOperatorOr:					retVal = "||"; break;
		case ENdhsTokenOperatorNot:					retVal = "!"; break;
		case ENdhsTokenOperatorModulus:				retVal = "%"; break;
		default:									retVal = "?"; break;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCExpression::CExprTokenFunctionName::getToken()
{
	LcTaString retVal;

	switch (m_fnType)
	{
		case ENdhsExprTokenFnAnd:					retVal = "AND"; break;
		case ENdhsExprTokenFnOr:					retVal = "OR"; break;
		case ENdhsExprTokenFnNot:					retVal = "NOT"; break;
		case ENdhsExprTokenFnIf:					retVal = "IF"; break;
		case ENdhsExprTokenFnToString:				retVal = "FORMAT"; break;
		case ENdhsExprTokenFnMin:					retVal = "MIN"; break;
		case ENdhsExprTokenFnMax:					retVal = "MAX"; break;
		case ENdhsExprTokenFnMod:					retVal = "MOD"; break;
		case ENdhsExprTokenFnPow:					retVal = "POW"; break;
		case ENdhsExprTokenFnFloor:					retVal = "FLOOR"; break;
		case ENdhsExprTokenFnCeil:					retVal = "CEIL"; break;
		default:									retVal = "???"; break;
	}
	retVal += "(" + LcTaString().fromInt(m_numArgs) + ")";

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCExpression::CExprSkeleton::toString()
{
	LcTaString retVal;

	if (m_badExpression)
	{
		retVal = m_infixExpression;
	}
	else
	{
		LcTmOwnerArray<CExprToken>::iterator it = m_rpnTokenStream.begin();

		if (it != m_rpnTokenStream.end())
		{
			retVal = (*it)->getToken();
			it++;
		}

		for (; it != m_rpnTokenStream.end(); it++)
		{
			retVal += " " + (*it)->getToken();
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	General factory function for creating an expression tree
*/
LcTaOwner<NdhsCExpression> NdhsCExpression::CExprSkeleton::createExpression(NdhsIFieldContext* fieldContext, int slotNum, NdhsCMenuItem * item, LcTmString* errorOutput)
{
	typedef LcTaAuto< LcTmOwnerArray<NdhsCExpression> > ExpressionStack;
	ExpressionStack	expressionStack;

	LcTmOwnerArray<CExprToken>::iterator rpnIt = m_rpnTokenStream.begin();

	for(; rpnIt != m_rpnTokenStream.end() && !m_badExpression; rpnIt++)
	{
		switch((*rpnIt)->getType())
		{
			case ENdhsExprTokenTypeOperator:
			case ENdhsExprTokenTypeFunctionName:
			{
				// Skip over else operators
				if ((*rpnIt)->getOperator() != ENdhsTokenOperatorElse)
				{
					int exprIterations = 1;

					switch ((*rpnIt)->getFnType())
					{
						case ENdhsExprTokenFnAnd:
						case ENdhsExprTokenFnOr:
						case ENdhsExprTokenFnMin:
						case ENdhsExprTokenFnMax:
							exprIterations = max(1, (*rpnIt)->getArgCount() - 1);
							break;

						default:
							break;
					}

					while (exprIterations > 0 && !m_badExpression)
					{
						LcTaOwner<NdhsCExpression> expr = NdhsCExpressionFunction::create(*rpnIt, expressionStack, errorOutput);

						if (expr)
						{
							expressionStack.push_back(expr);
						}
						else
						{
							m_badExpression = true;
						}

						exprIterations--;
					}
				}
			}
			break;

			case ENdhsExprTokenTypeFieldName:
			{
				expressionStack.push_back(NdhsCExpressionField::create((*rpnIt)->getToken(), fieldContext, slotNum, item));
			}
			break;

			case ENdhsExprTokenTypeLiteralString:
			{
				expressionStack.push_back(NdhsCExpression::createLiteralString((*rpnIt)->getToken()));
			}
			break;

			case ENdhsExprTokenTypeLiteralInt:
			{
				CExprTokenLiteralInt* token = (CExprTokenLiteralInt*)(*rpnIt);
				expressionStack.push_back(NdhsCExpression::createLiteralInt(token->getVal()));
			}
			break;

			case ENdhsExprTokenTypeLiteralScalar:
			{
				CExprTokenLiteralScalar* token = (CExprTokenLiteralScalar*)(*rpnIt);
				expressionStack.push_back(NdhsCExpression::createLiteralScalar(token->getVal()));
			}
			break;

			case ENdhsExprTokenTypeLiteralBool:
			{
				CExprTokenLiteralBool* token = (CExprTokenLiteralBool*)(*rpnIt);
				expressionStack.push_back(NdhsCExpression::createLiteralBool(token->getVal()));
			}
			break;

			default:
			{
				// error case
				LC_ASSERT(false);
			}
			break;
		}
	}

	// All tokens are parsed and expression tree is created. The final tree is in the stack.
	if (!m_badExpression && expressionStack.size() == 1)
	{
		LcTaOwner<NdhsCExpression> retVal = expressionStack.release_back();

		if (fieldContext && retVal)
		{
			// Evaluate once to clean
			retVal->evaluate(fieldContext, slotNum, item);
		}

		return retVal;
	}
	else
	{
		// If we get here, the expresison was malformed
		LcTaOwner<NdhsCExpression> retVal;

		m_badExpression = true;
		return retVal;
	}
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCExpression* NdhsCExpression::CExprSkeleton::getContextFreeExpression(LcTmString* errorOutput)
{
	if (!m_contextFreeExpression)
	{
		m_contextFreeExpression = createExpression(NULL, errorOutput);
	}

	return m_contextFreeExpression.ptr();
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCExpression::setDirty(bool dirty)
{
	if (dirty && !m_dirty && m_observer)
	{
		m_dirty = true; // Note set this here in case observer interogates expression
		m_observer->expressionDirty(this);
	}
	else
	{
		m_dirty = dirty;
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression> NdhsCExpression::createLiteralInt(int val)
{
	LcTaOwner<NdhsCExpression> ref;
	ref.set(new NdhsCExpression(false));
	ref->setValueInt(val);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression> NdhsCExpression::createLiteralScalar(LcTScalar val)
{
	LcTaOwner<NdhsCExpression> ref;
	ref.set(new NdhsCExpression(false));
	ref->setValueScalar(val);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression> NdhsCExpression::createLiteralBool(bool val)
{
	LcTaOwner<NdhsCExpression> ref;
	ref.set(new NdhsCExpression(false));
	ref->setValueBool(val);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<NdhsCExpression> NdhsCExpression::createLiteralString(const LcTmString& val)
{
	LcTaOwner<NdhsCExpression> ref;
	ref.set(new NdhsCExpression(false));
	ref->setValueString(val);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCExpression::reportError(const LcTmString& errorName, LcTmString* errorOutput)
{
	if (errorOutput)
	{
		if (errorOutput->isEmpty())
		{
			*errorOutput = errorName;
		}
		else
		{
			*errorOutput += "\n" + errorName;
		}
	}
	else
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleTemplate, errorName.bufUtf8());
	}
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCExpression::errorDiagnostics(const LcTmString& context, bool fatal)
{
#ifdef NDHS_TRACE_ENABLED

	LcTaString errorDescription;
	gatherErrorReports(errorDescription);

	LcTaString errorStr1 = context + " expression error";

	if (!errorDescription.isEmpty())
	{
		errorStr1 += ": " + errorDescription;
	}

	LcTaString errorStr2 = "  - evaluating  " + showExpression(false);
	LcTaString errorStr3 = "  - with values " + showExpression(true);

	if (fatal)
	{
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleExpressions, errorStr1);
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleExpressions, errorStr2);
		NDHS_TRACE(ENdhsTraceLevelError, ENdhsTraceModuleExpressions, errorStr3);
	}
	else
	{
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleExpressions, errorStr1);
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleExpressions, errorStr2);
		NDHS_TRACE(ENdhsTraceLevelWarning, ENdhsTraceModuleExpressions, errorStr3);
	}

#endif // def NDHS_TRACE_ENABLED

	if (fatal)
	{
		IFXP_Display_Error_Note((IFX_WCHAR*)L"Theme error");
		LC_CLEANUP_THROW(IFX_ERROR_RESTART);
	}
}

#ifdef NDHS_TRACE_ENABLED
/*-------------------------------------------------------------------------*//**
*/
LcTaString NdhsCExpression::showExpression(bool showValues) const
{
	LcTaString retVal = "";

	switch (m_valueType)
	{
		case ENdhsExpressionTypeUnset:
			retVal = "[unset]";
			break;

		case ENdhsExpressionTypeBool:
			retVal = m_valueBool ? "TRUE" : "FALSE";
			break;

		case ENdhsExpressionTypeInt:
			retVal = LcTaString().fromInt(m_valueInt);
			break;

		case ENdhsExpressionTypeScalar:
			retVal = LcTaString().fromScalar(m_valueScalar);
			break;

		case ENdhsExpressionTypeString:
			retVal = "\"" + m_valueString + "\"";
			break;

		case ENdhsExpressionTypeError:
		default:
			retVal = "[error]";
			break;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCExpression::gatherErrorReports(LcTmString& errorOutput) const
{
}
#endif // def NDHS_TRACE_ENABLED
