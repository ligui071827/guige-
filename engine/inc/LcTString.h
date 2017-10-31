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
#ifndef LcTStringH
#define LcTStringH

#include "inflexionui/engine/inc/LcDefs.h"
#include "inflexionui/engine/inc/LcTAuto.h"
#include "inflexionui/engine/ifxui_rtl.h"
#include <string.h>
class LcTaString;
class LcCSerializeMaster;
typedef IFX_WCHAR LcTWChar;


#define TYPES_BUF_SIZE	128

/*-------------------------------------------------------------------------*//**
	UTF8/Unicode string classes.  In accordance with NDE's cleanup rules,
	LcTmString is used for strings as embedded members of other objects,
	whilst LcTaString is used for automatic variables.  Construction of
	LcTmString objects should therefore never leave, i.e. never allocate
	heap memory.

	These string classes are optimized for efficient passing and returning
	of values, as this is such a frequent operation.  For passing string
	values as parameters, always use type "const LcTmString&".  Note the Tm
	and the reference, which are important.  For return values, always use
	type LcTaString.

	The use of "const LcTmString&" as a parameter type ensures that both
	LcTmString and LcTaString values can be passed, because LcTaString is
	derived from LcTmString (using the usual LcTaAuto<> mechanism).

	In order to pass literal strings efficiently, LcTmString provides a
	constructor that keeps a pointer to the literal rather than copying.
	These also avoids the need for allocation.  However, it means that the
	data within these objects is not guaranteed to survive for the lifetime
	of the object.  Therefore, as soon as these objects are copied or
	assigned, the data is copied too.  This means that if, for example, the
	object is assigned to a member LcTmString, the data is guaranteed for
	the lifetime of that member.

	In collections such as map and vector, LcTmString should always be used
	rather than LcTaString.  This is because cleanup of the collection
	values will be the responsibility of the containing collection.

	There is a trap to beware of as a result of these two points.  In order
	to be used in collections, the LcTmString class must expose a public
	copy constructor.  And because literal values are copied when LcTmString
	objects are copied, this constructor needs to allocate, and may thus
	leave.  Therefore the following rule must be adhered to: when a
	LcTmString is embedded within another object, it's copy constructor
	should never be used.  This translates into the following two guidelines
	in practice: either (a) initialize string members within construct()
	rather than the constructor, or (b) if there is no construct()/create()
	because the object is simple, pass initialization strings into the
	C++ constructor using LcTaString rather than "const LcTmString&".

	LcTmString is designed for general manipulation of strings of 16-bit
	characters, independent of the internal representation.  Where methods
	take or return char* types, these strings are in UTF8 format.  Where
	methods take or return LcTWChar* types, these are 16-bit Unicode.
	Unicode and UTF8 operations may be mixed on the same string, although
	this may be inefficient; it is however useful for string conversion.
	Most methods which take native strings, such as concatenation and
	comparison, assume just one string representation as the default,
	which in this version is UTF8.  In future versions it may be possible
	to opt for Unicode as the default.
*/

class LcTmString
{
#if defined(__TMS470__) || (defined(__CC_ARM) && (defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 210000))) // Compiler chokes
	LC_PRIVATE_INTERNAL_PUBLIC:
#else
	private:
#endif

	// Header
	struct THeader
	{
		// Must be aligned with start of header
		unsigned short			m_marker;	// non-UTF8 to distinguish from literal
		unsigned short			m_count;	// number of references to this block
		unsigned short			m_size;		// size in bytes, NOT length in chars, max 65k
		char  					padding[sizeof(unsigned short)]; // Padding to align wchar.
		char					m_data[sizeof(LcTWChar)];	// start of string, min length 1 char
	};

	#define _LC_STRING_INTERNAL_BUFFER_LENGTH	12
	#define _LC_STRING_MAXIMUM_LENGTH			0xFFFF

	//	Internal markers for the last byte of the union (m_modeInfo.m_dataMode)
	#define _LC_STRING_DATAMODE_BUFFER_UTF8		0
	#define _LC_STRING_DATAMODE_LITERAL_UTF8	1
	#define _LC_STRING_DATAMODE_EXTERNAL		2

	// Structure to provide easy access to the byte storing the data mode
	struct TDataModeInfo
	{
		char m_padding[_LC_STRING_INTERNAL_BUFFER_LENGTH - 1];
		char m_dataMode;
	};

private:

	// The string data: may be held externally or in the internal buffer. Last
	// byte of union stores the mode of the data.
	union
	{
		THeader*				m_header;
		const char*				m_literal;
		char					m_buffer[_LC_STRING_INTERNAL_BUFFER_LENGTH];
		TDataModeInfo			m_modeInfo;
	};

	// Internal helpers
	inline		int				size() const;
				void			setBody(int len, unsigned short mrk);
	static		THeader*		makeExternalBody(int len, unsigned short mrk);
				void			switchStrWChar();
				void			switchStrUtf8();

	// Exported helpers, called by inlines
	LC_IMPORT	void				attach(const LcTmString& s);
	LC_IMPORT	void				detach();
	LC_IMPORT	const char*			getStrUtf8() const;
	LC_IMPORT	const LcTWChar*		getStrWChar() const;

	// For internal use only
	inline						LcTmString(THeader* h)
									{ setExternal(h); }

	// Set the format of the string data
	inline		void 				setExternal(THeader* pHeader)
										{ m_header = pHeader; m_modeInfo.m_dataMode = _LC_STRING_DATAMODE_EXTERNAL; }
	inline		void 				setLiteral(const char* pLiteral)
										{ m_literal = pLiteral; m_modeInfo.m_dataMode = _LC_STRING_DATAMODE_LITERAL_UTF8; }
	inline		void 				setInternal();

	// Safe access to info about string data
	inline		bool				isDataUtf8() const;
	inline		bool				isDataUcs2() const;
	inline		bool				isInternal() const;
	inline		bool				isLiteral() const;
	inline		bool				isExternal() const;

	// Data access
	inline		char*				writableUtf8Data() const;
	inline		const char*			utf8Data() const;
	inline		LcTWChar*			writableUcs2Data() const;
	inline		const LcTWChar*		ucs2Data() const;

public:

	// Constants
	enum { npos = -1 };

	// Construction etc - see note above
	inline						LcTmString()
									{ setExternal(NULL); }
	inline						LcTmString(const char* p)
									{ setLiteral(p); }
								LcTmString(const LcTmString& s);
	inline 						~LcTmString()
									{ detach(); }


	// Buffer access
	inline		const char*			bufUtf8() const
										{ const char* p = getStrUtf8(); return p ? p : ""; }
	inline		const LcTWChar*		bufWChar() const
										{ const LcTWChar* p = getStrWChar(); return p ? p : (const LcTWChar*)L""; }
	inline		LcTWChar			operator[](int i) const
										{ const LcTWChar* p = getStrWChar(); return p[i]; }

	// Initialize from non-terminated string
	LC_IMPORT	LcTmString&		fromBufUtf8(const char* p, int size);
	LC_IMPORT	LcTmString&		fromBufWChar(const LcTWChar* p, int len);

#ifdef IFX_SERIALIZATION
	static		LcTaString		loadState(int handle,LcCSerializeMaster *serializeMaster);
				int				serialize(LcCSerializeMaster *serializeMaster);
				void			deSerialize(void *ptr);
#endif /* IFX_SERIALIZATION */

	// Assignment, including initialize from 0-terminated string
	LC_IMPORT	LcTmString&		operator=(const char* p);
	LC_IMPORT	LcTmString&		operator=(const LcTWChar* p);
	LC_IMPORT	LcTmString&		operator=(char c);
	LC_IMPORT	LcTmString&		operator=(const LcTmString& s);

	// Concatenation
	LC_IMPORT	LcTmString&		operator+=(const LcTmString& s);
	LC_IMPORT	LcTmString&		operator+=(const char* p);
	LC_IMPORT	LcTmString&		operator+=(char c);
	LC_IMPORT	LcTmString&		operator+=(LcTWChar c);

	// String search and manipulation etc
	LC_IMPORT	int				length() const;
	LC_IMPORT	bool			isEmpty() const;
	LC_IMPORT	LcTaString		subString(int pos, int len) const;
	LC_IMPORT	LcTaString		tail(int pos) const;
	LC_IMPORT	LcTaString		toLower() const;
	LC_IMPORT	LcTaString		toUpper() const;
	LC_IMPORT	LcTaString		getWord(int iWord, char cSep) const;
	LC_IMPORT	int				find(const char* p, int pos = 0) const;
	inline		int				find(const LcTmString& s, int pos = 0) const
									{ return find(s.getStrUtf8(), pos); }
	LC_IMPORT	void			replace(const char findStr, const char replaceStr);
	LC_IMPORT	int				findLastChar(const char p) const;

	// Parsing and formatting
	LC_IMPORT	int				toInt() const;
	LC_IMPORT	int				toBool() const;
	LC_IMPORT	LcTmString&		fromInt(int n);
	LC_IMPORT 	LcTmString& 	fromTime(IFX_TIME n);
	LC_IMPORT 	LcTmString& 	fromTime(IFX_TIME n, const LcTaString& format);

#if defined(IFX_GENERATE_SCRIPTS) || defined (IFX_USE_SCRIPTS)
		LC_EXPORT 	LcTmString&		fromLong(IFX_UINT32 n);
		LC_EXPORT	IFX_UINT32		toLong();
		LC_EXPORT 	LcTmString&		fromBool(bool);
#endif //defined(IFX_GENERATE_SCRIPTS) || defined (IFX_USE_SCRIPTS)

	LC_IMPORT	LcTScalar		toScalar() const;
	LC_IMPORT	LcTmString&		fromScalar(LcTScalar n);
	LC_IMPORT	LcTaString		toUrlSafe() const;
	LC_IMPORT	LcTaString		fromUrlSafe() const;

	// Comparison
	inline		int				compare(const LcTmString& s) const
									{ return lc_strcmp(bufUtf8(), s.bufUtf8()); }
	inline		int				compare(const char* p) const
									{ return lc_strcmp(bufUtf8(), p); }
	inline		int				compareNoCase(const LcTmString& s) const
									{ return lc_strcmpi(bufUtf8(), s.bufUtf8()); }
	inline		int				compareNoCaseN(const LcTmString& s,int n) const
										{ return lc_strnicmp(bufUtf8(), s.bufUtf8(),n); }
	inline		int				compareNoCase(const char* p) const
									{ return lc_strcmpi(bufUtf8(), p); }
	inline		int				compareNoCaseN(const char* p, int n) const
										{ return lc_strnicmp(bufUtf8(), p,n); }

	// Platform-specific string conversions
#ifdef __SYMBIAN32__
	inline		TPtrC16			toTDesC16() const // returns temporary reference, not copy
									{ return TPtrC16((TText16*)bufWChar()); }
	inline		LcTmString&		fromTDesC16(const TDesC16& d)
									{ fromBufWChar((LcTWChar*)d.Ptr(), d.Length()); return *this; }
#endif
};

// Comparison
inline bool operator==(const LcTmString& s1, const LcTmString& s2)
	{ return 0 == s1.compare(s2); }
inline bool operator==(const char* s1, const LcTmString& s2)
	{ return 0 == s2.compare(s1); }
inline bool operator==(const LcTmString& s1, const char* s2)
	{ return 0 == s1.compare(s2); }

inline bool operator!=(const LcTmString& s1, const LcTmString& s2)
	{ return 0 != s1.compare(s2); }
inline bool operator!=(const char* s1, const LcTmString& s2)
	{ return 0 != s2.compare(s1); }
inline bool operator!=(const LcTmString& s1, const char* s2)
	{ return 0 != s1.compare(s2); }

inline bool operator<(const LcTmString& s1, const LcTmString& s2)
	{ return 0 > s1.compare(s2); }
inline bool operator<(const char* s1, const LcTmString& s2)
	{ return 0 <= s2.compare(s1); }
inline bool operator<(const LcTmString& s1, const char* s2)
	{ return 0 > s1.compare(s2); }

inline bool operator>(const LcTmString& s1, const LcTmString& s2)
	{ return 0 < s1.compare(s2); }
inline bool operator>(const char* s1, const LcTmString& s2)
	{ return 0 >= s2.compare(s1); }
inline bool operator>(const LcTmString& s1, const char* s2)
	{ return 0 < s1.compare(s2); }

inline bool operator<=(const LcTmString& s1, const LcTmString& s2)
	{ return 0 >= s1.compare(s2); }
inline bool operator<=(const char* s1, const LcTmString& s2)
	{ return 0 < s2.compare(s1); }
inline bool operator<=(const LcTmString& s1, const char* s2)
	{ return 0 >= s1.compare(s2); }

inline bool operator>=(const LcTmString& s1, const LcTmString& s2)
	{ return 0 <= s1.compare(s2); }
inline bool operator>=(const char* s1, const LcTmString& s2)
	{ return 0 > s2.compare(s1); }
inline bool operator>=(const LcTmString& s1, const char* s2)
	{ return 0 <= s1.compare(s2); }


/*-------------------------------------------------------------------------*//**
	Adds constructors that may leave, and auto-cleanup via LcTaAutoT
*/
class LcTaString : public LcTaAuto<LcTmString>
{
public:

	// Construction - note that base class has no exposed ctors
	inline LcTaString()						{ }
	inline LcTaString(const LcTaString& s)	{ LcTmString::operator=(s); }
	inline LcTaString(const LcTmString& s)	{ LcTmString::operator=(s); }
	inline LcTaString(const char* p)		{ LcTmString::operator=(p); }
	inline LcTaString(char c)				{ LcTmString::operator=(c); }
};

// Concatenation
// NB: note use of LcTaString - if += fails, LHS will be cleaned up
inline LcTaString operator+(const LcTmString& s1, const LcTmString& s2)
	{ return LcTaString(s1) += s2; }
inline LcTaString operator+(const char* s1, const LcTmString& s2)
	{ return LcTaString(s1) += s2; }
inline LcTaString operator+(const LcTmString& s1, const char* s2)
	{ return LcTaString(s1) += s2; }
inline LcTaString operator+(char s1, const LcTmString& s2)
	{ return LcTaString(s1) += s2; }
inline LcTaString operator+(const LcTmString& s1, char s2)
	{ return LcTaString(s1) += s2; }

#endif //LcTStringH
