/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		WindowTitlebar.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI window titlebar.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "WindowTitlebar.h"
#include "InputHook.h"
#include "Skin.h"
#include "Font.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

// Window titlebar callback handlers
static void		mgui_titlebar_destroy			( MGuiElement* element );
static void		mgui_titlebar_on_mouse_click	( MGuiElement* element, int16 x, int16 y, MOUSEBTN button );
static void		mgui_titlebar_on_mouse_drag		( MGuiElement* element, int16 x, int16 y );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	mgui_titlebar_destroy,
	NULL, /* render */
	NULL, /* post_render */
	NULL, /* process */
	NULL, /* get_clip_region */
	NULL, /* on_bounds_change */
	NULL, /* on_flags_change */
	NULL, /* on_colour_change */
	NULL, /* on_text_change */
	NULL, /* on_mouse_enter */
	NULL, /* on_mouse_leave */
	mgui_titlebar_on_mouse_click,
	NULL, /* on_mouse_release */
	mgui_titlebar_on_mouse_drag,
	NULL, /* on_mouse_move */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

// --------------------------------------------------

MGuiTitlebar* mgui_create_titlebar( MGuiWindow* parent )
{
	MGuiTitlebar* titlebar;

	titlebar = mem_alloc_clean( sizeof(*titlebar) );
	titlebar->flags_int = INTFLAG_NOTEXT|INTFLAG_NOPARENT;

	mgui_element_create( cast_elem(titlebar), NULL );

	titlebar->flags |= (FLAG_MOUSECTRL|FLAG_DRAGGABLE);
	titlebar->type = GUI_TITLEBAR;
	titlebar->window = parent;

	titlebar->text = parent->text;
	titlebar->font = parent->font;
	titlebar->text->font = titlebar->font;

	titlebar->colour.hex = COL_TITLEBAR;
	titlebar->text->colour.hex = COL_TITLETEXT;

	// Inherit some values from the parent window
	titlebar->colour.a = parent->colour.a;
	titlebar->bounds.w = parent->bounds.w;

	// Titlebar callbacks
	titlebar->callbacks = &callbacks;

	return titlebar;
}

static void mgui_titlebar_destroy( MGuiElement* element )
{
	// Let the parent window clean up text
	element->text = NULL;
	element->font = NULL;
}

static void mgui_titlebar_on_mouse_click( MGuiElement* element, int16 x, int16 y, MOUSEBTN button )
{
	MGuiTitlebar* titlebar;
	struct MGuiWindow* window;

	UNREFERENCED_PARAM( button );

	titlebar = (MGuiTitlebar*)element;
	window = (struct MGuiWindow*)titlebar->window;

	if ( window == NULL ) return;

	window->click_offset.x = x - titlebar->bounds.x;
	window->click_offset.y = y - titlebar->bounds.y;
}

static void mgui_titlebar_on_mouse_drag( MGuiElement* element, int16 x, int16 y )
{
	MGuiTitlebar* titlebar;
	struct MGuiWindow* window;
	node_t* node;
	MGuiEvent event;

	titlebar = (MGuiTitlebar*)element;
	window = (struct MGuiWindow*)titlebar->window;

	if ( window == NULL ) return;

	mgui_element_request_redraw_all();

	window->bounds.x = x - window->click_offset.x;
	window->bounds.y = y - window->click_offset.y;

	window->callbacks->on_bounds_change( cast_elem(window), true, false );

	if ( window->event_handler )
	{
		event.type = EVENT_DRAG;
		event.mouse.element = cast_elem(window);
		event.mouse.data = window->event_data;
		event.mouse.cursor_x = x;
		event.mouse.cursor_y = y;

		window->event_handler( &event );
	}

	if ( window->children == NULL ) return;

	list_foreach( window->children, node )
	{
		mgui_element_update_abs_pos( cast_elem(node) );
	}
}
