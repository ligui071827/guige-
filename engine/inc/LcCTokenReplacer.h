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
#ifndef LcCTokenReplacerH
#define LcCTokenReplacerH

#include "inflexionui/engine/inc/LcCKeyValueList.h"

/*-------------------------------------------------------------------------*//**
	LcCTokenReplacer provides a generic way to replace one or more tokens 
	with dynamic values within text strings.  Tokens to be replaced are
	delimited with a start and end character, which are specified with
	setSpecialChars().  LcCTokenReplacer maintains a list of key-value 
	pairs for each token.  replaceTokens() parses a string, and whenever
	it finds a token that matches one of the keys in the list, it is
	replaced with the corresponding value.
	
	For example, with start and end chars of '{' and '}':
		replaceTokens("Hello {person}")
	would return:
		"Hello World"
	after a call to:
		setValue("person", "World")
	
	Note that values may themselves contain tokens, in which case
	replacement will occur recursively (e.g. "person" may have a 
	value of "{firstName} {surName}").

	loadFromXml() enables persistent token key-value pairs to be 
	loaded from file.
*/
class LcCTokenReplacer : public LcCKeyValueList
{
private:

	// Tokens path
	LcTmString				m_path;
	// Special character indicating start of token
	char					m_charStart;
	// Special character indicating end of token
	char					m_charEnd;
	// Special character with which start/end characters can be escaped
	char					m_charEscape;

	// Helper
	bool					replaceTokens(const LcTmString& text, LcTmString& outText, int depth, bool replaceUnknown);

	// Two-phase construction
							LcCTokenReplacer(): LcCKeyValueList() {}
	void					construct(const LcTmString& lang, const LcTmString& screen);

public:
	
	// Create/destruct
	static LcTaOwner<LcCTokenReplacer> create(const LcTmString& lang, const LcTmString& screen);

	// Public helpers:

	// Set the special characters used to parse tokens
	void					setSpecialChars(const char start, const char end, const char escape);
	// Replace all tokens in the specified string
	bool					replaceTokens(const LcTmString& text, LcTmString& outText, bool replaceUnknown = true)
								{ return replaceTokens(text, outText, 0, replaceUnknown); }
	// Escape any special chars in the given string
	LcTaString				escape(const LcTmString& text);
	LcTaString				getPath()					{ return m_path; }
	void					setPath(LcTaString path)	{ m_path=path; }
};

#endif //LcCTokenReplacerH
