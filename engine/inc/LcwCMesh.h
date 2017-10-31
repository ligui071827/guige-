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
#ifndef LcwCMeshH
#define LcwCMeshH


/*-------------------------------------------------------------------------*//**
	A simple, generic mesh widget
*/
class LcwCMesh : public LcCWidget
{
	LC_DECLARE_RTTI(LcwCMesh, LcCWidget)

private:

	LcCMesh*						m_mesh;
	bool							m_antiAlias;
	
	// LcCWidget interface
	LC_VIRTUAL		void			onRealize();
	LC_VIRTUAL		void			onPlacementChange(int mask);

	LC_VIRTUAL		void			onPaint(const LcTPixelRect& clip);
	LC_VIRTUAL		void			onPaintOpaque(const LcTPixelRect& clip);
	LC_VIRTUAL		void			onPrepareForPaint();
	LC_VIRTUAL		void			onWantBoundingBox();
	LC_VIRTUAL		bool			contains(const LcTVector& loc, LcTScalar expandRectEdge);
	virtual 		bool			canBeClipped()			{ return true; }

protected:
					void			setAntiAlias(bool antiAlias)	{ m_antiAlias = antiAlias; }

LC_PRIVATE_INTERNAL_PROTECTED:

	// Allow only 2-phase construction
	LC_IMPORT						LcwCMesh();

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcwCMesh> create();
	LC_VIRTUAL						~LcwCMesh();
	

	// Configuration
	LC_IMPORT		void			setMesh(LcCMesh* m);
	inline			LcCMesh*		getMesh()				{ return m_mesh; }
					bool			isAntiAliased()			{ return m_antiAlias; }
	LC_VIRTUAL		void			reloadResources();
	LC_VIRTUAL		void			releaseResources();
	LC_VIRTUAL		bool			obedientToZ()			{ return true;}
};

#endif
