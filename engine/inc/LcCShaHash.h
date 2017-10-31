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
#ifndef LcCShaHashH
#define LcCShaHashH

/*
-------------------------------------------------------------------------
	Class for generating hash codes from inputs data streams.
    Uses SHA 1.1 algorithm.
		1. Generates 20-byte signature for any length input
		2. Every bit in output depends on every bit in input
		3. Therefore do not have to use full 20 bytes of signature
		4. Input should contain some hard-coded data (i.e. secret key)
*/
class LcCShaHash
{
private:

    // Internals
    LcTUInt32		m_dw[5];			// Digest
    LcTUInt32		m_cbStream;			// Total bytes submitted so far
    unsigned char	m_overflow[64];		// Bytes pending

    // Helpers
    void 			updateHash64(unsigned char* pBlock);

public:

    // Construction
					LcCShaHash() 	{}
					~LcCShaHash() 	{}

    // API
    void 			init();
    void 			update(unsigned char* pBlock, LcTUInt32 cbBlock);  // Next data
    void 			complete();

    // Results
    void			getRawDigest(LcTUInt32* pBuf);	// 5 LcTUInt32s
    void			getAsLetters(char* buf);		// 24 chars + \0
    void			get8Chars(char* buf);			// 8 chars + \0
};

#endif
