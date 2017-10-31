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
#ifndef LcCKeyValueListH
#define LcCKeyValueListH

/*-------------------------------------------------------------------------*//**
	LcCKeyValueList provides a generic way to maintain a list of 
	string-based key-value pairs.  This is the basis of LcCTokenReplacer.
*/
class LcCKeyValueList : public LcCBase
{
private:

	typedef LcTmMap<LcTmString, LcTmString>	TmMPairs;

	// List of key-value pairs
	TmMPairs				m_pairs;
	LcTmString				m_language;
	LcTmString				m_screenSize;

	void					configureScreenFromXml(LcCXmlElem* screen);
	void					configureLanguageFromXml(LcCXmlElem* lang);

protected:

	// Two-phase construction
							LcCKeyValueList() {}
	void					construct(const LcTmString& lang, const LcTmString& screen);

public:
	
	// Create/destruct
	static LcTaOwner<LcCKeyValueList> create(const LcTmString& lang, const LcTmString& screen);
	virtual					~LcCKeyValueList();

	// Public helpers:

	// Clear all key-value pairs
	void					clear() { m_pairs.clear(); }
	
	// Initialize key-value pairs from specified XML file
	bool					loadFromXml(
								const LcTmString& path, 
								const LcTmString& file);
	// Initialize key-value pairs from the supplied XML node
	void					initFromXml(
								LcCXmlElem* root);

	// Save current list of key-value pairs
	#ifdef LC_USE_XML_SAVE
		bool				saveToXml(
								const LcTmString& path, 
								const LcTmString& file);
	#endif

	// Get value of specified key
	bool					getValue(const LcTmString& key, 
								LcTmString& value);
	// Set value of specified key
	void					setValue(
								const LcTmString& key, 
								const LcTmString& value)		 { m_pairs[key.toLower()] = value; }

	// This will return the value of the token that is the best match.
	bool					getBestMatchValue(const LcTmString& searchKey, LcTmString& value);
};

#endif //LcCKeyValueListH
