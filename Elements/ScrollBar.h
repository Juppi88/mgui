/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Scrollbar.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI scrollbar related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 * 
 **********************************************************************/

#pragma once
#ifndef __MGUI_SCROLLBAR_H
#define __MGUI_SCROLLBAR_H

#include "Element.h"

enum SCROLLBAR_FLAGS {
	SCROLL_BUTTON1_HOVER	= 1 << 0,	// Button 1 is hovered
	SCROLL_BUTTON2_HOVER	= 1 << 1,	// Button 2 is hovered
	SCROLL_BUTTON1_PRESSED	= 1 << 2,	// Button 1 is pressed
	SCROLL_BUTTON2_PRESSED	= 1 << 3,	// Button 2 is pressed
	SCROLL_BAR_HOVER		= 1 << 4,	// Bar is hovered
	SCROLL_BAR_PRESSED		= 1 << 5,	// Bar is pressed/dragged
};

struct MGuiScrollbar {
	MGuiElement;					// Inherit all generic element properties

	rectangle_t		button1;		// Bounds of button 1 (up/right) 
	rectangle_t		button2;		// Bounds of button 2 (down/left)
	rectangle_t		bar;			// Bounds of the actual bar
	rectangle_t		background;		// Bounds of the background
	vectorscreen_t	click_offset;	// Click offset when dragging the bar
	colour_t		bg_colour;		// Scrollbar background colour
	float			bg_shade;		// Scrollbar background shade
	float			content_size;	// Size of the content
	float			step_size;		// Minimum step size when moving the scrollbar
	float			bar_position;	// The position of the scrollbar, relative to content_size
	float			bar_size;		// The relative size of the scrollbar (0...1)
	uint32			scroll_flags;	// Scrollbar flags (see enum above)
	uint32			nudge_time;		// Timestamp when the next nudge should happen
};

MGuiScrollbar*	mgui_create_scrollbar			( MGuiElement* parent );
MGuiScrollbar*	mgui_create_scrollbar_ex		( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col );

void			mgui_scrollbar_set_params		( MGuiScrollbar* scrollbar, float content, float step, float position, float size );
float			mgui_scrollbar_get_content_size	( MGuiScrollbar* scrollbar );
void			mgui_scrollbar_set_content_size	( MGuiScrollbar* scrollbar, float size );
float			mgui_scrollbar_get_step_size	( MGuiScrollbar* scrollbar );
void			mgui_scrollbar_set_step_size	( MGuiScrollbar* scrollbar, float size );
float			mgui_scrollbar_get_bar_pos		( MGuiScrollbar* scrollbar );
void			mgui_scrollbar_set_bar_pos		( MGuiScrollbar* scrollbar, float position );
float			mgui_scrollbar_get_bar_size		( MGuiScrollbar* scrollbar );
void			mgui_scrollbar_set_bar_size		( MGuiScrollbar* scrollbar, float size );
float			mgui_scrollbar_get_bg_shade		( MGuiScrollbar* scrollbar );
void			mgui_scrollbar_set_bg_shade		( MGuiScrollbar* scrollbar, float shade );

#endif /* __MGUI_SCROLLBAR_H */
