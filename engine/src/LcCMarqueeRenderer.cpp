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
void LcCMarqueeRenderer::construct(LcwCLabel::EMarquee marquee,
									LcTPlaneRect bbox,
									LcTScalar scaleFactor)
{
	m_bbox = bbox;
	m_marquee = marquee;
	m_currentDelta = 0;
	m_wrap = false;
	m_currentLine = 0;
	m_marqueePositive = false;
	m_scaleFactor = scaleFactor;

	if (m_marqueePositive)
	{
		m_currentIncrement = 1;
	}
	else
	{
		m_currentIncrement = -1;
	}
}

/*-------------------------------------------------------------------------*//**
*/
LcTaOwner<LcCMarqueeRenderer> LcCMarqueeRenderer::create(LcwCLabel::EMarquee marquee,
															LcTPlaneRect bbox,
															LcTScalar scaleFactor)
{
	LcTaOwner<LcCMarqueeRenderer> ref;
	ref.set(new LcCMarqueeRenderer);
	ref->construct(marquee, bbox, scaleFactor);
	return ref;
}

/*-------------------------------------------------------------------------*//**
*/
LcTPlaneRect LcCMarqueeRenderer::getDestinationRect(LcTScalarRect sourceRect, LcTPlaneRect destRect)
{
	LcTPlaneRect output = destRect;

	if(m_marquee == LcwCLabel::HORIZONTAL)
	{
		LcTScalar leftX = destRect.getLeft();
		LcTScalar RightX = destRect.getRight();

		LcTScalar bboxLeftX = m_bbox.getLeft();
		LcTScalar bboxRightX = m_bbox.getRight();

		LcTScalar diffLeft = (leftX + m_currentDelta);

		if(diffLeft < bboxLeftX)
		{
			diffLeft = bboxLeftX;	
		}

		output.setLeft(diffLeft);

		LcTScalar diffRight = RightX + m_currentDelta;

		if(diffRight > bboxRightX)
		{
			diffRight = bboxRightX;
		}
		output.setRight(diffRight);
	}
	else if (m_marquee == LcwCLabel::VERTICAL)
	{
		LcTScalar leftX = destRect.getTop();
		LcTScalar RightX = destRect.getBottom();

		LcTScalar bboxLeftX = m_bbox.getTop();
		LcTScalar bboxRightX = m_bbox.getBottom();

		LcTScalar diffLeft = (leftX - m_currentDelta);

		if(diffLeft > bboxLeftX)
		{
			diffLeft = bboxLeftX;	
		}

		output.setTop(diffLeft);

		LcTScalar diffRight = RightX - m_currentDelta;

		if(diffRight < bboxRightX)
		{
			diffRight = bboxRightX;
		}
		output.setBottom(diffRight);
	}

	return output;
}

/*-------------------------------------------------------------------------*//**
*/
void LcCMarqueeRenderer::setDelta(int delta)
{	
	m_currentDelta = delta;
}

/*-------------------------------------------------------------------------*//**
*/
LcTScalarRect LcCMarqueeRenderer::getSourceRect(LcTScalarRect sourceRect, LcTPlaneRect destRect)
{
	LcTScalarRect output = sourceRect;

	// NB - destRect, m_bbox and m_currentDelta are not scaled by any theme scale factor,
	// but source rect is...we must convert destRect and m_bbox points into the same
	// scaled space as source rect before clipping 'output'
	if (m_marquee == LcwCLabel::HORIZONTAL)
	{
		LcTScalar scaledDestLeft = (destRect.getLeft() + m_currentDelta) * m_scaleFactor;
		LcTScalar scaledBboxLeft = m_bbox.getLeft() * m_scaleFactor;
		LcTScalar scaledBboxRight = m_bbox.getRight() * m_scaleFactor;

		// Clip left edge of output rect
		LcTScalar leftClip = scaledBboxLeft - scaledDestLeft;
		if (leftClip > output.getLeft())
			output.setLeft(leftClip);

		// Clip right edge of output rect
		LcTScalar rightClip = scaledBboxRight - scaledDestLeft;
		if (rightClip < output.getRight())
			output.setRight(rightClip);
	}	
	else if (m_marquee == LcwCLabel::VERTICAL)
	{
		LcTScalar scaledDestTop = (destRect.getTop() - m_currentDelta) * m_scaleFactor;
		LcTScalar scaledBboxTop = m_bbox.getTop() * m_scaleFactor;

		// Clip top edge of output rect
		LcTScalar topClip = scaledDestTop - scaledBboxTop;
		if (topClip > output.getTop())
			output.setTop(topClip);

		if (output.getTop() >= output.getBottom())
		{
			// Rect will be invisible - clip away totally
			output.setTop(output.getBottom());
		}
		else
		{
			LcTScalar scaledBboxBottom = m_bbox.getBottom() * m_scaleFactor;

			// Clip bottom edge of output rect
			LcTScalar bottomClip = scaledDestTop - scaledBboxBottom;
			if (bottomClip < output.getBottom())
				output.setBottom(bottomClip);
		}
	}

	return output;
}
