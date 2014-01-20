/**
 *
 * @file		Editbox.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI editbox related functions.
 *
 * @details		Functions and structures related to GUI editboxes.
 *
 **/

#include "Editbox.h"
#include "Skin.h"
#include "Renderer.h"
#include "Platform/Alloc.h"
#include "Platform/Timer.h"
#include "Platform/Window.h"
#include "Stringy/Stringy.h"
#include "Input/Input.h"

// --------------------------------------------------

extern MGuiRenderer* renderer;
extern uint32 tick_count;

// --------------------------------------------------

static void		mgui_editbox_refresh_cursor_bounds	( struct MGuiEditbox* editbox );
static void		mgui_editbox_erase_text				( struct MGuiEditbox* editbox, uint32 begin, uint32 end );
static void		mgui_editbox_insert_text			( struct MGuiEditbox* editbox, const char_t* text, size_t len );
static void		mgui_editbox_select_all				( struct MGuiEditbox* editbox );
static void		mgui_editbox_cut_selection			( struct MGuiEditbox* editbox );
static void		mgui_editbox_copy_selection			( struct MGuiEditbox* editbox );
static void		mgui_editbox_paste_selection		( struct MGuiEditbox* editbox );
static void		mgui_editbox_press_backspace		( struct MGuiEditbox* editbox );
static void		mgui_editbox_press_delete			( struct MGuiEditbox* editbox );
static void		mgui_editbox_press_return			( struct MGuiEditbox* editbox );
static void		mgui_editbox_move_left				( struct MGuiEditbox* editbox );
static void		mgui_editbox_move_right				( struct MGuiEditbox* editbox );
static void		mgui_editbox_press_home				( struct MGuiEditbox* editbox );
static void		mgui_editbox_press_end				( struct MGuiEditbox* editbox );

// Editbox callback handlers
static void		mgui_editbox_destroy				( MGuiElement* element );
static void		mgui_editbox_render					( MGuiElement* element );
static void		mgui_editbox_process				( MGuiElement* element );
static void		mgui_editbox_on_bounds_change		( MGuiElement* element, bool pos, bool size );
static void		mgui_editbox_on_text_change			( MGuiElement* element );
static void		mgui_editbox_on_mouse_click			( MGuiElement* element, int16 x, int16 y, MOUSEBTN button );
static void		mgui_editbox_on_mouse_release		( MGuiElement* element, int16 x, int16 y, MOUSEBTN button );
static void		mgui_editbox_on_mouse_drag			( MGuiElement* element, int16 x, int16 y );
static bool		mgui_editbox_on_character			( MGuiElement* element, char_t c );
static bool		mgui_editbox_on_key_press			( MGuiElement* element, uint32 key, bool down );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	mgui_editbox_destroy, /* destroy */
	mgui_editbox_render,
	NULL, /* post_render */
	mgui_editbox_process,
	NULL, /* get_clip_region */
	mgui_editbox_on_bounds_change,
	NULL, /* on_flags_change */
	NULL, /* on_colour_change */
	mgui_editbox_on_text_change,
	NULL, /* on_mouse_enter */
	NULL, /* on_mouse_leave */
	mgui_editbox_on_mouse_click,
	mgui_editbox_on_mouse_release,
	mgui_editbox_on_mouse_drag,
	NULL, /* on_mouse_move */
	NULL, /* on_mouse_wheel */
	mgui_editbox_on_character,
	mgui_editbox_on_key_press
};

// --------------------------------------------------

/**
 * @brief Creates an editbox.
 *
 * @details This function creates a GUI editbox. If the parent element
 * is NULL, the editbox will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @returns A pointer to the created editbox
 */
MGuiEditbox* mgui_create_editbox( MGuiElement* parent )
{
	struct MGuiEditbox* editbox;
	extern MGuiFont* default_font;

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

	editbox->font = default_font;
	editbox->text->font = default_font;
	editbox->text->alignment = ALIGN_LEFT|ALIGN_CENTERV;
	editbox->text->pad.left = 5;

	editbox->colour.hex = COL_ELEMENT_DARK;

	// Editbox callbacks
	editbox->callbacks = &callbacks;

	return cast_elem(editbox);
}

/**
 * @brief Creates an editbox (extended).
 *
 * @details This function creates a GUI editbox with the given parameters.
 * If the parent element is NULL, the editbox will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @param x The absolute x coordinate relative to the parent
 * @param y The absolute y coordinate relative to the parent
 * @param w The absolute width of the editbox
 * @param h The absolute height of the editbox
 * @param flags Any additional flags that will be applied as a bitmask (see @ref MGUI_FLAGS)
 * @param col The colour of the editbox as a 32bit hex integer
 * @param text Default text in the editbox (can be NULL)
 *
 * @returns A pointer to the created editbox
 */
MGuiEditbox* mgui_create_editbox_ex( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text )
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

void mgui_editbox_destroy( MGuiElement* element )
{
	struct MGuiEditbox* editbox = (struct MGuiEditbox*)element;

	if ( editbox->buffer )
	{
		mem_free( editbox->buffer );
		editbox->buffer = 0;
	}
}

static void mgui_editbox_render( MGuiElement* element )
{
	element->skin->draw_editbox( element );
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

		mgui_element_request_redraw( element );
	}
}

static void mgui_editbox_on_bounds_change( MGuiElement* element, bool pos, bool size )
{
	UNREFERENCED_PARAM( pos );
	UNREFERENCED_PARAM( size );

	mgui_editbox_refresh_cursor_bounds( (struct MGuiEditbox*)element );
}

static void mgui_editbox_on_text_change( MGuiElement* element )
{
	char* str;
	struct MGuiEditbox* editbox;
	editbox = (struct MGuiEditbox*)element;

	// TODO: make this less shitty...
	if ( editbox->buffer )
		mem_free( editbox->buffer );

	editbox->buffer = mstrdup( element->text->buffer, element->text->bufsize / sizeof(char_t) );

	if ( BIT_ON( editbox->flags, FLAG_EDITBOX_MASKINPUT ) )
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

static void mgui_editbox_on_mouse_click( MGuiElement* element, int16 x, int16 y, MOUSEBTN button )
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

static void mgui_editbox_on_mouse_release( MGuiElement* element, int16 x, int16 y, MOUSEBTN button )
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

static void mgui_editbox_on_mouse_drag( MGuiElement* element, int16 x, int16 y )
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

static bool mgui_editbox_on_character( MGuiElement* element, char_t c )
{
	char_t tmp[2];

	if ( (uchar_t)c < ' ' ) return true;

#ifndef _WIN32
	// Fix for visible Del character on X11
	if ( c == 0x7F ) return true;
#endif

	tmp[0] = c;
	tmp[1] = '\0';

	mgui_editbox_insert_text( (struct MGuiEditbox*)element, tmp, 1 );
	return true;
}

static bool mgui_editbox_on_key_press( MGuiElement* element, uint32 key, bool down )
{
	struct MGuiEditbox* editbox;
	editbox = (struct MGuiEditbox*)element;

	if ( !down ) return true;

	switch ( key )
	{
	case 'A':
		if ( input_get_key_state( MKEY_CONTROL ) )
		{
			mgui_editbox_select_all( editbox );
			return false;
		}
		break;

	case 'X':
		if ( input_get_key_state( MKEY_CONTROL ) )
		{
			mgui_editbox_cut_selection( editbox );
			return false;
		}
		break;

	case 'C':
		if ( input_get_key_state( MKEY_CONTROL ) )
		{
			mgui_editbox_copy_selection( editbox );
			return false;
		}
		break;

	case 'V':
		if ( input_get_key_state( MKEY_CONTROL ) )
		{
			mgui_editbox_paste_selection( editbox );
			return false;
		}
		break;

	case MKEY_BACKSPACE:
		mgui_editbox_press_backspace( editbox );
		return false;

	case MKEY_DELETE:
		mgui_editbox_press_delete( editbox );
		return false;

	case MKEY_RETURN:
		mgui_editbox_press_return( editbox );
		return false;

	case MKEY_LEFT:
		mgui_editbox_move_left( editbox );
		return false;

	case MKEY_RIGHT:
		mgui_editbox_move_right( editbox );
		return false;

	case MKEY_HOME:
		mgui_editbox_press_home( editbox );
		return false;

	case MKEY_END:
		mgui_editbox_press_end( editbox );
		return false;
	}

	return true;
}

/**
 * @brief Returns whether an editbox has text selected.
 *
 * @details This function returns whether the user has selected
 * text within an editbox by painting (a portion of) the text.
 *
 * @param editbox The editbox to check the selection for
 * @returns true if the user has selected text within this editbox, false otherwise
 */
bool mgui_editbox_has_text_selected( MGuiEditbox* editbox )
{
	struct MGuiEditbox* edit;
	edit = (struct MGuiEditbox*)editbox;

	if ( editbox == NULL ) return false;

	return ( edit->cursor_pos != edit->cursor_end );
}

/**
 * @brief Copies the selected text from an editbox into a text buffer.
 *
 * @details This function copies the text selected by the user into
 * another text buffer.
 *
 * @param editbox The editbox to retrieve the selection of
 * @param buf A pointer to a text buffer that will receive the selection
 * @param buflen The length of the buffer in characters
 */
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

/**
 * @brief Selects a portion of the text within an editbox.
 *
 * @details This function selects and paints a portion of the text
 * within an editbox.
 *
 * @param editbox The editbox to set the selection of
 * @param begin Beginning of the selection, offset from the beginning of the text
 * @param end End of the selection, offset from the beginning of the text
 */
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

/**
 * @brief Returns the current position of the editbox's cursor.
 *
 * @details This function returns the position of the editbox's cursor
 * as an offset from the beginning of the string.
 *
 * @param editbox The editbox to get the cursor position of
 * @returns The position of the cursor, as an offset from the beginning of the string
 */
uint32 mgui_editbox_get_cursor_pos( MGuiEditbox* editbox )
{
	struct MGuiEditbox* edit;
	edit = (struct MGuiEditbox*)editbox;

	if ( editbox == NULL ) return 0;
	if ( BIT_OFF( editbox->flags_int, INTFLAG_FOCUS ) ) return 0;

	return edit->cursor_pos;
}

/**
 * @brief Sets the position of the editbox's cursor.
 *
 * @details This function moves the editbox's cursor to the given offset.
 *
 * @param editbox The editbox to set the cursor position of
 * @param pos The new position for the cursor, as an offset from the beginning of the string
 */
void mgui_editbox_set_cursor_pos( MGuiEditbox* editbox, uint32 pos )
{
	struct MGuiEditbox* edit;
	edit = (struct MGuiEditbox*)editbox;

	if ( editbox == NULL ) return;
	if ( BIT_OFF( editbox->flags_int, INTFLAG_FOCUS ) ) return;

	pos = math_min( pos, edit->text->len );
	edit->cursor_pos = pos;
	edit->cursor_end = pos;

	mgui_element_request_redraw( editbox );
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

	mgui_element_request_redraw( cast_elem(editbox) );
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
	if ( BIT_ON( editbox->flags, FLAG_EDITBOX_MASKINPUT ) )
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
	if ( BIT_ON( editbox->flags, FLAG_EDITBOX_MASKINPUT ) )
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
	extern syswindow_t* system_window;
	char_t buf[512];
	uint32 begin, end;

	if ( editbox->cursor_pos == editbox->cursor_end ) return;

	mgui_editbox_get_selection( cast_elem(editbox), buf, lengthof(buf) );
	clipboard_copy( system_window, buf );

	begin = math_min( editbox->cursor_pos, editbox->cursor_end );
	end = math_max( editbox->cursor_pos, editbox->cursor_end );

	mgui_editbox_erase_text( editbox, begin, end );
}

static void mgui_editbox_copy_selection( struct MGuiEditbox* editbox )
{
	extern syswindow_t* system_window;
	char_t buf[512];

	if ( editbox->cursor_pos == editbox->cursor_end ) return;

	mgui_editbox_get_selection( cast_elem(editbox), buf, lengthof(buf) );
	clipboard_copy( system_window, buf );
}

static void mgui_editbox_handle_paste_selection( const char* pasted, void* element )
{
	struct MGuiEditbox* editbox;
	editbox = (struct MGuiEditbox*)element;

	if ( !element ) return;
	if ( !pasted ) return;

	mgui_editbox_insert_text( editbox, pasted, mstrlen(pasted) );
}

static void mgui_editbox_paste_selection( struct MGuiEditbox* editbox )
{
	extern syswindow_t* system_window;

	clipboard_paste( system_window, mgui_editbox_handle_paste_selection, (void*)editbox );
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

	if ( editbox->event_handler == NULL ) return;
	
	event.type = EVENT_INPUT_RETURN;
	event.keyboard.element = cast_elem(editbox);
	event.keyboard.data = editbox->event_data;
	event.keyboard.key = MKEY_RETURN;

	editbox->event_handler( &event );
}

static void mgui_editbox_move_left( struct MGuiEditbox* editbox )
{
	editbox->cursor_pos = math_max( 0, (int32)editbox->cursor_pos - 1 );

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;

	mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = get_tick_count();
	editbox->cursor_visible = true;
}

static void mgui_editbox_move_right( struct MGuiEditbox* editbox )
{
	editbox->cursor_pos = math_min( editbox->text->len, editbox->cursor_pos + 1 );

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;

	mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = get_tick_count();
	editbox->cursor_visible = true;
}

static void mgui_editbox_press_home( struct MGuiEditbox* editbox )
{
	editbox->cursor_pos = 0;

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;

	mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = get_tick_count();
	editbox->cursor_visible = true;
}

static void mgui_editbox_press_end( struct MGuiEditbox* editbox )
{
	editbox->cursor_pos = editbox->text->len;

	if ( !input_get_key_state( MKEY_SHIFT ) )
		editbox->cursor_end = editbox->cursor_pos;

	mgui_editbox_refresh_cursor_bounds( editbox );

	editbox->last_update = get_tick_count();
	editbox->cursor_visible = true;
}
