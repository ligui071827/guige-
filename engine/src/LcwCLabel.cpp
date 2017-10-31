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

#include "inflexionui/engine/inc/LcAll.h"
#ifdef LC_HDRSTOP
	#pragma hdrstop
#endif


/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTaOwner<LcwCLabel> LcwCLabel::create()
{
	LcTaOwner<LcwCLabel> ref;
	ref.set(new LcwCLabel);
	ref->construct();
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcwCLabel::LcwCLabel()
{
	m_fontHeight	= -1;
	m_halign		= HALIGN_CENTER;
	m_valign		= VALIGN_CENTER;
	m_caretPosition = -1;
	m_avoidMarqueeAnimation = false;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcwCLabel::~LcwCLabel()
{
	if (m_font)
		m_font->release();

	if (m_marqueeRenderer)
	{
		m_marqueeRenderer.destroy();
		m_activeMarquee = NOTMARQUEE;

		if (!m_avoidMarqueeAnimation)
		{
			LcTWidgetEvent ev(0);
			fireWidgetEvent(&ev);
		}
	}
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCLabel::onRealize()
{
	getSpace()->addWidget(this);

	if (m_fontHeight < 0)
	{
		// Make the height of the extent the default height
		m_fontHeight = getInternalExtent().y;
	}

	// Split lines if required
	wrapLines();
}



/*-------------------------------------------------------------------------*//**
*/
void LcwCLabel::releaseResources()
{
	m_lines.clear();
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCLabel::reloadResources()
{
	wrapLines();
#if	defined(IFX_RENDER_DIRECT_OPENGL_20)
	configEffectUniforms(getSpace(),"");
#endif
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCLabel::onPrepareForPaint()
{

	// Don't bother if we have no font configured
	if (!m_font && !updateFont())
		return;

	LcTVector extent = getInternalExtent();
	LcTVector location = LcTVector();

	int caretLine;
	int caretPos;

	calcCaretPosition(caretLine, caretPos);

	// Do not go through the motions if we are already rendered correctly.
	if (m_inCache)
		return;

	m_activeMarquee = NOTMARQUEE;

	m_textDirection = LEFT_TO_RIGHT;

	// Number of lines, wrapped or not
	int nlines = (int)m_lines.size();
	LcTScalar newHeight = nlines * m_fontHeight;

	// Align text to integer coords if font height is given as integral
	// NB: in some circumstances this alignment might be too extreme,
	// e.g. if using scaled coordinates where the height is 1.0
	bool bPixelAlignment = (m_fontHeight == int(m_fontHeight));

	// If pixel-aligning text, round height up to the nearest even number
	if (bPixelAlignment)
		newHeight = (LcTScalar)(((int(1 + newHeight)) / 2) * 2);

	if(m_marquee != VERTICAL || extent.y >= newHeight)
	{
		// Calculate Y of top edge of top line, to achieve requested valign
		// NOTE: The label bounding box (m_bbox) has its Y-axis increasing
		// downwards unlike all other graphics.
		// This means we use its top left as the origin.
		if (m_valign == VALIGN_TOP)
			m_bbox.setBottom((extent.y / 2) - newHeight + location.y);
		else if (m_valign == VALIGN_CENTER)
			m_bbox.setBottom(-newHeight / 2 + location.y);
		else
			m_bbox.setBottom(-(extent.y / 2) + location.y);

		// Set the bottom, remember Y increases upwards here.
		m_bbox.setTop(m_bbox.getBottom() + newHeight + location.y);
	}
	else
	{
		m_bbox.setBottom(-(extent.y / 2) + location.y);
		m_bbox.setTop((extent.y / 2) + location.y);

		m_activeMarquee = VERTICAL;
	}
	m_textHeight = newHeight;


	// Defaults
	m_bbox.setLeft(extent.x / 2  + location.x);
	m_bbox.setRight(m_bbox.getLeft() + location.x);
	LcTScalar maxX	= -extent.x/2;

	IFX_INT32 rtl = -1;
	m_textWidth = 0;

	// Calc x-positions for each line
	for (int i = 0; i < nlines; i++)
	{
		// Calc width of text
		LcTScalar w = m_font->getTextWidth(m_fontHeight, m_lines[i].m_text);

		// Make sure we have at least width 1 if it is a caret line
		if (caretLine == i && w < 1)
		{
			w = 1;
		}

		int roundedW = (w == int(w)) ? (int)w : (int)w + 1;

		// Calc x position of text
		LcTScalar x = location.x;
		if(m_marquee != HORIZONTAL || extent.x >= roundedW)
		{
			// Text is always horizontally aligned as an integer offset from
			// the edge of the extent - it is up to the theme designer to ensure
			// that the left edge of the extent is on a pixel boundary when the
			// label is stationary
			if (m_halign == HALIGN_LEFT)
				x -= (extent.x / 2);
			else if (m_halign == HALIGN_CENTER)
				x -= bPixelAlignment ? (roundedW / 2) : (w/2);
			else
				x -= bPixelAlignment ? (roundedW - (extent.x / 2)) : (w - (extent.x / 2)) ;

			// Find out the rightmost X position. This will be used later to
			// work out the width of the bounding box.
			maxX = max(maxX, (x + w));
		}
		else
		{
			x -= (extent.x / 2);
			maxX = max(maxX, (extent.x / 2));
			m_activeMarquee = HORIZONTAL;
		}
		m_textWidth = max(m_textWidth, w);

		// Adjust bbox
		m_bbox.setLeft(min(m_bbox.getLeft(), x));

		// Get X, Y position relative to widget (z=0)
		LcTVector vLoc;
		vLoc.x = x;

		if (m_activeMarquee == VERTICAL)
		{
			vLoc.y = m_bbox.getTop() - (i + 1) * m_fontHeight;
		}
		else
		{
			vLoc.y = m_bbox.getBottom() + ((nlines - 1) - i) * m_fontHeight;
		}
		vLoc.z = location.z;

		// Render the text - effectively caches X position too
		m_lines[i].m_tobj->render(
			vLoc,
			m_fontHeight,
			m_lines[i].m_text,
			caretLine == i ? caretPos : -1,
			&rtl);

		if (i==0 && rtl == 1)
		{
			m_textDirection = RIGHT_TO_LEFT;
		}
	}

	m_bbox.setRight(maxX);
	m_bbox.setZDepth(location.z);

	if(m_activeMarquee != NOTMARQUEE)
	{
		LcTScalar screenWidth = 0;
		LcTScalar globalWidth = 0;
		LcTScalar scaleFactor = 1;

		if (getSpace())
		{
			screenWidth = (LcTScalar)getSpace()->getCanvasBounds().getWidth();
			globalWidth = getSpace()->getGlobalExtent().x;
		}
		if (globalWidth > 0)
		{
			scaleFactor = screenWidth / globalWidth;
		}

		m_marqueeRenderer = LcCMarqueeRenderer::create(m_marquee, m_bbox, scaleFactor);
	}

	if (m_marqueeRenderer)
	{
		if (m_textDirection == RIGHT_TO_LEFT)
		{
			m_initialDelta = (int)(m_bbox.getWidth() - m_textWidth);
			setDelta(m_initialDelta);
		}
		else
		{
			m_initialDelta = 0;
		}

		if (!m_avoidMarqueeAnimation)
		{
			LcTWidgetEvent ev(0);
			fireWidgetEvent(&ev);
		}
	}

	m_inCache = true;
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCLabel::wrapLines()
{
	// Can't do anything unless shown as we have no font or metrics
	if (!getSpace())
		return;

	m_inCache = false;

	if (m_marqueeRenderer)
	{
		m_marqueeRenderer.destroy();
		m_activeMarquee = NOTMARQUEE;

		if (!m_avoidMarqueeAnimation)
		{
			LcTWidgetEvent ev(0);
			fireWidgetEvent(&ev);
		}
	}

	// Bin old text
	m_lines.clear();

	// Find out what font/size we'll be using
	if (!m_font && !updateFont())
		return;

	// Determine the maximum number of lines we can fit within this extent
	LcTVector extent   = getInternalExtent();
	int maxLines = (int)(extent.y / m_fontHeight);
	if (maxLines == 0)
		return;

	// Wrap the text - note that the units of height/width are not defined;
	// however, they are expected to be in the same units
	LcTaArray<LcTmString> vs = m_font->wrapLines(getFontHeight(), m_text, extent.x, m_marquee == HORIZONTAL && m_font->isNativeFont());

	// Build a vector of TLines (these include the x-position too)
	// If m_abbrevSuffix is set and we don't have enough room to show
	// all the lines, we append the suffix to the last visible line
	bool bCut = m_abbrevSuffix.length() > 0 && maxLines < (int)vs.size();

	if (m_marquee == VERTICAL && m_font->isNativeFont())
	{
		bCut = false;
		maxLines = (int)vs.size();
	}

	for (int i = 0; i < min(maxLines, (int)vs.size()); i++)
	{
		// Append suffix to final line if required
		if (bCut && i == maxLines - 1)
		{
			int lenSuffix = (int)m_font->getTextWidth(getFontHeight(), m_abbrevSuffix);

			LcTaString s = vs[i];
			vs[i] = s.subString(0, m_font->getTextLengthToFit(getFontHeight(), s, extent.x - lenSuffix));
			vs[i] = vs[i] + m_abbrevSuffix;
		}

		// Store a line record, which includes the rendered text object, although we
		// can't render yet because we don't necessarily know label position
		m_lines.push_back(TLine(vs[i], m_font));
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcwCLabel::TLine::TLine(const LcTmString& s, LcCFont* f)
: m_text(s)
{
	// Create a text object to be rendered by the font
	LcTaOwner<LcCText> newTobj = f->createText();
	m_tobj = newTobj;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCLabel::onWantBoundingBox()
{
	LcCSpace* sp = getSpace();

	if (m_tappable == EFull)
	{
		LcTScalar xDiv2		= getExtent().x/2;
		LcTScalar yDiv2		= getExtent().y/2;

		m_bbox  = LcTPlaneRect (-xDiv2, yDiv2, getExtent().z, xDiv2, -yDiv2);
	}
	sp->extendBoundingBox(m_bbox);
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCLabel::setDelta(int delta)
{
	if (m_marqueeRenderer)
	{
		m_marqueeRenderer->setDelta(delta);
	}
}

/*-------------------------------------------------------------------------*//**
	Should be hidden
*/
LC_EXPORT_VIRTUAL void LcwCLabel::onPaint(const LcTPixelRect& clip)
{
	LcTScalar opacity = getOverallOpacity();

	if (!m_font && !updateFont())
		return;

	if (opacity < LC_TRANSPARENT_THRESHOLD)
		return;

	if (opacity > LC_OPAQUE_THRESHOLD)
		opacity = 1;

#if defined(LC_PLAT_OGL)
	bool useDepthBuffer = m_meshGridX > 1 || m_meshGridY > 1;

	// Only use depth buffer for mesh grid cases
	if (useDepthBuffer)
		getSpace()->getOglContext()->setDepthMask(GL_TRUE);
#endif

	// Iterate wrapped lines
	int size = (int)m_lines.size();
	for (int i = 0; i < size; i++)
	{
		if(m_marqueeRenderer)
		{
			m_marqueeRenderer->setCurrentLine(i);
		}
		// Draw each at cached X and stepped Y
		m_lines[i].m_tobj->draw(
			clip,
			getFontColor(),
			getOverallOpacity(),
			m_antiAlias,
			m_meshGridX,
			m_meshGridY,
			m_marqueeRenderer.ptr());

	}

#if defined(LC_PLAT_OGL)
	if (useDepthBuffer)
		getSpace()->getOglContext()->setDepthMask(GL_FALSE);
#endif
}

/*-------------------------------------------------------------------------*//**
	Sets the height of one line of text within the label.
	The units are the same as those used to specify vertical extent.
	This is required because line wrapping may allow the widget to show
	several lines of text within its vertical extent.
	If set to -1, one line is shown, occupying the full vertical extent of
	the widget.
*/
LC_EXPORT void LcwCLabel::setFontHeight(LcTScalar fontHeight)
{
	m_fontHeight = fontHeight;

	// If already realized, apply default
	if (m_fontHeight < 0 && getSpace())
		m_fontHeight = getInternalExtent().y;

	// Recalculate line splits
	wrapLines();
	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcwCLabel::setCaption(const LcTmString& s)
{
	if (m_text.compare(s) == 0)
		return;

	m_text = s;

	// Recalculate line splits
	wrapLines();
	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcwCLabel::setAbbrevSuffix(const LcTmString& s)
{
	m_abbrevSuffix = s;

	// Recalculate line splits
	wrapLines();
	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcwCLabel::setHorizontalAlign(EHorizontalAlign i)
{
	m_halign = i;
	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcwCLabel::setVerticalAlign(EVerticalAlign i)
{
	m_valign = i;
	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCLabel::calcCaretPosition(int& caretLine, int& caretPos)
{
	// -1 to disable caret
	caretLine			= -1;
	caretPos			= -1;

	if (m_caretPosition <= -1 ||
		m_caretPosition > m_text.length())
		return;

	int size				= (int)m_lines.size();
	int lineLength			= 0;
	int charCount			= 0;
	int lineStartCharPos	= 0;
	int l_caretPos			= m_caretPosition;

	// determine which line has caret
	for (int i = 0; i < size; i++)
	{
		lineLength	    	= m_lines[i].m_text.length();
		lineStartCharPos	= charCount;
		charCount	   		+= lineLength;

		if (l_caretPos <= lineLength )
		{
			// Caret is within i line
			caretLine   = i;
			caretPos    = l_caretPos;
			break;
		}
		else
		{
			l_caretPos -= lineLength;

			// Account for the space striped at the start of the line during
			// line wrapping
			if (m_text[lineStartCharPos] == L' ')
				l_caretPos--;

			// Account for the space striped at the end of the line during
			// line wrapping
			if (m_text[charCount] == L' ')
				l_caretPos--;

		}
	}
}
/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCLabel::onFontUpdated()
{
	updateFont();
	wrapLines();
}

/*-------------------------------------------------------------------------*//**
	Return whether the image is at the specified point.

*/
LC_EXPORT_VIRTUAL bool LcwCLabel::contains(const LcTVector& loc, LcTScalar expandRectEdge)
{
	// If text is configured as not tappable, don't tap
	if (m_tappable == EOff)
		return false;

	// If tappable is full then use the tap tolerance setting.
	if (m_tappable == EFull)
		return  m_bbox.contains(loc, expandRectEdge);
	else
		return  m_bbox.contains(loc);
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT_VIRTUAL void LcwCLabel::onPlacementChange(int mask)
{
	if (mask & LcTPlacement::EExtent)
	{
		// We only want to re-calculate the line splits if
		// the extent has changed.
		wrapLines();
	}

	revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT void LcwCLabel::setCaretPosition(int pos)
{
	bool needsRevalidate = false;

	if (m_caretPosition != pos)
	{
		m_inCache = false;
		needsRevalidate = true;
	}

	m_caretPosition = pos;

	if (needsRevalidate)
		revalidate();
}

/*-------------------------------------------------------------------------*//**
*/
LcTaString LcwCLabel::getFontName()
{
	// Check for override on widget
	LcTaString value(LcCWidget::getFontName());
	if (value.length() > 0)
		return value;

	// Otherwise, search up widget hierarchy to applet and LAF
	return findFontName();
}

/*-------------------------------------------------------------------------*//**
*/
LcCFont::EStyle LcwCLabel::getFontStyle()
{
	// Check for override on widget
	LcCFont::EStyle value(LcCWidget::getFontStyle());
	if (value > -1)
		return value;

	// Otherwise, search up widget hierarchy to applet and LAF
	return findFontStyle();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTColor LcwCLabel::getFontColor()
{
	// Check for override on widget
	LcTColor value(LcCWidget::getFontColor());
	if (value != LcTColor::NONE)
		return value;

	// Otherwise, search up widget hierarchy to applet and LAF
	return findFontColor();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcTColor LcwCLabel::getFontColorDisabled()
{
	// Otherwise, search up widget hierarchy to applet and LAF
	return findFontColorDisabled();
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT LcCFont* LcwCLabel::getFont()
{
	if (!m_font)
		updateFont();

	return m_font;
}

/*-------------------------------------------------------------------------*//**
*/
LC_EXPORT bool LcwCLabel::updateFont()
{
	LcCApplet* ap(getApplet());
	if (!ap)
		return false;

	// Delete the old font
	if (m_font)
		m_font->release();

	// Create a new font object
	m_font = ap->getFont(getFontName(), getFontStyle());

	// Acquire the new font
	if (m_font) {
		m_font->acquire();
		return true;
	}

	return false;
}

/*-------------------------------------------------------------------------*//**
																			 */
void LcwCLabel::setMarquee(bool vMarquee, bool hMarquee)
{
	EMarquee prev = m_marquee;

	m_marquee = NOTMARQUEE;

	if (m_avoidMarqueeAnimation
		&& prev == HORIZONTAL)
	{
		prev = NOTMARQUEE;
	}

	m_avoidMarqueeAnimation = false;

	if (hMarquee)
	{
		m_marquee = HORIZONTAL;
	}
	else if (vMarquee)
	{
		m_marquee = VERTICAL;
	}
	else if (m_abbrevSuffix.length() == 0)
	{
		m_marquee = HORIZONTAL;
		m_avoidMarqueeAnimation = true;
	}

	if(m_marquee != prev)
	{
		// Recalculate line splits
		wrapLines();
		revalidate();
	}
}

/*-------------------------------------------------------------------------*//**
*/
void LcwCLabel::setMeshGrid(int x, int y)
{
	m_meshGridX = max(1, x);
	m_meshGridY = max(1, y);
}
