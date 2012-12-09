/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		WindowButton.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI window button (close) related functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "WindowButton.h"
#include "WindowTitlebar.h"
#include "Window.h"
#include "Skin.h"
#include "InputHook.h"
#include "Input/Input.h"
#include "Platform/Platform.h"

static MGuiFont* font = NULL;

static void __mgui_windowbutton_on_bounds_update( MGuiElement* button, bool pos, bool size );
static void __mgui_windowbutton_on_colour_update( MGuiElement* button );
static void __mgui_windowbutton_on_mouse_enter( MGuiElement* button );
static void __mgui_windowbutton_on_mouse_leave( MGuiElement* button );
static void __mgui_windowbutton_on_mouse_click( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
static void __mgui_windowbutton_on_mouse_release( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );

MGuiWindowButton* mgui_create_windowbutton( MGuiWindow* parent )
{
	MGuiWindowButton* button;

	button = mem_alloc_clean( sizeof(*button) );
	mgui_element_create( cast_elem(button), NULL, true );

	button->flags |= (FLAG_BORDER|FLAG_BACKGROUND|FLAG_MOUSECTRL);
	button->type = GUI_WINDOWBUTTON;
	button->window = parent;

	if ( !font ) font = mgui_font_create_range( DEFAULT_FONT, 10, FFLAG_NONE, ANSI_CHARSET, 'X', 'X' );

	button->font = font;
	button->text->font = font;
	mgui_set_text_s( cast_elem(button), _TEXT("X") );

	// Button callbacks
	button->on_bounds_update = __mgui_windowbutton_on_bounds_update;
	button->on_colour_update = __mgui_windowbutton_on_colour_update;
	button->on_mouse_enter = __mgui_windowbutton_on_mouse_enter;
	button->on_mouse_leave = __mgui_windowbutton_on_mouse_leave;
	button->on_mouse_click = __mgui_windowbutton_on_mouse_click;
	button->on_mouse_release = __mgui_windowbutton_on_mouse_release;

	return button;
}

void mgui_destroy_windowbutton( MGuiWindowButton* button )
{
	mgui_input_cleanup_references( cast_elem(button) );
	mem_free( button );
}

static void __mgui_windowbutton_on_bounds_update( MGuiElement* button, bool pos, bool size )
{
	MGuiWindowButton* btn;
	struct _MGuiWindow* window;

	UNREFERENCED_PARAM( pos );
	UNREFERENCED_PARAM( size );

	btn = (MGuiWindowButton*)button;
	window = (struct _MGuiWindow*)btn->window;

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

static void __mgui_windowbutton_on_colour_update( MGuiElement* button )
{
	MGuiWindowButton* btn;
	struct _MGuiWindow* wnd;
	uint8 alpha;

	btn = (MGuiWindowButton*)button;
	wnd = (struct _MGuiWindow*)btn->window;

	if ( !wnd ) return;

	if ( wnd->titlebar )
		btn->colour = wnd->titlebar->colour;

	alpha = btn->colour.a;

	colour_add_scalar( &btn->text->colour, &btn->colour, 75 );
	btn->text->colour.a = alpha;
}

static void __mgui_windowbutton_on_mouse_click( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y )
{
	MGuiWindowButton* btn;

	UNREFERENCED_PARAM( button );
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );

	btn = (MGuiWindowButton*)element;
}

static void __mgui_windowbutton_on_mouse_release( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y )
{
	MGuiWindowButton* btn;
	MGuiEvent guievent;

	UNREFERENCED_PARAM( button );
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );

	btn = (MGuiWindowButton*)element;

	// If this button has a parent window, hide it
	if ( btn->window )
	{
		btn->window->flags &= ~FLAG_VISIBLE;

		if ( btn->window->event_handler )
		{
			guievent.type = EVENT_WINDOW_CLOSE;
			guievent.element = cast_elem(btn->window);
			guievent.data = btn->window->event_data;
			guievent.mouse.x = x;
			guievent.mouse.y = y;

			btn->window->event_handler( &guievent );
		}
	}
}

static void __mgui_windowbutton_on_mouse_enter( MGuiElement* button )
{
	UNREFERENCED_PARAM( button );
}

static void __mgui_windowbutton_on_mouse_leave( MGuiElement* button )
{
	UNREFERENCED_PARAM( button );
}
