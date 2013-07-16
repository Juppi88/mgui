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
#include "Element.h"
#include "Renderer.h"
#include "SkinSimple.h"
#include "Platform/Alloc.h"
#include "Platform/Timer.h"
#include "Platform/Window.h"
#include "InputHook.h"

vectorscreen_t	draw_size;					// System window size
rectangle_t		draw_rect;					// System window size as a rectangle

static void*	system_window	= NULL;		// Pointer to the system window
MGuiSkin*		defskin			= NULL;		// Pointer to the default (basic) skin
MGuiSkin*		skin			= NULL;		// Current skin
MGuiRenderer*	renderer		= NULL;		// Pointer to the renderer interface
bool			redraw			= true;		// Force a scene redraw (use only with a standalone app)
list_t*			layers			= NULL;		// A list of rendered layers
uint32			tick_count		= 0;		// Current tick count

void mgui_initialize( void* wndhandle )
{
	mgui_input_initialize_hooks();
	mgui_fontmgr_initialize();

	system_window = wndhandle;

	get_window_size( wndhandle, &draw_size.w, &draw_size.h );

	draw_rect.x = 0;
	draw_rect.y = 0;
	draw_rect.w = draw_size.x;
	draw_rect.h = draw_size.y;

	defskin = mgui_setup_skin_simple();
	skin = defskin;

	layers = list_create();
}

void mgui_shutdown( void )
{
	node_t *node, *tmp;

	if ( layers != NULL )
	{
		list_foreach_safe( layers, node, tmp )
		{
			mgui_element_destroy( cast_elem(node) );
		}

		list_destroy( layers );
		layers = NULL;
	}

	// Cleanup the skin
	// Dont cleanup the defskin if its the same as skin else things will go bang
	if ( ( defskin ) && ( defskin != skin ) )
	{
		mem_free( defskin );
	}

	defskin = NULL;

	if ( skin )
	{
		mem_free( skin );
		skin = NULL;
	}

	mgui_fontmgr_shutdown();
	mgui_input_shutdown_hooks();
}

void mgui_process( void )
{
	node_t* node;
	MGuiElement* element;

	tick_count = get_tick_count();

#ifdef MGUI_USE_REDRAW
	static uint32 last_ticks = 0;

	if ( tick_count - last_ticks >= 1000 )
	{
		last_ticks = tick_count;
		mgui_redraw();
	}

	if ( !redraw ) return;
	redraw = false;
#endif

	if ( renderer == NULL ) return;
	if ( layers == NULL ) return;

	renderer->begin();

	// Render all elements on every visible parent control
	list_foreach( layers, node )
	{
		element = cast_elem(node);

		if ( BIT_ON(element->flags, FLAG_VISIBLE) )
		{
			mgui_element_process( element );
			mgui_element_render( element );
		}
	}

	renderer->end();
}

void mgui_redraw( void )
{
	redraw = true;
}

void mgui_set_renderer( MGuiRenderer* rend )
{
	if ( renderer ) mgui_fontmgr_invalidate_all();

	renderer = rend;

	if ( renderer ) mgui_fontmgr_initialize_all();
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
