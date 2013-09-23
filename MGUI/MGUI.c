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
#include "Texture.h"
#include "Renderer.h"
#include "SkinSimple.h"
#include "SkinTextured.h"
#include "Platform/Alloc.h"
#include "Platform/Timer.h"
#include "Platform/Window.h"
#include "InputHook.h"

// --------------------------------------------------

vectorscreen_t		draw_size;					// System window size
rectangle_t			draw_rect;					// System window size as a rectangle

syswindow_t*		system_window	= NULL;		// Pointer to the system window handle
MGuiSkin*			defskin			= NULL;		// Pointer to the default (basic) skin
MGuiSkin*			skin			= NULL;		// Current skin
MGuiRenderer		renderer_data;				// Renderer interface instance
MGuiRenderer*		renderer		= NULL;		// Pointer to the renderer interface
bool				redraw_all		= true;		// Force a scene redraw (use only with a standalone app)
bool				refresh_all		= false;	// An element has requested a scene redraw
bool				redraw_cache	= false;	// An element has requested a cache redraw
list_t*				layers			= NULL;		// A list of rendered layers
uint32				tick_count		= 0;		// Current tick count
uint32				params			= 0;		// The parameters MGUI was initialized with

// --------------------------------------------------

static void mgui_initialize_elements( void );
static void mgui_invalidate_elements( void );

// --------------------------------------------------

void mgui_initialize( void* wndhandle, uint32 parameters )
{
	if ( parameters & MGUI_PROCESS_INPUT ||
		 parameters & MGUI_HOOK_INPUT )
	{
		// Initialize the input library here
		input_initialize( wndhandle );

		if ( parameters & MGUI_HOOK_INPUT )
			input_enable_hook( true );
	}

	mgui_input_initialize_hooks();
	
	mgui_texturemgr_initialize();
	mgui_fontmgr_initialize();

	system_window = wndhandle;
	params = parameters;

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

	// Cleanup the skin.
	// Dont cleanup the defskin if its the same as skin else things will go bang
	if ( ( defskin ) && ( defskin != skin ) )
	{
		mem_free( defskin );
	}

	defskin = NULL;

	SAFE_DELETE( skin );

	mgui_fontmgr_shutdown();
	mgui_texturemgr_shutdown();

	mgui_input_shutdown_hooks();

	if ( params & MGUI_PROCESS_INPUT ||
		 params & MGUI_HOOK_INPUT )
	{
		// If we initialized the input library we should also shut it down.
		input_shutdown();
	}
}

void mgui_pre_process( void )
{
	node_t* node;
	MGuiElement* element;

	// Do we have cached textures to refresh?
	if ( !redraw_cache ) return;
	if ( renderer == NULL || layers == NULL ) return;

	renderer->begin();
	renderer->set_draw_mode( DRAWING_2D );

	list_foreach( layers, node )
	{
		element = cast_elem(node);

		if ( element->flags_int & INTFLAG_REFRESH &&
			 element->flags & FLAG_VISIBLE )
		{
			mgui_element_render_cache( element, false );
		}
	}

	renderer->end();
	redraw_cache = false;
}

void mgui_process( void )
{
	node_t* node;
	MGuiElement* element;

	if ( params & MGUI_PROCESS_INPUT )
		process_window_messages( system_window, input_process );

	else if ( params & MGUI_HOOK_INPUT )
		input_process( NULL );
	
	tick_count = get_tick_count();

	if ( renderer == NULL ) return;
	if ( layers == NULL ) return;
	
	// Redraw the scene, process and render all elements.
	if ( redraw_all )
	{
		renderer->begin();
		
		list_foreach( layers, node )
		{
			element = cast_elem(node);

			if ( element->flags & FLAG_VISIBLE )
			{
				mgui_element_render( element );
				mgui_element_process( element );
			}
		}

		renderer->end();
	}

	// Do processing only, don't render anything.
	else
	{
		list_foreach( layers, node )
		{
			element = cast_elem(node);

			if ( element->flags & FLAG_VISIBLE )
				mgui_element_process( element );
		}
	}

	if ( params & MGUI_USE_DRAW_EVENT )
	{
		if ( redraw_all )
		{
			// Make sure we don't draw the same window over and over.
			redraw_all = false;
		}

		if ( refresh_all )
		{
			// Make sure that the window is repainted after this frame.
			redraw_all = true;
			refresh_all = false;
		}
	}
}

void mgui_force_redraw( void )
{
	redraw_all = true;
}

void mgui_set_renderer( MGuiRenderer* rend )
{
	if ( renderer )
	{
		// If the old renderer still exists, invalidate everything related to it.
		mgui_fontmgr_invalidate_all();
		mgui_texturemgr_invalidate_all();
		mgui_invalidate_elements();

		renderer = NULL;
	}

	if ( rend )
	{
		// Copy the renderer instance to our internal storage
		renderer_data = *rend;
		renderer = &renderer_data;

		// Initialize everything with the new renderer.
		mgui_texturemgr_initialize_all();
		mgui_fontmgr_initialize_all();
		mgui_initialize_elements();
	}
}

void mgui_set_skin( const char_t* skinimg )
{
	if ( !skinimg )
	{
		// If the user didn't provide a skin texture, use the default basic skin.
		skin = defskin;
		return;
	}

	skin = mgui_setup_skin_textured( skinimg );
	if ( skin == NULL ) skin = defskin;
}

void mgui_screen_pos_to_world( const vector3_t* src, vector3_t* dst )
{
	if ( renderer ) renderer->screen_pos_to_world( src, dst );
}

void mgui_world_pos_to_screen( const vector3_t* src, vector3_t* dst )
{
	if ( renderer ) renderer->world_pos_to_screen( src, dst );
}

static void mgui_initialize_elements( void )
{
	node_t* node;

	if ( layers == NULL ) return;

	// Initialize all renderer dependent resources.
	list_foreach( layers, node )
	{
		mgui_element_initialize( cast_elem(node) );
	}
}

static void mgui_invalidate_elements( void )
{
	node_t* node;

	if ( layers == NULL ) return;

	// Invalidate all renderer dependent resources.
	list_foreach( layers, node )
	{
		mgui_element_invalidate( cast_elem(node) );
	}
}
