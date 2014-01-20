/**
 *
 * @file		Window.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI window related functions.
 *
 * @details		Functions and structures related to GUI windows.
 *
 **/

#pragma once
#ifndef __MGUI_WINDOW_H
#define __MGUI_WINDOW_H

#include "Element.h"

typedef struct MGuiTitlebar MGuiTitlebar;
typedef struct MGuiWindowButton MGuiWindowButton;

/**
 * @brief Window resize flags.
 * @details Internal flags used to determine the direction of a window resize event.
 */
enum WINDOW_RESIZE_FLAGS {
	RESIZE_NONE		= 0,		///< Window is not being resized
	RESIZE_HORIZ	= 1 << 0,	///< Window is being resized horizontally
	RESIZE_VERT		= 1 << 1,	///< Window is being resized vertically
};

/**
 * @brief GUI window.
 * @details A cotainer struct for a GUI window.
 */
struct MGuiWindow {
	MGuiElement;						///< Inherit MGuiElement members
	rectangle_t			window_bounds;	///< Bounding rectangle for the window, including its titlebar
	vectorscreen_t		min_size;		///< Minimum size the user can resize this window to
	vectorscreen_t		click_offset;	///< Click position offset when dragging this element
	vectorscreen_t		resize_rect;	///< Size of the resize rectangle
	uint8				resize_flags;	///< Resize flags (see @ref WINDOW_RESIZE_FLAGS)
	MGuiTitlebar*		titlebar;		///< Pointer to an internal window titlebar element
	MGuiWindowButton*	closebtn;		///< Pointer to an internal close button element
};

MGuiWindow*	mgui_create_window			( MGuiElement* parent );
MGuiWindow*	mgui_create_window_ex		( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );

void	mgui_window_get_title_colour	( MGuiWindow* window, colour_t* col );
void	mgui_window_set_title_colour	( MGuiWindow* window, const colour_t* col );
uint32	mgui_window_get_title_colour_i	( MGuiWindow* window );
void	mgui_window_set_title_colour_i	( MGuiWindow* window, uint32 hex );
void	mgui_window_get_drag_offset		( MGuiWindow* window, vectorscreen_t* pos );

#endif /* __MGUI_WINDOW_H */
