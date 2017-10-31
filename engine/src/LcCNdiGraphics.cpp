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


// Number of bytes per pixel
#if defined (IFX_CANVAS_MODE_8888)
	#define DEST_BYTES_PP	4
#elif defined (IFX_CANVAS_MODE_888)
	#define DEST_BYTES_PP	3
#elif defined(IFX_CANVAS_MODE_565) || defined(IFX_CANVAS_MODE_1555) || defined(IFX_CANVAS_MODE_444)
	#define DEST_BYTES_PP	2
#else
	#error No valid canvas mode is defined
#endif

/*-------------------------------------------------------------------------*//**
	Color shift and mask macros.
	These macros define how an individual color is extracted from the
	little endian source pixel data.
*/
#if !defined (IFX_USE_BIG_ENDIAN)
	#define PIXEL_GET_CH0(srcRgb)	((srcRgb >>  0)  & 0xFF)
	#define PIXEL_GET_CH1(srcRgb)	((srcRgb >>  8)  & 0xFF)
	#define PIXEL_GET_CH2(srcRgb)	((srcRgb >>  16) & 0xFF)
#else
	#define PIXEL_GET_CH0(srcRgb)	((srcRgb >>  24) & 0xFF)
	#define PIXEL_GET_CH1(srcRgb)	((srcRgb >>  16) & 0xFF)
	#define PIXEL_GET_CH2(srcRgb)	((srcRgb >>  8)  & 0xFF)
#endif

/*-------------------------------------------------------------------------*//**
	Pixel read/write helper macros.  Note that the "color" parameter should
	map to three integer variables with 0, 1 and 2 appended to the name.
	Note also that sampled values are shifted up to 8-bit values but not
	extended.  Pixel macros assume RGB channel order.  For hardware that
	uses a BGR ordering or a different pixel packing, new read/write macro
	definitions will need to be provided.
*/

#if defined(IFX_CANVAS_ORDER_RGB)
    #if defined(IFX_CANVAS_ORDER_BGR)
        #error Please only define one of IFX_CANVAS_ORDER_RGB or IFX_CANVAS_ORDER_BGR
    #endif
    
    // For 32bpp (ARGB) mode, access destination pixel directly
    #if defined(IFX_CANVAS_MODE_8888)
        #define PIXEL_READ(pixPtr, color, alpha) \
        {										\
            color##0 = (int)pixPtr[0];			\
            color##1 = (int)pixPtr[1];			\
            color##2 = (int)pixPtr[2];			\
            alpha	 = (int)pixPtr[3];			\
        }
        #define PIXEL_WRITE(pixPtr, color, alpha) \
        {										\
            pixPtr[0] = (color##0 & 0xFF);		\
            pixPtr[1] = (color##1 & 0xFF);		\
            pixPtr[2] = (color##2 & 0xFF);		\
            pixPtr[3] = (alpha & 0xFF);			\
        }

    // For 24bpp mode, access destination pixel directly
    #elif defined(IFX_CANVAS_MODE_888)
        #define PIXEL_READ(pixPtr, color)		\
        {										\
            color##0 = (int)pixPtr[0];			\
            color##1 = (int)pixPtr[1];			\
            color##2 = (int)pixPtr[2];			\
        }
        #define PIXEL_WRITE(pixPtr, color)		\
        {										\
            pixPtr[0] = (color##0 & 0xFF);		\
            pixPtr[1] = (color##1 & 0xFF);		\
            pixPtr[2] = (color##2 & 0xFF);		\
        }

    //	Pixel encoded as 16bpp (RGB=565)
    #elif defined(IFX_CANVAS_MODE_565)
        #define PIXEL_READ(pixPtr, color)		\
        {										\
            register unsigned samp = (unsigned)(*(LcTUInt16*)pixPtr); \
            color##0 = (samp >> 8) & 0x0F8;		\
            color##1 = (samp >> 3) & 0x0FC;		\
            color##2 = (samp << 3) & 0x0F8;		\
        }
        #define PIXEL_WRITE(pixPtr, color)		\
        {										\
            *(LcTUInt16*)pixPtr = (LcTUInt16)(	\
                ((color##0 & 0x00F8) << 8)		\
            |	((color##1 & 0x00FC) << 3)		\
            |	((color##2 & 0x00F8) >> 3));	\
        }

    //	Pixel encoded as 15bpp (ARGB=1555, 16bpp with top bit zeroed on write)
    #elif defined(IFX_CANVAS_MODE_1555)
        #define PIXEL_READ(pixPtr, color)		\
        {										\
            register unsigned samp = (unsigned)(*(LcTUInt16*)pixPtr); \
            color##0 = (samp >> 7) & 0x0F8;		\
            color##1 = (samp >> 2) & 0x0F8;		\
            color##2 = (samp << 3) & 0x0F8;		\
        }
        #define PIXEL_WRITE(pixPtr, color)		\
        {										\
            *(LcTUInt16*)pixPtr = (LcTUInt16)(	\
                ((color##0 & 0x00F8) << 7)		\
            |	((color##1 & 0x00F8) << 2)		\
            |	((color##2 & 0x00F8) >> 3));	\
        }

    //	Pixel encoded as 12bpp (ARGB=4444, 16bpp with top 4 bits zeroed on write)
    #elif defined (IFX_CANVAS_MODE_444)
        #define PIXEL_READ(pixPtr, color)		\
        {										\
            register unsigned samp = (unsigned)(*(LcTUInt16*)pixPtr); \
            color##0 = (samp >> 4) & 0x0F0;		\
            color##1 = (samp     ) & 0x0F0;		\
            color##2 = (samp << 4) & 0x0F0;		\
        }
        #define PIXEL_WRITE(pixPtr, color)		\
        {										\
            *(LcTUInt16*)pixPtr = (LcTUInt16)(	\
                ((color##0 & 0x00F0) << 4)		\
            |	((color##1 & 0x00F0)     )		\
            |	((color##2 & 0x00F0) >> 4));	\
        }
    #else
        #error Unsupported canvas mode
    #endif
    
#elif defined(IFX_CANVAS_ORDER_BGR)     // defined(IFX_CANVAS_ORDER_RGB)

    // For 32bpp (BGRA) mode, access destination pixel directly
    #if defined(IFX_CANVAS_MODE_8888)
        #define PIXEL_READ(pixPtr, color, alpha) \
        {										\
            color##0 = (int)pixPtr[3];			\
            color##1 = (int)pixPtr[2];			\
            color##2 = (int)pixPtr[1];			\
            alpha	 = (int)pixPtr[0];			\
        }
        #define PIXEL_WRITE(pixPtr, color, alpha) \
        {										\
            pixPtr[3] = (color##0 & 0xFF);		\
            pixPtr[2] = (color##1 & 0xFF);		\
            pixPtr[1] = (color##2 & 0xFF);		\
            pixPtr[0] = (alpha & 0xFF);			\
        }

    // For 24bpp mode, access destination pixel directly
    #elif defined(IFX_CANVAS_MODE_888)
        #define PIXEL_READ(pixPtr, color)		\
        {										\
            color##0 = (int)pixPtr[2];			\
            color##1 = (int)pixPtr[1];			\
            color##2 = (int)pixPtr[0];			\
        }
        #define PIXEL_WRITE(pixPtr, color)		\
        {										\
            pixPtr[2] = (color##0 & 0xFF);		\
            pixPtr[1] = (color##1 & 0xFF);		\
            pixPtr[0] = (color##2 & 0xFF);		\
        }

    //	Pixel encoded as 16bpp (BGR=565)
    #elif defined(IFX_CANVAS_MODE_565)
        #define PIXEL_READ(pixPtr, color)		\
        {										\
            register unsigned samp = (unsigned)(*(LcTUInt16*)pixPtr); \
            color##0 = (samp << 3) & 0x0F8;		\
            color##1 = (samp >> 3) & 0x0FC;		\
            color##2 = (samp >> 8) & 0x0F8;		\
        }
        #define PIXEL_WRITE(pixPtr, color)		\
        {										\
            *(LcTUInt16*)pixPtr = (LcTUInt16)(	\
                ((color##0 & 0x00F8) >> 3)		\
            |	((color##1 & 0x00FC) << 3)		\
            |	((color##2 & 0x00F8) << 8));	\
        }

    //	Pixel encoded as 15bpp (BGRA=5551, 16bpp with bottom bit zeroed on write)
    #elif defined(IFX_CANVAS_MODE_1555)
        #define PIXEL_READ(pixPtr, color)		\
        {										\
            register unsigned samp = (unsigned)(*(LcTUInt16*)pixPtr); \
            color##0 = (samp << 2) & 0x0F8;		\
            color##1 = (samp >> 3) & 0x0F8;		\
            color##2 = (samp >> 8) & 0x0F8;     \
        }
        #define PIXEL_WRITE(pixPtr, color)		\
        {										\
            *(LcTUInt16*)pixPtr = (LcTUInt16)(	\
                ((color##0 & 0x00F8) >> 2)		\
            |	((color##1 & 0x00F8) << 3)		\
            |	((color##2 & 0x00F8) << 8));	\
        }

    //	Pixel encoded as 12bpp (BGRA=4444, 16bpp with bottom 4 bits zeroed on write)
    #elif defined (IFX_CANVAS_MODE_444)
        #define PIXEL_READ(pixPtr, color)		\
        {										\
            register unsigned samp = (unsigned)(*(LcTUInt16*)pixPtr); \
            color##0 = (samp     ) & 0x0F0;     \
            color##1 = (samp >> 4) & 0x0F0;		\
            color##2 = (samp >> 8) & 0x0F0;		\
        }
        #define PIXEL_WRITE(pixPtr, color)		\
        {										\
            *(LcTUInt16*)pixPtr = (LcTUInt16)(	\
                ((color##0 & 0x00F0)     )		\
            |	((color##1 & 0x00F0) << 4)		\
            |	((color##2 & 0x00F0) << 8);	    \
        }
    #else
        #error Unsupported canvas mode
    #endif
    
#else   // defined(IFX_CANVAS_ORDER_RGB)
    #error No valid canvas color order is defined
#endif  // defined(IFX_CANVAS_ORDER_RGB)

/*-------------------------------------------------------------------------*//**
	Pixel blending helper macros.  Note that the "color" parameter should
	map to three unsigned variables with 0, 1 and 2 appended to the name,
	and the values in all "out" variables may be modified to the blended
	output values.  Note also that we do not attempt to be 100% color correct,
	i.e. we divide by 256 not 255 on blending.
*/

// For overlay modes, no background is drawn, and so the alpha value must also
// be blended and stored with the pixel for eventual overall blending with the
// background, possibly in hardware.  Color values in the pixel are stored
// pre-multiplied with alpha to avoid extra divisions.  The buffer must initially
// be cleared to (0,0,0,0) before compositing pixels.  When eventually drawn
// to screen, the resulting pixel should be combined with the background using
// the formula D' = D(1 - A) + S for each color channel (D = dest, S = source).
#if defined(IFX_CANVAS_MODE_8888)
	#define PIXEL_BLEND(destPixPtr, outColor, outAlpha)									\
	{																					\
		if (outAlpha < LC_ALPHA_MAX)													\
		{																				\
			register int destColor0;													\
			register int destColor1;													\
			register int destColor2;													\
			register int destAlpha;														\
																						\
			PIXEL_READ(destPixPtr, destColor, destAlpha);								\
																						\
			outColor##0 = destColor0 + (((outColor##0 - destColor0) * outAlpha) >> 8);	\
			outColor##1 = destColor1 + (((outColor##1 - destColor1) * outAlpha) >> 8);	\
			outColor##2 = destColor2 + (((outColor##2 - destColor2) * outAlpha) >> 8);	\
			outAlpha	= destAlpha  + (((0x100       - destAlpha)  * outAlpha) >> 8);	\
		}																				\
																						\
		PIXEL_WRITE(destPixPtr, outColor, outAlpha);									\
	}

// For solid color modes, the background is drawn first as solid color, so we can
// blend the pixel colors now and throw away the alpha value.
#else
	#define PIXEL_BLEND(destPixPtr, outColor, outAlpha)									\
	{																					\
		if (outAlpha < LC_ALPHA_MAX)													\
		{																				\
			register int destColor0;													\
			register int destColor1;													\
			register int destColor2;													\
																						\
			PIXEL_READ(destPixPtr, destColor);											\
																						\
			outColor##0 = destColor0 + (((outColor##0 - destColor0) * outAlpha) >> 8);	\
			outColor##1 = destColor1 + (((outColor##1 - destColor1) * outAlpha) >> 8);	\
			outColor##2 = destColor2 + (((outColor##2 - destColor2) * outAlpha) >> 8);	\
		}																				\
																						\
		PIXEL_WRITE(destPixPtr, outColor);												\
	}
#endif

/*-------------------------------------------------------------------------*//**
	RLE helper macro.  Note that this is perhaps the tightest, most
	performance-critical loop in the entire product when using compression.
	Hence note that we minimize everything inside the loop, including masking "lit"
*/
#define RLE_ADVANCE(_p, _a, _d0, _d1)					\
{														\
	LcTByte notUsed = 0;								\
	notUsed = notUsed;									\
														\
	register int adv = _a;								\
	if (adv > 0)										\
	{													\
		register LcTByte lit	= ((_p).lit? 0x80 : 0);	\
		register int count		= (_p).count;			\
		register LcTByte* ptr	= (_p).ptr;				\
														\
		while (adv >= count)							\
		{												\
			adv -= count;								\
														\
			if (lit & 0x80)								\
				ptr += (1 + count);						\
			else										\
				ptr += 2;								\
														\
			lit		= ptr[-1];							\
			count	= (lit & 0x7F);						\
		}												\
														\
		if (adv > 0)									\
		{												\
			count -= adv;								\
														\
			if (lit & 0x80)								\
			{											\
				ptr += adv;								\
				(_d0) = ptr[-1];						\
			}											\
			else										\
				(_d0) = ptr[0];							\
		}												\
		else											\
			(_d0) = ptr[-2];							\
														\
		(_p).lit	= ((lit & 0x80) != 0);				\
		(_p).count	= count;							\
		(_p).ptr	= ptr;								\
		(_d1)		= ptr[0];							\
	}													\
	else												\
	{													\
		(_d1) = (_p).ptr[0];							\
	}													\
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcCNdiGraphics> LcCNdiGraphics::create(ISurface* surf)
{
	LcTaOwner<LcCNdiGraphics> ref;
	ref.set(new LcCNdiGraphics(surf));
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
	Parameters to control a flat blit operation
*/
struct T2DBlitParams
{
	// Integer pixel edges of destination rect
	int		destPixL;
	int		destPixR;
	int		destPixT;
	int		destPixB;

	// X,Y start position in source plus interval to advance per dest pixel (<<16)
	int		srcSubpixX0;
	int		srcSubpixY0;
	int		srcScaleX;
	int		srcScaleY;

	// Clipping limits <<16
	int		srcClipR;
	int		srcClipB;
};

/*-------------------------------------------------------------------------*//**
	Populate flat blit parameters from the given data
*/
bool LcCNdiGraphics::calc2DBlitParams(
	T2DBlitParams&				params,
	const LcTPixelDim&			srcSize,
	const LcTScalarRect&		destRectI,
	const LcTPixelRect&			destClipI,
	bool						bLinear)
{
	// Copy param to modify
	LcTScalarRect	destRect(destRectI);
	LcTPixelRect	destClip(destClipI);

	// Check for parameter errors
	if (!m_surface
	||	destRect.getWidth()  <= 0
	||	destRect.getHeight() <= 0)
	{
		return false;
	}

	// We assume that bounds returned by surface map exactly to
	// the data and span also returned by surface (i.e. no clipping)
	LcTPixelRect canvas		= m_surface->getBounds();

	// For drawing, coords are required relative to canvas not screen
	destRect.setLeft(	destRect.getLeft() - canvas.getLeft());
	destRect.setTop(	destRect.getTop() - canvas.getTop());
	destRect.setRight(	destRect.getRight() - canvas.getLeft());
	destRect.setBottom(	destRect.getBottom() - canvas.getTop());
	destClip.setLeft(	destClip.getLeft() - canvas.getLeft());
	destClip.setTop(	destClip.getTop() - canvas.getTop());
	destClip.setRight(	destClip.getRight() - canvas.getLeft());
	destClip.setBottom(	destClip.getBottom() - canvas.getTop());

	// Work out clipped destination positions - note we use intermediate
	// variables to avoid issues with some compilers and templates
	LcTScalar tempf1 		= LcTScalar(destClip.getLeft());
	LcTScalar tempf2 		= LcTScalar(destClip.getTop());
	LcTScalar tempf3 		= LcTScalar(destClip.getRight());
	LcTScalar tempf4 		= LcTScalar(destClip.getBottom());
	LcTScalar destSubpixL	= max(tempf1, destRect.getLeft());
	LcTScalar destSubpixT	= max(tempf2, destRect.getTop());
	LcTScalar destSubpixR	= min(tempf3, destRect.getRight());
	LcTScalar destSubpixB	= min(tempf4, destRect.getBottom());

	// Scan lines must be integral (NB: casting truncates)
	params.destPixL			= int(destSubpixL);
	params.destPixT			= int(destSubpixT);
	params.destPixR			= int(destSubpixR);
	params.destPixB			= int(destSubpixB);

	// Apply edge weightings by simply not drawing any edge with weighting < 0.5.
	// Note that for properly smoothed edges, bitmaps should include a border
	// of zero-alpha pixels around their perimeter
	if (destSubpixR - float(params.destPixR) <= 0.5)
		params.destPixR--;
	if (destSubpixB - float(params.destPixB) <= 0.5)
		params.destPixB--;
	if (destSubpixL - float(params.destPixL) > 0.5)
		params.destPixL++;
	if (destSubpixT - float(params.destPixT) > 0.5)
		params.destPixT++;

	// Check if rect is totally clipped away
	if (params.destPixL > params.destPixR || params.destPixT > params.destPixB)
		return false;

	// Sample coords are based on integral position of destination pixels
	// after clipping has been applied (could end up negative!)
	LcTScalar destOffsetX	= 0.501f + params.destPixL - destRect.getLeft();
	LcTScalar destOffsetY	= 0.501f + params.destPixT - destRect.getTop();

	// Scale-up factors from dest to src
	LcTScalar srcScaleXf	= srcSize.width / destRect.getWidth();
	LcTScalar srcScaleYf	= srcSize.height / destRect.getHeight();

	// Map the sample point into source pixmap space
	LcTScalar srcSubpixX0f	= destOffsetX * srcScaleXf;
	LcTScalar srcSubpixY0f	= destOffsetY * srcScaleYf;

	// Sampling is done differently in linear mode
	if (bLinear)
	{
		srcSubpixX0f -= 0.5;
		srcSubpixY0f -= 0.5;
	}

	// Calc fixed-point versions for use in loop
	// NB: srcSubpixX0 & srcSubpixY may be -ve but int() truncation is OK because of multiplier
	params.srcSubpixX0		= int(srcSubpixX0f * 0x10000); // may be -ve
	params.srcSubpixY0		= int(srcSubpixY0f * 0x10000); // may be -ve
	params.srcScaleX		= int(srcScaleXf   * 0x10000);
	params.srcScaleY		= int(srcScaleYf   * 0x10000);

	// Calc clipping limits
	params.srcClipR			= (srcSize.width - 1) << 16;
	params.srcClipB			= (srcSize.height - 1) << 16;

	// Success
	return true;
}

#if defined(IFX_RENDER_INTERNAL)
/*-------------------------------------------------------------------------*//**
	Parameters to store info about scanned edges
*/
typedef int FP_COORD;
typedef int FP_FRACT;

struct TEdgeParams
{
	// Destination X coord
	FP_COORD				destX;

	// Source X,Y coords at this dest pixel, encoded as reciprocals
	FP_FRACT				src1oZ;
	FP_FRACT				srcUoZ;
	FP_FRACT				srcVoZ;

	// Fixed-point positions when using integers for the above.  We choose the
	// maximum precision that allows an integer part of suitable range within a
	// signed 32-bit integer.  dEye/Z is in range (0.5, 2) so maximum 1oZ is 2.  
	// Thus for UoZ the max is 2x maximum source dimension; set that at 512
	// and max UoZ is 1024 or 10 bits, allowing 20 bit precision with one bit 
	// safety.
	#define FP_1oZ_BITS		28
	#define FP_1oZ_ONE		(1 << FP_1oZ_BITS)
	#define FP_UoZ_BITS		20
	#define FP_UoZ_ONE		(1 << FP_UoZ_BITS)
	#define FP_X_BITS		20
	#define FP_X_ONE		(1 << FP_X_BITS)
};
#endif //defined(IFX_RENDER_INTERNAL)

/*-------------------------------------------------------------------------*//**
	Fixed-point utilities to speed up traversal of reciprocal values
*/
	// Take maximum screen width as 512, thus potential accumulated error after
	// iteration is 256 (8 bits).  Thus the traversed reciprocals will have
	// only 20-8 = 12 bits precision.  So we reduce to 12 bits before manipulation.
	// This reduces the number of Newton-Raphson iterations required.
	#define FP_REC_BITS			12
	#define FP_REC_TWO			(2 << FP_REC_BITS)
	#define FP_REC_K1			((int)(1.77 * (1 << FP_REC_BITS)))
	#define FP_REC_K2			((int)(0.72 * (1 << FP_REC_BITS)))
	#define FP_REC_MULT(a, b)	(((a) * (b)) >> FP_REC_BITS)

	// Reciprocal function optimized for range (0.5, 2) and 12-point precision
	#define FP_RECIPROCAL(_in, _out)									\
	{																	\
		register int in		= (_in);									\
		register int out	= FP_REC_K1 - FP_REC_MULT(in, FP_REC_K2);	\
		out	= FP_REC_MULT(out, (FP_REC_TWO - FP_REC_MULT(out, in)));	\
		out	= FP_REC_MULT(out, (FP_REC_TWO - FP_REC_MULT(out, in)));	\
		out	= FP_REC_MULT(out, (FP_REC_TWO - FP_REC_MULT(out, in)));	\
		(_out) = out;													\
	}

#if defined(IFX_RENDER_INTERNAL)
/*-------------------------------------------------------------------------*//**
 Parameters to control a perspective blit operation
*/
class T3DBlitParams
{
public:

	// Which dest pixel rows to scan
	int						destStartIndex;
	int						destEndIndex;
	int						destPixY;

	// Dest clipping limits
	int						destClipL;
	int						destClipR;

	// Source clipping limits <<8
	int						srcClipR;
	int						srcClipB;

	// Edge arrays
	TEdgeParams*			edgeUp;
	TEdgeParams*			edgeDown;
};
#endif //defined(IFX_RENDER_INTERNAL)

#if defined(IFX_RENDER_INTERNAL)
/*-------------------------------------------------------------------------*//**
	Populate 3D blit parameters from the given data
*/
bool LcCNdiGraphics::calc3DBlitParams(
	T3DBlitParams*				params,
	const LcTPixelDim&			srcSize,
	const LcTScalarQuad&		destQuad,
	const LcTPixelRect&			destClip,
	bool						bLinear)
{
	LC_UNUSED(bLinear)
	
	// Get source corners
	LcTVector srcTL(0, 0, 0);
	LcTVector srcTR((float)srcSize.width, 0, 0);
	LcTVector srcBR((float)srcSize.width, (float)srcSize.height, 0);
	LcTVector srcBL(0, (float)srcSize.height, 0);

	// Get destination corners
	LcTVector destTL = destQuad.getTopLeft();
	LcTVector destTR = destQuad.getTopRight();
	LcTVector destBR = destQuad.getBottomRight();
	LcTVector destBL = destQuad.getBottomLeft();

	// Get the maximum/minimum destination X coordinate
	int maxX = 0;
	int minX = 99999;
	minX = min(minX, int(destTL.x));
	maxX = max(maxX, int(destTL.x));
	minX = min(minX, int(destTR.x));
	maxX = max(maxX, int(destTR.x));
	minX = min(minX, int(destBL.x));
	maxX = max(maxX, int(destBL.x));
	minX = min(minX, int(destBR.x));
	maxX = max(maxX, int(destBR.x));

	// These are used later to ensure that edge adjustments are not allowed
	// to draw pixels outside the original bounding box
	params->destClipL = max(minX, destClip.getLeft());
	params->destClipR = min(maxX, destClip.getRight());

	// Get the maximum/minimum destination Y coordinate
	int maxY = 0;
	int minY = 99999;
	minY = min(minY, int(destTL.y));
	maxY = max(maxY, int(destTL.y));
	minY = min(minY, int(destTR.y));
	maxY = max(maxY, int(destTR.y));
	minY = min(minY, int(destBL.y));
	maxY = max(maxY, int(destBL.y));
	minY = min(minY, int(destBR.y));
	maxY = max(maxY, int(destBR.y));

	// Range in dest Y - this is how many rasters we need to scan
	params->destStartIndex	= 0;
	params->destEndIndex	= 1 + maxY - minY;

	// Save before clipping
	int arraySize			= params->destEndIndex;

	// Check for top clipping
	if (minY < destClip.getTop())
		params->destStartIndex = destClip.getTop() - minY;

	// Check for bottom clipping
	if (maxY >= destClip.getBottom())
		params->destEndIndex = destClip.getBottom() - minY;

	// Abort if we are vertically clipped away
	if (params->destEndIndex < params->destStartIndex)
		return false;

	// Calc Y of top row within clipped dest region
	params->destPixY		= minY + params->destStartIndex;

	// Allocate memory for edge records
	params->edgeUp = LcTmAlloc<TEdgeParams>::allocUnsafe(arraySize);
	if (!params->edgeUp)
		return false;

	params->edgeDown = LcTmAlloc<TEdgeParams>::allocUnsafe(arraySize);
	if (!params->edgeDown)
	{
		LcTmAlloc<TEdgeParams>::freeUnsafe(params->edgeUp);
		return false;
	}

	TEdgeParams* edgeUp		= (TEdgeParams*)params->edgeUp;
	TEdgeParams* edgeDown	= (TEdgeParams*)params->edgeDown;

	// If the face is inverted then ensure the down edges are
	// on the right.
	// We check this by finding the direction of z for the
	// normal of the top and left edge.
	if (((destTL.x - destTR.x) * (destTL.y - destBL.y))
		- ((destTL.x - destBL.x) * (destTL.y - destTR.y)) < 0)
	{
		edgeUp		= (TEdgeParams*)params->edgeDown;
		edgeDown	= (TEdgeParams*)params->edgeUp;
	}

	// Clear edges
	memset(edgeUp,   0, arraySize * sizeof(TEdgeParams));
	memset(edgeDown, 0, arraySize * sizeof(TEdgeParams));

	// Scan edges clockwise.  Down edges are always on the right,
	// including if the graphic is flipped to show the reverse side
	scanEdge(edgeUp, edgeDown, minY, destTL, destTR, srcTL, srcTR);
	scanEdge(edgeUp, edgeDown, minY, destTR, destBR, srcTR, srcBR);
	scanEdge(edgeUp, edgeDown, minY, destBR, destBL, srcBR, srcBL);
	scanEdge(edgeUp, edgeDown, minY, destBL, destTL, srcBL, srcTL);

	// Calc clipping limits
	params->srcClipR		= (srcSize.width - 1) << 8;
	params->srcClipB		= (srcSize.height - 1) << 8;

	// Success
	return true;
}
#endif //defined(IFX_RENDER_INTERNAL)

#if defined(IFX_RENDER_INTERNAL)
/*-------------------------------------------------------------------------*//**
	Local utility function to cleanup any operations performed in
	calc3DBlitParams
*/
void LcCNdiGraphics::cleanup3DBlitParams(
	T3DBlitParams*		params)
{
	// Free the memory for the edge arrays.
	if (params->edgeUp)
		LcTmAlloc<TEdgeParams>::freeUnsafe(params->edgeUp);
	if (params->edgeDown)
		LcTmAlloc<TEdgeParams>::freeUnsafe(params->edgeDown);
}
#endif //defined(IFX_RENDER_INTERNAL)

#if defined(IFX_RENDER_INTERNAL)
/*-------------------------------------------------------------------------*//**
	Local utility function to scan one edge of a quad and interpolate/cache
	the left/right end positions of dest/source scan lines
*/
void LcCNdiGraphics::scanEdge(
	TEdgeParams*		edgeUp,
	TEdgeParams*		edgeDown,
	int					minY,
	const LcTVector&	destA,
	const LcTVector&	destB,
	const LcTVector&	srcA, // u,v
	const LcTVector&	srcB)
{
	TEdgeParams*	edge;
	int				destdY;

	// Calc top and bottom Y, and height
	int		destAY		= int(destA.y);
	int		destBY		= int(destB.y);
	int		destH		= destBY - destAY;
	float	destHf		= destB.y - destA.y;

	// Determine edge vertical direction
	if (destH > 0)
	{
		edge	= edgeDown;
		destdY	= 1;
	}
	else if (destH < 0)
	{
		edge	= edgeUp;
		destdY	= -1;

		// Flip height if negative
		destH	= -destH;
		destHf	= -destHf;
	}
	else
	{
		// Ignore edge if horizontal
		return;
	}

	// Calculate X values and Z derivatives for the A end of this edge.
	// Canvas projection has already given us dEye/z in each Z value.
	// Note that canvas Z is negative so we invert to avoid signed maths
	FP_COORD	destAX		= (FP_COORD)(FP_X_ONE	* destA.x);
	FP_FRACT	srcA1oZ		= (FP_FRACT)(FP_1oZ_ONE * -destA.z);
	FP_FRACT	srcAUoZ		= (FP_FRACT)(FP_UoZ_ONE * (-destA.z * srcA.x));
	FP_FRACT	srcAVoZ		= (FP_FRACT)(FP_UoZ_ONE * (-destA.z * srcA.y));

	// Calculate X values and Z derivatives for the B end of this edge
	FP_COORD	destBX		= (FP_COORD)(FP_X_ONE	* destB.x);
	FP_FRACT	srcB1oZ		= (FP_FRACT)(FP_1oZ_ONE * -destB.z);
	FP_FRACT	srcBUoZ		= (FP_FRACT)(FP_UoZ_ONE * (-destB.z * srcB.x));
	FP_FRACT	srcBVoZ		= (FP_FRACT)(FP_UoZ_ONE * (-destB.z * srcB.y));

	// Calculate increment value per pixel in Y direction, for each
	float		dest1Hf		= 1.0f / destHf;
	FP_COORD	destdX		= (FP_COORD)((destBX - destAX)   * dest1Hf);
	FP_FRACT	srcd1oZ		= (FP_FRACT)((srcB1oZ - srcA1oZ) * dest1Hf);
	FP_FRACT	srcdUoZ		= (FP_FRACT)((srcBUoZ - srcAUoZ) * dest1Hf);
	FP_FRACT	srcdVoZ		= (FP_FRACT)((srcBVoZ - srcAVoZ) * dest1Hf);

	// Apply an offset to each to compensate for sub-pixel offset in Y direction
	// NB: We effectively add 0.5 because we are mapping a dest coord to a sample coord
	float		destYoffset = (destA.y - destAY - 0.5f) * destdY;
	destAX					-= (FP_COORD)(destdX  * destYoffset);
	srcA1oZ					-= (FP_FRACT)(srcd1oZ * destYoffset);
	srcAUoZ					-= (FP_FRACT)(srcdUoZ * destYoffset);
	srcAVoZ					-= (FP_FRACT)(srcdVoZ * destYoffset);

	// Iterate Y values and interpolate X values into the arrays
	for (; destAY != destBY;
		destAY		+= destdY,
		destAX		+= destdX,
		srcA1oZ		+= srcd1oZ,
		srcAUoZ		+= srcdUoZ,
		srcAVoZ		+= srcdVoZ)
	{
		TEdgeParams* rowStart = edge + destAY - minY;

		// Populate the edge record for each row
		rowStart->destX		= destAX;
		rowStart->src1oZ	= srcA1oZ;
		rowStart->srcUoZ	= srcAUoZ;
		rowStart->srcVoZ	= srcAVoZ;
	}
}
#endif //defined(IFX_RENDER_INTERNAL)

/*-------------------------------------------------------------------------*//**
	Parameters to control margin drawing
*/
struct TMarginParams
{
	// X margins - can be applied independently of Y; negative scale disables
	int		left;
	int		rightSize;
	int		rightExtent;
	int		scaleX;

	// Y margins - can be applied independently of X; negative scale disables
	int		top;
	int		bottomSize;
	int		bottomExtent;
	int		scaleY;
};

/*-------------------------------------------------------------------------*//**
*/
bool LcCNdiGraphics::calcMarginParams(
	TMarginParams&				params,
	LcTPixelDim&				srcEffectiveSize,
	TMargins*					srcMargins)
{
	// Set up margin mapping if one is provided
	if (!srcMargins)
		return false;

	// Calc margin points within extent (not original size)
	params.rightExtent		= srcMargins->extentWidth - srcMargins->right;
	params.bottomExtent		= srcMargins->extentHeight - srcMargins->bottom;

	// Calc margin points within original size (not extent)
	params.rightSize		= srcEffectiveSize.width - srcMargins->right;
	params.bottomSize		= srcEffectiveSize.height - srcMargins->bottom;

	// Calc scaling factors to apply to stretched region (<< 16)
	// If the margins are less then the width/height do not draw them.
	params.scaleX = -1;
	params.scaleY = -1;
	if ((srcMargins->left + srcMargins->right) < srcMargins->extentWidth)
	{
		params.scaleX = (int)(((params.rightSize - srcMargins->left) << 16)
							/ (params.rightExtent - srcMargins->left));
	}
	if ((srcMargins->top + srcMargins->bottom) < srcMargins->extentHeight)
	{
		params.scaleY = (int)(((params.bottomSize - srcMargins->top) << 16)
							/ (params.bottomExtent - srcMargins->top));
	}

	// Change effective size only where scale is positive
	// NB: if scale is negative it indicates not to use margin in this direction
	if (params.scaleX > 0)
		srcEffectiveSize.width	= srcMargins->extentWidth;
	if (params.scaleY > 0)
		srcEffectiveSize.height = srcMargins->extentHeight;

	// Calc margin points within source (fixed point, << 16)
	params.left				= srcMargins->left	<< 16;
	params.top				= srcMargins->top	<< 16;

	// Adjust margin params for using fixed point (<< 16)
	params.rightSize		<<= 16;
	params.bottomSize		<<= 16;
	params.rightExtent		<<= 16;
	params.bottomExtent		<<= 16;

	return true;
}

/***************************************************************************//**
	NDI BITMAP BLITTING FUNCTIONS

	blitNdiBitmap() delegates to one of two blit functions:-
		blitRawBitmap()
		blitRLE2DFull()
*/

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCNdiGraphics::blitNdiBitmap(
	LcCNdiBitmapFile*			srcNdi,
	int							srcFrame,
	const LcTPixelDim&			srcExtent,
	const LcTScalarQuad&		destQuad,
	const LcTPixelRect&			destClip,
	LcTColor					color,
	LcTScalar					opacity,
	bool						bLinear)
{
	// Parameter check
	if (!m_surface
	||	!srcNdi
	||	srcFrame > srcNdi->getFrameCount())
		return;

	// Get size and frame count
	LcTPixelDim srcSize	= srcNdi->getSize();
	int frameCount		= srcNdi->getFrameCount();
	int skipRows		= 0;
	
	// Adjust height for frame count
	if (frameCount > 1)
	{
		srcSize.height /= frameCount; 
		
		// Check frame number
		if (srcFrame < frameCount)
			skipRows = srcFrame * srcSize.height;
	}	

	// Margins struct to pass to raw blit functions
	TMargins srcMargins;
	bool bUseMargins = false;

	// Only observe margins if a reference extent has been provided
	// that is greater than the base image size in at least one direction
	if (srcExtent.width > srcSize.width
	||	srcExtent.height > srcSize.height)
	{
		// Extract margins from NDI
		srcMargins.left			= srcNdi->getMarginLeft();
		srcMargins.top			= srcNdi->getMarginTop();
		srcMargins.right		= srcNdi->getMarginRight();
		srcMargins.bottom		= srcNdi->getMarginBottom();

		// Save extent to pass through
		srcMargins.extentWidth	= srcExtent.width;
		srcMargins.extentHeight	= srcExtent.height;

		// We only apply margins if at least one of them is set
		bUseMargins = (srcMargins.left || srcMargins.right
					|| srcMargins.top || srcMargins.bottom);
	}

	#if defined(IFX_RENDER_INTERNAL_COMPRESSED)

		// Get both RLE indices (note that we must use the fresh NDI size
		// because we must account for all frames in a multi-frame bitmap)
		TRlePos* srcRleAlphaIndex;
		TRlePos* srcRleColorIndex;

		if (srcNdi->getFormat() == LcCNdiBitmapFile::KFormatGraphicTranslucent)
		{
			srcRleAlphaIndex = srcNdi->getRleIndex() + skipRows;
			srcRleColorIndex = srcRleAlphaIndex + srcNdi->getSize().height;
		}
		else
		{
			srcRleAlphaIndex = NULL;
			srcRleColorIndex = srcNdi->getRleIndex() + skipRows;
		}

		// In RLE mode, do nothing if we can't get flattened rect
		LcTScalarRect destRect;
		if (destQuad.convertToRect(destRect))
		{
			// Check item is within the valid clipping planes - the valid
			// range is (0.5, -2.0)
			if((destRect.getZDepth() > 0.5) || (destRect.getZDepth() < -2.0))
				return;

			// Frame number is offset into row array
			blitRLE2DFull(
				srcRleColorIndex,
				srcRleAlphaIndex,
				srcNdi->getPalette(),
				srcSize,
				bUseMargins ? &srcMargins : NULL,
				destRect,
				destClip,
				color,
				opacity,
				bLinear);
		}
	#else

		// Frame number is offset into pixel data
		int srcRowSpan = srcSize.width
			* (srcNdi->getFormat() == LcCNdiBitmapFile::KFormatGraphicTranslucent? 4 : 3);

		// Delegate to raw bitmap blit
		blitRawBitmap(
			srcNdi->getData() + (skipRows * srcRowSpan),
			srcSize,
			srcRowSpan,
			srcNdi->getFormat(),
			bUseMargins? &srcMargins : NULL,
			destQuad,
			destClip,
			color,
			opacity,
			bLinear);
	#endif

} //LcCNdiGraphics::blitNdiBitmap


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCNdiGraphics::blitCustomBitmap(
	LcCCustomBitmapFile*		srcBitmap,
	int							srcFrame,
	const LcTPixelDim&			srcExtent,
	const LcTScalarQuad&		destQuad,
	const LcTPixelRect&			destClip,
	LcTColor					color,
	LcTScalar					opacity,
	bool						bLinear)
{
#if !defined(IFX_RENDER_INTERNAL_COMPRESSED)
	// Parameter check
	if (!m_surface
	||	!srcBitmap
	||	srcFrame > srcBitmap->getFrameCount())
		return;

	// Get size and frame count
	LcTPixelDim srcSize	= srcBitmap->getSize();
	int frameCount		= srcBitmap->getFrameCount();
	int skipRows		= 0;

	// Adjust height for frame count
	if (frameCount > 1)
	{
		srcSize.height /= frameCount;

		// Check frame number
		if (srcFrame < frameCount)
			skipRows = srcFrame * srcSize.height;
	}

	// Margins struct to pass to raw blit functions
	TMargins srcMargins;
	bool bUseMargins = false;

	// Only observe margins if a reference extent has been provided
	// that is greater than the base image size in at least one direction
	if (srcExtent.width > srcSize.width
	||	srcExtent.height > srcSize.height)
	{
		// Extract margins from NDI
		srcMargins.left			= srcBitmap->getMarginLeft();
		srcMargins.top			= srcBitmap->getMarginTop();
		srcMargins.right		= srcBitmap->getMarginRight();
		srcMargins.bottom		= srcBitmap->getMarginBottom();

		// Save extent to pass through
		srcMargins.extentWidth	= srcExtent.width;
		srcMargins.extentHeight	= srcExtent.height;

		// We only apply margins if at least one of them is set
		bUseMargins = (srcMargins.left || srcMargins.right
					|| srcMargins.top || srcMargins.bottom);
	}
	// Frame number is offset into pixel data
	int srcRowSpan = srcSize.width
		* (srcBitmap->getFormat() == LcCCustomBitmapFile::KFormatGraphicTranslucent? 4 : 3);

	// Delegate to raw bitmap blit
	blitRawBitmap(
		srcBitmap->getData() + (skipRows * srcRowSpan),
		srcSize,
		srcRowSpan,
		(LcCNdiBitmapFile::EFormat)srcBitmap->getFormat(),
		bUseMargins? &srcMargins : NULL,
		destQuad,
		destClip,
		color,
		opacity,
		bLinear);
#else
	LC_UNUSED(srcBitmap)
	LC_UNUSED(srcExtent)
	LC_UNUSED(destQuad)
	LC_UNUSED(destClip)
	LC_UNUSED(color)
	LC_UNUSED(opacity)
	LC_UNUSED(bLinear)	
#endif

} //LcCNdiGraphics::blitCustomBitmap

#if defined(IFX_RENDER_INTERNAL_COMPRESSED)
/*-------------------------------------------------------------------------*//**
*/
void LcCNdiGraphics::blitRLE2DFull(
	TRlePos*					srcRleColorIndex,
	TRlePos*					srcRleAlphaIndex,
	LcTUInt32*					srcPalette,
	const LcTPixelDim&			srcSizeI,
	LcCNdiGraphics::TMargins*	srcMargins,
	const LcTScalarRect&		destRectI,
	const LcTPixelRect&			destClipI,
	LcTColor					color,
	LcTScalar					opacity,
	bool						bLinear)
{
	// Might modify effective size if we are using a margin mapping
	LcTPixelDim srcEffectiveSize(srcSizeI);

	// Calculate margin coords, if margins are specified
	TMarginParams margins;
	bool bUseMargins = srcMargins && calcMarginParams(
		margins,
		srcEffectiveSize,
		srcMargins);

	// Set up blit data
	T2DBlitParams params;
	if (!calc2DBlitParams(
		params,
		srcEffectiveSize,
		destRectI,
		destClipI,
		bLinear))
	{
		return;
	}


	// Used for advancing RLE to initial position before saving
	int srcPixX0			= max(params.srcSubpixX0, 0) >> 16;
	int srcSubpixY			= params.srcSubpixY0;

	// Modulation values
	int modColor0			= color.rgba.r;
	int modColor1			= color.rgba.g;
	int modColor2			= color.rgba.b;

	int modOpacity			= int(opacity * 0xFF);
	bool modWhite			= (modColor0 == 0xFF && modColor1 == 0xFF && modColor2 == 0xFF);

	// Carried over row pointers (RLE mode)
	TRlePos rleRowAT;
	TRlePos rleRowAB;
	TRlePos rleRowCT;
	TRlePos rleRowCB;
	int rleRowPos = -2;

	// Count of bytes between start of one row and start of next
	unsigned destRowSpan	= m_surface->getLineSpan();

	// Get a temporary margin X for the Y X Advancing.
	// Clamp X to clipped range
	unsigned tempXClamped = params.srcSubpixX0;
	if (params.srcSubpixX0 < 0)
		tempXClamped = 0;
	if (params.srcSubpixX0 >= params.srcClipR)
		tempXClamped = params.srcClipR;
	if (bUseMargins && margins.scaleX > 0)
	{
		if (tempXClamped > (unsigned)margins.rightExtent)
			tempXClamped -= (margins.rightExtent - margins.rightSize);
		else if (tempXClamped > (unsigned)margins.left)
			tempXClamped = margins.left + ((margins.scaleX * ((tempXClamped - margins.left) >> 8)) >> 8);
	}
	int srcPixXActual = (tempXClamped >> 16);

	// Addresses of first byte of first row
	LcTByte* destRowPtr		= m_surface->getDataStart()
								+ params.destPixT * destRowSpan
								+ params.destPixL * DEST_BYTES_PP;

	// ******* Vertical scan (row by row) *******

	// Iterate destination rows, advancing source Y by scale factor
	// Note that scan is inclusive of last row; first and last rows are weighted
	for (int i = params.destPixT; i <= params.destPixB;
			i++,
			srcSubpixY += params.srcScaleY,
			destRowPtr += destRowSpan)
	{
		bool bBottomEdge = (srcSubpixY >= params.srcClipB);

		// Clamp Y to clipped range
		unsigned clamped = srcSubpixY;
		if (srcSubpixY < 0)
			clamped	= 0;
		if (bBottomEdge)
			clamped	= params.srcClipB;

		// If we have a margin mapping, apply now
		if (bUseMargins && margins.scaleY > 0)
		{
			if (clamped > (unsigned)margins.bottomExtent)
				clamped -= (margins.bottomExtent - margins.bottomSize);
			else if (clamped > (unsigned)margins.top)
				clamped = margins.top + ((margins.scaleY * ((clamped - margins.top) >> 8)) >> 8);
		}

		// Round Y to get integral source pixel coord
		int srcPixY = (clamped >> 16);

		// Calculate row weightings for linear sampling
		int srcWeightY = 0;
		if (bLinear)
			srcWeightY = (clamped >> 8) & 0xFF;

		// Row-specific data, RLE or linear modes
		int rleCPos				= 0;
		int rleAPos				= 0;

		// RLE cursors for compressed mode
		TRlePos rleRowC0;
		TRlePos	rleRowC1;
		TRlePos	rleRowA0;
		TRlePos	rleRowA1;

		// Carried-over color values for linear sampling
		int srcColorL0			= 0;
		int srcColorL1			= 0;
		int srcColorL2			= 0;
		int srcColorR0			= 0;
		int srcColorR1			= 0;
		int srcColorR2			= 0;

		// Carried-over alpha values
		int	srcAlphaL			= 0;

		// Calc how many rows down to move.  Note that advancing to the correct
		// start position for a clipped draw might be expensive, so we re-use
		// previous cursors wherever possible
		int rleYOffset	= srcPixY - rleRowPos;
		rleRowPos		= srcPixY;

		// Advance alpha sample cursors
		if (srcRleAlphaIndex)
		{
			// Note that we will always have an offset of at least +2 at top row
			if (rleYOffset > 0)
			{
				// Re-use the last bottom cursor if we've only advanced one row
				if (bLinear && rleYOffset == 1)
					rleRowAT = rleRowAB;
				else
				{
					// Otherwise fetch new cursor and scan forwards
					rleRowAT = srcRleAlphaIndex[srcPixY];
					RLE_ADVANCE(rleRowAT, srcPixXActual, notUsed, notUsed);
				}

				// If we're not linear sampling, the top row is enough
				if (bLinear)
				{
					// Re-use the last bottom cursor if we've only advanced one row
					if (bBottomEdge)
						rleRowAB = rleRowAT;
					else
					{
						// Otherwise fetch new cursor and scan forwards
						rleRowAB = srcRleAlphaIndex[srcPixY + 1];
						RLE_ADVANCE(rleRowAB, srcPixXActual, notUsed, notUsed);
					}
				}
			}

			// Set cursors to saved start positions
			rleRowA0	= rleRowAT;
			rleRowA1	= rleRowAB;

			// Record current position - will advance this to track pixel scan
			rleAPos		= srcPixXActual;
		}

		// Advance color sample cursors
		// Note that we will always have an offset of at least +2 at top row
		if (rleYOffset > 0)
		{
			// Re-use the last bottom cursor if we've only advanced one row
			if (bLinear && rleYOffset == 1)
				rleRowCT = rleRowCB;
			else
			{
				// Otherwise fetch new cursor and scan forwards
				rleRowCT = srcRleColorIndex[srcPixY];
				RLE_ADVANCE(rleRowCT, srcPixXActual, notUsed, notUsed);
			}

			// If we're not linear sampling, the top row is enough
			if (bLinear)
			{
				// Re-use the last bottom cursor if we've only advanced one row
				if (bBottomEdge)
					rleRowCB = rleRowCT;
				else
				{
					// Otherwise fetch new cursor and scan forwards
					rleRowCB = srcRleColorIndex[srcPixY + 1];
					RLE_ADVANCE(rleRowCB, srcPixXActual, notUsed, notUsed);
				}
			}
		}

		// Set cursors to saved start positions
		rleRowC0	= rleRowCT;
		rleRowC1	= rleRowCB;

		// Record current position - will advance this to track pixel scan
		rleCPos		= srcPixXActual;

		// Initialize for destination row scan
		int srcSubpixX			= params.srcSubpixX0;
		LcTByte* destPixPtr		= destRowPtr;

		// ******* Horizontal scan (pixel by pixel) *******

		// Note: below this point, we are in a tight loop and execution efficiency
		// is paramount.  Register variables are suggested wherever appropriate.

		// Iterate destination pixels, advancing source X by scale factor
		// Note that scan is inclusive of last col; first and last cols are weighted
		for (int j = params.destPixL; j <= params.destPixR;
				j++,
				srcSubpixX += params.srcScaleX,
				destPixPtr += DEST_BYTES_PP)
		{
			bool bRightEdge = (srcSubpixX >= params.srcClipR);

			// Clamp X to clipped range
			clamped = srcSubpixX;
			if (srcSubpixX < 0)
				clamped	= 0;
			if (bRightEdge)
				clamped	= params.srcClipR;

			// If we have a margin mapping, apply now
			if (bUseMargins && margins.scaleX > 0)
			{
				if (clamped > (unsigned)margins.rightExtent)
					clamped -= (margins.rightExtent - margins.rightSize);
				else if (clamped > (unsigned)margins.left)
					clamped = margins.left + ((margins.scaleX * ((clamped - margins.left) >> 8)) >> 8);
			}

			// Round X to get integral source pixel coord
			int srcPixX = (clamped >> 16);

			// Calculate weightings for linear sampling
			int srcWeightX = 0;
			if (bLinear)
				srcWeightX = (clamped >> 8) & 0xFF;

			// ******* Alpha sampling *******

			int outAlpha = 0;

			// No alpha per pixel in background mode
			if (!srcRleAlphaIndex)
			{
				outAlpha = 0xFF;
			}
			else if (!bLinear)
			{
				// Nearest sample - advance through the encoding by a calculated offset
				int rleAOffset	= srcPixX - rleAPos;
				rleAPos			= srcPixX;

				// Advance RLE cursors through encoded data
				// NB: if offset is 0, will still read the value
				RLE_ADVANCE(rleRowA0, rleAOffset, notUsed, outAlpha);
			}
			else // linear sampling, imageType == KFormatFont or KFormatGraphicTranslucent
			{
				// All 4 samples available, do bilinear interpolation
				// Advance through the encoding by a calculated offset
				// NB: it is safe to read 1 pixel past the end when at right edge
				int rleAOffset	= (srcPixX + 1) - rleAPos;
				rleAPos			= (srcPixX + 1);

				// Performance note:  it's not worth re-using the right hand column
				// when rleAOffset == 0 because the additional logic and memory access
				// is more expensive than just re-calculating it

				register int samp00 = 0;
				register int samp01 = 0;
				register int samp10;
				register int samp11;

				// Advance RLE cursors through encoded data
				RLE_ADVANCE(rleRowA0, rleAOffset, samp00, samp10);
				RLE_ADVANCE(rleRowA1, rleAOffset, samp01, samp11);

				// If the offset is 0, RLE_ADVANCE does not set the left column samples
				// so we re-use the interpolated value from last time
				if (rleAOffset > 0)
					srcAlphaL = samp00 + (((samp01 - samp00) * srcWeightY) >> 8);

				// Bilerp right column UNLESS we are at the right edge
				if (bRightEdge)
					outAlpha = srcAlphaL;
				else
				{
					samp10 += (((samp11 - samp10) * srcWeightY) >> 8);
					outAlpha = srcAlphaL + (((samp10 - srcAlphaL) * srcWeightX) >> 8);
				}

			} // alpha modes

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// Apply global opacity factor
			if (modOpacity < 0xFF)
				outAlpha = (outAlpha * modOpacity) >> 8;

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// ******* Color sampling *******

			int outColor0 = 0;
			int outColor1 = 0;
			int outColor2 = 0;

			if (!bLinear)
			{
				// Sample nearest - advance through the encoding by a calculated offset
				int rleCOffset	= srcPixX - rleCPos;
				rleCPos			= srcPixX;

				// Performance note:  it doesn't seem worth saving the previous
				// decoded color values, given that we have only one sample and no
				// interpolation.  Rough calcs indicate a 70% hit per sampled pixel
				// to achieve a 30% improvement per savable pixel.  Not worth it
				// except where scaling up by a factor of >4 (which may be the case
				// for some large panels)

				LcTByte sample;
				RLE_ADVANCE(rleRowC0, rleCOffset, notUsed, sample);

				register LcTUInt32 srcRgb = srcPalette[sample];

				outColor0 = PIXEL_GET_CH0(srcRgb);
				outColor1 = PIXEL_GET_CH1(srcRgb);
				outColor2 = PIXEL_GET_CH2(srcRgb);
			}
			else // linear sampling, channels == 3 or 4
			{
				// In RLE mode, we advance color separately to alpha, since alpha
				// may act as a mask and result in colors sometimes being ignored
				// Advance through the encoding by a calculated offset
				// Specify position of lookahead pixel
				int rleCOffset	= (srcPixX + 1) - rleCPos;
				rleCPos			= (srcPixX + 1);

				// Note that on the left edge, the initial advance is always at least +1,
				// so we can never bypass this advance on the first pixel
				if (rleCOffset > 0)
				{
					register LcTByte samp00 = 0;
					register LcTByte samp01 = 0;
					register LcTByte samp10;
					register LcTByte samp11;

					// Advance RLE cursors through encoded data
					RLE_ADVANCE(rleRowC0, rleCOffset, samp00, samp10);
					RLE_ADVANCE(rleRowC1, rleCOffset, samp01, samp11);

					// Performance note:  color extraction including palette lookup
					// and columnar interpolation is relatively expensive, so we save
					// these values and re-use them on subsequent columns.  We have to
					// save the left column anyway as we can't get the samples back later.
					// Rough calcs suggest a minimal performance improvement saving the
					// right column as well

					// Note that we can't re-use the right column unless we have
					// previously done an RLE_ADVANCE, as the values won't be set
					if (rleCOffset == 1 && rleCPos > (srcPixX0 + 1))
					{
						// For offset +1, left column is last time's right column
						srcColorL0 = srcColorR0;
						srcColorL1 = srcColorR1;
						srcColorL2 = srcColorR2;
					}
					else // rleCOffset > 1
					{
						register int samp0;
						register int samp1;

						register LcTUInt32 srcRgb0;
						register LcTUInt32 srcRgb1;

						// Get left column samples and interpolate between rows
						srcRgb0 = srcPalette[samp00];
						srcRgb1 = srcPalette[samp01];

						// Channel 0
						samp0 = PIXEL_GET_CH0(srcRgb0);
						samp1 = PIXEL_GET_CH0(srcRgb1);

						srcColorL0 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

						// Channel 1
						samp0 = PIXEL_GET_CH1(srcRgb0);
						samp1 = PIXEL_GET_CH1(srcRgb1);

						srcColorL1 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

						// Channel 2
						samp0 = PIXEL_GET_CH2(srcRgb0);
						samp1 = PIXEL_GET_CH2(srcRgb1);

						srcColorL2 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);
					}

					// At right edge, no need to interpolate over right column
					if (!bRightEdge)
					{
						register int samp0;
						register int samp1;

						register LcTUInt32 srcRgb0;
						register LcTUInt32 srcRgb1;

						// Get right column samples and interpolate between rows
						srcRgb0 = srcPalette[samp10];
						srcRgb1 = srcPalette[samp11];

						// Channel 0
						samp0 = PIXEL_GET_CH0(srcRgb0);
						samp1 = PIXEL_GET_CH0(srcRgb1);

						srcColorR0 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

						// Channel 1
						samp0 = PIXEL_GET_CH1(srcRgb0);
						samp1 = PIXEL_GET_CH1(srcRgb1);

						srcColorR1 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

						// Channel 2
						samp0 = PIXEL_GET_CH2(srcRgb0);
						samp1 = PIXEL_GET_CH2(srcRgb1);

						srcColorR2 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);
					}
				}

				if (!bRightEdge)
				{
					// Apply interpolation along row
					// NB: we don't save X-interpolated values as the weighting always changes
					outColor0 = srcColorL0 + (((srcColorR0 - srcColorL0) * srcWeightX) >> 8);
					outColor1 = srcColorL1 + (((srcColorR1 - srcColorL1) * srcWeightX) >> 8);
					outColor2 = srcColorL2 + (((srcColorR2 - srcColorL2) * srcWeightX) >> 8);
				}
				else
				{
					outColor0 = srcColorL0;
					outColor1 = srcColorL1;
					outColor2 = srcColorL2;
				}
			} // linear

			// If we have a tint color, use it to modulate the pixel sample
			if (!modWhite)
			{
				outColor0 = (outColor0 * modColor0) >> 8;
				outColor1 = (outColor1 * modColor1) >> 8;
				outColor2 = (outColor2 * modColor2) >> 8;
			}

			// ******* Pixel output *******

			// Macro expands to pixel-format-specific blending code
			PIXEL_BLEND(destPixPtr, outColor, outAlpha);

		} // for j
	} // for i


} //LcCNdiGraphics::blitRLE2DFull
#endif //defined(IFX_RENDER_INTERNAL_COMPRESSED)


/***************************************************************************//**
	RAW BITMAP BLITTING FUNCTIONS

	blitRawBitmap() delegates to one of four internal blit functions:-
		blitBitmap2DFull()
		blitBitmap3DFull()
		blitBitmap2DFast()
		blitBitmap3DFast()
*/

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCNdiGraphics::blitRawBitmap(
	LcTByte*					srcPixData,
	const LcTPixelDim&			srcSize,
	unsigned					srcRowSpan,
	LcCNdiBitmapFile::EFormat	srcFormat,
	LcCNdiGraphics::TMargins*	srcMargins,
	const LcTScalarQuad&		destQuad,
	const LcTPixelRect&			destClip,
	LcTColor					color,
	LcTScalar					opacity,
	bool						bLinear)
{
	// Check state and parameters
	if (!m_surface
	||	!srcPixData
	||	srcSize.width		<= 0
	||	srcSize.height		<= 0)
	{
		return;
	}

	// Check to see if dest quad is in X,Y plane
	LcTScalarRect destRect;
	if (destQuad.convertToRect(destRect))
	{
		if (destRect.getZDepth())
		{
			// Check item is within the valid clipping planes.  We're looking
			// for dEye/Z in the valid range (0.5, 2.0).  Note that the only
			// image being drawn at Zdepth = 0 should be the background!
			LcTScalar oneOverZ = 1.0f / (-destRect.getZDepth());
			if((oneOverZ <= 0.5f) || (oneOverZ >= 2.0f))
				return;
		}

		// Check params to see if we can use fastest routine
		if (!bLinear
			&&	!srcMargins
			&&	color == LcTColor::WHITE
			&&	srcFormat == LcCNdiBitmapFile::KFormatGraphicTranslucent)
		{
			// Call fast 2D blit
			blitBitmap2DFast(
				srcPixData,
				srcSize,
				srcRowSpan,
				destRect,
				destClip,
				opacity);
		}
		else
		{
			// Call full-featured 2D blit
			blitBitmap2DFull(
				srcPixData,
				srcSize,
				srcRowSpan,
				srcFormat,
				srcMargins,
				destRect,
				destClip,
				color,
				opacity,
				bLinear);
		}
	}
#if defined(IFX_RENDER_INTERNAL)
	else
	{
		// If not, delegate to perspective blit instead
		// Check params to see if we can use fastest routine
		if (!bLinear
			&&	!srcMargins
			&&	color == LcTColor::WHITE
			&&	srcFormat == LcCNdiBitmapFile::KFormatGraphicTranslucent)
		{
			// Call fast 3D blit
			blitBitmap3DFast(
				srcPixData,
				srcSize,
				srcRowSpan,
				destQuad,
				destClip,
				opacity);
		}
		else
		{
			// Call full-featured 3D blit
			blitBitmap3DFull(
				srcPixData,
				srcSize,
				srcRowSpan,
				srcFormat,
				srcMargins,
				destQuad,
				destClip,
				color,
				opacity,
				bLinear);
		}
	}
#endif //defined(IFX_RENDER_INTERNAL)

} //LcCNdiGraphics::blitRawBitmap

/*-------------------------------------------------------------------------*//**
*/
void LcCNdiGraphics::blitBitmap2DFull(
	LcTByte*					srcData,
	const LcTPixelDim&			srcSizeI,
	unsigned					srcRowSpan,
	LcCNdiBitmapFile::EFormat	srcFormat,
	LcCNdiGraphics::TMargins*	srcMargins,
	const LcTScalarRect&		destRect,
	const LcTPixelRect&			destClip,
	LcTColor					color,
	LcTScalar					opacity,
	bool						bLinear)
{
	// Might modify effective size if we are using a margin mapping
	LcTPixelDim srcEffectiveSize(srcSizeI);

	// Calculate margin coords, if margins are specified
	TMarginParams margins;
	bool bUseMargins = srcMargins && calcMarginParams(
		margins,
		srcEffectiveSize,
		srcMargins);

	// Set up blit data
	T2DBlitParams params;
	if (!calc2DBlitParams(
		params,
		srcEffectiveSize,
		destRect,
		destClip,
		bLinear))
	{
		return;
	}


	// Channel data
	bool bHaveAlpha			= (srcFormat != LcCNdiBitmapFile::KFormatGraphicOpaque);
	bool bHaveColor			= (srcFormat != LcCNdiBitmapFile::KFormatFont);
	int srcAlphaK			= (bHaveColor? 3 : 0);
	int srcBytesPP			= (bHaveColor? 3 : 0) + (bHaveAlpha? 1 : 0);

	// Y cursor
	int srcSubpixY			= params.srcSubpixY0;

	// Modulation parameters
	int modColor0			= color.rgba.r;
	int modColor1			= color.rgba.g;
	int modColor2			= color.rgba.b;

	int modOpacity			= int(opacity * 0xFF);
	bool modWhite			= (modColor0 == 0xFF && modColor1 == 0xFF && modColor2 == 0xFF);

	// Count of bytes between start of one row and start of next
	unsigned destRowSpan	= m_surface->getLineSpan();

	// Addresses of first byte of first row
	LcTByte* destRowPtr		= m_surface->getDataStart()
								+ params.destPixT * destRowSpan
								+ params.destPixL * DEST_BYTES_PP;

	// ******* Vertical scan (row by row) *******

	// Iterate destination rows, advancing source Y by scale factor
	// Note that scan is inclusive of last row; first and last rows are weighted
	for (int i = params.destPixT; i <= params.destPixB;
			i++,
			srcSubpixY += params.srcScaleY,
			destRowPtr += destRowSpan)
	{
		unsigned 	offsetRowSpan	= srcRowSpan;
		int 		offsetBytesPP	= srcBytesPP;

		// Clamp Y to clipped range
		unsigned clamped = srcSubpixY;
		if (srcSubpixY < 0)
			clamped	= 0;
		
		// Check to see if we are on the bottom row of the source image.
		if (srcSubpixY >= params.srcClipB)
		{
			clamped			= params.srcClipB;
			offsetRowSpan	= 0;
		}

		// If we have a margin mapping, apply now
		if (bUseMargins && margins.scaleY > 0)
		{
			if (clamped > (unsigned)margins.bottomExtent)
				clamped -= (margins.bottomExtent - margins.bottomSize);
			else if (clamped > (unsigned)margins.top)
				clamped = margins.top + ((margins.scaleY * ((clamped - margins.top) >> 8)) >> 8);
		}

		// Round Y to get integral source pixel coord
		int srcPixY = (clamped >> 16);

		// Calculate row weightings for linear sampling
		int srcWeightY = 0;
		if (bLinear)
			srcWeightY = (clamped >> 8) & 0xFF;

		// Initialize for destination row scan
		LcTByte* destPixPtr		= destRowPtr;
		LcTByte* srcRowPtr		= srcData + (srcPixY * srcRowSpan);
		int srcSubpixX			= params.srcSubpixX0;

		// ******* Horizontal scan (pixel by pixel) *******

		// Iterate destination pixels, advancing source X by scale factor
		// Note that scan is inclusive of last col; first and last cols are weighted
		for (int j = params.destPixL; j <= params.destPixR;
				j++,
				srcSubpixX += params.srcScaleX,
				destPixPtr += DEST_BYTES_PP)
		{
			// Clamp X to clipped range
			clamped = srcSubpixX;
			if (srcSubpixX < 0)
				clamped	= 0;
			
			// Check to see if we are at the right edge of the source image 
			if (srcSubpixX >= params.srcClipR)
			{
				clamped			= params.srcClipR;
				offsetBytesPP	= 0;
			}

			// If we have a margin mapping, apply now
			if (bUseMargins && margins.scaleX > 0)
			{
				if (clamped > (unsigned)margins.rightExtent)
					clamped -= (margins.rightExtent - margins.rightSize);
				else if (clamped > (unsigned)margins.left)
					clamped = margins.left + ((margins.scaleX * ((clamped - margins.left) >> 8)) >> 8);
			}

			// Round X to get integral source pixel coord
			int srcPixX = (clamped >> 16);

			// Calculate weightings for linear sampling
			int srcWeightX = 0;
			if (bLinear)
				srcWeightX = (clamped >> 8) & 0xFF;

			// Uncompressed mode: calculate absolute source offset each time
			LcTByte* srcPixelPtr = srcRowPtr + (srcPixX * srcBytesPP);
			LcTByte* srcAlphaPtr = &srcPixelPtr[srcAlphaK];

			// ******* Alpha sampling *******

			int outAlpha = 0;

			// No alpha per pixel in opaque mode
			if (srcFormat == LcCNdiBitmapFile::KFormatGraphicOpaque)
			{
				outAlpha = 0xFF;
			}
			else if (!bLinear)
			{
				// Nearest sample
				outAlpha = srcAlphaPtr[0];
			}
			else // linear sampling, imageType == KFormatFont or KFormatGraphicTranslucent
			{
				// All 4 samples available, do bilinear interpolation
				register int samp0;
				register int samp1;

				// Sample left column and interpolate
				samp0 = srcAlphaPtr[0];
				samp1 = srcAlphaPtr[offsetRowSpan];

				outAlpha = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

				// Sample right column and interpolate column and row
				samp0 = srcAlphaPtr[offsetBytesPP];
				samp1 = srcAlphaPtr[offsetRowSpan + offsetBytesPP];

				samp0 += (((samp1 - samp0) * srcWeightY) >> 8);
				outAlpha += (((samp0 - outAlpha) * srcWeightX) >> 8);

			} // alpha modes

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// Apply per-row opacity factor
			if (modOpacity < 0xFF)
				outAlpha = (outAlpha * modOpacity) >> 8;

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// ******* Color sampling *******

			int outColor0 = 0;
			int outColor1 = 0;
			int outColor2 = 0;

			if (srcFormat == LcCNdiBitmapFile::KFormatFont)
			{
				// No color per pixel - modulate alpha with input color
				outColor0 = modColor0;
				outColor1 = modColor1;
				outColor2 = modColor2;
			}
			else
			{
				if (!bLinear)
				{
					// Sample nearest
					outColor0 = (int)srcPixelPtr[0];
					outColor1 = (int)srcPixelPtr[1];
					outColor2 = (int)srcPixelPtr[2];
				}
				else // linear sampling, channels == 3 or 4
				{
					// Sample and interpolate left edge
					register int samp0;
					register int samp1;

					// Channel 0 - sample left column and interpolate
					samp0 = srcPixelPtr[0];
					samp1 = srcPixelPtr[offsetRowSpan];

					outColor0 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

					// Sample right column and interpolate column and row
					samp0 = srcPixelPtr[offsetBytesPP];
					samp1 = srcPixelPtr[offsetRowSpan + offsetBytesPP];

					samp0 += (((samp1 - samp0) * srcWeightY) >> 8);
					outColor0 += (((samp0 - outColor0) * srcWeightX) >> 8);

					// Channel 1 - sample left column and interpolate
					samp0 = srcPixelPtr[1];
					samp1 = srcPixelPtr[1 + offsetRowSpan];

					outColor1 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

					samp0 = srcPixelPtr[1 + offsetBytesPP];
					samp1 = srcPixelPtr[1 + offsetRowSpan + offsetBytesPP];

					samp0 += (((samp1 - samp0) * srcWeightY) >> 8);
					outColor1 += (((samp0 - outColor1) * srcWeightX) >> 8);

					// Channel 2 - sample left column and interpolate
					samp0 = srcPixelPtr[2];
					samp1 = srcPixelPtr[2 + offsetRowSpan];

					outColor2 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

					samp0 = srcPixelPtr[2 + offsetBytesPP];
					samp1 = srcPixelPtr[2 + offsetRowSpan + offsetBytesPP];

					samp0 += (((samp1 - samp0) * srcWeightY) >> 8);
					outColor2 += (((samp0 - outColor2) * srcWeightX) >> 8);

				} // linear

				// If we have a tint color, use it to modulate the pixel sample
				if (!modWhite)
				{
					outColor0 = (outColor0 * modColor0) >> 8;
					outColor1 = (outColor1 * modColor1) >> 8;
					outColor2 = (outColor2 * modColor2) >> 8;
				}

			} // color modes

			// ******* Pixel output *******

			// Macro expands to pixel-format-specific blending code
			PIXEL_BLEND(destPixPtr, outColor, outAlpha);

		} // for j
	} // for i


} //LcCNdiGraphics::blitBitmap2DFull

/*-------------------------------------------------------------------------*//**
*/
void LcCNdiGraphics::blitBitmap2DFast(
	LcTByte*					srcData,
	const LcTPixelDim&			srcSizeI,
	unsigned					srcRowSpan,
	const LcTScalarRect&		destRect,
	const LcTPixelRect&			destClip,
	LcTScalar					opacity)
{
	// Might modify effective size if we are using a margin mapping
	LcTPixelDim srcEffectiveSize(srcSizeI);

	// Set up blit data
	T2DBlitParams params;
	if (!calc2DBlitParams(
		params,
		srcEffectiveSize,
		destRect,
		destClip,
		false))
	{
		return;
	}


	// Adjust R/B clip to outside rect, not inside
	params.srcClipB += (1 << 16);
	params.srcClipR += (1 << 16);

	// Trim top coord to remove any need for clamping
	while (params.srcSubpixY0 < 0)
	{
		params.destPixT++;
		params.srcSubpixY0 += params.srcScaleY;
	}

	// Trim bottom coord to remove any need for clamping
	while (params.srcSubpixY0 + (params.destPixB - params.destPixT) * params.srcScaleY
		> params.srcClipB)
	{
		params.destPixB--;
	}

	// Trim left coord to remove any need for clamping
	while (params.srcSubpixX0 < 0)
	{
		params.destPixL++;
		params.srcSubpixX0 += params.srcScaleX;
	}

	// Trim right coord to remove any need for clamping
	while (params.srcSubpixX0 + (params.destPixR - params.destPixL) * params.srcScaleX
		> params.srcClipR)
	{
		params.destPixR--;
	}

	// Y cursor
	int srcSubpixY			= params.srcSubpixY0;

	// Modulation parameters
	int modOpacity			= int(opacity * 0xFF);

	// Count of bytes between start of one row and start of next
	unsigned destRowSpan	= m_surface->getLineSpan();

	// Addresses of first byte of first row
	LcTByte* destRowPtr		= m_surface->getDataStart()
								+ params.destPixT * destRowSpan
								+ params.destPixL * DEST_BYTES_PP;

	// ******* Vertical scan (row by row) *******

	// Iterate destination rows, advancing source Y by scale factor
	// Note that scan is inclusive of last row; first and last rows are weighted
	for (int i = params.destPixT; i <= params.destPixB;
			i++,
			srcSubpixY += params.srcScaleY,
			destRowPtr += destRowSpan)
	{
		// Round Y to get integral source pixel coord
		int srcPixY = (srcSubpixY >> 16);

		// Initialize for destination row scan
		LcTByte* destPixPtr		= destRowPtr;
		LcTByte* srcRowPtr		= srcData + (srcPixY * srcRowSpan);
		int srcSubpixX			= params.srcSubpixX0;

		// ******* Horizontal scan (pixel by pixel) *******

		// Iterate destination pixels, advancing source X by scale factor
		// Note that scan is inclusive of last col; first and last cols are weighted
		for (int j = params.destPixL; j <= params.destPixR;
				j++,
				srcSubpixX += params.srcScaleX,
				destPixPtr += DEST_BYTES_PP)
		{
			// Round X to get integral source pixel coord
			int srcPixX = (srcSubpixX >> 16);

			// Uncompressed mode: calculate absolute source offset each time
			LcTByte* srcPixelPtr = srcRowPtr + (srcPixX * 4);

			// ******* Alpha sampling *******

			register int outAlpha = (srcPixelPtr[3] * modOpacity) >> 8;

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// ******* Color sampling *******

			register int outColor0 = (int)srcPixelPtr[0];
			register int outColor1 = (int)srcPixelPtr[1];
			register int outColor2 = (int)srcPixelPtr[2];

			// ******* Pixel output *******

			// Macro expands to pixel-format-specific blending code
			PIXEL_BLEND(destPixPtr, outColor, outAlpha);

		} // for j
	} // for i


} //LcCNdiGraphics::blitBitmap2DFast

#if defined(IFX_RENDER_INTERNAL)
/*-------------------------------------------------------------------------*//**
*/
void LcCNdiGraphics::blitBitmap3DFull(
	LcTByte*					srcData,
	const LcTPixelDim&			srcSizeI,
	unsigned					srcRowSpan,
	LcCNdiBitmapFile::EFormat	srcFormat,
	LcCNdiGraphics::TMargins*	srcMargins,
	const LcTScalarQuad&		destQuad,
	const LcTPixelRect&			destClip,
	LcTColor					color,
	LcTScalar					opacity,
	bool						bLinear)
{
	// Might modify effective size if we are using a margin mapping
	LcTPixelDim srcEffectiveSize(srcSizeI);

	// Calculate margin coords, if margins are specified
	TMarginParams margins;
	bool bUseMargins = srcMargins && calcMarginParams(
		margins,
		srcEffectiveSize,
		srcMargins);

	if (bUseMargins)
	{
		// Adjust fixed point values, as 3D routine uses <<8 not <<16
		margins.left			>>= 8;
		margins.top				>>= 8;
		margins.rightSize		>>= 8;
		margins.bottomSize		>>= 8;
		margins.rightExtent		>>= 8;
		margins.bottomExtent	>>= 8;
	}

	// Set up perspective data - edge end points, etc
	T3DBlitParams params;
	
	// This allocates memory that is not trapped in the cleanup code. 
	// Ensure that if calc3DBlitParams succeeds, then the remainder of this 
	// routine has only one return point, and can not throw an exception.
	if (!calc3DBlitParams(
		&params,
		srcEffectiveSize,
		destQuad,
		destClip,
		bLinear))
	{
		return;
	}


	// Channel details
	bool bHaveAlpha			= (srcFormat != LcCNdiBitmapFile::KFormatGraphicOpaque);
	bool bHaveColor			= (srcFormat != LcCNdiBitmapFile::KFormatFont);
	int srcAlphaK			= (bHaveColor? 3 : 0);
	int srcBytesPP			= (bHaveColor? 3 : 0) + (bHaveAlpha? 1 : 0);

	// Modulation values
	int modColor0			= color.rgba.r;
	int modColor1			= color.rgba.g;
	int modColor2			= color.rgba.b;

	int modOpacity			= int(opacity * 0xFF);
	bool modWhite			= (modColor0 == 0xFF && modColor1 == 0xFF && modColor2 == 0xFF);

	// Addresses of first byte of first row
	int destPixY			= params.destPixY;
	unsigned destRowSpan	= m_surface->getLineSpan();
	LcTByte* destRowPtr		= m_surface->getDataStart() + destPixY * destRowSpan;

	// ******* Vertical scan (row by row) *******

	for (int i = params.destStartIndex; i < params.destEndIndex;
			i++,
			destPixY++,
			destRowPtr += destRowSpan)
	{
		// Check for infinity condition
		if (!params.edgeUp[i].src1oZ)
			continue;

		// Get X coords of row start/end
		FP_COORD destUpX	= params.edgeUp[i].destX;
		int destPixL		= destUpX >> FP_X_BITS;
		int destPixR		= params.edgeDown[i].destX >> FP_X_BITS;

		// Apply horizontal clipping to the destination edge coords
		destPixL			= max(destPixL, params.destClipL);
		destPixR			= min(destPixR, params.destClipR);
		int destPixW		= destPixR - destPixL;

		// Skip the row if it is fully clipped (later rows might not be, so continue)
		if (destPixW < 0)
			continue;

		// Left hand edge values
		FP_FRACT src1oZ		= params.edgeUp[i].src1oZ;
		FP_FRACT srcUoZ		= params.edgeUp[i].srcUoZ;
		FP_FRACT srcVoZ		= params.edgeUp[i].srcVoZ;

		// Work out the linear increment for each parameter pix dest pixel moved
		float dest1Wf		= float(FP_X_ONE) / (params.edgeDown[i].destX  - destUpX);
		FP_FRACT srcd1oZ	= (FP_FRACT)((params.edgeDown[i].src1oZ - src1oZ) * dest1Wf);
		FP_FRACT srcdUoZ	= (FP_FRACT)((params.edgeDown[i].srcUoZ - srcUoZ) * dest1Wf);
		FP_FRACT srcdVoZ	= (FP_FRACT)((params.edgeDown[i].srcVoZ - srcVoZ) * dest1Wf);

		// Apply offset to start values to compensate for dest pixel clipping AND rounding
		// NB: Without this we will get a "postage stamp" effect
		// NB2: We add 0.5 when mapping a dest coord to a sample coord
		float destXOffset	= destPixL - (float(destUpX) / FP_X_ONE) + 0.5;
		src1oZ				+= (FP_FRACT)(destXOffset * srcd1oZ);
		srcUoZ				+= (FP_FRACT)(destXOffset * srcdUoZ);
		srcVoZ				+= (FP_FRACT)(destXOffset * srcdVoZ);

		// Ready to scan the row
		LcTByte* destPixPtr	= destRowPtr + (destPixL * DEST_BYTES_PP);

		// ******* Horizontal scan (pixel by pixel) *******

		// Iterate destination pixels, advancing source X/Y via reciprocals
		for (int j = 0; j < destPixW;
					j++,
					src1oZ += srcd1oZ,
					srcUoZ += srcdUoZ,
					srcVoZ += srcdVoZ,
					destPixPtr += DEST_BYTES_PP)
		{
			// Skip pixel if dEye/Z is outside the range (0.5, 2)
			if (src1oZ <= (FP_1oZ_ONE >> 1) || src1oZ >= (FP_1oZ_ONE << 1))
				continue;
			
			unsigned 	offsetRowSpan = srcRowSpan;
			int 		offsetBytesPP = srcBytesPP;
			
			// Fixed-point reciprocal is much faster than using integer division (on ARM7/9)
			int	srcZ;
			FP_RECIPROCAL((src1oZ >> (FP_1oZ_BITS - FP_REC_BITS)), srcZ);

			// Derive source X,Y from interpolated reciprocals by multiplying in Z
			int srcSubpixX		= FP_REC_MULT(srcZ, (srcUoZ >> (FP_UoZ_BITS - 8)));
			int srcSubpixY		= FP_REC_MULT(srcZ, (srcVoZ >> (FP_UoZ_BITS - 8)));

			// In linear mode we subtract 0.5 after mapping a dest coord to a sample coord
			if (bLinear)
			{
				// Note we are <<8 so this effectively subtracts 0.5
				srcSubpixX -= (0x80);
				srcSubpixY -= (0x80);
			}

			// Clamp X to clipped range
			unsigned clampedX = srcSubpixX;
			if (srcSubpixX < 0)
				clampedX = 0;
			
			// Check whether we are at the right edge of source bitmap
			if (srcSubpixX >= params.srcClipR)
			{
				clampedX		= params.srcClipR;
				offsetBytesPP 	= 0;
			}

			// Clamp Y to clipped range
			unsigned clampedY = srcSubpixY;
			if (srcSubpixY < 0)
				clampedY = 0;
			
			// Check whether we are at the bottom edge of source bitmap
			if (srcSubpixY >= params.srcClipB)
			{
				clampedY		= params.srcClipB;
				offsetRowSpan 	= 0;
			}

			// If we have a margin mapping, apply now
			if (bUseMargins)
			{
				// If scale is negative, ignore Y margins
				if (margins.scaleY > 0)
				{
					if (clampedY > (unsigned)margins.bottomExtent)
						clampedY -= (margins.bottomExtent - margins.bottomSize);
					else if (clampedY > (unsigned)margins.top)
						clampedY = margins.top + ((margins.scaleY * (clampedY - margins.top)) >> 16);
				}

				// If scale is negative, ignore X margins
				if (margins.scaleX > 0)
				{
					if (clampedX > (unsigned)margins.rightExtent)
						clampedX -= (margins.rightExtent - margins.rightSize);
					else if (clampedX > (unsigned)margins.left)
						clampedX = margins.left + ((margins.scaleX * (clampedX - margins.left)) >> 16);
				}
			}

			// Round to get integral source pixel coords
			int srcPixX = (clampedX >> 8);
			int srcPixY = (clampedY >> 8);

			// Calculate weightings for linear sampling
			int srcWeightX = 0;
			int srcWeightY = 0;
			if (bLinear)
			{
				srcWeightX = (clampedX & 0xFF);
				srcWeightY = (clampedY & 0xFF);
			}

			// Calculate absolute source offset each time (Y may change)
			LcTByte* srcPixelPtr = srcData +
				(srcPixY * srcRowSpan) +
				(srcPixX * srcBytesPP);
			
			LcTByte* srcAlphaPtr = &srcPixelPtr[srcAlphaK];

			// ******* Alpha sampling *******

			register int outAlpha = 0;

			// No alpha per pixel in opaque mode
			if (srcFormat == LcCNdiBitmapFile::KFormatGraphicOpaque)
			{
				outAlpha = 0xFF;
			}
			else if (!bLinear)
			{
				// Nearest sample
				outAlpha = srcAlphaPtr[0];
			}
			else // linear sampling, imageType == KFormatFont or KFormatGraphicTranslucent
			{
				register int samp0;
				register int samp1;

				// Sample left column and interpolate
				samp0 = srcAlphaPtr[0];
				samp1 = srcAlphaPtr[offsetRowSpan];

				outAlpha = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

				// Sample right column and interpolate column and row
				samp0 = srcAlphaPtr[offsetBytesPP];
				samp1 = srcAlphaPtr[offsetRowSpan + offsetBytesPP];

				samp0 += (((samp1 - samp0) * srcWeightY) >> 8);
				outAlpha += (((samp0 - outAlpha) * srcWeightX) >> 8);
			} // alpha modes

			// Apply global opacity factor
			if (modOpacity < 0xFF)
				outAlpha = (outAlpha * modOpacity) >> 8;

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// ******* Color sampling *******

			int outColor0 = 0;
			int outColor1 = 0;
			int outColor2 = 0;

			if (srcFormat == LcCNdiBitmapFile::KFormatFont)
			{
				// No color per pixel - modulate alpha with input color
				outColor0 = modColor0;
				outColor1 = modColor1;
				outColor2 = modColor2;
			}
			else
			{
				if (!bLinear)
				{
					// Sample nearest
					outColor0 = (int)srcPixelPtr[0];
					outColor1 = (int)srcPixelPtr[1];
					outColor2 = (int)srcPixelPtr[2];
				}
				else // linear sampling, channels == 3 or 4
				{
					// Sample and interpolate left edge
					register int samp0;
					register int samp1;

					// Channel 0 - sample left column and interpolate
					samp0 = srcPixelPtr[0];
					samp1 = srcPixelPtr[offsetRowSpan];

					outColor0 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

					// Sample right column and interpolate column and row
					samp0 = srcPixelPtr[offsetBytesPP];
					samp1 = srcPixelPtr[offsetRowSpan + offsetBytesPP];

					samp0 += (((samp1 - samp0) * srcWeightY) >> 8);
					outColor0 += (((samp0 - outColor0) * srcWeightX) >> 8);

					// Channel 1 - sample left column and interpolate
					samp0 = srcPixelPtr[1];
					samp1 = srcPixelPtr[1 + offsetRowSpan];

					outColor1 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

					// Sample right column and interpolate column and row
					samp0 = srcPixelPtr[1 + offsetBytesPP];
					samp1 = srcPixelPtr[1 + offsetRowSpan + offsetBytesPP];

					samp0 += (((samp1 - samp0) * srcWeightY) >> 8);
					outColor1 += (((samp0 - outColor1) * srcWeightX) >> 8);

					// Channel 2 - sample left column and interpolate
					samp0 = srcPixelPtr[2];
					samp1 = srcPixelPtr[2 + offsetRowSpan];

					outColor2 = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

					// Sample right column and interpolate column and row
					samp0 = srcPixelPtr[2 + offsetBytesPP];
					samp1 = srcPixelPtr[2 + offsetRowSpan + offsetBytesPP];

					samp0 += (((samp1 - samp0) * srcWeightY) >> 8);
					outColor2 += (((samp0 - outColor2) * srcWeightX) >> 8);

				} // linear

				// If we have a tint color, use it to modulate the pixel sample
				if (!modWhite)
				{
					outColor0 = (outColor0 * modColor0) >> 8;
					outColor1 = (outColor1 * modColor1) >> 8;
					outColor2 = (outColor2 * modColor2) >> 8;
				}
			} // color modes

			// ******* Pixel output *******

			// Macro expands to pixel-format-specific blending code
			PIXEL_BLEND(destPixPtr, outColor, outAlpha);

		} // for j
	} // for i

	// Clean up
	cleanup3DBlitParams(&params);


} //LcCNdiGraphics::blitBitmap3DFull
#endif //defined(IFX_RENDER_INTERNAL)

#if defined(IFX_RENDER_INTERNAL)
/*-------------------------------------------------------------------------*//**
*/
void LcCNdiGraphics::blitBitmap3DFast(
	LcTByte*					srcData,
	const LcTPixelDim&			srcSizeI,
	unsigned					srcRowSpan,
	const LcTScalarQuad&		destQuad,
	const LcTPixelRect&			destClip,
	LcTScalar					opacity)
{
	// Might modify effective size if we are using a margin mapping
	LcTPixelDim srcEffectiveSize(srcSizeI);

	// Set up perspective data - edge end points, etc
	T3DBlitParams params;

	// This allocates memory that is not trapped in the cleanup code. 
	// Ensure that if calc3DBlitParams succeeds, then the remainder of this 
	// routine has only one return point, and can not throw an exception.
	if (!calc3DBlitParams(
		&params,
		srcEffectiveSize,
		destQuad,
		destClip,
		false))
	{
		return;
	}


	// Modulation values
	int modOpacity			= int(opacity * 0xFF);

	// Addresses of first byte of first row
	int destPixY			= params.destPixY;
	unsigned destRowSpan	= m_surface->getLineSpan();
	LcTByte* destRowPtr		= m_surface->getDataStart() + destPixY * destRowSpan;

	// ******* Vertical scan (row by row) *******

	for (int i = params.destStartIndex; i < params.destEndIndex;
			i++,
			destPixY++,
			destRowPtr += destRowSpan)
	{
		// Check for infinity condition
		if (!params.edgeUp[i].src1oZ)
			continue;

		// Get X coords of row start/end
		FP_COORD destUpX	= params.edgeUp[i].destX;
		int destPixL		= destUpX >> FP_X_BITS;
		int destPixR		= params.edgeDown[i].destX >> FP_X_BITS;

		// Apply horizontal clipping to the destination edge coords
		destPixL			= max(destPixL, params.destClipL);
		destPixR			= min(destPixR, params.destClipR);
		int destPixW		= destPixR - destPixL;

		// Skip the row if it is fully clipped (later rows might not be, so continue)
		// Note: flipped/inverted images are reversed by scanEdges.
		if (destPixW <= 0)
			continue;

		// Left hand edge values
		FP_FRACT src1oZ		= params.edgeUp[i].src1oZ;
		FP_FRACT srcUoZ		= params.edgeUp[i].srcUoZ;
		FP_FRACT srcVoZ		= params.edgeUp[i].srcVoZ;

		// Work out the linear increment for each parameter pix dest pixel moved
		float dest1Wf		= float(FP_X_ONE) / (params.edgeDown[i].destX  - destUpX);
		FP_FRACT srcd1oZ	= (FP_FRACT)((params.edgeDown[i].src1oZ - src1oZ) * dest1Wf);
		FP_FRACT srcdUoZ	= (FP_FRACT)((params.edgeDown[i].srcUoZ - srcUoZ) * dest1Wf);
		FP_FRACT srcdVoZ	= (FP_FRACT)((params.edgeDown[i].srcVoZ - srcVoZ) * dest1Wf);

		// Apply offset to start values to compensate for dest pixel clipping AND rounding
		// NB: Without this we will get a "postage stamp" effect
		// NB2: We add 0.5 when mapping a dest coord to a sample coord
		float destXOffset	= destPixL - (float(destUpX) / FP_X_ONE) + 0.5;
		src1oZ				+= (FP_FRACT)(destXOffset * srcd1oZ);
		srcUoZ				+= (FP_FRACT)(destXOffset * srcdUoZ);
		srcVoZ				+= (FP_FRACT)(destXOffset * srcdVoZ);

		// Check for pixels at the start/end of each scan line that should be skipped
		// due to plane clipping or because the sample point lies outside source bitmap.
		// Speeds things up by avoiding testing/clamping within the pixel loop
		for (; destPixW > 0;
					destPixL++,
					destPixW--,
					src1oZ += srcd1oZ,
					srcUoZ += srcdUoZ,
					srcVoZ += srcdVoZ)
		{
			// Skip pixel if dEye/Z is outside the range (0.5, 2)
			if (src1oZ <= (FP_1oZ_ONE >> 1) || src1oZ >= (FP_1oZ_ONE << 1))
				continue;

			// Fixed-point reciprocal is much faster than using integer division (on ARM7/9)
			int	srcZ;
			FP_RECIPROCAL((src1oZ >> (FP_1oZ_BITS - FP_REC_BITS)), srcZ);

			// Check whether we are beyond horizontal edge of source and skip pixel if so
			int srcSubpixX		= FP_REC_MULT(srcZ, (srcUoZ >> (FP_UoZ_BITS - 8)));
			if (srcSubpixX < 0 || srcSubpixX >= params.srcClipR)
				continue;

			// Check whether we are beyond vertical edge of source and skip pixel if so
			int srcSubpixY	= FP_REC_MULT(srcZ, (srcVoZ >> (FP_UoZ_BITS - 8)));
			if (srcSubpixY < 0 || srcSubpixY >= params.srcClipB)
				continue;

			// If we don't skip this pixel for any of the reasons tested above,
			// drop out and proceed to the next step
			break;
		}

		// Now we test the pixels from the right-hand end of the scan line
		int srcEnd1oZ = src1oZ + destPixW * srcd1oZ;
		int srcEndUoZ = srcUoZ + destPixW * srcdUoZ;
		int srcEndVoZ = srcVoZ + destPixW * srcdVoZ;

		// Work backwards from the end, decrementing line width for each pixel skipped
		for (; destPixW > 0;
				destPixW--,
				srcEnd1oZ -= srcd1oZ,
				srcEndUoZ -= srcdUoZ,
				srcEndVoZ -= srcdVoZ)
		{
			// Skip pixel if dEye/Z is outside the range (0.5, 2)
			if (srcEnd1oZ <= (FP_1oZ_ONE >> 1) || srcEnd1oZ >= (FP_1oZ_ONE << 1))
				continue;

			// Fixed-point reciprocal is much faster than using integer division (on ARM7/9)
			int	srcZ;
			FP_RECIPROCAL((srcEnd1oZ >> (FP_1oZ_BITS - FP_REC_BITS)), srcZ);

			// Check whether we are beyond horizontal edge of source and skip pixel if so
			int srcSubpixX	= FP_REC_MULT(srcZ, (srcEndUoZ >> (FP_UoZ_BITS - 8)));
			if (srcSubpixX < 0 || srcSubpixX >= params.srcClipR)
				continue;

			// Check whether we are beyond vertical edge of source and skip pixel if so
			int srcSubpixY	= FP_REC_MULT(srcZ, (srcEndVoZ >> (FP_UoZ_BITS - 8)));
			if (srcSubpixY < 0 || srcSubpixY >= params.srcClipB)
				continue;

			// If we don't skip this pixel for any of the reasons tested above,
			// drop out and proceed to the line scan
			break;
		}

		// Ready to scan the row
		LcTByte* destPixPtr	= destRowPtr + (destPixL * DEST_BYTES_PP);

		// ******* Horizontal scan (pixel by pixel) *******

		// Iterate destination pixels, advancing source X/Y via reciprocals
		for (int j = 0; j < destPixW;
					j++,
					src1oZ += srcd1oZ,
					srcUoZ += srcdUoZ,
					srcVoZ += srcdVoZ,
					destPixPtr += DEST_BYTES_PP)
		{
			// Fixed-point reciprocal is much faster than using integer division (on ARM7/9)
			int	srcZ;
			FP_RECIPROCAL((src1oZ >> (FP_1oZ_BITS - FP_REC_BITS)), srcZ);

			// Derive source X,Y from interpolated reciprocals by multiplying in Z
			int srcPixX		= FP_REC_MULT(srcZ, (srcUoZ >> (FP_UoZ_BITS - 8))) >> 8;
			int srcPixY		= FP_REC_MULT(srcZ, (srcVoZ >> (FP_UoZ_BITS - 8))) >> 8;

			// Calculate absolute source offset each time (Y may change)
			LcTByte* srcPixelPtr = srcData +
				(srcPixY * srcRowSpan) +
				(srcPixX * 4);

			// ******* Alpha sampling *******

			register int outAlpha = (srcPixelPtr[3] * modOpacity) >> 8;

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// ******* Color sampling *******

			// Sample nearest
			register int outColor0 = (int)srcPixelPtr[0];
			register int outColor1 = (int)srcPixelPtr[1];
			register int outColor2 = (int)srcPixelPtr[2];

			// ******* Pixel output *******

			// Macro expands to pixel-format-specific blending code
			PIXEL_BLEND(destPixPtr, outColor, outAlpha);

		} // for j
	} // for i

	// Clean up
	cleanup3DBlitParams(&params);


} //LcCNdiGraphics::blitBitmap3DFast
#endif //defined(IFX_RENDER_INTERNAL)


/***************************************************************************//**
	TEXT BLITTING FUNCTIONS

	blitNdiText() delegates to one of four internal blit functions:-
		blitText2DFull()
		blitText3DFull()
		blitText2DFast()
		blitText3DFast() // not yet implemented
*/

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCNdiGraphics::blitNdiText(
	LcCNdiBitmapFile*			srcFontNdi,
	LcCNdiFont::TTextMap*		srcTextMap,
	const LcTPixelDim&			srcTextDim,
	const LcTScalarQuad&		destQuad,
	const LcTPixelRect&			destClip,
	LcTColor					color,
	LcTScalar					opacity,
	bool						bLinear)
{
	// Check state and parameters
	if (!m_surface
	||	!srcFontNdi
	||	!srcTextMap
	||	srcFontNdi->getFormat()	!= LcCNdiBitmapFile::KFormatFont
	||	srcTextDim.width		<= 0
	||	srcTextDim.height		<= 0)
	{
		return;
	}

	// Check state and parameters
	LcTPixelDim srcSize = srcFontNdi->getSize();
	if (srcSize.width			!= LC_BMF_BITMAP_WIDTH
	||	srcSize.height			!= LC_BMF_BITMAP_HEIGHT)
	{
		return;
	}

	// Check to see if dest quad is in X,Y plane
	LcTScalarRect destRect;
	if (destQuad.convertToRect(destRect))
	{
		// Check item is within the valid clipping planes - the valid
		// range is (0.5, -2.0)
		if((destRect.getZDepth() > 0.5) || (destRect.getZDepth() < -2.0))
			return;

		// Check params to see if we can use fastest routine
		if (!bLinear)
		{
			// Call fast 2D blit (sketchy)
			blitText2DFast(
				srcFontNdi->getData(),
				srcTextMap,
				srcTextDim,
				destRect,
				destClip,
				color,
				opacity);
		}
		else
		{
			// Call full-featured 2D blit
			blitText2DFull(
				srcFontNdi->getData(),
				srcTextMap,
				srcTextDim,
				destRect,
				destClip,
				color,
				opacity,
				bLinear);
		}
	}
	else
	{
		// Note:  3D "fast" version not yet implemented.  This might offer
		// some improvement in sketchy mode, but this would be the only
		// difference between fast and full versions, so given the complexity
		// of the 3D routine the performance gain would not be as much as it
		// is for bitmaps where color tint and margins are both avoided as well

		// If not, delegate to 3D blit instead
		#if defined(IFX_RENDER_INTERNAL)
			blitText3DFull(
				srcFontNdi->getData(),
				srcTextMap,
				srcTextDim,
				destQuad,
				destClip,
				color,
				opacity,
				bLinear);
		#endif //defined(IFX_RENDER_INTERNAL)
	}

} //LcCNdiGraphics::blitNdiText

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCNdiGraphics::blitText2DFull(
	LcTByte*					srcFontPixData,
	LcCNdiFont::TTextMap*		srcTextMap,
	const LcTPixelDim&			srcTextDim,
	const LcTScalarRect&		destRect,
	const LcTPixelRect&			destClip,
	LcTColor					color,
	LcTScalar					opacity,
	bool						bLinear)
{
	// Set up 2D blit params
	T2DBlitParams params;
	if (!calc2DBlitParams(
		params,
		srcTextDim,
		destRect,
		destClip,
		bLinear))
	{
		return;
	}


	// Initial Y position (subpixel)
	int srcSubpixY			= params.srcSubpixY0;

	// Modulation values
	int modColor0			= color.rgba.r;
	int modColor1			= color.rgba.g;
	int modColor2			= color.rgba.b;

	int modOpacity			= int(opacity * 0xFF);

	// Bitmap iterators
	LcTByte* srcData		= srcFontPixData;
	unsigned destRowSpan	= m_surface->getLineSpan();
	LcTByte* destRowPtr		= m_surface->getDataStart()
								+ params.destPixT * destRowSpan
								+ params.destPixL * DEST_BYTES_PP;

	// ******* Vertical scan (row by row) *******

	// Iterate destination rows, advancing source Y by scale factor
	// Note that scan is inclusive of last row; first and last rows are weighted
	for (int i = params.destPixT; i <= params.destPixB;
			i++,
			srcSubpixY += params.srcScaleY,
			destRowPtr += destRowSpan)
	{
		bool bBottomEdge = (srcSubpixY >= params.srcClipB);

		// Clamp Y to clipped range
		unsigned clamped = srcSubpixY;
		if (srcSubpixY < 0)
			clamped	= 0;
		if (bBottomEdge)
			clamped	= params.srcClipB;

		// Round Y to get integral source pixel coord
		int srcPixY = (clamped >> 16);

		// Calculate row weightings for linear sampling
		int srcWeightY = 0;
		if (bLinear)
			srcWeightY = (clamped >> 8) & 0xFF;

		// Initialize for destination row scan
		LcTByte* destPixPtr		= destRowPtr;
		int srcSubpixX			= params.srcSubpixX0;

		// ******* Horizontal scan (pixel by pixel) *******

		// Iterate destination pixels, advancing source X by scale factor
		// Note that scan is inclusive of last col; first and last cols are weighted
		for (int j = params.destPixL; j <= params.destPixR;
				j++,
				srcSubpixX += params.srcScaleX,
				destPixPtr += DEST_BYTES_PP)
		{
			bool bRightEdge = (srcSubpixX >= params.srcClipR);

			// Clamp X to clipped range
			clamped = srcSubpixX;
			if (srcSubpixX < 0)
				clamped	= 0;
			if (bRightEdge)
				clamped	= params.srcClipR;

			// Round X to get integral source pixel coord
			int srcPixX = (clamped >> 16);

			// Calculate weightings for linear sampling
			int srcWeightX = 0;
			if (bLinear)
				srcWeightX = (clamped >> 8) & 0xFF;

			// Use text map to get font bitmap pixel
			LcTByte* srcPixelPtr0 = srcData
				+ LC_BMF_BITMAP_WIDTH * (srcTextMap[srcPixX].glyphYOffset + srcPixY)
				+ srcTextMap[srcPixX].glyphX;

			// ******* Alpha sampling *******

			int outAlpha = 0;

			if (!bLinear)
			{
				// Nearest sample
				outAlpha = srcPixelPtr0[0];
			}
			else // linear sampling
			{
				// All 4 samples available, do bilinear interpolation
				register int samp0;
				register int samp1;

				// Sample left column and interpolate
				samp0 = srcPixelPtr0[0];
				samp1 = srcPixelPtr0[LC_BMF_BITMAP_WIDTH];

				outAlpha = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

				// Don't sample right column if we're at right edge of virtual bitmap
				if (!bRightEdge)
				{
					// Second lookup to get right column
					LcTByte* srcPixelPtr1 = srcData
						+ LC_BMF_BITMAP_WIDTH * (srcTextMap[srcPixX + 1].glyphYOffset + srcPixY)
						+ srcTextMap[srcPixX + 1].glyphX;

					// Sample right column and interpolate column and row
					samp0 = srcPixelPtr1[0];
					samp1 = srcPixelPtr1[LC_BMF_BITMAP_WIDTH];

					samp0 += (((samp1 - samp0) * srcWeightY) >> 8);
					outAlpha += (((samp0 - outAlpha) * srcWeightX) >> 8);
				}

			} // alpha modes

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// Apply combined global/row opacity factor
			if (modOpacity < 0xFF)
				outAlpha = (outAlpha * modOpacity) >> 8;

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// ******* Pixel output *******

			register int outColor0 = modColor0;
			register int outColor1 = modColor1;
			register int outColor2 = modColor2;

			// Macro expands to pixel-format-specific blending code
			PIXEL_BLEND(destPixPtr, outColor, outAlpha);

		} // for j
	} // for i


} //LcCNdiGraphics::blitText2DFull

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcCNdiGraphics::blitText2DFast(
	LcTByte*					srcFontPixData,
	LcCNdiFont::TTextMap*		srcTextMap,
	const LcTPixelDim&			srcTextDim,
	const LcTScalarRect&		destRect,
	const LcTPixelRect&			destClip,
	LcTColor					color,
	LcTScalar					opacity)
{
	// Set up 2D blit params
	T2DBlitParams params;
	if (!calc2DBlitParams(
		params,
		srcTextDim,
		destRect,
		destClip,
		false))
	{
		return;
	}


	// Trim top coord to remove any need for clamping
	while (params.srcSubpixY0 < 0)
	{
		params.destPixT++;
		params.srcSubpixY0 += params.srcScaleY;
	}

	// Trim bottom coord to remove any need for clamping
	while (params.srcSubpixY0 + (params.destPixB - params.destPixT) * params.srcScaleY
		> params.srcClipB)
	{
		params.destPixB--;
	}

	// Trim left coord to remove any need for clamping
	while (params.srcSubpixX0 < 0)
	{
		params.destPixL++;
		params.srcSubpixX0 += params.srcScaleX;
	}

	// Trim right coord to remove any need for clamping
	while (params.srcSubpixX0 + (params.destPixR - params.destPixL) * params.srcScaleX
		> params.srcClipR)
	{
		params.destPixR--;
	}

	// Y cursor
	int srcSubpixY			= params.srcSubpixY0;

	// Modulation parameters
	int modColor0			= color.rgba.r;
	int modColor1			= color.rgba.g;
	int modColor2			= color.rgba.b;
	int modOpacity			= int(opacity * 0xFF);

	// Count of bytes between start of one row and start of next
	unsigned destRowSpan	= m_surface->getLineSpan();

	// Addresses of first byte of first row
	LcTByte* destRowPtr		= m_surface->getDataStart()
								+ params.destPixT * destRowSpan
								+ params.destPixL * DEST_BYTES_PP;

	// ******* Vertical scan (row by row) *******

	// Iterate destination rows, advancing source Y by scale factor
	// Note that scan is inclusive of last row; first and last rows are weighted
	for (int i = params.destPixT; i <= params.destPixB;
			i++,
			srcSubpixY += params.srcScaleY,
			destRowPtr += destRowSpan)
	{
		// Round Y to get integral source pixel coord
		int srcPixY = (srcSubpixY >> 16);

		// Initialize for destination row scan
		LcTByte* destPixPtr		= destRowPtr;
		int srcSubpixX			= params.srcSubpixX0;

		// ******* Horizontal scan (pixel by pixel) *******

		// Iterate destination pixels, advancing source X by scale factor
		// Note that scan is inclusive of last col; first and last cols are weighted
		for (int j = params.destPixL; j <= params.destPixR;
				j++,
				srcSubpixX += params.srcScaleX,
				destPixPtr += DEST_BYTES_PP)
		{
			// Round X to get integral source pixel coord
			int srcPixX = (srcSubpixX >> 16);

			// Use text map to get font bitmap pixel
			LcTByte* srcPixelPtr0 = srcFontPixData
				+ LC_BMF_BITMAP_WIDTH * (srcTextMap[srcPixX].glyphYOffset + srcPixY)
				+ srcTextMap[srcPixX].glyphX;

			// ******* Alpha sampling *******

			register int outAlpha = (srcPixelPtr0[0] * modOpacity) >> 8;

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// ******* Pixel output *******

			register int outColor0 = modColor0;
			register int outColor1 = modColor1;
			register int outColor2 = modColor2;

			// Macro expands to pixel-format-specific blending code
			PIXEL_BLEND(destPixPtr, outColor, outAlpha);

		} // for j
	} // for i


} //LcCNdiGraphics::blitText2DFast

#if defined(IFX_RENDER_INTERNAL)
/*-------------------------------------------------------------------------*//**
*/
void LcCNdiGraphics::blitText3DFull(
	LcTByte*					srcFontPixData,
	LcCNdiFont::TTextMap*		srcTextMap,
	const LcTPixelDim&			srcTextDim,
	const LcTScalarQuad&		destQuad,
	const LcTPixelRect&			destClip,
	LcTColor					color,
	LcTScalar					opacity,
	bool						bLinear)
{
	// Set up perspective data - edge end points, etc
	T3DBlitParams params;

	// This allocates memory that is not trapped in the cleanup code. 
	// Ensure that if calc3DBlitParams succeeds, then the remainder of this 
	// routine has only one return point, and can not throw an exception.
	if (!calc3DBlitParams(
		&params,
		srcTextDim,
		destQuad,
		destClip,
		bLinear))
	{
		return;
	}


	// Modulation values
	int modColor0			= color.rgba.r;
	int modColor1			= color.rgba.g;
	int modColor2			= color.rgba.b;

	int modOpacity			= int(opacity * 0xFF);

	// Addresses of first byte of first row
	unsigned destRowSpan	= m_surface->getLineSpan();
	LcTByte* destRowPtr		= m_surface->getDataStart() + params.destPixY * destRowSpan;
	int destPixY			= params.destPixY;

	// ******* Vertical scan (row by row) *******

	for (int i = params.destStartIndex; i < params.destEndIndex;
			i++,
			destPixY++,
			destRowPtr += destRowSpan)
	{
		// Check for infinity condition
		if (!params.edgeUp[i].src1oZ)
			continue;

		// Get X coords of row start/end
		FP_COORD destUpX	= params.edgeUp[i].destX;
		int destPixL		= destUpX >> FP_X_BITS;
		int destPixR		= params.edgeDown[i].destX >> FP_X_BITS;

		// Apply horizontal clipping to the destination edge coords
		destPixL			= max(destPixL, destClip.getLeft());
		destPixR			= min(destPixR, destClip.getRight());
		int destPixW		= destPixR - destPixL;

		// Skip the row if it is fully clipped (later rows might not be, so continue)
		// Note: flipped/inverted images are reversed by scanEdges.
		if (destPixW <= 0)
			continue;

		// Left hand edge values
		FP_FRACT src1oZ		= params.edgeUp[i].src1oZ;
		FP_FRACT srcUoZ		= params.edgeUp[i].srcUoZ;
		FP_FRACT srcVoZ		= params.edgeUp[i].srcVoZ;

		// Work out the linear increment for each parameter pix dest pixel moved
		float dest1Wf		= float(FP_X_ONE) / (params.edgeDown[i].destX  - destUpX);
		FP_FRACT srcd1oZ	= (FP_FRACT)((params.edgeDown[i].src1oZ - src1oZ) * dest1Wf);
		FP_FRACT srcdUoZ	= (FP_FRACT)((params.edgeDown[i].srcUoZ - srcUoZ) * dest1Wf);
		FP_FRACT srcdVoZ	= (FP_FRACT)((params.edgeDown[i].srcVoZ - srcVoZ) * dest1Wf);

		// Apply offset to start values to compensate for dest pixel clipping AND rounding
		// NB: Without this we will get a "postage stamp" effect
		// NB2: We add 0.5 when mapping a dest coord to a sample coord
		float destXOffset	= destPixL - (float(destUpX) / FP_X_ONE) + 0.5;
		src1oZ				+= (FP_FRACT)(destXOffset * srcd1oZ);
		srcUoZ				+= (FP_FRACT)(destXOffset * srcdUoZ);
		srcVoZ				+= (FP_FRACT)(destXOffset * srcdVoZ);

		// Ready to scan the row
		int destPixX		= destPixL;
		LcTByte* destPixPtr	= destRowPtr + (destPixL * DEST_BYTES_PP);

		// ******* Horizontal scan (pixel by pixel) *******

		// Iterate destination pixels, advancing source X/Y via reciprocals
		for (int j = 0; j < destPixW;
					j++,
					src1oZ += srcd1oZ,
					srcUoZ += srcdUoZ,
					srcVoZ += srcdVoZ,
					destPixX++,
					destPixPtr += DEST_BYTES_PP)
		{
			// Skip pixel if dEye/Z is outside the range (0.5, 2)
			if (src1oZ <= (FP_1oZ_ONE >> 1) || src1oZ >= (FP_1oZ_ONE << 1))
				continue;
			
			// Fixed-point reciprocal is much faster than using integer division (on ARM7/9)
			int	srcZ;
			FP_RECIPROCAL((src1oZ >> (FP_1oZ_BITS - FP_REC_BITS)), srcZ);

			// Derive source X,Y from interpolated reciprocals by multiplying in Z
			int srcSubpixX	= FP_REC_MULT(srcZ, (srcUoZ >> (FP_UoZ_BITS - 8)));
			int srcSubpixY	= FP_REC_MULT(srcZ, (srcVoZ >> (FP_UoZ_BITS - 8)));

			// Adjust sample point for non-linear sampling
			// In linear mode, we subtract 0.5 after mapping a dest coord to a sample coord
			if (bLinear)
			{
				// Note we are <<8 so this effectively subtracts 0.5
				srcSubpixX -= (0x80);
				srcSubpixY -= (0x80);
			}

			// Check whether we are at edge of source bitmap
			bool bRightEdge		= (srcSubpixX >= params.srcClipR);
			bool bBottomEdge	= (srcSubpixY >= params.srcClipB);

			// Clamp X to clipped range
			unsigned clampedX = srcSubpixX;
			if (srcSubpixX < 0)
				clampedX = 0;
			if (bRightEdge)
				clampedX = params.srcClipR;

			// Clamp Y to clipped range
			unsigned clampedY = srcSubpixY;
			if (srcSubpixY < 0)
				clampedY = 0;
			if (bBottomEdge)
				clampedY = params.srcClipB;

			// Round to get integral source pixel coords
			int srcPixX		= (clampedX >> 8);
			int srcPixY		= (clampedY >> 8);

			// Calculate weightings for linear sampling
			int srcWeightX = 0;
			int srcWeightY = 0;
			if (bLinear)
			{
				// Take raw weight from coordinate fraction, scale, and clamp
				srcWeightX	= (clampedX & 0xFF);
				srcWeightY	= (clampedY & 0xFF);
			}

			// Use text map to get font bitmap pixel
			LcTByte* srcPixelPtr0 = srcFontPixData
				+ LC_BMF_BITMAP_WIDTH * (srcTextMap[srcPixX].glyphYOffset + srcPixY)
				+ srcTextMap[srcPixX].glyphX;

			// ******* Alpha sampling *******

			int outAlpha = 0;

			if (!bLinear)
			{
				// Nearest sample
				outAlpha = srcPixelPtr0[0];
			}
			else // linear sampling, imageType == KFormatFont or KFormatGraphicTranslucent
			{
				register int samp0;
				register int samp1;

				// Sample left column and interpolate
				samp0 = srcPixelPtr0[0];
				samp1 = srcPixelPtr0[LC_BMF_BITMAP_WIDTH];

				outAlpha = samp0 + (((samp1 - samp0) * srcWeightY) >> 8);

				// Don't sample right column if we're at right edge
				if (!bRightEdge)
				{
					// Use text map to get font bitmap pixel
					LcTByte* srcPixelPtr1 = srcFontPixData
						+ LC_BMF_BITMAP_WIDTH * (srcTextMap[srcPixX + 1].glyphYOffset + srcPixY)
						+ srcTextMap[srcPixX + 1].glyphX;

					// Sample right column and interpolate column and row
					samp0 = srcPixelPtr1[0];
					samp1 = srcPixelPtr1[LC_BMF_BITMAP_WIDTH];

					samp0 += (((samp1 - samp0) * srcWeightY) >> 8);
					outAlpha += (((samp0 - outAlpha) * srcWeightX) >> 8);
				}

			} // alpha modes

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// Apply global opacity factor
			if (modOpacity < 0xFF)
				outAlpha = (outAlpha * modOpacity) >> 8;

			// Skip now if pixel is close to transparent
			if (outAlpha < LC_ALPHA_MIN)
				continue;

			// ******* Pixel output *******

			register int outColor0 = modColor0;
			register int outColor1 = modColor1;
			register int outColor2 = modColor2;

			// Macro expands to pixel-format-specific blending code
			PIXEL_BLEND(destPixPtr, outColor, outAlpha);

		} // for j
	} // for i

	// Clean up
	cleanup3DBlitParams(&params);


} //LcCNdiGraphics::blitText3DFull
#endif //defined(IFX_RENDER_INTERNAL)


/***************************************************************************//**
	NATIVE BLITTING FUNCTIONS

	Contains two public blit functions designed to work with source bitmaps
	that are already converted to native canvas format:-
		blitNative()
		blitNativeRLE()
*/

#if defined(IFX_RENDER_INTERNAL_COMPRESSED)
/*-------------------------------------------------------------------------*//**
	Palette entries must already have been converted to native color format
*/
LC_EXPORT void LcCNdiGraphics::blitNativeRLE(
	TRlePos*				srcRleIndex,
	LcTUInt32*				srcPalette,
	const LcTPixelDim&		srcSize,
	const LcTPixelRect&		srcRect)
{
	// We assume that bounds returned by surface map exactly to
	// the data and span also returned by surface (i.e. no clipping)
	LcTPixelRect canvas		= m_surface->getBounds();

	// Copy param to modify
	LcTPixelRect srcSizeRect(0, 0, srcSize.width, srcSize.height);

	// Setup the destination rect, this must not be less then the screen size;
	LcTPixelRect destRect;
	if (srcSizeRect.getWidth() < canvas.getWidth())
		destRect = srcRect.intersection(canvas);
	else
		destRect = srcRect.intersection(srcSizeRect);

	// Check for parameter errors
	if (!m_surface
	||	destRect.getWidth()  < 1
	||	destRect.getHeight() < 1)
	{
		return;
	}


	// Adjust source size to account for screen image size differences.
	LcTPixelRect srcImageRect = srcSizeRect;
	if (srcSizeRect.getWidth() != canvas.getWidth())
	{
		srcImageRect.setLeft((canvas.getWidth() - srcSizeRect.getWidth()) / 2);
		srcImageRect.setRight(srcImageRect.getLeft() + srcSizeRect.getWidth());
	}

	// Set the clip rectangle to the background size. This cannot be greater
	// then the screen size.
	LcTPixelRect destClip;
	if (canvas.getWidth() > srcImageRect.getWidth())
		destClip = srcImageRect;
	else
		destClip = canvas;

	// For drawing, coords are required relative to canvas not screen
	destRect.setLeft(destRect.getLeft() - canvas.getLeft());
	destRect.setTop(destRect.getTop() - canvas.getTop());
	destClip.setLeft(destClip.getLeft() - canvas.getLeft());
	destClip.setTop(destClip.getTop() - canvas.getTop());

	// Work out clipped destination positions
	int destPixL	= max(destClip.getLeft(), destRect.getLeft());
	int destPixT	= max(destClip.getTop(), destRect.getTop());
	int destPixR	= min(destClip.getRight(), destRect.getRight());
	int destPixB	= min(destClip.getBottom(), destRect.getBottom());

	// Check if rect is totally clipped away
	// NB: R/B are outside draw rect
	if (destPixL >= destPixR || destPixT >= destPixB)
		return;

	// Sample coords are based on integral position of destination pixels
	// after clipping has been applied (could end up negative!)
	int destOffsetX			= destPixL - destRect.getLeft();
	int destOffsetY			= destPixT - destRect.getTop();

	// Map the sample point into source pixmap space
	int srcPixX0			= srcRect.getLeft() + destOffsetX - srcImageRect.getLeft();
	int srcPixY				= srcRect.getTop() + destOffsetY - srcImageRect.getTop();

	// Count of bytes between start of one row and start of next
	unsigned destRowSpan	= m_surface->getLineSpan();

	// Addresses of first byte of first row
	LcTByte* destRowPtr		= m_surface->getDataStart()
								+ destPixT * destRowSpan
								+ destPixL * DEST_BYTES_PP;

	// ******* Vertical scan (row by row) *******

	// Iterate destination rows, advancing source Y by scale factor
	// Note that scan is inclusive of last row; first and last rows are weighted
	for (int i = destPixT; i < destPixB;
				i++,
				srcPixY++,
				destRowPtr += destRowSpan)
	{
		// Scan the next row
		TRlePos rleRowC0	= srcRleIndex[srcPixY];
		int rleCPos			= 0;

		// Initialize for destination row scan
		int srcPixX			= srcPixX0;
		LcTByte* destPixPtr	= destRowPtr;

		// ******* Horizontal scan (pixel by pixel) *******

		// Iterate destination pixels, advancing source X by scale factor
		// Note that scan is inclusive of last col; first and last cols are weighted
		for (int j = destPixL; j < destPixR;
				j++,
				srcPixX++,
				destPixPtr += DEST_BYTES_PP)
		{
			// RLE mode: advance through the encoding by a calculated offset
			int rleCOffset		= srcPixX - rleCPos;
			rleCPos				= srcPixX;

			// Advance RLE cursors through encoded data
			LcTByte sample;
			RLE_ADVANCE(rleRowC0, rleCOffset, notUsed, sample);

			// ******* Pixel output *******

			// Copy pixel to destination
			#if defined(IFX_CANVAS_MODE_888)   || \
				defined (IFX_CANVAS_MODE_8888)
				LcTByte* srcPix = (LcTByte*)&srcPalette[sample];
				destPixPtr[0] = srcPix[0];
				destPixPtr[1] = srcPix[1];
				destPixPtr[2] = srcPix[2];

			// all other modes are 16 bit
			#else
				LcTUInt16* srcPix = (LcTUInt16*)&srcPalette[sample];
				*((LcTUInt16*)destPixPtr) = srcPix[0];
			#endif

		} // for j
	} // for i


} //LcCNdiGraphics::blitNativeRLE
#endif //defined(IFX_RENDER_INTERNAL_COMPRESSED)


