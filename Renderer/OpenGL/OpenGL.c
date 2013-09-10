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

static MGuiRenderer renderer;
static void* context = NULL;

MGuiRenderer* mgui_opengl_initialize( void* syswindow )
{
	context = mgui_opengl_create_context( syswindow );

	renderer.begin					= mgui_opengl_begin;
	renderer.end					= mgui_opengl_end;
	renderer.resize					= mgui_opengl_resize;
	renderer.set_draw_mode			= mgui_opengl_set_draw_mode;
	renderer.set_draw_colour		= mgui_opengl_set_draw_colour;
	renderer.set_draw_depth			= mgui_opengl_set_draw_depth;
	renderer.set_draw_transform		= mgui_opengl_set_draw_transform;
	renderer.reset_draw_transform	= mgui_opengl_reset_draw_transform;
	renderer.start_clip				= mgui_opengl_start_clip;
	renderer.end_clip				= mgui_opengl_end_clip;
	renderer.draw_rect				= mgui_opengl_draw_rect;
	renderer.draw_triangle			= mgui_opengl_draw_triangle;
	renderer.draw_pixel				= mgui_opengl_draw_pixel;
	renderer.load_texture			= mgui_opengl_load_texture;
	renderer.destroy_texture		= mgui_opengl_destroy_texture;
	renderer.draw_textured_rect		= mgui_opengl_draw_textured_rect;
	renderer.load_font				= mgui_opengl_load_font;
	renderer.destroy_font			= mgui_opengl_destroy_font;
	renderer.draw_text				= mgui_opengl_draw_text;
	renderer.measure_text			= mgui_opengl_measure_text;
	renderer.screen_pos_to_world	= mgui_opengl_screen_pos_to_world;
	renderer.world_pos_to_screen	= mgui_opengl_world_pos_to_screen;

	return &renderer;
}

void mgui_opengl_shutdown( void )
{
	mgui_opengl_destroy_context( context );
	context = NULL;
}
