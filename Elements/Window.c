/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Window.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI window related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "Window.h"
#include "WindowButton.h"
#include "WindowTitlebar.h"
#include "Skin.h"
#include "Font.h"
#include "Platform/Alloc.h"

// Window callback handlers
static void		mgui_window_render				( MGuiElement* window );
static void		mgui_window_on_bounds_change	( MGuiElement* window, bool pos, bool size );
static void		mgui_window_on_flags_change		( MGuiElement* window, uint32 old );
static void		mgui_window_on_colour_change	( MGuiElement* window );
static void		mgui_window_on_mouse_click		( MGuiElement* window, int16 x, int16 y, MOUSEBTN button );
static void		mgui_window_on_mouse_drag		( MGuiElement* window, int16 x, int16 y );

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_window_render,
	NULL, /* process */
	mgui_window_on_bounds_change,
	mgui_window_on_flags_change,
	mgui_window_on_colour_change,
	NULL, /* on_text_change */
	NULL, /* on_mouse_enter */
	NULL, /* on_mouse_leave */
	mgui_window_on_mouse_click,
	NULL, /* on_mouse_release */
	mgui_window_on_mouse_drag,
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

MGuiWindow* mgui_create_window( MGuiElement* parent )
{
	struct MGuiWindow* window;

	window = mem_alloc_clean( sizeof(*window) );
	mgui_element_create( cast_elem(window), parent );

	window->flags |= (FLAG_BORDER|FLAG_SHADOW|FLAG_BACKGROUND|FLAG_WINDOW_TITLEBAR|FLAG_WINDOW_CLOSEBTN);
	window->type = GUI_WINDOW;
	window->font = mgui_font_create( DEFAULT_FONT, 10, FFLAG_BOLD, CHARSET_ANSI );
	window->text->font = window->font;

	// We want to have the titlebar enabled by default, so let's add it
	mgui_window_on_flags_change( cast_elem(window), 0 );

	// Window callbacks
	window->callbacks = &callbacks;

	return cast_elem(window);
}

MGuiWindow* mgui_create_window_ex( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text )
{
	MGuiWindow* window;

	window = mgui_create_window( parent );

	mgui_set_abs_pos_i( window, x, y );
	mgui_set_abs_size_i( window, w, h );
	mgui_add_flags( window, flags );
	mgui_set_colour_i( window, col );
	mgui_set_text_s( window, text );

	return window;
}

static void mgui_window_render( MGuiElement* window )
{
	window->skin->draw_window( window );
}

static void mgui_window_on_bounds_change( MGuiElement* window, bool pos, bool size )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	UNREFERENCED_PARAM( pos );

	if ( wnd->titlebar )
	{
		if ( pos )
		{
			wnd->window_bounds.x = window->bounds.x;
			wnd->window_bounds.y = window->bounds.y;

			wnd->titlebar->bounds.x = window->bounds.x;
			wnd->titlebar->bounds.y = window->bounds.y;
			window->bounds.y += TITLE_H;
		}
		if ( size )
		{
			wnd->window_bounds.w = window->bounds.w;
			wnd->window_bounds.h = window->bounds.h;

			wnd->titlebar->bounds.w = window->bounds.w;
			wnd->titlebar->bounds.h = TITLE_H;
			window->bounds.h -= TITLE_H;
		}

		window->text->bounds = &wnd->titlebar->bounds;
		mgui_text_update_position( window->text );
	}
	else
	{
		wnd->window_bounds = window->bounds;
	}

	if ( wnd->closebtn && wnd->closebtn->callbacks->on_bounds_change )
	{
		wnd->closebtn->callbacks->on_bounds_change( cast_elem(wnd->closebtn), pos, size );
	}
}

static void mgui_window_on_flags_change( MGuiElement* window, uint32 old )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	// Enable the titlebar (and the closebutton if it exists)
	if ( BIT_ENABLED( window->flags, old, FLAG_WINDOW_TITLEBAR ) )
	{
		wnd->titlebar = mgui_create_titlebar( window );

		wnd->titlebar->bounds.x = window->bounds.x;
		wnd->titlebar->bounds.y = window->bounds.y;
		wnd->titlebar->bounds.w = window->bounds.w;
		wnd->titlebar->bounds.h = TITLE_H;

		window->text->bounds = &wnd->titlebar->bounds;

		window->bounds.y += TITLE_H;
		window->bounds.h -= TITLE_H;

		if ( wnd->closebtn )
			wnd->closebtn->flags |= FLAG_VISIBLE;
	}

	// Disable the titlebar and deactivate the close button if required
	else if ( BIT_DISABLED( window->flags, old, FLAG_WINDOW_TITLEBAR ) )
	{
		mgui_element_destroy( cast_elem(wnd->titlebar) );
		wnd->titlebar = NULL;

		window->bounds.y -= TITLE_H;
		window->bounds.h += TITLE_H;

		if ( wnd->closebtn )
		{
			wnd->closebtn->flags &= ~FLAG_VISIBLE;
		}
	}

	// If the titlebar is enabled add a closebutton
	if ( BIT_ENABLED( window->flags, old, FLAG_WINDOW_CLOSEBTN ) )
	{
		if ( BIT_ON( window->flags, FLAG_WINDOW_TITLEBAR ) )
		{
			if ( !wnd->closebtn )
				wnd->closebtn = mgui_create_windowbutton( window );

			wnd->closebtn->callbacks->on_bounds_change( cast_elem(wnd->closebtn), true, true );
		}
	}

	// Disable and destroy the closebutton
	else if ( BIT_DISABLED( window->flags, old, FLAG_WINDOW_CLOSEBTN ) )
	{
		if ( wnd->closebtn )
		{
			mgui_element_destroy( cast_elem(wnd->closebtn) );
			wnd->closebtn = NULL;
		}
	}
}

static void mgui_window_on_colour_change( MGuiElement* window )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	if ( wnd->titlebar )
	{
		wnd->titlebar->colour.a = wnd->colour.a;
		window->text->colour.a = wnd->colour.a;

		if ( wnd->closebtn )
		{
			wnd->closebtn->colour = wnd->titlebar->colour;
		}
	}
}

static void mgui_window_on_mouse_click( MGuiElement* window, int16 x, int16 y, MOUSEBTN button )
{
	struct MGuiWindow* wnd;

	UNREFERENCED_PARAM( button );

	wnd = (struct MGuiWindow*)window;

	wnd->click_offset.x = x;
	wnd->click_offset.y = y;
}

static void mgui_window_on_mouse_drag( MGuiElement* window, int16 x, int16 y )
{
	node_t* node;
	MGuiElement* child;
	MGuiEvent guievent;
	struct MGuiWindow* wnd = (struct MGuiWindow*)window;

	wnd->bounds.x = x - wnd->click_offset.x;
	wnd->bounds.y = y - wnd->click_offset.y;

	mgui_window_on_bounds_change( window, true, false );

	if ( window->children == NULL ) return;

	list_foreach( window->children, node )
	{
		child = cast_elem(node);
		mgui_element_update_abs_pos( child );
	}

	if ( window->event_handler )
	{
		guievent.type = EVENT_DRAG;
		guievent.element = window;
		guievent.data = window->event_data;
		guievent.mouse.x = x;
		guievent.mouse.y = y;

		window->event_handler( &guievent );
	}
}

void mgui_window_get_title_col( MGuiWindow* window, colour_t* col )
{
	MGuiTitlebar* titlebar;

	if ( window == NULL || col == NULL ) return;

	titlebar = ((struct MGuiWindow*)window)->titlebar;
	if ( titlebar == NULL ) return;

	*col = titlebar->colour;
}

void mgui_window_set_title_col( MGuiWindow* window, const colour_t* col )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	if ( wnd == NULL || col == NULL ) return;
	if ( wnd->titlebar == NULL ) return;

	wnd->titlebar->colour = *col;
	wnd->titlebar->colour.a = window->colour.a;

	if ( wnd->closebtn && wnd->closebtn->callbacks->on_colour_change )
	{
		wnd->closebtn->callbacks->on_colour_change( cast_elem(wnd->closebtn) );
	}
}

uint32 mgui_window_get_title_col_i( MGuiWindow* window )
{
	MGuiTitlebar* titlebar;

	if ( window == NULL ) return 0;

	titlebar = ((struct MGuiWindow*)window)->titlebar;
	if ( titlebar == NULL ) return 0;

	return titlebar->colour.hex;
}

void mgui_window_set_title_col_i( MGuiWindow* window, uint32 hex )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	if ( wnd == NULL ) return;
	if ( wnd->titlebar == NULL ) return;

	wnd->titlebar->colour.hex = hex;
	wnd->titlebar->colour.a = window->colour.a;

	if ( wnd->closebtn && wnd->closebtn->callbacks->on_colour_change )
	{
		wnd->closebtn->callbacks->on_colour_change( cast_elem(wnd->closebtn) );
	}
}

void mgui_window_get_drag_offset( MGuiWindow* window, vectorscreen_t* pos )
{
	if ( window == NULL || pos == NULL ) return;

	*pos = ((struct MGuiWindow*)window)->click_offset;
}
