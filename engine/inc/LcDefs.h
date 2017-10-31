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
#ifndef LcDefsH
#define LcDefsH

/* Build NDE statically */
#define LC_USE_STATIC_NDE

/*-------------------------------------------------------------------------*//**
	Pull in #defines that control the NDE feature sets and NDE build modes
	for the current project - if NDE is in a DLL, we must use the NDE defs.
*/
#ifdef LC_USE_STATIC_NDE
	#include "LcProjectDefs.h"
#else
	#include "inflexionui/engine/inc/LcProfileDefs.h"
#endif

/*-------------------------------------------------------------------------*//**
	Platform-specific feature set dependencies, or other mode dependencies
	that should NOT affect makefiles that may have included LcProfileDefs.h
*/

// Include stylus support if configured by the customer.
#if defined(IFX_USE_STYLUS)
	#define LC_USE_STYLUS
#endif

// Include mouseover support if configured by the customer (and stylus enabled).
#if defined(IFX_USE_MOUSEOVER) && defined(IFX_USE_STYLUS)
	#define LC_USE_MOUSEOVER
#endif

// Must repaint whole screen if highlighting bboxes
#if defined(LC_PAINT_DEBUG)
	#define LC_PAINT_FULL
#endif

// If we're using romized themes, do not attempt to save settings
#if defined(LC_USE_XML_SAVE) && defined(IFX_USE_ROM_FILES)
	#undef LC_USE_XML_SAVE
#endif

/* Enable script saving when script generation is enabled */
#ifdef IFX_GENERATE_SCRIPTS
	#ifndef LC_USE_XML_SAVE
		#define LC_USE_XML_SAVE
	#endif
#endif

#ifdef __cplusplus
	// Set the namespace
	#define LC_STL_NAMESPACE		std
	#define LC_STL_NAMESPACE_SEP	::
#endif
/*-------------------------------------------------------------------------*//**
	Platform-specific controls based on selections above or controls in
	project options.  These may constrain build options, alter modes, etc.
*/

#define LC_NO_RTTI

// Enable the using std:: within LcStl.h
#define LC_STL_USING_ENABLED

#ifdef __cplusplus
	extern "C" {
#endif
		// Use memory allocation implemented in LcCEnvironment.
		void* allocateMemory(size_t size);
		void  deallocateMemory(void* ptr);
        void* allocateMemoryUnsafe(size_t size);
        void* allocateMemoryExternal(size_t size);
        void  deallocateMemoryExternal(void* ptr);

#if !defined(NDHS_PACKAGER) && !defined(NDHS_INTEGRATOR)
		IFXE_FILE getRomFile (IFX_UINT32 romFileCount, LcTRomFileEntry* pRomFileTable, const char* file_name);
#endif

#ifdef __cplusplus
	}
#endif
	
	#define LcAllocateMemory 			allocateMemory
	#define LcDeallocateMemory 			deallocateMemory

	#define LcAllocateMemoryUnsafe 		allocateMemoryUnsafe
	#define LcDeallocateMemoryUnsafe	deallocateMemory

    #define LcAllocateMemoryExternal    allocateMemoryExternal
    #define LcDeallocateMemoryExternal  deallocateMemoryExternal

#if !defined(NDHS_PACKAGER) && !defined(NDHS_INTEGRATOR)
	#define LcGetRomFile				getRomFile
#endif	

// Things required for ARM RealView tools
#if defined(__CC_ARM)
	
	// Extras needed for the ADS 1.2 Toolset.
	#if (defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 210000))
		// Disable using std:: within LcStl.h
		#undef LC_STL_USING_ENABLED

		// Redefine the namespace
		#undef  LC_STL_NAMESPACE
		#define LC_STL_NAMESPACE

		#undef  LC_STL_NAMESPACE_SEP
		#define LC_STL_NAMESPACE_SEP
	#endif

// Things required for Edge Arm tools
#elif defined(_MRI)
	#include <stdlib.h>
	#if defined(__cplusplus)
		// Gives access	to the wchar functions in Edge Arm tools 1.3 and greater
		using namespace	LC_STL_NAMESPACE;

		// Disable exception support if necessary
		#if	!_EH
			#define	THROW_NONE
			#define	THROW_BAD_ALLOC
		#endif

		extern int		rtl_strcmpi(const char *, const	char *);
	#endif
// Things required for TI CodeComposer tools
#elif defined(__TMS470__)

	// Throw is not supported on this compiler.
	#define THROW_NONE
	#define THROW_BAD_ALLOC

	#if defined(LC_OMAP1710)
		// The Runtime libraries are not initialized on this chip.
		#ifdef __cplusplus
			#define _NS_PREFIX std::
			namespace std {
		#else
			#define _NS_PREFIX
		#endif // __cplusplus

		typedef char* va_list;

		#ifdef __cplusplus
			} // namespace std
		#endif

		#define va_start(ap, parmN)                                                   \
		   ((ap) = (!__va_argref(parmN) && sizeof(parmN) == sizeof(double)            \
				? (_NS_PREFIX va_list)((int)__va_parmadr(parmN) + 8)                  \
					: (!__va_argref(parmN) && sizeof(parmN) <= sizeof(short))         \
					? (_NS_PREFIX va_list)((int)__va_parmadr(parmN) + 4 & ~3)         \
					: (_NS_PREFIX va_list)((int)__va_parmadr(parmN) + 4 )))


		#ifdef __big_endian__
			#define va_arg(_ap, _type)                                                    \
					(__va_argref(_type)                                                   \
				 ? ((_ap += sizeof(_type*)),(**(_type**)(_ap-(sizeof(_type*)))))          \
					 : ((sizeof(_type) == sizeof(double)                                  \
						 ? ((_ap += 8), (*(_type *)(_ap - 8)))                            \
						 : ((_ap += 4), (*(_type *)(_ap - (sizeof(_type))))))))
		#else
			#define va_arg(_ap, _type)                                                    \
					(__va_argref(_type) 						      \
				 ? ((_ap += sizeof(_type*)),(**(_type**)(_ap-(sizeof(_type*)))))          \
					 : ((sizeof(_type) == sizeof(double)                                  \
						 ? ((_ap += 8), (*(_type *)(_ap - 8)))                            \
						 : ((_ap += 4), (*(_type *)(_ap - 4))))))
		#endif

		#define va_end(ap)

		#ifdef __cplusplus
			#ifndef _CPP_STYLE_HEADER
				using std::va_list;
			#endif // _CPP_STYLE_HEADER
		#endif // __cplusplus
	#endif

// Things required for Green Hills MIPS tools
#elif defined (__ghs__)

// Things required for SIM/VC++
#else
	#define _CONSOLE
	#define _LIB
#endif


/*-------------------------------------------------------------------------*//**
	Endianness independent extraction macros.
	Themes are always packaged on little endian	processors, so problems will
	occur extracting data from the packaged themes on big endian processors
	unless these macros are used.
*/
#define GETB8(p)	( (*((unsigned char*)p)) & 0xFF )
#define GETB16(p)	GETB8(p) | (GETB8(((unsigned char*)p) + 1) << 8)
#define GETB32(p)	GETB8(p) | (GETB8(((unsigned char*)p) + 1) << 8) | (GETB8(((unsigned char*)p) + 2)<<16) | (GETB8(((unsigned char*)p) + 3)<<24)

/*-------------------------------------------------------------------------*//**
	Data writing macros that enforce little-endian output
*/
#define SETB8(p, val)			( (*((unsigned char*)p)) = val & 0xFF )
#define SETB16_LE(p, val)		SETB8(p, val); SETB8(((unsigned char*)p) + 1, val >> 8)
#define SETB32_LE(p, val)		SETB8(p, val); SETB8(((unsigned char*)p) + 1, val >> 8); SETB8(((unsigned char*)p) + 2, val >> 16); SETB8(((unsigned char*)p) + 3, val >> 24)

/*-------------------------------------------------------------------------*//**
	Data writing macros that enforce little-endian output, and increment the
	input pointer to the start of the next address to write to
*/

// Have to remove cast becasue mingw5.1.3 gives errors
// i.e. ISO C++ forbids cast to non-reference type used as lvalue
#define SETB8_INC(p, val)		SETB8(p, val); (p)++
#define SETB16_LE_INC(p, val)	SETB16_LE(p, val); (p) += 2
#define SETB32_LE_INC(p, val)	SETB32_LE(p, val); (p) += 4

/*-------------------------------------------------------------------------*//**
Data writing macros that enforce big-endian output
*/
#define SETB16_BE(p, val)		SETB8(p, val >> 8); SETB8(((unsigned char*)p) + 1, val)
#define SETB32_BE(p, val)		SETB8(p, val >> 24); SETB8(((unsigned char*)p) + 1, val >> 16); SETB8(((unsigned char*)p) + 2, val >> 8); SETB8(((unsigned char*)p) + 3, val)


/*-------------------------------------------------------------------------*//**
	Import/export directives
*/
#ifdef LC_USE_STATIC_NDE
	#ifdef LC_BUILD_NDE_DLL
		#error "Cannot build NDE.DLL with static link specified in header!"
	#endif

	// No import/export required
	#define LC_IMPORT
	#define LC_EXPORT
	#define LC_VIRTUAL				virtual
	#define LC_EXPORT_VIRTUAL
#else
	#if !defined(LC_IMPORT) || !defined(LC_EXPORT) || !defined(LC_VIRTUAL) || !defined(LC_EXPORT_VIRTUAL)
		#error Please define LC_IMPORT, LC_EXPORT, LC_VIRTUAL and LC_EXPORT_VIRTUAL for a dll build
	#endif
#endif

typedef enum
{
	EFull,
	EPartial,
	EOff
} ETappable;


/*-------------------------------------------------------------------------*//**
	Internal class member access specifiers
*/
#if defined(LC_BUILD_NDE_DLL) || defined(LC_USE_STATIC_NDE)
	#define LC_PRIVATE_INTERNAL_PUBLIC		public
	#define LC_PRIVATE_INTERNAL_PROTECTED	protected
	#define LC_PROTECTED_INTERNAL_PUBLIC	public
#else
	#define LC_PRIVATE_INTERNAL_PUBLIC		private
	#define LC_PRIVATE_INTERNAL_PROTECTED	private
	#define LC_PROTECTED_INTERNAL_PUBLIC	protected
#endif

/*-------------------------------------------------------------------------*//**
	Compiler controls
*/
// Use header stops for precompiled headers where possible
#define LC_HDRSTOP

#if defined(_MSC_VER)
	#pragma warning(disable:4786) // mangled name truncated
//	#pragma warning(disable:4503)
	#pragma warning(disable:4355) // "this" used in member/base initializer
	#pragma warning(disable:4127) // conditional expression constant
	#pragma warning(disable:4786) // debug name truncated to 255 chars
#endif

#if defined(__CC_ARM) && !(defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 210000))
	// 997 is "virtual function override intended?" warnings
	// 554 is "LcTTime::operator void() const" will not be called for implicit or explicit conversions
	#pragma diag_suppress 1, 1300, 997, 554
#endif

#if	defined(_MRI)
	#if defined(_MCCCF)
		/* Do Nothing */
	#elif defined(_CCCCF)
		// C0554 is	"LcTTime::operator void() const" will not be called for implicit or explicit conversions	
		#pragma	options	-QmsC0554
		/* ColdFire 5.5G C++ Compiler miscalculate warning message number. It should be C0997 instead of C1003.
           Workaround to suppress the warning message using number C1003. */
		#pragma	options	-QmsC1003
	#else /* for EDGE PowerPC, EDGE ARM */
		// C0554 is	"LcTTime::operator void() const" will not be called for implicit or explicit conversions	
		#pragma	options	-QmsC0554
		// C0997 is	"virtual function override intended?" warning
		#pragma	options	-QmsC0997
	#endif
#endif

// Disable nothrow new and delete operators for ADS toolset.
#if defined(__CC_ARM) && (defined(__ARMCC_VERSION) && (__ARMCC_VERSION < 210000))
	#define LC_STL_DISABLE_NOTHROW
#endif

// Disable nothrow new and delete operators for Visual Studio <= version 6.0
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
	#define LC_STL_DISABLE_NOTHROW
#endif

#if defined(__GNUC__)
// GCC doesn't support this pragma
#undef LC_HDRSTOP
#endif
/*-------------------------------------------------------------------------*//**
	Debug macros
*/
#ifdef IFX_ASSERT
	// Default is to map our LC_ASSERT onto the IFX_ASSERT
	#define LC_ASSERT(b)		IFX_ASSERT(b)
#else
	#define LC_ASSERT(b)
#endif
/*-------------------------------------------------------------------------*//**
	Mode-specific modifications.  These are further conditional sections
	based on modes or build options, but are not platform-specific except
	where platform-specific sections above have enabled/disabled modes
*/

// Implementations of macros for doing RTTI stuff
#ifdef LC_NO_RTTI

	// To trap mistakes
	#define dynamic_cast	ERROR_MUST_USE_LC_DYNAMIC_CAST
	#define typeid			ERROR_MUST_USE_LC_TYPEID_NAME

	// Dynamic cast defers instance-of test to implemented methods
	#define LC_DYNAMIC_CAST(c, p)								\
		((p) && (p)->LC_isInstanceOf(#c) ? (c*)(p) : NULL)

	// Type ID request gets name via virtual methods
	#define LC_TYPEID_NAME(p)									\
		(p)->LC_getTypeID()

	// Base class instance-of test, makes class abstract
	#define LC_DECLARE_RTTI_BASE(c)								\
		public:													\
			virtual const char* LC_getTypeID() = 0;				\
			virtual bool LC_isInstanceOf(const char* s)			\
				{ return !lc_strcmp(s, #c); }						\
		private:

	// Derived class instance-of test
	#define LC_DECLARE_RTTI(c, bc)								\
			typedef bc LC_bc;									\
		public:													\
			virtual const char* LC_getTypeID()					\
				{ return #c; }									\
			virtual bool LC_isInstanceOf(const char* s)			\
				{ return !lc_strcmp(s, #c) || LC_bc::LC_isInstanceOf(s); }	\
		private:

#else
	// Compiler RTTI
	#include <typeinfo>

	// With built-in RTTI, we don't need isInstanceOf() methods
	#define LC_DYNAMIC_CAST(c, p)								\
		dynamic_cast<c*>(p)

	// Nor do we need getTypeID() methods
	#define LC_TYPEID_NAME(p)									\
		typeid(*(p)).name()

	// We don't need to add anything to classes
	#define LC_DECLARE_RTTI_BASE(c)
	#define LC_DECLARE_RTTI(c, bc)
#endif

/*-------------------------------------------------------------------------*//**
	Default type for 3D space coordinates, and scalars thereof.
	The type must be able accurately to represent coordinates, and also one
	coordinate divided by another
*/

typedef float					LcTScalar;
typedef double					LcTDScalar;
typedef float					LcTUnitScalar;

// Constants - tenth is often used for near-zero checks
#define LC_PI					3.1415926535897932384626433832795f
#define LC_1_PI					(1.0f/LC_PI)
#define LC_TENTH				(1.0f/10.0f)
#define LC_ALMOST_ZERO			(1.0f/512.0f)

// Conversions
#define LC_RADIANS(a)			((a) * (LC_PI / 180))
#define LC_DEGREES(a)			((a) * (180 / LC_PI))

// Unit scalar macros
#define LC_UNIT_ALMOST_ZERO		(1.0f/32768.0f)
#define LC_MULT_BY_UNIT(a, b)	((float)(a) * (float)(b))
#define LC_DIV_BY_UNIT(a, b)	((float)(a) / (float)(b))
#define LC_DIV_TO_UNIT(a, b)	((float)(a) / (float)(b))
#define LC_FLOOR(a)				(floor(a))

/*-------------------------------------------------------------------------*//**
	General useful types and defines
*/
typedef unsigned		LcTUInt32;
typedef unsigned short	LcTUInt16;
typedef unsigned char	LcTByte;

#define LC_DEFAULT_LANGUAGE			"en"
#define LC_DEFAULT_DISPLAY_MODE		"QVGA_P"

//Threshold values for rounding alpha'd pixels to opaque/transparent
#define LC_ALPHA_MAX				245
#define LC_ALPHA_MIN				10
#define	LC_OPAQUE_THRESHOLD		   (LC_ALPHA_MAX / 255.0f)
#define LC_TRANSPARENT_THRESHOLD   (LC_ALPHA_MIN / 255.0f)

#if !defined(THROW_NONE)
	#define THROW_NONE				throw()
	#define THROW_BAD_ALLOC			throw(LC_STL_NAMESPACE LC_STL_NAMESPACE_SEP bad_alloc)
#endif

/* File access types */
typedef enum
{
	ETheme = 0,
	EShader
} ERomFileSystem;


// Macro to prevent compiler warnings on unused variables
#define LC_UNUSED(a)				(void)a;

#endif // LcDefsH

