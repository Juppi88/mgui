/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		SkinTextured.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An implementation of a textured GUI skin.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "SkinTextured.h"
#include "Element.h"
#include "Renderer.h"
#include "Platform/Alloc.h"

extern MGuiRenderer* renderer;

// TODO: Actually finish this some day!

typedef struct
{
	uint16 x, y;
	uint16 w, h;
	struct { uint16 top, bottom, left, right; } margin;
} MGuiPrimitive;

typedef struct
{
	MGuiPrimitive	button_normal;
	MGuiPrimitive	button_hovered;
	MGuiPrimitive	button_pressed;
	MGuiPrimitive	button_inactive;
	MGuiPrimitive	textbox_normal;
	MGuiPrimitive	textbox_focus;
	MGuiPrimitive	textbox_inactive;
	MGuiPrimitive	window;
	MGuiPrimitive	window_titlebar;
	MGuiPrimitive	windowbtn_close_normal;
	MGuiPrimitive	windowbtn_close_hovered;
	MGuiPrimitive	windowbtn_close_pressed;
} MGuiTexturedSkin;

MGuiSkin* mgui_setup_skin_textured( const char_t* texture )
{
	MGuiSkin* skin;

	// For now
	// TODO: Load it
	UNREFERENCED_PARAM( texture );

	skin = mem_alloc_clean( sizeof(*skin) );

	skin->texture				= NULL;

	skin->draw_panel			= skin_textured_draw_panel;
	skin->draw_border			= skin_textured_draw_border;
	skin->draw_shadow			= skin_textured_draw_shadow;
	skin->draw_button			= skin_textured_draw_button;
	skin->draw_editbox			= skin_textured_draw_editbox;
	skin->draw_label			= skin_textured_draw_label;
	skin->draw_memobox			= skin_textured_draw_memobox;
	skin->draw_scrollbar		= skin_textured_draw_scrollbar;
	skin->draw_window			= skin_textured_draw_window;
	skin->draw_window_titlebar	= skin_textured_draw_window_titlebar;

	return skin;
}

static void skin_textured_draw_panel( const rectangle_t* r, const colour_t* col )
{
	renderer->set_draw_colour( col );
	renderer->draw_rect( r->x, r->y, r->w, r->h );
}

static void skin_textured_draw_border( const rectangle_t* r, const colour_t* col, uint32 borders, uint32 thickness )
{
	renderer->set_draw_colour( col );

	if ( borders & BORDER_LEFT )
		renderer->draw_rect( r->x, r->y, thickness, r->h );

	if ( borders & BORDER_RIGHT )
		renderer->draw_rect( r->x + r->w-thickness, r->y, thickness, r->h );

	if ( borders & BORDER_TOP )
		renderer->draw_rect( r->x, r->y, r->w, thickness );

	if ( borders & BORDER_BOTTOM )
		renderer->draw_rect( r->x, r->y + r->h-thickness, r->w, thickness );
}

static void skin_textured_draw_shadow( const rectangle_t* r, uint offset )
{
	static const colour_t c = { 0x0A0A0A32 };

	renderer->set_draw_colour( &c );

	renderer->draw_rect( r->x + r->w, r->y + offset, offset, r->h - offset );
	renderer->draw_rect( r->x + offset, r->y + r->h, r->w, offset );
}

static void skin_textured_draw_button( MGuiElement* element )
{
	UNREFERENCED_PARAM( element );
}

static void skin_textured_draw_editbox( MGuiElement* element )
{
	UNREFERENCED_PARAM( element );
}

static void skin_textured_draw_label( MGuiElement* element )
{
	UNREFERENCED_PARAM( element );
}

static void skin_textured_draw_memobox( MGuiElement* element )
{
	UNREFERENCED_PARAM( element );
}

static void skin_textured_draw_scrollbar( MGuiElement* element )
{
	UNREFERENCED_PARAM( element );
}

static void skin_textured_draw_window( MGuiElement* element )
{
	UNREFERENCED_PARAM( element );
}

static void skin_textured_draw_window_titlebar( MGuiElement* element )
{
	UNREFERENCED_PARAM( element );
}
