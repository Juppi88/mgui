/**
 *
 * @file		Window.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI window related functions.
 *
 * @details		Functions and structures related to GUI windows.
 *
 **/

#include "Window.h"
#include "WindowButton.h"
#include "WindowTitlebar.h"
#include "Renderer.h"
#include "Skin.h"
#include "Font.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

extern MGuiRenderer* renderer;

// --------------------------------------------------

// Window callback handlers
static void		mgui_window_render				( MGuiElement* window );
static void		mgui_window_post_render			( MGuiElement* window );
static void		mgui_window_get_clip_region		( MGuiElement* window, rectangle_t** rect );
static void		mgui_window_on_bounds_change	( MGuiElement* window, bool pos, bool size );
static void		mgui_window_on_flags_change		( MGuiElement* window, uint32 old );
static void		mgui_window_on_colour_change	( MGuiElement* window );
static void		mgui_window_on_mouse_click		( MGuiElement* window, int16 x, int16 y, MOUSEBTN button );
static void		mgui_window_on_mouse_release	( MGuiElement* window, int16 x, int16 y, MOUSEBTN button );
static void		mgui_window_on_mouse_drag		( MGuiElement* window, int16 x, int16 y );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_window_render,
	mgui_window_post_render,
	NULL, /* process */
	mgui_window_get_clip_region,
	mgui_window_on_bounds_change,
	mgui_window_on_flags_change,
	mgui_window_on_colour_change,
	NULL, /* on_text_change */
	NULL, /* on_mouse_enter */
	NULL, /* on_mouse_leave */
	mgui_window_on_mouse_click,
	mgui_window_on_mouse_release,
	mgui_window_on_mouse_drag,
	NULL, /* on_mouse_move */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

// --------------------------------------------------

/**
 * @brief Creates a window.
 *
 * @details This function creates a GUI window. If the parent element
 * is NULL, the window will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @returns A pointer to the created window
 */
MGuiWindow* mgui_create_window( MGuiElement* parent )
{
	struct MGuiWindow* window;
	extern MGuiFont* default_font;

	window = mem_alloc_clean( sizeof(*window) );
	mgui_element_create( cast_elem(window), parent );

	window->flags |= (FLAG_BORDER|FLAG_SHADOW|FLAG_BACKGROUND|FLAG_WINDOW_TITLEBAR|FLAG_WINDOW_CLOSEBTN|FLAG_MOUSECTRL|FLAG_DRAGGABLE|FLAG_WINDOW_RESIZABLE);
	window->type = GUI_WINDOW;
	window->font = default_font;
	window->text->font = default_font;

	window->min_size.w = 100;
	window->min_size.h = 100;

	// We want to have the titlebar enabled by default, so let's add it
	mgui_window_on_flags_change( cast_elem(window), 0 );

	// Window callbacks
	window->callbacks = &callbacks;

	return cast_elem(window);
}

/**
 * @brief Creates a window (extended).
 *
 * @details This function creates a GUI window with the given parameters.
 * If the parent element is NULL, the window will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @param x The absolute x coordinate relative to the parent
 * @param y The absolute y coordinate relative to the parent
 * @param w The absolute width of the window
 * @param h The absolute height of the window
 * @param flags Any additional flags that will be applied as a bitmask (see @ref MGUI_FLAGS)
 * @param col The background colour of the window as a 32bit hex integer
 * @param text The text to be shown on the titlebar
 *
 * @returns A pointer to the created window
 */
MGuiWindow* mgui_create_window_ex( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text )
{
	MGuiWindow* window;

	window = mgui_create_window( parent );

	mgui_set_abs_pos_i( window, x, y );
	mgui_set_abs_size_i( window, w, h );
	mgui_add_flags( window, flags );
	mgui_set_colour_i( window, col );
	mgui_set_text_s( window, text );

	return window;
}

static void mgui_window_render( MGuiElement* window )
{
	window->skin->draw_window( window );
}

static void mgui_window_post_render( MGuiElement* window )
{
	struct MGuiWindow* wnd;
	rectangle_t* r;
	colour_t col;
	uint16 w, h;

	wnd = (struct MGuiWindow*)window;

	if ( wnd->resize_flags == 0 ) return;

	r = &wnd->window_bounds;
	w = wnd->resize_rect.w;
	h = wnd->resize_rect.h;

	colour_invert_no_alpha( &col, &wnd->colour );

	renderer->set_draw_colour( &col );
	renderer->draw_rect( r->x, r->y, 2, h );
	renderer->draw_rect( r->x + w - 2, r->y, 2, h );
	renderer->draw_rect( r->x, r->y, w, 2 );
	renderer->draw_rect( r->x, r->y + h - 2, w, 2 );
}

static void mgui_window_get_clip_region( MGuiElement* window, rectangle_t** rect )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	if ( rect == NULL ) return;

	*rect = &wnd->window_bounds;
}

static void mgui_window_on_bounds_change( MGuiElement* window, bool pos, bool size )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	UNREFERENCED_PARAM( pos );

	if ( wnd->titlebar )
	{
		if ( pos )
		{
			wnd->window_bounds.x = window->bounds.x;
			wnd->window_bounds.y = window->bounds.y;

			wnd->titlebar->bounds.x = window->bounds.x;
			wnd->titlebar->bounds.y = window->bounds.y;
			window->bounds.y += TITLE_H;
		}
		if ( size )
		{
			wnd->window_bounds.w = window->bounds.w;
			wnd->window_bounds.h = window->bounds.h;

			wnd->titlebar->bounds.w = window->bounds.w;
			wnd->titlebar->bounds.h = TITLE_H;
			window->bounds.h -= TITLE_H;
		}

		window->text->bounds = &wnd->titlebar->bounds;
		mgui_text_update_position( window->text );
	}
	else
	{
		wnd->window_bounds = window->bounds;
	}

	if ( wnd->closebtn && wnd->closebtn->callbacks->on_bounds_change )
		wnd->closebtn->callbacks->on_bounds_change( cast_elem(wnd->closebtn), pos, size );
}

static void mgui_window_on_flags_change( MGuiElement* window, uint32 old )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	// Enable the titlebar (and the closebutton if it exists)
	if ( BIT_ENABLED( window->flags, old, FLAG_WINDOW_TITLEBAR ) )
	{
		wnd->titlebar = mgui_create_titlebar( window );

		wnd->titlebar->bounds.x = window->bounds.x;
		wnd->titlebar->bounds.y = window->bounds.y;
		wnd->titlebar->bounds.w = window->bounds.w;
		wnd->titlebar->bounds.h = TITLE_H;

		window->text->bounds = &wnd->titlebar->bounds;

		window->bounds.y += TITLE_H;
		window->bounds.h -= TITLE_H;

		if ( wnd->closebtn )
			wnd->closebtn->flags |= FLAG_VISIBLE;
	}

	// Disable the titlebar and deactivate the close button if required
	else if ( BIT_DISABLED( window->flags, old, FLAG_WINDOW_TITLEBAR ) )
	{
		mgui_element_destroy( cast_elem(wnd->titlebar) );
		wnd->titlebar = NULL;

		window->bounds.y -= TITLE_H;
		window->bounds.h += TITLE_H;

		if ( wnd->closebtn )
			wnd->closebtn->flags &= ~FLAG_VISIBLE;
	}

	// If the titlebar is enabled add a closebutton
	if ( BIT_ENABLED( window->flags, old, FLAG_WINDOW_CLOSEBTN ) )
	{
		if ( BIT_ON( window->flags, FLAG_WINDOW_TITLEBAR ) )
		{
			if ( wnd->closebtn == NULL )
				wnd->closebtn = mgui_create_windowbutton( window );

			wnd->closebtn->callbacks->on_bounds_change( cast_elem(wnd->closebtn), true, true );
		}
	}

	// Disable and destroy the closebutton
	else if ( BIT_DISABLED( window->flags, old, FLAG_WINDOW_CLOSEBTN ) )
	{
		if ( wnd->closebtn != NULL )
		{
			mgui_element_destroy( cast_elem(wnd->closebtn) );
			wnd->closebtn = NULL;
		}
	}

	mgui_element_request_redraw( window );
}

static void mgui_window_on_colour_change( MGuiElement* window )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	if ( wnd->titlebar != NULL )
	{
		wnd->titlebar->colour.a = wnd->colour.a;
		window->text->colour.a = wnd->colour.a;

		if ( wnd->closebtn != NULL )
			wnd->closebtn->colour = wnd->titlebar->colour;
	}
}

static void mgui_window_on_mouse_click( MGuiElement* window, int16 x, int16 y, MOUSEBTN button )
{
	struct MGuiWindow* wnd;
	int16 x_max, y_max;

	UNREFERENCED_PARAM( button );

	if ( BIT_OFF( window->flags, FLAG_WINDOW_RESIZABLE ) ) return;

	wnd = (struct MGuiWindow*)window;
	x_max = wnd->bounds.x + wnd->bounds.w;
	y_max = wnd->bounds.y + wnd->bounds.h;

	if ( x > x_max - 12 && x <= x_max ) wnd->resize_flags |= RESIZE_HORIZ;
	if ( y > y_max - 12 && y <= y_max ) wnd->resize_flags |= RESIZE_VERT;

	wnd->click_offset.x = x;
	wnd->click_offset.y = y;
	wnd->resize_rect.x = wnd->window_bounds.w;
	wnd->resize_rect.y = wnd->window_bounds.h;
}

static void mgui_window_on_mouse_release( MGuiElement* window, int16 x, int16 y, MOUSEBTN button )
{
	struct MGuiWindow* wnd;
	MGuiEvent event;
	node_t* node;

	wnd = (struct MGuiWindow*)window;

	UNREFERENCED_PARAM( button );

	if ( wnd->resize_flags == 0 ) return;

	x = wnd->window_bounds.w + ( ( wnd->resize_flags & RESIZE_HORIZ ) ? x - wnd->click_offset.x : 0 );
	y = wnd->window_bounds.h + ( ( wnd->resize_flags & RESIZE_VERT ) ? y - wnd->click_offset.y : 0 );

	x = ( x < wnd->min_size.w ) ? wnd->min_size.w : x;
	y = ( y < wnd->min_size.h ) ? wnd->min_size.h : y;
	
	wnd->bounds.w = (uint16)x;
	wnd->bounds.h = (uint16)y;

	mgui_window_on_bounds_change( window, false, true );
	mgui_element_request_redraw( window );

	if ( wnd->flags & FLAG_CACHE_TEXTURE )
		mgui_element_resize_cache( window );

	wnd->resize_flags = 0;

	if ( window->event_handler )
	{
		event.type = EVENT_WINDOW_RESIZE;
		event.resize.element = window;
		event.resize.data = window->event_data;
		event.resize.width = wnd->window_bounds.w;
		event.resize.height = wnd->window_bounds.h;

		window->event_handler( &event );
	}

	if ( wnd->children == NULL ) return;

	list_foreach( wnd->children, node )
	{
		mgui_element_update_rel_pos( cast_elem(node) );
	}
}

static void mgui_window_on_mouse_drag( MGuiElement* window, int16 x, int16 y )
{
	struct MGuiWindow* wnd = (struct MGuiWindow*)window;

	if ( wnd->resize_flags == 0 ) return;

	x = wnd->window_bounds.w + ( ( wnd->resize_flags & RESIZE_HORIZ ) ? x - wnd->click_offset.x : 0 );
	y = wnd->window_bounds.h + ( ( wnd->resize_flags & RESIZE_VERT ) ? y - wnd->click_offset.y : 0 );

	wnd->resize_rect.w = ( x < wnd->min_size.w ) ? wnd->min_size.w : x;
	wnd->resize_rect.h = ( y < wnd->min_size.h ) ? wnd->min_size.h : y;

	mgui_element_request_redraw_all();
}

/**
 * @brief Returns the titlebar colour of a window.
 *
 * @details This function returns the colour of the titlebar for a window.
 * If MGUI is using a textureless skin, this colour will also be applied
 * to the border of the window.
 *
 * @param window The window to get the titlebar colour of
 * @param col A pointer to a colour_t struct that will receive the colour
 */
void mgui_window_get_title_colour( MGuiWindow* window, colour_t* col )
{
	MGuiTitlebar* titlebar;

	if ( window == NULL || col == NULL )
		return;

	titlebar = ((struct MGuiWindow*)window)->titlebar;
	if ( titlebar == NULL )
		return;

	col->hex = titlebar->colour.hex;
}

/**
 * @brief Sets the titlebar colour of a window.
 *
 * @details This function sets the colour of the titlebar for a window.
 * If MGUI is using a textureless skin, this colour will also be applied
 * to the border of the window.
 *
 * @param window The window to set the titlebar colour of
 * @param col A pointer to a colour_t struct that contains the new colour
 */
void mgui_window_set_title_colour( MGuiWindow* window, const colour_t* col )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	if ( wnd == NULL || col == NULL )
		return;

	if ( wnd->titlebar == NULL )
		return;

	wnd->titlebar->colour = *col;
	wnd->titlebar->colour.a = window->colour.a;

	if ( wnd->closebtn && wnd->closebtn->callbacks->on_colour_change )
		wnd->closebtn->callbacks->on_colour_change( cast_elem(wnd->closebtn) );

	mgui_element_request_redraw( window );
}

/**
 * @brief Returns the titlebar colour of a window as a 32bit integer.
 *
 * @details This function gets the colour of the titlebar for a window.
 * If MGUI is using a textureless skin, this colour will also be applied
 * to the border of the window. The colour is returned as a 32bit hex
 * integer in format 0xRRGGBBAA.
 *
 * @param window The window to get the titlebar colour of
 * @returns Titlebar colour as a 32bit integer
 */
uint32 mgui_window_get_title_colour_i( MGuiWindow* window )
{
	MGuiTitlebar* titlebar;

	if ( window == NULL )
		return 0;

	titlebar = ((struct MGuiWindow*)window)->titlebar;
	if ( titlebar == NULL )
		return 0;

	return titlebar->colour.hex;
}

/**
 * @brief Sets the titlebar colour of a window as a 32bit integer.
 *
 * @details This function sets the colour of the titlebar for a window.
 * If MGUI is using a textureless skin, this colour will also be applied
 * to the border of the window. The colour is passed as a 32bit hex
 * integer in format 0xRRGGBBAA.
 *
 * @param window The window to set the titlebar colour of
 * @param hex The new titlebar colour as a 32bit integer
 */
void mgui_window_set_title_colour_i( MGuiWindow* window, uint32 hex )
{
	struct MGuiWindow* wnd;
	wnd = (struct MGuiWindow*)window;

	if ( wnd == NULL || wnd->titlebar == NULL )
		return;

	wnd->titlebar->colour.hex = hex;
	wnd->titlebar->colour.a = window->colour.a;

	if ( wnd->closebtn && wnd->closebtn->callbacks->on_colour_change )
		wnd->closebtn->callbacks->on_colour_change( cast_elem(wnd->closebtn) );

	mgui_element_request_redraw( window );
}

/**
 * @brief Returns the click offset for a window when it is being dragged.
 *
 * @details This function returns the offset of user's click when the window
 * is being dragged. This offset is in pixels and relative to the position of
 * the window. Note that the value is not valid if the window is not being dragged.
 *
 * @param window The window to get the drag offset of
 * @param pos A pointer to a vectorscreen_t struct that will receive the offset coordinates
 */
void mgui_window_get_drag_offset( MGuiWindow* window, vectorscreen_t* pos )
{
	if ( window == NULL || pos == NULL )
		return;

	*pos = ((struct MGuiWindow*)window)->click_offset;
}
