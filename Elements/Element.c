/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Element.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		A base for all real-world GUI elements.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "Element.h"
#include "Skin.h"
#include "InputHook.h"
#include "Platform/Platform.h"
#include "Stringy/Stringy.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

extern vectorscreen_t screen_size;

// Default callbacks
static void		__mgui_render			( element_t* e )					{ (void)e; }
static void		__mgui_destroy			( element_t* e )					{ (void)e; }
static void		__mgui_process			( element_t* e, uint32 u )			{ (void)e; (void)u; }
static void		__mgui_on_bounds_update	( element_t* e, bool b1, bool b2 )	{ (void)e; (void)b1; (void)b2; }
static void		__mgui_on_colour_update	( element_t* e )					{ (void)e; }
static void		__mgui_on_text_update	( element_t* e )					{ (void)e; }
static void		__mgui_on_mouse_enter	( element_t* e )					{ (void)e; }
static void		__mgui_on_mouse_leave	( element_t* e )					{ (void)e; }
static void		__mgui_on_mouse_click	( element_t* e, MOUSEBTN b, uint16 x, uint16 y )	{ (void)e; (void)b; (void)x; (void)y; }
static void		__mgui_on_mouse_release	( element_t* e, MOUSEBTN b, uint16 x, uint16 y )	{ (void)e; (void)b; (void)x; (void)y; }
static void		__mgui_on_mouse_drag	( element_t* e, uint16 x, uint16 y ){ (void)e; (void)x; (void)y; }
static void		__mgui_on_mouse_wheel	( element_t* e, float f )			{ (void)e; (void)f; }
static void		__mgui_on_character		( element_t* e, char_t c )			{ (void)e; (void)c; }
static void		__mgui_on_key_press		( element_t* e, uint i, bool b )	{ (void)e; (void)i; (void)b; }

void mgui_element_create( element_t* element, control_t* parent, bool has_text )
{
	element->flags |= (FLAG_VISIBLE|FLAG_ELEMENT|FLAG_CLIP);

	if ( has_text )
	{
		element->text = mgui_text_create();
		element->text->bounds = &element->bounds;
		hex_to_colour( COL_TEXT, &element->text->colour );
	}

	hex_to_colour( COL_ELEMENT, &element->colour );

	// Setup default callbacks
	element->render = __mgui_render;
	element->destroy = __mgui_destroy;
	element->process = __mgui_process;
	element->on_bounds_update = __mgui_on_bounds_update;
	element->on_colour_update = __mgui_on_colour_update;
	element->on_text_update = __mgui_on_text_update;
	element->on_mouse_enter = __mgui_on_mouse_enter;
	element->on_mouse_leave = __mgui_on_mouse_leave;
	element->on_mouse_click = __mgui_on_mouse_click;
	element->on_mouse_release = __mgui_on_mouse_release;
	element->on_mouse_drag = __mgui_on_mouse_drag;
	element->on_mouse_wheel = __mgui_on_mouse_wheel;
	element->on_character = __mgui_on_character;
	element->on_key_press = __mgui_on_key_press;

	if ( !parent ) return;
	if ( !parent->children )
		parent->children = list_create();

	mgui_add_child( parent, element );

	// Set parent alpha
	if ( BIT_OFF( parent->flags, FLAG_ELEMENT ) ) return;

	element->colour.a = cast_elem(parent)->colour.a;

	if ( element->text )
		element->text->colour.a = element->colour.a;
}

void mgui_destroy_element( element_t* element )
{
	node_t *node, *tmp;
	element_t* tmpelem;

	if ( element->parent )
		mgui_remove_child( element );

	// Destroy children if any
	if ( element->children )
	{
		list_foreach_safe( element->children, node, tmp )
		{
			tmpelem = cast_elem(node);
			mgui_destroy_element( tmpelem );
		}

		list_destroy( element->children );
	}

	if ( element->destroy )
		element->destroy( element );

	if ( element->text )
		mgui_text_destroy( element->text );

	if ( element->font )
		mgui_font_destroy( element->font );

	mgui_input_cleanup_references( element );

	// Finally free the element itself
	mem_free( element );
}

void mgui_element_render( element_t* element )
{
	node_t* node;
	element_t* child;

	assert( element != NULL );

	if ( BIT_OFF(element->flags, FLAG_VISIBLE) ) return;

	element->render( element );

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_element_render( child );
	}
}

void mgui_element_process( element_t* element, uint32 ticks )
{
	node_t* node;
	element_t* child;

	assert( element != NULL );

	if ( BIT_OFF(element->flags, FLAG_VISIBLE) ) return;

	element->process( element, ticks );

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_element_process( child, ticks );
	}
}

void mgui_update_abs_pos( element_t* element )
{
	rectangle_t* r;
	node_t* node;
	element_t* child;
	
	if ( element->parent )
	{
		r = &element->parent->bounds;

		element->bounds.x = r->x + (uint16)( element->pos.x * r->w );
		element->bounds.y = r->y + (uint16)( element->pos.y * r->h );
	}
	else
	{
		element->bounds.x = (uint16)( element->pos.x * screen_size.x );
		element->bounds.y = (uint16)( element->pos.y * screen_size.y );
	}

	element->text->bounds = &element->bounds;

	mgui_text_update_position( element->text );
	element->on_bounds_update( element, true, false );

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_update_abs_pos( child ); 
	}
}

void mgui_update_abs_size( element_t* element )
{
	rectangle_t* r;
	node_t* node;
	element_t* child;

	if ( element->parent )
	{
		r = &element->parent->bounds;

		element->bounds.w  = (uint16)( element->size.x * r->w );
		element->bounds.h  = (uint16)( element->size.y * r->h );
	}
	else
	{
		element->bounds.w = (uint16)( element->size.x * screen_size.x );
		element->bounds.h  = (uint16)( element->size.y * screen_size.y );
	}

	element->on_bounds_update( element, false, true );

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);

		if ( BIT_ON( child->flags, FLAG_RESIZE ) )
			mgui_update_abs_size( child ); 
	}
}

void mgui_update_rel_pos( element_t* element )
{
	node_t* node;
	element_t* child;

	if ( element->parent )
	{
		element->pos.x = (float)( element->bounds.x - element->parent->bounds.x ) / element->parent->bounds.w;
		element->pos.y = (float)( element->bounds.y - element->parent->bounds.y ) / element->parent->bounds.h;
	}
	else
	{
		element->pos.x = (float)element->bounds.x / screen_size.x;
		element->pos.y = (float)element->bounds.y / screen_size.y;
	}

	element->text->bounds = &element->bounds;

	mgui_text_update_position( element->text );
	element->on_bounds_update( element, true, false );

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_update_abs_pos( child ); 
	}
}

void mgui_update_rel_size( element_t* element )
{
	node_t* node;
	element_t* child;

	if ( element->parent )
	{
		element->size.x = (float)element->bounds.w / element->parent->bounds.w;
		element->size.y = (float)element->bounds.h / element->parent->bounds.h;
	}
	else
	{
		element->size.x = (float)element->bounds.w / screen_size.x;
		element->size.y = (float)element->bounds.h / screen_size.y;
	}

	element->text->bounds = &element->bounds;

	element->on_bounds_update( element, false, true );

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);

		if ( BIT_ON( child->flags, FLAG_RESIZE ) )
			mgui_update_abs_size( child ); 
	}
}

void mgui_get_pos( element_t* element, float* x, float* y )
{
	assert( element != NULL );

	*x = element->pos.x;
	*y = element->pos.y;
}

void mgui_get_size( element_t* element, float* w, float* h )
{
	assert( element != NULL );

	*w = element->size.x;
	*h = element->size.y;
}

void mgui_set_pos( element_t* element, float x, float y )
{
	assert( element != NULL );

	element->pos.x = x;
	element->pos.y = y;

	mgui_update_abs_pos( element );
}

void mgui_set_size( element_t* element, float w, float h )
{
	assert( element != NULL );

	element->size.x = w;
	element->size.y = h;

	mgui_update_abs_size( element );
}

void mgui_get_pos_v( element_t* element, vector2_t* pos )
{
	assert( element != NULL );
	assert( pos != NULL );

	*pos = element->pos;
}

void mgui_get_size_v( element_t* element, vector2_t* size )
{
	assert( element != NULL );
	assert( size != NULL );

	*size = element->size;
}

void mgui_set_pos_v( element_t* element, const vector2_t* pos )
{
	assert( element != NULL );
	assert( pos != NULL );

	element->pos = *pos;

	mgui_update_abs_pos( element );
}

void mgui_set_size_v( element_t* element, const vector2_t* size )
{
	assert( element != NULL );
	assert( size != NULL );

	element->size = *size;

	mgui_update_abs_size( element );
}

void mgui_get_abs_pos( element_t* element, uint16* x, uint16* y )
{
	assert( element != NULL );

	if ( element->parent )
	{
		*x = element->bounds.x - element->parent->bounds.x;
		*y = element->bounds.y - element->parent->bounds.y;
	}
	else
	{
		*x = element->bounds.x;
		*y = element->bounds.y;
	}
}

void mgui_get_abs_size( element_t* element, uint16* w, uint16* h )
{
	assert( element != NULL );

	*w = element->bounds.w;
	*h = element->bounds.h;
}

void mgui_set_abs_pos( element_t* element, uint16 x, uint16 y )
{
	assert( element != NULL );

	if ( element->parent )
	{
		element->bounds.x = element->parent->bounds.x + x;
		element->bounds.y = element->parent->bounds.y + y;
	}
	else
	{
		element->bounds.x = x;
		element->bounds.y = y;
	}

	mgui_update_rel_pos( element );
}

void mgui_set_abs_size( element_t* element, uint16 w, uint16 h )
{
	assert( element != NULL );

	element->bounds.w = w;
	element->bounds.h = h;

	mgui_update_rel_size( element );
}

void mgui_get_abs_pos_v( element_t* element, vectorscreen_t* pos )
{
	assert( element != NULL );
	assert( pos != NULL );

	mgui_get_abs_pos( element, &pos->x, &pos->y );
}

void mgui_get_abs_size_v( element_t* element, vectorscreen_t* size )
{
	assert( element != NULL );
	assert( size != NULL );

	mgui_get_abs_size( element, &size->x, &size->y );
}

void mgui_set_abs_pos_v( element_t* element, const vectorscreen_t* pos )
{
	assert( element != NULL );
	assert( pos != NULL );

	mgui_set_abs_pos( element, pos->x, pos->y );
}

void mgui_set_abs_size_v( element_t* element, const vectorscreen_t* size )
{
	assert( element != NULL );
	assert( size != NULL );

	mgui_set_abs_size( element, size->x, size->y );
}

uint32 mgui_get_colour( element_t* element )
{
	assert( element != NULL );
	return colour_to_hex( &element->colour );
}

void mgui_set_colour( element_t* element, uint32 colour )
{
	node_t* node;
	element_t* child;

	assert( element != NULL );

	hex_to_colour( colour, &element->colour );

	if ( !element->text )
		element->text->colour.a = element->colour.a;

	element->on_colour_update( element );

	if ( !element->children ) return;
	
	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_set_alpha( child, element->colour.a );
	}
}

uint32 mgui_get_text_colour( element_t* element )
{
	assert( element != NULL );

	if ( !element->text ) return 0;

	return colour_to_hex( &element->text->colour );
}

void mgui_set_text_colour( element_t* element, uint32 colour )
{
	assert( element != NULL );

	if ( !element->text ) return;

	hex_to_colour( colour, &element->text->colour );
	element->text->colour.a = element->colour.a;
}

uint8 mgui_get_alpha( element_t* element )
{
	assert( element != NULL );
	return element->colour.a;
}

void mgui_set_alpha( element_t* element, uint8 alpha )
{
	node_t* node;
	element_t* child;

	assert( element != NULL );

	element->colour.a = alpha;

	if ( !element->text )
		element->text->colour.a = alpha;

	element->on_colour_update( element );

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_set_alpha( child, alpha );
	}
}

const char_t* mgui_get_text( element_t* element )
{
	if ( !element->text ) return NULL;

	return element->text->buffer;
}

uint32 mgui_get_text_len( element_t* element )
{
	if ( !element->text ) return 0;

	return element->text->len;
}

void mgui_set_text( element_t* element, const char_t* fmt, ... )
{
	va_list	marker;

	assert( element != NULL );

	if ( !element->text ) return;

	va_start( marker, fmt );
	mgui_text_set_buffer_va( element->text, fmt, marker );
	va_end( marker );

	element->on_text_update( element );
}

void mgui_set_text_s( element_t* element, const char_t* text )
{
	assert( element != NULL );

	if ( !element->text ) return;

	mgui_text_set_buffer_s( element->text, text );
	element->on_text_update( element );
}

uint32 mgui_get_alignment( element_t* element )
{
	assert( element != NULL );
	
	if ( !element->text ) return ALIGN_CENTER;

	return element->text->alignment;
}

void mgui_set_alignment( element_t* element, uint32 alignment )
{
	assert( element != NULL );
	
	if ( !element->text ) return;

	element->text->alignment = alignment;
	mgui_text_update_position( element->text );
}

void mgui_get_text_padding( element_t* element, uint8* top, uint8* bottom, uint8* left, uint8* right )
{
	assert( element != NULL );
	assert( element->text != NULL );

	*top = element->text->pad.top;
	*bottom = element->text->pad.bottom;
	*left = element->text->pad.left;
	*right = element->text->pad.right;
}

void mgui_set_text_padding( element_t* element, uint8 top, uint8 bottom, uint8 left, uint8 right )
{
	assert( element != NULL );
	
	if ( !element->text ) return;

	element->text->pad.top = top;
	element->text->pad.bottom = bottom;
	element->text->pad.left = left;
	element->text->pad.right = right;
}

const char_t* mgui_get_font_name( element_t* element )
{
	assert( element != NULL );

	if ( !element->font ) return NULL;
	return element->font->name;
}

uint8 mgui_get_font_size( element_t* element )
{
	assert( element != NULL );

	if ( !element->font ) return 0;
	return element->font->size;
}

uint8 mgui_get_font_flags( element_t* element )
{
	assert( element != NULL );

	if ( !element->font ) return FFLAG_NONE;
	return element->font->flags;
}

void mgui_set_font_name( element_t* element, const char_t* font )
{
	assert( element != NULL );
	assert( font != NULL );
	assert( element->font != NULL );

	mgui_font_set_font( element->font, font );
}

void mgui_set_font_size( element_t* element, uint8 size )
{
	assert( element != NULL );
	assert( element->font != NULL );

	mgui_font_set_size( element->font, size );
}

void mgui_set_font_flags( element_t* element, uint8 flags )
{
	assert( element != NULL );
	assert( element->font != NULL );

	mgui_font_set_flags( element->font, flags );
}

void mgui_set_font( element_t* element, const char_t* font, uint8 size, uint8 flags, uint8 charset )
{
	assert( element != NULL );
	assert( font != NULL );

	if ( element->font )
		mgui_font_destroy( element->font );

	if ( charset == 0 )
		charset = CHARSET_ANSI;

	element->font = mgui_font_create( font, size, flags, charset );

	if ( element->text )
		element->text->font = element->font;
}

uint32 mgui_get_flags( element_t* element )
{
	assert( element != NULL );
	return element->flags;
}

void mgui_set_flags( element_t* element, const uint32 flags )
{
	assert( element != NULL );

	// Set the given flags, leave internal flags (most significant 16) intact
	element->flags &= 0xFFFF0000;
	element->flags |= (flags & ~0xFFFF0000);
}

void mgui_add_flags( element_t* element, const uint32 flags )
{
	assert( element != NULL );
	element->flags |= (flags & ~0xFFFF0000);
}

void mgui_remove_flags( element_t* element, const uint32 flags )
{
	assert( element != NULL );
	element->flags &= ~(flags & ~0xFFFF0000);
}

void mgui_set_event_handler( element_t* element, mgui_event_handler_t handler, void* data )
{
	assert( element != NULL );

	element->event_handler = handler;
	element->event_data = data;
}
