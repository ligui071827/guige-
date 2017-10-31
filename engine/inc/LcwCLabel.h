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
#ifndef LcwCLabelH
#define LcwCLabelH

#include "inflexionui/engine/inc/LcTPlaneRect.h"
class LcCFont;
class LcCMarqueeRenderer;

/*-------------------------------------------------------------------------*//**
	A simple, generic label widget. The text is shown aligned as specified
	against the location point.  The horizontal scale determines the word wrap
	width if wrapping, otherwise it is ignored. The vertical scale determines
	the height of one line.
*/
class LcwCLabel : public LcCWidget
{
	LC_DECLARE_RTTI(LcwCLabel, LcCWidget)

public:

	enum EHorizontalAlign
	{
		HALIGN_LEFT,
		HALIGN_CENTER,
		HALIGN_RIGHT
	};

	enum EVerticalAlign
	{
		VALIGN_TOP,
		VALIGN_CENTER,
		VALIGN_BOTTOM
	};

	enum EMarquee
	{
		NOTMARQUEE	= 0x0,
		HORIZONTAL	= 0x1,
		VERTICAL	= 0x2
	};

	enum ETextDirection
	{
		LEFT_TO_RIGHT	= 0x0,
		RIGHT_TO_LEFT	= 0x1
	};

private:

	// Single wrapped line
	struct TLine
	{
		LcTmString		m_text;
		LcTmOwner<LcCText>	m_tobj;

					TLine(const LcTmString& s, LcCFont* f);
		inline		TLine() {}
	};

	LcCFont*								m_font;

	EMarquee								m_marquee;
	LcTmOwner<LcCMarqueeRenderer>			m_marqueeRenderer;
	EMarquee								m_activeMarquee;
	ETextDirection							m_textDirection;
	LcTScalar								m_textWidth;
	LcTScalar								m_textHeight;
	bool									m_avoidMarqueeAnimation;
	int										m_initialDelta;

	// Configured info
	LcTmString								m_text;
	LcTScalar								m_fontHeight;
	EHorizontalAlign  						m_halign;
	EVerticalAlign							m_valign;
	LcTmString								m_abbrevSuffix;
	bool									m_antiAlias;
	int										m_meshGridX;
	int										m_meshGridY;

	// Cached intermediate info
	LcTPlaneRect							m_bbox;
	LcTmArray<TLine>						m_lines;

	// Cached info
	bool									m_inCache;

	//Caret related info
	int										m_caretPosition;

	// LcCWidget methods
	LC_VIRTUAL			void				onRealize();
	LC_VIRTUAL			void				onFontUpdated();
	LC_VIRTUAL			bool				contains(const LcTVector& loc, LcTScalar expandRectEdge);


	// Helpers
						void				wrapLines();
						void				calcCaretPosition(int& caretLine, int& caretPos);

	virtual				bool				canBeClipped()  { return true; }
	LC_VIRTUAL			void				onPaint(const LcTPixelRect& clip);

	LC_VIRTUAL			void				onWantBoundingBox();

protected:
						void				setAntiAlias(bool antiAlias)	{ m_antiAlias = antiAlias; }

	// LcCWidget method - for access by derived classes e.g. LcwCFrameRateLabel
	LC_VIRTUAL			void				onPrepareForPaint();

	LC_VIRTUAL			void				onPlacementChange(int mask);


	// Allow only 2-phase construction and derivation
	LC_IMPORT								LcwCLabel();

public:

	// Construction/Destruction
	LC_IMPORT			static LcTaOwner<LcwCLabel> create();
	LC_VIRTUAL						    			~LcwCLabel();

	// Configuration
	// Font
	LC_IMPORT			void				setFontHeight(LcTScalar fontHeight);
	LC_IMPORT			LcTScalar			getFontHeight() 		{ return m_fontHeight; }
	LC_IMPORT			LcCFont*			getFont();
	LC_IMPORT			LcTaString 			getFontName();
	LC_IMPORT			LcCFont::EStyle 	getFontStyle();
	LC_IMPORT			LcTColor			getFontColor();
	LC_IMPORT			LcTColor			getFontColorDisabled();
	LC_IMPORT			bool				updateFont();

	// Accessors for the properties.
	LC_IMPORT			void				setCaption(const LcTmString& s);
	inline				LcTaString			getCaption() 			{ return m_text; }
	LC_IMPORT			void				setCaretPosition(int pos);
	inline				int					getCaretPosition()		{ return -1; }
	LC_IMPORT			void				setHorizontalAlign(EHorizontalAlign i);
	inline				EHorizontalAlign	getHorizontalAlign() 	{ return m_halign; }
	LC_IMPORT			void				setVerticalAlign(EVerticalAlign i);
	inline				EVerticalAlign		getVerticalAlign() 		{ return m_valign; }
	LC_IMPORT			void				setAbbrevSuffix(const LcTmString& s);
	inline				LcTaString			getAbbrevSuffix()		{ return m_abbrevSuffix; }
						void				setMeshGrid(int x, int y);

						bool				isAntiAliased()			{ return m_antiAlias; }

	virtual 			void  				releaseResources();
	virtual 			void  				reloadResources();

	inline				bool				isHorizontalMarqueeEnabled() { return m_activeMarquee == HORIZONTAL; }
	inline				bool				isVerticalMarqueeEnabled() { return m_activeMarquee == VERTICAL; }
	inline				bool				isTextRTL() { return m_textDirection == RIGHT_TO_LEFT; }
	inline				LcTScalar			getTextWidth() { return m_textWidth; }
	inline				LcTScalar			getTextHeight() { return m_textHeight; }
						void				setMarquee(bool vMarquee, bool hMarquee);
						void				setDelta(int delta);
						void				resetDelta()			{ setDelta(m_initialDelta); }
};

#endif
