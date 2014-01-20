/**
 *
 * @file		Checkbox.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI checkbox related functions.
 *
 * @details		Functions and structures related to GUI checkboxes.
 *
 **/

#include "Checkbox.h"
#include "Skin.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

// Checkbox callback handlers
static void		mgui_checkbox_render			( MGuiElement* checkbox );
static void		mgui_checkbox_on_mouse_release	( MGuiElement* checkbox, int16 x, int16 y, MOUSEBTN mousebtn );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_checkbox_render,
	NULL, /* post_render */
	NULL, /* process */
	NULL, /* get_clip_region */
	NULL, /* on_bounds_change */
	NULL, /* on_flags_change */
	NULL, /* on_colour_change */
	NULL, /* on_text_change */
	NULL, /* on_mouse_enter */
	NULL, /* on_mouse_leave */
	NULL, /* on_mouse_press */
	mgui_checkbox_on_mouse_release,
	NULL, /* on_mouse_drag */
	NULL, /* on_mouse_move */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

// --------------------------------------------------

/**
 * @brief Creates a checkbox.
 *
 * @details This function creates a GUI checkbox. If the parent element
 * is NULL, the checkbox will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @returns A pointer to the created checkbox
 */
MGuiCheckbox* mgui_create_checkbox( MGuiElement* parent )
{
	struct MGuiCheckbox* checkbox;

	checkbox = mem_alloc_clean( sizeof(*checkbox) );
	checkbox->flags_int |= INTFLAG_NOTEXT;

	mgui_element_create( cast_elem(checkbox), parent );

	checkbox->type = GUI_CHECKBOX;
	checkbox->flags |= (FLAG_BORDER|FLAG_BACKGROUND|FLAG_MOUSECTRL|FLAG_TABSTOP);
	checkbox->font = NULL;
	checkbox->colour.hex = RGBCOL( 225, 225, 225 );

	// Checkbox callbacks
	checkbox->callbacks = &callbacks;

	mgui_set_abs_size_i( cast_elem(checkbox), 12, 12 );

	return cast_elem(checkbox);
}

/**
 * @brief Creates a checkbox (extended).
 *
 * @details This function creates a GUI checkbox with the given parameters.
 * If the parent element is NULL, the checkbox will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @param x The absolute x coordinate relative to the parent
 * @param y The absolute y coordinate relative to the parent
 * @param flags Any additional flags that will be applied as a bitmask (see @ref MGUI_FLAGS)
 * @param col The colour of the checkbox as a 32bit hex integer
 *
 * @returns A pointer to the created checkbox
 */
MGuiCheckbox* mgui_create_checkbox_ex( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col )
{
	MGuiCheckbox* checkbox;

	checkbox = mgui_create_checkbox( parent );

	mgui_set_abs_pos_i( checkbox, x, y );
	mgui_add_flags( checkbox, flags );
	mgui_set_colour_i( checkbox, col );

	return checkbox;
}

static void mgui_checkbox_render( MGuiElement* checkbox )
{
	checkbox->skin->draw_checkbox( checkbox );
}

static void mgui_checkbox_on_mouse_release( MGuiElement* checkbox, int16 x, int16 y, MOUSEBTN mousebtn )
{
	MGuiEvent event;

	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );

	if ( mousebtn != MOUSE_LBUTTON ) return;

	BIT_TOGGLE( checkbox->flags, FLAG_CHECKBOX_CHECKED );

	if ( checkbox->event_handler )
	{
		event.type = EVENT_CHECKBOX_TOGGLE;
		event.any.element = checkbox;
		event.any.data = checkbox->event_data;

		checkbox->event_handler( &event );
	}

	mgui_element_request_redraw( checkbox );
}
