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
#ifndef LcCNdiGraphicsH
#define LcCNdiGraphicsH

#include "inflexionui/engine/inc/LcCBase.h"
#include "inflexionui/engine/inc/LcCNdiBitmapFile.h"
#include "inflexionui/engine/inc/LcCCustomBitmapFile.h"
#include "inflexionui/engine/inc/LcCNdiFont.h"
struct T2DBlitParams;
class T3DBlitParams;
struct TEdgeParams;
struct TMarginParams;

/*-------------------------------------------------------------------------*//**
	Graphics methods for drawing NDI bitmaps to an LcCNdiGraphics::ISurface.
	The latter interface would typically be implemented by platform-specific
	space classes to draw to the off-screen canvas
*/
class LcCNdiGraphics : public LcCBase
{
public: //types

	// Target drawing surface must implement this
	class ISurface
	{
	public:
		virtual		int				getLineSpan()	= 0;
		virtual		LcTPixelRect	getBounds()		= 0;
		virtual		LcTByte*		getDataStart()	= 0;
	};

	// Description of bitmap margins
	struct TMargins
	{
		int							left;				
		int							right;				
		int							top;				
		int							bottom;				

		// Effective size of bitmap being drawn
		int							extentWidth;	
		int							extentHeight;				
	};

private:

	// Surface to draw to
	ISurface*						m_surface;

	// Helpers
					bool			calc2DBlitParams(
										T2DBlitParams&				params,
										const LcTPixelDim&			srcSize,
										const LcTScalarRect&		destRectI,
										const LcTPixelRect&			destClipI,
										bool						bLinear);

					bool			calcMarginParams(
										TMarginParams&				params,
										LcTPixelDim&				srcEffectiveSize,
										TMargins*					srcMargins);

#if defined(IFX_RENDER_INTERNAL)
					bool			calc3DBlitParams(
										T3DBlitParams*				params,
										const LcTPixelDim&			srcSize,
										const LcTScalarQuad&		destQuad,
										const LcTPixelRect&			destClipI, 
										bool						bLinear);

					void			cleanup3DBlitParams(
										T3DBlitParams*				params);

					void			scanEdge(
										TEdgeParams*				edgeUp,
										TEdgeParams*				edgeDown,
										int							minY,
										const LcTVector&			destA,
										const LcTVector&			destB,
										const LcTVector&			srcA, // u,v
										const LcTVector&			srcB);
#endif //defined(IFX_RENDER_INTERNAL)

	// Main blit functions - blit calls should go to the public functions below,
	// from where they will be dispatched to the appropriate internal blit function

					void			blitBitmap2DFull(
										LcTByte*					srcPixData,
										const LcTPixelDim&			srcSize,
										unsigned					srcRowSpan,
										LcCNdiBitmapFile::EFormat	srcFormat,
										TMargins*					srcMargins,
										const LcTScalarRect&		destRect,
										const LcTPixelRect&			destClip,
										LcTColor					color,
										LcTScalar					opacity,
										bool						bLinear);

					void			blitBitmap2DFast(
										LcTByte*					srcPixData,
										const LcTPixelDim&			srcSize,
										unsigned					srcRowSpan,
										const LcTScalarRect&		destRect,
										const LcTPixelRect&			destClip,
										LcTScalar					opacity);

					void			blitText2DFull(
										LcTByte*					srcFontPixData,
										LcCNdiFont::TTextMap*		srcTextMap,
										const LcTPixelDim&			srcTextDim,
										const LcTScalarRect&		destRect,
										const LcTPixelRect&			destClip,
										LcTColor					color,
										LcTScalar					opacity,
										bool						bLinear);

					void			blitText2DFast(
										LcTByte*					srcFontPixData,
										LcCNdiFont::TTextMap*		srcTextMap,
										const LcTPixelDim&			srcTextDim,
										const LcTScalarRect&		destRect,
										const LcTPixelRect&			destClip,
										LcTColor					color,
										LcTScalar					opacity);

#if defined(IFX_RENDER_INTERNAL)
					void			blitBitmap3DFull(
										LcTByte*					srcPixData,
										const LcTPixelDim&			srcSize,
										unsigned					srcRowSpan,
										LcCNdiBitmapFile::EFormat	srcFormat,
										TMargins*					srcMargins,
										const LcTScalarQuad&		destQuad,
										const LcTPixelRect&			destClip,
										LcTColor					color,
										LcTScalar					opacity,
										bool						bLinear);

					void			blitBitmap3DFast(
										LcTByte*					srcPixData,
										const LcTPixelDim&			srcSize,
										unsigned					srcRowSpan,
										const LcTScalarQuad&		destQuad,
										const LcTPixelRect&			destClip,
										LcTScalar					opacity);

					void			blitText3DFull(
										LcTByte*					srcFontPixData,
										LcCNdiFont::TTextMap*		srcTextMap,
										const LcTPixelDim&			srcTextDim,
										const LcTScalarQuad&		destQuad,
										const LcTPixelRect&			destClip,
										LcTColor					color,
										LcTScalar					opacity,
										bool						bLinear);

					void			blitText3DFast(
										LcTByte*					srcFontPixData,
										LcCNdiFont::TTextMap*		srcTextMap,
										const LcTPixelDim&			srcTextDim,
										const LcTScalarQuad&		destQuad,
										const LcTPixelRect&			destClip,
										LcTColor					color,
										LcTScalar					opacity);
#endif //defined(IFX_RENDER_INTERNAL)

#if defined(IFX_RENDER_INTERNAL_COMPRESSED)
					void			blitRLE2DFull(
										TRlePos*					srcRleColorIndex,
										TRlePos*					srcRleAlphaIndex,
										LcTUInt32*					srcPalette,
										const LcTPixelDim&			srcSize,
										TMargins*					srcMargins,
										const LcTScalarRect&		destRect,
										const LcTPixelRect&			destClip,
										LcTColor					color,
										LcTScalar					opacity,
										bool						bLinear);
#endif //defined(IFX_RENDER_INTERNAL_COMPRESSED)

	// Two-phase construction
	inline							LcCNdiGraphics(ISurface* surf)
										{ m_surface = surf; }

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcCNdiGraphics> create(ISurface* surf);
	virtual inline					~LcCNdiGraphics() {}

	// Public blit functions - blit calls should go to these,
	// from where they will be dispatched to the appropriate internal blit function

	LC_IMPORT		void			blitNdiBitmap(
										LcCNdiBitmapFile*			srcNdi,
										int							srcFrame,
										const LcTPixelDim&			srcExtent,
										const LcTScalarQuad&		destQuad,
										const LcTPixelRect&			destClip,
										LcTColor					color,
										LcTScalar					opacity,
										bool						bLinear);

	LC_EXPORT void 					blitCustomBitmap(
										LcCCustomBitmapFile*		srcBitmap,
										int							srcFrame,
										const LcTPixelDim&			srcExtent,
										const LcTScalarQuad&		destQuad,
										const LcTPixelRect&			destClip,
										LcTColor					color,
										LcTScalar					opacity,
										bool						bLinear);

	LC_IMPORT		void			blitRawBitmap(
										LcTByte*					srcPixData,
										const LcTPixelDim&			srcSize,
										unsigned					srcRowSpan,
										LcCNdiBitmapFile::EFormat	srcFormat,
										TMargins*					srcMargins,
										const LcTScalarQuad&		destQuad,
										const LcTPixelRect&			destClip,
										LcTColor					color,
										LcTScalar					opacity,
										bool						bLinear);

	LC_IMPORT		void			blitNdiText(
										LcCNdiBitmapFile*			srcFontNdi,
										LcCNdiFont::TTextMap*		srcTextMap,
										const LcTPixelDim&			srcTextDim,
										const LcTScalarQuad&		destQuad,
										const LcTPixelRect&			destClip,
										LcTColor					color,
										LcTScalar					opacity,
										bool						bLinear);

	LC_IMPORT		void			blitNative(
										LcTByte*					srcPixData,
										const LcTPixelDim&			srcSize,
										const LcTPixelRect&			srcRect);

	LC_IMPORT		void			blitNativeRLE(
										TRlePos*					srcRleIndex,
										LcTUInt32*					srcPalette,
										const LcTPixelDim&			srcSize,
										const LcTPixelRect&			srcRect);
};

#endif //LcCNdiGraphicsH

