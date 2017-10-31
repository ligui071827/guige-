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
#ifndef LcTPlacementH
#define LcTPlacementH

#include "inflexionui/engine/inc/LcTVector.h"
#include "inflexionui/engine/inc/LcTQuaternion.h"
#include "inflexionui/engine/inc/LcCSerializeMaster.h" 

#if defined(IFX_RENDER_DIRECT_OPENGL_20)

class	LcOglCSLType;
class	LcOglCEffect;
template <class T> class LcOglCSLTypeScalar; 
template <class T> class LcOglCSLTypeVector;

#else
class	LcOglCEffect;
#endif

/*-------------------------------------------------------------------------*//**
	Encapsulates all those parameters which determine the placement
	(location, extent, scale, orientation and opacity) of any widget within its
	parent aggregate or space.  Note that orientation angle is stored in
	radians
*/
class LcTPlacement : public ISerializeable
							
{
LC_PRIVATE_INTERNAL_PUBLIC:

	static			int				calcFrameChange(int delta, LcTUnitScalar d);

public:

	// Reserve bits 0 to 15 for base widget class.
	// bits 16 to 31 can be used by widget sub-class
	// Mask values for set/interpolate etc
	enum
	{
		// Note: high-word bit values are defined in LcCAnimator
		// and must not be used here.  Make sure that EAll only covers
		// the bits set by the values specified below
		ENone				= 0x00,
		ELocation			= 0x01,
		EExtent				= 0x02,
		EScale				= 0x04,
		EOrientation		= 0x10,
		EOpacity			= 0x20,
		EColor				= 0x40,
		EFrame				= 0x80,
		EOffset				= 0x100,
		EIntensity			= 0x200,
		EColor2				= 0x400,

#if defined (IFX_RENDER_DIRECT_OPENGL_20)
		EUniform			= 0x800,
#endif

		EAll				= 0xFFFF
	};


	// Public so no m_
	LcTVector						location;
	LcTVector						extent;
	LcTVector						centerOffset;
	LcTVector						scale;
	LcTQuaternion					orientation;
	LcTScalar						opacity;
	LcTColor						color;
	int								frame;
	LcTScalar						intensity;
	LcTColor						color2;
	
#if defined(IFX_RENDER_DIRECT_OPENGL_20)	
	typedef	LcTmOwnerMap<LcTmString, LcOglCSLType>		TmEffectUniMap;
	TmEffectUniMap					layoutUniMap;	
	LcTmOwner<LcOglCSLType>			slTypeF;
	
	// Copy Constructor
	LC_IMPORT						LcTPlacement(const LcTPlacement & p);
	
	// Destructor
	LC_VIRTUAL						~LcTPlacement();
	
	// Assignment Operator
	LC_IMPORT void					operator = (const LcTPlacement & p);
	
	// Utility APIs
	LC_IMPORT bool populateMap(const LcTPlacement &p);

#endif

	// Construction
	LC_IMPORT						LcTPlacement();
	LC_IMPORT						LcTPlacement(
										const LcTVector&	lo,
										const LcTVector&	ex,
										const LcTVector&	sc,
										const LcTQuaternion& ori,
										LcTScalar			op,
										LcTColor			col,
										int					fr,
										LcTScalar			inty,
										LcTColor			col2
										);


	// XML configuration
#ifdef LC_USE_XML
	
	LC_IMPORT		int				configureFromXml(LcOglCEffect *effect, LcCXmlElem* pElem, int iMask, bool bPopulateConfigUnimap);
#endif

	// Useful methods
	LC_IMPORT		int				assign(
										const LcTPlacement&	p1,
										int					mask);
	LC_IMPORT		void			offset(
										const LcTPlacement& p1,
										int					mask);
	LC_IMPORT		void			interpolate(
										const LcTPlacement&	p1,
										const LcTPlacement&	p2,
										int					mask,
										LcTScalar			d);
#ifdef IFX_SERIALIZATION
	LcTPlacement* loadState(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	SerializeHandle	serialize(LcCSerializeMaster *serializeMaster,bool force=false);
	void	deSerialize(SerializeHandle handle,LcCSerializeMaster *serializeMaster);
	bool isMenuItemChild(){return false;};
#endif /* IFX_SERIALIZATION */
};

#endif //LcTPlacementH
