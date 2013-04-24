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
static void		mgui_destroy_window				( MGuiElement* window );
static void		mgui_window_render				( MGuiElement* window );
static void		mgui_window_set_bounds			( MGuiElement* window, bool pos, bool size );
static void		mgui_window_set_flags			( MGuiElement* window, uint32 old );
static void		mgui_window_set_colour			( MGuiElement* window );
static void		mgui_window_on_mouse_click		( MGuiElement* window, MOUSEBTN button, uint16 x, uint16 y );
static void		mgui_window_on_mouse_drag		( MGuiElement* window, uint16 x, uint16 y );


MGuiWindow* mgui_create_window( MGuiControl* parent )
{
	struct MGuiWindow* window;

	window = mem_alloc_clean( sizeof(*window) );
	mgui_element_create( cast_elem(window), parent, true );

	window->flags |= (FLAG_BORDER|FLAG_SHADOW|FLAG_WINDOW_TITLEBAR|FLAG_WINDOW_CLOSEBTN);
	window->type = GUI_WINDOW;
	window->font = mgui_font_create( DEFAULT_FONT, 10, FFLAG_BOLD, ANSI_CHARSET );
	window->text->font = window->font;

	// Window callbacks
	window->destroy			= mgui_destroy_window;
	window->render			= mgui_window_render;
	window->set_bounds		= mgui_window_set_bounds;
	window->set_flags		= mgui_window_set_flags;
	window->set_colour		= mgui_window_set_colour;
	window->on_mouse_click	= mgui_window_on_mouse_click;
	window->on_mouse_drag	= mgui_window_on_mouse_drag;

	// We want to have the titlebar by default, so let's add it
	window->set_flags( cast_elem(window), 0 );

	return cast_elem(window);
}

static void mgui_destroy_window( MGuiElement* window )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	if ( wnd->titlebar )
		mgui_destroy_titlebar( wnd->titlebar );

	if ( wnd->closebtn )
		mgui_destroy_windowbutton( wnd->closebtn );
}

static void mgui_window_render( MGuiElement* window )
{
	window->skin->draw_window( window );
}

static void mgui_window_set_bounds( MGuiElement* window, bool pos, bool size )
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

	if ( wnd->closebtn )
	{
		wnd->closebtn->set_bounds( cast_elem(wnd->closebtn), pos, size );
	}
}

static void mgui_window_set_flags( MGuiElement* window, uint32 old )
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
		mgui_destroy_titlebar( wnd->titlebar );
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

			wnd->closebtn->set_bounds( cast_elem(wnd->closebtn), true, true );	
		}
	}
	// Disable and destroy the closebutton
	else if ( BIT_DISABLED( window->flags, old, FLAG_WINDOW_CLOSEBTN ) )
	{
		if ( wnd->closebtn )
		{
			mgui_destroy_windowbutton( wnd->closebtn );
			wnd->closebtn = NULL;
		}
	}
}

static void mgui_window_set_colour( MGuiElement* window )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	if ( wnd->titlebar )
	{
		wnd->titlebar->colour.a = wnd->colour.a;
		window->text->colour.a = wnd->colour.a;

		if ( wnd->closebtn )
			wnd->closebtn->colour = wnd->titlebar->colour;
	}
}

static void mgui_window_on_mouse_click( MGuiElement* window, MOUSEBTN button, uint16 x, uint16 y )
{
	struct MGuiWindow* wnd;
	extern rectangle_t draw_rect;

	UNREFERENCED_PARAM( button );

	wnd = (struct MGuiWindow*)window;

	wnd->click_offset.x = x;
	wnd->click_offset.y = y;
}

static void mgui_window_on_mouse_drag( MGuiElement* window, uint16 x, uint16 y )
{
	node_t* node;
	MGuiElement* child;
	MGuiEvent guievent;
	extern vectorscreen_t draw_size;
	struct MGuiWindow* wnd = (struct MGuiWindow*)window;

	wnd->bounds.x = (uint16)math_clamp( (int16)x - wnd->click_offset.x, 0, draw_size.x - wnd->window_bounds.w );
	wnd->bounds.y = (uint16)math_clamp( (int16)y - wnd->click_offset.y, 0, draw_size.y - wnd->window_bounds.h );

	mgui_window_set_bounds( window, true, false );

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

	if ( wnd->closebtn )
	{
		wnd->closebtn->set_colour( cast_elem(wnd->closebtn) );
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

	if ( wnd->closebtn )
	{
		wnd->closebtn->set_colour( cast_elem(wnd->closebtn) );
	}
}

void mgui_window_get_drag_offset( MGuiWindow* window, vectorscreen_t* pos )
{
	if ( window == NULL || pos == NULL ) return;

	*pos = ((struct MGuiWindow*)window)->click_offset;
}
