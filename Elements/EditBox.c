/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		EditBox.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI editbox related functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "EditBox.h"
#include "Skin.h"
#include "Renderer.h"
#include "Platform/Platform.h"
#include "Stringy/Stringy.h"
#include "Input/Input.h"
#include <assert.h>
#include <malloc.h>

static void __mgui_destroy_editbox( element_t* editbox );
static void __mgui_editbox_render( element_t* element );
static void __mgui_editbox_process( element_t* element, uint32 ticks );
static void __mgui_editbox_on_text_update( element_t* element );
static void __mgui_editbox_on_mouse_click( element_t* element, MOUSEBTN button, uint16 x, uint16 y );
static void __mgui_editbox_on_mouse_release( element_t* element, MOUSEBTN button, uint16 x, uint16 y );
static void __mgui_editbox_on_mouse_drag( element_t* element, uint16 x, uint16 y );
static void __mgui_editbox_on_character( element_t* element, char_t c );
static void __mgui_editbox_on_key_press( element_t* element, uint key, bool down );

editbox_t* mgui_create_editbox( control_t* parent )
{
	struct editbox_s* editbox;

	editbox = mem_alloc_clean( sizeof(*editbox) );
	mgui_element_create( cast_elem(editbox), parent, true );

	editbox->type = GUI_EDITBOX;
	editbox->flags |= (FLAG_BORDER|FLAG_BACKGROUND|FLAG_CLIP|FLAG_DRAGGABLE|FLAG_MOUSECTRL|FLAG_KBCTRL);

	editbox->font = mgui_font_create( DEFAULT_FONT, 11, FFLAG_NONE, ANSI_CHARSET );
	editbox->text->font = editbox->font;
	editbox->text->alignment = ALIGN_LEFT|ALIGN_CENTERV;
	editbox->text->pad.left = 5;

	// Create an initial buffer
	editbox->text->bufsize = 20;
	editbox->text->buffer = mem_alloc( editbox->text->bufsize );
	editbox->buffer = mem_alloc( editbox->text->bufsize );
	*editbox->text->buffer = '\0';
	*editbox->buffer = '\0';

	editbox->cursor.w = 1;

	hex_to_colour( COL_ELEMENT_DARK, &editbox->colour );

	// Editbox callbacks
	editbox->destroy = __mgui_destroy_editbox;
	editbox->render = __mgui_editbox_render;
	editbox->process = __mgui_editbox_process;
	editbox->on_text_update = __mgui_editbox_on_text_update;
	editbox->on_mouse_click = __mgui_editbox_on_mouse_click;
	editbox->on_mouse_release = __mgui_editbox_on_mouse_release;
	editbox->on_mouse_drag = __mgui_editbox_on_mouse_drag;
	editbox->on_character = __mgui_editbox_on_character;
	editbox->on_key_press = __mgui_editbox_on_key_press;

	return cast_elem(editbox);
}

static void __mgui_destroy_editbox( element_t* editbox )
{
	UNREFERENCED_PARAM(editbox);
}

static void __mgui_editbox_render( element_t* element )
{
	struct editbox_s* editbox;
	colour_t col;
	char* tmp;

	editbox = (struct editbox_s*)element;

	// This is a really ugly hack to make mgui_get_text return the correct buffer:
	// We replace the text_t buffer with our own (with masked input etc cool)
	// while we render, and put the original buffer back afterwards
	tmp = editbox->text->buffer;
	editbox->text->buffer = editbox->buffer;

	skin->draw_editbox( &editbox->bounds, &editbox->colour, editbox->flags, editbox->text );

	if ( BIT_ON( editbox->flags, FLAG_FOCUS ) )
	{
		if ( editbox->cursor_visible )
		{
			// Draw the cursor, make sure it's within the editbox boundaries
			if ( editbox->cursor.x < editbox->bounds.x + editbox->bounds.w )
				skin->draw_panel( &editbox->cursor, &editbox->text->colour );
		}

		if ( editbox->cursor_pos != editbox->cursor_end )
		{
			// Draw the selection. TODO: Change text colour as well
			col = editbox->colour;
			colour_invert( &col );
			col.a = 100;

			skin->draw_panel( &editbox->selection, &col );
		}
	}

	// Really ugly hack cleanup
	editbox->text->buffer = tmp;
}

static void __mgui_editbox_process( element_t* element, uint32 ticks )
{
	struct editbox_s* editbox;
	editbox = (struct editbox_s*)element;

	if ( BIT_OFF( editbox->flags, FLAG_FOCUS ) ) return;

	if ( ticks - editbox->last_update >= 500 )
	{
		editbox->last_update = ticks;

		editbox->cursor_visible = !editbox->cursor_visible;
		mgui_force_redraw();
	}
}

static void __mgui_editbox_refresh_cursor_bounds( struct editbox_s* editbox )
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

static void __mgui_editbox_on_text_update( element_t* element )
{
	char* str;
	struct editbox_s* editbox;
	editbox = (struct editbox_s*)element;

	// TODO: make this less shitty...
	if ( editbox->buffer )
		mem_free( editbox->buffer );

	editbox->buffer = mstrdup( element->text->buffer, element->text->bufsize / sizeof(char_t) );

	if ( BIT_ON( editbox->flags, FLAG_MASKINPUT ) )
	{
		// Mask our input
		for ( str = editbox->buffer; *str; str++ )
			*str = '*';
	}

	editbox->cursor_pos = math_min( editbox->cursor_pos, editbox->text->len );
	editbox->cursor_end = editbox->cursor_pos;

	__mgui_editbox_refresh_cursor_bounds( editbox );
}

static void __mgui_editbox_on_mouse_click( element_t* element, MOUSEBTN button, uint16 x, uint16 y )
{
	uint32 ch;
	uint16 cx, cy;
	char* tmp;
	struct editbox_s* editbox;

	UNREFERENCED_PARAM(button);

	editbox = (struct editbox_s*)element;

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

	__mgui_editbox_refresh_cursor_bounds( editbox );
}

static void __mgui_editbox_on_mouse_release( element_t* element, MOUSEBTN button, uint16 x, uint16 y )
{
	uint32 ch;
	uint16 cx, cy;
	char* tmp;
	struct editbox_s* editbox;

	UNREFERENCED_PARAM(button);

	editbox = (struct editbox_s*)element;

	cx = math_max( 0, (int16)x - editbox->text->pos.x );
	cy = math_max( 0, (int16)y - editbox->text->pos.y );

	tmp = editbox->text->buffer;
	editbox->text->buffer = editbox->buffer;

	ch = mgui_text_get_closest_char( element->text, cx, cy );

	editbox->text->buffer = tmp;
	editbox->cursor_pos = ch;

	__mgui_editbox_refresh_cursor_bounds( editbox );
}

static void __mgui_editbox_on_mouse_drag( element_t* element, uint16 x, uint16 y )
{
	uint32 ch;
	uint16 cx, cy;
	char* tmp;
	struct editbox_s* editbox;

	editbox = (struct editbox_s*)element;

	cx = math_max( 0, (int16)x - editbox->text->pos.x );
	cy = math_max( 0, (int16)y - editbox->text->pos.y );

	tmp = editbox->text->buffer;
	editbox->text->buffer = editbox->buffer;

	ch = mgui_text_get_closest_char( element->text, cx, cy );

	editbox->text->buffer = tmp;
	editbox->cursor_pos = ch;

	__mgui_editbox_refresh_cursor_bounds( editbox );
}

static void __mgui_editbox_erase_text( struct editbox_s* editbox, uint32 begin, uint32 end )
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
	if ( BIT_ON( editbox->flags, FLAG_MASKINPUT ) )
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
	__mgui_editbox_refresh_cursor_bounds( editbox );
}

static void __mgui_editbox_insert_text( struct editbox_s* editbox, const char_t* text, size_t len )
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

		__mgui_editbox_erase_text( editbox, begin, end );
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
	if ( BIT_ON( editbox->flags, FLAG_MASKINPUT ) )
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
	__mgui_editbox_refresh_cursor_bounds( editbox );
}

static void __mgui_editbox_on_character( element_t* element, char_t c )
{
	char_t tmp[2];

	if ( input_get_key_state( MKEY_CONTROL ) ) return;
	if ( input_get_key_state( MKEY_ALT ) ) return;
	if ( (uchar_t)c < ' ' ) return;

	tmp[0] = c;
	tmp[1] = '\0';

	__mgui_editbox_insert_text( (struct editbox_s*)element, tmp, 1 );
}

static void __mgui_editbox_select_all( struct editbox_s* editbox )
{
	editbox->cursor_pos = editbox->text->len;
	editbox->cursor_end = 0;

	__mgui_editbox_refresh_cursor_bounds( editbox );
}

static void __mgui_editbox_cut_selection( struct editbox_s* editbox )
{
	char_t buf[512];
	uint32 begin, end;

	if ( editbox->cursor_pos == editbox->cursor_end ) return;

	mgui_editbox_get_selection( cast_elem(editbox), buf, lengthof(buf) );
	copy_to_clipboard( buf );

	begin = math_min( editbox->cursor_pos, editbox->cursor_end );
	end = math_max( editbox->cursor_pos, editbox->cursor_end );

	__mgui_editbox_erase_text( editbox, begin, end );
}

static void __mgui_editbox_copy_selection( struct editbox_s* editbox )
{
	char_t buf[512];

	if ( editbox->cursor_pos == editbox->cursor_end ) return;

	mgui_editbox_get_selection( cast_elem(editbox), buf, lengthof(buf) );
	copy_to_clipboard( buf );
}

static void __mgui_editbox_paste_selection( struct editbox_s* editbox )
{
	const char_t* pasted;
	pasted = paste_from_clipboard();

	if ( !pasted ) return;

	__mgui_editbox_insert_text( editbox, pasted, mstrlen(pasted) );
}

static void __mgui_editbox_press_backspace( struct editbox_s* editbox )
{
	uint32 begin, end;

	if ( editbox->cursor_pos != editbox->cursor_end )
	{
		begin = math_min( editbox->cursor_pos, editbox->cursor_end );
		end = math_max( editbox->cursor_pos, editbox->cursor_end );

		__mgui_editbox_erase_text( editbox, begin, end );
	}
	else if ( editbox->cursor_pos > 0 )
	{
		__mgui_editbox_erase_text( editbox, editbox->cursor_pos - 1, editbox->cursor_pos );
	}
}

static void __mgui_editbox_press_delete( struct editbox_s* editbox )
{
	uint32 begin, end;

	if ( editbox->cursor_pos != editbox->cursor_end )
	{
		begin = math_min( editbox->cursor_pos, editbox->cursor_end );
		end = math_max( editbox->cursor_pos, editbox->cursor_end );

		__mgui_editbox_erase_text( editbox, begin, end );
	}
	else if ( editbox->cursor_pos < editbox->text->len )
	{
		__mgui_editbox_erase_text( editbox, editbox->cursor_pos, editbox->cursor_pos + 1 );
	}
}

static void __mgui_editbox_press_return( struct editbox_s* editbox )
{
	guievent_t event;

	if ( editbox->event_handler )
	{
		event.type = EVENT_INPUT_RETURN;
		event.element = cast_elem(editbox);
		event.data = editbox->event_data;
		event.keyboard.key = MKEY_RETURN;

		editbox->event_handler( &event );
	}
}

static void __mgui_editbox_move_left( struct editbox_s* editbox )
{
	editbox->cursor_pos = math_max( 0, (int32)editbox->cursor_pos - 1 );

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;
	
	__mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = timer_get_ticks();
	editbox->cursor_visible = true;

	mgui_force_redraw();
}

static void __mgui_editbox_move_right( struct editbox_s* editbox )
{
	editbox->cursor_pos = math_min( editbox->text->len, editbox->cursor_pos + 1 );

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;

	__mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = timer_get_ticks();
	editbox->cursor_visible = true;

	mgui_force_redraw();
}

static void __mgui_editbox_press_home( struct editbox_s* editbox )
{
	editbox->cursor_pos = 0;

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;

	__mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = timer_get_ticks();
	editbox->cursor_visible = true;

	mgui_force_redraw();
}

static void __mgui_editbox_press_end( struct editbox_s* editbox )
{
	editbox->cursor_pos = editbox->text->len;

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;

	__mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = timer_get_ticks();
	editbox->cursor_visible = true;

	mgui_force_redraw();
}

static void __mgui_editbox_on_key_press( element_t* element, uint key, bool down )
{
	struct editbox_s* editbox;
	editbox = (struct editbox_s*)element;

	if ( !down ) return;

	switch ( key )
	{
	case 'A':
		if ( input_get_key_state( MKEY_CONTROL ) )
			__mgui_editbox_select_all( editbox );
		break;

	case 'X':
		if ( input_get_key_state( MKEY_CONTROL ) )
			__mgui_editbox_cut_selection( editbox );
		break;

	case 'C':
		if ( input_get_key_state( MKEY_CONTROL ) )
			__mgui_editbox_copy_selection( editbox );
		break;

	case 'V':
		if ( input_get_key_state( MKEY_CONTROL ) )
			__mgui_editbox_paste_selection( editbox );
		break;

	case MKEY_BACKSPACE:
		__mgui_editbox_press_backspace( editbox );
		break;

	case MKEY_DELETE:
		__mgui_editbox_press_delete( editbox );
		break;

	case MKEY_RETURN:
		__mgui_editbox_press_return( editbox );
		break;
		
	case MKEY_LEFT:
		__mgui_editbox_move_left( editbox );
		break;

	case MKEY_RIGHT:
		__mgui_editbox_move_right( editbox );
		break;

	case MKEY_HOME:
		__mgui_editbox_press_home( editbox );
		break;

	case MKEY_END:
		__mgui_editbox_press_end( editbox );
		break;
	}
}

void mgui_editbox_get_selection( editbox_t* editbox, char_t* buf, size_t buflen )
{
	size_t len;
	uint32 pos;
	struct editbox_s* edit;
	edit = (struct editbox_s*)editbox;

	assert( editbox != NULL );

	if ( BIT_OFF( edit->flags, FLAG_FOCUS ) )
	{
		*buf = _TEXT('\0');
	}

	edit = (struct editbox_s*)editbox;

	len = math_abs( (int32)edit->cursor_end - (int32)edit->cursor_pos ) + 1;
	if ( len == 0 ) return;

	pos = math_min( edit->cursor_pos, edit->cursor_end );
	len = math_min( len, buflen );

	assert( pos <= edit->text->len );

	mstrcpy( buf, &editbox->text->buffer[pos], len );
}

void mgui_editbox_select_text( editbox_t* editbox, uint32 begin, uint32 end )
{
	struct editbox_s* edit;
	edit = (struct editbox_s*)editbox;

	assert( editbox != NULL );

	if ( BIT_OFF( edit->flags, FLAG_FOCUS ) ) return;

	begin = begin < edit->text->len ? begin : edit->text->len;
	begin = math_max( 0, begin );

	if ( end == 0 ) end = edit->text->len;

	end = end < edit->text->len ? end : edit->text->len;
	end = math_max( begin, end );

	edit->cursor_pos = begin;
	edit->cursor_end = end;

	__mgui_editbox_refresh_cursor_bounds( edit );
}

uint32 mgui_editbox_get_cursor_pos( editbox_t* editbox )
{
	struct editbox_s* edit;
	edit = (struct editbox_s*)editbox;

	assert( editbox != NULL );

	if ( BIT_OFF( editbox->flags, FLAG_FOCUS ) ) return 0;

	return edit->cursor_pos;
}

void mgui_editbox_set_cursor_pos( editbox_t* editbox, uint32 pos )
{
	struct editbox_s* edit;
	edit = (struct editbox_s*)editbox;

	assert( editbox != NULL );

	if ( BIT_OFF( editbox->flags, FLAG_FOCUS ) ) return;

	pos = math_min( pos, edit->text->len );
	edit->cursor_pos = pos;
	edit->cursor_end = pos;

	mgui_force_redraw();
}
