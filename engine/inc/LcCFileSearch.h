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
#ifndef LcCFileSearchH
#define LcCFileSearchH

#define LC_FILE_SEARCH_TYPE_ALL		0x00
#define LC_FILE_SEARCH_TYPE_FILE	0x01
#define LC_FILE_SEARCH_TYPE_DIR		0x02

/*-------------------------------------------------------------------------*//**
	Base class for file search objects - pure virtual.
*/
class LcCFileSearch : public LcCBase
{
protected:
	
	// Force two-phase construction
	inline							LcCFileSearch() {}
	
public:

	virtual							~LcCFileSearch() {}

	virtual			bool			fileFindFirst(const LcTmString& searchTerm, unsigned char filter, LcTmString& filePath) = 0;
	virtual			bool			fileFindNext(LcTmString& filePath) = 0;
	virtual			bool			fileFindClose() = 0;

};

#endif //LcCFileSearchH
