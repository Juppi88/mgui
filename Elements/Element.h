/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Element.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A base for all real-world GUI elements.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_ELEMENT_H
#define __MGUI_ELEMENT_H

#include "MGUI.h"
#include "Text.h"
#include "Control.h"
#include "Input/Input.h"
#include "Types/Types.h"

/* The following are internal flags and should not be used by the library user */
enum MGUI_INTERNAL_FLAGS
{
	FLAG_TABSTOP	= 0x00010000,	/* Tab press can change focus to this element */
	FLAG_MOUSECTRL	= 0x00020000,	/* Allow mouse input */
	FLAG_KBCTRL		= 0x00040000,	/* Allow keyboard input */
	FLAG_DRAGGABLE	= 0x00080000,	/* This element can be dragged */
	FLAG_HOVER		= 0x00100000,	/* Element is hovered over by a mouse cursor */
	FLAG_PRESSED	= 0x00200000,	/* Element is pressed down */
	FLAG_TITLEBAR	= 0x00400000,	/* Enable window titlebar */
	FLAG_CLOSEBTN	= 0x00800000,	/* Enable window close button */
	FLAG_MASKINPUT	= 0x01000000,	/* Mask user input */
	FLAG_TOPBOTTOM	= 0x02000000,	/* Memobox order is top to bottom */
	FLAG_INACTIVE	= 0x10000000,	/* This element is inactive (=background) */
	FLAG_FOCUS		= 0x20000000,	/* This element has focus */
	FLAG_ELEMENT	= 0x40000000,	/* This is an element (not a control) */
};

enum MGUI_TYPE
{
	GUI_NONE,
	GUI_BUTTON,
	GUI_CHECKBOX,
	GUI_DROPLIST,
	GUI_EDITBOX,
	GUI_GRIDLIST,
	GUI_LABEL,
	GUI_MEMOBOX,
	GUI_MENUBAR,
	GUI_PROGRESSBAR,
	GUI_RADIOBUTTON,
	GUI_SCROLLBAR,
	GUI_SLIDER,
	GUI_SPRITE,
	GUI_TAB,
	GUI_TITLEBAR,
	GUI_WINDOW,
	GUI_WINDOWBUTTON,
};

// Internal element types
MGUI_ELEMENT_DECL( titlebar_t );
MGUI_ELEMENT_DECL( windowbutton_t );

struct element_s
{
	control_t;								// Inherit control data
	enum MGUI_TYPE			type;			// The type of this GUI element
	vector2_t				pos;			// Relative position (within parent element)
	vector2_t				size;			// Relative size (within parent element)
	colour_t				colour;			// Element colour
	text_t*					text;			// Text on this element (label, title etc)
	font_t*					font;			// Default font used for all the text on this element
	mgui_event_handler_t	event_handler;	// User event handler callback
	void*					event_data;		// User-specified data to be passed via event_handler

	// Internal callbacks
	struct
	{
		void	( *render )				( element_t* element );					// Render callback
		void	( *destroy )			( element_t* element );					// Destructor callback
		void	( *process )			( element_t* element, uint32 ticks );	// Element processing

		void	( *on_bounds_update )	( element_t* element, bool pos, bool size );
		void	( *on_colour_update )	( element_t* element );
		void	( *on_text_update )		( element_t* element );
		void	( *on_mouse_enter )		( element_t* element );
		void	( *on_mouse_leave )		( element_t* element );
		void	( *on_mouse_click )		( element_t* element, MOUSEBTN button, uint16 x, uint16 y );
		void	( *on_mouse_release )	( element_t* element, MOUSEBTN button, uint16 x, uint16 y );
		void	( *on_mouse_drag )		( element_t* element, uint16 x, uint16 y );
		void	( *on_mouse_wheel )		( element_t* element, float diff );
		void	( *on_character )		( element_t* element, char_t c );
		void	( *on_key_press )		( element_t* element, uint key, bool down );
	};
};

void		mgui_element_create		( element_t* element, control_t* parent, bool has_text );
void		mgui_element_render		( element_t* element );
void		mgui_element_process	( element_t* element, uint32 ticks );

void		mgui_update_abs_pos		( element_t* element );
void		mgui_update_abs_size	( element_t* element );
void		mgui_update_rel_pos		( element_t* element );
void		mgui_update_rel_size	( element_t* element );

#endif /* __MGUI_ELEMENT_H */
