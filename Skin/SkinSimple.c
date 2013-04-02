/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		SkinSimple.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An implementation of a basic, textureless skin.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "SkinSimple.h"
#include "Element.h"
#include "MemoBox.h"
#include "Renderer.h"
#include "Platform/Alloc.h"

extern MGuiRenderer* renderer;

MGuiSkin* mgui_setup_skin_simple( void )
{
	MGuiSkin* skin;

	skin = mem_alloc_clean( sizeof(*skin) );

	skin->texture				= NULL;

	skin->draw_panel			= skin_simple_draw_panel;
	skin->draw_border			= skin_simple_draw_border;
	skin->draw_shadow			= skin_simple_draw_shadow;
	skin->draw_button			= skin_simple_draw_button;
	skin->draw_editbox			= skin_simple_draw_editbox;
	skin->draw_label			= skin_simple_draw_label;
	skin->draw_memobox			= skin_simple_draw_memobox;
	skin->draw_memobox_lines	= skin_simple_draw_memobox_lines;
	skin->draw_scrollbar		= skin_simple_draw_scrollbar;
	skin->draw_scrollbar_bar	= skin_simple_draw_scrollbar_bar;
	skin->draw_scrollbar_button	= skin_simple_draw_scrollbar_button;
	skin->draw_window			= skin_simple_draw_window;
	skin->draw_window_titlebar	= skin_simple_draw_window_titlebar;

	return skin;
}

static void skin_simple_draw_panel( const rectangle_t* r, const colour_t* col )
{
	renderer->set_draw_colour( col );
	renderer->draw_rect( r->x, r->y, r->w, r->h );
}

static void skin_simple_draw_border( const rectangle_t* r, const colour_t* col, uint32 borders, uint32 thickness )
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

static void skin_simple_draw_shadow( const rectangle_t* r, uint offset )
{
	static const colour_t c = { 0x0A0A0A32 };

	renderer->set_draw_colour( &c );
	
	renderer->draw_rect( r->x + r->w, r->y + offset, offset, r->h - offset );
	renderer->draw_rect( r->x + offset, r->y + r->h, r->w, offset );
}

static void skin_simple_draw_button( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text )
{
	colour_t c;
	c = *col;

	// Actual button
	if ( BIT_ON( flags, FLAG_HOVER ) )
	{
		colour_add_scalar( &c, &c, 10 );
	}

	renderer->set_draw_colour( &c );
	renderer->draw_rect( r->x + 1, r->y + 1, r->w - 2, r->h - 2 );

	if ( text )
	{
		renderer->start_clip( text->bounds->x, text->bounds->y, text->bounds->w, text->bounds->h );
		renderer->set_draw_colour( &text->colour );

		if ( flags & FLAG_PRESSED )
			renderer->draw_text( text->font->data, text->buffer, text->pos.x+1, text->pos.y+1, text->font->flags );
		else
			renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );

		renderer->end_clip();
	}

	if ( BIT_OFF( flags, FLAG_BORDER ) ) return;

	// Borders
	colour_add_scalar( &c, &c, 40 );

	if ( BIT_OFF( flags, FLAG_PRESSED ) )
	{
		renderer->set_draw_colour( &c );
		renderer->draw_rect( r->x, r->y, r->w, 1 );
		renderer->draw_rect( r->x, r->y, 1, r->h );

		colour_subtract_scalar( &c, &c, 80 );

		renderer->set_draw_colour( &c );
		renderer->draw_rect( r->x + r->w - 1, r->y, 1, r->h );
		renderer->draw_rect( r->x, r->y + r->h - 1, r->w, 1 );
	}
	else
	{
		renderer->set_draw_colour( &c );
		renderer->draw_rect( r->x + r->w - 1, r->y, 1, r->h );
		renderer->draw_rect( r->x, r->y + r->h - 1, r->w, 1 );

		colour_subtract_scalar( &c, &c, 80 );

		renderer->set_draw_colour( &c );
		renderer->draw_rect( r->x, r->y, r->w, 1 );
		renderer->draw_rect( r->x, r->y, 1, r->h );
	}
}

static void skin_simple_draw_editbox( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text )
{
	colour_t c;

	if ( BIT_ON( flags, FLAG_BACKGROUND ) )
	{
		skin_simple_draw_panel( r, col );
	}

	if ( BIT_ON( flags, FLAG_BORDER ) )
	{
		colour_add_scalar( &c, col, 60 );
		c.a = col->a;

		skin_simple_draw_border( r, &c, BORDER_BOTTOM|BORDER_RIGHT, 1 );

		colour_subtract_scalar( &c, &c, 60 );
		c.a = col->a;

		skin_simple_draw_border( r, &c, BORDER_TOP|BORDER_LEFT, 1 );
	}

	if ( text )
	{
		renderer->set_draw_colour( &text->colour );

		if ( BIT_ON( flags, FLAG_CLIP ) )
		{
			renderer->start_clip( text->bounds->x, text->bounds->y, text->bounds->w, text->bounds->h );
			renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
			renderer->end_clip();
		}
		else
		{
			renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
		}
	}
}

static void skin_simple_draw_label( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text )
{
	colour_t c;

	if ( BIT_ON( flags, FLAG_BACKGROUND ) )
	{
		skin_simple_draw_panel( r, col );
	}

	if ( BIT_ON( flags, FLAG_BORDER ) )
	{
		c.r = col->r / 2;
		c.g = col->g / 2;
		c.b = col->b / 2;
		c.a = col->a;

		skin_simple_draw_border( r, &c, BORDER_ALL, 1 );
	}

	if ( text )
	{
		renderer->set_draw_colour( &text->colour );

		if ( BIT_ON( flags, FLAG_CLIP ) )
		{
			renderer->start_clip( text->bounds->x, text->bounds->y, text->bounds->w, text->bounds->h );
			renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
			renderer->end_clip();
		}
		else
		{
			renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
		}
	}
}

static void skin_simple_draw_memobox( const rectangle_t* r, const colour_t* col, uint32 flags )
{
	colour_t c;

	if ( BIT_ON( flags, FLAG_BACKGROUND ) )
	{
		skin_simple_draw_panel( r, col );
	}

	if ( BIT_ON( flags, FLAG_BORDER ) )
	{
		colour_add_scalar( &c, col, 60 );
		c.a = col->a;

		skin_simple_draw_border( r, &c, BORDER_BOTTOM|BORDER_RIGHT, 1 );

		colour_subtract_scalar( &c, &c, 60 );
		c.a = col->a;

		skin_simple_draw_border( r, &c, BORDER_TOP|BORDER_LEFT, 1 );
	}
}

static void skin_simple_draw_memobox_lines( const rectangle_t* r, uint32 flags, list_t* lines, node_t* first, uint32 count )
{
	struct MGuiMemoLine* line;
	node_t* node;
	uint32 i;

	if ( lines->size == 0 ) return;
	if ( count == 0 ) count = 0xFFFFFFFF;

	if ( BIT_ON( flags, FLAG_CLIP ) )
	{
		renderer->start_clip( r->x, r->y, r->w, r->h );
	}

	for ( node = first, i = 0;
		  node != lines->end && i < count;
		  node = node->prev, i++ )
	{
		line = (struct MGuiMemoLine*)node;

		renderer->set_draw_colour( &line->colour );
		renderer->draw_text( line->font->data, line->text, line->pos.x, line->pos.y, line->font->flags );
	}

	if ( BIT_ON( flags, FLAG_CLIP ) )
	{
		renderer->end_clip();
	}
}

static void skin_simple_draw_scrollbar( const rectangle_t* r, const colour_t* col, uint32 flags )
{
	UNREFERENCED_PARAM( flags );

	skin_simple_draw_panel( r, col );
}

static void skin_simple_draw_scrollbar_bar( const rectangle_t* r, const colour_t* col, uint32 flags )
{
	skin_simple_draw_button( r, col, flags|FLAG_BORDER, NULL );
}

static void skin_simple_draw_scrollbar_button( const rectangle_t* r, const colour_t* col, uint32 flags, const colour_t* arrowcol, uint32 direction )
{
	uint32 x1, x2, y1, y2, xm, ym;
	colour_t c;

	skin_simple_draw_button( r, col, flags|FLAG_BORDER, NULL );

	colour_subtract_scalar( &c, arrowcol, 10 );

	renderer->set_draw_colour( &c );

	x1 = r->x + r->w / 3;
	x2 = r->x + 2 * r->w / 3;
	y1 = r->y + r->h / 3;
	y2 = r->y + 2 * r->h / 3;
	xm = r->x + r->w / 2;
	ym = r->y + r->h / 2;

	switch ( direction )
	{
	case ARROW_UP:
		renderer->draw_triangle( xm, y1, x2, y2, x1, y2 );
		break;

	case ARROW_DOWN:
		renderer->draw_triangle( xm, y2, x1, y1, x2, y1 );
		break;

	case ARROW_LEFT:
		renderer->draw_triangle( x1, ym, x2, y1, x2, y2 );
		break;

	case ARROW_RIGHT:
		renderer->draw_triangle( x2, ym, x1, y2, x1, y1 );
		break;
	}
}

static void skin_simple_draw_window( const rectangle_t* r, const colour_t* col, uint32 flags )
{
	UNREFERENCED_PARAM(flags);

	renderer->set_draw_colour( col );
	renderer->draw_rect( r->x, r->y, r->w, r->h );
}

static void skin_simple_draw_window_titlebar( const rectangle_t* r, const colour_t* col, const MGuiText* text )
{
	uint h, h2;
	colour_t c;

	h2 = r->h * 3/5;
	h = r->h - h2;
	c = *col;

	renderer->set_draw_colour( &c );
	renderer->draw_rect( r->x, r->y+h, r->w, h2 );

	colour_add_scalar( &c, &c, 8 );

	renderer->set_draw_colour( &c );
	renderer->draw_rect( r->x, r->y, r->w, h );

	if ( text )
	{
		renderer->set_draw_colour( &text->colour );
		renderer->start_clip( text->bounds->x, text->bounds->y, text->bounds->w, text->bounds->h );
		renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
		renderer->end_clip();
	}
}
