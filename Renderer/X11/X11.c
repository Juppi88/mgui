/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (X11)
 * FILE:		X11.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		An X11 renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "X11.h"
#include "Drawing.h"
#include "Platform/Window.h"
#include <string.h>

static MGuiRenderer renderer;
syswindow_t* window = NULL;
GC gc;

MGuiRenderer* mgui_x11_initialize( void* syswindow )
{
	XGCValues values;
	uint32 mask;

	renderer.begin				= mgui_x11_begin;
	renderer.end				= mgui_x11_end;
	renderer.resize				= mgui_x11_resize;
	renderer.set_draw_colour	= mgui_x11_set_draw_colour;
	renderer.start_clip			= mgui_x11_start_clip;
	renderer.end_clip			= mgui_x11_end_clip;
	renderer.draw_rect			= mgui_x11_draw_rect;
	renderer.draw_triangle		= mgui_x11_draw_triangle;
	renderer.load_texture		= mgui_x11_load_texture;
	renderer.destroy_texture	= mgui_x11_destroy_texture;
	renderer.draw_textured_rect	= mgui_x11_draw_textured_rect;
	renderer.load_font			= mgui_x11_load_font;
	renderer.destroy_font		= mgui_x11_destroy_font;
	renderer.draw_text			= mgui_x11_draw_text;
	renderer.measure_text		= mgui_x11_measure_text;

	window = syswindow;

	memset( &values, 0, sizeof(values) );
	values.background = 0xFF000000;

	mask = GCCapStyle|GCJoinStyle;

	gc = XCreateGC( window->display, window->window, mask, &values );
	if ( gc < 0 ) return NULL;

	XSetFillStyle( window->display, gc, FillSolid );

	return &renderer;
}

void mgui_x11_shutdown( void )
{
	XFreeGC( window->display, gc );

	window = NULL;
}
