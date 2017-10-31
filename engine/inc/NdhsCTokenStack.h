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
#ifndef NdhsCTokenStackH
#define NdhsCTokenStackH

#include "inflexionui/engine/inc/LcCTokenReplacer.h"

class NdhsCTokenStack : public LcCBase, public ISerializeable
{
private:

	typedef LcTmOwnerArray<LcCTokenReplacer> TmATokenStack;
	TmATokenStack			m_tokenStack;

	LcTmOwner<LcCTokenReplacer> m_ndhsIniTokens;

	LcTmString				m_language;
	LcTmString				m_screenSize;
	NdhsCPlugin*			m_plugin;

	// Special character indicating start of token
	char					m_charStart;
	// Special character indicating end of token
	char					m_charEnd;
	// Special character with which start/end characters can be escaped
	char					m_charEscape;
	// Special character indicating start of field token
	char					m_fieldTokenStart;
	// Special character indicating end of field token
	char					m_fieldTokenEnd;

	bool					replaceTokens(const LcTmString& text,
											LcTmString& outText,
											NdhsCElement* element,
											NdhsCMenu* menu,
											NdhsCMenuItem* item,
											NdhsIFieldContext* fieldContext,
											int stackLevel,
											int depth);

	// Two-phase construction
							NdhsCTokenStack(const LcTmString& lang, const LcTmString& screen);
	void					construct();

public:

	// Create/destruct
	static LcTaOwner<NdhsCTokenStack> create(const LcTmString& lang, const LcTmString& screen);
	virtual					~NdhsCTokenStack();

	void					flushAllTokens();

	// If you call these, existing tokens may need to be reloaded because they are screen size and language specific
	void					setScreenSize(const LcTmString& screen)	{ m_screenSize = screen; }
	void					setLanguage(const LcTmString& lang)		{ m_language = lang; }
	void					setPlugin(NdhsCPlugin* plugin)			{ m_plugin = plugin; }

	bool					pushTokens(const LcTmString& path);
	void					popTokens(int count = 1);
	void					popTokensToLevel(int level);

	bool					loadIniTokens(const LcTmString& path, const LcTmString& file);
	bool					saveIniTokens(const LcTmString& path, const LcTmString& file);
	inline int				getTokenStackSize() {return m_tokenStack.size();}
	LcCTokenReplacer*		getIniTokens()		{ return m_ndhsIniTokens.ptr(); }

	// Replace all tokens in the specified string
	inline bool				replaceTokens(const LcTmString& text,
											LcTmString& outText,
											NdhsCElement* element,
											NdhsCMenu* menu,
											NdhsCMenuItem* item,
											NdhsIFieldContext* fieldContext,
											int stackLevel)
	{ return replaceTokens(text, outText, element, menu, item, fieldContext, stackLevel, 0); }

	// Get value of specified key
	bool					getValue(const LcTmString& key, LcTmString& value, int stackLevel);
	// This will return the value of the token that is the best match (uses dot notation)
	bool					getBestMatchValue(const LcTmString& searchKey, LcTmString& value, int stackLevel);


#ifdef IFX_SERIALIZATION
	//serialization
	virtual SerializeHandle serialize(LcCSerializeMaster *serializeMaster,bool force=false);
			void			deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
			bool			isMenuItemChild(){return false;}
#endif /* IFX_SERIALIZATION */
};

#endif //NdhsCTokenStackH
