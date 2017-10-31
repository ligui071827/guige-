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
#ifndef LcCNdiBitmapFileH
#define LcCNdiBitmapFileH

// Type for entry in RLE row index (use shorts for compactness)
struct TRlePos
{
	LcTByte*	ptr;
	short		count;
	short		lit;
};

// Helper for advancing through an RLE encoding
extern void rleAdvance(TRlePos& pos, int adv);

/*-------------------------------------------------------------------------*//**
	Class to manage loading and saving of bitmaps in our proprietary
	NDI format.  NDI bitmaps can support 1-4 color channels, each with
	4-8 bits per pixel and different channels having different depths.
	When loaded, all channels are returned as 8 bits.
*/
class LcCNdiBitmapFile : public LcCBase
{
public:

	// For forcing file format conversion on load
	enum EFormat
	{
		KFormatAny,
		KFormatGraphicOpaque,
		KFormatGraphicTranslucent,
		KFormatFont
	};

private:

	// Data for opened/saved file
	LcTUInt32						m_signature;
	LcTPixelDim						m_size;
	unsigned						m_channels;
	EFormat							m_format;
	LcTmAlloc<LcTByte>				m_data;
	int								m_marginTop;
	int								m_marginBottom;
	int								m_marginLeft;
	int								m_marginRight;
	int								m_frameCount;

#ifndef LC_OMIT_NDI_OPEN
	LcTmAlloc<LcTByte>				m_rle;
	LcTmAlloc<TRlePos>				m_rleIndex;
	LcTmAlloc<LcTUInt32>			m_palette;
#endif


// Define the current NDI version.
#define LC_NDI_CURRENT_VERSION				2
#define LC_NDI_EARLIEST_SUPPORTED_VERSION	2




/*-------------------------------------------------------------------------*//**
*/
struct TNdiHeader
{
	LcTUInt32	marker;			// always ASCII %NDI
	LcTUInt16	version;		// currently 2
	LcTUInt16	headerSize;		// size of header
	LcTUInt32	signature;		// used for signing images
	LcTUInt32	dataStart;		// offset to data, if internal palette is being used.
	LcTUInt32	dataSize;		// size of data block
	LcTUInt16	imageWidth;		// in pixels
	LcTUInt16	imageHeight;	// in pixels
	LcTUInt16	marginTop;		// top margin
	LcTUInt16	marginBottom;	// bottom margin
	LcTUInt16	marginLeft;		// left margin
	LcTUInt16	marginRight;	// right margin
	LcTUInt16	frameCount;		// cell frame count
	LcTUInt16	imageType;		// Bit 0 - Image Mode (Palettized = 1) or (Non-Palettized = 0)
								// Bit 1 - Palette location (included in the NDI = 1) or (specified else where = 0)
								// Bit 2 & 3 - Image Type (11 - Normal), (01 - Font), (10 - Background).
								// Effectively Bit 2 is RGB and bit 3 specifies alpha.
	LcTUInt16	paletteSize;	// size of palette, if internal palette is being used, otherwise = 0.
};

/*-------------------------------------------------------------------------*//**
	This is used to pass raw images from the UIDesigner
*/
struct TRawImageHeader
{
	  LcTByte     marker[4];        // always ASCII %NDR
	  LcTUInt32   dataSize;         // size of data block
	  LcTUInt16   imageWidth;       // in pixels
	  LcTUInt16   imageHeight;      // in pixels
	  LcTUInt16   marginTop;        // top margin
	  LcTUInt16   marginBottom;     // bottom margin
	  LcTUInt16   marginLeft;       // left margin
	  LcTUInt16   marginRight;      // right margin
	  LcTUInt16   frameCount;       // cell frame count
	  LcTUInt16   imageType;        // Image Type 1 - Background, 2 - Graphic, 3 - Font;
};

public:

	// Owner-based creation may not be supported e.g. in command-line tools
#ifdef LcTOwnerH
	LC_IMPORT static LcTaOwner<LcCNdiBitmapFile> create();
#endif

	// Destruction
	LC_VIRTUAL						~LcCNdiBitmapFile();

#ifndef LC_OMIT_NDI_OPEN
	// Open NDI file and decode bitmap
	LC_IMPORT		bool			open(
										const LcTmString&	file,
										EFormat				format);
#endif

#ifdef LC_USE_NDI_SAVE
	// Attach the given data, which has the specified number of 8-bit channels
	LC_IMPORT		void			create(
										LcTPixelDim			size,
										unsigned			channels,
										LcTByte*			data,
										int					marginTop,
										int					marginBottom,
										int					marginLeft,
										int					marginRight,
										int					frameCount);

	// Save the first N channels of the attached data, with depths as specified
	LC_IMPORT		bool			save(
										unsigned			channels,
										LcTByte*			depths,
										const LcTmString&	file,
										bool				ditherImage);
#endif

	// Bitmap info
	inline			LcTUInt32		getSignature()		{ return m_signature; }
	inline			LcTPixelDim		getSize()			{ return m_size; }
	inline			EFormat			getFormat()			{ return m_format; }
	inline			int				getFrameCount()		{ return m_frameCount; }

// Expose data accessors only for the supported mode
#if !defined(LC_OMIT_NDI_OPEN)
#if defined(IFX_RENDER_INTERNAL_COMPRESSED)
	inline			TRlePos*		getRleIndex()		{ return m_rleIndex; }
	inline			LcTUInt32*		getPalette()		{ return m_palette; }
#endif
	inline			LcTByte*		getData()			{ return m_data; }
#endif

	// Margins
	inline			int				getMarginTop()		{ return m_marginTop; }
	inline			int				getMarginBottom()	{ return m_marginBottom; }
	inline			int				getMarginLeft()		{ return m_marginLeft; }
	inline			int				getMarginRight()	{ return m_marginRight; }

};

#endif //LcCNdiBitmapFileH
