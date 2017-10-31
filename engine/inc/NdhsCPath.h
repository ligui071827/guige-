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
#ifndef NdhsCPathH
#define NdhsCPathH

/*-------------------------------------------------------------------------*//**
	A path animator is an animator that changes the placement of a widget
	over time.  The placement details must be provided by an NdhsCPath object,
	which will be passed an index between 0 and 1 to indicate the relative
	position of the widget between the end points of the path.  The IPath
	may implement any continuous path between these limits, for example,
	NdhsCTweenPath provides linear interpolation, and LcCAnimatorLinePath
	allows an arbitrary snake-like path to be defined.
*/

// A path for the widget to follow through 3D space.
class NdhsCPath : public LcCBase
{
private:

	// A is absolute position, B is relative to A
	LcTPlacement				m_A;
	LcTPlacement				m_B;

	int							m_rDelta;
	int							m_gDelta;
	int							m_bDelta;
	int							m_aDelta;

	int							m_r2Delta;
	int							m_g2Delta;
	int							m_b2Delta;
	int							m_a2Delta;

	int							m_mask;

	ENdhsDecoratorType			m_LocationBasic;

	ENdhsDecoratorType			m_locationAnimType;
	ENdhsDecoratorType			m_extentAnimType;
	ENdhsDecoratorType			m_frameAnimType;
	ENdhsDecoratorType			m_colorAnimType;
	ENdhsDecoratorType			m_scaleAnimType;
	ENdhsDecoratorType			m_rotationAnimType;
	ENdhsDecoratorType			m_opacityAnimType;
	ENdhsDecoratorType			m_offsetAnimType;
	ENdhsDecoratorType			m_intensityAnimType;
	ENdhsDecoratorType			m_color2AnimType;

public:

	NdhsCPath();

	// Construction
	LC_IMPORT static LcTaOwner<NdhsCPath> create();


	// NdhsCPath methods
	LC_IMPORT		void		getPlacement(LcTUnitScalar d, LcTPlacement& sa);

	LC_IMPORT		int			getMask() const;
	LC_IMPORT		void		setPathData(const LcTPlacement& a, const LcTPlacement& b, int mask, ENdhsDecoratorType type);

	LC_IMPORT		void		clearPath();

	LC_IMPORT		void		setLocation(const LcTVector& a, const LcTVector& b, ENdhsDecoratorType type);
	LC_IMPORT		void		setExtent(const LcTVector& a, const LcTVector& b, ENdhsDecoratorType type);
	LC_IMPORT		void		setScale(const LcTVector& a, const LcTVector& b, ENdhsDecoratorType type);
	LC_IMPORT		void		setOrientation(const LcTQuaternion& a, const LcTQuaternion& b, ENdhsDecoratorType type);
	LC_IMPORT		void		setOpacity(LcTScalar a, LcTScalar b, ENdhsDecoratorType type);
	LC_IMPORT		void		setColor(LcTColor& a, LcTColor& b, ENdhsDecoratorType type);
	LC_IMPORT		void		setFrame(int a, int b, ENdhsDecoratorType type);
	LC_IMPORT		void		setOffset(const LcTVector& a, const LcTVector& b, ENdhsDecoratorType type);
	LC_IMPORT		void		setIntensity(LcTScalar a, LcTScalar b, ENdhsDecoratorType type);
	LC_IMPORT		void		setColor2(LcTColor& a, LcTColor& b, ENdhsDecoratorType type);

	LC_IMPORT		void		setColorA(const LcTColor& a);
	LC_IMPORT		void		setColorB(const LcTColor& b, ENdhsDecoratorType type);

	LC_IMPORT		void		setColor2A(const LcTColor& a);
	LC_IMPORT		void		setColor2B(const LcTColor& b, ENdhsDecoratorType type);

	LC_IMPORT		NdhsCPath&	operator=(const NdhsCPath& path);

	LC_IMPORT		int			calcDecoratorFrameChange(int delta, LcTUnitScalar d);
};

#endif // NdhsCPathH
