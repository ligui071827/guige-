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
LcTaOwner<LcCTokenReplacer> LcCTokenReplacer::create(const LcTmString& lang, const LcTmString& screen)
{
	LcTaOwner<LcCTokenReplacer> ref;
	ref.set(new LcCTokenReplacer());
	ref->construct(lang, screen);
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Construction
*/
void LcCTokenReplacer::construct(const LcTmString& lang, const LcTmString& screen)
{
	// Call inherited construct()
	LcCKeyValueList::construct(lang, screen);

	// Set default special characters
	setSpecialChars('{', '}', '$');
}

/*-------------------------------------------------------------------------*//**
	'Special' chars are used to start and end tokens, and a third allows
	us to escape the first two so that they can be included as 'normal'
	characters.
*/
void LcCTokenReplacer::setSpecialChars(const char start, const char end, const char escape)
{
	m_charStart		= start;
	m_charEnd		= end;
	m_charEscape	= escape;
}

/*-------------------------------------------------------------------------*//**
	Look for token names in the supplied text, and replace them with
	their associated values.  The returned bool indicates whether the
	token has been fully expanded
*/
bool LcCTokenReplacer::replaceTokens(const LcTmString& text, LcTmString& outText, int depth, bool replaceUnknown)
{
	// NB: we can't skip calling this unless we search for all 3 special
	// characters, because otherwise escaping and error checking won't be applied

	bool		unresolved = false;
	LcTaString	result	= "";
	if (text.length() > 0)
	{
		LcTaString	key		= "";
		LcTaString	val		= "";
		LcTaString	retVal	= "";
		bool		escaped	= false;
		bool		inToken	= false;
		bool		valFound = false;
		bool		retTR	= false;
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
					result += c;		// Otherwise add escaped char to result
			}
			else
			if (c == m_charEscape)			// Token escape char
			{
				escaped = true;
			}
			else
			if (c == m_charStart)			// Start of token char
			{
				if (inToken) return true;		// Abort if already in a token
				if (depth > 10) return true; // Abort if we are nesting too deep

				inToken = true;				// Otherwise we are in a new token
			}
			else
			if (c == m_charEnd)				// End of token char
			{
				if (!inToken) return true;	// Abort if we are not in a token

				inToken = false;			// Otherwise we are no longer in a token
				valFound = getValue(key, val);
				if (valFound == true && val.length() == 0)
				{
					// Token explicitly set to "", so we will
					// use it.  Nothing to do here
				}
				else if (valFound == false && replaceUnknown == true)
				{
					// Token not found, but since replaceUnknown
					// is true, we will resolve it to "", so
					// nothing to do here
				}
				else if (valFound == true)
				{
					retVal = "";

					// Val must have length > 0
					retTR = replaceTokens(					// Replace the built-up key with it's value
											val,			// Note the recursion to support nested tokens
											retVal,
											depth + 1,		// ...and the limit to stop infinite cyclic expansion
											replaceUnknown);

					result += retVal;

					// Check to see if anything was left unresolved
					if (retTR == false)
						unresolved = true;
				}
				else
				{
					// We add the unexpanded token to the result
					// Another token replacer may be able to expand it later
					result += "{" + key + "}";
					unresolved = true;
					inToken = false;
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

		if (escaped || inToken) return true;	// Finally abort if done but still escaped or in a token

	}

	outText = result;

	return !unresolved;
}

/*-------------------------------------------------------------------------*//**
	Escape any special chars in the given string
*/
LcTaString LcCTokenReplacer::escape(const LcTmString& text)
{
	LcTaString	result	= "";
	const char*	p		= text.bufUtf8();

	// NOTE: This must be strlen and not text.length() because it is 
	// the number of bytes required, not the number of characters
	int			len		= (int)lc_strlen(p);
	int			i;

	// NB: this algorithm is UTF8-safe, because our separators are always ASCII
	for (i = 0; i < len; i++)
	{
		char c = p[i];

		// Insert escape character if this is any special character
		if (c == m_charEscape || c == m_charStart || c == m_charEnd)
			result += m_charEscape;

		result += c;
	}

	return result;
}

