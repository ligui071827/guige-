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


/*-------------------------------------------------------------------------*//**
	Creation
*/
LcTaOwner<NdhsCTokenStack> NdhsCTokenStack::create(const LcTmString& lang, const LcTmString& screen)
{
	LcTaOwner<NdhsCTokenStack> ref;
	ref.set(new NdhsCTokenStack(lang, screen));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCTokenStack::NdhsCTokenStack(const LcTmString& lang, const LcTmString& screen)
{
	m_language = lang;
	m_screenSize = screen;
	m_plugin = NULL;

	m_charStart = '{';
	m_charEnd = '}';
	m_charEscape = '$';
	m_fieldTokenStart = '[';
	m_fieldTokenEnd = ']';
}

/*-------------------------------------------------------------------------*//**
	Construction
*/
void NdhsCTokenStack::construct()
{
	LcTaOwner<LcCTokenReplacer> newTokenReplacer = LcCTokenReplacer::create(m_language, m_screenSize);
	m_ndhsIniTokens = newTokenReplacer;
}

/*-------------------------------------------------------------------------*//**
*/
NdhsCTokenStack::~NdhsCTokenStack()
{
	flushAllTokens();

	m_ndhsIniTokens.destroy();
}

#ifdef IFX_SERIALIZATION

SerializeHandle NdhsCTokenStack::serialize(LcCSerializeMaster *serializeMaster,bool force)
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

	int tokenStackSize=m_tokenStack.size();
	int outputSize = sizeof(NdhsCTokenStack)+sizeof(IFX_INT32)*1+sizeof(IFX_INT32)*6*(tokenStackSize);

	void * ptr=0;

	if(IFXP_Mem_Allocate(IFXP_MEM_ENGINE,outputSize,&ptr)!=IFX_SUCCESS)
		return -1;

	void * cPtr=ptr;

	SERIALIZE(tokenStackSize,serializeMaster,cPtr)

	TmATokenStack::iterator iter=m_tokenStack.begin();
	iter++;
	for(;iter!=m_tokenStack.end();iter++)
	{
		LcTaString path=(*iter)->getPath();
		SERIALIZE_String(path,serializeMaster,cPtr)
	}


	serializeMaster->setData(handle,outputSize,(LcTByte*)ptr);
	IFXP_Mem_Deallocate(IFXP_MEM_ENGINE,ptr);
	return handle;
}

void NdhsCTokenStack::deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster)
{
	void * cPtr=serializeMaster->getOffset(handle);
	serializeMaster->setPointer(handle,this);

	int tokenStackSize=0;

	DESERIALIZE(tokenStackSize,serializeMaster,cPtr);

	LcTaString path="";
	for(int i=1;i<tokenStackSize;++i)
	{
		DESERIALIZE_String(path,serializeMaster,cPtr);
		pushTokens(path);
	}
}
#endif /* IFX_SERIALIZATION */

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTokenStack::flushAllTokens()
{
	m_tokenStack.clear();
	if(m_ndhsIniTokens)
		m_ndhsIniTokens->clear();
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTokenStack::pushTokens(const LcTmString& path)
{
	LcTaOwner<LcCTokenReplacer> tr = LcCTokenReplacer::create(m_language, m_screenSize);

	// Load the tokens for all languages
	tr->loadFromXml(path, NDHS_GENERAL_TOKENS_FILENAME);

	tr->setPath(path);

	m_tokenStack.push_back(tr);

	// Doesn't really matter if there are no tokens
	// Its important to have the frame on the stack
	// to keep it synced with  the manifest stack
	return true;
}

/*-------------------------------------------------------------------------*//**
*/
void NdhsCTokenStack::popTokens(int count)
{
	if (count > (int)m_tokenStack.size())
		count = (int)m_tokenStack.size();

	for (int i = 0; i < count; i++)
	{
		m_tokenStack.pop_back();
	}
}

/*-------------------------------------------------------------------------*//**
	Pops the stack leaving the supplied level At the top of the stack
*/
void NdhsCTokenStack::popTokensToLevel(int level)
{
	if (level >= (int)m_tokenStack.size())
		return;

	// Calculate how many items to pop
	int count = ((int)m_tokenStack.size() - level) - 1;

	popTokens(count);
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTokenStack::loadIniTokens(const LcTmString& path, const LcTmString& file)
{
	LC_UNUSED(file)
	LC_UNUSED(path)

	// Disabled for now
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTokenStack::saveIniTokens(const LcTmString& path, const LcTmString& file)
{
	LC_UNUSED(file)
	LC_UNUSED(path)

	// Disabled for now
	return false;
}

/*-------------------------------------------------------------------------*//**
	Look for token names in the supplied text, and replace them with
	their associated values.  The returned bool indicates whether the
	token has been fully expanded
*/
bool NdhsCTokenStack::replaceTokens(const LcTmString& text,
									LcTmString& outText,
									NdhsCElement* element,
									NdhsCMenu* menu,
									NdhsCMenuItem* item,
									NdhsIFieldContext* fieldContext,
									int stackLevel,
									int depth)
{
	// NB: we can't skip calling this unless we search for all 3 special
	// characters, because otherwise escaping and error checking won't be applied

	LcTaString	result	= "";
	if (text.length() > 0)
	{
		LcTaString	key		= "";
		LcTaString	val		= "";
		LcTaString	retVal	= "";
		bool		escaped	= false;
		bool		inToken	= false;
		bool		inFieldToken = false;
		bool		valFound = false;
		const char*	p		= text.bufUtf8();

		// NOTE: This must be strlen and not text.length() because it is
		// the number of bytes required, not the number of characters
		int			len		= (int)lc_strlen(p);
		int			i;

		// NB: this algorithm is UTF8-safe, because our separators are always ASCII
		for (i = 0; i < len; i++)
		{
			char c = p[i];

			// First see if we're currently escaped
			// NB: any character can be escaped, including the escape character
			if (escaped)
			{
				escaped = false;		// We are no longer escaped

				if (inToken)
					key += c;			// Add the escaped char to key if in a token
				else
				{
					result += m_charEscape;		// 2016Äê7ÔÂ6ÈÕ 20:12:53
					result += c;		// Otherwise add escaped char to result
				}
			}
			else
			if (c == m_charEscape)			// Token escape char
			{
				escaped = true;
			}
			else
			if (c == m_charStart)			// Start of token char
			{
				if (inToken) return true;	// Abort if already in a token
				if (depth > 10) return true; // Abort if we are nesting too deep

				inToken = true;				// Otherwise we are in a new token
				inFieldToken = false;		// We are not in a field token
			}
			else
			if (c == m_fieldTokenStart)
			{
				if (inToken) return true;	// Abort if already in a token
				if (depth > 10) return true; // Abort if we are nesting too deep

				inToken = true;				// Otherwise we are in a new token
				inFieldToken = true;		// We are in a field token
			}
			else
			if (c == m_charEnd)				// End of token char
			{
				if (!inToken ) // Abort if we are not in a token
				{
//					while(i<len)
//						result += p[i++];
//					outText = result;
					outText = text;
					return true;	
				}
				if (inFieldToken) return true; // Abort if we are in a field token

				inToken = false;			// Otherwise we are no longer in a token
				valFound = getValue(key, val, stackLevel);
				if (valFound == true)
				{
					retVal = "";

					// Val must have length > 0
					replaceTokens(					// Replace the built-up key with it's value
									val,			// Note the recursion to support nested tokens
									retVal,
									element,
									menu,
									item,
									fieldContext,
									stackLevel,
									depth + 1);		// ...and the limit to stop infinite cyclic expansion

					result += retVal;
				}
				else
				{
					result += m_charStart;
					result += key;
					result += m_charEnd;
				}

				key = "";					// Reset key for next time
			}
			else
			if (c == m_fieldTokenEnd)
			{
				if (!inToken ) 	// Abort if we are not in a token
				{
//					while(i<len)
//						result += p[i++];
					outText = text;
					return true;	
				}
				if (!inFieldToken) return true; // Abort if we are not in a field token

				inToken = false;			// Otherwise we are no longer in a token
				inFieldToken = false;

				if (item)
				{
					// We have item context, so ask the item
					valFound = item->getFieldData(element, key, val);
				}
				else if (menu)
				{
					// We have menu context, so ask the menu
					valFound = menu->getFieldData(element, key, val);
				}
				else
				{
					if (m_plugin)
					{
						// We have no menu or item context, so ask the plugin directly
						valFound = m_plugin->getElementData(-1, key, IFX_FIELD, element, NULL, val);
					}
					else
					{
						valFound = false;
					}
				}

				if (!valFound && fieldContext)
				{
					NdhsCField* field = fieldContext->getField(key, -1, item);

					if (field)
					{
						valFound = true;
						val = field->getFieldData(element);
					}
				}

				// Register the field with the element for refreshes AFTER first
				// getting the data, so we don't get an unnecessary refresh broadcast
				// from the field
				if (element)
				{
					NdhsCField* field = NULL;

					if (item)
					{
						field = item->getField(key, element);
					}
					else if (menu)
					{
						field = menu->getField(key, -1, element);
					}

					if (!field && fieldContext)
					{
						field = fieldContext->getField(key, -1, item);
					}

					if (field)
					{
						element->registerFieldToken(field);
					}
				}

				if (valFound == true)
				{
					retVal = "";

					// Val must have length > 0
					replaceTokens(					// Replace the built-up key with it's value
									val,			// Note the recursion to support nested tokens
									retVal,
									element,
									menu,
									item,
									fieldContext,
 									stackLevel,
									depth + 1);		// ...and the limit to stop infinite cyclic expansion

					result += retVal;
				}
				else
				{
					result += m_fieldTokenStart;
					result += key;
					result += m_fieldTokenEnd;
				}
				
				key = "";					// Reset key for next time
			}
			else
			if (inToken)
			{
				key += c;					// Add non-special chars to key if in a token
			}
			else
			{
				result += c;				// Add non-special chars to result if not in a token
			}
		}

		if (escaped || inToken) 	// Finally abort if done but still escaped or in a token
		{
//			if(inFieldToken)
//				result += m_fieldTokenStart;
//			else
//				result += m_charEnd;
//			outText = result;
			outText = text;
			return true;	
		}

	}

	outText = result;

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTokenStack::getValue(const LcTmString& key, LcTmString& value, int stackLevel)
{
	LcTaString retVal;

	// Check the Ini tokes
	if (m_ndhsIniTokens->getValue(key, retVal))
	{
		value = retVal;
		return true;
	}

	// Check the token stack

	// Make sure we don't overflow
	if (stackLevel==-1 || stackLevel >= (int)m_tokenStack.size())
		stackLevel = (int)m_tokenStack.size() - 1;

	for (int i = stackLevel; i >= 0; i--)
	{
		if (m_tokenStack[i]->getValue(key, retVal))
		{
			value = retVal;
			return true;
		}
	}

	// If we got here, it means that the key was not found
	return false;
}

/*-------------------------------------------------------------------------*//**
*/
bool NdhsCTokenStack::getBestMatchValue(const LcTmString& searchKey, LcTmString& value, int stackLevel)
{
	LcTaString retVal;

	// Check the Ini tokes
	if (m_ndhsIniTokens->getBestMatchValue(searchKey, retVal))
	{
		value = retVal;
		return true;
	}

	// Check the token stack

	// Make sure we don't overflow
	if (stackLevel >= (int)m_tokenStack.size())
		stackLevel = (int)m_tokenStack.size() - 1;

	for (int i = stackLevel; i >= 0; i--)
	{
		if (m_tokenStack[i]->getBestMatchValue(searchKey, retVal))
		{
			value = retVal;
			return true;
		}
	}

	// If we got here, it means that the key was not found
	return false;
}
