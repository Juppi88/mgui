/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		WindowButton.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI window button (close) related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "WindowButton.h"
#include "WindowTitlebar.h"
#include "Window.h"
#include "Skin.h"
#include "InputHook.h"
#include "Input/Input.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

// Window button callback handlers
static void		mgui_windowbutton_on_bounds_change	( MGuiElement* button, bool pos, bool size );
static void		mgui_windowbutton_on_colour_change	( MGuiElement* button );
static void		mgui_windowbutton_on_mouse_hover	( MGuiElement* button );
static void		mgui_windowbutton_on_mouse_click	( MGuiElement* button, int16 x, int16 y, MOUSEBTN mousebtn );
static void		mgui_windowbutton_on_mouse_release	( MGuiElement* button, int16 x, int16 y, MOUSEBTN mousebtn );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	NULL, /* render */
	NULL, /* post_render */
	NULL, /* process */
	NULL, /* get_clip_region */
	mgui_windowbutton_on_bounds_change,
	NULL, /* on_flags_change */
	mgui_windowbutton_on_colour_change,
	NULL, /* on_text_change */
	mgui_windowbutton_on_mouse_hover,
	mgui_windowbutton_on_mouse_hover,
	mgui_windowbutton_on_mouse_click,
	mgui_windowbutton_on_mouse_release,
	NULL, /* on_mouse_drag */
	NULL, /* on_mouse_move */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

// --------------------------------------------------

MGuiWindowButton* mgui_create_windowbutton( MGuiWindow* parent )
{
	MGuiWindowButton* button;
	extern MGuiFont* wndbutton_font; // Font used for the close button X

	button = mem_alloc_clean( sizeof(*button) );
	button->flags_int = INTFLAG_NOPARENT;

	mgui_element_create( cast_elem(button), NULL );

	button->flags |= (FLAG_BORDER|FLAG_BACKGROUND|FLAG_MOUSECTRL);
	button->type = GUI_WINDOWBUTTON;
	button->window = parent;

	// WindoButton callbacks
	button->callbacks = &callbacks;

	button->font = wndbutton_font;
	button->text->font = wndbutton_font;

	mgui_set_text_s( cast_elem(button), _MTEXT("X") );

	return button;
}

static void mgui_windowbutton_on_bounds_change( MGuiElement* button, bool pos, bool size )
{
	MGuiWindowButton* btn;
	struct MGuiWindow* window;

	UNREFERENCED_PARAM( pos );
	UNREFERENCED_PARAM( size );

	btn = (MGuiWindowButton*)button;
	window = (struct MGuiWindow*)btn->window;

	if ( window && window->titlebar )
	{
		btn->bounds.x = window->titlebar->bounds.x + window->titlebar->bounds.w - 28;
		btn->bounds.y = window->titlebar->bounds.y + 5;
		btn->bounds.w = 24;
		btn->bounds.h = 16;
		btn->text->bounds = &btn->bounds;

		mgui_text_update_position( btn->text );
	}
}

static void mgui_windowbutton_on_colour_change( MGuiElement* button )
{
	MGuiWindowButton* btn;
	struct MGuiWindow* wnd;
	uint8 alpha;

	btn = (MGuiWindowButton*)button;
	wnd = (struct MGuiWindow*)btn->window;

	if ( !wnd ) return;

	if ( wnd->titlebar )
	{
		btn->colour = wnd->titlebar->colour;
	}

	alpha = btn->colour.a;

	colour_add_scalar( &btn->text->colour, &btn->colour, 60 );
	btn->text->colour.a = alpha;
}

static void mgui_windowbutton_on_mouse_hover( MGuiElement* button )
{
	MGuiWindowButton* btn;
	btn = (MGuiWindowButton*)button;

	if ( btn->window == NULL ) return;
	mgui_element_request_redraw( cast_elem(btn->window) );
}

static void mgui_windowbutton_on_mouse_click( MGuiElement* button, int16 x, int16 y, MOUSEBTN mousebtn )
{
	MGuiWindowButton* btn;
	btn = (MGuiWindowButton*)button;

	UNREFERENCED_PARAM( mousebtn );
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );

	if ( btn->window == NULL ) return;
	mgui_element_request_redraw( cast_elem(btn->window) );
}

static void mgui_windowbutton_on_mouse_release( MGuiElement* button, int16 x, int16 y, MOUSEBTN mousebtn )
{
	MGuiWindowButton* btn;
	MGuiEvent event;

	UNREFERENCED_PARAM( mousebtn );
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );

	btn = (MGuiWindowButton*)button;

	// If this button has a parent window, hide it
	if ( btn->window == NULL ) return;
	
	btn->window->flags &= ~FLAG_VISIBLE;

	if ( btn->window->event_handler )
	{
		event.type = EVENT_WINDOW_CLOSE;
		event.any.element = cast_elem(btn->window);
		event.any.data = btn->window->event_data;

		btn->window->event_handler( &event );
	}
}
