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

MGuiTitlebar* mgui_create_titlebar( MGuiWindow* parent )
{
	MGuiTitlebar* titlebar;

	titlebar = mem_alloc_clean( sizeof(*titlebar) );
	mgui_element_create( cast_elem(titlebar), NULL, false );

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

	titlebar->on_mouse_click = mgui_titlebar_on_mouse_click;
	titlebar->on_mouse_drag = mgui_titlebar_on_mouse_drag;

	return titlebar;
}

void mgui_destroy_titlebar( MGuiTitlebar* titlebar )
{
	mgui_input_cleanup_references( cast_elem(titlebar) );
	mem_free( titlebar );
}

static void mgui_titlebar_on_mouse_click( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y )
{
	MGuiTitlebar* titlebar;
	titlebar = (MGuiTitlebar*)element;

	UNREFERENCED_PARAM( button );

	if ( titlebar->window == NULL ) return;

	titlebar->window->on_mouse_click( titlebar->window, button, x - titlebar->bounds.x, y - titlebar->bounds.y );
}

static void mgui_titlebar_on_mouse_drag( MGuiElement* element, uint16 x, uint16 y )
{
	MGuiTitlebar* titlebar;
	titlebar = (MGuiTitlebar*)element;

	if ( titlebar->window == NULL ) return;

	titlebar->window->on_mouse_drag( titlebar->window, x, y );
}
