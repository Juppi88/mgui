/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		SkinSimple.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An implementation of a basic, textureless skin.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "Skin.h"
#include "Element.h"
#include "MemoBox.h"
#include "Renderer.h"
#include "Platform/Platform.h"
#include <assert.h>

static void __draw_panel( const rectangle_t* r, const colour_t* col )
{
	render->set_draw_colour( col );
	render->draw_rect( r->x, r->y, r->w, r->h );
}

static void __draw_border( const rectangle_t* r, const colour_t* col, uint8 borders, uint8 thickness )
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

static void __draw_shadow( const rectangle_t* r, uint offset )
{
	static const colour_t c = { 0x0A0A0A32 };

	render->set_draw_colour( &c );
	
	render->draw_rect( r->x + r->w, r->y + offset, offset, r->h - offset );
	render->draw_rect( r->x + offset, r->y + r->h, r->w, offset );
}

static void __draw_button( const rectangle_t* r, const colour_t* col, uint32 flags, const text_t* text )
{
	colour_t c;
	c = *col;

	// Actual button
	if ( BIT_ON( flags, FLAG_HOVER ) )
	{
		colour_add_scalar( &c, &c, 10 );
	}

	render->set_draw_colour( &c );
	render->draw_rect( r->x + 1, r->y + 1, r->w - 2, r->h - 2 );

	if ( text )
	{
		render->start_clip( text->bounds->x, text->bounds->y, text->bounds->w, text->bounds->h );
		render->set_draw_colour( &text->colour );

		if ( flags & FLAG_PRESSED )
			render->draw_text( text->font->data, text->buffer, text->pos.x+1, text->pos.y+1, text->font->flags );
		else
			render->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );

		render->end_clip();
	}

	if ( BIT_OFF( flags, FLAG_BORDER ) ) return;

	// Borders
	colour_add_scalar( &c, &c, 40 );

	if ( BIT_OFF( flags, FLAG_PRESSED ) )
	{
		render->set_draw_colour( &c );
		render->draw_rect( r->x, r->y, r->w, 1 );
		render->draw_rect( r->x, r->y, 1, r->h );

		colour_subtract_scalar( &c, &c, 80 );

		render->set_draw_colour( &c );
		render->draw_rect( r->x + r->w - 1, r->y, 1, r->h );
		render->draw_rect( r->x, r->y + r->h - 1, r->w, 1 );
	}
	else
	{
		render->set_draw_colour( &c );
		render->draw_rect( r->x + r->w - 1, r->y, 1, r->h );
		render->draw_rect( r->x, r->y + r->h - 1, r->w, 1 );

		colour_subtract_scalar( &c, &c, 80 );

		render->set_draw_colour( &c );
		render->draw_rect( r->x, r->y, r->w, 1 );
		render->draw_rect( r->x, r->y, 1, r->h );
	}
}

static void __draw_editbox( const rectangle_t* r, const colour_t* col, uint32 flags, const text_t* text )
{
	colour_t c;

	if ( BIT_ON( flags, FLAG_BACKGROUND ) )
	{
		__draw_panel( r, col );
	}

	if ( BIT_ON( flags, FLAG_BORDER ) )
	{
		colour_add_scalar( &c, col, 60 );
		c.a = col->a;

		__draw_border( r, &c, BORDER_BOTTOM|BORDER_RIGHT, 1 );

		colour_subtract_scalar( &c, &c, 60 );
		c.a = col->a;

		__draw_border( r, &c, BORDER_TOP|BORDER_LEFT, 1 );
	}

	if ( text )
	{
		render->set_draw_colour( &text->colour );

		if ( BIT_ON( flags, FLAG_CLIP ) )
		{
			render->start_clip( text->bounds->x, text->bounds->y, text->bounds->w, text->bounds->h );
			render->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
			render->end_clip();
		}
		else
		{
			render->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
		}
	}
}

static void __draw_label( const rectangle_t* r, const colour_t* col, uint32 flags, const text_t* text )
{
	colour_t c;

	if ( BIT_ON( flags, FLAG_BACKGROUND ) )
	{
		__draw_panel( r, col );
	}

	if ( BIT_ON( flags, FLAG_BORDER ) )
	{
		c.r = col->r / 2;
		c.g = col->g / 2;
		c.b = col->b / 2;
		c.a = col->a;

		__draw_border( r, &c, BORDER_ALL, 1 );
	}

	if ( text )
	{
		render->set_draw_colour( &text->colour );

		if ( BIT_ON( flags, FLAG_CLIP ) )
		{
			render->start_clip( text->bounds->x, text->bounds->y, text->bounds->w, text->bounds->h );
			render->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
			render->end_clip();
		}
		else
		{
			render->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
		}
	}
}

static void __draw_memobox( const rectangle_t* r, const colour_t* col, uint32 flags )
{
	colour_t c;

	if ( BIT_ON( flags, FLAG_BACKGROUND ) )
	{
		__draw_panel( r, col );
	}

	if ( BIT_ON( flags, FLAG_BORDER ) )
	{
		colour_add_scalar( &c, col, 60 );
		c.a = col->a;

		__draw_border( r, &c, BORDER_BOTTOM|BORDER_RIGHT, 1 );

		colour_subtract_scalar( &c, &c, 60 );
		c.a = col->a;

		__draw_border( r, &c, BORDER_TOP|BORDER_LEFT, 1 );
	}
}

static void __draw_memobox_lines( const rectangle_t* r, uint32 flags, list_t* lines, node_t* first, uint32 count )
{
	struct memoline_s* line;
	node_t* node;
	uint32 i;

	if ( lines->size == 0 ) return;
	if ( count == 0 ) count = 0xFFFFFFFF;

	if ( BIT_ON( flags, FLAG_CLIP ) )
	{
		render->start_clip( r->x, r->y, r->w, r->h );
	}

	for ( node = first, i = 0;
		  node != lines->end && i < count;
		  node = node->prev, i++ )
	{
		line = (struct memoline_s*)node;

		render->set_draw_colour( &line->colour );
		render->draw_text( line->font->data, line->text, line->pos.x, line->pos.y, line->font->flags );
	}

	if ( BIT_ON( flags, FLAG_CLIP ) )
	{
		render->end_clip();
	}
}

static void __draw_scrollbar( const rectangle_t* r, colour_t* col, uint32 flags )
{
	UNREFERENCED_PARAM( flags );

	__draw_panel( r, col );
}

static void __draw_scrollbar_bar( const rectangle_t* r, colour_t* col, uint32 flags )
{
	__draw_button( r, col, flags|FLAG_BORDER, NULL );
}

static void __draw_scrollbar_button( const rectangle_t* r, colour_t* col, uint32 flags, colour_t* arrowcol, uint32 direction )
{
	uint32 x1, x2, y1, y2, xm, ym;
	colour_t c;

	__draw_button( r, col, flags|FLAG_BORDER, NULL );

	colour_subtract_scalar( &c, arrowcol, 10 );

	render->set_draw_colour( &c );

	x1 = r->x + r->w / 3;
	x2 = r->x + 2 * r->w / 3;
	y1 = r->y + r->h / 3;
	y2 = r->y + 2 * r->h / 3;
	xm = r->x + r->w / 2;
	ym = r->y + r->h / 2;

	switch ( direction )
	{
	case ARROW_UP:
		render->draw_triangle( xm, y1, x2, y2, x1, y2 );
		break;

	case ARROW_DOWN:
		render->draw_triangle( xm, y2, x1, y1, x2, y1 );
		break;

	case ARROW_LEFT:
		render->draw_triangle( x1, ym, x2, y1, x2, y2 );
		break;

	case ARROW_RIGHT:
		render->draw_triangle( x2, ym, x1, y2, x1, y1 );
		break;
	}
}

static void __draw_window( const rectangle_t* r, const colour_t* col, uint32 flags )
{
	UNREFERENCED_PARAM(flags);

	render->set_draw_colour( col );
	render->draw_rect( r->x, r->y, r->w, r->h );
}

static void __draw_window_titlebar( const rectangle_t* r, const colour_t* col, const text_t* text )
{
	uint h, h2;
	colour_t c;

	h2 = r->h * 3/5;
	h = r->h - h2;
	c = *col;

	render->set_draw_colour( &c );
	render->draw_rect( r->x, r->y+h, r->w, h2 );

	colour_add_scalar( &c, &c, 8 );

	render->set_draw_colour( &c );
	render->draw_rect( r->x, r->y, r->w, h );

	if ( text )
	{
		render->set_draw_colour( &text->colour );
		render->start_clip( text->bounds->x, text->bounds->y, text->bounds->w, text->bounds->h );
		render->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
		render->end_clip();
	}
}

skin_t* mgui_setup_skin_simple( void )
{
	skin_t* skin;

	skin = mem_alloc_clean( sizeof(*skin) );

	skin->draw_panel			= __draw_panel;
	skin->draw_border			= __draw_border;
	skin->draw_shadow			= __draw_shadow;
	skin->draw_button			= __draw_button;
	skin->draw_editbox			= __draw_editbox;
	skin->draw_label			= __draw_label;
	skin->draw_memobox			= __draw_memobox;
	skin->draw_memobox_lines	= __draw_memobox_lines;
	skin->draw_scrollbar		= __draw_scrollbar;
	skin->draw_scrollbar_bar	= __draw_scrollbar_bar;
	skin->draw_scrollbar_button	= __draw_scrollbar_button;
	skin->draw_window			= __draw_window;
	skin->draw_window_titlebar	= __draw_window_titlebar;

	return skin;
}
