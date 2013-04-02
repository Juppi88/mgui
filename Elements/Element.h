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
#include "Control.h"
#include "Input/Input.h"

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

typedef enum MGUI_TYPE
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
} MGUI_TYPE;

struct MGuiElement
{
	MGuiControl;							// Inherit control data
	enum MGUI_TYPE			type;			// The type of this GUI element
	vector2_t				pos;			// Relative position (within parent element)
	vector2_t				size;			// Relative size (within parent element)
	colour_t				colour;			// Element colour
	MGuiText*				text;			// Text on this element (label, title etc)
	MGuiFont*				font;			// Default font used for all the text on this element
	mgui_event_handler_t	event_handler;	// User event handler callback
	void*					event_data;		// User-specified data to be passed via event_handler

	// ---------- Internal callbacks ---------- 
	void		( *render )				( MGuiElement* element );
	void		( *destroy )			( MGuiElement* element );
	void		( *process )			( MGuiElement* element, uint32 ticks );

	void		( *set_bounds )			( MGuiElement* element, bool pos, bool size );
	void		( *set_colour )			( MGuiElement* element );
	void		( *set_text )			( MGuiElement* element );

	void		( *on_mouse_enter )		( MGuiElement* element );
	void		( *on_mouse_leave )		( MGuiElement* element );
	void		( *on_mouse_click )		( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
	void		( *on_mouse_release )	( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
	void		( *on_mouse_drag )		( MGuiElement* element, uint16 x, uint16 y );
	void		( *on_mouse_wheel )		( MGuiElement* element, float diff );
	void		( *on_character )		( MGuiElement* element, char_t c );
	void		( *on_key_press )		( MGuiElement* element, uint key, bool down );
};

// Generic element functions
void			mgui_element_create			( MGuiElement* element, MGuiControl* parent, bool has_text );
void			mgui_element_destroy		( MGuiElement* element );
void			mgui_element_render			( MGuiElement* element );
void			mgui_element_process		( MGuiElement* element, uint32 ticks );

void			mgui_element_update_abs_pos	( MGuiElement* element );
void			mgui_element_update_abs_size( MGuiElement* element );
void			mgui_element_update_rel_pos	( MGuiElement* element );
void			mgui_element_update_rel_size( MGuiElement* element );
void			mgui_element_update_child_pos( MGuiElement* element );

void			mgui_get_pos				( MGuiElement* element, vector2_t* pos );
void			mgui_get_size				( MGuiElement* element, vector2_t* size );
void			mgui_set_pos				( MGuiElement* element, const vector2_t* pos );
void			mgui_set_size				( MGuiElement* element, const vector2_t* size );
void			mgui_get_abs_pos			( MGuiElement* element, vectorscreen_t* pos );
void			mgui_get_abs_size			( MGuiElement* element, vectorscreen_t* size );
void			mgui_set_abs_pos			( MGuiElement* element, const vectorscreen_t* pos );
void			mgui_set_abs_size			( MGuiElement* element, const vectorscreen_t* size );
void			mgui_get_pos_f				( MGuiElement* element, float* x, float* y );
void			mgui_get_size_f				( MGuiElement* element, float* w, float* h );
void			mgui_set_pos_f				( MGuiElement* element, float x, float y );
void			mgui_set_size_f				( MGuiElement* element, float w, float h );
void			mgui_get_abs_pos_i			( MGuiElement* element, uint16* x, uint16* y );
void			mgui_get_abs_size_i			( MGuiElement* element, uint16* w, uint16* h );
void			mgui_set_abs_pos_i			( MGuiElement* element, uint16 x, uint16 y );
void			mgui_set_abs_size_i			( MGuiElement* element, uint16 w, uint16 h );

void			mgui_get_colour				( MGuiElement* element, colour_t* col );
void			mgui_set_colour				( MGuiElement* element, const colour_t* col );
void			mgui_get_text_colour		( MGuiElement* element, colour_t* col );
void			mgui_set_text_colour		( MGuiElement* element, const colour_t* col );
uint32			mgui_get_colour_i			( MGuiElement* element );
void			mgui_set_colour_i			( MGuiElement* element, uint32 hex );
uint32			mgui_get_text_colour_i		( MGuiElement* element );
void			mgui_set_text_colour_i		( MGuiElement* element, uint32 hex );
uint8			mgui_get_alpha				( MGuiElement* element );
void			mgui_set_alpha				( MGuiElement* element, uint8 alpha );

const char_t*	mgui_get_text				( MGuiElement* element );
uint32			mgui_get_text_len			( MGuiElement* element );
void			mgui_set_text				( MGuiElement* element, const char_t* fmt, ... );
void			mgui_set_text_s				( MGuiElement* element, const char_t* text );
uint32			mgui_get_alignment			( MGuiElement* element );
void			mgui_set_alignment			( MGuiElement* element, uint32 alignment );
void			mgui_get_text_padding		( MGuiElement* element, uint8* top, uint8* bottom, uint8* left, uint8* right );
void			mgui_set_text_padding		( MGuiElement* element, uint8 top, uint8 bottom, uint8 left, uint8 right );

const char_t*	mgui_get_font_name			( MGuiElement* element );
uint8			mgui_get_font_size			( MGuiElement* element );
uint8			mgui_get_font_flags			( MGuiElement* element );
void			mgui_set_font_name			( MGuiElement* element, const char_t* font );
void			mgui_set_font_size			( MGuiElement* element, uint8 size );
void			mgui_set_font_flags			( MGuiElement* element, uint8 flags );
void			mgui_set_font				( MGuiElement* element, const char_t* font, uint8 size, uint8 flags, uint8 charset );

uint32			mgui_get_flags				( MGuiElement* element );
void			mgui_set_flags				( MGuiElement* element, const uint32 flags );
void			mgui_add_flags				( MGuiElement* element, const uint32 flags );
void			mgui_remove_flags			( MGuiElement* element, const uint32 flags );

void			mgui_set_event_handler		( MGuiElement* element, mgui_event_handler_t handler, void* data );

// Default callback handlers
static void		mgui_render_cb				( MGuiElement* element );
static void		mgui_destroy_cb				( MGuiElement* element );
static void		mgui_process_cb				( MGuiElement* element, uint32 ticks );
static void		mgui_set_bounds_cb			( MGuiElement* element, bool pos, bool size );
static void		mgui_set_colour_cb			( MGuiElement* element );
static void		mgui_set_text_cb			( MGuiElement* element );
static void		mgui_on_mouse_enter_cb		( MGuiElement* element );
static void		mgui_on_mouse_leave_cb		( MGuiElement* element );
static void		mgui_on_mouse_click_cb		( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
static void		mgui_on_mouse_release_cb	( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
static void		mgui_on_mouse_drag_cb		( MGuiElement* element, uint16 x, uint16 y );
static void		mgui_on_mouse_wheel_cb		( MGuiElement* element, float wheel );
static void		mgui_on_character_cb		( MGuiElement* element, char_t key );
static void		mgui_on_key_press_cb		( MGuiElement* element, uint key, bool down );

#endif /* __MGUI_ELEMENT_H */
