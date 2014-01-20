/**
 *
 * @file		Canvas.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI canvas related functions.
 *
 * @details		Functions and structures related to GUI canvases.
 *
 **/

#include "Canvas.h"
#include "Renderer.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

extern vectorscreen_t draw_size;
extern MGuiRenderer* renderer;

// --------------------------------------------------

// Canvas callback handlers
static void		mgui_canvas_render				( MGuiElement* canvas );
static void		mgui_canvas_on_bounds_change	( MGuiElement* canvas, bool pos, bool size );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_canvas_render,
	NULL, /* post_render */
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
	NULL, /* on_mouse_move */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

// --------------------------------------------------

/**
 * @brief Creates a canvas.
 *
 * @details This function creates a GUI canvas. Canvas is an invisible
 * element that can be used as a parent for everything else. If the parent
 * element is NULL, the canvas will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @returns A pointer to the created canvas
 */
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
		renderer->draw_rect( canvas->bounds.x, canvas->bounds.y, canvas->bounds.w, canvas->bounds.h );
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
