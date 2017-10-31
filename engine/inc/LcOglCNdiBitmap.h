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
#ifndef LcOglCNdiBitmapH
#define LcOglCNdiBitmapH

#include "inflexionui/engine/inc/LcOglCBitmap.h"

/*-------------------------------------------------------------------------*//**
*/
class LcOglCNdiBitmap : public LcOglCBitmap
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
	inline						LcOglCNdiBitmap(LcCSpace* sp) : LcOglCBitmap(sp)	{}

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcOglCNdiBitmap> create(LcCSpace* sp);

	// Destruction
	LC_VIRTUAL					~LcOglCNdiBitmap() {};

	// Load from NDI file
	LC_IMPORT	bool			open(const LcTmString& filename);
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

#endif //LcOglCNdiBitmapH
