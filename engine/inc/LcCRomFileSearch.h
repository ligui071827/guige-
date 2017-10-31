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
#ifndef LcCRomFileSearchH
#define LcCRomFileSearchH

/*-------------------------------------------------------------------------*//**
	file search class for interrogating the ROMFILE file table.
*/
class LcCRomFileSearch : public LcCFileSearch
{
private:
	LcTmString						m_searchTerm;
	int								m_currentIndex;
	unsigned char					m_filter;
	
	// Force two-phase construction
									LcCRomFileSearch();
protected:
	LC_IMPORT		void			construct();

public:

	LC_IMPORT static LcTaOwner<LcCRomFileSearch> create();

	LC_VIRTUAL						~LcCRomFileSearch();

	LC_VIRTUAL		bool			fileFindFirst(const LcTmString& searchTerm, unsigned char filter, LcTmString& filePath);
	LC_VIRTUAL		bool			fileFindNext(LcTmString& filePath);
	LC_VIRTUAL		bool			fileFindClose();

};

#endif //LcCRomFileSearchH
