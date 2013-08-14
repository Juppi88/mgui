/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Canvas.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI canvas related functions. Canvas is an invisible
 *				element that can be used as a parent for everything else.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "Canvas.h"
#include "Renderer.h"
#include "Platform/Alloc.h"

extern vectorscreen_t draw_size;
extern MGuiRenderer* renderer;

// Canvas callback handlers
static void		mgui_canvas_render				( MGuiElement* canvas );
static void		mgui_canvas_on_bounds_change	( MGuiElement* canvas, bool pos, bool size );

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_canvas_render,
	NULL, /* process */
	NULL, /* get_clip_region */
	mgui_canvas_on_bounds_change,
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

MGuiCanvas* mgui_create_canvas( MGuiElement* parent )
{
	struct MGuiCanvas* canvas;

	canvas = mem_alloc_clean( sizeof(*canvas) );
	canvas->flags_int |= INTFLAG_NOTEXT;

	mgui_element_create( cast_elem(canvas), parent );

	canvas->type = GUI_CANVAS;
	canvas->colour.hex = 0;
	canvas->children = list_create();
	canvas->parent = NULL;
	canvas->flags = FLAG_VISIBLE;
	canvas->bounds.x = 0;
	canvas->bounds.y = 0;
	canvas->bounds.w = draw_size.x;
	canvas->bounds.h = draw_size.y;

	// Canvas callbacks
	canvas->callbacks = &callbacks;

	return cast_elem(canvas);
}

static void mgui_canvas_render( MGuiElement* canvas )
{
	if ( canvas->colour.hex != 0 )
	{
		renderer->set_draw_colour( &canvas->colour );
		renderer->draw_rect( canvas->bounds.x, canvas->bounds.y, canvas->bounds.w, canvas->bounds.h, 1.0f );
	}
}

static void mgui_canvas_on_bounds_change( MGuiElement* canvas, bool pos, bool size )
{
	UNREFERENCED_PARAM( pos );
	UNREFERENCED_PARAM( size );

	// Don't change the size.
	canvas->bounds.x = 0;
	canvas->bounds.y = 0;
	canvas->bounds.w = draw_size.x;
	canvas->bounds.h = draw_size.y;
}
