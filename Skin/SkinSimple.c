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
#include "EditBox.h"
#include "MemoBox.h"
#include "ScrollBar.h"
#include "Window.h"
#include "WindowTitlebar.h"
#include "Renderer.h"
#include "Platform/Alloc.h"

extern MGuiRenderer* renderer;

MGuiSkin* mgui_setup_skin_simple( void )
{
	MGuiSkin* skin;

	skin = (MGuiSkin*)mem_alloc_clean( sizeof(*skin) );

	skin->texture				= NULL;

	skin->draw_panel			= skin_simple_draw_panel;
	skin->draw_border			= skin_simple_draw_border;
	skin->draw_shadow			= skin_simple_draw_shadow;
	skin->draw_button			= skin_simple_draw_button;
	skin->draw_editbox			= skin_simple_draw_editbox;
	skin->draw_label			= skin_simple_draw_label;
	skin->draw_memobox			= skin_simple_draw_memobox;
	skin->draw_scrollbar		= skin_simple_draw_scrollbar;
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

static void skin_simple_draw_generic_button( const rectangle_t* r, const colour_t* col, uint32 flags )
{
	colour_t c;

	c = *col;

	// Actual button part
	if ( BIT_ON( flags, INTFLAG_HOVER ) )
	{
		colour_add_scalar( &c, &c, 10 );
	}

	renderer->set_draw_colour( &c );
	renderer->draw_rect( r->x + 1, r->y + 1, r->w - 2, r->h - 2 );

	// Borders
	colour_add_scalar( &c, &c, 40 );

	if ( BIT_OFF( flags, INTFLAG_PRESSED ) )
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


static void skin_simple_draw_shadow( const rectangle_t* r, uint offset )
{
	static const colour_t c = { 0x0A0A0A32 };

	renderer->set_draw_colour( &c );
	
	renderer->draw_rect( r->x + r->w, r->y + offset, offset, r->h - offset );
	renderer->draw_rect( r->x + offset, r->y + r->h, r->w, offset );
}

static void skin_simple_draw_button( MGuiElement* element )
{
	colour_t c;
	rectangle_t* r;
	MGuiText* text;

	c = element->colour;
	r = &element->bounds;

	// Actual button part
	if ( BIT_ON( element->flags_int, INTFLAG_HOVER ) )
	{
		colour_add_scalar( &c, &c, 10 );
	}

	if ( BIT_ON( element->flags, FLAG_BACKGROUND ) )
	{
		renderer->set_draw_colour( &c );
		renderer->draw_rect( r->x + 1, r->y + 1, r->w - 2, r->h - 2 );
	}

	if ( ( text = element->text ) != NULL )
	{
		renderer->start_clip( text->bounds->x, text->bounds->y, text->bounds->w, text->bounds->h );
		renderer->set_draw_colour( &text->colour );

		if ( element->flags & INTFLAG_PRESSED )
		{
			renderer->draw_text( text->font->data, text->buffer, text->pos.x+1, text->pos.y+1, text->font->flags );
		}
		else
		{
			renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
		}

		renderer->end_clip();
	}

	// Borders
	if ( BIT_ON( element->flags, FLAG_BORDER ) )
	{
		colour_add_scalar( &c, &c, 40 );

		if ( BIT_OFF( element->flags_int, INTFLAG_PRESSED ) )
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
}

static void skin_simple_draw_editbox( MGuiElement* element )
{
	colour_t c;
	rectangle_t* r;
	MGuiText* text;
	char_t* tmp;
	struct MGuiEditbox* editbox;

	r = &element->bounds;
	editbox = (struct MGuiEditbox*)element;
	
	if ( BIT_ON( element->flags, FLAG_BACKGROUND ) )
	{
		skin_simple_draw_panel( r, &element->colour );
	}

	if ( BIT_ON( element->flags, FLAG_BORDER ) )
	{
		colour_add_scalar( &c, &element->colour, 60 );
		c.a = element->colour.a;

		skin_simple_draw_border( r, &c, BORDER_BOTTOM|BORDER_RIGHT, 1 );

		colour_subtract_scalar( &c, &c, 60 );
		c.a = element->colour.a;

		skin_simple_draw_border( r, &c, BORDER_TOP|BORDER_LEFT, 1 );
	}

	if ( ( text = element->text ) != NULL )
	{
		// This is a really ugly hack to make mgui_get_text return the correct buffer:
		// We replace the MGuiText buffer with our own (with masked input etc cool)
		// while we render, and put the original buffer back afterwards
		tmp = element->text->buffer;
		element->text->buffer = editbox->buffer;

		renderer->set_draw_colour( &text->colour );

		if ( BIT_ON( element->flags, FLAG_CLIP ) )
		{
			renderer->start_clip( text->bounds->x, text->bounds->y, text->bounds->w, text->bounds->h );
			renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
			renderer->end_clip();
		}
		else
		{
			renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y, text->font->flags );
		}

		// Really ugly hack cleanup
		element->text->buffer = tmp;
	}
}

static void skin_simple_draw_label( MGuiElement* element )
{
	colour_t c;
	rectangle_t* r;
	MGuiText* text;

	r = &element->bounds;
	c = element->colour;

	if ( BIT_ON( element->flags, FLAG_BACKGROUND ) )
	{
		skin_simple_draw_panel( r, &c );
	}

	if ( BIT_ON( element->flags, FLAG_BORDER ) )
	{
		colour_divide( &c, &element->colour, 2 );
		c.a = element->colour.a;

		skin_simple_draw_border( r, &c, BORDER_ALL, 1 );
	}

	if ( ( text = element->text ) != NULL )
	{
		renderer->set_draw_colour( &text->colour );

		if ( BIT_ON( element->flags, FLAG_CLIP ) )
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

static void skin_simple_draw_memobox( MGuiElement* element )
{
	colour_t c;
	rectangle_t* r;
	struct MGuiMemobox* memo;
	struct MGuiMemoLine* line;
	node_t* node;
	uint32 i, count;

	memo = (struct MGuiMemobox*)element;

	// Actual memobox
	r = &element->bounds;

	if ( BIT_ON( element->flags, FLAG_BACKGROUND ) )
	{
		skin_simple_draw_panel( r, &element->colour );
	}

	if ( BIT_ON( element->flags, FLAG_BORDER ) )
	{
		colour_add_scalar( &c, &element->colour, 60 );
		c.a = element->colour.a;

		skin_simple_draw_border( r, &c, BORDER_BOTTOM|BORDER_RIGHT, 1 );

		colour_subtract_scalar( &c, &c, 60 );
		c.a = element->colour.a;

		skin_simple_draw_border( r, &c, BORDER_TOP|BORDER_LEFT, 1 );
	}

	// Memobox lines
	if ( memo->lines->size == 0 ) return;

	count = memo->visible_lines;
	if ( count == 0 ) count = 0xFFFFFFFF;

	if ( BIT_ON( memo->flags, FLAG_CLIP ) )
	{
		renderer->start_clip( r->x, r->y, r->w, r->h );
	}

	for ( node = memo->first_line, i = 0;
		  node != list_end(memo->lines) && i < count;
		  node = node->prev, i++ )
	{
		line = (struct MGuiMemoLine*)node;

		renderer->set_draw_colour( &line->colour );
		renderer->draw_text( line->font->data, line->text, line->pos.x, line->pos.y, line->font->flags );
	}

	if ( BIT_ON( memo->flags, FLAG_CLIP ) )
	{
		renderer->end_clip();
	}
}

static void skin_simple_draw_scrollbar( MGuiElement* element )
{
	rectangle_t* r;
	struct MGuiScrollbar* bar;

	r = &element->bounds;
	bar = (struct MGuiScrollbar*)element;

	// Scrollbar background
	skin_simple_draw_panel( r, &bar->track_col );

	// Scrollbar buttons - for now we only have up/down
	// TODO: Add horizontal direction
	skin_simple_draw_scrollbar_button( &bar->button1, &bar->colour, &bar->track_col, bar->flags_button1, ARROW_UP );
	skin_simple_draw_scrollbar_button( &bar->button2, &bar->colour, &bar->track_col, bar->flags_button2, ARROW_DOWN );

	// Scrollbac track
	if ( bar->bar_size != 1.0f )
	{
		skin_simple_draw_generic_button( &bar->bar, &bar->colour, bar->flags_int );
	}
}

static void skin_simple_draw_scrollbar_button( const rectangle_t* r, const colour_t* col, const colour_t* arrowcol, uint32 flags, uint32 dir )
{
	uint32 x1, x2, y1, y2, xm, ym;
	colour_t c;

	skin_simple_draw_generic_button( r, col, flags|FLAG_BORDER );

	colour_subtract_scalar( &c, arrowcol, 10 );

	renderer->set_draw_colour( &c );

	x1 = r->x + r->w / 3;
	x2 = r->x + 2 * r->w / 3;
	y1 = r->y + r->h / 3;
	y2 = r->y + 2 * r->h / 3;
	xm = r->x + r->w / 2;
	ym = r->y + r->h / 2;

	switch ( dir )
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

static void skin_simple_draw_window( MGuiElement* element )
{
	rectangle_t* r;
	rectangle_t border;
	colour_t col;
	struct MGuiWindow* window = (struct MGuiWindow*)element;

	r = &element->bounds;

	if ( BIT_ON( element->flags, FLAG_SHADOW ) )
	{
		skin_simple_draw_shadow( &window->window_bounds, 3 );
	}

	if ( BIT_ON( element->flags, FLAG_WINDOW_TITLEBAR ) && window->titlebar != NULL )
	{
		skin_simple_draw_window_titlebar( cast_elem(window->titlebar) );

		renderer->set_draw_colour( &element->colour );
		renderer->draw_rect( r->x, r->y, r->w, r->h );

		if ( BIT_ON( element->flags, FLAG_WINDOW_CLOSEBTN ) && window->closebtn != NULL )
		{
			skin->draw_button( cast_elem(window->closebtn) );
		}

		if ( BIT_ON( element->flags, FLAG_BORDER ) )
		{
			border.x = r->x + 1;
			border.y = r->y;
			border.w = r->w - 2;
			border.h = r->h;

			colour_add_scalar( &col, &window->titlebar->colour, 60 );
			col.a = window->titlebar->colour.a;

			skin_simple_draw_border( &border, &window->titlebar->colour, BORDER_ALL&(~BORDER_TOP), 2 );
			skin_simple_draw_border( &window->window_bounds, &col, BORDER_ALL, 1 );
		}
	}
	else
	{
		renderer->set_draw_colour( &element->colour );
		renderer->draw_rect( r->x, r->y, r->w, r->h );

		if ( element->flags & FLAG_BORDER )
		{
			colour_subtract_scalar( &col, &element->colour, 40 );
			col.a = element->colour.a;

			skin_simple_draw_border( r, &col, BORDER_ALL, 2 );
		}
	}
}

static void skin_simple_draw_window_titlebar( MGuiElement* element )
{
	rectangle_t* r;
	colour_t c;
	uint32 h, h2;
	MGuiText* text;

	r = &element->bounds;
	text = element->text;

	h2 = r->h * 3/5;
	h = r->h - h2;
	c = element->colour;

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
