/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (OpenGL)
 * FILE:		OpenGL.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An OpenGL renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "OpenGL.h"
#include "Drawing.h"

static mgui_renderer_t		renderer;

mgui_renderer_t* mgui_opengl_initialize( void* syswindow )
{
	renderer.type = GUIREND_OPENGL;
	renderer.device_context = mgui_opengl_create_context( syswindow );

	renderer.begin				= mgui_opengl_begin;
	renderer.end				= mgui_opengl_end;
	renderer.resize				= mgui_opengl_resize;
	renderer.set_draw_colour	= mgui_opengl_set_draw_colour;
	renderer.start_clip			= mgui_opengl_start_clip;
	renderer.end_clip			= mgui_opengl_end_clip;
	renderer.draw_rect			= mgui_opengl_draw_rect;
	renderer.draw_triangle		= mgui_opengl_draw_triangle;
	renderer.load_texture		= mgui_opengl_load_texture;
	renderer.destroy_texture	= mgui_opengl_destroy_texture;
	renderer.draw_textured_rect	= mgui_opengl_draw_textured_rect;
	renderer.load_font			= mgui_opengl_load_font;
	renderer.destroy_font		= mgui_opengl_destroy_font;
	renderer.draw_text			= mgui_opengl_draw_text;
	renderer.measure_text		= mgui_opengl_measure_text;

	return &renderer;
}

void mgui_opengl_shutdown( void )
{
	mgui_opengl_destroy_context( renderer.device_context );
	renderer.device_context = NULL;
}
