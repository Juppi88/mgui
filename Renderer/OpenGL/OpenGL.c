/**********************************************************************
 *
 * PROJECT:		Mylly GUI - OpenGL Renderer
 * FILE:		OpenGL.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An OpenGL reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "OpenGL.h"
#include "Extensions.h"
#include "Renderer.h"
#include "Platform/Utils.h"

// --------------------------------------------------

static MGuiRenderer	renderer;
static bool			initialized	= false;
static HWND			hwnd		= NULL;

// --------------------------------------------------

void	mgui_opengl_initialize_system	( void* window );
void	mgui_opengl_shutdown_system		( void );

// --------------------------------------------------

MGuiRenderer* mgui_opengl_initialize( void* window )
{
	// Do platform specific initialization.
	mgui_opengl_initialize_system( window );

	// Initialize extensions.
	mgui_opengl_initialize_extensions();

	renderer.properties = REND_SUPPORTS_TEXTTAGS |
						  REND_SUPPORTS_TEXTURES;
						  REND_SUPPORTS_TARGETS;

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

	renderer_initialize();

	initialized = true;

	return &renderer;
}

void mgui_opengl_shutdown( void )
{
	if ( !initialized ) return;

	renderer_shutdown();

	initialized = false;
}

void mgui_opengl_begin_scene( void )
{
}

void mgui_opengl_end_scene( void )
{
	// If we're doing rendering each frame, swap buffers here.
	// Because we're not (we only draw the window when there is something to draw),
	// this is done in the actual renderer code (renderer_end)
}
