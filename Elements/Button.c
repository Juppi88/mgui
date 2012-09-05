/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Button.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI button related functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "Button.h"
#include "Skin.h"
#include "Renderer.h"
#include "Input/Input.h"
#include "Platform/Platform.h"

static void __mgui_destroy_button( element_t* button );
static void __mgui_button_render( element_t* button );
static void __mgui_button_on_bounds_update( element_t* button, bool pos, bool size );
static void __mgui_button_on_mouse_enter( element_t* button );
static void __mgui_button_on_mouse_leave( element_t* button );
static void __mgui_button_on_key_press( element_t* element, uint key, bool down );

button_t* mgui_create_button( control_t* parent )
{
	struct button_s* button;

	button = mem_alloc_clean( sizeof(*button) );
	mgui_element_create( cast_elem(button), parent, true );

	button->flags |= (FLAG_BORDER|FLAG_BACKGROUND|FLAG_MOUSECTRL|FLAG_KBCTRL);
	button->type = GUI_BUTTON;

	hex_to_colour( COL_ELEMENT_DARK, &button->colour );

	button->font = mgui_font_create( DEFAULT_FONT, 11, FFLAG_NONE, ANSI_CHARSET );
	button->text->font = button->font;

	// Button callbacks
	button->destroy = __mgui_destroy_button;
	button->render = __mgui_button_render;
	button->on_mouse_enter = __mgui_button_on_mouse_enter;
	button->on_mouse_leave = __mgui_button_on_mouse_leave;
	button->on_key_press = __mgui_button_on_key_press;

	return cast_elem(button);
}

static void __mgui_destroy_button( element_t* button )
{
	UNREFERENCED_PARAM(button);
	// Nothing to do here.
}

static void __mgui_button_render( element_t* button )
{
	skin->draw_button( &button->bounds, &button->colour, button->flags, button->text );
}

static void __mgui_button_on_mouse_enter( element_t* button )
{
	UNREFERENCED_PARAM( button );
}

static void __mgui_button_on_mouse_leave( element_t* button )
{
	UNREFERENCED_PARAM( button );
}

static void __mgui_button_on_key_press( element_t* element, uint key, bool down )
{
	guievent_t guievent;
	
	if ( key != MKEY_RETURN && key != MKEY_SPACE ) return;

	if ( down )
	{
		element->flags |= FLAG_PRESSED;
		element->on_mouse_click( element, MOUSE_LBUTTON, 0, 0 );

		if ( element->event_handler )
		{
			guievent.type = EVENT_CLICK;
			guievent.element = element;
			guievent.data = element->event_data;
			guievent.mouse.x = 0;
			guievent.mouse.y = 0;

			element->event_handler( &guievent );
		}
	}
	else
	{
		element->flags &= ~FLAG_PRESSED;
		element->on_mouse_release( element, MOUSE_LBUTTON, 0, 0 );

		if ( element->event_handler )
		{
			guievent.type = EVENT_RELEASE;
			guievent.element = element;
			guievent.data = element->event_data;
			guievent.mouse.x = 0;
			guievent.mouse.y = 0;

			element->event_handler( &guievent );
		}
	}

	mgui_force_redraw();
}
