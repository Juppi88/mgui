/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		EditBox.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI editbox related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "EditBox.h"
#include "Skin.h"
#include "Renderer.h"
#include "Platform/Alloc.h"
#include "Platform/Timer.h"
#include "Platform/Window.h"
#include "Stringy/Stringy.h"
#include "Input/Input.h"

extern MGuiRenderer* renderer;
extern uint32 tick_count;

static void			mgui_editbox_refresh_cursor_bounds	( struct MGuiEditbox* editbox );
static void			mgui_editbox_erase_text				( struct MGuiEditbox* editbox, uint32 begin, uint32 end );
static void			mgui_editbox_insert_text			( struct MGuiEditbox* editbox, const char_t* text, size_t len );
static void			mgui_editbox_select_all				( struct MGuiEditbox* editbox );
static void			mgui_editbox_cut_selection			( struct MGuiEditbox* editbox );
static void			mgui_editbox_copy_selection			( struct MGuiEditbox* editbox );
static void			mgui_editbox_paste_selection		( struct MGuiEditbox* editbox );
static void			mgui_editbox_press_backspace		( struct MGuiEditbox* editbox );
static void			mgui_editbox_press_delete			( struct MGuiEditbox* editbox );
static void			mgui_editbox_press_return			( struct MGuiEditbox* editbox );
static void			mgui_editbox_move_left				( struct MGuiEditbox* editbox );
static void			mgui_editbox_move_right				( struct MGuiEditbox* editbox );
static void			mgui_editbox_press_home				( struct MGuiEditbox* editbox );
static void			mgui_editbox_press_end				( struct MGuiEditbox* editbox );

// Editbox callback handlers
static void			mgui_destroy_editbox				( MGuiElement* editbox );
static void			mgui_editbox_render					( MGuiElement* element );
static void			mgui_editbox_process				( MGuiElement* element );
static void			mgui_editbox_set_bounds				( MGuiElement* element, bool pos, bool size );
static void			mgui_editbox_set_text				( MGuiElement* element );
static void			mgui_editbox_on_mouse_click			( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
static void			mgui_editbox_on_mouse_release		( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
static void			mgui_editbox_on_mouse_drag			( MGuiElement* element, uint16 x, uint16 y );
static void			mgui_editbox_on_character			( MGuiElement* element, char_t c );
static void			mgui_editbox_on_key_press			( MGuiElement* element, uint key, bool down );


MGuiEditbox* mgui_create_editbox( MGuiElement* parent )
{
	struct MGuiEditbox* editbox;

	editbox = mem_alloc_clean( sizeof(*editbox) );
	mgui_element_create( cast_elem(editbox), parent );

	editbox->type = GUI_EDITBOX;
	editbox->flags |= (FLAG_BORDER|FLAG_BACKGROUND|FLAG_CLIP|FLAG_DRAGGABLE|FLAG_MOUSECTRL|FLAG_KBCTRL|FLAG_ANIMATION);
	editbox->flags_int &= ~INTFLAG_NOTEXT;

	// Create an initial buffer
	editbox->text->bufsize = 20;
	editbox->text->buffer = mem_alloc( editbox->text->bufsize );
	editbox->buffer = mem_alloc( editbox->text->bufsize );
	*editbox->text->buffer = '\0';
	*editbox->buffer = '\0';

	editbox->font = mgui_font_create( DEFAULT_FONT, 11, FFLAG_NONE, ANSI_CHARSET );
	editbox->text->font = editbox->font;
	editbox->text->alignment = ALIGN_LEFT|ALIGN_CENTERV;
	editbox->text->pad.left = 5;

	editbox->colour.hex = COL_ELEMENT_DARK;

	// Editbox callbacks
	editbox->destroy		= mgui_destroy_editbox;
	editbox->render			= mgui_editbox_render;
	editbox->process		= mgui_editbox_process;
	editbox->set_bounds		= mgui_editbox_set_bounds;
	editbox->set_text		= mgui_editbox_set_text;
	editbox->on_mouse_click	= mgui_editbox_on_mouse_click;
	editbox->on_mouse_release = mgui_editbox_on_mouse_release;
	editbox->on_mouse_drag	= mgui_editbox_on_mouse_drag;
	editbox->on_character	= mgui_editbox_on_character;
	editbox->on_key_press	= mgui_editbox_on_key_press;

	return cast_elem(editbox);
}

MGuiEditbox* mgui_create_editbox_ex( MGuiElement* parent, uint16 x, uint16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text )
{
	MGuiEditbox* editbox;

	editbox = mgui_create_editbox( parent );

	mgui_set_abs_pos_i( editbox, x, y );
	mgui_set_abs_size_i( editbox, w, h );
	mgui_add_flags( editbox, flags );
	mgui_set_colour_i( editbox, col );
	mgui_set_text_s( editbox, text );

	return editbox;
}

static void mgui_destroy_editbox( MGuiElement* editbox )
{
	UNREFERENCED_PARAM( editbox );
}

static void mgui_editbox_render( MGuiElement* element )
{
	struct MGuiEditbox* editbox;
	colour_t col;

	editbox = (struct MGuiEditbox*)element;

	element->skin->draw_editbox( element );

	if ( BIT_ON( editbox->flags_int, INTFLAG_FOCUS ) )
	{
		// Draw the cursor, make sure it's within the editbox boundaries
		if ( editbox->cursor_visible || BIT_OFF( editbox->flags, FLAG_ANIMATION ) )
		{
			if ( editbox->cursor.x < editbox->bounds.x + editbox->bounds.w )
			{
				renderer->set_draw_colour( &editbox->text->colour );
				renderer->draw_rect( editbox->cursor.x, editbox->cursor.y, editbox->cursor.w, editbox->cursor.h );
			}
		}

		// Draw the selection. TODO: Would be cool to change the text colour as well
		if ( editbox->cursor_pos != editbox->cursor_end )
		{
			col = editbox->colour;
			colour_invert( &col, &col );
			col.a = 90;

			renderer->set_draw_colour( &col );
			renderer->draw_rect( editbox->selection.x, editbox->selection.y, editbox->selection.w, editbox->selection.h );
		}
	}
}

static void mgui_editbox_process( MGuiElement* element )
{
	struct MGuiEditbox* editbox;
	editbox = (struct MGuiEditbox*)element;

	if ( BIT_OFF( editbox->flags_int, INTFLAG_FOCUS ) ) return;

	if ( tick_count - editbox->last_update >= 500 )
	{
		editbox->last_update = tick_count;

		editbox->cursor_visible = !editbox->cursor_visible;
		mgui_force_redraw();
	}
}

static void mgui_editbox_set_bounds( MGuiElement* element, bool pos, bool size )
{
	UNREFERENCED_PARAM( pos );
	UNREFERENCED_PARAM( size );

	mgui_editbox_refresh_cursor_bounds( (struct MGuiEditbox*)element );
}

static void mgui_editbox_set_text( MGuiElement* element )
{
	char* str;
	struct MGuiEditbox* editbox;
	editbox = (struct MGuiEditbox*)element;

	// TODO: make this less shitty...
	if ( editbox->buffer )
		mem_free( editbox->buffer );

	editbox->buffer = mstrdup( element->text->buffer, element->text->bufsize / sizeof(char_t) );

	if ( BIT_ON( editbox->flags, FLAG_EDIT_MASKINPUT ) )
	{
		// Mask our input
		for ( str = editbox->buffer; *str; str++ )
		{
			*str = '*';
		}
	}

	editbox->cursor_pos = math_min( editbox->cursor_pos, editbox->text->len );
	editbox->cursor_end = editbox->cursor_pos;

	mgui_editbox_refresh_cursor_bounds( editbox );
}

static void mgui_editbox_on_mouse_click( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y )
{
	uint32 ch;
	uint16 cx, cy;
	char* tmp;
	struct MGuiEditbox* editbox;

	UNREFERENCED_PARAM( button );

	editbox = (struct MGuiEditbox*)element;

	cx = math_max( 0, x - editbox->text->pos.x );
	cy = math_max( 0, y - editbox->text->pos.y );

	// Another ugly hack right here... This really needs rethinking
	tmp = editbox->text->buffer;
	editbox->text->buffer = editbox->buffer;

	ch = mgui_text_get_closest_char( element->text, cx, cy );

	// Ugly hack cleanup
	editbox->text->buffer = tmp;

	editbox->cursor_pos = ch;
	editbox->cursor_end = ch;

	mgui_editbox_refresh_cursor_bounds( editbox );
}

static void mgui_editbox_on_mouse_release( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y )
{
	uint32 ch;
	uint16 cx, cy;
	char* tmp;
	struct MGuiEditbox* editbox;

	UNREFERENCED_PARAM(button);

	editbox = (struct MGuiEditbox*)element;

	cx = math_max( 0, (int16)x - editbox->text->pos.x );
	cy = math_max( 0, (int16)y - editbox->text->pos.y );

	tmp = editbox->text->buffer;
	editbox->text->buffer = editbox->buffer;

	ch = mgui_text_get_closest_char( element->text, cx, cy );

	editbox->text->buffer = tmp;
	editbox->cursor_pos = ch;

	mgui_editbox_refresh_cursor_bounds( editbox );
}

static void mgui_editbox_on_mouse_drag( MGuiElement* element, uint16 x, uint16 y )
{
	uint32 ch;
	uint16 cx, cy;
	char* tmp;
	struct MGuiEditbox* editbox;

	editbox = (struct MGuiEditbox*)element;

	cx = math_max( 0, (int16)x - editbox->text->pos.x );
	cy = math_max( 0, (int16)y - editbox->text->pos.y );

	tmp = editbox->text->buffer;
	editbox->text->buffer = editbox->buffer;

	ch = mgui_text_get_closest_char( element->text, cx, cy );

	editbox->text->buffer = tmp;
	editbox->cursor_pos = ch;

	mgui_editbox_refresh_cursor_bounds( editbox );
}

static void mgui_editbox_on_character( MGuiElement* element, char_t c )
{
	char_t tmp[2];

	if ( (uchar_t)c < ' ' ) return;

	tmp[0] = c;
	tmp[1] = '\0';

	mgui_editbox_insert_text( (struct MGuiEditbox*)element, tmp, 1 );
}

static void mgui_editbox_on_key_press( MGuiElement* element, uint key, bool down )
{
	struct MGuiEditbox* editbox;
	editbox = (struct MGuiEditbox*)element;

	if ( !down ) return;

	switch ( key )
	{
	case 'A':
		if ( input_get_key_state( MKEY_CONTROL ) )
			mgui_editbox_select_all( editbox );
		break;

	case 'X':
		if ( input_get_key_state( MKEY_CONTROL ) )
			mgui_editbox_cut_selection( editbox );
		break;

	case 'C':
		if ( input_get_key_state( MKEY_CONTROL ) )
			mgui_editbox_copy_selection( editbox );
		break;

	case 'V':
		if ( input_get_key_state( MKEY_CONTROL ) )
			mgui_editbox_paste_selection( editbox );
		break;

	case MKEY_BACKSPACE:
		mgui_editbox_press_backspace( editbox );
		break;

	case MKEY_DELETE:
		mgui_editbox_press_delete( editbox );
		break;

	case MKEY_RETURN:
		mgui_editbox_press_return( editbox );
		break;

	case MKEY_LEFT:
		mgui_editbox_move_left( editbox );
		break;

	case MKEY_RIGHT:
		mgui_editbox_move_right( editbox );
		break;

	case MKEY_HOME:
		mgui_editbox_press_home( editbox );
		break;

	case MKEY_END:
		mgui_editbox_press_end( editbox );
		break;
	}
}

void mgui_editbox_get_selection( MGuiEditbox* editbox, char_t* buf, size_t buflen )
{
	size_t len;
	uint32 pos;
	struct MGuiEditbox* edit;
	edit = (struct MGuiEditbox*)editbox;

	if ( editbox == NULL ) return;

	if ( BIT_OFF( edit->flags_int, INTFLAG_FOCUS ) )
	{
		*buf = '\0';
	}

	edit = (struct MGuiEditbox*)editbox;

	len = math_abs( (int32)edit->cursor_end - (int32)edit->cursor_pos ) + 1;
	if ( len == 0 ) return;

	pos = math_min( edit->cursor_pos, edit->cursor_end );
	len = math_min( len, buflen );

	assert( pos <= edit->text->len );

	mstrcpy( buf, &editbox->text->buffer[pos], len );
}

void mgui_editbox_select_text( MGuiEditbox* editbox, uint32 begin, uint32 end )
{
	struct MGuiEditbox* edit;
	edit = (struct MGuiEditbox*)editbox;

	if ( editbox == NULL ) return;
	if ( BIT_OFF( edit->flags_int, INTFLAG_FOCUS ) ) return;

	begin = begin < edit->text->len ? begin : edit->text->len;
	begin = math_max( 0, begin );

	if ( end == 0 ) end = edit->text->len;

	end = end < edit->text->len ? end : edit->text->len;
	end = math_max( begin, end );

	edit->cursor_pos = begin;
	edit->cursor_end = end;

	mgui_editbox_refresh_cursor_bounds( edit );
}

uint32 mgui_editbox_get_cursor_pos( MGuiEditbox* editbox )
{
	struct MGuiEditbox* edit;
	edit = (struct MGuiEditbox*)editbox;

	if ( editbox == NULL ) return 0;
	if ( BIT_OFF( editbox->flags_int, INTFLAG_FOCUS ) ) return 0;

	return edit->cursor_pos;
}

void mgui_editbox_set_cursor_pos( MGuiEditbox* editbox, uint32 pos )
{
	struct MGuiEditbox* edit;
	edit = (struct MGuiEditbox*)editbox;

	if ( editbox == NULL ) return;
	if ( BIT_OFF( editbox->flags_int, INTFLAG_FOCUS ) ) return;

	pos = math_min( pos, edit->text->len );
	edit->cursor_pos = pos;
	edit->cursor_end = pos;

	mgui_force_redraw();
}

static void mgui_editbox_refresh_cursor_bounds( struct MGuiEditbox* editbox )
{
	uint16 x1, y1, x2, y2, w1, w2;
	char_t* tmp;

	tmp = editbox->text->buffer;
	editbox->text->buffer = editbox->buffer;

	mgui_text_get_char_pos( editbox->text, editbox->cursor_pos, &x1, &y1 );
	mgui_text_get_char_pos( editbox->text, editbox->cursor_end, &x2, &y2 );

	editbox->text->buffer = tmp;

	editbox->selection.x = editbox->text->pos.x + math_min( x1, x2 );
	editbox->selection.y = editbox->bounds.y + 3;
	editbox->selection.w = math_abs( (int32)x2 - x1 );
	editbox->selection.h = editbox->bounds.h - 6;

	editbox->cursor.x = editbox->text->pos.x + x1;
	editbox->cursor.y = editbox->bounds.y + 3;
	editbox->cursor.w = 1;
	editbox->cursor.h = editbox->bounds.h - 6;

	// TODO: Eventually make this move/rotate the text instead of clipping the end
	w1 = editbox->selection.w;
	w2 = editbox->bounds.w;
	x1 = editbox->selection.x;
	x2 = editbox->bounds.x;

	editbox->selection.w = x1 + w1 < x2 + w2 ? w1 : w1 - ( (x1+w1) - (x2+w2) );

	mgui_force_redraw();
}

static void mgui_editbox_erase_text( struct MGuiEditbox* editbox, uint32 begin, uint32 end )
{
	char_t* str;
	char_t* str2;

	str = &editbox->text->buffer[begin];
	str2 = &editbox->text->buffer[end];

	while ( *str2 )
	{
		*str++ = *str2++;
	}

	*str = '\0';

	// Secondary buffer
	str = &editbox->buffer[begin];
	str2 = &editbox->buffer[end];

	while ( *str2 )
	{
		*str++ = *str2++;
	}

	*str = '\0';

	editbox->text->len -= ( end - begin );
	editbox->cursor_pos = editbox->cursor_end = begin;

	// Mask input if the user wants it
	if ( BIT_ON( editbox->flags, FLAG_EDIT_MASKINPUT ) )
	{
		str = editbox->text->buffer;
		str2 = editbox->buffer;

		for ( ; *str; str++ ) { *str2++ = '*'; }
	}
	else
	{
		str = editbox->text->buffer;
		str2 = editbox->buffer;

		for ( ; *str; str++ ) { *str2++ = *str; }
	}

	*str2 = '\0';

	mgui_text_update_dimensions( editbox->text );
	mgui_editbox_refresh_cursor_bounds( editbox );
}

static void mgui_editbox_insert_text( struct MGuiEditbox* editbox, const char_t* text, size_t len )
{
	uint32 begin, end;
	size_t size;
	char_t *old, *str;

	old = NULL;

	if ( editbox->cursor_pos != editbox->cursor_end )
	{
		// Some text has been selected, erase it
		begin = math_min( editbox->cursor_pos, editbox->cursor_end );
		end = math_max( editbox->cursor_pos, editbox->cursor_end );

		mgui_editbox_erase_text( editbox, begin, end );
	}

	size = editbox->text->len + len + 1;

	if ( editbox->text->bufsize < size )
	{
		// Allocate a new, big enough buffer for the text + some extra
		size += 32;

		old = editbox->text->buffer;

		mem_free( editbox->buffer );

		editbox->text->bufsize = size;
		editbox->text->buffer = mem_alloc( size * sizeof(char_t) );
		editbox->buffer = mem_alloc( size * sizeof(char_t) );

		mstrcpy( editbox->text->buffer, old, size );
		mem_free( old );
	}

	mstrins( editbox->text->buffer, text, editbox->text->bufsize, editbox->cursor_pos );

	// Finally either copy the text into the secondary buffer or mask the input
	if ( BIT_ON( editbox->flags, FLAG_EDIT_MASKINPUT ) )
	{
		str = editbox->text->buffer;
		old = editbox->buffer;

		for ( ; *str; str++ ) { *old++ = '*'; }
	}
	else
	{
		str = editbox->text->buffer;
		old = editbox->buffer;

		for ( ; *str; str++ ) { *old++ = *str; }
	}

	*old = '\0';

	editbox->text->len += len;
	editbox->cursor_pos += len;
	editbox->cursor_end = editbox->cursor_pos;

	mgui_text_update_dimensions( editbox->text );
	mgui_editbox_refresh_cursor_bounds( editbox );
}

static void mgui_editbox_select_all( struct MGuiEditbox* editbox )
{
	editbox->cursor_pos = editbox->text->len;
	editbox->cursor_end = 0;

	mgui_editbox_refresh_cursor_bounds( editbox );
}

static void mgui_editbox_cut_selection( struct MGuiEditbox* editbox )
{
	char_t buf[512];
	uint32 begin, end;

	if ( editbox->cursor_pos == editbox->cursor_end ) return;

	mgui_editbox_get_selection( cast_elem(editbox), buf, lengthof(buf) );
	copy_to_clipboard( buf );

	begin = math_min( editbox->cursor_pos, editbox->cursor_end );
	end = math_max( editbox->cursor_pos, editbox->cursor_end );

	mgui_editbox_erase_text( editbox, begin, end );
}

static void mgui_editbox_copy_selection( struct MGuiEditbox* editbox )
{
	char_t buf[512];

	if ( editbox->cursor_pos == editbox->cursor_end ) return;

	mgui_editbox_get_selection( cast_elem(editbox), buf, lengthof(buf) );
	copy_to_clipboard( buf );
}

static void mgui_editbox_paste_selection( struct MGuiEditbox* editbox )
{
	const char_t* pasted;
	pasted = paste_from_clipboard();

	if ( !pasted ) return;

	mgui_editbox_insert_text( editbox, pasted, mstrlen(pasted) );
}

static void mgui_editbox_press_backspace( struct MGuiEditbox* editbox )
{
	uint32 begin, end;

	if ( editbox->cursor_pos != editbox->cursor_end )
	{
		begin = math_min( editbox->cursor_pos, editbox->cursor_end );
		end = math_max( editbox->cursor_pos, editbox->cursor_end );

		mgui_editbox_erase_text( editbox, begin, end );
	}
	else if ( editbox->cursor_pos > 0 )
	{
		mgui_editbox_erase_text( editbox, editbox->cursor_pos - 1, editbox->cursor_pos );
	}
}

static void mgui_editbox_press_delete( struct MGuiEditbox* editbox )
{
	uint32 begin, end;

	if ( editbox->cursor_pos != editbox->cursor_end )
	{
		begin = math_min( editbox->cursor_pos, editbox->cursor_end );
		end = math_max( editbox->cursor_pos, editbox->cursor_end );

		mgui_editbox_erase_text( editbox, begin, end );
	}
	else if ( editbox->cursor_pos < editbox->text->len )
	{
		mgui_editbox_erase_text( editbox, editbox->cursor_pos, editbox->cursor_pos + 1 );
	}
}

static void mgui_editbox_press_return( struct MGuiEditbox* editbox )
{
	MGuiEvent event;

	if ( editbox->event_handler )
	{
		event.type = EVENT_INPUT_RETURN;
		event.element = cast_elem(editbox);
		event.data = editbox->event_data;
		event.keyboard.key = MKEY_RETURN;

		editbox->event_handler( &event );
	}
}

static void mgui_editbox_move_left( struct MGuiEditbox* editbox )
{
	editbox->cursor_pos = math_max( 0, (int32)editbox->cursor_pos - 1 );

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;

	mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = get_tick_count();
	editbox->cursor_visible = true;

	mgui_force_redraw();
}

static void mgui_editbox_move_right( struct MGuiEditbox* editbox )
{
	editbox->cursor_pos = math_min( editbox->text->len, editbox->cursor_pos + 1 );

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;

	mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = get_tick_count();
	editbox->cursor_visible = true;

	mgui_force_redraw();
}

static void mgui_editbox_press_home( struct MGuiEditbox* editbox )
{
	editbox->cursor_pos = 0;

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;

	mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = get_tick_count();
	editbox->cursor_visible = true;

	mgui_force_redraw();
}

static void mgui_editbox_press_end( struct MGuiEditbox* editbox )
{
	editbox->cursor_pos = editbox->text->len;

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;

	mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = get_tick_count();
	editbox->cursor_visible = true;

	mgui_force_redraw();
}
