/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		MGUI.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		A basic GUI library written in C.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "MGUI.h"
#include "Control.h"
#include "Renderer.h"
#include "Skin.h"
#include "Platform/Platform.h"
#include "InputHook.h"

static enum MGUI_RENDERER	renderer_type	= GUIREND_NULL;	// The type of the renderer
static void*				system_window	= NULL;			// Pointer to the system window
vectorscreen_t				screen_size;					// System window size
rectangle_t					screen_rect;					// System window size as a rectangle
skin_t*						defskin			= NULL;			// Pointer to the default (basic) skin
skin_t*						skin			= NULL;			// Current skin
bool						redraw			= true;			// Force a scene redraw


void mgui_initialize( MGUI_RENDERER renderer, void* window )
{
	mgui_render_initialize( renderer, window );
	mgui_input_initialize_hooks();

	system_window = window;
	renderer_type = renderer;

	get_window_size( window, &screen_size.x, &screen_size.y );
	screen_rect.x = 0;
	screen_rect.y = 0;
	screen_rect.w = screen_size.x;
	screen_rect.h = screen_size.y;

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
		// If the user didn't provide a skin image, use the default basic skin.
		skin = defskin;
		return;
	}

	// TODO: Add texture loading
}
