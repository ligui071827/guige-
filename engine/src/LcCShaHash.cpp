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

#ifdef LcCShaHashH

// Rotate DWORD a left by b bits
#define ROTLEFT(a, b)  (((a) << (b)) | ((a) >> (32 - (b))))

/*
-------------------------------------------------------------------------
	Helper - reverse byte order of given DWORD
*/
void bigEnd(LcTUInt32* pdwVal)
{
	unsigned char	b;
	unsigned char*	pb = (unsigned char*)pdwVal;

	b = pb[0];
	pb[0] = pb[3];
	pb[3] = b;

	b = pb[1];
	pb[1] = pb[2];
	pb[2] = b;
}

/*
-------------------------------------------------------------------------
	Prepares a digest for calls to updateHash and CloseHash
*/
void LcCShaHash::init()
{
	// Initialize digest
	m_dw[0] = 0x67452301L;
	m_dw[1] = 0xEFCDAB89L;
	m_dw[2] = 0x98BADCFEL;
	m_dw[3] = 0x10325476L;
	m_dw[4] = 0xC3D2E1F0L;

	// Data count is zero; overflow buffer contents don't matter
	m_cbStream = 0;
}

/*-------------------------------------------------------------------------*//**
	Helper - process aligned 64 byte data block into digest
*/
void LcCShaHash::updateHash64(unsigned char* pBlock)
{
	// Initialize working digest buffer
	LcTUInt32 dwIDA = m_dw[0];
	LcTUInt32 dwIDB = m_dw[1];
	LcTUInt32 dwIDC = m_dw[2];
	LcTUInt32 dwIDD = m_dw[3];
	LcTUInt32 dwIDE = m_dw[4];
	LcTUInt32 dwNewA;

	// W functions - avoid working out every time
    LcTUInt32 dwFuncW[80];

	// Process 80 rounds on the given block
	int t;
	for (t = 0; t < 80; t++)
	{
		// Work out W function
		if (t < 16)
		{
			// First 16 correspond to input data
			dwFuncW[t] = ((LcTUInt32*)pBlock)[t];
	
#ifndef IFX_USE_BIG_ENDIAN
			bigEnd(&dwFuncW[t]);
#endif
		}
		else
		{
			// Rest are combinations of previous values
			dwFuncW[t] = dwFuncW[t - 16]
					^	 dwFuncW[t - 14]
					^	 dwFuncW[t - 8]
					^	 dwFuncW[t - 3];
			
			// SHA standard updated due to security flaw
			dwFuncW[t] = ROTLEFT(dwFuncW[t], 1);
		}

		// Derive new value of A, encoding message content
		dwNewA =  ROTLEFT(dwIDA, 5);
		dwNewA += dwFuncW[t];
		dwNewA += dwIDE;

		// Shake things up a bit
		if (t < 20)
			dwNewA += 0x5A827999L + ((dwIDB & dwIDC) | (~dwIDB & dwIDD));
		else if (t < 40)
			dwNewA += 0x6ED9EBA1L + (dwIDB ^ dwIDC ^ dwIDD);
		else if (t < 60)
			dwNewA += 0x8F1BBCDCL + ((dwIDB & dwIDC) | (dwIDB & dwIDD) | (dwIDC & dwIDD));
		else
			dwNewA += 0xCA62C1D6L + (dwIDB ^ dwIDC ^ dwIDD);

		// Set new values for next cycle
		dwIDE = dwIDD;
		dwIDD = dwIDC;
		dwIDC = ROTLEFT(dwIDB, 30);
		dwIDB = dwIDA;
		dwIDA = dwNewA;
	}

	// Add working buffer to digest (modulo 2^32)
	m_dw[0] += dwIDA;
	m_dw[1] += dwIDB;
	m_dw[2] += dwIDC;
	m_dw[3] += dwIDD;
	m_dw[4] += dwIDE;
}

/*-------------------------------------------------------------------------*//**
	Allows a hash key to be incrementally formed for a datastream
*/
void LcCShaHash::update(unsigned char* pBlock, LcTUInt32 cbBlock)
{
	// Check for overflowed data before updating length
	LcTUInt32 cbOverflow = m_cbStream % 64;
	m_cbStream += cbBlock;

	// If there is overflowed data...
	if (cbOverflow > 0)
	{
		// Do we have 64 bytes to process yet?
		LcTUInt32 cbRoom = 64 - cbOverflow;
		if (cbBlock >= cbRoom)
		{
			// Yes; prepare and process the buffer
			memcpy(m_overflow + cbOverflow, pBlock, cbRoom);
			updateHash64(m_overflow);

			// Skip the processed bytes
			pBlock += cbRoom;
			cbBlock -= cbRoom;
		}
		else
		{
			// No; append data to overflow buffer
			memcpy(m_overflow + cbOverflow, pBlock, cbBlock);
			return;
		}
	}

	// Process 64-byte chunks in turn
	while (cbBlock >= 64)
	{
		// Process
		updateHash64(pBlock);

		// Skip processed bytes
		pBlock += 64;
		cbBlock -= 64;
	}

	// If there are unprocessed bytes, put into overflow buffer
	if (cbBlock > 0)
		memcpy(m_overflow, pBlock, cbBlock);
}

/*-------------------------------------------------------------------------*//**
	Completes incremental formation of a hash key for a datastream
*/
void LcCShaHash::complete()
{
	// Check for overflowed data
	LcTUInt32 cbOverflow = m_cbStream % 64;

	// Add padding to this block (NB may be 0 bytes but never 64 bytes of overflow!)
	lc_memset(m_overflow + cbOverflow, 0, 64 - cbOverflow);
	m_overflow[cbOverflow] = 0x80;

	// Work out length and make big-endian
	LcTUInt32 dwBitLength = m_cbStream * 8;

#ifndef IFX_USE_BIG_ENDIAN
	bigEnd(&dwBitLength);
#endif

	// If there's no room to fit the 8-byte length in this block...
	if (cbOverflow > 55)
	{
		// Process what we have...
		updateHash64(m_overflow);

		// ...and start a fresh block
		lc_memset(m_overflow, 0, 64);
	}

	// Put length at end of block
	((LcTUInt32*)m_overflow)[15] = dwBitLength;

	// Process into digest
	updateHash64(m_overflow);

#ifndef IFX_USE_BIG_ENDIAN
	// Always produce a bug endian hash
	bigEnd(&m_dw[0]);
	bigEnd(&m_dw[1]);
	bigEnd(&m_dw[2]);
	bigEnd(&m_dw[3]);
	bigEnd(&m_dw[4]);
#endif
}

/*-------------------------------------------------------------------------*//**
*/
void LcCShaHash::getRawDigest(LcTUInt32* pBuf)
{
	memcpy(pBuf, m_dw, 5 * sizeof(LcTUInt32));
}

/*-------------------------------------------------------------------------*//**
	Converts a DIGEST struct into a 20 char string consisting of
	upper-case letters with dashes every 4 chars
	e.g. XXXX-XXXX-XXXX-XXXX-XXXX
*/
void LcCShaHash::getAsLetters(char* buf)	// 24 chars + \0
{
	typedef struct longInBytesType
	{
		unsigned char b[4];
	}longInBytes;

	longInBytes *lib;
	int i,k,index = 0;

	for (i = 0; i < 5; i++)
	{
		lib = (longInBytes*) &m_dw[i];
		for (k = 0; k < 4; k++)
		{
			buf[index++] = (lib->b[k] % 26) + 'A';
		}
		buf[index++] = '-';
	}
	buf[index-1] = '\0';
}

/*-------------------------------------------------------------------------*//**
	19990707	JOL
	Converts a DIGEST struct into an 8 char string consisting of
	upper-case letters - used for unique filename generation, etc.
	Code is very similar to ConvertHashDataToLetters, the difference
	being that only the first two dwords are used, and no dashes are
	inserted.
*/
void LcCShaHash::get8Chars(char* buf)		// 8 chars + \0
{
	typedef struct longInBytesType
	{
		unsigned char b[4];
	}longInBytes;

	longInBytes *lib;
	int i,k,index = 0;

	for (i = 0; i < 2; i++)
	{
		lib = (longInBytes*) &m_dw[i];
		for (k = 0; k < 4; k++)
		{
			buf[index++] = (lib->b[k] % 26) + 'A';
		}
	}
	buf[index] = '\0';
}

#endif
