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

struct MGuiWindow
{
	MGuiElement;						// Inherit all generic element properties

	rectangle_t			window_bounds;	// Window boundaries including the titlebar
	vectorscreen_t		click_offset;	// Position offset when dragging this element
	MGuiTitlebar*		titlebar;		// Window titlebar
	MGuiWindowButton*	closebtn;		// Close button
};

MGuiWindow*		mgui_create_window				( MGuiControl* parent );

bool			mgui_window_get_closebtn		( MGuiWindow* window );
void			mgui_window_set_closebtn		( MGuiWindow* window, bool enabled );
bool			mgui_window_get_titlebar		( MGuiWindow* window );
void			mgui_window_set_titlebar		( MGuiWindow* window, bool enabled );
void			mgui_window_get_title_col		( MGuiWindow* window, colour_t* col );
void			mgui_window_set_title_col		( MGuiWindow* window, const colour_t* col );
uint32			mgui_window_get_title_col_i		( MGuiWindow* window );
void			mgui_window_set_title_col_i		( MGuiWindow* window, uint32 hex );
void			mgui_window_get_drag_offset		( MGuiWindow* window, vectorscreen_t* pos );

#endif /* __MGUI_WINDOW_H */
