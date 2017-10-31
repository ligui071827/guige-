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


#ifdef IFX_USE_ROM_FILES

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCRomFileSearch> LcCRomFileSearch::create()
{
	LcTaOwner<LcCRomFileSearch> ref;
	ref.set(new LcCRomFileSearch);
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCRomFileSearch::construct()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCRomFileSearch::LcCRomFileSearch()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCRomFileSearch::~LcCRomFileSearch()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCRomFileSearch::fileFindFirst(const LcTmString& searchTerm,
											   unsigned char filter,
											   LcTmString& filePath)
{
	bool retVal = false;

	// check ROM table for search string
	//prune the wild-card character
	LcTaString searchFile = searchTerm.subString(0, searchTerm.find("*", 0));

	if(searchFile.isEmpty() == true)
		return false;

	//currently we only support searching for directories
	if(filter != LC_FILE_SEARCH_TYPE_DIR)
		return false;

	m_filter = filter;

	searchFile = searchFile.toLower();
	for (unsigned i = 0; i < g_romFileCount; i++)
	{
		LcTaString name = g_romFileTable[i].name;
		name = name.toLower();
		int termPos = name.find(searchFile.toLower());
		if (termPos != LcTaString::npos)
		{
			//ignore the sig files
			if(name.find(".sig") != LcTaString::npos)
				continue;

			// If there is something in the ROM table with the right path
			// we want the root directory of the package...we expect name to
			// be of the form "packages/<package name>/<file>"
			// we need to isolate the packages/<package name>/ part
			int separatorPos = name.find(NDHS_DIR_SEP, termPos + 1);
			termPos += separatorPos + 1;
			separatorPos = name.find(NDHS_DIR_SEP, separatorPos + 1);
			filePath = name.subString(termPos, separatorPos - termPos);
			name = name.subString(0, separatorPos + 1);
			
			retVal = true;

			//store the search term for subsequent calls
			m_searchTerm = searchFile;

			//now skip on through the table, ignoring all other entries that would match the precise string
			//returned...otherwise the next 'find next' call would return the same value as before

			//assume the worst case
			m_currentIndex = g_romFileCount;
			for(unsigned j = i + 1; j < g_romFileCount; j++)
			{
				LcTaString nextname = g_romFileTable[j].name;
				nextname = nextname.toLower();
				if(nextname.find(name) == LcTaString::npos)
				{
					//store the index for subsequent calls
					m_currentIndex = j;

					//finish up inner loop
					break;
				}
			}

			//job done - quit outer loop
			break;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCRomFileSearch::fileFindNext(LcTmString& filePath)
{
	bool retVal = false;

	for (unsigned i = m_currentIndex; i < g_romFileCount; i++)
	{
		LcTaString name = g_romFileTable[i].name;
		name = name.toLower();
		int termPos = name.find(m_searchTerm.toLower());
		if (termPos != LcTaString::npos)
		{
			//ignore the sig files
			if(name.find(".sig") != LcTaString::npos)
				continue;

			// If there is something in the ROM table with the right path
			// we want the root directory of the package...we expect name to
			// be of the form "packages/<package name>/<file>"
			// we need to isolate the packages/<package name>/ part
			int separatorPos = name.find(NDHS_DIR_SEP, termPos + 1);
			termPos += separatorPos + 1;
			separatorPos = name.find(NDHS_DIR_SEP, separatorPos + 1);
			filePath = name.subString(termPos, separatorPos - termPos);
			name = name.subString(0, separatorPos + 1);

			retVal = true;

			//now skip on through the table, ignoring all other entries that would match the precise string
			//returned...otherwise the next 'find next' call would return the same value as before

			//assume the worst case
			m_currentIndex = g_romFileCount;
			for(unsigned j = i + 1; j < g_romFileCount; j++)
			{
				LcTaString nextname = g_romFileTable[j].name;
				nextname = nextname.toLower();
				if(nextname.find(name) == LcTaString::npos)
				{
					//store the index for subsequent calls
					m_currentIndex = j;

					//finish up inner loop
					break;
				}
			}

			break;
		}
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCRomFileSearch::fileFindClose()
{
	return true;
}

#endif //IFX_USE_ROM_FILES
