/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Button.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI button related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "Button.h"
#include "Skin.h"
#include "Renderer.h"
#include "Input/Input.h"
#include "Platform/Alloc.h"

// Button callback handlers
static void		mgui_button_render			( MGuiElement* button );
static void		mgui_button_on_key_press	( MGuiElement* button, uint32 key, bool down );

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_button_render,
	NULL, /* process */
	NULL, /* on_bounds_change */
	NULL, /* on_flags_change */
	NULL, /* on_colour_change */
	NULL, /* on_text_change */
	NULL, /* on_mouse_enter */
	NULL, /* on_mouse_leave */
	NULL, /* on_mouse_click */
	NULL, /* on_mouse_release */
	NULL, /* on_mouse_drag */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	mgui_button_on_key_press
};

MGuiButton* mgui_create_button( MGuiElement* parent )
{
	struct MGuiButton* button;

	button = mem_alloc_clean( sizeof(*button) );
	mgui_element_create( cast_elem(button), parent );

	button->flags |= (FLAG_BORDER|FLAG_BACKGROUND|FLAG_MOUSECTRL|FLAG_KBCTRL);
	button->type = GUI_BUTTON;

	button->colour.hex = COL_ELEMENT_DARK;
	button->font = mgui_font_create( DEFAULT_FONT, 11, FFLAG_NONE, CHARSET_ANSI );
	button->text->font = button->font;

	// Button callbacks
	button->callbacks = &callbacks;

	return cast_elem(button);
}

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

static void mgui_button_on_key_press( MGuiElement* button, uint32 key, bool down )
{
	MGuiEvent guievent;

	if ( key != MKEY_RETURN && key != MKEY_SPACE ) return;

	if ( down )
	{
		button->flags_int |= INTFLAG_PRESSED;

		if ( button->event_handler )
		{
			guievent.type = EVENT_CLICK;
			guievent.element = button;
			guievent.data = button->event_data;
			guievent.mouse.x = 0;
			guievent.mouse.y = 0;

			button->event_handler( &guievent );
		}
	}
	else
	{
		button->flags_int &= ~INTFLAG_PRESSED;

		if ( button->event_handler )
		{
			guievent.type = EVENT_RELEASE;
			guievent.element = button;
			guievent.data = button->event_data;
			guievent.mouse.x = 0;
			guievent.mouse.y = 0;

			button->event_handler( &guievent );
		}
	}

	mgui_force_redraw();
}
