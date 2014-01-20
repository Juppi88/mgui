/**
 *
 * @file		InputHook.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		MGUI input handler.
 *
 * @details		User input hooking and processing.
 *
 **/

#include "InputHook.h"
#include "Element.h"
#include "Renderer.h"
#include "WindowTitlebar.h"

// --------------------------------------------------

static bool		mgui_input_handle_char			( InputEvent* event );
static bool		mgui_input_handle_key_up		( InputEvent* event );
static bool		mgui_input_handle_key_down		( InputEvent* event );
static bool		mgui_input_handle_mouse_move	( InputEvent* event );
static bool		mgui_input_handle_mouse_wheel	( InputEvent* event );
static bool		mgui_input_handle_lmb_up		( InputEvent* event );
static bool		mgui_input_handle_lmb_down		( InputEvent* event );

// --------------------------------------------------

static MGuiElement*	hovered		= NULL; // Element being hovered currently
static MGuiElement*	pressed		= NULL; // Element being pressed down currently
static MGuiElement*	dragged		= NULL;	// Element that is being dragged
static MGuiElement*	mousefocus	= NULL; // The element that has the mouse focus
static MGuiElement*	kbfocus		= NULL;	// The element that has the keyboard focus

// --------------------------------------------------

void mgui_input_initialize_hooks( void )
{
	input_add_hook( INPUT_CHARACTER, mgui_input_handle_char );
	input_add_hook( INPUT_KEY_UP, mgui_input_handle_key_up );
	input_add_hook( INPUT_KEY_DOWN, mgui_input_handle_key_down );
	input_add_hook( INPUT_MOUSE_MOVE, mgui_input_handle_mouse_move );
	input_add_hook( INPUT_MOUSE_WHEEL, mgui_input_handle_mouse_wheel );
	input_add_hook( INPUT_LBUTTON_UP, mgui_input_handle_lmb_up );
	input_add_hook( INPUT_LBUTTON_DOWN, mgui_input_handle_lmb_down );
}

void mgui_input_shutdown_hooks( void )
{
	input_remove_hook( INPUT_CHARACTER, mgui_input_handle_char );
	input_remove_hook( INPUT_KEY_UP, mgui_input_handle_key_up );
	input_remove_hook( INPUT_KEY_DOWN, mgui_input_handle_key_down );
	input_remove_hook( INPUT_MOUSE_MOVE, mgui_input_handle_mouse_move );
	input_remove_hook( INPUT_MOUSE_WHEEL, mgui_input_handle_mouse_wheel );
	input_remove_hook( INPUT_LBUTTON_UP, mgui_input_handle_lmb_up );
	input_remove_hook( INPUT_LBUTTON_DOWN, mgui_input_handle_lmb_down );
}

void mgui_input_cleanup_references( MGuiElement* element )
{
	if ( element == hovered ) hovered = NULL;
	if ( element == pressed ) pressed = NULL;
	if ( element == dragged ) dragged = NULL;
	if ( element == mousefocus ) mousefocus = NULL;
	if ( element == kbfocus ) kbfocus = NULL;
}

/**
 * @brief Returns the element that has keyboard focus.
 *
 * @details This function returns the element that currently
 * has keyboard focus. The function will return NULL if no
 * element has focus right now.
 *
 * @return The element that has keyboard focus
 */
MGuiElement* mgui_get_focus( void )
{
	return kbfocus;
}

/**
 * @brief Sets the element that has keyboard focus.
 *
 * @details This function changes or sets the element that
 * has keyboard focus. If you pass NULL, all elements will
 * lose keyboard focus.
 *
 * @param element The element that will receive keyboard focus
 */
void mgui_set_focus( MGuiElement* element )
{
	MGuiEvent event;

	if ( kbfocus != NULL )
	{
		if ( kbfocus->event_handler )
		{
			event.type = EVENT_FOCUS_EXIT;
			event.any.element = kbfocus;
			event.any.data = kbfocus->event_data;

			kbfocus->event_handler( &event );
		}

		mgui_element_request_redraw( kbfocus );

		kbfocus->flags_int &= ~INTFLAG_FOCUS;
		kbfocus = NULL;
	}

	if ( element == NULL )
		return;

	if ( BIT_ON( element->flags, FLAG_KBCTRL ) &&
		 BIT_ON( element->flags, FLAG_VISIBLE ) )
	{
		kbfocus = element;
		element->flags_int |= INTFLAG_FOCUS;

		mgui_element_request_redraw( kbfocus );

		if ( kbfocus->event_handler )
		{
			event.type = EVENT_FOCUS_ENTER;
			event.any.element = kbfocus;
			event.any.data = element->event_data;

			kbfocus->event_handler( &event );
		}
	}
}

static bool mgui_input_handle_char( InputEvent* event )
{
	if ( kbfocus && kbfocus->callbacks->on_character )
	{
		return kbfocus->callbacks->on_character( kbfocus, (char_t)event->keyboard.key );
	}

	return true;
}

static bool mgui_input_handle_key_up( InputEvent* event )
{
	if ( kbfocus && kbfocus->callbacks->on_key_press )
	{
		return kbfocus->callbacks->on_key_press( kbfocus, event->keyboard.key, false );
	}

	return true;
}

static bool mgui_input_handle_key_down( InputEvent* event )
{
	if ( kbfocus && kbfocus->callbacks->on_key_press )
	{
		return kbfocus->callbacks->on_key_press( kbfocus, event->keyboard.key, true );
	}

	return true;
}

static bool mgui_input_handle_mouse_move( InputEvent* event )
{
	int16 x, y;
	MGuiElement* element;
	MGuiEvent guievent;

	x = event->mouse.x;
	y = event->mouse.y;

	if ( dragged && dragged->callbacks->on_mouse_drag )
		dragged->callbacks->on_mouse_drag( dragged, x, y );

	element = mgui_get_element_at( x, y );
	if ( element == hovered )
	{
		if ( hovered && hovered->callbacks->on_mouse_move )
			hovered->callbacks->on_mouse_move( hovered, x, y );

		return true;
	}

	if ( hovered )
	{
		hovered->flags_int &= ~INTFLAG_HOVER;

		if ( hovered->callbacks->on_mouse_leave )
			hovered->callbacks->on_mouse_leave( hovered );

		if ( hovered->event_handler )
		{
			guievent.type = EVENT_HOVER_LEAVE;
			guievent.mouse.element = hovered;
			guievent.mouse.data = hovered->event_data;
			guievent.mouse.cursor_x = x;
			guievent.mouse.cursor_y = y;

			hovered->event_handler( &guievent );
		}
	}

	if ( ( hovered = element ) != NULL )
	{
		hovered->flags_int |= INTFLAG_HOVER;

		if ( hovered->callbacks->on_mouse_enter )
			hovered->callbacks->on_mouse_enter( hovered );

		if ( hovered->event_handler )
		{
			guievent.type = EVENT_HOVER_ENTER;
			guievent.mouse.element = hovered;
			guievent.mouse.data = hovered->event_data;
			guievent.mouse.cursor_x = x;
			guievent.mouse.cursor_y = y;

			hovered->event_handler( &guievent );
		}
	}

	return true;
}

static bool mgui_input_handle_mouse_wheel( InputEvent* event )
{
	UNREFERENCED_PARAM( event );
	return true;
}

static bool mgui_input_handle_lmb_up( InputEvent* event )
{
	int16 x, y;
	MGuiEvent guievent;

	x = event->mouse.x;
	y = event->mouse.y;

	dragged = NULL;

	if ( pressed )
	{
		pressed->flags_int &= ~INTFLAG_PRESSED;

		if ( pressed->callbacks->on_mouse_release )
			pressed->callbacks->on_mouse_release( pressed, x, y, MOUSE_LBUTTON );

		if ( pressed->event_handler )
		{
			guievent.type = EVENT_RELEASE;
			guievent.mouse.element = pressed;
			guievent.mouse.data = pressed->event_data;
			guievent.mouse.cursor_x = x;
			guievent.mouse.cursor_y = y;

			pressed->event_handler( &guievent );
		}

		pressed = NULL;
	}

	return true;
}

static bool mgui_input_handle_lmb_down( InputEvent* event )
{
	int16 x, y;
	MGuiElement* element;
	MGuiEvent guievent;

	x = event->mouse.x;
	y = event->mouse.y;

	dragged = NULL;
	element = mgui_get_element_at( x, y );

	// Remove old keyboard focus
	if ( kbfocus && kbfocus != element )
	{
		if ( kbfocus->event_handler )
		{
			guievent.type = EVENT_FOCUS_EXIT;
			guievent.any.element = kbfocus;
			guievent.any.data = kbfocus->event_data;

			kbfocus->event_handler( &guievent );
		}

		mgui_element_request_redraw( kbfocus );

		kbfocus->flags_int &= ~INTFLAG_FOCUS;
		kbfocus = NULL;
	}

	// Remove old pressed focus
	if ( pressed )
	{
		pressed->flags_int &= ~INTFLAG_PRESSED;

		if ( pressed->callbacks->on_mouse_release )
			pressed->callbacks->on_mouse_release( pressed, x, y, MOUSE_LBUTTON );

		if ( pressed->event_handler )
		{
			guievent.type = EVENT_RELEASE;
			guievent.mouse.element = pressed;
			guievent.mouse.data = pressed->event_data;
			guievent.mouse.cursor_x = x;
			guievent.mouse.cursor_y = y;

			pressed->event_handler( &guievent );
		}
	}

	// If a new element is being pressed handle focus/event stuff
	if ( ( pressed = element ) != NULL )
	{
		pressed->flags_int |= INTFLAG_PRESSED;

		// Move the element to the top.
		// Note to self: Never use sub-elements again.
		if ( pressed->type == GUI_TITLEBAR )
			element = ((struct MGuiTitlebar*)pressed)->window;
		
		if ( element != NULL )
			mgui_send_to_top( element );

		if ( pressed->callbacks->on_mouse_click )
			pressed->callbacks->on_mouse_click( pressed, x, y, MOUSE_LBUTTON );

		if ( BIT_ON( pressed->flags, FLAG_DRAGGABLE ) )
			dragged = pressed;

		if ( pressed->event_handler )
		{
			guievent.type = EVENT_CLICK;
			guievent.mouse.element = pressed;
			guievent.mouse.data = pressed->event_data;
			guievent.mouse.cursor_x = x;
			guievent.mouse.cursor_y = y;

			pressed->event_handler( &guievent );
		}

		if ( BIT_ON( element->flags, FLAG_KBCTRL ) )
		{
			kbfocus = pressed;
			pressed->flags_int |= INTFLAG_FOCUS;

			if ( kbfocus->event_handler )
			{
				guievent.type = EVENT_FOCUS_ENTER;
				guievent.any.element = kbfocus;
				guievent.any.data = kbfocus->event_data;

				kbfocus->event_handler( &guievent );
			}
		}
	}

	return true;
}
