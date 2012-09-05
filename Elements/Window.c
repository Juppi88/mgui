/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Window.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI window related functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "Window.h"
#include "Skin.h"
#include "Font.h"
#include "Platform/Platform.h"
#include <assert.h>

static void __mgui_destroy_window( element_t* window );
static void __mgui_window_render( element_t* window );
static void __mgui_window_on_bounds_update( element_t* window, bool pos, bool size );
static void __mgui_window_on_colour_update( element_t* window );
static void __mgui_window_on_mouse_click( element_t* element, MOUSEBTN button, uint16 x, uint16 y );
static void __mgui_window_on_mouse_drag( element_t* element, uint16 x, uint16 y );

window_t* mgui_create_window( control_t* parent )
{
	struct window_s* window;

	window = mem_alloc_clean( sizeof(*window) );
	mgui_element_create( cast_elem(window), parent, true );

	window->flags |= (FLAG_BORDER|FLAG_SHADOW);
	window->type = GUI_WINDOW;
	window->font = mgui_font_create( DEFAULT_FONT, 10, FFLAG_BOLD, ANSI_CHARSET );
	window->text->font = window->font;

	// Window callbacks
	window->destroy = __mgui_destroy_window;
	window->render = __mgui_window_render;
	window->on_bounds_update = __mgui_window_on_bounds_update;
	window->on_colour_update = __mgui_window_on_colour_update;
	window->on_mouse_click = __mgui_window_on_mouse_click;
	window->on_mouse_drag = __mgui_window_on_mouse_drag;

	// We want to have the titlebar by default, so let's add it
	mgui_window_set_titlebar( cast_elem(window), true );
	mgui_window_set_closebtn( cast_elem(window), true );

	return cast_elem(window);
}

static void __mgui_destroy_window( element_t* window )
{
	struct window_s* wnd;
	wnd = (struct window_s*)window;

	if ( wnd->titlebar )
		mgui_destroy_titlebar( wnd->titlebar );

	if ( wnd->closebtn )
		mgui_destroy_windowbutton( wnd->closebtn );
}

static void __mgui_window_render( element_t* window )
{
	rectangle_t* r;
	rectangle_t border;
	uint32 flags;
	colour_t col;
	struct window_s* wnd = (struct window_s*)window;

	r = &window->bounds;
	flags = window->flags;

	if ( BIT_ON( flags, FLAG_SHADOW ) )
	{
		skin->draw_shadow( &wnd->window_bounds, 3 );
	}

	if ( BIT_ON( flags, FLAG_TITLEBAR ) && wnd->titlebar )
	{
		skin->draw_window_titlebar( &wnd->titlebar->bounds, &wnd->titlebar->colour, window->text );
		skin->draw_window( r, &window->colour, flags );

		if ( BIT_ON( flags, FLAG_CLOSEBTN ) && wnd->closebtn )
		{
			skin->draw_button( &wnd->closebtn->bounds, &wnd->closebtn->colour, wnd->closebtn->flags, wnd->closebtn->text );
		}

		if ( BIT_ON( flags, FLAG_BORDER ) )
		{
			border.x = wnd->bounds.x + 1;
			border.y = wnd->bounds.y;
			border.w = wnd->bounds.w - 2;
			border.h = wnd->bounds.h;

			col.r = colour_clamp( wnd->titlebar->colour.r + 60 );
			col.g = colour_clamp( wnd->titlebar->colour.g + 60 );
			col.b = colour_clamp( wnd->titlebar->colour.b + 60 );
			col.a = wnd->titlebar->colour.a;

			skin->draw_border( &border, &wnd->titlebar->colour, BORDER_ALL&(~BORDER_TOP), 2 );
			skin->draw_border( &wnd->window_bounds, &col, BORDER_ALL, 1 );
		}
	}
	else
	{
		skin->draw_window( r, &window->colour, flags );

		if ( flags & FLAG_BORDER )
		{
			col.r = colour_clamp( wnd->colour.r - 40 );
			col.g = colour_clamp( wnd->colour.g - 40 );
			col.b = colour_clamp( wnd->colour.b - 40 );
			col.a = wnd->colour.a;

			skin->draw_border( r, &col, BORDER_ALL, 2 );
		}
	}
}

static void __mgui_window_on_bounds_update( element_t* window, bool pos, bool size )
{
	struct window_s* wnd;
	wnd = (struct window_s*)window;

	UNREFERENCED_PARAM(pos);

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
		wnd->closebtn->on_bounds_update( cast_elem(wnd->closebtn), pos, size );
	}
}

static void __mgui_window_on_colour_update( element_t* window )
{
	struct window_s* wnd;
	wnd = (struct window_s*)window;

	if ( wnd->titlebar )
	{
		wnd->titlebar->colour.a = wnd->colour.a;
		window->text->colour.a = wnd->colour.a;

		if ( wnd->closebtn )
			wnd->closebtn->colour = wnd->titlebar->colour;
	}
}

static void __mgui_window_on_mouse_click( element_t* element, MOUSEBTN button, uint16 x, uint16 y )
{
	struct window_s* window;
	extern rectangle_t screen_rect;

	UNREFERENCED_PARAM(button);

	window = (struct window_s*)element;

	window->click_offset.x = x;
	window->click_offset.y = y;
}

static void __mgui_window_on_mouse_drag( element_t* element, uint16 x, uint16 y )
{
	node_t* node;
	element_t* child;
	guievent_t guievent;
	extern vectorscreen_t screen_size;
	struct window_s* window = (struct window_s*)element;

	window->bounds.x = (uint16)math_clamp( (int16)x - window->click_offset.x, 0, screen_size.x - window->window_bounds.w );
	window->bounds.y = (uint16)math_clamp( (int16)y - window->click_offset.y, 0, screen_size.y - window->window_bounds.h );

	__mgui_window_on_bounds_update( cast_elem(window), true, false );

	if ( !window->children ) return;

	list_foreach( window->children, node )
	{
		child = cast_elem(node);
		mgui_update_abs_pos( child ); 
	}

	if ( element->event_handler )
	{
		guievent.type = EVENT_DRAG;
		guievent.element = element;
		guievent.data = element->event_data;
		guievent.mouse.x = x;
		guievent.mouse.y = y;

		element->event_handler( &guievent );
	}
}

bool mgui_window_get_titlebar( window_t* window )
{
	assert( window != NULL );
	return BIT_ON( window->flags, FLAG_TITLEBAR );
}

void mgui_window_set_titlebar( window_t* window, bool enabled )
{
	struct window_s* wnd;
	wnd = (struct window_s*)window;

	assert( window != NULL );

	if ( enabled )
	{
		if ( BIT_ON( window->flags, FLAG_TITLEBAR ) ) return;

		window->flags |= FLAG_TITLEBAR;
		wnd->titlebar = mgui_create_titlebar( wnd );

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
	else
	{
		if ( BIT_OFF( window->flags, FLAG_TITLEBAR ) || !wnd->titlebar ) return;

		window->flags &= ~FLAG_TITLEBAR;
		mgui_destroy_titlebar( wnd->titlebar );
		wnd->titlebar = NULL;

		window->bounds.y -= TITLE_H;
		window->bounds.h += TITLE_H;

		if ( wnd->closebtn )
		{
			window->flags &= ~FLAG_CLOSEBTN;
			mgui_destroy_windowbutton( wnd->closebtn );
			wnd->closebtn = NULL;
		}
	}
}

bool mgui_window_get_closebtn( window_t* window )
{
	assert( window != NULL );
	return BIT_ON( window->flags, FLAG_CLOSEBTN );
}

void mgui_window_set_closebtn( window_t* window, bool enabled )
{
	struct window_s* wnd;
	wnd = (struct window_s*)window;

	assert( window != NULL );

	if ( enabled )
	{
		if ( BIT_ON( window->flags, FLAG_CLOSEBTN ) ) return;
		if ( BIT_OFF( window->flags, FLAG_TITLEBAR ) ) return;

		window->flags |= FLAG_CLOSEBTN;

		if ( !wnd->closebtn )
			wnd->closebtn = mgui_create_windowbutton( wnd );

		wnd->closebtn->on_bounds_update( cast_elem(wnd->closebtn), true, true );
	}
	else
	{
		if ( BIT_OFF( window->flags, FLAG_CLOSEBTN ) ) return;

		window->flags &= ~FLAG_CLOSEBTN;
		
		if ( wnd->closebtn )
		{
			mgui_destroy_windowbutton( wnd->closebtn );
			wnd->closebtn = NULL;
		}
	}
}

uint32 mgui_window_get_title_col( window_t* window )
{
	struct titlebar_s* titlebar;

	assert( window != NULL );

	titlebar = ((struct window_s*)window)->titlebar;
	if ( !titlebar ) return 0;

	return colour_to_hex( &titlebar->colour );
}

void mgui_window_set_title_col( window_t* window, uint32 colour )
{
	struct window_s* wnd;
	wnd = (struct window_s*)window;

	assert( window != NULL );

	if ( !wnd->titlebar ) return;

	hex_to_colour( colour, &wnd->titlebar->colour );
	wnd->titlebar->colour.a = window->colour.a;

	if ( wnd->closebtn )
	{
		wnd->closebtn->on_colour_update( cast_elem(wnd->closebtn) );
	}
}

void mgui_window_get_drag_offset( window_t* window, vectorscreen_t* pos )
{
	assert( window != NULL );

	*pos = ((struct window_s*)window)->click_offset;
}
