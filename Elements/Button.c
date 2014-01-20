/**
 *
 * @file		Button.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI button related functions.
 *
 * @details		Functions and structures related to GUI buttons.
 *
 **/

#include "Button.h"
#include "Skin.h"
#include "Renderer.h"
#include "Input/Input.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

// Button callback handlers
static void		mgui_button_render			( MGuiElement* button );
static void		mgui_button_on_mouse_hover	( MGuiElement* button );
static void		mgui_button_on_mouse_click	( MGuiElement* button, int16 x, int16 y, MOUSEBTN mousebtn );
static bool		mgui_button_on_key_press	( MGuiElement* button, uint32 key, bool down );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_button_render,
	NULL, /* post_render */
	NULL, /* process */
	NULL, /* get_clip_region */
	NULL, /* on_bounds_change */
	NULL, /* on_flags_change */
	NULL, /* on_colour_change */
	NULL, /* on_text_change */
	mgui_button_on_mouse_hover,
	mgui_button_on_mouse_hover,
	mgui_button_on_mouse_click,
	mgui_button_on_mouse_click,
	NULL, /* on_mouse_drag */
	NULL, /* on_mouse_move */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	mgui_button_on_key_press
};

// --------------------------------------------------

/**
 * @brief Creates a button.
 *
 * @details This function creates a GUI button. If the parent element
 * is NULL, the button will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @returns A pointer to the created button
 */
MGuiButton* mgui_create_button( MGuiElement* parent )
{
	struct MGuiButton* button;
	extern MGuiFont* default_font;

	button = mem_alloc_clean( sizeof(*button) );
	mgui_element_create( cast_elem(button), parent );

	button->flags |= (FLAG_BORDER|FLAG_BACKGROUND|FLAG_MOUSECTRL|FLAG_KBCTRL);
	button->type = GUI_BUTTON;

	button->colour.hex = COL_ELEMENT_DARK;
	button->font = default_font;
	button->text->font = default_font;

	// Button callbacks
	button->callbacks = &callbacks;

	return cast_elem(button);
}

/**
 * @brief Creates a button (extended).
 *
 * @details This function creates a GUI button with the given parameters.
 * If the parent element is NULL, the button will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @param x The absolute x coordinate relative to the parent
 * @param y The absolute y coordinate relative to the parent
 * @param w The absolute width of the button
 * @param h The absolute height of the button
 * @param flags Any additional flags that will be applied as a bitmask (see @ref MGUI_FLAGS)
 * @param col The colour of the button as a 32bit hex integer
 * @param text The text on the button
 *
 * @returns A pointer to the created button
 */
MGuiButton* mgui_create_button_ex( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text )
{
	MGuiButton* button;

	button = mgui_create_button( parent );

	mgui_set_abs_pos_i( button, x, y );
	mgui_set_abs_size_i( button, w, h );
	mgui_add_flags( button, flags );
	mgui_set_colour_i( button, col );
	mgui_set_text_s( button, text );

	return button;
}

static void mgui_button_render( MGuiElement* button )
{
	button->skin->draw_button( button );
}

static void mgui_button_on_mouse_hover( MGuiElement* button )
{
	mgui_element_request_redraw( button );
}

static void mgui_button_on_mouse_click( MGuiElement* button, int16 x, int16 y, MOUSEBTN mousebtn )
{
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );
	UNREFERENCED_PARAM( mousebtn );

	mgui_element_request_redraw( button );
}

static bool mgui_button_on_key_press( MGuiElement* button, uint32 key, bool down )
{
	MGuiEvent event;

	if ( key != MKEY_RETURN && key != MKEY_SPACE ) return true;

	if ( down )
	{
		button->flags_int |= INTFLAG_PRESSED;

		if ( button->event_handler )
		{
			event.type = EVENT_CLICK;
			event.mouse.element = button;
			event.mouse.data = button->event_data;
			event.mouse.cursor_x = 0;
			event.mouse.cursor_y = 0;

			button->event_handler( &event );
		}
	}
	else
	{
		button->flags_int &= ~INTFLAG_PRESSED;

		if ( button->event_handler )
		{
			event.type = EVENT_RELEASE;
			event.mouse.element = button;
			event.mouse.data = button->event_data;
			event.mouse.cursor_x = 0;
			event.mouse.cursor_y = 0;

			button->event_handler( &event );
		}
	}

	mgui_element_request_redraw( button );
	return true;
}
