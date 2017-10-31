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

#ifndef NdhsCMemoryImageH
#define NdhsCMemoryImageH

#if !defined(LC_PLAT_OGL)
	class LcCNdiGraphics;
#endif

#if defined(LC_PLAT_OGL)
	class NdhsCMemoryImage : public LcOglCBitmap
#else
	class NdhsCMemoryImage : public LcCBitmap
#endif
{
public:

    typedef enum
    {
        EImageTypeInternalBuffer = 0,
        EImageTypeExternalBuffer,
#if defined(LC_PLAT_OGL)
        EImageTypeExternalTexture
#endif
    } EImageType;

private:

	// Graphics object to draw to
#if defined(LC_PLAT_OGL)
	LcTmOwner<LcOglCTexture>		m_texture;
#else
	LcCNdiGraphics*					m_gfx;
#endif

	EImageType                      m_imageType;
	IFX_BUFFER_FORMAT				m_format;
	void						   *m_buffer;
	LcTmAlloc<LcTByte>				m_data;
	int								m_width;
	int								m_height;

	// LcIImage methods
	LC_VIRTUAL		bool			canBeClipped()	{ return true; }

	// LcCBitmap methods
	LC_VIRTUAL		void			drawRegion(
										const LcTScalarRect&	src,
										const LcTPlaneRect&		dest,
										const LcTPixelRect&		clip,
										LcTColor				color,
										LcTScalar				opacity,
										bool					antiAlias,
										int						meshGridX,
										int						meshGridY);

	LC_VIRTUAL		bool			isPointTransparent(int iX, int iY);

	// Two-phase construction
									NdhsCMemoryImage(LcCSpace* sp);

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<NdhsCMemoryImage> create(LcCSpace* sp, int width, int height, IFX_BUFFER_FORMAT format, EImageType imageType = EImageTypeInternalBuffer);

	virtual							~NdhsCMemoryImage();

	bool							construct(int width, int height, IFX_BUFFER_FORMAT format, EImageType imageType);

	bool							resize(int width, int height);
#if defined(LC_PLAT_OGL)
	bool							setExternalTexture(GLuint texture[IFX_OGL_MAX_TEXTURES], LcTPixelDim texDim, LcTPixelRect bitmapRect, IFX_UINT32 textureTarget);
#endif
	void							setDirty();
	void							setBufferData(void *buffer);
	void	 						setImageData(LcTByte *data);
	LcTByte* 						releaseImageData();

	LcTByte*						data()								{ return m_data; }

	void							setTranslucency(bool isTrans) { m_isTranslucent = isTrans; }
	void							setPreMultipliedAlpha(bool value);
};
#endif//NdhsCMemoryImageH
