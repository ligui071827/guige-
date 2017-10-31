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

#if (defined(LC_PLAT_NUCLEUS) && !(defined(__CC_ARM) && (defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 210000))))
	#include <wchar.h>
#endif

/*-------------------------------------------------------------------------*//**
	Implementation notes

	The class uses a typical reference-counted handle-body pattern, with
	special support for literals and short strings.  Literals may be pointed
	to directly from the handle, to avoid memory allocation.  This is not
	only more efficient, but allows strings to be initialized to literal values
	without risking exceptions due to allocation failures (e.g. this can
	be done safely in a constructor).

	Short UTF8 strings will be stored in an internal buffer. Short strings
	are frequently used, and this also avoids an expensive allocation.

	The string class has three objectives - minimize size, so that ASCII
	strings such as parsed XML configuration data are not inflated to twice
	their size, and secondly, to allow full support for foreign character sets.
	For this reason, UTF8 is the main representation used, because it allows
	16-bit character support while keeping ASCII strings small.  However,
	for Asian languages, many characters require 3 bytes in UTF8, and so
	in future a fully Unicode string may be preferable.

	The third objective of LcTmString is therefore to provide a general
	purpose class for string manipulation that conceals the internal choice
	of UTF8 or Unicode representation.

	Internally, any string can switch between a UTF8 and Unicode
	representation as required for the current operation.  This allows
	mixed UTF8/Unicode operations, and conversion between formats, etc.
	Currently most operations work on UTF8, but this design will make it
	easy to add Unicode versions in future, either as new defaults, or
	for a full mixed-mode set of operations.

	Summary of UTF8 byte encoding of 16-bit codes:-
	00000000 0xxxxxxx -> 0xxxxxxx
	00000yyy yyxxxxxx -> 110yyyyy 10xxxxxx
	zzzzyyyy yyxxxxxx -> 1110zzzz 10yyyyyy 10xxxxxx

	The union storing the data is formatted in one of several ways.
	To identify what is stored in the union, access the last byte of it
	(m_modeInfo.m_dataMode or m_buffer[_LC_STRING_INTERNAL_BUFFER_LENGTH-1]).
	If it is 0, then the internal buffer is being used. (The zero is used
	to both identify the buffer, and potentially terminate the string
	data.) Other values of _LC_STRING_DATAMODE_* specify whether the data
	being pointed to is literal or dynamically allocated.
*/

#define MARKER_UTF8			0xF9F9
#define MARKER_WCHAR		0xFAFA
#define IS_MARKER_UTF8(c)	((c) == '\xF9')
#define IS_MARKER_WCHAR(c)	((c) == '\xFA')
#define IS_MARKER(c)		((int(c) & 0x0F8) == 0x0F8)
#define REAL_SIZE(siz)		((siz + 0x0F) & 0xFFFFFFF0U)

/*-------------------------------------------------------------------------*//**
	Copy constructor
*/
LcTmString::LcTmString(const LcTmString& s)
{
	setExternal(NULL);
	if (s.isLiteral())
	{
		// If it's a literal, simply copy the pointer
		setLiteral(s.m_literal);
	}
	else
	{
		attach(s);
	}
}

/*-------------------------------------------------------------------------*//**
	Sets the string data to the internal buffer
*/
inline void LcTmString::setInternal()
{
	m_buffer[0] = '\0';
	m_modeInfo.m_dataMode = _LC_STRING_DATAMODE_BUFFER_UTF8;
}

/*-------------------------------------------------------------------------*//**
	Private utility - returns the number of UTF8 chars in the given string
	(assuming correct UTF8)
*/
int utf8len(const char* p)
{
	int len = 0;

	if (p)
	{
		// Count non-extension bytes (i.e. ignore those that start with 10)
		while (*p)
		{
			if ((*p++ & 0xC0) != 0x80)
				len++;
		}
	}

	return len;
}

/*-------------------------------------------------------------------------*//**
	Private utility - returns the size in bytes of the given UTF8 string
	up to a maximum length of len logical characters (assuming correct UTF8)
*/
int utf8size(const char* s, int len)
{
	const char* p = s;
	int i = 0;

	// Increment i on each non-extension byte, and stop at count
	while (*p && i <= len)
	{
		if ((*p++ & 0xC0) != 0x80)
			i++;
	}

	// If we hit the character limit, we stepped one past the end
	if (i > len)
		p--;

	// Return number of bytes
	return (int)(p - s);
}
#ifdef IFX_SERIALIZATION
LcTaString LcTmString::loadState(int handle,LcCSerializeMaster *serializeMaster)
{
	LcTaString string="";
	string.deSerialize(serializeMaster->getOffset(handle));
	return string;
}

SerializeHandle LcTmString::serialize(LcCSerializeMaster *serializeMaster)
{
	const char * utf8=bufUtf8();
	int size=utf8size(utf8,length())+1;
	SerializeHandle thisHandle= serializeMaster->newHandle();
	serializeMaster->setData(thisHandle,size,(LcTByte*)utf8);
	return thisHandle;
}

void LcTmString::deSerialize(void * ptr)
{
	const char * utf=(const char *)ptr;
	fromBufUtf8(utf,utf8len(utf));
}
#endif /* IFX_SERIALIZATION */


/*-------------------------------------------------------------------------*//**
	Makes a new body of the specified size (bytes) and type, but does not
	attach it to any string object
*/
LcTmString::THeader* LcTmString::makeExternalBody(int size, unsigned short marker)
{
	// Allocate memory using special text heap
	// NB: 2 bytes added for terminating zero
	// MR 14/3/07 - buffers now always rounded up to 16 bytes, but stored size is actual not max
	THeader* header = NULL;

	if (size > 0)
	{
		header = (THeader*)(LcCEnvironment::get())->stringAlloc(sizeof(THeader) + REAL_SIZE(size));

		// Set up body
		header->m_marker	= marker;
		header->m_count		= 1;
		header->m_size		= (unsigned short)size;
		*header->m_data		= 0;
	}

	// This one used for internal work so return rather than setting m_header
	// NB: cleanup safe as long as nothing ever leaves before header is attached
	return header;
}

/*-------------------------------------------------------------------------*//**
	Sets the string data format for current string: will use internal buffer
	if there is space, or else allcoates an external buffer.
*/
void LcTmString::setBody(int size, unsigned short marker)
{
	if (marker == MARKER_UTF8 && size < _LC_STRING_INTERNAL_BUFFER_LENGTH)
	{
		setInternal();
	}
	else
	{
		setExternal(makeExternalBody(size, marker));
	}
}

/*-------------------------------------------------------------------------*//**
	Attaches a header returned by another string for sharing
*/
LC_EXPORT void LcTmString::attach(const LcTmString& s)
{
	char this_dataType = m_modeInfo.m_dataMode;
	bool is_s_dataType_external = (s.m_modeInfo.m_dataMode == _LC_STRING_DATAMODE_EXTERNAL);

	// Avoid self-assignment as that may result in deletion of body
	if (	this_dataType != _LC_STRING_DATAMODE_EXTERNAL
		|| 	!is_s_dataType_external
		|| 	m_header != s.m_header)
	{
		// Throw away current content
		detach();

		// Make a copy of p's internal buffer
		memcpy(m_buffer, s.m_buffer, _LC_STRING_INTERNAL_BUFFER_LENGTH);

		if (is_s_dataType_external && m_header)
		{
			m_header->m_count++;
		}
	}
}

/*-------------------------------------------------------------------------*//**
	Detaches the string from its current body, deleting the body if it
	is no longer attached to other strings
*/
LC_EXPORT void LcTmString::detach()
{
	// Throw away the current content - only if it's a header
	if (isExternal() && !--m_header->m_count)
	{
		(LcCEnvironment::get())->stringFree(m_header);
	}
}

/*-------------------------------------------------------------------------*//**
	For internal use - return pointer to string data in UTF8 format,
	or NULL if empty.  If the string is in wide char format it is converted
	back to UTF8 first
*/
LC_EXPORT const char* LcTmString::getStrUtf8() const
{
	// If we are in wide char mode, switch back to UTF8
	const char* retVal = NULL;

	// Return string data
	switch (m_modeInfo.m_dataMode)
	{
	case _LC_STRING_DATAMODE_BUFFER_UTF8:
		retVal = m_buffer;
		break;

	case _LC_STRING_DATAMODE_LITERAL_UTF8:
		retVal = m_literal;
		break;

	case _LC_STRING_DATAMODE_EXTERNAL:
		if (m_header && !IS_MARKER_UTF8(m_literal[0]))
			((LcTmString*)this)->switchStrUtf8();
		retVal = m_header ? m_header->m_data : NULL;
		break;
	}

	return retVal;
}

/*-------------------------------------------------------------------------*//**
	For internal use - return pointer to string data in wide char format,
	or NULL if empty.  If the string is in UTF8 format it is converted
	to wide char first
*/
LC_EXPORT const LcTWChar* LcTmString::getStrWChar() const
{
	// If we are NOT in wide char mode, switch to wide char
	if (m_modeInfo.m_dataMode != _LC_STRING_DATAMODE_EXTERNAL || (m_header && !IS_MARKER_WCHAR(m_literal[0])))
		((LcTmString*)this)->switchStrWChar();

	return m_header ? (const LcTWChar*)m_header->m_data : NULL;
}

/*-------------------------------------------------------------------------*//**
	Convert this string to wide char format by replacing the body
*/
void LcTmString::switchStrWChar()
{
	// Get UTF8 string to convert
	const char* p = utf8Data();

	// Create new body of right size
	int len        = utf8len(p);
	THeader* h 	   = makeExternalBody(len * sizeof(LcTWChar), MARKER_WCHAR);

	if (h && p)
	{
		LcTWChar* buf = (LcTWChar*)h->m_data;

		// Must leave room for terminator
		int i = 0;
		while (*p && i < len)
		{
			switch (*p & 0xE0)
			{
				// Bit pattern 0xxxxxxx
				case 0x00: case 0x20: case 0x40: case 0x60:
				{
					buf[i++] = *p++;
					break;
				}

				// Bit pattern illegal, skip byte
				case 0x80: case 0xA0:
				{
					p++;
					break;
				}

				// Bit pattern 110yyyyy (followed by 10xxxxxx)
				case 0xC0:
				{
					LcTWChar c = (LcTWChar(*p++) & 0x1F) << 6;
					if ((*p & 0xC0) == 0x80)
						buf[i++] = c | (*p++ & 0x3F);
					break;
				}

				// Bit pattern 1110zzzz (followed by 10yyyyyy 10xxxxxx)
				case 0xE0:
				{
					LcTWChar c = (LcTWChar(*p++) & 0x0F) << 12;
					if ((*p & 0xC0) == 0x80)
					{
						c |= (LcTWChar(*p++) & 0x3F) << 6;
						if ((*p & 0xC0) == 0x80)
							buf[i++] = c | (*p++ & 0x3F);
					}
					break;
				}
			}
		}

		// Terminate
		buf[i] = 0;
	}

	// Attach new body
	detach();
	setExternal(h);
}

/*-------------------------------------------------------------------------*//**
	Convert the current string from wide char to UTF8 representation
*/
void LcTmString::switchStrUtf8()
{
	// Get wide char string to convert
	const LcTWChar* buf = ucs2Data();

	if (buf)
	{
		// String length in chars is a proportion of byte length, depending on
		// how wide the chars are
		int len = size() / sizeof(LcTWChar);

		// Calculate required size by adding extra for MB chars
		int i, size = len;
		for (i = 0; i < len; i++)
		{
			if (buf[i] > 0x07FF)
				size += 2;
			else if (buf[i] > 0x007F)
				size += 1;
		}

		// Make a big enough body and fill with UTF8
		THeader* h = makeExternalBody(size, MARKER_UTF8);

		if (h)
		{
			char* p = h->m_data;
			for (i = 0; i < len && buf[i]; i++)
			{
				if (buf[i] > 0x07FF)
				{
					// zzzzyyyy yyxxxxxx -> 1110zzzz 10yyyyyy 10xxxxxx
					*p++ = 0xE0 | (buf[i] >> 12);
					*p++ = 0x80 | ((buf[i] >> 6) & 0x3F);
					*p++ = 0x80 | (buf[i] & 0x3F);
				}
				else if (buf[i] > 0x007F)
				{
					// 00000yyy yyxxxxxx -> 110yyyyy 10xxxxxx
					*p++ = 0xC0 | (buf[i] >> 6);
					*p++ = 0x80 | (buf[i] & 0x3F);
				}
				else
				{
					// 00000000 0xxxxxxx -> 0xxxxxxx
					*p++ = (char)buf[i];
				}
			}

			// Terminate and attach
			*p = 0;
		}

		detach();
		setExternal(h);
	}
}

/*-------------------------------------------------------------------------*//**
	Set the string value to the given zero-terminated UTF8 string
*/
LC_EXPORT LcTmString& LcTmString::operator=(const char* p)
{
	detach();

	// Make body of appropriate length and copy string in
	setBody((int)lc_strlen(p), MARKER_UTF8);

	char* buf = writableUtf8Data();
	if (buf)
	{
		lc_strcpy(buf, p);
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
	Set the string value to the given zero-terminated wide string
*/
LC_EXPORT LcTmString& LcTmString::operator=(const LcTWChar* p)
{
	detach();

	// Make body of appropriate length and copy string in
	int size = sizeof(LcTWChar) * (int)(lc_wcslen((const IFX_WCHAR*)p));
	setExternal(makeExternalBody(size, MARKER_WCHAR));

	if (m_header)
	{
		memcpy(m_header->m_data, p, size + sizeof(LcTWChar));
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmString& LcTmString::operator=(char c)
{
	detach();

	m_buffer[0] = c;
	m_buffer[1] = '\0';
	m_modeInfo.m_dataMode = _LC_STRING_DATAMODE_BUFFER_UTF8;

	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmString& LcTmString::operator=(const LcTmString& s)
{
	if (s.isLiteral())
	{
		// If it's a literal, copy the contents
		operator=(s.m_literal);
	}
	else
	{
		attach(s);
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
	Use when the original string is not zero terminated.  Second parameter
	is the size in bytes, NOT length in chars
*/
LC_EXPORT LcTmString& LcTmString::fromBufUtf8(const char* p, int size)
{
	detach();

	// Make body of appropriate length and copy string in
	setExternal(makeExternalBody(size, MARKER_UTF8));

	char* data = writableUtf8Data();

	if (data)
	{
		lc_strncpy(data, p, size);
		data[size] = 0;
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
	Use when the original string is not zero terminated.  Second parameter
	is the length in chars, NOT size in bytes
*/
LC_EXPORT LcTmString& LcTmString::fromBufWChar(const LcTWChar* p, int len)
{
	detach();

	// Make body of appropriate length and copy string in
	setExternal(makeExternalBody(len * sizeof(LcTWChar), MARKER_WCHAR));

	LcTWChar* data = writableUcs2Data();

	if (data)
	{
		memcpy(data, p, len * sizeof(LcTWChar));
		data[len] = 0;
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
	The size in bytes of the UTF8 string (which may be longer than length
	in chars) or, if in wide char mode, the size in bytes of the Unicode string,
	which is double the length.  Both exclude the terminator char
*/
inline int LcTmString::size() const
{
	char dataType = m_modeInfo.m_dataMode;

	switch (dataType)
	{
	case _LC_STRING_DATAMODE_BUFFER_UTF8:
		return (int)(lc_strlen(m_buffer));

	case _LC_STRING_DATAMODE_LITERAL_UTF8:
		if (m_literal)
		{
			return (int)(lc_strlen(m_literal));
		}
		break;

	case _LC_STRING_DATAMODE_EXTERNAL:
		if (m_header)
		{
			return m_header->m_size;
		}
		break;
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
	Return string length excluding terminator, either from header, or from
	scanning literal
*/
LC_EXPORT int LcTmString::length() const
{
	if (isDataUtf8())
	{
		return utf8len(utf8Data());
	}
	else if (isExternal())
	{
		return m_header->m_size / sizeof(LcTWChar);
	}

	return 0;
}

/*-------------------------------------------------------------------------*//**
	Return true if the string contains no characters
*/
LC_EXPORT bool LcTmString::isEmpty() const
{
	char dataType = m_modeInfo.m_dataMode;

	switch (dataType)
	{
	case _LC_STRING_DATAMODE_BUFFER_UTF8:
		return m_buffer[0] == 0;

	case _LC_STRING_DATAMODE_LITERAL_UTF8:
		return (!m_literal) || (m_literal[0] == 0);

	case _LC_STRING_DATAMODE_EXTERNAL:
		if (m_header)
		{
			return m_header->m_size == 0;
		}
		else
		{
			return true;
		}
	}

	return true;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmString& LcTmString::operator+=(const LcTmString& s)
{
	// Get strings first, to ensure UTF8 format
	const char* strA = getStrUtf8();
	const char* strB = s.getStrUtf8();

	int sizeA = size();
	int sizeB = s.size();
	char dataType = m_modeInfo.m_dataMode;
	bool haveSpace = false;

	unsigned int totalSize = sizeA + sizeB;
	totalSize = (totalSize<=_LC_STRING_MAXIMUM_LENGTH)?totalSize:_LC_STRING_MAXIMUM_LENGTH;
	sizeB=totalSize-sizeA;

	switch (dataType)
	{
	case _LC_STRING_DATAMODE_BUFFER_UTF8:
		haveSpace = totalSize < _LC_STRING_INTERNAL_BUFFER_LENGTH;
		break;

	case _LC_STRING_DATAMODE_EXTERNAL:
		if (m_header)
		{
			haveSpace = (m_header->m_count == 1 && sizeA > 0 && totalSize < REAL_SIZE(sizeA));
		}
		break;
	}

	if (strA && strB && haveSpace)
	{
		// There is room for the additional data, without any further allocs
		char* buf = writableUtf8Data();
		if (buf)
		{
			lc_strcpy(buf + sizeA, strB);
		}

		if (m_header && dataType == _LC_STRING_DATAMODE_EXTERNAL)
			m_header->m_size = totalSize;
	}
	else
	{
		// Create new body with enough room
		THeader* h = makeExternalBody(totalSize, MARKER_UTF8);

		if (h)
		{
			// Note that either side may initially be literal
			if (sizeA > 0 && strA)
				lc_strcpy(h->m_data, strA);
			if (sizeB > 0 && strB)
				lc_strncpy(h->m_data + sizeA, strB,sizeB);
			h->m_data[totalSize]=0;
		}

		// Attach new body
		detach();
		setExternal(h);
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmString& LcTmString::operator+=(const char* s)
{
	if (s)
	{
		// Get strings first, to ensure UTF8 format
		const char* strA = getStrUtf8();

		int sizeA = size();
		int sizeB = (int)lc_strlen(s);
		unsigned int totalSize = sizeA + sizeB;
		totalSize = (totalSize<=_LC_STRING_MAXIMUM_LENGTH)?totalSize:_LC_STRING_MAXIMUM_LENGTH;
		sizeB=totalSize-sizeA;

		char dataType = m_modeInfo.m_dataMode;
		bool haveSpace = false;

		switch (dataType)
		{
		case _LC_STRING_DATAMODE_BUFFER_UTF8:
			haveSpace = totalSize < _LC_STRING_INTERNAL_BUFFER_LENGTH;
			break;

		case _LC_STRING_DATAMODE_EXTERNAL:
			if (m_header)
			{
				haveSpace = (m_header->m_count == 1 && sizeA > 0 && totalSize < REAL_SIZE(sizeA));
			}
			break;
		}

		if (strA && s && haveSpace)
		{
			// There is room for the additional data, without any further allocs
			char* buf = writableUtf8Data();
			if (buf)
			{
				lc_strcpy(buf + sizeA, s);
			}

			if (isExternal())
			{
				m_header->m_size = totalSize;
			}
		}
		else
		{
			// Create new body with enough room
			THeader* h = makeExternalBody(totalSize, MARKER_UTF8);

			if (h)
			{
				// Note that LHS may initially be literal
				if (sizeA > 0 && strA)
					lc_strcpy(h->m_data, strA);
				if (sizeB > 0)
					lc_strncpy(h->m_data + sizeA, s,sizeB);
				h->m_data[totalSize]=0;
			}

			// Attach new body
			detach();
			setExternal(h);
		}
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmString& LcTmString::operator+=(char s)
{
	// MR 14/3/07 - if the stored size is short of the allocated space...
	// And only if we have a header with UTF8 content...
	unsigned int sizeData = size();
	char dataType = m_modeInfo.m_dataMode;
	bool haveSpace = false;

	switch (dataType)
	{
	case _LC_STRING_DATAMODE_BUFFER_UTF8:
		haveSpace = sizeData < (_LC_STRING_INTERNAL_BUFFER_LENGTH - 1);
		break;

	case _LC_STRING_DATAMODE_EXTERNAL:
		if (m_header && IS_MARKER_UTF8(m_literal[0]))
		{
			haveSpace = (m_header->m_count == 1 && sizeData > 0 && sizeData < REAL_SIZE(sizeData));
		}
		break;
	}

	if (haveSpace)
	{
		// ...we can avoid realloc'ing memory
		char* data = NULL;

		if (dataType == _LC_STRING_DATAMODE_EXTERNAL)
		{
			m_header->m_size++;
			data = m_header->m_data;
		}
		else
		{
			data = m_buffer;
		}

		data[sizeData]		= s;
		data[sizeData + 1]	= 0;
	}
	else
	{
		// Get strings first, to ensure UTF8 format
		const char* strA = getStrUtf8();
		bool copyCharacter=false;

		// update sizeData, as getStrUtf8 can change it
		sizeData = size();
		copyCharacter=sizeData<_LC_STRING_MAXIMUM_LENGTH;
		sizeData = (copyCharacter)?sizeData:(sizeData-1);

		// Create a new body with enough room
		THeader* h = makeExternalBody(sizeData + 1, MARKER_UTF8);

		if (h)
		{
			// Note that LHS may initially be literal
			if (sizeData > 0)
				lc_strcpy(h->m_data, strA);
			if(copyCharacter)
				h->m_data[sizeData]	= s;
			h->m_data[sizeData + 1]	= 0;
		}

		// Attach new body
		detach();
		setExternal(h);
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmString& LcTmString::operator+=(LcTWChar s)
{
	// Make sure we're in WChar mode
	if (m_modeInfo.m_dataMode != _LC_STRING_DATAMODE_EXTERNAL || (m_header && !IS_MARKER_WCHAR(m_literal[0])))
		((LcTmString*)this)->switchStrWChar();

	unsigned int sizeData = size();
	bool haveSpace = (m_header && m_header->m_count == 1 && (sizeData > 0) && (sizeData + sizeof(LcTWChar) <= REAL_SIZE(sizeData)));

	if (haveSpace)
	{
		// ...we can avoid realloc'ing memory
		LcTWChar* data = NULL;

		m_header->m_size += sizeof(LcTWChar);
		data = (LcTWChar*) m_header->m_data;

		data[sizeData/sizeof(LcTWChar)]		= s;
		data[(sizeData/sizeof(LcTWChar)) + 1]	= 0;
	}
	else
	{
		const LcTWChar* strA = getStrWChar();

		bool copyCharacter=false;

		copyCharacter=sizeData<_LC_STRING_MAXIMUM_LENGTH;

		sizeData = (copyCharacter)?sizeData:(sizeData-sizeof(LcTWChar));

		// Create a new body with enough room
		THeader* h = makeExternalBody(sizeData + sizeof(LcTWChar), MARKER_WCHAR);

		if (h)
		{
			LcTWChar* data = (LcTWChar*) h->m_data;

			if (sizeData > 0)
				lc_wcscpy(data, strA);
			if(copyCharacter)
				data[sizeData/sizeof(LcTWChar)]		= s;
			data[(sizeData/sizeof(LcTWChar)) + 1]	= 0;
		}

		// Attach new body
		detach();
		setExternal(h);
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaString LcTmString::subString(int pos, int len) const
{
	// Length of string
	int t = length();
	if (t == 0)
		return LcTaString();

	// Calc length of extracted portion
	len = min(len, t - pos);
	if (len <= 0)
		return LcTaString();

	// Special case where whole string extracted
	if (pos == 0 && len == t)
		return *this;

	// Get position and size of substring
	const char* str = getStrUtf8();
	str += utf8size(str, pos);
	int size = utf8size(str, len);

	// Create a body big enough for the substring, and copy it
	THeader* h = makeExternalBody(size, MARKER_UTF8);

	if (h)
	{
		lc_strncpy(h->m_data, str, size);
		h->m_data[size] = 0;
	}

	// No constructor for LcTaString direct from header
	return LcTmString(h);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaString LcTmString::tail(int pos) const
{
	return subString(pos, 0x7FFFFFFF);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaString LcTmString::toUpper() const
{
	// Get string first, to ensure UTF8 format
	const char* str = getStrUtf8();

	// Make a copy
	unsigned n = size();
	THeader* h = makeExternalBody(n, MARKER_UTF8);

	if (h)
	{
		lc_strcpy(h->m_data, str);

		// This is UTF8-safe, because MB chars will be outside the a-z range
		for (unsigned i = 0; i < n; i++)
		{
			if (h->m_data[i] >= 'a' && h->m_data[i] <= 'z')
				h->m_data[i] -= ('a' - 'A');
		}
	}

	// No constructor for LcTaString direct from header
	return LcTmString(h);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaString LcTmString::toLower() const
{
	// Get string first, to ensure UTF8 format
	const char* str = getStrUtf8();

	// Make a copy
	unsigned n = size();
	THeader* h = makeExternalBody(n, MARKER_UTF8);

	if (h)
	{
		lc_strcpy(h->m_data, str);

		// This is UTF8-safe, because MB chars will be outside the A-Z range
		for (unsigned i = 0; i < n; i++)
		{
			if (h->m_data[i] >= 'A' && h->m_data[i] <= 'Z')
				h->m_data[i] += ('a' - 'A');
		}
	}

	// No constructor for LcTaString direct from header
	return LcTmString(h);
}

/*-------------------------------------------------------------------------*//**
	Finds the specified item within the string.
*/
LC_EXPORT int LcTmString::find(const char* p, int pos) const
{
	// Fail if neither string has contents.
	const char* str = getStrUtf8();
	if ((!str || !p) || (pos > length()))
		return npos;

	str = (str + pos);

	// Find occurrence of p in this (works for UTF8)
	const char* pss = lc_strstr(str, p);

	// Return offset if found
	return pss ? (pss - str + pos) : npos;
}

/*-------------------------------------------------------------------------*//**
	Replaces the specified item within the string.
*/
LC_EXPORT void LcTmString::replace(const char findChar, const char replaceChar)
{

	// There is no point doing this if the characters match.
	if (findChar == replaceChar)
		return;

	// Fail if neither string has contents.
	const char* str = getStrUtf8();
	if (!str)
		return;

	// Make a copy
	int n = size();
	THeader* h = makeExternalBody(n, MARKER_UTF8);

	if (h)
	{
		lc_strcpy(h->m_data, str);

		// Replace occurrence of findChar in this (works for UTF8)
		char* pss = lc_strchr(h->m_data, findChar);
		while (pss)
		{
			*pss = replaceChar;
			++pss;
			pss = lc_strchr(pss, findChar);
		}
	}

	// Attach new body
	detach();
	setExternal(h);
}

/*-------------------------------------------------------------------------*//**
	Returns the position of the last occurrence of a char
*/
LC_EXPORT int LcTmString::findLastChar(const char p) const
{
	const char* str = getStrUtf8();
	if (!str)
		return npos;

	// Find occurrence of p in this (works for UTF8)
	const char* pss = lc_strrchr(str, p);

	// Return offset if found
	return pss ? (pss - str) : npos;
}

/*-------------------------------------------------------------------------*//**
	Get the 'iWord'th cSep-separated word of string s.
	If iWord is +ve, words are parsed from the left  (from  1 up).
	If iWord is -ve, words are parsed from the right (from -1 down).
*/
LC_EXPORT LcTaString LcTmString::getWord(int iWord, char cSep) const
{
	// 0th word is empty
	if (iWord == 0)
		return "";

	int iCount = 1;

	const char* p = getStrUtf8();
	int iLen = length();
	LcTaString sWord = "";

	// NB: this algorithm is UTF8-safe, because cSep is always ASCII
	for (int i = 0; i < iLen; i++)
	{
		// Work backwards if iWord is negative
		char c = p[iWord > 0 ? i : iLen - i - 1];

		if (c == cSep)
		{
			// Increase current count if the separator is found
			if (iCount == abs(iWord))
				break;
			iCount++;
		}
		else if (iCount == abs(iWord))
		{
			// Otherwise if we are on the 'right' word, append/prepend
			// (where iWord is +ve/-ve respectively) this char to sWord
			if (iWord > 0)
				sWord = sWord + c;
			else
				sWord = c + sWord;
		}
	}

	return sWord;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcTmString::toInt() const
{
	// Get string first, to ensure UTF8 format
	const char* str = getStrUtf8();
	if (!str)
		return 0;

	// Check for hex (strings starting with "#")
	if (str[0] == '#')
	{
		int nVal = 0;

		str++;
		while (*str)
		{
			int cVal = 0;
			if (*str >= '0' && *str <= '9')
				cVal = (*str - '0');
			else if (*str >= 'A' && *str <= 'F')
				cVal = 10 + (*str - 'A');
			else if (*str >= 'a' && *str <= 'f')
				cVal = 10 + (*str - 'a');
			else
				break;

			nVal = (nVal * 16) + cVal;
			str++;
		}

		return nVal;
	}
	else
	{
		// Assumes string is ASCII representation of integer
		return lc_atoi(str);
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT int LcTmString::toBool() const
{
	// Get string first, to ensure UTF8 format
	const char* str = getStrUtf8();
	if (!str)
		return 0;
	if (lc_strcmp(str, "false") == 0)
		return 0;
	else if (lc_strcmp(str, "true") == 0)
		return 1;
	return 0;
}	

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTScalar LcTmString::toScalar() const
{
	// Note: There is a problem case e.g "+005.005000"; => 5.0050001

	// Assumes string is ASCII representation of float
	const char* orig = getStrUtf8();
	float convertedVal = 0.0;

	sscanf(orig, "%f", &convertedVal);

	return (LcTScalar)convertedVal;

}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmString& LcTmString::fromInt(int n)
{
	// Format number
	char sz[TYPES_BUF_SIZE];
	lc_sprintf(sz, "%d", n);

	// Assign to this string
	operator=(sz);
	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_IMPORT LcTmString& LcTmString::fromTime(IFX_TIME t, const LcTaString& format)
{
	LcTaString timeFormat = format;
	if (timeFormat.length()==0)
	{
		timeFormat.fromBufWChar((const IFX_WCHAR*)DEFAULT_TIME_FORMAT, (int)lc_wcslen((const IFX_WCHAR*)DEFAULT_TIME_FORMAT));
	}

	// Format time
	LcTWChar sz[TYPES_BUF_SIZE];

	struct tm* tim=gmtime(&t);
	if (tim)
	{
		lc_wcsftime(sz, TYPES_BUF_SIZE, timeFormat.bufWChar(), tim);

		// Assign to this string
		operator=(sz);
	}

	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmString& LcTmString::fromTime(IFX_TIME t)
{
	const LcTWChar *frmt = (const LcTWChar *) DEFAULT_TIME_FORMAT;
	// Format time
	LcTWChar sz[TYPES_BUF_SIZE];

	struct tm* tim=gmtime(&t);
	if(tim)
	{
		lc_wcsftime(sz, 1000, frmt, tim);

		// Assign to this string
		operator=(sz);
	}

	return *this;
}

#if defined(IFX_GENERATE_SCRIPTS) || defined (IFX_USE_SCRIPTS)
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmString& LcTmString::fromLong(IFX_UINT32 n)
{
	// Format number
	char sz[TYPES_BUF_SIZE];
	lc_sprintf(sz, "%lu", n);

	// Assign to this string
	operator=(sz);
	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmString& LcTmString::fromBool(bool b)
{
	// Format number
	char sz[6];
	if (b)
		lc_strcpy(sz, "true");
	else
		lc_strcpy(sz, "false");

	// Assign to this string
	operator=(sz);
	return *this;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT IFX_UINT32 LcTmString::toLong()
{
	// Assumes string is ASCII representation of long
	const char* orig = getStrUtf8();
	IFX_UINT32  convertedVal = 0;

	sscanf(orig, "%lu", &convertedVal);

	return (IFX_UINT32)convertedVal;
}

#endif /* (IFX_GENERATE_SCRIPTS) || defined (IFX_USE_SCRIPTS) */

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTmString& LcTmString::fromScalar(LcTScalar n)
{
	// Format number
	char sz[TYPES_BUF_SIZE];
	lc_sprintf(sz, "%f", n);

	// Assign to this string
	operator=(sz);
	return *this;
}

/*-------------------------------------------------------------------------*//**
	Make a string 'safe' for inclusion in a URL.  Note that this function
	is NOT designed to cope with complete URL's
	(e.g. "http://www.search.com?query=test me"), but rather with
	individual items that will be appended to standard URL's (in this case
	"test me" or "query=test me".  This is because (for example) the first
	colon in a URL should NOT be escaped, but subsequent ones should be.
*/
LC_EXPORT LcTaString LcTmString::toUrlSafe() const
{
	LcTaString result = "";

	const char* p = getStrUtf8();
	int iLen = size();

	// NB: this algorithm is UTF8-safe
	for (int i = 0; i < iLen; i++)
	{
		char c = p[i];

		// Copy safe characters as is, those being...
		// 0-9    a-z    A-Z    $-_.+!*'(),
		if ( (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
			|| (lc_strchr("$-_.+!*'(),", c) != NULL))
		{
			result += c;
		}
		// Format all other characters as 2-digit hex number preceded by %
		else
		{
			char sz[4];
			lc_sprintf(sz, "%%%02X", c);
			result += sz;
		}
	}

	return result;
}

/*-------------------------------------------------------------------------*//**
	Inverse of toUrlSafe()
*/
LC_EXPORT LcTaString LcTmString::fromUrlSafe() const
{
	LcTaString result = "";

	const char* p = getStrUtf8();
	int iLen = size();

	// NB: this algorithm is UTF8-safe
	for (int i = 0; i < iLen; i++)
	{
		char c = p[i];

		if (c != '%' || i == iLen - 1)
		{
			result += c;
		}
		else if (p[i + 1] == '%')
		{
			result += '%';
			i++;
		}
		else
		{
			int n;
			lc_sscanf(p + i + 1, "%02x", &n);
			result += (char)n;
			i += 2;
		}
	}

	return result;
}

/*-------------------------------------------------------------------------*//**
	Returns whether the string data in UTF8 format
*/
inline bool LcTmString::isDataUtf8() const
{
	switch (m_modeInfo.m_dataMode)
	{
	case _LC_STRING_DATAMODE_BUFFER_UTF8:
		return true;
	case _LC_STRING_DATAMODE_LITERAL_UTF8:
		return m_literal ? true : false; // explicit conversion to bool to avoid warning
	case _LC_STRING_DATAMODE_EXTERNAL:
		return m_header ? IS_MARKER_UTF8(m_literal[0]) : false;
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
	Returns whether the string data in UCS2 format
*/
inline bool LcTmString::isDataUcs2() const
{
	return isExternal() && IS_MARKER_WCHAR(m_literal[0]);
}

/*-------------------------------------------------------------------------*//**
	Returns whether the string data is held in the internal buffer
*/
inline bool LcTmString::isInternal() const
{
	return m_modeInfo.m_dataMode == _LC_STRING_DATAMODE_BUFFER_UTF8;
}

/*-------------------------------------------------------------------------*//**
	Returns whether the string data is an external literal
*/
inline bool LcTmString::isLiteral() const
{
	return m_modeInfo.m_dataMode == _LC_STRING_DATAMODE_LITERAL_UTF8 && m_literal;
}

/*-------------------------------------------------------------------------*//**
	Returns whether the string data is external, dynamically allocated memory
*/
inline bool LcTmString::isExternal() const
{
	return m_modeInfo.m_dataMode == _LC_STRING_DATAMODE_EXTERNAL && m_header;
}

/*-------------------------------------------------------------------------*//**
	Data access: returns pointer to writable UTF8 data, or NULL if unavailable
*/
inline char* LcTmString::writableUtf8Data() const
{
	switch (m_modeInfo.m_dataMode)
	{
	case _LC_STRING_DATAMODE_BUFFER_UTF8:
		return (char*)m_buffer;

	case _LC_STRING_DATAMODE_EXTERNAL:
		return m_header ? m_header->m_data : NULL;
	}

	return NULL;
}

/*-------------------------------------------------------------------------*//**
	Data access: returns pointer to UTF8 data, or NULL if unavailable
*/
inline const char* LcTmString::utf8Data() const
{
	switch (m_modeInfo.m_dataMode)
	{
	case _LC_STRING_DATAMODE_BUFFER_UTF8:
		return m_buffer;

	case _LC_STRING_DATAMODE_LITERAL_UTF8:
		return m_literal;

	case _LC_STRING_DATAMODE_EXTERNAL:
		return m_header ? m_header->m_data : NULL;
	}

	return NULL;
}

/*-------------------------------------------------------------------------*//**
	Data access: returns pointer to writable UCS2 data, or NULL if unavailable
*/
inline LcTWChar* LcTmString::writableUcs2Data() const
{
	if (isExternal())
	{
		return (LcTWChar*)(m_header->m_data);
	}

	return NULL;
}

/*-------------------------------------------------------------------------*//**
	Data access: returns pointer to UCS2 data, or NULL if unavailable
*/
inline const LcTWChar* LcTmString::ucs2Data() const
{
	if (isExternal())
	{
		return (const LcTWChar*)m_header->m_data;
	}

	return NULL;
}
