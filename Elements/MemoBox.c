/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		MemoBox.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI memobox related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "MemoBox.h"
#include "Skin.h"
#include "Renderer.h"
#include "Platform/Alloc.h"
#include "Stringy/Stringy.h"

extern MGuiRenderer* renderer;

static void		mgui_memobox_update_display_positions			( struct MGuiMemobox* memobox );
static void		mgui_memobox_update_display_positions_topbottom	( struct MGuiMemobox* memobox );
static void		mgui_memobox_update_display_positions_bottomtop	( struct MGuiMemobox* memobox );
static void		mgui_memobox_process_new_line					( struct MGuiMemobox* memobox, struct MGuiMemoRaw* raw );
static void		mgui_memobox_wrap_line							( struct MGuiMemobox* memobox, struct MGuiMemoRaw* raw );
static void		mgui_memobox_refresh_lines						( struct MGuiMemobox* memobox );

// Memobox callback handlers
static void		mgui_memobox_destroy			( MGuiElement* memobox );
static void		mgui_memobox_render				( MGuiElement* memobox );
static void		mgui_memobox_on_bounds_change	( MGuiElement* memobox, bool pos, bool size );
static void		mgui_memobox_on_flags_change	( MGuiElement* memobox, uint32 old );

static struct MGuiCallbacks callbacks =
{
	mgui_memobox_destroy,
	mgui_memobox_render,
	NULL, /* process */
	NULL, /* get_clip_region */
	mgui_memobox_on_bounds_change,
	mgui_memobox_on_flags_change,
	NULL, /* on_colour_change */
	NULL, /* on_text_change */
	NULL, /* on_mouse_enter */
	NULL, /* on_mouse_leave */
	NULL, /* on_mouse_click */
	NULL, /* on_mouse_release */
	NULL, /* on_mouse_drag */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

MGuiMemobox* mgui_create_memobox( MGuiElement* parent )
{
	struct MGuiMemobox* memobox;

	memobox = mem_alloc_clean( sizeof(*memobox) );
	mgui_element_create( cast_elem(memobox), parent );

	memobox->type = GUI_MEMOBOX;
	memobox->flags |= (FLAG_BACKGROUND|FLAG_BORDER|FLAG_MOUSECTRL|FLAG_KBCTRL|FLAG_DRAGGABLE|FLAG_CLIP|FLAG_WRAP);

	memobox->max_history = 50;
	memobox->num_lines = 0;
	memobox->lines = list_create();
	memobox->raw_lines = list_create();
	memobox->first_line = list_end( memobox->lines );

	memobox->font = mgui_font_create( DEFAULT_FONT, 11, FFLAG_NONE, CHARSET_ANSI );
	memobox->text->font = memobox->font;

	memobox->text->pad.bottom = 5;
	memobox->text->pad.top = 5;
	memobox->text->pad.left = 5;
	memobox->text->pad.right = 5;
	memobox->margin = 5;

	// Memobox callbacks
	memobox->callbacks = &callbacks;

	return cast_elem(memobox);
}

MGuiMemobox* mgui_create_memobox_ex( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col )
{
	MGuiMemobox* memobox;

	memobox = mgui_create_memobox( parent );

	mgui_set_abs_pos_i( memobox, x, y );
	mgui_set_abs_size_i( memobox, w, h );
	mgui_add_flags( memobox, flags );
	mgui_set_colour_i( memobox, col );

	return memobox;
}

static void mgui_memobox_destroy( MGuiElement* memobox )
{
	struct MGuiMemobox* memo;
	memo = (struct MGuiMemobox*)memobox;

	mgui_memobox_clear( memobox );

	list_destroy( memo->raw_lines );
	list_destroy( memo->lines );
}

static void mgui_memobox_render( MGuiElement* memobox )
{
	memobox->skin->draw_memobox( memobox );
}

static void mgui_memobox_on_bounds_change( MGuiElement* memobox, bool pos, bool size )
{
	node_t *node, *tmp;
	struct MGuiMemoLine* line;
	struct MGuiMemobox* memo;

	memo = (struct MGuiMemobox*)memobox;

	if ( pos )
	{
		mgui_memobox_update_display_positions( memo );
	}

	if ( size )
	{
		// We have to change the size, so we must delete old lines first.
		list_foreach_safe( memo->lines, node, tmp )
		{
			line = (struct MGuiMemoLine*)node;
			list_remove( memo->lines, node );

			if ( line->text ) mem_free( line->text );
			mem_free( line );
		}

		// Then re-add the lines, wrapping properly
		list_foreach( memo->raw_lines, node )
		{
			mgui_memobox_process_new_line( memo, (struct MGuiMemoRaw*)node );
		}
	}
}

static void mgui_memobox_on_flags_change( MGuiElement* memobox, uint32 old )
{
	// If there was a change, update the memobox
	if ( BIT_ENABLED( memobox->flags, old, FLAG_MEMO_TOPBOTTOM ) ||
		 BIT_DISABLED( memobox->flags, old, FLAG_MEMO_TOPBOTTOM ) )
	{
		mgui_memobox_update_display_positions( (struct MGuiMemobox*)memobox );
	}
}

void mgui_memobox_add_line( MGuiMemobox* memobox, const char* fmt, ... )
{
	char_t tmp[1024];
	va_list	marker;

	if ( memobox == NULL ) return;

	va_start( marker, fmt );
	(void)msnprintf( tmp, lengthof(tmp), fmt, marker );
	va_end( marker );

	// It seems the MS implementation doesn't follow standards and in some cases
	// doesn't write the null terminator. What a surprise.
	tmp[lengthof(tmp)-1] = '\0';

	mgui_memobox_add_line_col_s( memobox, tmp, &memobox->text->colour );
}

void mgui_memobox_add_line_col( MGuiMemobox* memobox, const char* fmt, const colour_t* col, ... )
{
	char_t tmp[1024];
	va_list marker;

	if ( memobox == NULL ) return;

	va_start( marker, col );
	(void)msnprintf( tmp, lengthof(tmp), fmt, marker );
	va_end( marker );

	tmp[lengthof(tmp)-1] = '\0';

	mgui_memobox_add_line_col_s( memobox, tmp, col );
}

void mgui_memobox_add_line_s( MGuiMemobox* memobox, const char* text )
{
	if ( memobox == NULL ) return;
	if ( memobox->text == NULL ) return;

	mgui_memobox_add_line_col_s( memobox, text, &memobox->text->colour );
}

void mgui_memobox_add_line_col_s( MGuiMemobox* memobox, const char* text, const colour_t* col )
{
	struct MGuiMemobox* memo;
	struct MGuiMemoRaw* line;
	size_t len;

	if ( memobox == NULL ) return;
	if ( memobox->text == NULL ) return;

	memo = (struct MGuiMemobox*)memobox;
	line = mem_alloc_clean( sizeof(*line) );

	len = mstrlen( text );

	line->text = mstrdup( text, len );
	line->colour = *col;

	mgui_memobox_process_new_line( memo, line );
}

void mgui_memobox_clear( MGuiMemobox* memobox )
{
	node_t *node, *tmp;
	struct MGuiMemoLine* line;
	struct MGuiMemoRaw* raw;
	struct MGuiMemobox* memo;

	if ( memobox == NULL ) return;

	memo = (struct MGuiMemobox*)memobox;

	list_foreach_safe( memo->raw_lines, node, tmp )
	{
		raw = (struct MGuiMemoRaw*)node;
		list_remove( memo->raw_lines, node );

		if ( raw->text ) mem_free( raw->text );
		mem_free( raw );
	}

	list_foreach_safe( memo->lines, node, tmp )
	{
		line = (struct MGuiMemoLine*)node;
		list_remove( memo->lines, node );

		if ( line->text ) mem_free( line->text );
		mem_free( line );
	}

	mgui_element_request_redraw();
}

float mgui_memobox_get_display_pos( MGuiMemobox* memobox )
{
	if ( memobox == NULL ) return 0.0f;

	return ((struct MGuiMemobox*)memobox)->position;
}

void mgui_memobox_set_display_pos( MGuiMemobox* memobox, float pos )
{
	struct MGuiMemobox* memo;

	if ( memobox == NULL ) return;

	memo = (struct MGuiMemobox*)memobox;
	memo->position = pos;

	mgui_memobox_update_display_positions( memo );
}

uint32 mgui_memobox_get_lines( MGuiMemobox* memobox )
{
	if ( memobox == NULL ) return 0;

	return ((struct MGuiMemobox*)memobox)->num_lines;
}

void mgui_memobox_set_lines( MGuiMemobox* memobox, uint32 lines )
{
	struct MGuiMemobox* memo;

	if ( memobox == NULL ||
		 memobox->font == NULL ||
		 memobox->text == NULL )
		 return;

	memo = (struct MGuiMemobox*)memobox;

	memo->num_lines = (uint8)lines;
	memo->bounds.h = (uint16)( memo->text->pad.top + memo->text->pad.bottom + memo->num_lines * ( memo->margin + memo->font->size ) - memo->margin );

	mgui_memobox_refresh_lines( memo );
}

uint32 mgui_memobox_get_num_lines( MGuiMemobox* memobox )
{
	if ( memobox == NULL ) return 0;

	return ((struct MGuiMemobox*)memobox)->lines->size;
}

uint32 mgui_memobox_get_history( MGuiMemobox* memobox )
{
	if ( memobox == NULL ) return 0;

	return ((struct MGuiMemobox*)memobox)->max_history;
}

void mgui_memobox_set_history( MGuiMemobox* memobox, uint32 lines )
{
	if ( memobox == NULL ) return;

	((struct MGuiMemobox*)memobox)->max_history = (uint8)lines;
}

uint32 mgui_memobox_get_margin( MGuiMemobox* memobox )
{
	if ( memobox == NULL ) return 0;

	return ((struct MGuiMemobox*)memobox)->margin;
}

void mgui_memobox_set_margin( MGuiMemobox* memobox, uint32 margin )
{
	struct MGuiMemobox* memo;

	if ( memobox == NULL ) return;
	if ( memobox->text == NULL ) return;
	if ( memobox->font == NULL ) return;

	memo = (struct MGuiMemobox*)memobox;
	memo->margin = (uint8)margin;
	memo->bounds.h = (uint16)( memo->text->pad.top + memo->text->pad.bottom + memo->num_lines * ( margin + memo->font->size ) );

	mgui_memobox_refresh_lines( memo );
}

static void mgui_memobox_update_display_positions_topbottom( struct MGuiMemobox* memobox )
{
	uint16 line_height, height, display_height;
	uint16 diff;
	int32 x, y;
	node_t* node;
	struct MGuiMemoLine* line;

	// Yeah, don't ask how this works... it just does.
	line_height = memobox->font->size + memobox->margin;			// Height of one text line with spacing
	height = (uint16)( memobox->lines->size - 1 ) * line_height;	// Height of all text lines together

	display_height = memobox->bounds.h - memobox->margin - memobox->text->pad.top - line_height;
	display_height = height > display_height ? display_height : height;

	diff = height <= memobox->bounds.h ? 0 : height - memobox->bounds.h;

	x = memobox->bounds.x + memobox->text->pad.left;
	y = memobox->bounds.y + display_height + memobox->text->pad.top + (uint16)( memobox->position * diff );

	memobox->first_line = NULL;
	memobox->visible_lines = 0;

	list_foreach_r( memobox->lines, node )
	{
		line = (struct MGuiMemoLine*)node;

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

			if ( diff != 0 )
			{
				y = memobox->bounds.y + memobox->bounds.h - memobox->font->size - memobox->text->pad.bottom - 2;
			}
		}

		memobox->visible_lines++;

		line->pos.x = (uint16)x;
		line->pos.y = (uint16)y;

		y -= line_height;
	}
}

static void mgui_memobox_update_display_positions_bottomtop( struct MGuiMemobox* memobox )
{
	uint16 line_height, height;
	uint16 diff;
	int32 x, y;
	node_t* node;
	struct MGuiMemoLine* line;

	line_height = memobox->font->size + memobox->text->pad.bottom;
	height = (uint16)memobox->lines->size * line_height;

	diff = height <= memobox->bounds.h ? 0 : height - memobox->bounds.h;

	x = memobox->bounds.x + memobox->text->pad.left;
	y = memobox->bounds.y + memobox->bounds.h - memobox->font->size - memobox->text->pad.bottom - 2 + (uint16)( memobox->position * diff );

	memobox->first_line = NULL;
	memobox->visible_lines = 0;

	list_foreach_r( memobox->lines, node )
	{
		line = (struct MGuiMemoLine*)node;

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

static void mgui_memobox_update_display_positions( struct MGuiMemobox* memobox )
{
	if ( memobox->lines->size == 0 ) return;

	if ( BIT_ON( memobox->flags, FLAG_MEMO_TOPBOTTOM ) )
	{
		mgui_memobox_update_display_positions_topbottom( memobox );
	}
	else
	{
		mgui_memobox_update_display_positions_bottomtop( memobox );
	}
}

static void mgui_memobox_process_new_line( struct MGuiMemobox* memobox, struct MGuiMemoRaw* raw )
{
	node_t* node;
	struct MGuiMemoRaw* oldraw;
	struct MGuiMemoLine* oldline;

	list_push( memobox->raw_lines, cast_node(raw) );

	if ( memobox->raw_lines->size > memobox->max_history )
	{
		// If the line count exceeds the history size pop some lines
		node = list_pop_front( memobox->raw_lines );
		oldraw = (struct MGuiMemoRaw*)node;

		mem_free( oldraw->text );
		mem_free( oldraw );
	}

	mgui_memobox_wrap_line( memobox, raw );

	while ( memobox->lines->size > memobox->max_history )
	{
		// Pop some old display lines
		node = list_pop_front( memobox->lines );
		oldline = (struct MGuiMemoLine*)node;

		mem_free( oldline->text );
		mem_free( oldline );
	}

	mgui_memobox_update_display_positions( memobox );
	mgui_element_request_redraw();
}

static void mgui_memobox_wrap_line( struct MGuiMemobox* memobox, struct MGuiMemoRaw* raw )
{
	struct MGuiMemoLine* line;
	uint32 max_width, ntags;
	MGuiFormatTag *tags = NULL, **tag_buf = NULL;
	char* buf;

	max_width = memobox->bounds.w - memobox->text->pad.left - memobox->text->pad.right;

	// If format tags are enabled, fetch the pointer for tags
	if ( memobox->flags & FLAG_TEXT_TAGS )
	{
		tag_buf = &tags;
	}
	
	for ( ntags = mgui_text_parse_and_get_line( raw->text, memobox->text->font, &raw->colour, max_width, &buf, tag_buf ); buf;
		  ntags = mgui_text_parse_and_get_line( NULL, memobox->text->font, &raw->colour, max_width, &buf, tag_buf ) )
	{
		line = mem_alloc_clean( sizeof(*line) );
		line->colour = raw->colour;
		line->font = memobox->text->font;
		line->text = buf;
		line->tags = tag_buf ? tags : NULL;
		line->ntags = ntags;

		list_push( memobox->lines, cast_node(line) );
	}
}

static void mgui_memobox_refresh_lines( struct MGuiMemobox* memobox )
{
	struct MGuiMemoLine* line;
	struct MGuiMemoRaw* raw;
	node_t *node, *tmp;

	// Remove old formatted lines
	list_foreach_safe( memobox->lines, node, tmp )
	{
		line = (struct MGuiMemoLine*)node;
		list_remove( memobox->lines, node );

		if ( line->text ) mem_free( line->text );
		mem_free( line );
	}

	// Then re-add every raw line
	list_foreach( memobox->raw_lines, node )
	{
		raw = (struct MGuiMemoRaw*)node;
		mgui_memobox_wrap_line( memobox, raw );
	}

	mgui_memobox_update_display_positions( memobox );
}
