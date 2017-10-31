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


/*-------------------------------------------------------------------------*//**
	Notes on NDI file format
	PROPRIETARY AND CONFIDENTIAL

	The NDI file comprises a header (see below), followed by an optional palette
	of up to 256 color values of 4 bytes each, followed by a block of data at
	a file offset specified in the header.

	NOTE: we currently have the following 3 cases:
	 - Font	images.  Alpha channel only.
	 - Backgrounds.  Images with no alpha channel.
	 - Normal.  Images with an alpha channel (icons, furniture, etc).

	The data comprises one or two run-length encoded streams - an 8-bit
	value for the alpha channel, and/or an 8-bit color palette index for images with
	color data.  Rasters are encoded top-down.  Rows are not padded or aligned.

	Encoding is as a sequence of runs of varying lengths.  Runs are either of
	repeated channel values, or of literal (different to each other) channel
	values.  Runs never span channel boundaries.

	Each run starts with a 8-bit marker.  The top bit is a control bit and
	the lower 7 bits are a count.  The control bit determines whether the
	run is literal (1) or repeat (0).

	Following a repeat marker/count, comes a single 8-bit color or alpha value.
	Following a literal marker/count, comes N such color values, where N is the
	count.
*/

#ifdef LcTOwnerH
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCNdiBitmapFile> LcCNdiBitmapFile::create()
{
	LcTaOwner<LcCNdiBitmapFile> ref;
	ref.set(new LcCNdiBitmapFile);
	ref->construct();
	return ref;
}
#endif

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL LcCNdiBitmapFile::~LcCNdiBitmapFile()
{
	m_data.free();

#if !defined(LC_OMIT_NDI_OPEN)
	m_rle.free();
	m_rleIndex.free();
	m_palette.free();
#endif
}

#ifndef LC_OMIT_NDI_OPEN
/*-------------------------------------------------------------------------*//**
	Decode routine is designed to be fast
*/
LC_EXPORT bool LcCNdiBitmapFile::open(const LcTmString& fileName, EFormat format)
{
	// Open file
	LcTaOwner<LcCReadOnlyFile> file = LcCReadOnlyFile::openFile(fileName);
	if (!file)
		return false;

#if defined(NDHS_JNI_INTERFACE)
	// Load the header
	int headerSize = sizeof(TRawImageHeader);
	TRawImageHeader rawHeader;
	memset(&rawHeader, 0, headerSize);

	if (!file->read(&rawHeader, headerSize, 1))
	{
		file->close();
		return false;
	}

	// Check that this is an NDR file...
	if (lc_strnicmp((const char*)&rawHeader.marker[0], "%ndr", 4) != 0)
	{
		file->close();
		return false;
	}

	// Set up the image member variables.
	// Determine the format based on # of stored channels
	EFormat decodedFormat;
	switch (rawHeader.imageType)
	{
		case 1:	// Opaque
		{
			decodedFormat	= KFormatGraphicOpaque;
			m_channels		= 3;
			break;
		}
		case 2:	// Translucent
		{
			decodedFormat	= KFormatGraphicTranslucent;
			m_channels		= 4;
			break;
		}
		case 3:	// Font
		{
			decodedFormat	= KFormatFont;
			m_channels		= 1;
			break;
		}
		default:
		{
			file->close();
			return false;
			break;
		}
	}

	// Validate the format is correct.
	if (format == KFormatAny)
		format = decodedFormat;
	else if (format != decodedFormat)
	{
		file->close();
		return false;
	}
	m_format = format;

	// Validate the margins are not too big.
	if ((rawHeader.marginTop + rawHeader.marginBottom) < rawHeader.imageHeight)
	{
		m_marginTop		= rawHeader.marginTop;
		m_marginBottom	= rawHeader.marginBottom;
	}
	else
	{
		NDHS_TRACE_EXT(ENdhsTraceLevelError, NULL, "The top and bottom margins are greater than the image height", fileName, -1);
	}
	if ((rawHeader.marginLeft + rawHeader.marginRight) < rawHeader.imageWidth)
	{
		m_marginLeft	= rawHeader.marginLeft;
		m_marginRight	= rawHeader.marginRight;
	}
	else
	{
		NDHS_TRACE_EXT(ENdhsTraceLevelError, NULL, "The left and right margins are greater than the image width", fileName, -1);
	}

	// The rest of the attributes.
	m_size.width	= rawHeader.imageWidth;
	m_size.height	= rawHeader.imageHeight;
	m_frameCount	= rawHeader.frameCount;


	// Read the image data.
	unsigned outputSize = m_size.width * m_size.height * m_channels;
	m_data.alloc(outputSize);

	if (m_data == NULL)
	{
		// not enough memory
		file->close();
		return false;
	}

	bool success = (m_data && (rawHeader.dataSize == file->read(m_data, 1, rawHeader.dataSize)));

	file->close();

	return success;

#else

	// Read the first three NDI header sections and validate the file, header
	// size and version number.
	const int initialHeaderSize = sizeof(LcTUInt32) + (2 * sizeof(LcTUInt16));
	TNdiHeader header;
	memset(&header, 0, sizeof(header));

	LcTaAlloc<LcTByte> pBuffOrig(initialHeaderSize);
	LcTByte* pBuff = pBuffOrig;

	if(pBuff == NULL)
	{
		file->close();
		return false;
	}

	if (!file->read(pBuff, initialHeaderSize, 1))
	{
		file->close();
		return false;
	}

	//check that this is an NDI file...
	if( GETB8(pBuff) == '%'
	   && (GETB8(pBuff+1) == 'N' || GETB8(pBuff+1) == 'n')
	   && (GETB8(pBuff+2) == 'D' || GETB8(pBuff+2) == 'd')
	   && (GETB8(pBuff+3) == 'I' || GETB8(pBuff+3) == 'i') )
	{
		//now use the endian-safe macros to populate the first part of the header structure
		header.marker = GETB32(pBuff);
		pBuff += sizeof(LcTUInt32);
		header.version = GETB16(pBuff);
		pBuff += sizeof(LcTUInt16);
		header.headerSize = GETB16(pBuff);
	}
	else
	{
		file->close();
		return false;
	}

	// We can free the buffer now.
	pBuffOrig.free();

	// Check the file version to see if we support it.
	if ((header.version > LC_NDI_CURRENT_VERSION)
#if (LC_NDI_EARLIEST_SUPPORTED_VERSION > 0)
		|| (header.version < LC_NDI_EARLIEST_SUPPORTED_VERSION)
#endif
		)
	{
		file->close();
		return false;
	}

	pBuffOrig.alloc(header.headerSize - initialHeaderSize);
	pBuff = pBuffOrig;

	if (pBuff == NULL)
	{
		file->close();
		return false;
	}

	// Read the rest of the header this will use header size data to ensure we
	// do not read data into the struct.
	if (!file->read(pBuff, header.headerSize - initialHeaderSize, 1))
	{
		file->close();
		return false;
	}

	//	now use the endian-safe macros to populate the rest of the header structure
	header.signature = GETB32(pBuff);
	pBuff += sizeof(LcTUInt32);
	header.dataStart = GETB32(pBuff);
	pBuff += sizeof(LcTUInt32);
	header.dataSize = GETB32(pBuff);
	pBuff += sizeof(LcTUInt32);
	header.imageWidth = GETB16(pBuff);
	pBuff += sizeof(LcTUInt16);
	header.imageHeight = GETB16(pBuff);
	pBuff += sizeof(LcTUInt16);
	header.marginTop = GETB16(pBuff);
	pBuff += sizeof(LcTUInt16);
	header.marginBottom = GETB16(pBuff);
	pBuff += sizeof(LcTUInt16);
	header.marginLeft = GETB16(pBuff);
	pBuff += sizeof(LcTUInt16);
	header.marginRight = GETB16(pBuff);
	pBuff += sizeof(LcTUInt16);
	header.frameCount = GETB16(pBuff);
	pBuff += sizeof(LcTUInt16);
	header.imageType = GETB16(pBuff);
	pBuff += sizeof(LcTUInt16);
	header.paletteSize = GETB16(pBuff);

	// We can free the buffer now
	pBuffOrig.free();

	// Determine the format based on # of stored channels
	EFormat decodedFormat;
	if ((header.imageType & 0x0C) == 0x08)
		decodedFormat = KFormatFont;
	else if ((header.imageType & 0x0C) == 0x04)
		decodedFormat = KFormatGraphicOpaque;
	else if ((header.imageType & 0x0C) == 0x0C)
		decodedFormat = KFormatGraphicTranslucent;
	else
	{
		file->close();
		return false;
	}

	// Validate the format is correct.
	if (format == KFormatAny)
		format = decodedFormat;
	else if (format != decodedFormat)
	{
		file->close();
		return false;
	}
	m_format = format;

	// Check if the NDI is palettized.
	bool bPalette = ((header.imageType & 0x01) == 0x01);

	// If this is not palettized then it is an error.
	if (!bPalette && (format != KFormatFont))
	{
		// Error abort file.
		file->close();
		return false;
	}

	int paletteCount = header.paletteSize / 4;

	// Allocate the palette, this is freed when the class is destroyed.
	if (bPalette && (format != KFormatFont))
	{
		m_palette.alloc(paletteCount);
		if (!m_palette)
		{
			file->close();
			return false;
		}

		// Fill palette buffer - note that header size may differ from our struct size
		if (!file->seek(header.headerSize, LcCReadOnlyFile::EStart)
			||	((format != KFormatFont) && header.paletteSize > file->read(m_palette, 1, header.paletteSize)))
		{
			file->close();
			return false;
		}
	}


	m_rle.alloc(header.dataSize + 2);
	if (!m_rle)
	{
		file->close();
		return false;
	}

	// Fill data buffer - note that header size may differ from our struct size
	if (!file->seek(header.dataStart, LcCReadOnlyFile::EStart)
		||	header.dataSize > file->read(m_rle, 1, header.dataSize))
	{
		file->close();
		return false;
	}

	// Put a marker here as simple check for decoding error
	// NB: this also makes it OK to read 1 byte past the end, although data may be invalid
	m_rle[header.dataSize] = 0x5B;

	// Done with file already
	file->close();

	// Alpha channel number
	int channelAlpha	= -1;
	int rleChannelAlpha	= -1;

	// Number of separate channels.
	int encodings;

	// Now check/set channel counts according to format
	switch (m_format)
	{
		// Can mask to RGB from stored RGBA
		case KFormatGraphicOpaque:
		{
			m_channels		= 3;
			encodings		= 1;

			#if defined(IFX_RENDER_INTERNAL_COMPRESSED)
                #if defined(IFX_CANVAS_ORDER_BGR) || \
                    (!defined(IFX_CANVAS_MODE_888) && \
                    !defined(IFX_CANVAS_MODE_8888))
                    
                    // Backgrounds are expanded to their native format
                    // immediately to speed up blitting.
                    int i;
                    LcTUInt32* pPalette = m_palette;
                    for (i = 0; i < paletteCount; i++)
                    {
                        LcTUInt32 currPalette = *pPalette;

                        #if defined(IFX_CANVAS_ORDER_BGR)
                            // Swap the Red and Blue components before converting to native
                            *pPalette = (LcTUInt32)(
                                (((currPalette >> 0)  & 0xFF) << 16)
                            |	(((currPalette >> 8)  & 0xFF) << 8)
                            |	(((currPalette >> 16) & 0xFF) << 0));
                            
                            currPalette = *pPalette;
                        #endif
                        
                        // Convert the palette to the native format for backgrounds
                        // Reduce pixel into 16bpp destination (565)
                        #if defined(IFX_CANVAS_MODE_565)
                            #if !defined (IFX_USE_BIG_ENDIAN)
                                *pPalette = (LcTUInt32)(
                                    ((((currPalette >> 0)  & 0xFF) & 0x00F8) << 8)
                                |	((((currPalette >> 8)  & 0xFF) & 0x00FC) << 3)
                                |	((((currPalette >> 16) & 0xFF) & 0x00F8) >> 3));
                            #else
                                *pPalette = (LcTUInt32)(
                                    ((((currPalette >> 24) & 0xFF) & 0x00F8) << 8)
                                |	((((currPalette >> 16) & 0xFF) & 0x00FC) << 3)
                                |	((((currPalette >>  8) & 0xFF) & 0x00F8) >> 3));
                            #endif
                        // Reduce pixel into 16bpp destination (555)
                        #elif defined(IFX_CANVAS_MODE_1555)
                            #if !defined (IFX_USE_BIG_ENDIAN)
                                *pPalette = (LcTUInt32)(
                                    ((((currPalette >> 0)  & 0xFF) & 0x00F8) << 7)
                                |	((((currPalette >> 8)  & 0xFF) & 0x00F8) << 2)
                                |	( ((currPalette >> 16) & 0xFF)           >> 3));
                            #else
                                *pPalette = (LcTUInt32)(
                                    ((((currPalette >> 24) & 0xFF) & 0x00F8) << 7)
                                |	((((currPalette >> 16) & 0xFF) & 0x00F8) << 2)
                                |	( ((currPalette >>  8) & 0xFF)           >> 3));
                            #endif
                            #if defined(IFX_CANVAS_ORDER_BGR)
                                *pPalette = (*pPalette << 1) & 0xFFFE;
                            #endif
                        // Reduce pixel into 12bpp destination
                        #elif defined(IFX_CANVAS_MODE_444)
                            #if !defined (IFX_USE_BIG_ENDIAN)
                                *pPalette = (LcTUInt32)(
                                    ((((currPalette >> 0)  & 0xFF) & 0x00F0) << 4)
                                |	((((currPalette >> 8)  & 0xFF) & 0x00F0))
                                |	( ((currPalette >> 16) & 0xFF)           >> 4));
                            #else
                                *pPalette = (LcTUInt32)(
                                    ((((currPalette >> 24) & 0xFF) & 0x00F0) << 4)
                                |	((((currPalette >> 16) & 0xFF) & 0x00F0))
                                |	( ((currPalette >>  8) & 0xFF)           >> 4));
                            #endif
                            #if defined(IFX_CANVAS_ORDER_BGR)
                                *pPalette = (*pPalette << 4) & 0xFFF0;
                            #endif
                        #else
                            //Nothing required for ARGB8888 and RGB888 modes
                        #endif
                
                        ++pPalette;
                    }
                #endif
			#endif // defined(IFX_RENDER_INTERNAL_COMPRESSED)

			break;
		}

		// Can pad to RGBA from stored RGB
		case KFormatGraphicTranslucent:
		{
			// NB: channelAlpha will be ignored if padded
			m_channels		= 4;
			channelAlpha	= 3;
			rleChannelAlpha	= 0;

			encodings = 2;
			break;
		}

		// Cannot convert to/from alpha-only
		case KFormatFont:
		{
			m_channels		= 1;
			channelAlpha	= 0;
			rleChannelAlpha	= 0;

			encodings = 1;
			break;
		}

		// This should never happen.
		default:
		{
			return false;
		}
	}

	// Extract other details from header
	m_size.width				= header.imageWidth;
	m_size.height				= header.imageHeight;

	// Version 1 NDI onwards.
	if (header.version > 0)
	{
		// Validate the margins are not too big.
		if ((header.marginTop + header.marginBottom) < header.imageHeight)
		{
			m_marginTop		= header.marginTop;
			m_marginBottom	= header.marginBottom;
		}
		if ((header.marginLeft + header.marginRight) < header.imageWidth)
		{
			m_marginLeft	= header.marginLeft;
			m_marginRight	= header.marginRight;
		}

		m_frameCount				= header.frameCount;
	}


	bool success = true;

// Expand the data into the pixel memory to enable fast drawing
// When we are using compressed blitting we only use this for fonts.
#if defined(IFX_RENDER_INTERNAL_COMPRESSED)
	if (m_format == KFormatFont)
#endif
	{
		// Input data pointers
		LcTByte*	inputCurrent	= m_rle;
		LcTByte*	inputEnd		= m_rle + header.dataSize;

		// Read image properties
		int			channelIndex	= 0;

		// Allocate space for unpacked data
		unsigned	outputSize		= m_size.width * m_size.height * m_channels;

		m_data.alloc(outputSize);
		if (!m_data)
			return false;

		LcTByte*	outputCurrent	= m_data;
		LcTByte*	outputEnd		= m_data + outputSize;

		// Pad the data structure.
		lc_memset(m_data, 0xFF, outputSize);

		// State for read loop
		enum {
			eBetween,
			eRunLiteral,
			eRunRepeat
		} state;

		// Loop state
					state			= eBetween;
		unsigned	decodedVal		= 0;
		unsigned	runCount		= 0;

		// Keep on reading till we're done
		success = false;
		while (!success)
		{
			// If we have no more buffered data, file must be truncated
			// so just break out of the loop to clean up and return false
			if (inputCurrent >= inputEnd)
				break;

			// Get the next byte and load into the barrel
			decodedVal = *inputCurrent++;

			// File is decoded via simple state machine
			switch (state)
			{
				// Decoded val will be 6 bits
				case eBetween:
				{
					// Enter run state now
					if (decodedVal & 0x80)
						state		= eRunLiteral;
					else
						state		= eRunRepeat;

					// Now read channel values
					runCount	= (decodedVal & 0x7F);
					break;
				}

				// Decoded val will match channel depth
				case eRunLiteral:
				{
					// Retrieve the decoded value.
					register LcTUInt32 srcRgb = decodedVal;

					// Put into next output pos
					if (channelIndex == rleChannelAlpha)
					{
						outputCurrent[channelAlpha]	= (LcTByte)srcRgb;
					}
					else
					{
						// Extract the palettized data.
						srcRgb				= m_palette[decodedVal];
						#if !defined (IFX_USE_BIG_ENDIAN)
							outputCurrent[0]	= (srcRgb >>  0) & 0xFF;
							outputCurrent[1]	= (srcRgb >>  8) & 0xFF;
							outputCurrent[2]	= (srcRgb >> 16) & 0xFF;
						#else
							outputCurrent[0]	= (srcRgb >> 24) & 0xFF;
							outputCurrent[1]	= (srcRgb >> 16) & 0xFF;
							outputCurrent[2]	= (srcRgb >>  8) & 0xFF;
						#endif
					}
					outputCurrent	+= m_channels;

					// We abort the run early if it overruns channel
					// NB: this could only happen due to a corrupt file
					if (0 == --runCount || outputCurrent >= outputEnd)
						state = eBetween;

					break;
				}

				// Decoded val will match channel depth
				case eRunRepeat:
				{
					// Retrieve the decoded value.
					// Extract the palettized data here so that it is not
					// repeated for each loop.
					register LcTUInt32 srcRgb = decodedVal;

					if (channelIndex != rleChannelAlpha)
						srcRgb = m_palette[decodedVal];

					// We abort the run early if it overruns channel
					// NB: this could only happen due to a corrupt file
					while (runCount-- && outputCurrent < outputEnd)
					{
						// Put into next output pos
						if (channelIndex == rleChannelAlpha)
						{
							outputCurrent[channelAlpha]	= (LcTByte)srcRgb;
						}
						else
						{
							#if !defined (IFX_USE_BIG_ENDIAN)
								outputCurrent[0] = (srcRgb >>  0) & 0xFF;
								outputCurrent[1] = (srcRgb >>  8) & 0xFF;
								outputCurrent[2] = (srcRgb >> 16) & 0xFF;
							#else
								outputCurrent[0] = (srcRgb >> 24) & 0xFF;
								outputCurrent[1] = (srcRgb >> 16) & 0xFF;
								outputCurrent[2] = (srcRgb >>  8) & 0xFF;
							#endif
						}

						outputCurrent	+= m_channels;
					}

					// Look for next run
					state = eBetween;
					break;
				}
			}

			// If we have dropped back into eBetween state, we may be at the
			// end of the current channel
			if (eBetween == state && outputCurrent >= outputEnd)
			{
				// If so, advance to next
				if (++channelIndex >= encodings)
				{
					// We've loaded file when all channels read
					success = true;
					break;
				}
				else
				{
					// Prepare to read next channel
					outputCurrent	= m_data;
				}
			}

		} // while(1)

		// Clean up the because in this mode it is used temporarily.
		if (m_rle)
		{
			m_rle.free();
		}

		if (m_palette)
		{
			m_palette.free();
		}
	}
// Keep the RLE data for drawing direct from compressed format
#if defined(IFX_RENDER_INTERNAL_COMPRESSED)
	else
	{
		// Build an index of row start positions so that the blit routines can jump
		// quickly to the correct locations in the RLE encoded image.
		// Format = R blocks of N positions, where R is # rows, and N is # channels
		m_rleIndex.alloc(m_size.height * encodings);
		if (!m_rleIndex)
			return false;

		// Cursor for scanning the RLE encoded data
		TRlePos csr;
		csr.ptr		= m_rle + 1;
		csr.lit		= ((*m_rle & 0x80) != 0);
		csr.count	= (*m_rle & 0x7F);

		// Iterate in encoded order - channel-major, row-minor
		for (int i = 0; i < encodings; i++)
		{
			for (int j = 0; j < m_size.height; j++)
			{
				// Store position in row-major, channel-minor order
				m_rleIndex[(i * m_size.height) + j] = csr;

				// Repeatedly advance by row width to find successive positions
				rleAdvance(csr, m_size.width);
			}
		}

		// Check tail marker in case of encode/decode error
		if (csr.ptr[-1] != 0x5B)
			success = false;
	}
#endif // #if defined(IFX_RENDER_INTERNAL_COMPRESSED)


	return success;

#endif // if defined(NDHS_JNI_INTERFACE)

}
#endif


/*-------------------------------------------------------------------------*//**
	RLE helper function - advance through an encoding based on TRlePos
*/
void rleAdvance(TRlePos& pos, int adv)
{
	register LcTByte* ptr	= pos.ptr;
	register int count		= pos.count;
	register LcTByte lit	= pos.lit? 0x80 : 0;

	while (adv >= count)
	{
		adv -= count;

		if (lit & 0x80)
			ptr += (1 + count);
		else
			ptr += 2;

		lit			= ptr[-1];
		count		= (lit & 0x7F);
	}

	if (adv > 0)
	{
		count -= adv;

		if (lit & 0x80)
			ptr += adv;
	}

	pos.ptr		= ptr;
	pos.count	= count;
	pos.lit		= (lit & 0x80) > 0;
}

