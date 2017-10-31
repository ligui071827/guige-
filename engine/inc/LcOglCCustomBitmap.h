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
#ifndef LcOglCCustomBitmapH
#define LcOglCCustomBitmapH

#include "inflexionui/engine/inc/LcOglCBitmap.h"

/*-------------------------------------------------------------------------*//**
*/
class LcOglCCustomBitmap : public LcOglCBitmap
{
private:

	// Indicates whether last open operation was successful
	bool						m_bIsOpen;

	// Multi-texturing and mip-mapping is only available in OpenGL ES 2.0 mode
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	int                         m_unit;
	bool                        m_bMipmap;
#endif

protected:

	// Allow only 2-phase construction
	inline						LcOglCCustomBitmap(LcCSpace* sp) : LcOglCBitmap(sp)	{}

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcOglCCustomBitmap> create(LcCSpace* sp);

	// Destruction
	LC_VIRTUAL					~LcOglCCustomBitmap() {};

	// Load from NDI file
	LC_IMPORT	bool			open(const LcTmString& filename,
												int marginLeft = 0,
												int marginRight = 0,
												int marginTop = 0,
												int marginBottom = 0,
												int frameCount = 0);
	inline		bool			isOpen()			{ return m_bIsOpen; }

	virtual		void  			releaseResources();
	virtual		void  			reloadResources();
	
	// Multi-texturing and mip-mapping is only available in OpenGL ES 2.0 mode
#if defined(IFX_RENDER_DIRECT_OPENGL_20)
	inline		void			setTextureUnit(int unit)	{	m_unit = unit;	} 
	inline		int				getTextureUnit()			{	return m_unit;	}
				
	inline		void			setTextureMipmap(bool mipmap)	{	m_bMipmap = mipmap;	} 
	inline		bool			getTextureMipmap()				{	return m_bMipmap;	}
#endif			
};

#endif //LcOglCCustomBitmapH
