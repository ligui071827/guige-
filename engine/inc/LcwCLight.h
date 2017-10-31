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
#ifndef LcwCLightH
#define LcwCLightH


/*-------------------------------------------------------------------------*//**
	A simple, generic light widget
*/
class LcwCLight : public LcCWidget
{
	LC_DECLARE_RTTI(LcwCLight, LcCWidget)

public:

	typedef enum {EBulb, EDirectional} TLightType;

private:

	LcCLight*						m_light;

	TLightType						m_lightType;
	LcTScalar						m_attenuationConstant;
	LcTScalar						m_attenuationLinear;
	LcTScalar						m_attenuationQuadratic;
	LcTColor						m_specularColor;

	// LcCWidget interface
	LC_VIRTUAL		void			onRealize();
	LC_VIRTUAL		void			onPrepareForPaint() {};
	LC_VIRTUAL		void			onRetire();

LC_PRIVATE_INTERNAL_PROTECTED:

	// Allow only 2-phase construction
	LC_IMPORT						LcwCLight();

	// Redefined to stop bulbs being redrawn unless they change position.
	LC_VIRTUAL		void			setDirty()			{}

	// Inform widgets that they or their parents have changed position.
	// This means the lights are now actually dirty.
	LC_VIRTUAL		void			onPlacementChange(int mask)	{ LcCWidget::setDirty(); }
	LC_VIRTUAL		void			onParentPlacementChange()	{ LcCWidget::setDirty(); }

protected:
					void			setAttenuation(LcTScalar constant, LcTScalar linear, LcTScalar quadratic);
					void			setLightType(TLightType type);
					void			setSpecularColor(LcTColor color);

public:

	// Construction/destruction
	LC_IMPORT static LcTaOwner<LcwCLight> create();
	LC_VIRTUAL						~LcwCLight();

	// LcCWidget 
	LC_VIRTUAL		void			onPaint(const LcTPixelRect& clip);
	LC_VIRTUAL		void			setVisible(bool b);

	// Configuration
	LC_IMPORT		LcCLight*		getLight()		{ return m_light; }
	LC_IMPORT		void			setLight(LcCLight* light);
	LC_IMPORT		TLightType		getLightType()	{ return m_lightType; }
					void  			releaseResources();	
					void			reloadResources();
};

#endif // LcwCLightH
