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

MGuiWindow* mgui_create_window( MGuiControl* parent )
{
	struct MGuiWindow* window;

	window = mem_alloc_clean( sizeof(*window) );
	mgui_element_create( cast_elem(window), parent, true );

	window->flags |= (FLAG_BORDER|FLAG_SHADOW);
	window->type = GUI_WINDOW;
	window->font = mgui_font_create( DEFAULT_FONT, 10, FFLAG_BOLD, ANSI_CHARSET );
	window->text->font = window->font;

	// Window callbacks
	window->destroy			= mgui_destroy_window;
	window->render			= mgui_window_render;
	window->set_bounds		= mgui_window_set_bounds;
	window->set_colour		= mgui_window_set_colour;
	window->on_mouse_click	= mgui_window_on_mouse_click;
	window->on_mouse_drag	= mgui_window_on_mouse_drag;

	// We want to have the titlebar by default, so let's add it
	mgui_window_set_titlebar( cast_elem(window), true );
	mgui_window_set_closebtn( cast_elem(window), true );

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
	rectangle_t* r;
	rectangle_t border;
	uint32 flags;
	colour_t col;
	struct MGuiWindow* wnd = (struct MGuiWindow*)window;

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

			colour_add_scalar( &col, &wnd->titlebar->colour, 60 );
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
			colour_subtract_scalar( &col, &wnd->colour, 40 );
			col.a = wnd->colour.a;

			skin->draw_border( r, &col, BORDER_ALL, 2 );
		}
	}
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

static void mgui_window_on_mouse_click( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y )
{
	struct MGuiWindow* window;
	extern rectangle_t draw_rect;

	UNREFERENCED_PARAM( button );

	window = (struct MGuiWindow*)element;

	window->click_offset.x = x;
	window->click_offset.y = y;
}

static void mgui_window_on_mouse_drag( MGuiElement* element, uint16 x, uint16 y )
{
	node_t* node;
	MGuiElement* child;
	MGuiEvent guievent;
	extern vectorscreen_t draw_size;
	struct MGuiWindow* window = (struct MGuiWindow*)element;

	window->bounds.x = (uint16)math_clamp( (int16)x - window->click_offset.x, 0, draw_size.x - window->window_bounds.w );
	window->bounds.y = (uint16)math_clamp( (int16)y - window->click_offset.y, 0, draw_size.y - window->window_bounds.h );

	mgui_window_set_bounds( cast_elem(window), true, false );

	if ( !window->children ) return;

	list_foreach( window->children, node )
	{
		child = cast_elem(node);
		mgui_element_update_abs_pos( child ); 
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

bool mgui_window_get_closebtn( MGuiWindow* window )
{
	if ( window == NULL ) return false;

	return BIT_ON( window->flags, FLAG_CLOSEBTN );
}

void mgui_window_set_closebtn( MGuiWindow* window, bool enabled )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	if ( window == NULL ) return;

	if ( enabled )
	{
		if ( BIT_ON( window->flags, FLAG_CLOSEBTN ) ) return;
		if ( BIT_OFF( window->flags, FLAG_TITLEBAR ) ) return;

		window->flags |= FLAG_CLOSEBTN;

		if ( !wnd->closebtn )
			wnd->closebtn = mgui_create_windowbutton( window );

		wnd->closebtn->set_bounds( cast_elem(wnd->closebtn), true, true );
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

bool mgui_window_get_titlebar( MGuiWindow* window )
{
	if ( window == NULL ) return false;

	return BIT_ON( window->flags, FLAG_TITLEBAR );
}

void mgui_window_set_titlebar( MGuiWindow* window, bool enabled )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	if ( window == NULL ) return;

	if ( enabled )
	{
		if ( BIT_ON( window->flags, FLAG_TITLEBAR ) ) return;

		window->flags |= FLAG_TITLEBAR;
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

	if ( wnd == NULL ) return;
	if ( wnd->titlebar == NULL ) return;

	wnd->titlebar->colour = *col;
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
