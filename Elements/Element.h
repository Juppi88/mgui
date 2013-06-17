/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Element.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A base for all real-world GUI elements.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_ELEMENT_H
#define __MGUI_ELEMENT_H

#include "MGUI.h"
#include "Text.h"
#include "Skin.h"
#include "Input/Input.h"

/* The following are internal flags and should not be used by the library user */
enum MGUI_INTERNAL_FLAGS
{
	INTFLAG_BACKBUFFER	= 1 << 0,	/* Cache this element into a backbuffer */
	INTFLAG_REFRESH		= 1 << 1,	/* Backbuffer needs refreshing */
	INTFLAG_FOCUS		= 1 << 2,	/* This element has captured focus */
	INTFLAG_HOVER		= 1 << 3,	/* The element is hovered over by a mouse cursor */
	INTFLAG_PRESSED		= 1 << 4,	/* The element is pressed down */
	INTFLAG_NOTEXT		= 1 << 5,	/* Element has no text */
	INTFLAG_LAYER		= 1 << 6,	/* This element is a main GUI layer */
};

typedef enum MGUI_TYPE
{
	GUI_NONE,
	GUI_BUTTON,
	GUI_CANVAS,
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
} MGUI_TYPE;

struct MGuiElement
{
	node_t;									// Next and previous elements (linked list node)
	uint32					flags;			// Element flags
	uint32					flags_int;		// Internal flags
	rectangle_t				bounds;			// Absolute boundaries for this element (in pixels)
	MGuiElement*			parent;			// Parent element, NULL if none
	list_t*					children;		// List of children elements
	enum MGUI_TYPE			type;			// The type of this GUI element
	vector2_t				pos;			// Relative position (within parent element)
	vector2_t				size;			// Relative size (within parent element)
	colour_t				colour;			// Element colour
	MGuiText*				text;			// Text on this element (label, title etc)
	MGuiFont*				font;			// Default font used for all the text on this element
	MGuiSkin*				skin;			// Skin to be used for rendering
	mgui_event_handler_t	event_handler;	// User event handler callback
	void*					event_data;		// User-specified data to be passed via event_handler

	// ---------- Internal callbacks ----------

	struct MGuiCallbacks {
		void		( *destroy )			( MGuiElement* element );
		void		( *render )				( MGuiElement* element );
		void		( *process )			( MGuiElement* element );

		void		( *on_bounds_change )	( MGuiElement* element, bool pos, bool size );
		void		( *on_flags_change )	( MGuiElement* element, uint32 old );
		void		( *on_colour_change )	( MGuiElement* element );
		void		( *on_text_change )		( MGuiElement* element );
		void		( *on_mouse_enter )		( MGuiElement* element );
		void		( *on_mouse_leave )		( MGuiElement* element );
		void		( *on_mouse_click )		( MGuiElement* element, uint16 x, uint16 y, MOUSEBTN button );
		void		( *on_mouse_release )	( MGuiElement* element, uint16 x, uint16 y, MOUSEBTN button );
		void		( *on_mouse_drag )		( MGuiElement* element, uint16 x, uint16 y );
		void		( *on_mouse_wheel )		( MGuiElement* element, float diff );
		void		( *on_character )		( MGuiElement* element, char_t c );
		void		( *on_key_press )		( MGuiElement* element, uint32 key, bool down );
	} *callbacks;
};

// Generic element functions
void			mgui_element_create				( MGuiElement* element, MGuiElement* parent );
void			mgui_element_destroy			( MGuiElement* element );
void			mgui_element_render				( MGuiElement* element );
void			mgui_element_process			( MGuiElement* element );

MGuiElement*	mgui_get_element_at				( int16 x, int16 y );

void			mgui_add_child					( MGuiElement* parent, MGuiElement* child );
void			mgui_remove_child				( MGuiElement* child );
void			mgui_move_forward				( MGuiElement* child );
void			mgui_move_backward				( MGuiElement* child );
void			mgui_send_to_top				( MGuiElement* child );
void			mgui_send_to_bottom				( MGuiElement* child );
bool			mgui_is_child_of				( MGuiElement* parent, MGuiElement* child );

void			mgui_element_update_abs_pos		( MGuiElement* element );
void			mgui_element_update_abs_size	( MGuiElement* element );
void			mgui_element_update_rel_pos		( MGuiElement* element );
void			mgui_element_update_rel_size	( MGuiElement* element );
void			mgui_element_update_child_pos	( MGuiElement* element );

void			mgui_get_pos					( MGuiElement* element, vector2_t* pos );
void			mgui_get_size					( MGuiElement* element, vector2_t* size );
void			mgui_set_pos					( MGuiElement* element, const vector2_t* pos );
void			mgui_set_size					( MGuiElement* element, const vector2_t* size );
void			mgui_get_abs_pos				( MGuiElement* element, vectorscreen_t* pos );
void			mgui_get_abs_size				( MGuiElement* element, vectorscreen_t* size );
void			mgui_set_abs_pos				( MGuiElement* element, const vectorscreen_t* pos );
void			mgui_set_abs_size				( MGuiElement* element, const vectorscreen_t* size );
void			mgui_get_pos_f					( MGuiElement* element, float* x, float* y );
void			mgui_get_size_f					( MGuiElement* element, float* w, float* h );
void			mgui_set_pos_f					( MGuiElement* element, float x, float y );
void			mgui_set_size_f					( MGuiElement* element, float w, float h );
void			mgui_get_abs_pos_i				( MGuiElement* element, int16* x, int16* y );
void			mgui_get_abs_size_i				( MGuiElement* element, uint16* w, uint16* h );
void			mgui_set_abs_pos_i				( MGuiElement* element, int16 x, int16 y );
void			mgui_set_abs_size_i				( MGuiElement* element, uint16 w, uint16 h );

void			mgui_get_colour					( MGuiElement* element, colour_t* col );
void			mgui_set_colour					( MGuiElement* element, const colour_t* col );
void			mgui_get_text_colour			( MGuiElement* element, colour_t* col );
void			mgui_set_text_colour			( MGuiElement* element, const colour_t* col );
uint32			mgui_get_colour_i				( MGuiElement* element );
void			mgui_set_colour_i				( MGuiElement* element, uint32 hex );
uint32			mgui_get_text_colour_i			( MGuiElement* element );
void			mgui_set_text_colour_i			( MGuiElement* element, uint32 hex );
uint8			mgui_get_alpha					( MGuiElement* element );
void			mgui_set_alpha					( MGuiElement* element, uint8 alpha );

const char_t*	mgui_get_text					( MGuiElement* element );
uint32			mgui_get_text_len				( MGuiElement* element );
void			mgui_set_text					( MGuiElement* element, const char_t* fmt, ... );
void			mgui_set_text_s					( MGuiElement* element, const char_t* text );
uint32			mgui_get_alignment				( MGuiElement* element );
void			mgui_set_alignment				( MGuiElement* element, uint32 alignment );
void			mgui_get_text_padding			( MGuiElement* element, uint8* top, uint8* bottom, uint8* left, uint8* right );
void			mgui_set_text_padding			( MGuiElement* element, uint8 top, uint8 bottom, uint8 left, uint8 right );

const char_t*	mgui_get_font_name				( MGuiElement* element );
uint8			mgui_get_font_size				( MGuiElement* element );
uint8			mgui_get_font_flags				( MGuiElement* element );
void			mgui_set_font_name				( MGuiElement* element, const char_t* font );
void			mgui_set_font_size				( MGuiElement* element, uint8 size );
void			mgui_set_font_flags				( MGuiElement* element, uint8 flags );
void			mgui_set_font					( MGuiElement* element, const char_t* font, uint8 size, uint8 flags, uint8 charset );

uint32			mgui_get_flags					( MGuiElement* element );
void			mgui_set_flags					( MGuiElement* element, uint32 flags );
void			mgui_add_flags					( MGuiElement* element, uint32 flags );
void			mgui_remove_flags				( MGuiElement* element, uint32 flags );

void			mgui_set_event_handler			( MGuiElement* element, mgui_event_handler_t handler, void* data );

#endif /* __MGUI_ELEMENT_H */
