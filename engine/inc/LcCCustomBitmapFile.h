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
#ifndef LcCCustomBitmapFileH
#define LcCCustomBitmapFileH

/*-------------------------------------------------------------------------*//**
	Class to manage loading and saving of custom non-NDI bitmaps in our 
	custom/proprietary formats. 
	When loaded, all channels are returned as 8 bits.
*/
class LcCCustomBitmapFile : public LcCBase
{
public:

	// For forcing file format conversion on load
	enum EFormat
	{
		KFormatAny,
		KFormatGraphicOpaque,
		KFormatGraphicTranslucent,
		KFormatFont,
		KFormatCompressedOpenGL
	};

private:

	// Data for opened file
	IFX_IMAGE_INFO 					m_imageInfo;
	LcTPixelDim						m_size;
	EFormat							m_format;
	LcTmAlloc<LcTByte>				m_data;
	
	// Bitmap margins
	int								m_marginTop;
	int								m_marginBottom;
	int								m_marginLeft;
	int								m_marginRight;
	int								m_frameCount;
	
	// Related to compressed bitmap data
	unsigned 						m_levelCount;
	unsigned 						m_levelFirst;
	unsigned						m_levelLast;
	unsigned 						m_textureFormat;
	
	inline void setLevelCount(unsigned levels) 
	{
		m_levelCount = levels;
	}
	
	inline void setLevelFirst(unsigned levelFirst) 
	{
		m_levelFirst = levelFirst;
	}
	
	inline void setLevelLast(unsigned levelLast) 
	{
		m_levelLast = levelLast;
	}
	
	inline void setOGLCompressionFormat(unsigned oglFormat) 
	{
		m_textureFormat = oglFormat;
	}

public:

	// Owner-based creation may not be supported e.g. in command-line tools
#ifdef LcTOwnerH
	LC_IMPORT static LcTaOwner<LcCCustomBitmapFile> create();
#endif

	// Destruction
	LC_VIRTUAL						~LcCCustomBitmapFile();

	// Open Custom Bitmap file and decode via custom handler in porting layer 
	LC_IMPORT		bool			open(const LcTmString&	file,
												int marginLeft=0,
												int marginRight=0,
												int marginTop=0,
												int marginBottom=0,
												int frameCount=1);

	// Check whether we support this extension
	LC_IMPORT		bool			checkExtension(const LcTmString& file);

	// Close Custom Bitmap file and allow resource clean-up in porting layer 
	LC_IMPORT		bool			close();

	// get Custom Bitmap data via custom handler in porting layer
	LC_IMPORT		bool			readData(void **readData,  
	                                         unsigned level, 
											 IFX_UINT32 *length, 
											 bool cacheData);

	// release Custom Bitmap data via custom handler in porting layer
	LC_IMPORT		void			releaseData(void *data);

	// Bitmap info
	inline			LcTPixelDim		getSize()			{ return m_size; }
	inline			EFormat			getFormat()			{ return m_format; }
	inline			bool			isTranslucent()		{ return m_imageInfo.translucency
																== IFX_TRANSLUCENCY_IMAGE_NONOPAQUE; }

	// Expose data accessors only for the supported mode
	inline			LcTByte*		getData()			{ return m_data; }

	// Margins
	inline			int				getMarginTop()		{ return m_marginTop; }
	inline			int				getMarginBottom()	{ return m_marginBottom; }
	inline			int				getMarginLeft()		{ return m_marginLeft; }
	inline			int				getMarginRight()	{ return m_marginRight; }
	inline			int				getFrameCount()		{ return m_frameCount; }
	
	// Related to compressed bitmap
	inline 			unsigned        getLevelCount()		{ return m_levelCount;	}
	inline          unsigned        getLevelFirst()		{ return m_levelFirst;	}
	inline          unsigned        getLevelLast()		{ return m_levelLast;	}
	inline          unsigned        getOGLCompressionFormat() { return m_textureFormat; }
};

#endif //LcCNdiBitmapFileH
