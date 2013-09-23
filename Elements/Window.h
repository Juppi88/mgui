/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Window.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI window related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_WINDOW_H
#define __MGUI_WINDOW_H

#include "Element.h"

typedef struct MGuiTitlebar MGuiTitlebar;
typedef struct MGuiWindowButton MGuiWindowButton;

enum WINDOW_RESIZE_FLAGS {
	RESIZE_NONE		= 0,
	RESIZE_HORIZ	= 1 << 0,
	RESIZE_VERT		= 1 << 1,
};

struct MGuiWindow {
	MGuiElement;						// Inherit all generic element properties
	rectangle_t			window_bounds;	// Window boundaries including the titlebar
	vectorscreen_t		min_size;		// Minimum size the user can resize this window to
	vectorscreen_t		click_offset;	// Position offset when dragging this element
	vectorscreen_t		resize_rect;	// Size of the resize rectangle
	uint8				resize_flags;	// Resize flags (see enum above)
	MGuiTitlebar*		titlebar;		// Window titlebar
	MGuiWindowButton*	closebtn;		// Close button
};

MGuiWindow*		mgui_create_window				( MGuiElement* parent );
MGuiWindow*		mgui_create_window_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );

void			mgui_window_get_title_col		( MGuiWindow* window, colour_t* col );
void			mgui_window_set_title_col		( MGuiWindow* window, const colour_t* col );
uint32			mgui_window_get_title_col_i		( MGuiWindow* window );
void			mgui_window_set_title_col_i		( MGuiWindow* window, uint32 hex );
void			mgui_window_get_drag_offset		( MGuiWindow* window, vectorscreen_t* pos );

#endif /* __MGUI_WINDOW_H */
