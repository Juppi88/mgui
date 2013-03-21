/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		MGUI.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		A basic GUI library written in C.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "MGUI.h"
#include "Control.h"
#include "Renderer.h"
#include "SkinSimple.h"
#include "Platform/Alloc.h"
#include "Platform/Timer.h"
#include "Platform/Window.h"
#include "InputHook.h"

static MGUI_RENDERER	renderer_type	= GUIREND_NULL;	// The type of the renderer
static void*			system_window	= NULL;			// Pointer to the system window
vectorscreen_t			draw_size;						// System window size
rectangle_t				draw_rect;						// System window size as a rectangle
MGuiSkin*				defskin			= NULL;			// Pointer to the default (basic) skin
MGuiSkin*				skin			= NULL;			// Current skin
bool					redraw			= true;			// Force a scene redraw

void mgui_initialize( MGUI_RENDERER renderer, void* window )
{
	mgui_render_initialize( renderer, window );
	mgui_input_initialize_hooks();

	system_window = window;
	renderer_type = renderer;

	get_window_size( window, &draw_size.x, &draw_size.y );
	draw_rect.x = 0;
	draw_rect.y = 0;
	draw_rect.w = draw_size.x;
	draw_rect.h = draw_size.y;

	defskin = mgui_setup_skin_simple();
	skin = defskin;
}

void mgui_shutdown( void )
{
	mgui_do_cleanup();
	mgui_render_shutdown();
	mgui_input_shutdown_hooks();
}

void mgui_process( void )
{
	uint32 ticks;
	static uint32 last_ticks;

	ticks = get_tick_count();

	mgui_process_controls( ticks );

#ifdef MGUI_USE_REDRAW
	if ( ticks - last_ticks >= 1000 )
	{
		last_ticks = ticks;
		mgui_redraw();
	}

	if ( !redraw ) return;
	redraw = false;
#endif

	if ( render == NULL ) return;

	render->begin();
	mgui_render_controls();
	render->end();
}

void mgui_redraw( void )
{
	redraw = true;
}

void mgui_set_renderer( MGUI_RENDERER renderer )
{
	if ( render )
		mgui_render_shutdown();

	mgui_render_initialize( renderer, system_window );
	renderer_type = renderer;
}

void mgui_set_skin( const char_t* skinimg )
{
	if ( !skinimg )
	{
		// If the user didn't provide a skin texture, use the default basic skin.
		skin = defskin;
		return;
	}

	// TODO: Add texture loading
}
