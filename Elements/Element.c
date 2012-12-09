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
static void		__mgui_render			( MGuiElement* e )					{ (void)e; }
static void		__mgui_destroy			( MGuiElement* e )					{ (void)e; }
static void		__mgui_process			( MGuiElement* e, uint32 u )			{ (void)e; (void)u; }
static void		__mgui_on_bounds_update	( MGuiElement* e, bool b1, bool b2 )	{ (void)e; (void)b1; (void)b2; }
static void		__mgui_on_colour_update	( MGuiElement* e )					{ (void)e; }
static void		__mgui_on_text_update	( MGuiElement* e )					{ (void)e; }
static void		__mgui_on_mouse_enter	( MGuiElement* e )					{ (void)e; }
static void		__mgui_on_mouse_leave	( MGuiElement* e )					{ (void)e; }
static void		__mgui_on_mouse_click	( MGuiElement* e, MOUSEBTN b, uint16 x, uint16 y )	{ (void)e; (void)b; (void)x; (void)y; }
static void		__mgui_on_mouse_release	( MGuiElement* e, MOUSEBTN b, uint16 x, uint16 y )	{ (void)e; (void)b; (void)x; (void)y; }
static void		__mgui_on_mouse_drag	( MGuiElement* e, uint16 x, uint16 y ){ (void)e; (void)x; (void)y; }
static void		__mgui_on_mouse_wheel	( MGuiElement* e, float f )			{ (void)e; (void)f; }
static void		__mgui_on_character		( MGuiElement* e, char_t c )			{ (void)e; (void)c; }
static void		__mgui_on_key_press		( MGuiElement* e, uint i, bool b )	{ (void)e; (void)i; (void)b; }

void mgui_element_create( MGuiElement* element, MGuiControl* parent, bool has_text )
{
	element->flags |= (FLAG_VISIBLE|FLAG_ELEMENT|FLAG_CLIP);

	if ( has_text )
	{
		element->text = mgui_text_create();
		element->text->bounds = &element->bounds;
		element->text->colour.hex = COL_TEXT;
	}

	element->colour.hex = COL_ELEMENT;

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

void mgui_destroy_element( MGuiElement* element )
{
	node_t *node, *tmp;
	MGuiElement* tmpelem;

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

void mgui_element_render( MGuiElement* element )
{
	node_t* node;
	MGuiElement* child;

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

void mgui_element_process( MGuiElement* element, uint32 ticks )
{
	node_t* node;
	MGuiElement* child;

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

void mgui_update_abs_pos( MGuiElement* element )
{
	rectangle_t* r;
	node_t* node;
	MGuiElement* child;
	
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

	if ( element->text )
	{
		element->text->bounds = &element->bounds;
		mgui_text_update_position( element->text );
	}

	element->on_bounds_update( element, true, false );

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_update_abs_pos( child ); 
	}
}

void mgui_update_abs_size( MGuiElement* element )
{
	rectangle_t* r;
	node_t* node;
	MGuiElement* child;

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

void mgui_update_rel_pos( MGuiElement* element )
{
	node_t* node;
	MGuiElement* child;

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

	if ( element->text )
	{
		element->text->bounds = &element->bounds;
		mgui_text_update_position( element->text );
	}

	element->on_bounds_update( element, true, false );

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_update_abs_pos( child ); 
	}
}

void mgui_update_rel_size( MGuiElement* element )
{
	node_t* node;
	MGuiElement* child;

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

	if ( element->text )
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

void mgui_get_pos( MGuiElement* element, float* x, float* y )
{
	assert( element != NULL );

	*x = element->pos.x;
	*y = element->pos.y;
}

void mgui_get_size( MGuiElement* element, float* w, float* h )
{
	assert( element != NULL );

	*w = element->size.x;
	*h = element->size.y;
}

void mgui_set_pos( MGuiElement* element, float x, float y )
{
	assert( element != NULL );

	element->pos.x = x;
	element->pos.y = y;

	mgui_update_abs_pos( element );
}

void mgui_set_size( MGuiElement* element, float w, float h )
{
	assert( element != NULL );

	element->size.x = w;
	element->size.y = h;

	mgui_update_abs_size( element );
}

void mgui_get_pos_v( MGuiElement* element, vector2_t* pos )
{
	assert( element != NULL );
	assert( pos != NULL );

	*pos = element->pos;
}

void mgui_get_size_v( MGuiElement* element, vector2_t* size )
{
	assert( element != NULL );
	assert( size != NULL );

	*size = element->size;
}

void mgui_set_pos_v( MGuiElement* element, const vector2_t* pos )
{
	assert( element != NULL );
	assert( pos != NULL );

	element->pos = *pos;

	mgui_update_abs_pos( element );
}

void mgui_set_size_v( MGuiElement* element, const vector2_t* size )
{
	assert( element != NULL );
	assert( size != NULL );

	element->size = *size;

	mgui_update_abs_size( element );
}

void mgui_get_abs_pos( MGuiElement* element, uint16* x, uint16* y )
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

void mgui_get_abs_size( MGuiElement* element, uint16* w, uint16* h )
{
	assert( element != NULL );

	*w = element->bounds.w;
	*h = element->bounds.h;
}

void mgui_set_abs_pos( MGuiElement* element, uint16 x, uint16 y )
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

void mgui_set_abs_size( MGuiElement* element, uint16 w, uint16 h )
{
	assert( element != NULL );

	element->bounds.w = w;
	element->bounds.h = h;

	mgui_update_rel_size( element );
}

void mgui_get_abs_pos_v( MGuiElement* element, vectorscreen_t* pos )
{
	assert( element != NULL );
	assert( pos != NULL );

	mgui_get_abs_pos( element, &pos->x, &pos->y );
}

void mgui_get_abs_size_v( MGuiElement* element, vectorscreen_t* size )
{
	assert( element != NULL );
	assert( size != NULL );

	mgui_get_abs_size( element, &size->x, &size->y );
}

void mgui_set_abs_pos_v( MGuiElement* element, const vectorscreen_t* pos )
{
	assert( element != NULL );
	assert( pos != NULL );

	mgui_set_abs_pos( element, pos->x, pos->y );
}

void mgui_set_abs_size_v( MGuiElement* element, const vectorscreen_t* size )
{
	assert( element != NULL );
	assert( size != NULL );

	mgui_set_abs_size( element, size->x, size->y );
}

uint32 mgui_get_colour( MGuiElement* element )
{
	assert( element != NULL );
	return element->colour.hex;
}

void mgui_set_colour( MGuiElement* element, uint32 colour )
{
	node_t* node;
	MGuiElement* child;

	assert( element != NULL );

	element->colour.hex = colour;

	if ( element->text )
		element->text->colour.a = element->colour.a;

	element->on_colour_update( element );

	if ( !element->children ) return;
	
	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_set_alpha( child, element->colour.a );
	}
}

uint32 mgui_get_text_colour( MGuiElement* element )
{
	assert( element != NULL );

	if ( !element->text ) return 0;

	return element->text->colour.hex;
}

void mgui_set_text_colour( MGuiElement* element, uint32 colour )
{
	assert( element != NULL );

	if ( !element->text ) return;

	element->text->colour.hex = colour;
	element->text->colour.a = element->colour.a;
}

uint8 mgui_get_alpha( MGuiElement* element )
{
	assert( element != NULL );
	return element->colour.a;
}

void mgui_set_alpha( MGuiElement* element, uint8 alpha )
{
	node_t* node;
	MGuiElement* child;

	assert( element != NULL );

	element->colour.a = alpha;

	if ( element->text )
		element->text->colour.a = alpha;

	element->on_colour_update( element );

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_set_alpha( child, alpha );
	}
}

const char_t* mgui_get_text( MGuiElement* element )
{
	if ( !element->text ) return NULL;

	return element->text->buffer;
}

uint32 mgui_get_text_len( MGuiElement* element )
{
	if ( !element->text ) return 0;

	return element->text->len;
}

void mgui_set_text( MGuiElement* element, const char_t* fmt, ... )
{
	va_list	marker;

	assert( element != NULL );

	if ( !element->text ) return;

	va_start( marker, fmt );
	mgui_text_set_buffer_va( element->text, fmt, marker );
	va_end( marker );

	element->on_text_update( element );
}

void mgui_set_text_s( MGuiElement* element, const char_t* text )
{
	assert( element != NULL );

	if ( !element->text ) return;

	mgui_text_set_buffer_s( element->text, text );
	element->on_text_update( element );
}

uint32 mgui_get_alignment( MGuiElement* element )
{
	assert( element != NULL );
	
	if ( !element->text ) return ALIGN_CENTER;

	return element->text->alignment;
}

void mgui_set_alignment( MGuiElement* element, uint32 alignment )
{
	assert( element != NULL );
	
	if ( !element->text ) return;

	element->text->alignment = alignment;
	mgui_text_update_position( element->text );
}

void mgui_get_text_padding( MGuiElement* element, uint8* top, uint8* bottom, uint8* left, uint8* right )
{
	assert( element != NULL );
	assert( element->text != NULL );

	*top = element->text->pad.top;
	*bottom = element->text->pad.bottom;
	*left = element->text->pad.left;
	*right = element->text->pad.right;
}

void mgui_set_text_padding( MGuiElement* element, uint8 top, uint8 bottom, uint8 left, uint8 right )
{
	assert( element != NULL );
	
	if ( !element->text ) return;

	element->text->pad.top = top;
	element->text->pad.bottom = bottom;
	element->text->pad.left = left;
	element->text->pad.right = right;

	element->on_bounds_update( element, false, true );
}

const char_t* mgui_get_font_name( MGuiElement* element )
{
	assert( element != NULL );

	if ( !element->font ) return NULL;
	return element->font->name;
}

uint8 mgui_get_font_size( MGuiElement* element )
{
	assert( element != NULL );

	if ( !element->font ) return 0;
	return element->font->size;
}

uint8 mgui_get_font_flags( MGuiElement* element )
{
	assert( element != NULL );

	if ( !element->font ) return FFLAG_NONE;
	return element->font->flags;
}

void mgui_set_font_name( MGuiElement* element, const char_t* font )
{
	assert( element != NULL );
	assert( font != NULL );
	assert( element->font != NULL );

	mgui_font_set_font( element->font, font );

	if ( element->text && element->text->buffer )
		mgui_text_update_dimensions( element->text );
}

void mgui_set_font_size( MGuiElement* element, uint8 size )
{
	assert( element != NULL );
	assert( element->font != NULL );

	mgui_font_set_size( element->font, size );

	if ( element->text && element->text->buffer )
		mgui_text_update_dimensions( element->text );
}

void mgui_set_font_flags( MGuiElement* element, uint8 flags )
{
	assert( element != NULL );
	assert( element->font != NULL );

	mgui_font_set_flags( element->font, flags );

	if ( element->text && element->text->buffer )
		mgui_text_update_dimensions( element->text );
}

void mgui_set_font( MGuiElement* element, const char_t* font, uint8 size, uint8 flags, uint8 charset )
{
	assert( element != NULL );
	assert( font != NULL );

	if ( element->font )
		mgui_font_destroy( element->font );

	if ( charset == 0 )
		charset = CHARSET_ANSI;

	element->font = mgui_font_create( font, size, flags, charset );

	if ( element->text )
	{
		element->text->font = element->font;

		if ( element->text->buffer )
			mgui_text_update_dimensions( element->text );
	}
}

uint32 mgui_get_flags( MGuiElement* element )
{
	assert( element != NULL );
	return element->flags;
}

void mgui_set_flags( MGuiElement* element, const uint32 flags )
{
	assert( element != NULL );

	// Set the given flags, leave internal flags (most significant 16) intact
	element->flags &= 0xFFFF0000;
	element->flags |= (flags & ~0xFFFF0000);
}

void mgui_add_flags( MGuiElement* element, const uint32 flags )
{
	assert( element != NULL );
	element->flags |= (flags & ~0xFFFF0000);
}

void mgui_remove_flags( MGuiElement* element, const uint32 flags )
{
	assert( element != NULL );
	element->flags &= ~(flags & ~0xFFFF0000);
}

void mgui_set_event_handler( MGuiElement* element, mgui_event_handler_t handler, void* data )
{
	assert( element != NULL );

	element->event_handler = handler;
	element->event_data = data;
}
