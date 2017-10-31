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
#ifndef LcTColorH
#define LcTColorH

// Annoying int-char conversion warning (re-enabled at bottom)
#if defined(_MSC_VER)
	#pragma warning(disable:4244)
#endif

/*-------------------------------------------------------------------------*//**
	Abstracts a NDE color value.  Color values are guaranteed to fit into
	an int and NDE allows conversion to/from ints for storage.  However,
	any extraction of color values from bytes should be done via this class
	as byte order may differ between platforms.
*/
class LcTColor
{
public:

	// Some compilers don't like anonymous structs
	typedef struct
	{
		unsigned char	a;
		unsigned char	b;
		unsigned char	g;
		unsigned char	r;
	} _TRgba;

	// Color components - for fastest compatibility with OpenGL we order RGBA little-endian
	union
	{
		int		_all;
		_TRgba	rgba;
	};

	// Special colors
	enum EColor
	{
		NONE	= 0x80000000, // special value invalid for RGB8
		BLACK	= 0x00000000,
		WHITE	= 0x00FFFFFF,
		RED		= 0x00FF0000,
		GREEN	= 0x0000FF00,
		BLUE	= 0x000000FF,
		GRAY20	= 0x00333333,
		GRAY50	= 0x00808080
	};

	// Default copy behavior, but __TMS470__ chokes without this
	LcTColor(const LcTColor& c)
		{ _all = c._all; }

	// Construction
	LcTColor(unsigned char ir, unsigned char ig, unsigned char ib, unsigned char ia) 
		{ rgba.r = ir; rgba.b = ib; rgba.g = ig; rgba.a = ia; }

	// Conversion from RGB8 integer - this is the standard used in e.g. XML
	LcTColor(int i = NONE) 
#if defined (IFX_USE_BIG_ENDIAN)
        // Assignment on a BE platform needs to swap the byte order.
		{ SETB32_LE(&_all, (i == int(NONE))? 0 : ((i << 8) | 0x0FF)); }
#else
		{ _all = (i == int(NONE))? 0 : ((i << 8) | 0x0FF); }
#endif

	// Conversion to various formats for use in APIs - OpenGL's RGBA8 is most efficient
	int rgb8()  
		{ return (( rgba8() >> 8) & 0x00FFFFFF); }
	int rgba8()  
		{ int res; SETB32_LE(&res, _all); return res;  }
	int argb8()  
		{ int res = rgba8(); return (( res >> 8) & 0x00FFFFFF) | ((res << 24) & 0xFF000000); }

	// Combine two colors
	static LcTColor mixColors(LcTColor c0, LcTColor c1, LcTScalar dWeight)
	{
		return LcTColor(
			c0.rgba.r + (unsigned char)(int)(dWeight * ((int)c1.rgba.r - c0.rgba.r)),
			c0.rgba.g + (unsigned char)(int)(dWeight * ((int)c1.rgba.g - c0.rgba.g)),
			c0.rgba.b + (unsigned char)(int)(dWeight * ((int)c1.rgba.b - c0.rgba.b)),
			c0.rgba.a + (unsigned char)(int)(dWeight * ((int)c1.rgba.a - c0.rgba.a)));
	}
	
	bool isWhite()
		{ return (rgba.r == 0xFF) && (rgba.g == 0xFF) && (rgba.b == 0xFF); }
};

// Comparison
inline bool operator==(LcTColor c1, LcTColor c2)
	{ return c1.rgba8() == c2.rgba8(); }
inline bool operator!=(LcTColor c1, LcTColor c2)
	{ return c1.rgba8() != c2.rgba8(); }

// Annoying int-char conversion warning might be important elsewhere
#if defined(_MSC_VER)
	#pragma warning(default:4244)
#endif

#endif //LcTColorH
