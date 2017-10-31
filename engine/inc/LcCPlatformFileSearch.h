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
#ifndef LcCPlatformFileSearchH
#define LcCPlatformFileSearchH
/*
extern "C"
{
	#include "inflexionui/engine/ifxui_porting.h"
}*/
//#include "inflexionui/engine/inc/LcCReadOnlyFile.h"

/*-------------------------------------------------------------------------*//**
	File search class for interrogating an IFXP wrapped file system.
*/
class LcCPlatformFileSearch : public LcCFileSearch
{
private:
	IFXP_FILE						m_handle;
	IFXP_SEARCH						m_searchObject;
	unsigned char					m_filter;

	// Force two-phase construction
									LcCPlatformFileSearch();

protected:
	LC_IMPORT		void			construct();

public:

	LC_IMPORT static LcTaOwner<LcCPlatformFileSearch> create();

	LC_VIRTUAL						~LcCPlatformFileSearch();

	LC_VIRTUAL		bool			fileFindFirst(const LcTmString& searchTerm, unsigned char filter, LcTmString& filePath);
	LC_VIRTUAL		bool			fileFindNext(LcTmString& filePath);
	LC_VIRTUAL		bool			fileFindClose();

};

#endif //LcCPlatformFileSearchH
