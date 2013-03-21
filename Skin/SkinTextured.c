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

static void		skin_textured_draw_panel			( const rectangle_t* r, const colour_t* col );
static void		skin_textured_draw_border			( const rectangle_t* r, const colour_t* col, uint32 borders, uint32 thickness );
static void		skin_textured_draw_shadow			( const rectangle_t* r, uint offset );
static void		skin_textured_draw_button			( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text );
static void		skin_textured_draw_editbox			( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text );
static void		skin_textured_draw_label			( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text );
static void		skin_textured_draw_memobox			( const rectangle_t* r, const colour_t* col, uint32 flags );
static void		skin_textured_draw_memobox_lines	( const rectangle_t* r, uint32 flags, list_t* lines, node_t* first, uint32 count );
static void		skin_textured_draw_scrollbar		( const rectangle_t* r, const colour_t* col, uint32 flags );
static void		skin_textured_draw_scrollbar_bar	( const rectangle_t* r, const colour_t* col, uint32 flags );
static void		skin_textured_draw_scrollbar_button	( const rectangle_t* r, const colour_t* col, uint32 flags, const colour_t* arrowcol, uint32 direction );
static void		skin_textured_draw_window			( const rectangle_t* r, const colour_t* col, uint32 flags );
static void		skin_textured_draw_window_titlebar	( const rectangle_t* r, const colour_t* col, const MGuiText* text );

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
	skin->draw_memobox_lines	= skin_textured_draw_memobox_lines;
	skin->draw_scrollbar		= skin_textured_draw_scrollbar;
	skin->draw_scrollbar_bar	= skin_textured_draw_scrollbar_bar;
	skin->draw_scrollbar_button	= skin_textured_draw_scrollbar_button;
	skin->draw_window			= skin_textured_draw_window;
	skin->draw_window_titlebar	= skin_textured_draw_window_titlebar;

	return skin;
}

static void skin_textured_draw_panel( const rectangle_t* r, const colour_t* col )
{
	render->set_draw_colour( col );
	render->draw_rect( r->x, r->y, r->w, r->h );
}

static void skin_textured_draw_border( const rectangle_t* r, const colour_t* col, uint32 borders, uint32 thickness )
{
	render->set_draw_colour( col );

	if ( borders & BORDER_LEFT )
		render->draw_rect( r->x, r->y, thickness, r->h );

	if ( borders & BORDER_RIGHT )
		render->draw_rect( r->x + r->w-thickness, r->y, thickness, r->h );

	if ( borders & BORDER_TOP )
		render->draw_rect( r->x, r->y, r->w, thickness );

	if ( borders & BORDER_BOTTOM )
		render->draw_rect( r->x, r->y + r->h-thickness, r->w, thickness );
}

static void skin_textured_draw_shadow( const rectangle_t* r, uint offset )
{
	static const colour_t c = { 0x0A0A0A32 };

	render->set_draw_colour( &c );

	render->draw_rect( r->x + r->w, r->y + offset, offset, r->h - offset );
	render->draw_rect( r->x + offset, r->y + r->h, r->w, offset );
}

static void skin_textured_draw_button( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text )
{
	UNREFERENCED_PARAM( r );
	UNREFERENCED_PARAM( col );
	UNREFERENCED_PARAM( flags );
	UNREFERENCED_PARAM( text );
}

static void skin_textured_draw_editbox( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text )
{
	UNREFERENCED_PARAM( r );
	UNREFERENCED_PARAM( col );
	UNREFERENCED_PARAM( flags );
	UNREFERENCED_PARAM( text );
}

static void skin_textured_draw_label( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text )
{
	UNREFERENCED_PARAM( r );
	UNREFERENCED_PARAM( col );
	UNREFERENCED_PARAM( flags );
	UNREFERENCED_PARAM( text );
}

static void skin_textured_draw_memobox( const rectangle_t* r, const colour_t* col, uint32 flags )
{
	UNREFERENCED_PARAM( r );
	UNREFERENCED_PARAM( col );
	UNREFERENCED_PARAM( flags );
}

static void skin_textured_draw_memobox_lines( const rectangle_t* r, uint32 flags, list_t* lines, node_t* first, uint32 count )
{
	UNREFERENCED_PARAM( r );
	UNREFERENCED_PARAM( flags );
	UNREFERENCED_PARAM( lines );
	UNREFERENCED_PARAM( first );
	UNREFERENCED_PARAM( count );
}

static void skin_textured_draw_scrollbar( const rectangle_t* r, const colour_t* col, uint32 flags )
{
	UNREFERENCED_PARAM( r );
	UNREFERENCED_PARAM( col );
	UNREFERENCED_PARAM( flags );
}

static void skin_textured_draw_scrollbar_bar( const rectangle_t* r, const colour_t* col, uint32 flags )
{
	UNREFERENCED_PARAM( r );
	UNREFERENCED_PARAM( col );
	UNREFERENCED_PARAM( flags );
}

static void skin_textured_draw_scrollbar_button( const rectangle_t* r, const colour_t* col, uint32 flags, const colour_t* arrowcol, uint32 direction )
{
	UNREFERENCED_PARAM( r );
	UNREFERENCED_PARAM( col );
	UNREFERENCED_PARAM( flags );
	UNREFERENCED_PARAM( arrowcol );
	UNREFERENCED_PARAM( direction );
}

static void skin_textured_draw_window( const rectangle_t* r, const colour_t* col, uint32 flags )
{
	UNREFERENCED_PARAM( r );
	UNREFERENCED_PARAM( col );
	UNREFERENCED_PARAM( flags );
}

static void skin_textured_draw_window_titlebar( const rectangle_t* r, const colour_t* col, const MGuiText* text )
{
	UNREFERENCED_PARAM( r );
	UNREFERENCED_PARAM( col );
	UNREFERENCED_PARAM( text );
}
