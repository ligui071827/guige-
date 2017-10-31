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
#ifndef LcCMarqueeRendererH
#define LcCMarqueeRendererH

#include "inflexionui/engine/inc/LcTPlaneRect.h"
class LcCFont;

class LcCMarqueeRenderer : public LcCBase
{
private:
	LcTPlaneRect											m_bbox;
	LcwCLabel::EMarquee										m_marquee;
	int														m_currentDelta;
	bool													m_wrap;
	int														m_currentLine;
	bool													m_marqueePositive;
	int														m_currentIncrement;
	LcTScalar												m_scaleFactor;

						void								construct(LcwCLabel::EMarquee marquee,
																		LcTPlaneRect bbox,
																		LcTScalar scaleFactor);
protected:
															LcCMarqueeRenderer() {}
public:
	// Construction
	static				LcTaOwner<LcCMarqueeRenderer>		create(LcwCLabel::EMarquee marquee,
																		LcTPlaneRect bbox,
																		LcTScalar scaleFactor);

						LcTPlaneRect						getDestinationRect(LcTScalarRect sourceRect, LcTPlaneRect destRect);

						LcTScalarRect						getSourceRect(LcTScalarRect sourceRect, LcTPlaneRect destRect);

						void								setDelta(int delta);


						int									getCurrentLine() { return m_currentLine; }
						void								setCurrentLine(int line) { m_currentLine = line; }
};

#endif
