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
#ifndef LcCXmlH
#define LcCXmlH

#include "inflexionui/engine/inc/LcDefs.h"
#include "inflexionui/engine/inc/LcTString.h"
#include "inflexionui/engine/inc/LcCFont.h"
#include <stdio.h>

#define LC_XML_ELEM_SEP	"\\"
#define LC_XML_BUF_SIZE	1024

class LcCXmlElem;

#ifdef IFX_GENERATE_SCRIPTS
	class NdhsCScriptGenerator;
#endif

#ifdef IFX_USE_SCRIPTS
	class NdhsCScriptExecutor;
#endif

/*-------------------------------------------------------------------------*//**
	Handles XML attributes - this class is used internally by LcCXmlElem.
*/
class LcCXmlAttr : public LcCBase
{
	friend class LcCXmlElem;

#ifdef IFX_GENERATE_SCRIPTS
	friend class NdhsCScriptGenerator;
#endif

#ifdef IFX_USE_SCRIPTS
	friend class NdhsCScriptExecutor;
#endif

private:

	LcTmString							m_sKey;
	LcTmString							m_sVal;

	LcTmOwner<LcCBase>					m_pNext;
	LcCXmlAttr*							m_pPrev;

						void			setVal(const LcTmString& sVal);

protected:
										LcCXmlAttr(const LcTmString& sKey);

public:

	inline				LcCXmlAttr*		getNext()		{ return (LcCXmlAttr*)(m_pNext.ptr()); }
	inline				LcTaString		getName()		{ return m_sKey; }
	inline				LcTaString		getVal ()		{ return m_sVal; }

#ifdef LC_USE_XML_SAVE
	// Change the name of the attribute, used by the packager.
	inline				void			setName(const LcTmString& sKeyName)		{ m_sKey = sKeyName; }
#endif

	// Utilities
	LC_IMPORT static	LcCFont::EStyle	strToFontStyle(const LcTmString& s);
	LC_IMPORT static	bool			strToBool(const LcTmString& s);

	static LcTaOwner<LcCXmlAttr>		create(const LcTmString& sKey);
	virtual 							~LcCXmlAttr();
};

/*-------------------------------------------------------------------------*//**
	Handles the manipulation of XML elements/documents.
*/
class LcCXmlElem : public LcCBase
{
#ifdef IFX_GENERATE_SCRIPTS
	friend class NdhsCScriptGenerator;
#endif

#ifdef IFX_USE_SCRIPTS
	friend class NdhsCScriptExecutor;
#endif

private:

	LcTmString							m_sKey;

	LcCXmlElem*							m_pParent;
	LcTmOwner<LcCBase>					m_pNext;
	LcCXmlElem*							m_pPrev;
	LcTmOwner<LcCBase>					m_pFirstChild;
	LcCXmlElem*							m_pLastChild;
	LcTmOwner<LcCXmlAttr>				m_pFirstAttr;
	LcCXmlAttr*							m_pLastAttr;

#ifdef LC_USE_XML_TEXT_NODES
	LcTmString							m_sText;	// Text content
#endif
										LcCXmlElem(const LcTmString& sKey);

	// Helpers (private)
						LcCXmlElem*		findInternal(const char* szFind, bool bCreateIfNotFound);
	static				LcTaString		unescapeXml	(const LcTmString& s);

#ifdef LC_USE_XML_SAVE
	// Save-specific helpers (private)
	#ifdef IFX_GENERATE_SCRIPTS
		static				bool			writeString	(IFXP_FILE* pFile, const LcTmString& s);
		static				bool			writeXml	(IFXP_FILE* pFile, const LcTmString& s);
	#else
		static				bool			writeString	(FILE* pFile, const LcTmString& s);
		static				bool			writeXml	(FILE* pFile, const LcTmString& s);
	#endif
#endif

#ifdef LC_USE_XML_SAVE
LC_PRIVATE_INTERNAL_PUBLIC:
#endif

	// These methods public if LC_USE_XML_SAVE enabled,
	// otherwise private but retained for internal use by load method
	LC_IMPORT			LcCXmlElem*		createElem(const LcTmString& sKey);
	LC_IMPORT			LcCXmlElem*		attach(LcTmOwner<LcCXmlElem>& pAdd);
	LC_IMPORT			LcTaOwner<LcCXmlElem> detach();
	LC_IMPORT			void			setAttr(const LcTmString& sKey, const LcTmString& sVal);

public:

	// Public API for navigating XML
	inline				LcCXmlElem*		getParent()			{ return m_pParent; }
	inline				LcCXmlElem*		getNext()			{ return (LcCXmlElem*)(m_pNext.ptr()); }
	inline				LcCXmlElem*		getFirstChild()		{ return (LcCXmlElem*)(m_pFirstChild.ptr()); }
	inline				LcCXmlAttr*		getFirstAttr() 		{ return m_pFirstAttr.ptr(); }
	LC_IMPORT			LcCXmlElem*		find(const LcTmString& sFind);
	LC_IMPORT			LcCXmlAttr*		findAttr(const LcTmString& sKey);
	LC_IMPORT			LcTaString		getAttr(const LcTmString& sKey, const LcTmString& sDef);
	inline				LcTaString		getAttr(const LcTmString& sKey)  { return getAttr(sKey, ""); }
	inline				LcTaString		getName()			{ return m_sKey; }
	inline				bool			hasName(const char* name )		{ return 0 == m_sKey.compareNoCase(name); }

	// Public API for loading XML
	LC_IMPORT static	LcTaOwner<LcCXmlElem> create(const LcTmString& sKey);
	LC_IMPORT static	LcTaOwner<LcCXmlElem> load(LcCReadOnlyFile* file, LcTmString& sErr, int& iErrLine);
	LC_IMPORT static	LcTaOwner<LcCXmlElem> load(const LcTmString& sFile, LcTmString& sErr, ERomFileSystem romFileSystem = ETheme);
								virtual ~LcCXmlElem();

	// Optional public API for modifying/saving XML
	LC_IMPORT			void			clearAttrs();
	LC_IMPORT			LcCXmlAttr*		addAttr (const LcTmString& sKey);
#ifdef LC_USE_XML_SAVE
	LC_IMPORT			LcCXmlElem*		findOrCreate(const LcTmString& sFind);
	LC_IMPORT			void			delAttr(const LcTmString& sKey);
	LC_IMPORT			void			setName(const LcTmString& sKey);
	LC_IMPORT			bool			save(const LcTmString& sFile, LcTmString& sErr);
#endif

#ifdef LC_USE_XML_TEXT_NODES
	inline				LcTaString		getText()	{ return m_sText; }
#endif

};

#ifdef LC_USE_XML_TEXT_NODES

/*-------------------------------------------------------------------------*//**
	An iterator for walking the XML DOM structure
*/
class LcTXmlWalker
{

private:
	LcCXmlElem*							m_pLocalRoot;
	LcCXmlElem*							m_pElement;

	public:
										LcTXmlWalker( LcCXmlElem* pLocalRoot );

	inline				LcCXmlElem*		getElement() { return m_pElement; }
	LC_IMPORT			void			next();
	inline				bool			hasMoreElements(){ return  m_pElement != NULL; }
};

/*-------------------------------------------------------------------------*//**
	An iterator across siblings under a common parent
*/
class LcTXmlIterator
{

private:
	LcCXmlElem*							m_pLocalRoot;	// Starting element for iteration
	LcCXmlElem*							m_pElement;	// Current element the iterator is on
	const char*							m_pKey;	// Key used to match element names

	public:
										LcTXmlIterator( LcCXmlElem* pLocalRoot, const char* pKey);

	inline				LcCXmlElem*		getElement() { return  m_pElement; }
	LC_IMPORT			void			next();
	inline				bool			hasMoreElements() { return  m_pElement != NULL;}
};

/*-------------------------------------------------------------------------*//**
	An iterator across whitespace-separated tokens in XML text nodes
*/
class LcTXMLTextTokenizer
{

private:
	LcCXmlElem *						m_pEl;	// The current element being tokenized
	const char *						m_pText; // The text content of *m_pEl

	public:
										LcTXMLTextTokenizer( LcCXmlElem* firstEl );

	LC_IMPORT			LcTaString		getToken();
	LC_IMPORT			int				getIntToken();
	LC_IMPORT			int				getIntTokenNext();
	LC_IMPORT			LcTScalar		getScalarToken();
	LC_IMPORT			LcTScalar		getScalarTokenNext();
	LC_IMPORT			void			next();

	// Are more tokens available ?
	inline				bool		hasMoreTokens() { return  m_pEl && *m_pText; }
};

#endif //LC_USE_XML_TEXT_NODES

#endif

