/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		MemoBox.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI memobox related functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 * 
 **********************************************************************/

#include "MemoBox.h"
#include "Skin.h"
#include "Renderer.h"
#include "Platform/Platform.h"
#include "Stringy/Stringy.h"
#include <assert.h>

static void __mgui_destroy_memobox( element_t* memobox );
static void __mgui_memobox_render( element_t* memobox );
static void __mgui_memobox_on_bounds_update( element_t* memobox, bool pos, bool size );
static void __mgui_memobox_on_mouse_click( element_t* element, MOUSEBTN button, uint16 x, uint16 y );
static void __mgui_memobox_on_mouse_release( element_t* element, MOUSEBTN button, uint16 x, uint16 y );
static void __mgui_memobox_on_mouse_drag( element_t* element, uint16 x, uint16 y );
static void __mgui_memobox_on_key_press( element_t* element, uint key, bool down );

memobox_t* mgui_create_memobox( control_t* parent )
{
	struct memobox_s* memobox;

	memobox = mem_alloc_clean( sizeof(*memobox) );
	mgui_element_create( cast_elem(memobox), parent, true );

	memobox->type = GUI_MEMOBOX;
	memobox->flags |= (FLAG_BACKGROUND|FLAG_BORDER|FLAG_MOUSECTRL|FLAG_KBCTRL|FLAG_DRAGGABLE|FLAG_CLIP|FLAG_WRAP);

	memobox->max_history = 100;
	memobox->lines = list_create();
	memobox->raw_lines = list_create();

	memobox->font = mgui_font_create( DEFAULT_FONT, 11, FFLAG_NONE, ANSI_CHARSET );
	memobox->text->font = memobox->font;
	memobox->text->pad.left = 5;
	memobox->text->pad.right = 5;
	memobox->text->pad.bottom = 5;
	memobox->text->pad.top = 5;

	// Memobox callbacks
	memobox->destroy = __mgui_destroy_memobox;
	memobox->render = __mgui_memobox_render;
	memobox->on_bounds_update = __mgui_memobox_on_bounds_update;

	return cast_elem(memobox);
}

static void __mgui_destroy_memobox( element_t* memobox )
{
	struct memobox_s* memo;
	memo = (struct memobox_s*)memobox;

	mgui_memobox_clear( memobox );

	list_destroy( memo->raw_lines );
	list_destroy( memo->lines );
}

static void __mgui_memobox_render( element_t* memobox )
{
	struct memobox_s* memo;
	memo = (struct memobox_s*)memobox;

	skin->draw_memobox( &memo->bounds, &memo->colour, memo->flags );
	skin->draw_memobox_lines( &memo->bounds, memo->flags, memo->lines, memo->first_line, memo->visible_lines );
}

static void __mgui_memobox_on_mouse_click( element_t* element, MOUSEBTN button, uint16 x, uint16 y )
{
	UNREFERENCED_PARAM(element);
	UNREFERENCED_PARAM(button);
	UNREFERENCED_PARAM(x);
	UNREFERENCED_PARAM(y);
}

static void __mgui_memobox_on_mouse_release( element_t* element, MOUSEBTN button, uint16 x, uint16 y )
{
	UNREFERENCED_PARAM(element);
	UNREFERENCED_PARAM(button);
	UNREFERENCED_PARAM(x);
	UNREFERENCED_PARAM(y);
}

static void __mgui_memobox_on_mouse_drag( element_t* element, uint16 x, uint16 y )
{
	UNREFERENCED_PARAM(element);
	UNREFERENCED_PARAM(x);
	UNREFERENCED_PARAM(y);
}

static void __mgui_memobox_on_key_press( element_t* element, uint key, bool down )
{
	// TODO: pgup/down
	UNREFERENCED_PARAM(element);
	UNREFERENCED_PARAM(key);
	UNREFERENCED_PARAM(down);
}

static void __mgui_memobox_update_display_positions_topbottom( struct memobox_s* memobox )
{
	uint16 line_height, height, display_height;
	uint16 diff;
	int32 x, y;
	node_t* node;
	struct memoline_s* line;

	// Yeah, don't ask how this works... it just does.
	line_height = memobox->font->size + memobox->text->pad.bottom;	// Height of one text line with spacing
	height = (uint16)( memobox->lines->size - 1 ) * line_height;	// Height of all text lines

	display_height = memobox->bounds.h - memobox->text->pad.bottom - memobox->text->pad.top - line_height;
	display_height = height > display_height ? display_height : height;
	diff = height <= memobox->bounds.h ? 0 : height - memobox->bounds.h;

	x = memobox->bounds.x + memobox->text->pad.left;
	y = memobox->bounds.y + display_height + memobox->text->pad.top + (uint16)( memobox->position * diff );

	memobox->first_line = NULL;
	memobox->visible_lines = 0;

	list_foreach_r( memobox->lines, node )
	{
		line = (struct memoline_s*)node;

		if ( y > memobox->bounds.y + memobox->bounds.h - memobox->text->pad.bottom )
		{
			y -= line_height;
			continue;
		}

		if ( y < memobox->bounds.y + memobox->text->pad.top )
			break;

		if ( !memobox->first_line )
		{
			memobox->first_line = node;

			if ( diff )
				y = memobox->bounds.y + memobox->bounds.h - memobox->font->size - memobox->text->pad.bottom - 2;
		}

		memobox->visible_lines++;

		line->pos.x = (uint16)x;
		line->pos.y = (uint16)y;

		y -= line_height;
	}
}

static void __mgui_memobox_update_display_positions_bottomtop( struct memobox_s* memobox )
{
	uint16 line_height, height;
	uint16 diff;
	int32 x, y;
	node_t* node;
	struct memoline_s* line;

	line_height = memobox->font->size + memobox->text->pad.bottom;
	height = (uint16)memobox->lines->size * line_height;
	diff = height <= memobox->bounds.h ? 0 : height - memobox->bounds.h;

	x = memobox->bounds.x + memobox->text->pad.left;
	y = memobox->bounds.y + memobox->bounds.h - memobox->font->size - memobox->text->pad.bottom - 2 + (uint16)( memobox->position * diff );

	memobox->first_line = NULL;
	memobox->visible_lines = 0;

	list_foreach_r( memobox->lines, node )
	{
		line = (struct memoline_s*)node;

		if ( y > memobox->bounds.y + memobox->bounds.h )
		{
			y -= line_height;
			continue;
		}

		if ( y <= memobox->bounds.y )
			break;

		if ( !memobox->first_line )
			memobox->first_line = node;

		memobox->visible_lines++;

		line->pos.x = (uint16)x;
		line->pos.y = (uint16)y;

		y -= line_height;
	}
}

static void __mgui_memobox_update_display_positions( struct memobox_s* memobox )
{
	if ( memobox->lines->size == 0 ) return;

	if ( BIT_ON( memobox->flags, FLAG_TOPBOTTOM ) )
		__mgui_memobox_update_display_positions_topbottom( memobox );

	else
		__mgui_memobox_update_display_positions_bottomtop( memobox );

}

static void __mgui_memobox_wrap_line( struct memobox_s* memobox, struct memoraw_s* raw )
{
	struct memoline_s* line;
	uint32 linelength, spacepos;
	uint32 width, w, h;
	char_t *str, *tmp, *lastspace;
	char_t tmpbuf[512] = { 0 };

	width = memobox->bounds.w - memobox->text->pad.left - memobox->text->pad.right;
	linelength = spacepos = 0;
	lastspace = NULL;

	for ( str = raw->text, tmp = tmpbuf;
		  *str && linelength < lengthof(tmpbuf) - 1; )
	{
		*tmp = *str;
		*(tmp+sizeof(char_t)) = '\0';

		if ( *str == ' ' )
		{
			if ( linelength == 0 )
			{
				str += sizeof(char_t); // Remove leading spaces
				continue;
			}

			spacepos = linelength;
			lastspace = str;
		}

		render->measure_text( memobox->font->data, tmpbuf, &w, &h );

		if ( w > width || *str == '\n' )
		{
			if ( *str != '\n' )
				linelength = ( spacepos == 0 ) ? linelength : spacepos;

			line = mem_alloc( sizeof(*line) );
			line->colour = raw->colour;
			line->font = memobox->text->font;
			line->text = mstrdup( tmpbuf, linelength );

			if ( *str != '\n' )
				str = lastspace ? lastspace : str;
			else
				str = str += sizeof(char_t);

			linelength = spacepos = 0;
			lastspace = NULL;
			tmp = tmpbuf;

			list_push( memobox->lines, cast_node(line) );
			continue;	
		}

		linelength++;

		tmp += sizeof(char_t);
		str += sizeof(char_t);
	}

	line = mem_alloc( sizeof(*line) );
	line->colour = raw->colour;
	line->font = memobox->text->font;
	line->text = mstrdup( tmpbuf, ++linelength );

	list_push( memobox->lines, cast_node(line) );
}

static void __mgui_memobox_process_new_line( struct memobox_s* memobox, struct memoraw_s* raw )
{
	node_t* node;
	struct memoraw_s* oldraw;
	struct memoline_s* oldline;

	list_push( memobox->raw_lines, cast_node(raw) );

	if ( memobox->raw_lines->size > memobox->max_history )
	{
		// If the line count exceeds the history size pop some lines
		node = list_pop_front( memobox->raw_lines );
		oldraw = (struct memoraw_s*)node;

		mem_free( oldraw->text );
		mem_free( oldraw );
	}

	__mgui_memobox_wrap_line( memobox, raw );

	while ( memobox->lines->size > memobox->max_history )
	{
		// Pop some old display lines
		node = list_pop_front( memobox->lines );
		oldline = (struct memoline_s*)node;

		mem_free( oldline->text );
		mem_free( oldline );
	}

	__mgui_memobox_update_display_positions( memobox );

	mgui_force_redraw();
}

static void __mgui_memobox_on_bounds_update( element_t* memobox, bool pos, bool size )
{
	node_t *node, *tmp;
	struct memoline_s* line;
	struct memobox_s* memo;

	memo = (struct memobox_s*)memobox;

	if ( pos )
	{
		__mgui_memobox_update_display_positions( memo );
	}

	if ( size )
	{
		// We have to change the size, so need to delete old lines first.
		list_foreach_safe( memo->lines, node, tmp )
		{
			line = (struct memoline_s*)node;
			list_remove( memo->lines, node );

			if ( line->text ) mem_free( line->text );
			mem_free( line );
		}

		// Then re-add the lines, wrapping properly
		list_foreach( memo->raw_lines, node )
		{
			__mgui_memobox_process_new_line( memo, (struct memoraw_s*)node );
		}
	}
}


void mgui_memobox_add_line( memobox_t* memobox, const char* fmt, ... )
{
	size_t	len;
	char_t	tmp[512];
	va_list	marker;

	assert( memobox != NULL );

	va_start( marker, fmt );
	len = msnprintf( tmp, lengthof(tmp), fmt, marker );
	va_end( marker );

	mgui_memobox_add_line_col_s( memobox, &tmp[0], colour_to_hex(&memobox->text->colour) );
}

void mgui_memobox_add_line_col( memobox_t* memobox, const char* fmt, uint32 colour, ... )
{
	size_t	len;
	char_t	tmp[512];
	va_list	marker;

	assert( memobox != NULL );

	va_start( marker, colour );
	len = msnprintf( tmp, lengthof(tmp), fmt, marker );
	va_end( marker );

	mgui_memobox_add_line_col_s( memobox, &tmp[0], colour );
}

void mgui_memobox_add_line_s( memobox_t* memobox, const char* text )
{
	assert( memobox != NULL );
	assert( memobox->text != NULL );

	mgui_memobox_add_line_col_s( memobox, text, colour_to_hex(&memobox->text->colour) );
}

void mgui_memobox_add_line_col_s( memobox_t* memobox, const char* text, uint32 colour )
{
	struct memobox_s* memo;
	struct memoraw_s* line;
	size_t len;

	assert( memobox != NULL );
	assert( memobox->text != NULL );

	memo = (struct memobox_s*)memobox;
	line = mem_alloc( sizeof(*line) );

	len = mstrlen( text );

	line->text = mstrdup( text, len );
	hex_to_colour( colour, &line->colour );

	__mgui_memobox_process_new_line( memo, line );
}

void mgui_memobox_clear( memobox_t* memobox )
{
	node_t *node, *tmp;
	struct memoline_s* line;
	struct memoraw_s* raw;
	struct memobox_s* memo;

	memo = (struct memobox_s*)memobox;

	assert( memobox != NULL );

	list_foreach_safe( memo->raw_lines, node, tmp )
	{
		raw = (struct memoraw_s*)node;
		list_remove( memo->raw_lines, node );

		if ( raw->text ) mem_free( raw->text );
		mem_free( raw );
	}

	list_foreach_safe( memo->lines, node, tmp )
	{
		line = (struct memoline_s*)node;
		list_remove( memo->lines, node );

		if ( line->text ) mem_free( line->text );
		mem_free( line );
	}

	mgui_force_redraw();
}

float mgui_memobox_get_display_pos( memobox_t* memobox )
{
	return ((struct memobox_s*)memobox)->position;
}

void mgui_memobox_set_display_pos( memobox_t* memobox, float pos )
{
	struct memobox_s* memo;
	memo = (struct memobox_s*)memobox;

	memo->position = pos;

	__mgui_memobox_update_display_positions( memo );
}

bool mgui_memobox_get_top_to_bottom( memobox_t* memobox )
{
	return BIT_ON( memobox->flags, FLAG_TOPBOTTOM );
}

void mgui_memobox_set_top_to_bottom( memobox_t* memobox, bool enable )
{
	if ( enable )
	{
		if ( BIT_OFF( memobox->flags, FLAG_TOPBOTTOM ) )
		{
			memobox->flags |= FLAG_TOPBOTTOM;
			__mgui_memobox_update_display_positions( (struct memobox_s*)memobox );
		}
	}
	else
	{
		if ( BIT_ON( memobox->flags, FLAG_TOPBOTTOM ) )
		{
			memobox->flags &= ~FLAG_TOPBOTTOM;
			__mgui_memobox_update_display_positions( (struct memobox_s*)memobox );
		}
	}
}

uint32 mgui_memobox_get_lines( memobox_t* memobox )
{
	return ((struct memobox_s*)memobox)->lines->size;
}

uint32 mgui_memobox_get_history( memobox_t* memobox )
{
	return ((struct memobox_s*)memobox)->max_history;
}

void mgui_memobox_set_history( memobox_t* memobox, uint32 lines )
{
	((struct memobox_s*)memobox)->max_history = lines;
}
