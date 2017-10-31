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

#ifdef LC_USE_XML_TEXT_NODES

// The max size of an LcTmString used in string nodes. This should be less than 0xffff and much smaller values, e.g. 0xff seem to work
// much faster.
#define LC_STRING_ELEMENT_LIMIT 0xff

#endif
/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<LcCXmlAttr> LcCXmlAttr::create(const LcTmString& sKey)
{
	LcTaOwner<LcCXmlAttr> ref;
	ref.set(new LcCXmlAttr(sKey));
	//ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcCXmlAttr::LcCXmlAttr(const LcTmString& sKey)
:	m_sKey(sKey)
{
}

/*-------------------------------------------------------------------------*//**
*/
LcCXmlAttr::~LcCXmlAttr()
{
	if (getNext())
		getNext()->m_pPrev = m_pPrev;

	if (m_pPrev)
		m_pPrev->m_pNext = m_pNext;
}

/*-------------------------------------------------------------------------*//**
	Sets the value of an attribute.
*/
void LcCXmlAttr::setVal(const LcTmString& sVal)
{
	m_sVal = sVal;
}

/*-------------------------------------------------------------------------*//**
	LcCXmlElem constructor.
	This is used internally.  New XML elements should be constructed with
	create() and createRoot().
*/
LcCXmlElem::LcCXmlElem(const LcTmString& sKey)
: m_sKey(sKey)
{
}

/*-------------------------------------------------------------------------*//**
	LcCXmlElem destructor.
	This is used internally.  XML elements should be destroyed
	with del(), which also destroys all children.
*/
LcCXmlElem::~LcCXmlElem()
{
#if !((defined(__CC_ARM) && (defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 200000))) || defined(__TMS470__))
	// Destroy all children in reverse order
	// (required to prevent potentially huge recursion in destructor due to ownership structure)
	while (m_pLastChild)
	{
		m_pLastChild->detach();
	}

	clearAttrs();
	detach();
#endif
}

LC_EXPORT LcTaOwner<LcCXmlElem> LcCXmlElem::create(const LcTmString& sKey)
{
	LcTaOwner<LcCXmlElem> ref;
	ref.set(new LcCXmlElem(sKey));
	//ref->construct();
	return ref;
}

#ifdef LC_USE_XML_SAVE
/*-------------------------------------------------------------------------*//**
	Sets the name of an element.
*/
LC_EXPORT void LcCXmlElem::setName(const LcTmString& sKey)
{
	m_sKey = sKey;
}
#endif

/*-------------------------------------------------------------------------*//**
	create constructs a new element as a child of the current one.
*/
LC_EXPORT LcCXmlElem* LcCXmlElem::createElem(const LcTmString& sKey)
{
	LcTaOwner<LcCXmlElem> pNew = LcCXmlElem::create(sKey);
	return attach(pNew);
}

/*-------------------------------------------------------------------------*//**
	Attaches the supplied element (including its subtree, if any) as a child of
	the current node.
*/
LC_EXPORT LcCXmlElem* LcCXmlElem::attach(LcTmOwner<LcCXmlElem>& pAdd)
{
	LcCXmlElem* pAddPtr = pAdd.ptr();

	pAdd->m_pParent = this;

	// Add to end of child list
	pAdd->m_pPrev = m_pLastChild;
	if (m_pLastChild)
		m_pLastChild->m_pNext = pAdd;

	m_pLastChild = pAddPtr;

	if (!m_pFirstChild)
		m_pFirstChild = pAdd;

	return pAddPtr;
}

/*-------------------------------------------------------------------------*//**
	Detaches an element (including its subtree, if any) from its parent tree.
*/
LC_EXPORT LcTaOwner<LcCXmlElem> LcCXmlElem::detach()
{
	LcTaOwner<LcCXmlElem> retVal;

	// Remove any references from sibling and parent nodes
	if (getNext())
		getNext()->m_pPrev = m_pPrev;

	if (m_pPrev)
	{
		retVal.set((LcCXmlElem*)m_pPrev->m_pNext.release());
		m_pPrev->m_pNext = m_pNext;
	}

	if (m_pParent)
	{
		if (m_pParent->m_pLastChild == this)
			m_pParent->m_pLastChild = m_pPrev;

		if (m_pParent->m_pFirstChild.ptr() == this)
		{
			retVal.set((LcCXmlElem*)m_pParent->m_pFirstChild.release());
			m_pParent->m_pFirstChild = m_pNext;
		}
	}

	// Remove references to sibling and parent nodes
	m_pPrev   = NULL;
	m_pParent = NULL;

	if (m_pNext)
	{
		// This should not happen unless something has gone wrong: there was no
		// object found to pass ownership to
		LC_ASSERT(false);

		// Clean up as best we can
		m_pNext.destroy();
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	find locates the specified child of the current node.  The LC_XML_ELEM_SEP
	character can be used to specify multiple levels of the tree.
*/
LcCXmlElem* LcCXmlElem::findInternal(const char* szFind, bool bCreateIfNotFound)
{
	char szThis[LC_XML_BUF_SIZE], szNext[LC_XML_BUF_SIZE];

	// Extract first key, and save the remainder (if any) for the next level.
	lc_strcpy(szThis, szFind);
	lc_strtok(szThis, LC_XML_ELEM_SEP);
	char* pSep = lc_strchr((char*)szFind, LC_XML_ELEM_SEP[0]);
	lc_strcpy(szNext, pSep ? &szThis[lc_strlen(szThis)+1] : "");

	// Now look to see if the 'first' key exists
	LcCXmlElem* pElem = getFirstChild();
	while (pElem)
	{
		if (!pElem->m_sKey.compareNoCase(szThis))
			break;

		pElem = pElem->getNext();
	}

	// If bCreateIfNotFound is set, we now create the node if it doesn't exist.
	if (bCreateIfNotFound && !pElem)
		pElem = createElem(szThis);

	// If pElem is set when we reach here, we look for the next key (if
	// one exists), or return the current one.
	if (pElem)
	{
		if (szNext[0])
			return pElem->findInternal(szNext, bCreateIfNotFound);
		else
			return pElem;
	}

	// If we reach here, we were unsuccessful.
	return NULL;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCXmlElem* LcCXmlElem::find(const LcTmString& sFind)
{
	return findInternal(sFind.bufUtf8(), false);
}

#ifdef LC_USE_XML_SAVE
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCXmlElem* LcCXmlElem::findOrCreate(const LcTmString& sFind)
{
	return findInternal(sFind.bufUtf8(), true);
}
#endif

/*-------------------------------------------------------------------------*//**
	addAttr adds an attribute szKey to the current element. addAttr does _not_
	check whether the attribute already exists, so duplicates are possible.
*/
LC_EXPORT LcCXmlAttr* LcCXmlElem::addAttr(const LcTmString& sKey)
{
	LcTaOwner<LcCXmlAttr> pAttr = LcCXmlAttr::create(sKey);
	LcCXmlAttr* pAttrPtr = pAttr.ptr();

	// Add to end of child list
	pAttr->m_pPrev = m_pLastAttr;

	if (m_pLastAttr)
	{
		m_pLastAttr->m_pNext = pAttr;
	}

	m_pLastAttr = pAttrPtr;

	if (!m_pFirstAttr)
	{
		m_pFirstAttr = pAttr;
	}

	return pAttrPtr;
}

/*-------------------------------------------------------------------------*//**
	findAttr finds the first occurrence of the attribute with the specified name.
*/
LC_EXPORT LcCXmlAttr* LcCXmlElem::findAttr(const LcTmString& sKey)
{
	LcCXmlAttr* pAttr = m_pFirstAttr.ptr();
	while (pAttr)
	{
		if (!pAttr->m_sKey.compareNoCase(sKey))
			break;

		pAttr = pAttr->getNext();
	}
	return pAttr;
}

/*-------------------------------------------------------------------------*//**
	getAttr gets the value of attribute sKey for the current element.
	If the attribute doesn't exist, sDef is returned.
*/
LC_EXPORT LcTaString LcCXmlElem::getAttr(const LcTmString& sKey, const LcTmString& sDef)
{
	LcCXmlAttr* pAttr = findAttr(sKey);

	if (pAttr)
		return pAttr->m_sVal;
	else
		return sDef;
}

/*-------------------------------------------------------------------------*//**
	setAttr sets the value of attribute sKey to sVal for the current element.
	The attribute is created if it doesn't already exist.
*/
LC_EXPORT void LcCXmlElem::setAttr(const LcTmString& sKey, const LcTmString& sVal)
{
/*
	Note that this check was NOT correct, because it didn't allow us
	to distinguish between NO attribute and an attribute that was
	EXPLICITLY set to "".  With this fix, it is now possible to use
	getAttr() with a default value and have "" returned (if explicitly
	set): the default will only be returned if there is NO attribute.

	if (sVal.isEmpty())
	{
		// Delete the attribute if no value is supplied
		delAttr(sKey);
	}
	else

*/
	{
		// Otherwise find (or create) the attribute and set its value
		LcCXmlAttr* pAttr = findAttr(sKey);
		if (!pAttr)
			pAttr = addAttr(sKey);
		pAttr->setVal(sVal);
	}
}

#ifdef LC_USE_XML_SAVE
/*-------------------------------------------------------------------------*//**
	delAttr deletes the first attribute matching sKey if one exists for the
	current element.
*/
LC_EXPORT void LcCXmlElem::delAttr(const LcTmString& sKey)
{
	LcCXmlAttr* pAttr = findAttr(sKey);

	// Delete if found
	if (pAttr)
	{
		if (m_pLastAttr == pAttr)
		{
			m_pLastAttr = pAttr->m_pPrev;
			if (pAttr->m_pPrev)
				pAttr->m_pPrev->m_pNext.release();
		}

		if (m_pFirstAttr.ptr() == pAttr)
		{
			m_pFirstAttr = (LcTmOwner<LcCXmlAttr>&)pAttr->m_pNext;
			if (m_pFirstAttr)
				m_pFirstAttr->m_pPrev = NULL;
		}
		else
		{
			pAttr->m_pPrev->m_pNext = pAttr->m_pNext;
			pAttr->getNext()->m_pPrev = pAttr->m_pPrev;
		}

		delete pAttr;
	}
}
#endif

/*-------------------------------------------------------------------------*//**
	clearAttrs deletes all attributes of the current element.
*/
LC_EXPORT void LcCXmlElem::clearAttrs()
{
	// Destroy all attributes in reverse order
	// (required to prevent potentially huge recursion in destructor due to ownership structure)
	while (m_pLastAttr)
	{
		LcCXmlAttr* penultimateAttr = m_pLastAttr->m_pPrev;

		if (penultimateAttr)
		{
			penultimateAttr->m_pNext.destroy();
		}

		m_pLastAttr = penultimateAttr;
	}

	if (m_pFirstAttr)
	{
		m_pFirstAttr.destroy();
	}

	m_pLastAttr = NULL;
}

/*-------------------------------------------------------------------------*//**
	THIS FUNCTION WAS ONLY USED BY NAVBUILDER
	It may be useful in the future, in which case we should #ifdef it.

	Copy returns an exact duplicate of the current element tree.
*/
/*
LC_EXPORT LcCXmlElem* LcCXmlElem::copy()
{
	bool bNext = false;
	LcCXmlAttr* pAttr;
	LcCXmlElem* pWalk = this;

	// Create a root-level node and copy attributes
	LcCXmlElem* pNew = createRoot(pWalk->m_sKey);
	for (pAttr = pWalk->m_pFirstAttr; pAttr; pAttr = pAttr->m_pNext)
		pNew->addAttr(pAttr->m_sKey)->setVal(pAttr->m_sVal);

	while (pWalk)
	{
		// Breadth-first traversal..
		if (pWalk->m_pFirstChild && !bNext)
		{
			// ..try first child
			pWalk = pWalk->m_pFirstChild;

			// Duplicate node and copy attributes
			pNew = pNew->create(pWalk->m_sKey);
			for (pAttr = pWalk->m_pFirstAttr; pAttr; pAttr = pAttr->m_pNext)
				pNew->addAttr(pAttr->m_sKey)->setVal(pAttr->m_sVal);
		}
		else
		if (pWalk->m_pNext && pWalk != this)
		{
			bNext = false;

			// ..try next sibling
			pWalk = pWalk->m_pNext;

			// Duplicate node and copy attributes
			pNew = pNew->m_pParent->create(pWalk->m_sKey);
			for (pAttr = pWalk->m_pFirstAttr; pAttr; pAttr = pAttr->m_pNext)
				pNew->addAttr(pAttr->m_sKey)->setVal(pAttr->m_sVal);
		}
		else
		{
			// ..walk back up
			while (pWalk != this && !bNext)
			{
				pWalk = pWalk->m_pParent;
				pNew = pNew->m_pParent;

				if (pWalk->m_pNext)
					bNext = true;
			}
		}

		// Break when back to start
		if (pWalk == this)
			break;
	}

	return pNew;
}
*/

#ifdef LC_USE_XML_SAVE
/*-------------------------------------------------------------------------*//**
	writeString is a wrapper for the standard fwrite() function.
*/
#ifdef IFX_GENERATE_SCRIPTS
	bool LcCXmlElem::writeString(IFXP_FILE* pFile, const LcTmString& s)
#else
	bool LcCXmlElem::writeString(FILE* pFile, const LcTmString& s)
#endif
{
	const char* szBuf = s.bufUtf8();

#ifdef IFX_GENERATE_SCRIPTS

	IFX_UINT32 uLen = (int)lc_strlen(szBuf);
	IFX_UINT32 bytes_wrote = 0;

	IFXP_File_Write(*pFile, (char*)szBuf, uLen, &bytes_wrote);

	return (uLen == bytes_wrote);

#else

	unsigned int uLen = (int)lc_strlen(szBuf);

	return (uLen == fwrite(szBuf, sizeof(char), uLen, pFile));

#endif

}
#endif

#ifdef LC_USE_XML_SAVE
/*-------------------------------------------------------------------------*//**
	writeXml is a wrapper for the standard write() function that handles
	escaped XML characters correctly.
*/
#ifdef IFX_GENERATE_SCRIPTS
	bool LcCXmlElem::writeXml(IFXP_FILE* pFile, const LcTmString& s)
#else
	bool LcCXmlElem::writeXml(FILE* pFile, const LcTmString& s)
#endif
{
	const char* szBuf = s.bufUtf8();
	unsigned int uLen = (int)lc_strlen(szBuf);

	for (unsigned int u = 0; u < uLen; u++)
	{
		char sz[16] = "X";
		switch (szBuf[u])
		{
			case '\'' : lc_strcpy(sz, "&apos;"); break;
			case '&' : lc_strcpy(sz, "&amp;"); break;
			case '<' : lc_strcpy(sz, "&lt;" ); break;
			case '>' : lc_strcpy(sz, "&gt;" ); break;
			case '\"': lc_strcpy(sz, "&quot;"); break;
			default  :
				{
					// Escape non-ASCII chars
					unsigned char uc = szBuf[u];
					if (uc < 32 || uc > 127)
						lc_sprintf(sz, "&#%03d;", uc);
					else
						sz[0] = szBuf[u];
				}
		}
		if (!writeString(pFile, sz))
			return false;
	}
	return true;
}
#endif

#ifdef LC_USE_XML_SAVE
/*-------------------------------------------------------------------------*//**
	save generates an XML file from the current element tree.
*/
LC_EXPORT bool LcCXmlElem::save(const LcTmString& sFile, LcTmString& sErr)
{
	#ifdef IFX_GENERATE_SCRIPTS
		IFXP_FILE pFile = NULL;
	#else
		FILE* pFile = NULL;
	#endif

	int i, iLevel = 0;
	LcTaString sTab;

	LcCXmlElem* pWalk = this;

	bool bDone = false, bError = false;


	do // fake 'try' - execute code only once
	{
		// Open file for writing
		#ifdef IFX_GENERATE_SCRIPTS
			// Abort on error
			if (IFXP_File_Create (&pFile, (char*)sFile.bufUtf8()) == IFX_ERROR)
			{
				sErr = "unable to open file";
				bError = true;
				break;
			}
		#else
			// Open file for writing
			pFile = fopen(sFile.bufUtf8(), "wb");

			// Abort on error
			if (NULL == pFile)
			{
				sErr = "unable to open file";
				bError = true;
				break;
			}
		#endif

		// Write XML header
		#ifdef IFX_GENERATE_SCRIPTS
			if (!writeString(&pFile, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"))
		#else
			if (!writeString(pFile, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"))
		#endif
		{
			sErr = "failed to write (1)";
			bError = true;
			break;
		}

		while (pWalk)
		{
			// Indent according to current level
			sTab = "";
			for (i = 0; i < iLevel; i++)
				sTab += '\t';
			#ifdef IFX_GENERATE_SCRIPTS
				if (!writeString(&pFile, sTab))
			#else
				if (!writeString(pFile, sTab))
			#endif
			{
				sErr = "failed to write (2)";
				bError = true;
				break;
			}

			// Open start tag
			// '<element_name'
			#ifdef IFX_GENERATE_SCRIPTS
				if (!writeString(&pFile, "<" + pWalk->m_sKey))
			#else
				if (!writeString(pFile, "<" + pWalk->m_sKey))
			#endif
			{
				sErr = "failed to write (3)";
				bError = true;
				break;
			}

			// Attributes
			LcCXmlAttr* pAttr = pWalk->m_pFirstAttr.ptr();
			while (pAttr)
			{
				// ' attr_name="'
				#ifdef IFX_GENERATE_SCRIPTS
					if (!writeString(&pFile, " " + pAttr->m_sKey + "=\""))
				#else
					if (!writeString(pFile, " " + pAttr->m_sKey + "=\""))
				#endif
				{
					sErr = "failed to write (4)";
					bError = true;
					break;
				}

				// 'attr_value' - written with the helper function writeXml()
				#ifdef IFX_GENERATE_SCRIPTS
					if (pAttr->m_sVal != ""
						&&  !writeXml(&pFile, pAttr->m_sVal))
				#else
					if (pAttr->m_sVal != ""
						&&  !writeXml(pFile, pAttr->m_sVal))
				#endif
				{
					sErr = "failed to write (5)";
					bError = true;
					break;
				}

				// '"'
				#ifdef IFX_GENERATE_SCRIPTS
					if (!writeString(&pFile, "\""))
				#else
					if (!writeString(pFile, "\""))
				#endif
				{
					sErr = "failed to write (6)";
					bError = true;
					break;
				}

				pAttr = (LcCXmlAttr*)pAttr->m_pNext.ptr();
			} // while (pAttr)

			// Abort on error
			if (bError) break;

			// Close start tag
			#ifdef IFX_GENERATE_SCRIPTS
				if (!writeString(&pFile, LcTaString(pWalk->m_pFirstChild ? "" : " /") + ">\n"))
			#else
				if (!writeString(pFile, LcTaString(pWalk->m_pFirstChild ? "" : " /") + ">\n"))
			#endif
			{
				sErr = "failed to write (7)";
				bError = true;
				break;
			}

			// Breadth-first traversal..
			if (pWalk->m_pFirstChild)
			{
				// ..try first child
				iLevel++;
				pWalk = (LcCXmlElem*)pWalk->m_pFirstChild.ptr();
			}
			else
			if (pWalk->m_pNext && pWalk != this)
			{
				// ..try next sibling
				pWalk = (LcCXmlElem*)pWalk->m_pNext.ptr();
			}
			else
			{
				// ..walk back up
				while (pWalk != this)
				{
					pWalk = pWalk->m_pParent;

					iLevel--;

					sTab = "";
					for (i = 0; i < iLevel; i++)
						sTab += '\t';
					#ifdef IFX_GENERATE_SCRIPTS
						if (!writeString(&pFile, sTab))
					#else
						if (!writeString(pFile, sTab))
					#endif
					{
						sErr = "failed to write (8)";
						bError = true;
						break;
					}

					// Write end tag
					#ifdef IFX_GENERATE_SCRIPTS
						if (!writeString(&pFile, "</" + pWalk->m_sKey + ">\n"))
					#else
						if (!writeString(pFile, "</" + pWalk->m_sKey + ">\n"))
					#endif
					{
						sErr = "failed to write (9)";
						bError = true;
						break;
					}

					if (pWalk->m_pNext)
					{
						pWalk = (LcCXmlElem*)pWalk->m_pNext.ptr();
						break;
					}
				} // while (pWalk != this)

				// Abort on error
				if (bError) break;
			}

			// Break when back to start
			if (pWalk == this)
			{
				bDone = true;
				break;
			}
		} // while (pWalk)

	} while (false); // fake 'try' - execute code only once

	// Clean up
	if (pFile != NULL)
	{
		/* Close file and ignore the return status. */
#ifdef IFX_GENERATE_SCRIPTS

	#ifndef	NDHS_JNI_INTERFACE
		IFXP_File_Close((FILE*)pFile);
	#else
		fclose((FILE*)pFile);
	#endif

#else
			fclose(pFile);
#endif
		
		pFile = NULL;
	}


	// Return success or otherwise
		return bDone;
}
#endif

/*-------------------------------------------------------------------------*//**
	For ripping out &xxx; escape sequences
*/
LcTaString LcCXmlElem::unescapeXml(const LcTmString& s)
{
	char szBuf[1024];
	const char* pBufI = s.bufUtf8();
	char* pBufO = szBuf;
	while (*pBufI && pBufO < szBuf + sizeof(szBuf) - 1)
	{
		char c = *pBufI++;
		if (c == '&')
		{
			if (!lc_strncmp(pBufI, "apos;", 5))
			{
				*pBufO++ = '\'';
				pBufI += 5;
			}
			else if (!lc_strncmp(pBufI, "amp;", 4))
			{
				*pBufO++ = '&';
				pBufI += 4;
			}
			else if (!lc_strncmp(pBufI, "rtl;", 4))
			{
				*pBufO++ = (char)0xE2;
				*pBufO++ = (char)0x80;
				*pBufO++ = (char)0xAE;
				pBufI += 4;
			}
			else if (!lc_strncmp(pBufI, "ltr;", 4))
			{
				*pBufO++ = (char)0xE2;
				*pBufO++ = (char)0x80;
				*pBufO++ = (char)0xAD;
				pBufI += 4;
			}
			else if (!lc_strncmp(pBufI, "pdf;", 4))
			{
				*pBufO++ = (char)0xE2;
				*pBufO++ = (char)0x80;
				*pBufO++ = (char)0xAC;
				pBufI += 4;
			}
			else if (!lc_strncmp(pBufI, "lt;", 3))
			{
				*pBufO++ = '<';
				pBufI += 3;
			}
			else if (!lc_strncmp(pBufI, "gt;", 3))
			{
				*pBufO++ = '>';
				pBufI += 3;
			}
			else if (!lc_strncmp(pBufI, "quot;", 5))
			{
				*pBufO++ = '\"';
				pBufI += 5;
			}
			else if (*pBufI == '#' && pBufI[4] == ';')
			{
				int i;
				lc_sscanf(pBufI + 1, "%03d", &i);
				*pBufO++ = (char)i;
				pBufI += 5;
			}
		}
		else
			*pBufO++ = c;
	}

	*pBufO = 0;
	return LcTaString(szBuf);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCXmlElem> LcCXmlElem::load(const LcTmString& name, LcTmString& sErr, ERomFileSystem romFileSystem)
{

	int iErrLine;
	LcTaOwner<LcCXmlElem> elem;
	LcTaOwner<LcCReadOnlyFile> file = LcCReadOnlyFile::openFile(name, romFileSystem);

	// Abort on error
	if (!file)
	{
		sErr = "unable to open file ";
		sErr += name;
		sErr += " ";

		return elem;
	}

	elem = load(file.ptr(), sErr, iErrLine);

	// Include additional info in sErr on failure
	if (!elem)
		sErr += " on line " + LcTaString().fromInt(iErrLine);

	// UID error display
	#if defined(NDHS_JNI_INTERFACE)
		if (!elem)
			NDHS_TRACE_EXT(ENdhsTraceLevelError, NULL, "XML error: " + sErr, name, iErrLine);
		else
			NDHS_TRACE_EXT(ENdhsTraceLevelInfo, NULL, "XML loaded: ", name, -1);
	#endif


	return elem;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCXmlElem> LcCXmlElem::load(LcCReadOnlyFile* file, LcTmString& sErr, int& iErrLine)
{
	typedef enum {
		FindElemStart,
		InElemStart,
		InElemName,
		FindAttrName,
		InAttrName,
		FindAttrEquals,
		FindAttrValue,
		InAttrValue,
		FindElemEnd,
		InElemEnd,
		InDeclStart,
		FindDeclEnd,
		FindCommentEnd
#ifdef LC_USE_XML_TEXT_NODES
		,
		InTextNode
#endif
		} TLcxState;

	TLcxState state = FindElemStart;

	int i = 0, iLineCount = 1, iCharCount = 0;
	char szBuf[LC_XML_BUF_SIZE];

	LcTaString sElemStart, sElemName, sAttrName, sAttrValue, sElemEnd, sDecl;

	LcTaOwner<LcCXmlElem> pRoot = LcCXmlElem::create("temp");
	LcTaOwner<LcCXmlElem> pLoad;
	LcCXmlElem* pThis = pRoot.ptr();

	bool bDone = false, bError = false, bComplete = false;


#ifdef LC_USE_XML_TEXT_NODES
	int stringNodeCount = 0;
#endif
	do // fake 'try' - execute code only once
	{
		// Read a chunk at a time
		size_t len;
		do
		{
			len = file->read(szBuf, sizeof(char), sizeof(szBuf));

			// Parse character by character
			i = 0;
			while (i < int(len))
			{
				// If we come across a line end, we update the line number for error messages
				if (szBuf[i] == '\n')
				{
					iLineCount++;
				}

				// What we do depends on state
				switch (state)
				{

					// Look for the start of an element
					case FindElemStart:

						if (lc_isspace((unsigned char)(szBuf[i])) != 0)
						{
							// Ignore all whitespace
							i++;
						}
						else
						if (bComplete)
						{
							// Abort if we have completed parsing and non-whitespace is found
							sErr = LcTaString("") + "unexpected char '" + szBuf[i] + "'";
							bError = true;
							break;
						}
						else
						if (szBuf[i] == '<')
						{
							// When an element is found start parsing its 'start'
							sElemStart = "";
							sElemName  = "";
							sElemEnd   = "";
							state 		= InElemStart;
							i++;
						}
						else
						{
#ifdef LC_USE_XML_TEXT_NODES
							if(pThis != pRoot.ptr()) {
								pThis = pThis->createElem("_");
								state = InTextNode;
								stringNodeCount = 0;
							}
							else
							{
								// we are outside the root node, so ignore
								i++;
							}
#else
							// FOR NOW we ignore stuff between tags
							i++;
#endif
						}
						break; // FindElemStart

#ifdef LC_USE_XML_TEXT_NODES
					// Save the text node
					case InTextNode:
						if (szBuf[i] == '<')
						{
							// When a '<' is found, we have the whole text node
							pThis = pThis->m_pParent;
							state  = FindElemStart;
						}
						else
						{
							// Add each successive char to the text
							if(stringNodeCount > LC_STRING_ELEMENT_LIMIT)
							{
								// Too many for one string - make a new text node element
								pThis = pThis->m_pParent->createElem("_");
								stringNodeCount = 0;
							}
							pThis->m_sText += szBuf[i++];
							stringNodeCount++;
						}

						break; // InTextNode
#endif

					// The start of an element has been found
					case InElemStart:

						if (szBuf[i] == '?' && sElemStart.isEmpty())
						{
							// Handle directives (e.g. '<?xml ...?>')
							sElemStart += szBuf[i++];
						}
						else
						if (szBuf[i] == '!' && sElemStart.isEmpty())
						{
							// Handle declarations (e.g. '<!elem>')
							sDecl = "";
							state = InDeclStart;
							i++;
						}
						else
						if (szBuf[i] == '/' && sElemStart.isEmpty())
						{
							// Handle closing tags (e.g. '</elem>')
							sElemStart += szBuf[i++];
						}
						else
						if (lc_isalnum((unsigned char)(szBuf[i])) || szBuf[i] == ':' || szBuf[i] == '_' || szBuf[i] == '-' || szBuf[i] == '.')
						{
							// When an AN char is found start parsing the name
							state = InElemName;
						}
						else
						{
							// Abort if any other char is found
							sErr = LcTaString("") + "invalid char (1) '" + szBuf[i] + "'";
							bError = true;
							break;
						}

						break; // InElemStart

					// Read declarations
					case InDeclStart:

						if (lc_isspace((unsigned char)(szBuf[i])) != 0)
						{
							state = FindDeclEnd;
						}
						else
						{
							sDecl += szBuf[i];
							if (sDecl == "--")
							{
								sDecl = "";
								state = FindCommentEnd;
							}
							else
							if (sDecl == "[CDATA[")
							{
								// Abort if CDATA is found - we don't support this
								sErr = LcTaString("") + "CDATA not supported";
								bError = true;
								break;
							}
						}

						i++;

						break; // InDeclStart

					// Ignore comments
					case FindCommentEnd:

						if (sDecl == "--")
						{
							if (szBuf[i] == '>')
								state = FindElemStart;
							else
							{
								// Abort if -- is found - not supported in comments
								sErr = LcTaString("") + "'--' in comment";
								bError = true;
								break;
							}
						}
						else
						if (szBuf[i] == '-')
							sDecl += szBuf[i];
						else
							sDecl = "";

						i++;

						break; // FindCommentEnd

					// Ignore anything inside <! ... >
					case FindDeclEnd:

						if (szBuf[i] == '>')
							state = FindElemStart;

							i++;
						break;

					// Read the element name
					case InElemName:

						if (szBuf[i] == '/' && sElemName.isEmpty())
						{
							// Look for names beginning with a '/'
							sElemName += szBuf[i++];
						}
						else
						if (lc_isalnum((unsigned char)(szBuf[i])) || szBuf[i] == ':' || szBuf[i] == '_' || szBuf[i] == '-' || szBuf[i] == '.')
						{
							// Add each successive AN char to the name
							sElemName += szBuf[i++];
						}
						else
						if (!sElemName.isEmpty() && sElemName != "/")
						{
							// If any other char is found, we must have the full name
							#ifdef DEBUG_XML
								lc_printf("<%s%s", sElemStart.bufUtf8(), sElemName.bufUtf8());
							#endif

							// First we see if the name begins with a '/'
							if (sElemStart == "/")
							{
								// This is a closing tag so (a) there can be no
								// attributes, and (b) we should not create a new element
								state = FindElemEnd;
							}
							else
							{
								// This is an opening tag, so we create a new child element
								pThis = pThis->createElem(sElemStart + sElemName);

								// Start looking for attributes
								state = FindAttrName;
							}
						}
						else
						{
							// Abort if any other char is found
							sErr = LcTaString("") + "invalid char (2) '" + szBuf[i] + "'";
							bError = true;
							break;
						}

						break; // InElemName


					// Look for the start of an attribute
					case FindAttrName:

						if (lc_isspace((unsigned char)(szBuf[i])) != 0)
						{
							// Ignore all whitespace
							i++;
						}
						else
						if (lc_isalnum((unsigned char)(szBuf[i])) || szBuf[i] == ':' || szBuf[i] == '_' || szBuf[i] == '-' || szBuf[i] == '.')
						{
							// When an AN char is found start parsing the name
							sAttrName  = "";
							sAttrValue = "";
							state = InAttrName;
						}
						else
						{
							// If any other char is found, we must be at the end of the element
							state = FindElemEnd;
						}

						break; // FindAttrName


					// Read the attribute name
					case InAttrName:

						if (lc_isalnum((unsigned char)(szBuf[i])) || szBuf[i] == ':' || szBuf[i] == '_' || szBuf[i] == '-' || szBuf[i] == '.')
						{
							// Add each successive AN char to the name
							sAttrName += szBuf[i++];
						}
						else
						{
							// When anything else is found assume name is complete and
							// start looking for '='
							state = FindAttrEquals;
						}

						break; // InAttrName


					// Look for the '=' sign between the attribute and its value
					case FindAttrEquals:

						if (lc_isspace((unsigned char)(szBuf[i])) != 0)
						{
							// Ignore all whitespace
							i++;
						}
						else
						if (szBuf[i] == '=')
						{
							// When an '=' is found start looking for the value
							state = FindAttrValue;
							i++;
						}
						else
						{
							// Abort if any other char is found
							sErr = LcTaString("") + "invalid char (3) '" + szBuf[i] + "'";
							bError = true;
							break;
						}

						break; // FindAttrEquals


					// Look for the start of the attribute's value
					case FindAttrValue:

						if (lc_isspace((unsigned char)(szBuf[i])) != 0)
						{
							// Ignore all whitespace
							i++;
						}
						else
						if (szBuf[i] == '\"')
						{
							// When an '"' is found start parsing the value
							state = InAttrValue;
							i++;
						}
						else
						{
							// Abort if any other char is found
							sErr = LcTaString("") + "invalid char (4) '" + szBuf[i] + "'";
							bError = true;
							break;
						}

						break; // FindAttrValue


					// Read the attribute value
					case InAttrValue:

						if (szBuf[i] == '\"')
						{
							// When a final '"' is found, we have the whole attribute value
							pThis->setAttr(sAttrName, unescapeXml(sAttrValue));

							#ifdef DEBUG_XML
								lc_printf(" %s=\"%s\"", sAttrName.bufUtf8(), sAttrValue.bufUtf8());
							#endif

							// Look for more attributes
							state = FindAttrName;
							i++;
						}
						else
						{
							// Add each successive char to the value
							sAttrValue += szBuf[i++];
						}

						break; // InAttrValue


					// Look for the end of the element
					case FindElemEnd:

						if (lc_isspace((unsigned char)(szBuf[i])) != 0)
						{
							// Ignore all whitespace
							i++;
						}
						else
						{
							// When anything else is found assume we have reached the end
							state = InElemEnd;
						}

						break; // FindElemEnd


					// The end of the element has been found
					case InElemEnd:

						if (szBuf[i] == '?' && sElemEnd.isEmpty())
						{
							// Handle non-standard elements (e.g. '?' directives)
							sElemEnd += szBuf[i++];
						}
						else
						if (szBuf[i] == '/' && sElemEnd.isEmpty())
						{
							// This is an element with no children
							sElemEnd += szBuf[i++];
						}
						else
						if (szBuf[i] == '>')
						{
							// We have found the end of the element
							#ifdef DEBUG_XML
								lc_printf("%s>\n", sElemEnd.bufUtf8());
							#endif

							// Now we process the element, depending on sElemStart and sElemEnd

							if (sElemStart == "/" && sElemEnd.isEmpty())
							{
								// First look for a closing tag
								if (sElemName.compareNoCase(pThis->getName()) != 0)
								{
									// Abort if name doesn't match the current node's parent
									sErr = "mismatched tag (1) '" + sElemName + "'";
									bError = true;
									break;
								}

								// If we reach here, we are OK, and the current node is closed
								pThis = pThis->m_pParent;
							}
							else
							if (sElemStart.isEmpty()  && sElemEnd == "/")
							{
								// This is a 'complete' tag
								pThis = pThis->m_pParent;
							}
							else
							if (sElemStart == "?" && sElemEnd == "?")
							{
								// This is a directive - for now we delete these
								LcCXmlElem* pDel = pThis;
								pThis = pThis->m_pParent;
								pDel->detach();
								pDel = NULL;
							}
							else
							if (sElemStart.isEmpty()  && sElemEnd.isEmpty())
							{
								// This is an opening tag, so do nothing
							}
							else
							{
								// Abort if the tag fits none of the above criteria
								sErr = "mismatched tag (2) '" + sElemName + "'";
								bError = true;
								break;
							}

							// If we reach here, we have stepped back up to the current
							// node's parent.  If we have stepped all the way back to the
							// root node, we set a flag indicating that processing is complete
							// and thus _should_ now finish.  If more non-whitespace chars
							// are subsequently found, we must raise an error.
							if (pThis == pRoot.ptr() && pThis->getFirstChild())
							{
								bComplete = true;
							}

							// Return to looking for a new element
							state = FindElemStart;
							i++;
						}
						else
						{
							// Abort if any other char is found
							sErr = LcTaString("") + "invalid char (5) '" + szBuf[i] + "'";
							bError = true;
							break;
						}

						break; // InElemStart


					default:

						// Abort if we reach an unhandled state
						sErr = "unhandled state";
						bError = true;
						break;

				} // switch (state)

				// Abort on error
				if (bError) break;

			} // while (i < len)

			// Abort on error
			if (bError) break;

			iCharCount = iCharCount + (int)len;

		} while (len > 0);

		// Abort on error
		if (bError) break;

		// If we reach here we have processed the whole file
		if (!bComplete)
		{
			// Abort if parsing has not been completed
			sErr = "unexpected end of file";
			bError = true;
			break;
		}

		// If we reach here we're OK
		bDone = true;

	} while (false); // fake 'try' - execute code only once

	// If we were successful, we detach the loaded tree from our temporary
	// 'root' node.
	if (bDone && pRoot)
	{
		if (pRoot->m_pFirstChild)
		{
			pLoad = pRoot->getFirstChild()->detach();
		}
	}

	// Clean up
	file->close();

	// We delete our temporary root node (which, in the event of a partial
	// parse, will cause all children to be deleted too).
	if (pRoot)
	{
		pRoot.destroy();
	}

	// Finally make sure error line is returned on failure
	if (bError)
		iErrLine = iLineCount;

	// Return new element tree
	return pLoad;
}

/*-------------------------------------------------------------------------*//**
	THIS FUNCTION WAS ONLY USED BY NAVBUILDER
	It may be useful in the future, in which case we should #ifdef it.

	Utility function - change the name of 'szOld' elements to 'szNew'.
*/
/*
LC_EXPORT void LcCXmlElem::replaceTags(const LcTmString& sOld, const LcTmString& sNew)
{
	LcCXmlElem* pWalk = this;
	while (pWalk)
	{
		// Change name if a match is found
		if (pWalk->getName().compareNoCase(sOld) == 0)
			pWalk->setName(sNew);

		// Walk the tree
		if (pWalk->getFirstChild())
			pWalk = pWalk->getFirstChild();
		else
		{
			while (pWalk && !pWalk->getNext())
				pWalk = pWalk->getParent();

			if (pWalk)
				pWalk = pWalk->getNext();
		}
	}
}
*/

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCFont::EStyle LcCXmlAttr::strToFontStyle(const LcTmString& s)
{
	int i = 0;
	if (s.find("bold") != LcTmString::npos)
		i |= LcCFont::BOLD;
	if (s.find("italic") != LcTmString::npos)
		i |= LcCFont::ITALIC;
	return (LcCFont::EStyle)i;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCXmlAttr::strToBool(const LcTmString& s)
{
	// Assumes string is ASCII representation of float
	return (s.compare("1") == 0)
		|| (s.compareNoCase("true") == 0)
		|| (s.compareNoCase("yes") == 0)
		|| (s.compareNoCase("y"  ) == 0);
}

#ifdef LC_USE_XML_TEXT_NODES

/*-------------------------------------------------------------------------*/
/**
Create an iterator over all the child elements of 'pLocalRoot' which have the name '*pKey'

PRE: pLocalRoot != NULL
*/

LcTXmlIterator::LcTXmlIterator(LcCXmlElem* pLocalRoot, const char* pKey)
{
	m_pLocalRoot = pLocalRoot;
	m_pKey = pKey;
	m_pElement = pLocalRoot->getFirstChild();
	bool found = false;

	// Go to the first child with name sKey
	while(m_pElement && !found)
	{
		if(m_pElement->getName().compareNoCase(m_pKey) == 0)
		{
			found = true;
		}
		else
		{
			m_pElement = m_pElement->getNext();
		}
	}
}


/*-------------------------------------------------------------------------*//**
Move on to the next valid sibling element.
*/
LC_EXPORT void LcTXmlIterator::next()
{
	m_pElement = m_pElement->getNext();
	bool found = false;
	while(m_pElement && !found)
	{
		if(m_pElement->getName().compareNoCase(m_pKey) == 0)
		{
			found = true;
		}
		else
		{
			m_pElement = m_pElement->getNext();
		}
	}
}


/*-------------------------------------------------------------------------*//**
Create a depth-first walker for all the descendent elements of 'pLocalRoot'
*/
LcTXmlWalker::LcTXmlWalker(LcCXmlElem* pLocalRoot)
{
	m_pLocalRoot = pLocalRoot;
	m_pElement = pLocalRoot->getFirstChild();
}


/*-------------------------------------------------------------------------*//**
Walk to the next valid element, in depth-first order.
*/
LC_EXPORT void LcTXmlWalker::next()
{
	if (m_pElement)
	{
		// Not at end
		if (m_pElement->getFirstChild())
		{
			// There is a child, so go to it
			m_pElement = m_pElement->getFirstChild();
		}
		else if (m_pElement->getNext())
		{
			// No child but there is a sibling, so go to it
			m_pElement = m_pElement->getNext();
		}
		else
		{
			// No child or sibling so backtrack until we find a sibling or get back to the root
			do
			{
				m_pElement = m_pElement->getParent();
			}
			while (m_pElement != m_pLocalRoot && !m_pElement->getNext());

			if (m_pElement != m_pLocalRoot)
			{
				// Not back at root => there is a sibling, so go to it
				m_pElement = m_pElement->getNext();
			}
			else
			{
				// We're back at the root, so indicate no more elements
				m_pElement = NULL;
			}
		}
	}
}


/*-------------------------------------------------------------------------*//**
Create a tokeniser

PRE: firstEl != NULL
PRE: firstEl->sTExt is read-only
*/

LcTXMLTextTokenizer::LcTXMLTextTokenizer(LcCXmlElem* firstEl)
{
	m_pEl = firstEl;
	m_pText = m_pEl->getText().bufUtf8();

	// Skip leading whitespace
	bool done = false;
	while(m_pEl && !done)
	{
		while(*m_pText && !done)
		{
			if(lc_isspace((unsigned char)(*m_pText)))
			{
				m_pText++;
			}
			else
			{
				done = true;
			}
		}

		if(!done)
		{
			m_pEl = m_pEl->getNext();
			m_pText = m_pEl ? m_pEl->getText().bufUtf8(): "";
		}
	}
}

/*-------------------------------------------------------------------------*//**
Get the current token
*/
LC_EXPORT LcTaString LcTXMLTextTokenizer::getToken()
{
	LcTaString result;
	LcCXmlElem* el = m_pEl;
	const char * pText = m_pText;

	bool done = false;
	while(el && *pText && !done)
	{
		while(*pText && !done)
		{
			if(!lc_isspace((unsigned char)(*pText)))
			{
				result += *pText;
				pText++;
			}
			else
			{
				done = true;
			}
		}

		if(!done)
		{
			el = el->getNext();
			pText = el ? el->getText().bufUtf8(): "";
		}
	}
	return result;
}

/*-------------------------------------------------------------------------*//**
Get the current token as an int.
Ignore any irrelevant non-space characters.
*/
LC_EXPORT int LcTXMLTextTokenizer::getIntToken()
{
	LcCXmlElem* el = m_pEl;
	const char *pText = m_pText;
	unsigned char c;

	bool done = false;
	enum {
		Starting,
		Integral
	} state = Starting;

	int result = 0;
	bool negative = false;

	state = Starting;
	while(el && *pText && !done)
	{
		while(*pText && !done)
		{
			c = (unsigned char)(*pText);
			switch (state)
			{
			case Starting:
				if( c == '-')
				{
					negative = true;
					state = Integral;
					pText++;
				}
				else if( '0' <= c && c <= '9' )
				{
					result = result * 10 + (c - '0');
					state = Integral;
					pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else
				{
					pText++;
				}
				break;

			case Integral:
				if( '0' <= c && c <= '9' )
				{
					result = result * 10 + (c - '0');
					pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else
				{
					pText++;
				}
				break;
			}
		}
		if(!done)
		{
			el = el->getNext();
			pText = el ? el->getText().bufUtf8(): "";
		}
	}
	return (negative) ? -result : result;
}

/*-------------------------------------------------------------------------*//**
Get the current token as an int and move on to the start of the next token.
Ignore any irrelevant non-space characters.
*/
LC_EXPORT int LcTXMLTextTokenizer::getIntTokenNext()
{
	unsigned char c;

	bool done = false;
	enum {
		Starting,
		Integral
	} state = Starting;

	int result = 0;
	bool negative = false;

	state = Starting;
	while(m_pEl && *m_pText && !done)
	{
		while(*m_pText && !done)
		{
			c = (unsigned char)(*m_pText);
			switch (state)
			{
			case Starting:
				if( c == '-')
				{
					negative = true;
					state = Integral;
					m_pText++;
				}
				else if( '0' <= c && c <= '9' )
				{
					result = result * 10 + (c - '0');
					state = Integral;
					m_pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else
				{
					m_pText++;
				}
				break;

			case Integral:
				if( '0' <= c && c <= '9' )
				{
					result = result * 10 + (c - '0');
					m_pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else
				{
					m_pText++;
				}
				break;
			}
		}
		if(!done)
		{
			m_pEl = m_pEl->getNext();
			m_pText = m_pEl ? m_pEl->getText().bufUtf8(): "";
		}
	}

	done = false;
	// Skip subsequent whitespace
	while (m_pEl && ! done)
	{
		while (*m_pText && !done)
		{
			if (lc_isspace((unsigned char)(*m_pText)))
			{
				m_pText++;
			}
			else
			{
				done = true;
			}
		}

		if (!done)
		{
			m_pEl = m_pEl->getNext();
			m_pText = m_pEl ? m_pEl->getText().bufUtf8() : "";
		}
	}
	return (negative) ? -result: result;
}


/*-------------------------------------------------------------------------*//**
Get the current token as a scalar.
Ignore any irrelevant non-space characters.
Exponent format is not recognised.
*/
LC_EXPORT LcTScalar LcTXMLTextTokenizer::getScalarToken()
{
	LcCXmlElem* el = m_pEl;
	const char *pText = m_pText;
	unsigned char c;

	bool done = false;
	enum {
		Integral,
		NegativeIntegral,
		Fractional,
		Exponent,
		NegativeExponent
	} state = Integral;

	double result = 0.0;
	double fraction = 0.0;
	double divisor = 10;
	int exponent = 0;
	bool negative = false;
	bool negativeExponent = false;

	state = Integral;
	while(el && *pText && !done)
	{
		while(*pText && !done)
		{
			c = (unsigned char)(*pText);
			switch (state)
			{
			case Integral:
				if( '0' <= c && c <= '9' )
				{
					result = result * 10.0 + (c - '0');
					state = Integral;
					pText++;
				}
				else if( c == '-')
				{
					negative = true;
					state = NegativeIntegral;
					pText++;
				}
				else if( c == '.')
				{
					state = Fractional;
					pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else if( c == 'e' || c == 'E' )
				{
					state = Exponent;
					pText++;
				}
				else
				{
					pText++;
				}
				break;

			case NegativeIntegral:
				if( '0' <= c && c <= '9' )
				{
					result = result * 10.0 + (c - '0');
					pText++;
				}
				else if( c == '.')
				{
					state = Fractional;
					pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else if( c == 'e' || c == 'E' )
				{
					state = Exponent;
					pText++;
				}
				else
				{
					pText++;
				}
				break;

			case Fractional:
				if( '0' <= c && c <= '9' )
				{
					fraction = fraction + (double)(c - '0') / divisor;
					divisor = divisor*10;
					pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else if( c == 'e' || c == 'E' )
				{
					state = Exponent;
					pText++;
				}
				else
				{
					pText++;
				}
				break;
			
			case Exponent:
				if( '0' <= c && c <= '9' )
				{
					exponent = exponent * 10 + (c - '0');
					pText++;
				}
				else if( c == '-')
				{
					negativeExponent = true;
					state = NegativeExponent;
					pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else
				{
					pText++;
				}
				break;
			
			case NegativeExponent:
				if( '0' <= c && c <= '9' )
				{
					exponent = exponent * 10 + (c - '0');
					pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else
				{
					pText++;
				}
				break;
			}
		}
		if(!done)
		{
			el = el->getNext();
			pText = el ? el->getText().bufUtf8(): "";
		}
	}
	result = ((negative) ? -(result + fraction) : (result + fraction));
	if( exponent != 0 )
	{
		// Avoid using pow()
		if(negativeExponent)
		{
			for( int i=0; i < exponent; i++)
			{
				result = result / 10.0;
			}
		}
		else
		{
			for( int i=0; i < exponent; i++)
			{
				result = result * 10.0;
			}
		}
	}
	return (LcTScalar) result;
}


/*-------------------------------------------------------------------------*//**
Get the current token as a scalar and move on to the start of the next token.
Ignore any irrelevant non-space characters.
Exponent format is not recognised.
*/
LC_EXPORT LcTScalar LcTXMLTextTokenizer::getScalarTokenNext()
{
	unsigned char c;

	bool done = false;
	enum {
		Integral,
		NegativeIntegral,
		Fractional,
		Exponent,
		NegativeExponent
	} state = Integral;

	double result = 0.0;
	double fraction = 0.0;
	double divisor = 10;
	int exponent = 0;
	bool negative = false;
	bool negativeExponent = false;

	state = Integral;
	while(m_pEl && *m_pText && !done)
	{
		while(*m_pText && !done)
		{
			c = (unsigned char)(*m_pText);
			switch (state)
			{
			case Integral:
				if( '0' <= c && c <= '9' )
				{
					result = result * 10.0 + (c - '0');
					state = Integral;
					m_pText++;
				}
				else if( c == '-')
				{
					negative = true;
					state = NegativeIntegral;
					m_pText++;
				}
				else if( c == '.')
				{
					state = Fractional;
					m_pText++;
				} 
				else if(lc_isspace(c))
				{
					done = true;
				}
				else if( c == 'e' || c == 'E' )
				{
					state = Exponent;
					m_pText++;
				}
				else
				{
					m_pText++;
				}
				break;

			case NegativeIntegral:
				if( '0' <= c && c <= '9' )
				{
					result = result * 10.0 + (c - '0');
					m_pText++;
				}
				else if( c == '.')
				{
					state = Fractional;
					m_pText++;
				} 
				else if(lc_isspace(c))
				{
					done = true;
				}
				else if( c == 'e' || c == 'E' )
				{
					state = Exponent;
					m_pText++;
				}
				else
				{
					m_pText++;
				}
				break;

			case Fractional:
				if( '0' <= c && c <= '9' )
				{
					fraction = fraction + (double)(c - '0') / divisor;
					divisor = divisor*10;
					m_pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else if( c == 'e' || c == 'E' )
				{
					state = Exponent;
					m_pText++;
				}
				else
				{
					m_pText++;
				}
				break;
			
			case Exponent:
				if( '0' <= c && c <= '9' )
				{
					exponent = exponent * 10 + (c - '0');
					m_pText++;
				}
				else if( c == '-')
				{
					negativeExponent = true;
					state = NegativeExponent;
					m_pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else
				{
					m_pText++;
				}
				break;

			case NegativeExponent:
				if( '0' <= c && c <= '9' )
				{
					exponent = exponent * 10 + (c - '0');
					m_pText++;
				}
				else if(lc_isspace(c))
				{
					done = true;
				}
				else
				{
					m_pText++;
				}
				break;
			}
		}
		if(!done)
		{
			m_pEl = m_pEl->getNext();
			m_pText = m_pEl ? m_pEl->getText().bufUtf8(): "";
		}
	}

	done = false;
	// Skip subsequent whitespace
	while (m_pEl && ! done)
	{
		while (*m_pText && !done)
		{
			if (lc_isspace((unsigned char)(*m_pText)))
			{
				m_pText++;
			}
			else
			{
				done = true;
			}
		}

		if (!done)
		{
			m_pEl = m_pEl->getNext();
			m_pText = m_pEl ? m_pEl->getText().bufUtf8() : "";
		}
	}
	result = ((negative) ? -(result + fraction) : (result + fraction));
	if( exponent != 0 )
	{
		// Avoid using pow()
		if(negativeExponent)
		{
			for( int i=0; i < exponent; i++)
			{
				result = result / 10.0;
			}
		}
		else
		{
			for( int i=0; i < exponent; i++)
			{
				result = result * 10.0;
			}
		}
	}
	return (LcTScalar) result;
}

/*-------------------------------------------------------------------------*//**
Move on to the next token
*/
LC_EXPORT void LcTXMLTextTokenizer::next()
{
	bool done = false;

	// Skip current token
	while (m_pEl && !done)
	{
		while (*m_pText && !done)
		{
			if (!lc_isspace((unsigned char)(*m_pText)))
			{
				m_pText++;
			}
			else
			{
				done = true;
			}
		}

		if (!done)
		{
			m_pEl = m_pEl->getNext();
			m_pText = m_pEl ? m_pEl->getText().bufUtf8() : "";
		}
	}

	done = false;

	// Skip subsequent whitespace
	while (m_pEl && ! done)
	{
		while (*m_pText && !done)
		{
			if (lc_isspace((unsigned char)(*m_pText)))
			{
				m_pText++;
			}
			else
			{
				done = true;
			}
		}

		if (!done)
		{
			m_pEl = m_pEl->getNext();
			m_pText = m_pEl ? m_pEl->getText().bufUtf8() : "";
		}
	}
}



#endif //LC_USE_XML_TEXT_NODES
