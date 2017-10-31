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


#ifdef IFX_USE_PLATFORM_FILES

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCPlatformFileSearch> LcCPlatformFileSearch::create()
{
	LcTaOwner<LcCPlatformFileSearch> ref;
	ref.set(new LcCPlatformFileSearch);
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCPlatformFileSearch::construct()
{
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCPlatformFileSearch::LcCPlatformFileSearch()
{
	m_handle = NULL;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCPlatformFileSearch::~LcCPlatformFileSearch()
{
	if (m_handle != NULL)
	{
		fileFindClose();
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCPlatformFileSearch::fileFindFirst(const LcTmString& searchTerm,
											   unsigned char filter,
											   LcTmString& filePath)
{
	bool retVal		= false;
	bool bContinue	= true;

	//currently search for directories only
	if (filter != LC_FILE_SEARCH_TYPE_DIR)
		return false;

	m_filter = filter;

	// Change the search file to the API slash type.
	LcTaString localSearchFile = searchTerm;
	#if (NDHS_PLAT_DIR_SEP_CHAR != NDHS_DIR_SEP_CHAR)
		localSearchFile.replace(NDHS_DIR_SEP_CHAR, NDHS_PLAT_DIR_SEP_CHAR);
	#endif

	if (IFX_SUCCESS == IFXP_File_Find_First(&m_handle, &m_searchObject,
	                                        (char*)localSearchFile.bufUtf8()))
	{
		do
		{
			// We're only interested in directories
			if (IFXP_INFO_DIRECTORY == m_searchObject.info)
			{
				bContinue = false;
			}
		}
		while (bContinue == true && IFXP_File_Find_Next(m_handle, &m_searchObject) == IFX_SUCCESS);

		if (bContinue == false)
		{//directory found
			filePath = m_searchObject.fileName;
 			retVal = true;
		}
		else
			IFXP_File_Find_Close(m_handle, &m_searchObject);
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCPlatformFileSearch::fileFindNext(LcTmString& filePath)
{
	bool retVal		= false;
	bool bContinue	= true;

	while (bContinue == true && IFXP_File_Find_Next(m_handle, &m_searchObject) == IFX_SUCCESS)
	{
		if (IFXP_INFO_DIRECTORY == m_searchObject.info)
		{
			bContinue = false;
		}
	}

	if (bContinue == false)
	{
		// Directory found.
		filePath = m_searchObject.fileName;
		retVal = true;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcCPlatformFileSearch::fileFindClose()
{
	bool retVal = false;

	if (IFX_SUCCESS == IFXP_File_Find_Close(m_handle, &m_searchObject))
	{
		retVal = true;
		m_handle = NULL;
	}

	return retVal;
}

#endif //IFX_USE_PLATFORM_FILES
