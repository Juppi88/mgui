/**
 *
 * @file		MGUI.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		MGUI core functions.
 *
 * @details		Functions to initialize and run MGUI.
 *
 **/

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

/**
 * @brief Initializes the library.
 *
 * @details This function initializes Mylly GUI and prepares it for
 * the application. It must always be called before calling anything else.
 * You can use this function to hook user input automatically, or force
 * MGUI to redraw the window only when there is something new to draw.
 *
 * @param wndhandle A handle to the window that MGUI will draw to (HWND on Windows, syswindow_t on linux - see Lib-Platform)
 * @param parameters - A bitfield for special initialization parameters (see @ref MGUI_PARAMETERS)
 * @sa MGUI_PARAMETERS
 */
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

	// Get the initial size of the window.
	get_window_drawable_size( wndhandle, &draw_size.ux, &draw_size.uy );

	draw_rect.x = 0;
	draw_rect.y = 0;
	draw_rect.w = draw_size.ux;
	draw_rect.h = draw_size.uy;

	defskin = mgui_setup_skin_simple();
	skin = defskin;

	layers = list_create();
}

/**
 * @brief Shuts down the library and cleans up memory.
 *
 * @details This function shuts down Mylly GUI frees all the memory
 * allocated by it. After calling this function, no other MGUI function
 * should be called.
 */
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

	// Cleanup the skin. Don't destroy the default skin if its the same
	// as the current skin or things will go bang.
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

/**
 * @brief Pre-processes all elements.
 *
 * @details This function refreshes texture caches for all the elements that
 * use them. It should be called from the application's main loop. It is not
 * necessary to call this function if no element uses texture caching.
 */
void mgui_pre_process( void )
{
	node_t* node;
	MGuiElement* element;

	// Do we have cached textures to refresh?
	if ( !redraw_cache )
		return;

	// Make sure the library is actually initialized and we have a valid renderer.
	if ( renderer == NULL || layers == NULL )
		return;

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

/**
 * @brief Processes the library.
 *
 * @details This function processes everything within the library
 * (drawing, user input, animations). It should be called from the
 * application's main loop.
 */
void mgui_process( void )
{
	node_t* node;
	MGuiElement* element;

	if ( params & MGUI_PROCESS_INPUT )
		process_window_messages( system_window, input_process );

	else if ( params & MGUI_HOOK_INPUT )
		input_process( NULL );
	
	tick_count = get_tick_count();

	if ( renderer == NULL || layers == NULL )
		return;
	
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

/**
 * @brief Forces a window redraw.
 *
 * @details This function forces the application to redraw the current window.
 * If MGUI was not initialized with the parameter @ref MGUI_USE_DRAW_EVENT,
 * this function will not do anything.
 *
 * @sa mgui_initialize
 */
void mgui_force_redraw( void )
{
	redraw_all = true;
}

/**
 * @brief Sets the renderer.
 *
 * @details This function can be used to set or change the renderer which is
 * used to draw into the window. If a renderer has already been set, all data
 * related to that renderer will be deestroyed and reloaded with the new renderer.
 *
 * @param rend A pointer to a valid @ref MGuiRenderer struct
 */
void mgui_set_renderer( MGuiRenderer* rend )
{
	if ( renderer != NULL )
	{
		// If the old renderer still exists, invalidate everything related to it.
		mgui_fontmgr_invalidate_all();
		mgui_texturemgr_invalidate_all();
		mgui_invalidate_elements();

		renderer = NULL;
	}

	if ( rend != NULL )
	{
		// Copy the renderer instance to our internal storage
		renderer_data = *rend;
		renderer = &renderer_data;

		// Initialize everything with the new renderer.
		mgui_texturemgr_initialize_all();
		mgui_fontmgr_initialize_all();
		mgui_initialize_elements();
	}
	else
	{
		// Make sure that when the renderer is set to NULL we don't refer to an invalid pointer.
		renderer = NULL;
	}
}

/**
 * @brief Changes the GUI skin.
 *
 * @details This function can be used to change the skin that MGUI uses.
 * You can choose between a textured skin (loaded from an image file) or
 * a basic textureless skin.
 *
 * @param skinimg Filename of a valid skin texture, or NULL for a textureless skin
 */
void mgui_set_skin( const char_t* skinimg )
{
	if ( skinimg == NULL || *skinimg == '\0' )
	{
		// If the user didn't provide a skin texture, use the default basic skin.
		skin = defskin;
		return;
	}

	skin = mgui_setup_skin_textured( skinimg );

	if ( skin == NULL )
		skin = defskin;
}

/**
 * @brief Lets MGUI know that the size of the window has changed.
 *
 * @details This function can be used to inform MGUI about a change
 * in the size of the window it is drawing to.
 *
 * @param width The new width of the drawable area
 * @param height The new height of the drawable area
 */
void mgui_resize( uint16 width, uint16 height )
{
	node_t *node, *node2;
	MGuiElement* element;
	MGuiRenderer* tmprend;
	bool reset;

	draw_rect.w = draw_size.w = width;
	draw_rect.h = draw_size.h = height;

	if ( layers == NULL ) return;

	if ( renderer != NULL )
	{
		tmprend = renderer;
		reset = BIT_ON( renderer->properties, REND_RESET_ON_RESIZE );

		// Invalidate everything (fonts, textures, render caches).
		if ( reset )
			mgui_set_renderer( NULL );

		// Let the renderer know the new window size.
		tmprend->resize( width, height ); 

		// Reinitialize everything (fonts, textures, render caches).
		if ( reset )
			mgui_set_renderer( tmprend );
	}

	// Update all canvases.
	list_foreach( layers, node )
	{
		element = cast_elem(node);

		if ( element->type == GUI_CANVAS )
		{
			if ( element->children == NULL ) continue;

			list_foreach( element->children, node2 )
			{
				mgui_element_update_child_pos( cast_elem(node2) );
			}

		}
	}

	mgui_element_request_redraw_all();
}

/**
 * @brief Converts on-screen coordinates to 3D world coordinates.
 *
 * @details This function converts window coordinates to 3D world
 * coordinates. The z field of the screen coordinate means distance
 * from the screen, 1 being at the monitor level.
 *
 * @param src A pointer to a vector3_t struct that contains the on-screen coordinates
 * @param dst A pointer to a vector3_t struct that will receive the 3D world coordinates
 */
void mgui_screen_pos_to_world( const vector3_t* src, vector3_t* dst )
{
	if ( renderer != NULL )
		renderer->screen_pos_to_world( src, dst );
}

/**
 * @brief Converts 3D world coordinates to on-screen coordinates.
 *
 * @details This function converts 3D world coordinates to window
 * coordinates. The z field of the screen coordinate means distance
 * from the screen, 1 being at the monitor level.
 *
 * @param src A pointer to a vector3_t struct that contains the 3D world coordinates
 * @param dst A pointer to a vector3_t struct that will receive the on-screen coordinates
 */
void mgui_world_pos_to_screen( const vector3_t* src, vector3_t* dst )
{
	if ( renderer != NULL )
		renderer->world_pos_to_screen( src, dst );
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
