/**
 *
 * @file		Progressbar.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI progressbar related functions.
 *
 * @details		Functions and structures related to GUI progressbars.
 *
 **/

#include "Progressbar.h"
#include "Skin.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

// Progressbar callback handlers and static methods
static void		mgui_progressbar_render				( MGuiProgressbar* bar );
static void		mgui_progressbar_on_colour_change	( MGuiProgressbar* bar );
static void		mgui_progressbar_update_colours		( struct MGuiProgressbar* bar );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_progressbar_render,
	NULL, /* post_render */
	NULL, /* process */
	NULL, /* get_clip_region */
	NULL, /* on_bounds_change */
	NULL, /* on_flags_change */
	mgui_progressbar_on_colour_change,
	NULL, /* on_text_change */
	NULL, /* on_mouse_enter */
	NULL, /* on_mouse_leave */
	NULL, /* on_mouse_click */
	NULL, /* on_mouse_release */
	NULL, /* on_mouse_drag */
	NULL, /* on_mouse_move */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

// --------------------------------------------------

/**
 * @brief Creates a progressbar.
 *
 * @details This function creates a GUI progressbar. If the parent element
 * is NULL, the progressbar will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @returns A pointer to the created progressbar
 */
MGuiProgressbar* mgui_create_progressbar( MGuiElement* parent )
{
	struct MGuiProgressbar* bar;

	bar = mem_alloc_clean( sizeof(*bar) );
	bar->flags_int |= INTFLAG_NOTEXT;

	mgui_element_create( cast_elem(bar), parent );

	bar->flags |= FLAG_BACKGROUND|FLAG_BORDER;

	bar->type = GUI_PROGRESSBAR;
	bar->value = 1;
	bar->max_value = 1;
	bar->bg_shade = 0.5f;
	bar->thickness = 1;

	// Progressbar callbacks
	bar->callbacks = &callbacks;

	return cast_elem(bar);
}

/**
 * @brief Creates a progressbar (extended).
 *
 * @details This function creates a GUI progressbar with the given parameters.
 * If the parent element is NULL, the progressbar will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @param x The absolute x coordinate relative to the parent
 * @param y The absolute y coordinate relative to the parent
 * @param w The absolute width of the progressbar
 * @param h The absolute height of the progressbar
 * @param flags Any additional flags that will be applied as a bitmask (see @ref MGUI_FLAGS)
 * @param col1 The start colour of the progressbar as a 32bit hex integer
 * @param col2 The end colour of the progressbar as a 32bit hex integer
 * @param max_value The maximum value that the progressbar can reach
 *
 * @returns A pointer to the created progressbar
 */
MGuiProgressbar* mgui_create_progressbar_ex( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col1, uint32 col2, float max_value )
{
	MGuiProgressbar* bar;

	bar = mgui_create_progressbar( parent );

	mgui_set_abs_pos_i( bar, x, y );
	mgui_set_abs_size_i( bar, w, h );
	mgui_add_flags( bar, flags );
	mgui_progressbar_set_colour_i( bar, col1, col2 );
	mgui_progressbar_set_max_value( bar, max_value );
	
	return bar;
}

static void mgui_progressbar_render( MGuiProgressbar* bar )
{
	bar->skin->draw_progressbar( bar );
}

static void mgui_progressbar_on_colour_change( MGuiProgressbar* bar )
{
	struct MGuiProgressbar* progbar;
	uint8 alpha;

	progbar = (struct MGuiProgressbar*)bar;

	// Only apply new alpha here, progressbars have
	// separate methods for setting the actual colours.
	alpha = progbar->colour.a;

	progbar->colour_start.a = alpha;
	progbar->colour_end.a = alpha;
	progbar->colour_fg.a = alpha;
	progbar->colour_bg.a = alpha;

	mgui_element_request_redraw( bar );
}

static void mgui_progressbar_update_colours( struct MGuiProgressbar* bar )
{
	float percentage;

	if ( bar == NULL ) return;

	percentage = bar->value / bar->max_value;
	percentage = math_clampf( percentage, 0, 1 );

	colour_lerp_no_alpha( &bar->colour_fg, &bar->colour_start, &bar->colour_end, percentage );
	colour_multiply( &bar->colour_bg, &bar->colour_fg, bar->bg_shade );

	mgui_progressbar_on_colour_change( cast_elem(bar) );
}

/**
 * @brief Returns the current value of a progressbar.
 *
 * @details This function returns the current value of a progressbar.
 * The value is between 0 and the maximum value of the progressbar.
 *
 * @param bar The progressbar to get the value of
 * @returns Current value of the progressbar
 */
float mgui_progressbar_get_value( MGuiProgressbar* bar )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL )
		return 0;

	progbar = (struct MGuiProgressbar*)bar;
	return progbar->value;
}

/**
 * @brief Sets the current value of a progressbar.
 *
 * @details This function sets the current value of a progressbar.
 * The value is between 0 and the maximum value of the progressbar.
 *
 * @param bar The progressbar to set the value of
 * @param value The new value
 */
void mgui_progressbar_set_value( MGuiProgressbar* bar, float value )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL )
		return;

	progbar = (struct MGuiProgressbar*)bar;

	value = math_clampf( value, 0, progbar->max_value );
	progbar->value = value;

	mgui_progressbar_update_colours( progbar );
}

/**
 * @brief Returns the current maximum value of a progressbar.
 *
 * @details This function returns the current maximum value of a progressbar.
 *
 * @param bar The progressbar to get the maximum value of
 * @returns Current maximum value of the progressbar
 */
float mgui_progressbar_get_max_value( MGuiProgressbar* bar )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL )
		return 0;

	progbar = (struct MGuiProgressbar*)bar;
	return progbar->max_value;
}

/**
 * @brief Sets the maximum value of a progressbar.
 *
 * @details This function sets the maximum value of a progressbar.
 * If the new maximum value is smaller than the actual value of the
 * progressbar, the value will be scaled down to match the new
 * maximum value
 *
 * @param bar The progressbar to set the maximum value of
 * @param value The new maximum value
 */
void mgui_progressbar_set_max_value( MGuiProgressbar* bar, float value )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL )
		return;

	progbar = (struct MGuiProgressbar*)bar;
	progbar->max_value = value;

	progbar->value = math_clampf( progbar->value, 0, value );

	mgui_progressbar_update_colours( progbar );
}

/**
 * @brief Returns the start and end colours of a progressbar.
 *
 * @details This function returns the start and end colours of a
 * progressbar. The actual colour is calculated from these two
 * colours based on the current value of the progressbar.
 *
 * @param bar The progressbar to get the colours of
 * @param col_start A pointer to a colour_t struct that will receive the start colour
 * @param col_end A pointer to a colour_t struct that will receive the end colour
 */
void mgui_progressbar_get_colour( MGuiProgressbar* bar, colour_t* col_start, colour_t* col_end )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ||
		 col_start == NULL ||
		 col_end == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;

	*col_start = progbar->colour_start;
	*col_end = progbar->colour_end;
}

/**
 * @brief Sets the start and end colours of a progressbar.
 *
 * @details This function sets the start and end colours of a
 * progressbar. The actual colour is calculated from these two
 * colours based on the current value of the progressbar.
 *
 * @param bar The progressbar to set the colours of
 * @param col_start A pointer to a colour_t struct that contains the new start colour
 * @param col_end A pointer to a colour_t struct that contains the new end colour
 */
void mgui_progressbar_set_colour( MGuiProgressbar* bar, const colour_t* col_start, const colour_t* col_end )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ||
		 col_start == NULL ||
		 col_end == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;

	progbar->colour_start = *col_start;
	progbar->colour_end = *col_end;

	mgui_set_alpha( bar, col_start->a );
	mgui_progressbar_update_colours( progbar );
}

/**
 * @brief Returns the start and end colours of a progressbar as 32bit integers.
 *
 * @details This function returns the start and end colours of a progressbar.
 * The actual colour is calculated from these two colours based on the current
 * value of the progressbar. The colours are returned as 32bit hex integers
 * in format 0xRRGGBBAA.
 *
 * @param bar The progressbar to get the colours of
 * @param col_start A pointer to a 32bit integer that will receive the start colour
 * @param col_end A pointer to a 32bit integer that will receive the end colour
 */
void mgui_progressbar_get_colour_i( MGuiProgressbar* bar, uint32* col_start, uint32* col_end )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ||
		 col_start == NULL ||
		 col_end == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;

	*col_start = progbar->colour_start.hex;
	*col_end = progbar->colour_end.hex;
}

/**
 * @brief Sets the start and end colours of a progressbar as 32bit integers.
 *
 * @details This function sets the start and end colours of a progressbar.
 * The actual colour is calculated from these two colours based on the current
 * value of the progressbar. The colours are passed as 32bit hex integers
 * in format 0xRRGGBBAA.
 *
 * @param bar The progressbar to set the colours of
 * @param col_start A 32bit integer representing the start colour
 * @param col_end A 32bit integer representing the end colour
 */
void mgui_progressbar_set_colour_i( MGuiProgressbar* bar, uint32 col_start, uint32 col_end )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL )
		return;

	progbar = (struct MGuiProgressbar*)bar;

	progbar->colour_start.hex = col_start;
	progbar->colour_end.hex = col_end;

	mgui_set_alpha( bar, progbar->colour_start.a );
	mgui_progressbar_update_colours( progbar );
}

/**
 * @brief Returns the background shade of a progressbar.
 *
 * @details This function returns the background shade of a progressbar.
 * The shade is a percentage of the main colour (scaled between 0 and 1).
 *
 * @param bar The progressbar to get the background shade of
 * @returns Current background shade
 */
float mgui_progressbar_get_bg_shade( MGuiProgressbar* bar )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL )
		return 0;

	progbar = (struct MGuiProgressbar*)bar;
	return progbar->bg_shade;
}

/**
 * @brief Sets the background shade of a progressbar.
 *
 * @details This function sets the background of a progressbar. The shade
 * is a percentage of the main colour (scaled between 0 and 1).
 *
 * @param bar The progressbar to set the background shade of
 * @param shade The new background shade
 */
void mgui_progressbar_set_bg_shade( MGuiProgressbar* bar, float shade )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;

	shade = math_clampf( shade, 0, 1 );
	progbar->bg_shade = shade;

	mgui_progressbar_update_colours( progbar );
}

/**
 * @brief Returns the border thickness of a progressbar.
 *
 * @details This function returns the thickness of the border for a
 * progressbar in pixels. Note that the value is only valid if MGUI
 * is using a textureless skin.
 *
 * @param bar The progressbar to get the border thickness of
 * @returns Thickness of the border in pixels
 */
uint8 mgui_progressbar_get_thickness( MGuiProgressbar* bar )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL )
		return 0;

	progbar = (struct MGuiProgressbar*)bar;
	return progbar->thickness;
}

/**
 * @brief Sets the border thickness of a progressbar.
 *
 * @details This function sets the thickness of the border for a
 * progressbar in pixels. Note that the value is only valid if MGUI
 * is using a textureless skin.
 *
 * @param bar The progressbar to set the border thickness of
 * @param thickness The thickness of the border in pixels
 */
void mgui_progressbar_set_thickness( MGuiProgressbar* bar, uint8 thickness )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL )
		return;

	progbar = (struct MGuiProgressbar*)bar;
	progbar->thickness = thickness;

	mgui_element_request_redraw( bar );
}
