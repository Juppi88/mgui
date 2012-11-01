/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		ScrollBar.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI scrollbar related functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 * 
 **********************************************************************/

#include "Scrollbar.h"
#include "Skin.h"
#include "Renderer.h"
#include "Platform/Platform.h"
#include <assert.h>

static void __mgui_destroy_scrollbar( element_t* scrollbar );
static void __mgui_scrollbar_render( element_t* scrollbar );
static void __mgui_scrollbar_on_bounds_update( element_t* scrollbar, bool pos, bool size );

scrollbar_t* mgui_create_scrollbar( control_t* parent )
{
	struct scrollbar_s* scrollbar;

	scrollbar = mem_alloc_clean( sizeof(*scrollbar) );
	mgui_element_create( cast_elem(scrollbar), parent, false );

	scrollbar->type = GUI_SCROLLBAR;
	scrollbar->flags |= (FLAG_MOUSECTRL|FLAG_DRAGGABLE);

	scrollbar->font = NULL;
	scrollbar->bar_position = 1.0f;
	scrollbar->bar_size = 1.0f;
	scrollbar->nudge_amount = 0.1f;

	// Scrollbar callbacks
	scrollbar->destroy = __mgui_destroy_scrollbar;
	scrollbar->render = __mgui_scrollbar_render;
	scrollbar->on_bounds_update = __mgui_scrollbar_on_bounds_update;

	return cast_elem(scrollbar);
}

static void __mgui_destroy_scrollbar( element_t* scrollbar )
{
	UNREFERENCED_PARAM(scrollbar);
}

static void __mgui_scrollbar_render( element_t* scrollbar )
{
	struct scrollbar_s* bar;
	bar = (struct scrollbar_s*)scrollbar;

	skin->draw_scrollbar( &bar->bounds, &bar->track_col, 0 );
	skin->draw_scrollbar_button( &bar->button1, &bar->colour, 0, &bar->track_col, ARROW_UP );
	skin->draw_scrollbar_button( &bar->button2, &bar->colour, 0, &bar->track_col, ARROW_DOWN );

	if ( bar->bar_size != 1.0f )
		skin->draw_scrollbar_bar( &bar->bar, &bar->colour, 0 );
}

void __mgui_scrollbar_on_bounds_update( element_t* scrollbar, bool pos, bool size )
{
	struct scrollbar_s* bar;
	uint16 tracksize;

	UNREFERENCED_PARAM(pos);
	UNREFERENCED_PARAM(size);

	bar = (struct scrollbar_s*)scrollbar;

	bar->button1.x = bar->bounds.x;
	bar->button1.y = bar->bounds.y;
	bar->button1.w = bar->bounds.w;
	bar->button1.h = bar->bounds.w;

	bar->button2.x = bar->bounds.x;
	bar->button2.y = bar->bounds.y + bar->bounds.h - bar->bounds.w;
	bar->button2.w = bar->bounds.w;
	bar->button2.h = bar->bounds.w;

	tracksize = bar->bounds.h - 2 * bar->bounds.w;

	bar->bar.x = bar->bounds.x;
	bar->bar.w = bar->bounds.w;
	bar->bar.h = (uint16)( bar->bar_size * tracksize );
	bar->bar.y = bar->bounds.y + bar->bounds.w + (uint16)( ( tracksize - bar->bar.h ) * bar->bar_position );

	mgui_force_redraw();
}

uint32 mgui_scrollbar_get_track_colour( scrollbar_t* scrollbar )
{
	struct scrollbar_s* bar;
	bar = (struct scrollbar_s*)scrollbar;

	return colour_to_hex( &bar->track_col );
}

void mgui_scrollbar_set_track_colour( scrollbar_t* scrollbar, uint32 colour )
{
	struct scrollbar_s* bar;
	bar = (struct scrollbar_s*)scrollbar;

	hex_to_colour( colour, &bar->track_col );
	bar->track_col.a = bar->colour.a;
}
