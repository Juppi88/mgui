/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		ScrollBar.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI scrollbar related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 * 
 **********************************************************************/

#include "Scrollbar.h"
#include "Skin.h"
#include "Renderer.h"
#include "Platform/Alloc.h"

// Scrollbar callback handlers
static void		mgui_destroy_scrollbar			( MGuiElement* scrollbar );
static void		mgui_scrollbar_render			( MGuiElement* scrollbar );
static void		mgui_scrollbar_set_bounds		( MGuiElement* scrollbar, bool pos, bool size );

MGuiScrollbar* mgui_create_scrollbar( MGuiControl* parent )
{
	struct MGuiScrollbar* scrollbar;

	scrollbar = mem_alloc_clean( sizeof(*scrollbar) );
	mgui_element_create( cast_elem(scrollbar), parent, false );

	scrollbar->type = GUI_SCROLLBAR;
	scrollbar->flags |= (FLAG_MOUSECTRL|FLAG_DRAGGABLE);

	scrollbar->font = NULL;
	scrollbar->bar_position = 1.0f;
	scrollbar->bar_size = 1.0f;
	scrollbar->nudge_amount = 0.1f;

	// Scrollbar callbacks
	scrollbar->destroy = mgui_destroy_scrollbar;
	scrollbar->render = mgui_scrollbar_render;
	scrollbar->set_bounds = mgui_scrollbar_set_bounds;

	return cast_elem(scrollbar);
}

static void mgui_destroy_scrollbar( MGuiElement* scrollbar )
{
	UNREFERENCED_PARAM( scrollbar );
}

static void mgui_scrollbar_render( MGuiElement* scrollbar )
{
	scrollbar->skin->draw_scrollbar( scrollbar );
}

void mgui_scrollbar_set_bounds( MGuiElement* scrollbar, bool pos, bool size )
{
	struct MGuiScrollbar* bar;
	uint16 tracksize;

	UNREFERENCED_PARAM( pos );
	UNREFERENCED_PARAM( size );

	bar = (struct MGuiScrollbar*)scrollbar;

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

void mgui_scrollbar_get_track_colour( MGuiScrollbar* scrollbar, colour_t* col )
{
	struct MGuiScrollbar* bar;
	bar = (struct MGuiScrollbar*)scrollbar;

	*col = bar->track_col;
}

void mgui_scrollbar_set_track_colour( MGuiScrollbar* scrollbar, const colour_t* col )
{
	struct MGuiScrollbar* bar;
	bar = (struct MGuiScrollbar*)scrollbar;

	bar->track_col = *col;
	bar->track_col.a = bar->colour.a;
}
