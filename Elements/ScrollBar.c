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
static void		mgui_scrollbar_render			( MGuiElement* scrollbar );
static void		mgui_scrollbar_on_bounds_change	( MGuiElement* scrollbar, bool pos, bool size );

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_scrollbar_render,
	NULL, /* process */
	mgui_scrollbar_on_bounds_change,
	NULL, /* on_flags_change */
	NULL, /* on_colour_change */
	NULL, /* on_text_change */
	NULL, /* on_mouse_enter */
	NULL, /* on_mouse_leave */
	NULL, /* on_mouse_click */
	NULL, /* on_mouse_release */
	NULL, /* on_mouse_drag */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

MGuiScrollbar* mgui_create_scrollbar( MGuiElement* parent )
{
	struct MGuiScrollbar* scrollbar;

	scrollbar = mem_alloc_clean( sizeof(*scrollbar) );
	scrollbar->flags_int |= INTFLAG_NOTEXT;

	mgui_element_create( cast_elem(scrollbar), parent );

	scrollbar->type = GUI_SCROLLBAR;
	scrollbar->flags |= (FLAG_MOUSECTRL|FLAG_DRAGGABLE);

	scrollbar->font = NULL;
	scrollbar->bar_position = 1.0f;
	scrollbar->bar_size = 1.0f;
	scrollbar->nudge_amount = 0.1f;

	// Scrollbar callbacks
	scrollbar->callbacks = &callbacks;

	return cast_elem(scrollbar);
}

static void mgui_scrollbar_render( MGuiElement* scrollbar )
{
	scrollbar->skin->draw_scrollbar( scrollbar );
}

void mgui_scrollbar_on_bounds_change( MGuiElement* scrollbar, bool pos, bool size )
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
