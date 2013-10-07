/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (Xlib)
 * FILE:		Xlib.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An Xlib renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "Xlib.h"
#include "Renderer.h"
#include "Platform/Window.h"
#include <string.h>

// --------------------------------------------------

static MGuiRenderer	renderer;
syswindow_t*		window		= NULL;
bool				initialized	= false;
GC					gc;

// --------------------------------------------------

MGuiRenderer* mgui_xlib_initialize( void* syswindow )
{
	XGCValues values;
	uint32 mask;

	renderer.begin					= renderer_begin;
	renderer.end					= renderer_end;
	renderer.resize					= renderer_resize;
	renderer.set_draw_mode			= renderer_set_draw_mode;
	renderer.set_draw_colour		= renderer_set_draw_colour;
	renderer.set_draw_depth			= renderer_set_draw_depth;
	renderer.set_draw_transform		= renderer_set_draw_transform;
	renderer.reset_draw_transform	= renderer_reset_draw_transform;
	renderer.start_clip				= renderer_start_clip;
	renderer.end_clip				= renderer_end_clip;
	renderer.draw_rect				= renderer_draw_rect;
	renderer.draw_triangle			= renderer_draw_triangle;
	renderer.draw_pixel				= renderer_draw_pixel;
	renderer.load_texture			= renderer_load_texture;
	renderer.destroy_texture		= renderer_destroy_texture;
	renderer.draw_textured_rect		= renderer_draw_textured_rect;
	renderer.load_font				= renderer_load_font;
	renderer.destroy_font			= renderer_destroy_font;
	renderer.draw_text				= renderer_draw_text;
	renderer.measure_text			= renderer_measure_text;
	renderer.create_render_target	= renderer_create_render_target;
	renderer.destroy_render_target	= renderer_destroy_render_target;
	renderer.draw_render_target		= renderer_draw_render_target;
	renderer.enable_render_target	= renderer_enable_render_target;
	renderer.disable_render_target	= renderer_disable_render_target;
	renderer.screen_pos_to_world	= renderer_screen_pos_to_world;
	renderer.world_pos_to_screen	= renderer_world_pos_to_screen;

	window = syswindow;

	memset( &values, 0, sizeof(values) );
	values.background = 0xFF000000;

	mask = GCCapStyle|GCJoinStyle;

	gc = XCreateGC( window->display, window->window, mask, &values );
	if ( gc < 0 ) return NULL;

	XSetFillStyle( window->display, gc, FillSolid );

	initialized = true;

	return &renderer;
}

void mgui_xlib_shutdown( void )
{
	if ( !initialized ) return;

	XFreeGC( window->display, gc );

	window = NULL;
	initialized = false;
}

void mgui_xlib_begin_scene( void )
{
	// Nothing to do here, everything is processed by the renderer.
}

void mgui_xlib_end_scene( void )
{
	// Nothing to do here, everything is processed by the renderer.
}
