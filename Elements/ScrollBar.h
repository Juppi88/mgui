/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		ScrollBar.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI scrollbar related functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 * 
 **********************************************************************/

#pragma once
#ifndef __MGUI_SCROLLBAR_H
#define __MGUI_SCROLLBAR_H

#include "Element.h"

// Scrollbar flags
#define SBFLAG_B1HOVER		0x0001		// Button 1 is hovered
#define SBFLAG_B2HOVER		0x0002		// Button 2 is hovered
#define SBFLAG_B1PRESSED	0x0004		// Button 1 is pressed
#define SBFLAG_B2PRESSED	0x0008		// Button 2 is pressed
#define SBFLAG_BARHOVER		0x0010		// Bar is hovered
#define SBFLAG_BARPRESSED	0x0020		// Bar is pressed/dragged
#define SBFLAG_HORIZONTAL	0x0040		// Scrollbar is horizontal

struct scrollbar_s
{
	element_t;						// Inherit all generic element properties

	rectangle_t		button1;		// Bounds of button 1 (up/right) 
	rectangle_t		button2;		// Bounds of button 2 (down/left)
	rectangle_t		bar;			// Bounds of the actual bar
	vectorscreen_t	click_offset;	// Click offset when dragging the bar
	float			bar_position;	// The position of the scrollbar (0...1)
	float			bar_size;		// The relative size of the scrollbar (0...1)
	float			nudge_amount;	// Amount to move the bar when a button is clicked
	colour_t		track_col;		// Scrollbar track colour
	uint32			sbflags;		// Scrollbar flags
};

#endif /* __MGUI_SCROLLBAR_H */
