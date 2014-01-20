/**
 *
 * @file		Scrollbar.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI scrollbar related functions.
 *
 * @details		Functions and structures related to GUI scrollbars.
 *
 **/

#include "Scrollbar.h"
#include "Skin.h"
#include "Renderer.h"
#include "Platform/Alloc.h"
#include "Platform/Timer.h"

// --------------------------------------------------

// Scrollbar callback handlers
static void		mgui_scrollbar_render			( MGuiScrollbar* scrollbar );
static void		mgui_scrollbar_process			( MGuiScrollbar* scrollbar );
static void		mgui_scrollbar_on_bounds_change	( MGuiScrollbar* scrollbar, bool pos, bool size );
static void		mgui_scrollbar_on_flags_change	( MGuiScrollbar* scrollbar, uint32 old );
static void		mgui_scrollbar_on_colour_change	( MGuiScrollbar* scrollbar );
static void		mgui_scrollbar_on_mouse_leave	( MGuiScrollbar* scrollbar );
static void		mgui_scrollbar_on_mouse_click	( MGuiScrollbar* scrollbar, int16 x, int16 y, MOUSEBTN mousebtn );
static void		mgui_scrollbar_on_mouse_release	( MGuiScrollbar* scrollbar, int16 x, int16 y, MOUSEBTN mousebtn );
static void		mgui_scrollbar_on_mouse_drag	( MGuiScrollbar* scrollbar, int16 x, int16 y );
static void		mgui_scrollbar_on_mouse_move	( MGuiScrollbar* scrollbar, int16 x, int16 y );

static void		mgui_scrollbar_update_bounds	( struct MGuiScrollbar* scrollbar );
static void		mgui_scrollbar_process_nudge	( struct MGuiScrollbar* scrollbar );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_scrollbar_render,
	NULL, /* post_render */
	mgui_scrollbar_process,
	NULL, /* get_clip_region */
	mgui_scrollbar_on_bounds_change,
	mgui_scrollbar_on_flags_change,
	mgui_scrollbar_on_colour_change,
	NULL, /* on_text_change */
	NULL, /* on_mouse_enter */
	mgui_scrollbar_on_mouse_leave,
	mgui_scrollbar_on_mouse_click,
	mgui_scrollbar_on_mouse_release,
	mgui_scrollbar_on_mouse_drag,
	mgui_scrollbar_on_mouse_move,
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

// --------------------------------------------------

/**
 * @brief Creates a scrollbar.
 *
 * @details This function creates a GUI scrollbar. If the parent element
 * is NULL, the scrollbar will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @returns A pointer to the created scrollbar
 */
MGuiScrollbar* mgui_create_scrollbar( MGuiElement* parent )
{
	struct MGuiScrollbar* scrollbar;

	scrollbar = mem_alloc_clean( sizeof(*scrollbar) );
	scrollbar->flags_int |= INTFLAG_NOTEXT;

	mgui_element_create( cast_elem(scrollbar), parent );

	scrollbar->type = GUI_SCROLLBAR;
	scrollbar->flags |= (FLAG_MOUSECTRL|FLAG_DRAGGABLE);

	scrollbar->font = NULL;
	scrollbar->bg_shade = 0.4f;
	scrollbar->content_size = 1.0f;
	scrollbar->step_size = 0.1f;
	scrollbar->bar_position = 0.0f;
	scrollbar->bar_size = 0.25f;

	// Scrollbar callbacks
	scrollbar->callbacks = &callbacks;

	return cast_elem(scrollbar);
}

/**
 * @brief Creates a scrollbar (extended).
 *
 * @details This function creates a GUI scrollbar with the given parameters.
 * If the parent element is NULL, the scrollbar will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @param x The absolute x coordinate relative to the parent
 * @param y The absolute y coordinate relative to the parent
 * @param w The absolute width of the scrollbar
 * @param h The absolute height of the scrollbar
 * @param flags Any additional flags that will be applied as a bitmask (see @ref MGUI_FLAGS)
 * @param col The colour of the scrollbar as a 32bit hex integer
 *
 * @returns A pointer to the created scrollbar
 */
MGuiScrollbar* mgui_create_scrollbar_ex( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col )
{
	MGuiScrollbar* scrollbar;

	scrollbar = mgui_create_scrollbar( parent );

	mgui_set_abs_pos_i( scrollbar, x, y );
	mgui_set_abs_size_i( scrollbar, w, h );
	mgui_add_flags( scrollbar, flags );
	mgui_set_colour_i( scrollbar, col );

	return cast_elem(scrollbar);
}

static void mgui_scrollbar_render( MGuiScrollbar* scrollbar )
{
	scrollbar->skin->draw_scrollbar( scrollbar );
}

static void mgui_scrollbar_process( MGuiScrollbar* scrollbar )
{
	struct MGuiScrollbar* bar = (struct MGuiScrollbar*)scrollbar;
	uint32 ticks, button_flags = SCROLL_BUTTON1_PRESSED|SCROLL_BUTTON2_PRESSED;

	if ( scrollbar == NULL ||
		 bar->nudge_time == 0 )
		 return;

	ticks = get_tick_count();

	if ( BIT_ON( bar->scroll_flags, button_flags ) &&
		 ticks > bar->nudge_time )
	{
		bar->nudge_time = ticks + 100;

		mgui_scrollbar_process_nudge( bar );
		mgui_element_request_redraw( scrollbar );
	}
}

static void mgui_scrollbar_on_bounds_change( MGuiScrollbar* scrollbar, bool pos, bool size )
{
	UNREFERENCED_PARAM( pos );
	UNREFERENCED_PARAM( size );

	mgui_scrollbar_update_bounds( (struct MGuiScrollbar*)scrollbar );
}

static void mgui_scrollbar_on_flags_change( MGuiScrollbar* scrollbar, uint32 old )
{
	if ( BIT_ENABLED( scrollbar->flags, old, FLAG_SCROLLBAR_HORIZ ) ||
		 BIT_DISABLED( scrollbar->flags, old, FLAG_SCROLLBAR_HORIZ ) )
	{
		// We need to re-calculate the boundaries because the direction of the bar was changed.
		mgui_scrollbar_update_bounds( (struct MGuiScrollbar*)scrollbar );
	}
}

static void mgui_scrollbar_on_colour_change( MGuiScrollbar* scrollbar )
{
	struct MGuiScrollbar* bar = (struct MGuiScrollbar*)scrollbar;

	colour_multiply( &bar->bg_colour, &bar->colour, bar->bg_shade );
	bar->bg_colour.a = bar->colour.a;

	mgui_element_request_redraw( scrollbar );
}

static void mgui_scrollbar_on_mouse_leave( MGuiElement* scrollbar )
{
	struct MGuiScrollbar* bar;
	uint32 hover_mask = SCROLL_BAR_HOVER|SCROLL_BUTTON1_HOVER|SCROLL_BUTTON2_HOVER;

	if ( scrollbar == NULL ) return;

	bar = (struct MGuiScrollbar*)scrollbar;

	// Clean up old hover flags.
	if ( bar->scroll_flags & hover_mask )
	{
		bar->scroll_flags &= ~hover_mask;
		mgui_element_request_redraw( scrollbar );
	}
}

static void mgui_scrollbar_on_mouse_click( MGuiScrollbar* scrollbar, int16 x, int16 y, MOUSEBTN mousebtn )
{
	struct MGuiScrollbar* bar;
	uint32 flags = 0, button_mask = SCROLL_BUTTON1_PRESSED|SCROLL_BUTTON2_PRESSED;

	if ( scrollbar == NULL ) return;
	if ( mousebtn != MOUSE_LBUTTON ) return;

	bar = (struct MGuiScrollbar*)scrollbar;

	// Did the user click the bar or one of the buttons?
	if ( rect_is_point_in( &bar->bar, x, y ) )
	{
		flags = SCROLL_BAR_PRESSED;

		bar->click_offset.x = x - bar->bar.x;
		bar->click_offset.y = y - bar->bar.y;
	}

	else if ( rect_is_point_in( &bar->button1, x, y ) )
		flags = SCROLL_BUTTON1_PRESSED;

	else if ( rect_is_point_in( &bar->button2, x, y ) )
		flags = SCROLL_BUTTON2_PRESSED;

	// If something other than the background was clicked, we need to update our bar.
	if ( flags != 0 )
	{
		bar->scroll_flags |= flags;
		mgui_element_request_redraw( scrollbar );

		if ( flags & button_mask )
		{
			// Seems the user is pressing a button, process nudge.
			bar->nudge_time = get_tick_count() + 500;

			mgui_scrollbar_process_nudge( bar );
		}
	}
}

static void mgui_scrollbar_on_mouse_release( MGuiScrollbar* scrollbar, int16 x, int16 y, MOUSEBTN mousebtn )
{
	struct MGuiScrollbar* bar;
	uint32 press_mask = SCROLL_BAR_PRESSED|SCROLL_BUTTON1_PRESSED|SCROLL_BUTTON2_PRESSED;

	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );

	if ( scrollbar == NULL ) return;
	if ( mousebtn != MOUSE_LBUTTON ) return;

	bar = (struct MGuiScrollbar*)scrollbar;

	// Clean up old click flags.
	if ( bar->scroll_flags & press_mask )
	{
		bar->scroll_flags &= ~press_mask;
		bar->nudge_time = 0;

		mgui_element_request_redraw( scrollbar );
	}
}

static void mgui_scrollbar_on_mouse_drag( MGuiScrollbar* scrollbar, int16 x, int16 y )
{
	struct MGuiScrollbar* bar;
	int16 bar_begin, bar_end, bar_pos;
	float position, diff;
	MGuiEvent event;
	
	if ( scrollbar == NULL ) return;

	bar = (struct MGuiScrollbar*)scrollbar;
	if ( BIT_OFF( bar->scroll_flags, SCROLL_BAR_PRESSED ) ) return;

	if ( bar->flags & FLAG_SCROLLBAR_HORIZ )
	{
		// Calculate the absolute position of the bar.
		bar_begin = bar->bounds.x + bar->button1.w;
		bar_end = bar->bounds.x + bar->bounds.w - bar->bar.w - bar->button1.w;
		bar_pos = x - bar->click_offset.x;

		bar->bar.x = (int16)math_clamp( bar_pos, bar_begin, bar_end );

		// Calculate relative position and position change.
		position = (float)( bar->bar.x - bar_begin ) / ( bar_end - bar_begin );
		diff = bar->bar_position;

		bar->bar_position = bar->content_size * math_clampf( position, 0, 1 );
		diff -= bar->bar_position;
	}
	else
	{
		// Calculate the absolute position of the bar.
		bar_begin = bar->bounds.y + bar->button1.h;
		bar_end = bar->bounds.y + bar->bounds.h - bar->bar.h - bar->button1.h;
		bar_pos = y - bar->click_offset.y;

		bar->bar.y = (int16)math_clamp( bar_pos, bar_begin, bar_end );

		// Calculate relative position and position change.
		position = (float)( bar->bar.y - bar_begin ) / ( bar_end - bar_begin );
		diff = bar->bar_position;

		bar->bar_position = bar->content_size * math_clampf( position, 0, 1 );
		diff -= bar->bar_position;
	}

	if ( scrollbar->event_handler )
	{
		event.type = EVENT_SCROLL;
		event.scroll.element = scrollbar;
		event.scroll.data = scrollbar->event_data;
		event.scroll.position = bar->bar_position;
		event.scroll.change = diff;

		scrollbar->event_handler( &event );
	}

	mgui_element_request_redraw( scrollbar );
}

static void mgui_scrollbar_on_mouse_move( MGuiScrollbar* scrollbar, int16 x, int16 y )
{
	struct MGuiScrollbar* bar;
	uint32 hover_mask = SCROLL_BAR_HOVER|SCROLL_BUTTON1_HOVER|SCROLL_BUTTON2_HOVER;
	uint32 flags = 0;

	if ( scrollbar == NULL ) return;

	bar = (struct MGuiScrollbar*)scrollbar;

	// Reset old hover flags.
	if ( bar->scroll_flags & hover_mask )
	{
		bar->scroll_flags &= ~hover_mask;
		mgui_element_request_redraw( scrollbar );
	}

	// Check whether we're touching either the buttons or the bar.
	if ( rect_is_point_in( &bar->bar, x, y ) )
		flags = SCROLL_BAR_HOVER;

	else if ( rect_is_point_in( &bar->button1, x, y ) )
		flags = SCROLL_BUTTON1_HOVER;

	else if ( rect_is_point_in( &bar->button2, x, y ) )
		flags = SCROLL_BUTTON2_HOVER;

	if ( flags != 0 )
	{
		bar->scroll_flags |= flags;
		mgui_element_request_redraw( scrollbar );
	}
}

static void mgui_scrollbar_update_bounds( struct MGuiScrollbar* scrollbar )
{
	uint16 bar_size, button_size;
	float position;

	position = scrollbar->bar_position / scrollbar->content_size;

	if ( scrollbar->flags & FLAG_SCROLLBAR_HORIZ )
	{
		button_size = scrollbar->bounds.h;
		bar_size = scrollbar->bounds.w - 2 * button_size;

		scrollbar->button1.x = scrollbar->bounds.x;
		scrollbar->button1.y = scrollbar->bounds.y;
		scrollbar->button1.w = button_size;
		scrollbar->button1.h = button_size;

		scrollbar->button2.x = scrollbar->bounds.x + scrollbar->bounds.w - button_size;
		scrollbar->button2.y = scrollbar->bounds.y;
		scrollbar->button2.w = button_size;
		scrollbar->button2.h = button_size;

		scrollbar->bar.y = scrollbar->bounds.y;
		scrollbar->bar.h = button_size;
		scrollbar->bar.w = (uint16)( scrollbar->bar_size * bar_size );
		scrollbar->bar.x = scrollbar->bounds.x + button_size + (uint16)( position * ( bar_size - scrollbar->bar.w ) );

		scrollbar->background.x = scrollbar->bounds.x + button_size;
		scrollbar->background.y = scrollbar->bounds.y;
		scrollbar->background.w = bar_size;
		scrollbar->background.h = button_size;
	}
	else
	{
		button_size = scrollbar->bounds.w;
		bar_size = scrollbar->bounds.h - 2 * button_size;

		scrollbar->button1.x = scrollbar->bounds.x;
		scrollbar->button1.y = scrollbar->bounds.y;
		scrollbar->button1.w = button_size;
		scrollbar->button1.h = button_size;

		scrollbar->button2.x = scrollbar->bounds.x;
		scrollbar->button2.y = scrollbar->bounds.y + scrollbar->bounds.h - button_size;
		scrollbar->button2.w = button_size;
		scrollbar->button2.h = button_size;

		scrollbar->bar.x = scrollbar->bounds.x;
		scrollbar->bar.w = button_size;
		scrollbar->bar.h = (uint16)( scrollbar->bar_size * bar_size );
		scrollbar->bar.y = scrollbar->bounds.y + button_size + (uint16)( position * ( bar_size - scrollbar->bar.h ) );

		scrollbar->background.x = scrollbar->bounds.x;
		scrollbar->background.y = scrollbar->bounds.y + button_size;
		scrollbar->background.w = button_size;
		scrollbar->background.h = bar_size;
	}

	mgui_element_request_redraw( cast_elem(scrollbar) );
}

static void mgui_scrollbar_process_nudge( struct MGuiScrollbar* scrollbar )
{
	float diff, position = scrollbar->bar_position;
	int16 bar_size;
	MGuiEvent event;

	// Check the direction of the nudge.
	if ( scrollbar->scroll_flags & SCROLL_BUTTON1_PRESSED )
		position -= scrollbar->step_size;

	else if ( scrollbar->scroll_flags & SCROLL_BUTTON2_PRESSED )
		position += scrollbar->step_size;

	// Make sure the bar is still within the boundaries.
	position = math_clampf( position, 0, scrollbar->content_size );

	diff = scrollbar->bar_position - position;
	scrollbar->bar_position = position;

	if ( scrollbar->flags & FLAG_SCROLLBAR_HORIZ )
	{
		// Horizontal scrollbar
		bar_size = scrollbar->bounds.w - 2 * scrollbar->button1.w;

		scrollbar->bar.x = scrollbar->bounds.x + scrollbar->button1.w +
						   (uint16)( position * ( bar_size - scrollbar->bar.w ) / scrollbar->content_size );	
	}
	else
	{
		// Vertical scrollbar
		bar_size = scrollbar->bounds.h - 2 * scrollbar->button1.h;

		scrollbar->bar.y = scrollbar->bounds.y + scrollbar->button1.h +
						   (uint16)( position * ( bar_size - scrollbar->bar.h ) / scrollbar->content_size );
	}

	if ( scrollbar->event_handler )
	{
		event.type = EVENT_SCROLL;
		event.scroll.element = cast_elem(scrollbar);
		event.scroll.data = scrollbar->event_data;
		event.scroll.position = position;
		event.scroll.change = diff;

		scrollbar->event_handler( &event );
	}
}

/**
 * @brief Sets all scrollbar parameters with a single function.
 *
 * @details This function can be used to set all scrollbar parameters
 * with a single function call.
 *
 * @param scrollbar The scrollbar to set the parameters of
 * @param content The size of the content
 * @param step The step amount that clicking a button will cause
 * @param position The position of the bar as an offset from the top/left edge (scaled between 0 and content)
 * @param size The size of actual bar, scaled between 0 and 1
 */
void mgui_scrollbar_set_params( MGuiScrollbar* scrollbar, float content, float step, float position, float size )
{
	struct MGuiScrollbar* bar;

	if ( scrollbar == NULL )
		return;

	bar = (struct MGuiScrollbar*)scrollbar;

	bar->content_size = content;
	bar->step_size = step;
	bar->bar_size = math_clampf( size, 0, 1 );
	bar->bar_position = math_clampf( position, 0, content );

	mgui_scrollbar_update_bounds( bar );
}

/**
 * @brief Returns the content size of a scrollbar.
 *
 * @details This function returns the size of the content for a scrollbar.
 * Content size is used as a reference value for the position of the bar.
 *
 * @param scrollbar The scrollbar to get the content size of
 * @returns Current content size
 */
float mgui_scrollbar_get_content_size( MGuiScrollbar* scrollbar )
{
	struct MGuiScrollbar* bar;

	if ( scrollbar == NULL )
		return 0;

	bar = (struct MGuiScrollbar*)scrollbar;
	return bar->content_size;
}

/**
* @brief Sets the content size of a scrollbar.
*
* @details This function sets the size of the content for a scrollbar.
* Content size is used as a reference value for the position of the bar.
*
* @param scrollbar The scrollbar to set the content size of
* @param size The size of the content
*/
void mgui_scrollbar_set_content_size( MGuiScrollbar* scrollbar, float size )
{
	struct MGuiScrollbar* bar;

	if ( scrollbar == NULL )
		return;

	bar = (struct MGuiScrollbar*)scrollbar;
	bar->content_size = size;
	bar->bar_position = math_clampf( bar->bar_position, 0, size );

	mgui_scrollbar_update_bounds( bar );
}

/**
 * @brief Returns the step size of a scrollbar.
 *
 * @details This function returns the step size of a scrollbar.
 * Step size is the distance the bar moves when one of the scrollbar
 * buttons is pressed.
 *
 * @param scrollbar The scrollbar to get the step size of
 * @returns Current step size
 */
float mgui_scrollbar_get_step_size( MGuiScrollbar* scrollbar )
{
	struct MGuiScrollbar* bar;

	if ( scrollbar == NULL )
		return 0;

	bar = (struct MGuiScrollbar*)scrollbar;
	return bar->step_size;
}

/**
 * @brief Sets the step size of a scrollbar.
 *
 * @details This function sets the step size of a scrollbar.
 * Step size is the distance the bar moves when one of the scrollbar
 * buttons is pressed.
 *
 * @param scrollbar The scrollbar to set the step size of
 * @param size The new step size
 */
void mgui_scrollbar_set_step_size( MGuiScrollbar* scrollbar, float size )
{
	struct MGuiScrollbar* bar;

	if ( scrollbar == NULL )
		return;

	bar = (struct MGuiScrollbar*)scrollbar;
	bar->step_size = size;
}

/**
 * @brief Returns the bar position of a scrollbar.
 *
 * @details This function returns the position of the bar for a scrollbar.
 * The position is scaled between 0 and the size of the content.
 *
 * @param scrollbar The scrollbar to get the bar position of
 * @returns Current bar position
 */
float mgui_scrollbar_get_bar_pos( MGuiScrollbar* scrollbar )
{
	struct MGuiScrollbar* bar;

	if ( scrollbar == NULL )
		return 0;

	bar = (struct MGuiScrollbar*)scrollbar;
	return bar->bar_position;
}

/**
 * @brief Sets the bar position of a scrollbar.
 *
 * @details This function sets the position of the bar for a scrollbar.
 * The position is scaled between 0 and the size of the content.
 *
 * @param scrollbar The scrollbar to set the bar position of
 * @param position The new position for the bar
 */
void mgui_scrollbar_set_bar_pos( MGuiScrollbar* scrollbar, float position )
{
	struct MGuiScrollbar* bar;

	if ( scrollbar == NULL )
		return;

	bar = (struct MGuiScrollbar*)scrollbar;
	bar->bar_position = math_clampf( position, 0, bar->content_size );

	mgui_scrollbar_update_bounds( bar );
}

/**
 * @brief Returns the bar size of a scrollbar.
 *
 * @details This function returns the size of the actual bar for a scrollbar.
 * The size is relative to the absolute size of the bar and it is scaled
 * between 0 and 1.
 *
 * @param scrollbar The scrollbar to get the bar size of
 * @returns Bar size scaled between 0 and 1
 */
float mgui_scrollbar_get_bar_size( MGuiScrollbar* scrollbar )
{
	struct MGuiScrollbar* bar;

	if ( scrollbar == NULL )
		return 0;

	bar = (struct MGuiScrollbar*)scrollbar;
	return bar->bar_size;
}

/**
 * @brief Sets the bar size of a scrollbar.
 *
 * @details This function sets the size of the actual bar for a scrollbar.
 * The size is relative to the absolute size of the bar and it is scaled
 * between 0 and 1.
 *
 * @param scrollbar The scrollbar to set the bar size of
 * @param size The new size of the bar
 */
void mgui_scrollbar_set_bar_size( MGuiScrollbar* scrollbar, float size )
{
	struct MGuiScrollbar* bar;

	if ( scrollbar == NULL ) return;

	size = math_clampf( size, 0, 1 );

	bar = (struct MGuiScrollbar*)scrollbar;
	bar->bar_size = size;

	mgui_element_request_redraw( scrollbar );
}

/**
 * @brief Returns the background shade of a scrollbar.
 *
 * @details This function returns the background shade of a scrollbar.
 * The shade is a percentage of the main colour (scaled between 0 and 1).
 *
 * @param scrollbar The scrollbar to get the background shade of
 * @returns Current background shade
 */
float mgui_scrollbar_get_bg_shade( MGuiScrollbar* scrollbar )
{
	struct MGuiScrollbar* bar;

	if ( scrollbar == NULL )
		return 0;

	bar = (struct MGuiScrollbar*)scrollbar;
	return bar->bg_shade;
}

/**
 * @brief Sets the background shade of a scrollbar.
 *
 * @details This function sets the background of a scrollbar. The shade
 * is a percentage of the main colour (scaled between 0 and 1).
 *
 * @param scrollbar The scrollbar to set the background shade of
 * @param shade The new background shade
 */
void mgui_scrollbar_set_bg_shade( MGuiScrollbar* scrollbar, float shade )
{
	struct MGuiScrollbar* bar;

	if ( scrollbar == NULL ) return;

	shade = math_clampf( shade, 0, 1 );

	bar = (struct MGuiScrollbar*)scrollbar;
	bar->bg_shade = shade;

	colour_multiply( &bar->bg_colour, &bar->colour, bar->bg_shade );
	bar->bg_colour.a = bar->colour.a;

	mgui_element_request_redraw( scrollbar );
}
