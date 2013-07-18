/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Element.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		A base for all real-world GUI elements.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "Element.h"
#include "Skin.h"
#include "InputHook.h"
#include "Window.h"
#include "Renderer.h"
#include "Platform/Alloc.h"
#include "Stringy/Stringy.h"
#include <stdio.h>
#include <stdarg.h>

extern vectorscreen_t draw_size;
extern list_t* layers;
extern MGuiRenderer* renderer;

static MYLLY_INLINE MGuiElement*	mgui_get_element_at_test_self		( MGuiElement* element, int16 x, int16 y );
static MYLLY_INLINE MGuiElement*	mgui_get_element_at_test_bounds		( MGuiElement* element, int16 x, int16 y );

void mgui_element_create( MGuiElement* element, MGuiElement* parent )
{
	element->flags |= (FLAG_VISIBLE|FLAG_CLIP|FLAG_INHERIT_ALPHA);
	element->skin = skin;

	if ( BIT_OFF( element->flags_int, INTFLAG_NOTEXT ) )
	{
		element->text = mgui_text_create();
		element->text->bounds = &element->bounds;
		element->text->colour.hex = COL_TEXT;
	}

	element->colour.hex = COL_ELEMENT;

	if ( parent )
	{
		// Add this element to it's parents children
		mgui_add_child( parent, element );

		// Set alpha to parent's value, if the parent is an element
		element->colour.a = parent->colour.a;
	}
	else
	{
		// Add this element to the main layer list
		list_push( layers, cast_node(element) );
		element->flags_int |= INTFLAG_LAYER;
	}

	if ( element->text )
	{
		element->text->colour.a = element->colour.a;
	}
}

void mgui_element_destroy( MGuiElement* element )
{
	node_t *node, *tmp;
	extern MGuiFont* wndbutton_font;

	mgui_remove_child( element );

	// Destroy children if any
	if ( element->children )
	{
		list_foreach_safe( element->children, node, tmp )
		{
			mgui_element_destroy( cast_elem(node) );
		}

		list_destroy( element->children );
	}

	if ( element->callbacks->destroy )
		element->callbacks->destroy( element );

	if ( element->text )
		mgui_text_destroy( element->text );

	if ( element->font && element->font != wndbutton_font )
		mgui_font_destroy( element->font );

	mgui_input_cleanup_references( element );

	// Finally free the element itself
	mem_free( element );
}

void mgui_element_render( MGuiElement* element )
{
	node_t* node;
	rectangle_t* r;
	MGuiElement* child;

	if ( element == NULL ) return;
	if ( BIT_OFF( element->flags, FLAG_VISIBLE ) ) return;

	if ( BIT_ON( element->flags, FLAG_CLIP ) )
	{
		if ( element->callbacks->get_clip_region )
			element->callbacks->get_clip_region( element, &r );

		else r = &element->bounds;

		renderer->start_clip( r->x, r->y, r->w, r->h );
	}

	// Render the element itself
	if ( element->callbacks->render )
	{
		element->callbacks->render( element );
	}

	if ( !element->children ) return;

	// Render all the child elements
	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_element_render( child );
	}

	if ( BIT_ON( element->flags, FLAG_CLIP ) )
	{
		renderer->end_clip();
	}

	// Re-enable parent clipping if necessary
	element = element->parent;

	if ( element && BIT_ON( element->flags, FLAG_CLIP ) )
	{
		if ( element->callbacks->get_clip_region )
			element->callbacks->get_clip_region( element, &r );

		else r = &element->bounds;

		renderer->start_clip( r->x, r->y, r->w, r->h );
	}
}

void mgui_element_process( MGuiElement* element )
{
	node_t* node;
	MGuiElement* child;

	if ( element == NULL ) return;
	if ( BIT_OFF( element->flags, FLAG_VISIBLE ) ) return;

	if ( element->callbacks->process )
	{
		element->callbacks->process( element );
	}

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);
		mgui_element_process( child );
	}
}

void mgui_element_request_redraw( void )
{
	extern bool refresh_all;
	refresh_all = true;
}

MGuiElement* mgui_get_element_at( int16 x, int16 y )
{
	node_t* node;
	MGuiElement *ret = NULL;

	if ( layers == NULL || list_empty( layers ) )
	{
		return ret;
	}

	list_foreach_r( layers, node )
	{
		ret = mgui_get_element_at_test_self( cast_elem(node), x, y );
		if ( ret )
		{
			return ret;
		}
	}

	return ret;
}

MYLLY_INLINE MGuiElement* mgui_get_element_at_test_self( MGuiElement* element, int16 x, int16 y )
{
	node_t* node;
	MGuiElement *ret, *tmp = NULL;

	// Check that the element is actually visible
	if ( BIT_OFF( element->flags, FLAG_VISIBLE ) )
	{
		return NULL;
	}

	// Check that the point is within the element (or one of it's sub-elements)
	if ( ( ret = mgui_get_element_at_test_bounds( element, x, y ) ) == NULL )
	{
		return NULL;
	}

	// If so, check all the child elements
	if ( ret->children )
	{
		list_foreach_r( ret->children, node )
		{
			if ( ( tmp = mgui_get_element_at_test_self( cast_elem(node), x, y ) ) != NULL )
			{
				return tmp;
			}
		}
	}

	// If a suitable child was not found check whether this element is a candidate
	if ( BIT_OFF( ret->flags, FLAG_MOUSECTRL ) ||
		 BIT_ON( ret->flags, FLAG_DISABLED ) )
	{
		return NULL;
	}

	return ret;
}

static MYLLY_INLINE MGuiElement* mgui_get_element_at_test_bounds( MGuiElement* element, int16 x, int16 y )
{
	struct MGuiWindow* window;
	MGuiElement* tmp;

	switch ( element->type )
	{
	case GUI_WINDOW:
		{
			window = (struct MGuiWindow*)element;

			if ( !rect_is_point_in( &window->window_bounds, x, y ) )
			{
				return NULL;
			}

			if ( window->closebtn )
			{
				tmp = mgui_get_element_at_test_self( cast_elem(window->closebtn), x, y );
				if ( tmp ) return tmp;
			}

			if ( window->titlebar )
			{
				tmp = mgui_get_element_at_test_self( cast_elem(window->titlebar), x, y );
				if ( tmp ) return tmp;
			}

			return element;

			break;
		}

	default:
		{
			if ( rect_is_point_in( &element->bounds, x, y ) )
			{
				return element;
			}
		}
	}

	return NULL;
}

void mgui_add_child( MGuiElement* parent, MGuiElement* child )
{
	if ( child == NULL ) return;
	if ( child->parent != NULL ) return;
	if ( BIT_ON( child->flags_int, INTFLAG_LAYER ) ) return;

	if ( parent )
	{
		if ( !parent->children ) parent->children = list_create();

		list_push( parent->children, cast_node(child) );
		child->parent = parent;
	}
	else
	{
		list_push( layers, cast_node(child) );
		child->flags_int |= INTFLAG_LAYER;
	}
}

void mgui_remove_child( MGuiElement* child )
{
	if ( child == NULL ) return;

	if ( child->parent && child->parent->children )
	{
		list_remove( child->parent->children, cast_node(child) );
		child->parent = NULL;
	}
	else if ( BIT_ON( child->flags_int, INTFLAG_LAYER ) )
	{
		list_remove( layers, cast_node(child) );
		child->flags_int &= ~INTFLAG_LAYER;
	}
}

void mgui_move_forward( MGuiElement* child )
{
	if ( child == NULL ) return;

	if ( child->parent && child->parent->children )
	{
		// Yes, this is a bit backwards (no pun intended), but the list is rendered from front to back.
		list_move_backward( child->parent->children, cast_node(child) );
	}
	else if ( BIT_ON( child->flags_int, INTFLAG_LAYER ) )
	{
		list_move_backward( layers, cast_node(child) );
	}
}

void mgui_move_backward( MGuiElement* child )
{
	if ( child == NULL ) return;

	if ( child->parent && child->parent->children )
	{
		list_move_forward( child->parent->children, cast_node(child) );
	}
	else if ( BIT_ON( child->flags_int, INTFLAG_LAYER ) )
	{
		list_move_forward( layers, cast_node(child) );
	}
}

void mgui_send_to_top( MGuiElement* child )
{
	if ( child == NULL ) return;

	if ( child->parent && child->parent->children )
	{
		list_send_to_back( child->parent->children, cast_node(child) );
	}
	else if ( BIT_ON( child->flags_int, INTFLAG_LAYER ) )
	{
		list_send_to_back( layers, cast_node(child) );
	}
}

void mgui_send_to_bottom( MGuiElement* child )
{
	if ( child == NULL ) return;

	if ( child->parent && child->parent->children )
	{
		list_send_to_front( child->parent->children, cast_node(child) );
	}
	else if ( BIT_ON( child->flags_int, INTFLAG_LAYER ) )
	{
		list_send_to_front( layers, cast_node(child) );
	}
}

bool mgui_is_child_of( MGuiElement* parent, MGuiElement* child )
{
	if ( parent == NULL || child == NULL ) return false;

	return ( parent == child->parent );
}

void mgui_element_update_abs_pos( MGuiElement* elem )
{
	rectangle_t* r;
	node_t* node;

	if ( elem == NULL ) return;

	if ( elem->parent )
	{
		r = &elem->parent->bounds;

		elem->bounds.x = r->x + (int16)( elem->pos.x * r->w );
		elem->bounds.y = r->y + (int16)( elem->pos.y * r->h );
	}
	else
	{
		elem->bounds.x = (int16)( elem->pos.x * draw_size.x );
		elem->bounds.y = (int16)( elem->pos.y * draw_size.y );
	}

	if ( elem->text )
	{
		elem->text->bounds = &elem->bounds;
		mgui_text_update_position( elem->text );
	}

	if ( elem->callbacks->on_bounds_change )
	{
		elem->callbacks->on_bounds_change( elem, true, false );
	}

	if ( elem->children == NULL ) return;

	list_foreach( elem->children, node )
	{
		mgui_element_update_child_pos( cast_elem(node) );
	}
}

void mgui_element_update_abs_size( MGuiElement* elem )
{
	node_t* node;

	if ( elem == NULL ) return;

	if ( elem->parent )
	{
		elem->bounds.w = (uint16)( elem->size.x * elem->parent->bounds.w );
		elem->bounds.h = (uint16)( elem->size.y * elem->parent->bounds.h );
	}
	else
	{
		elem->bounds.w = (uint16)( elem->size.x * draw_size.x );
		elem->bounds.h = (uint16)( elem->size.y * draw_size.y );
	}

	if ( elem->text )
	{
		elem->text->bounds = &elem->bounds;
		mgui_text_update_position( elem->text );
	}

	if ( elem->callbacks->on_bounds_change )
	{
		elem->callbacks->on_bounds_change( elem, false, true );
	}

	if ( elem->children == NULL ) return;

	list_foreach( elem->children, node )
	{
		mgui_element_update_child_pos( cast_elem(node) );
	}
}

void mgui_element_update_rel_pos( MGuiElement* elem )
{
	rectangle_t* r;
	node_t* node;

	if ( elem == NULL ) return;

	if ( elem->parent )
	{
		r = &elem->parent->bounds;

		elem->pos.x = (float)( elem->bounds.x - r->x ) / r->w;
		elem->pos.y = (float)( elem->bounds.y - r->y ) / r->h;
	}
	else
	{
		elem->pos.x = (float)elem->bounds.x / draw_size.x;
		elem->pos.y = (float)elem->bounds.y / draw_size.y;
	}

	if ( elem->text )
	{
		elem->text->bounds = &elem->bounds;
		mgui_text_update_position( elem->text );
	}

	if ( elem->callbacks->on_bounds_change )
	{
		elem->callbacks->on_bounds_change( elem, true, false );
	}

	if ( elem->children == NULL ) return;

	list_foreach( elem->children, node )
	{
		mgui_element_update_child_pos( cast_elem(node) );
	}
}

void mgui_element_update_rel_size( MGuiElement* elem )
{
	node_t* node;

	if ( elem == NULL ) return;

	if ( elem->parent )
	{
		elem->size.x = (float)elem->bounds.w / elem->parent->bounds.w;
		elem->size.y = (float)elem->bounds.h / elem->parent->bounds.h;
	}
	else
	{
		elem->size.x = (float)elem->bounds.w / draw_size.x;
		elem->size.y = (float)elem->bounds.h / draw_size.y;
	}

	if ( elem->text )
	{
		elem->text->bounds = &elem->bounds;
		mgui_text_update_position( elem->text );
	}

	if ( elem->callbacks->on_bounds_change )
	{
		elem->callbacks->on_bounds_change( elem, false, true );
	}

	if ( !elem->children ) return;

	list_foreach( elem->children, node )
	{
		mgui_element_update_child_pos( cast_elem(node) );
	}
}

void mgui_element_update_child_pos( MGuiElement* elem )
{
	node_t* node;
	rectangle_t* r;

	if ( elem == NULL || elem->parent == NULL ) return;

	r = &elem->parent->bounds;

	elem->bounds.x = r->x + (int16)( elem->pos.x * r->w );
	elem->bounds.y = r->y + (int16)( elem->pos.y * r->h );

	if ( BIT_ON( elem->flags, FLAG_AUTO_RESIZE ) )
	{
		elem->bounds.w = (uint16)( elem->size.x * r->w );
		elem->bounds.h = (uint16)( elem->size.y * r->h );
	}

	if ( elem->callbacks->on_bounds_change )
	{
		elem->callbacks->on_bounds_change( elem, true, false );
	}

	if ( elem->text )
	{
		elem->text->bounds = &elem->bounds;
		mgui_text_update_position( elem->text );
	}

	if ( elem->children == NULL ) return;

	list_foreach( elem->children, node )
	{
		mgui_element_update_child_pos( cast_elem(node) );
	}
}

void mgui_get_pos( MGuiElement* elem, vector2_t* pos )
{
	if ( elem == NULL || pos == NULL ) return;

	pos->x = elem->pos.x;
	pos->y = elem->pos.y;
}

void mgui_set_pos( MGuiElement* elem, const vector2_t* pos )
{
	if ( elem == NULL || pos == NULL ) return;

	elem->pos.x = pos->x;
	elem->pos.y = pos->y;

	mgui_element_update_abs_pos( elem );
}

void mgui_get_size( MGuiElement* elem, vector2_t* size )
{
	if ( elem == NULL || size == NULL ) return;

	size->x = elem->size.x;
	size->y = elem->size.y;
}

void mgui_set_size( MGuiElement* elem, const vector2_t* size )
{
	if ( elem == NULL || size == NULL ) return;

	elem->size.x = size->x;
	elem->size.y = size->y;

	mgui_element_update_abs_size( elem );
}

void mgui_get_abs_pos( MGuiElement* elem, vectorscreen_t* pos )
{
	if ( elem == NULL || pos == NULL ) return;

	if ( elem->parent )
	{
		pos->x = elem->bounds.x - elem->parent->bounds.x;
		pos->y = elem->bounds.y - elem->parent->bounds.y;
	}
	else
	{
		pos->x = elem->bounds.x;
		pos->y = elem->bounds.y;
	}
}

void mgui_set_abs_pos( MGuiElement* elem, const vectorscreen_t* pos )
{
	if ( elem == NULL || pos == NULL ) return;

	if ( elem->parent )
	{
		elem->bounds.x = elem->parent->bounds.x + pos->x;
		elem->bounds.y = elem->parent->bounds.y + pos->y;
	}
	else
	{
		elem->bounds.x = pos->x;
		elem->bounds.y = pos->y;
	}

	mgui_element_update_rel_pos( elem );
}

void mgui_get_abs_size( MGuiElement* elem, vectorscreen_t* size )
{
	if ( elem == NULL || size == NULL ) return;

	size->x = elem->bounds.w;
	size->y = elem->bounds.h;
}

void mgui_set_abs_size( MGuiElement* elem, const vectorscreen_t* size )
{
	if ( elem == NULL || size == NULL ) return;

	elem->bounds.w = size->x;
	elem->bounds.h = size->y;

	mgui_element_update_rel_size( elem );
}

void mgui_get_pos_f( MGuiElement* elem, float* x, float* y )
{
	if ( elem == NULL || x == NULL || y == NULL ) return;

	*x = elem->pos.x;
	*y = elem->pos.y;
}

void mgui_set_pos_f( MGuiElement* elem, float x, float y )
{
	if ( elem == NULL ) return;

	elem->pos.x = x;
	elem->pos.y = y;

	mgui_element_update_abs_pos( elem );
}

void mgui_get_size_f( MGuiElement* elem, float* w, float* h )
{
	if ( elem == NULL || w == NULL || h == NULL ) return;

	*w = elem->size.x;
	*h = elem->size.y;
}

void mgui_set_size_f( MGuiElement* elem, float w, float h )
{
	if ( elem == NULL ) return;

	elem->size.x = w;
	elem->size.y = h;

	mgui_element_update_abs_size( elem );
}

void mgui_get_abs_pos_i( MGuiElement* elem, int16* x, int16* y )
{
	if ( elem == NULL || x == NULL || y == NULL ) return;

	if ( elem->parent )
	{
		*x = elem->bounds.x - elem->parent->bounds.x;
		*y = elem->bounds.y - elem->parent->bounds.y;
	}
	else
	{
		*x = elem->bounds.x;
		*y = elem->bounds.y;
	}
}

void mgui_set_abs_pos_i( MGuiElement* elem, int16 x, int16 y )
{
	if ( elem == NULL ) return;

	if ( elem->parent )
	{
		elem->bounds.x = elem->parent->bounds.x + x;
		elem->bounds.y = elem->parent->bounds.y + y;
	}
	else
	{
		elem->bounds.x = x;
		elem->bounds.y = y;
	}

	mgui_element_update_rel_pos( elem );
}

void mgui_get_abs_size_i( MGuiElement* elem, uint16* w, uint16* h )
{
	if ( elem == NULL || w == NULL || h == NULL ) return;

	*w = elem->bounds.w;
	*h = elem->bounds.h;
}

void mgui_set_abs_size_i( MGuiElement* elem, uint16 w, uint16 h )
{
	if ( elem == NULL ) return;

	elem->bounds.w = w;
	elem->bounds.h = h;

	mgui_element_update_rel_size( elem );
}

void mgui_get_colour( MGuiElement* elem, colour_t* col )
{
	if ( elem == NULL || col == NULL ) return;

	*col = elem->colour;
}

void mgui_set_colour( MGuiElement* elem, const colour_t* col )
{
	if ( elem == NULL || col == NULL ) return;

	elem->colour = *col;
	mgui_set_alpha( elem, col->a );
}

void mgui_get_text_colour( MGuiElement* elem, colour_t* col )
{
	if ( elem == NULL || col == NULL || elem->text == NULL ) return;

	*col = elem->text->colour;
}

void mgui_set_text_colour( MGuiElement* elem, const colour_t* col )
{
	if ( elem == NULL || col == NULL || elem->text == NULL ) return;

	elem->text->colour = *col;
	elem->text->colour.a = elem->colour.a;
}

uint32 mgui_get_colour_i( MGuiElement* elem )
{
	if ( elem == NULL ) return 0;

	return elem->colour.hex;
}

void mgui_set_colour_i( MGuiElement* elem, uint32 hex )
{
	if ( elem == NULL ) return;

	elem->colour.hex = hex;
	mgui_set_alpha( elem, elem->colour.a );
}

uint32 mgui_get_text_colour_i( MGuiElement* elem )
{
	if ( elem == NULL || elem->text == NULL ) return 0;

	return elem->text->colour.hex;
}

void mgui_set_text_colour_i( MGuiElement* elem, uint32 hex )
{
	if ( elem == NULL || elem->text == NULL ) return;

	elem->text->colour.hex = hex;
	elem->text->colour.a = elem->colour.a;
}

uint8 mgui_get_alpha( MGuiElement* elem )
{
	if ( elem == NULL ) return 0;

	return elem->colour.a;
}

void mgui_set_alpha( MGuiElement* elem, uint8 alpha )
{
	node_t* node;
	MGuiElement* child;

	if ( elem == NULL ) return;

	elem->colour.a = alpha;

	if ( elem->text ) elem->text->colour.a = alpha;

	if ( elem->callbacks->on_colour_change )
	{
		elem->callbacks->on_colour_change( elem );
	}

	if ( elem->children == NULL ) return;

	list_foreach( elem->children, node )
	{
		child = cast_elem(node);

		if ( BIT_ON( child->flags, FLAG_INHERIT_ALPHA ) )
		{
			// If this child element is supposed to inherit its parent's alpha, do it
			mgui_set_alpha( child, alpha );
		}
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

	if ( element == NULL || fmt == NULL ) return;
	if ( element->text == NULL ) return;

	va_start( marker, fmt );
	mgui_text_set_buffer_va( element->text, fmt, marker );
	va_end( marker );

	if ( element->callbacks->on_text_change )
	{
		element->callbacks->on_text_change( element );
	}
}

void mgui_set_text_s( MGuiElement* element, const char_t* text )
{
	if ( element == NULL || text == NULL ) return;
	if ( element->text == NULL ) return;

	mgui_text_set_buffer_s( element->text, text );

	if ( element->callbacks->on_text_change )
	{
		element->callbacks->on_text_change( element );
	}
}

uint32 mgui_get_alignment( MGuiElement* element )
{
	if ( element == NULL ) return ALIGN_CENTER;
	if ( element->text == NULL ) return ALIGN_CENTER;

	return element->text->alignment;
}

void mgui_set_alignment( MGuiElement* element, uint32 alignment )
{
	if ( element == NULL ) return;
	if ( element->text == NULL ) return;

	element->text->alignment = alignment;
	mgui_text_update_position( element->text );
}

void mgui_get_text_padding( MGuiElement* element, uint8* top, uint8* bottom, uint8* left, uint8* right )
{
	if ( element == NULL ) return;
	if ( element->text == NULL ) return;

	*top = element->text->pad.top;
	*bottom = element->text->pad.bottom;
	*left = element->text->pad.left;
	*right = element->text->pad.right;
}

void mgui_set_text_padding( MGuiElement* element, uint8 top, uint8 bottom, uint8 left, uint8 right )
{
	if ( element == NULL ) return;
	if ( element->text == NULL ) return;

	element->text->pad.top = top;
	element->text->pad.bottom = bottom;
	element->text->pad.left = left;
	element->text->pad.right = right;

	if ( element->callbacks->on_bounds_change )
	{
		element->callbacks->on_bounds_change( element, false, true );
	}
}

const char_t* mgui_get_font_name( MGuiElement* element )
{
	if ( element == NULL ) return NULL;
	if ( element->font == NULL ) return NULL;

	return element->font->name;
}

uint8 mgui_get_font_size( MGuiElement* element )
{
	if ( element == NULL ) return 0;
	if ( element->font == NULL ) return 0;

	return element->font->size;
}

uint8 mgui_get_font_flags( MGuiElement* element )
{
	if ( element == NULL ) return FFLAG_NONE;
	if ( element->font == NULL ) return FFLAG_NONE;

	return element->font->flags;
}

void mgui_set_font_name( MGuiElement* element, const char_t* font )
{
	if ( element == NULL || font == NULL ) return;
	if ( element->font == NULL ) return;

	element->font = mgui_font_set_font( element->font, font );

	if ( element->text && element->text->buffer )
		mgui_text_update_dimensions( element->text );
}

void mgui_set_font_size( MGuiElement* element, uint8 size )
{
	if ( element == NULL ) return;
	if ( element->font == NULL ) return;

	element->font = mgui_font_set_size( element->font, size );

	if ( element->text && element->text->buffer )
		mgui_text_update_dimensions( element->text );
}

void mgui_set_font_flags( MGuiElement* element, uint8 flags )
{
	if ( element == NULL ) return;
	if ( element->font == NULL ) return;

	element->font = mgui_font_set_flags( element->font, flags );

	if ( element->text && element->text->buffer )
		mgui_text_update_dimensions( element->text );
}

void mgui_set_font( MGuiElement* element, const char_t* font, uint8 size, uint8 flags, uint8 charset )
{
	if ( element == NULL || font == NULL ) return;

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
	if ( element == NULL ) return 0;

	return element->flags;
}

void mgui_set_flags( MGuiElement* element, uint32 flags )
{
	uint32 old;

	if ( element == NULL ) return;

	old = element->flags;
	element->flags = flags;

	if ( element->callbacks->on_flags_change )
	{
		element->callbacks->on_flags_change( element, old );
	}
}

void mgui_add_flags( MGuiElement* element, uint32 flags )
{
	uint32 old;

	if ( element == NULL ) return;

	old = element->flags;
	element->flags |= flags;

	if ( element->callbacks->on_flags_change )
	{
		element->callbacks->on_flags_change( element, old );
	}
}

void mgui_remove_flags( MGuiElement* element, uint32 flags )
{
	uint32 old;

	if ( element == NULL ) return;

	old = element->flags;
	element->flags &= ~flags;

	if ( element->callbacks->on_flags_change )
	{
		element->callbacks->on_flags_change( element, old );
	}
}

void mgui_set_event_handler( MGuiElement* element, mgui_event_handler_t handler, void* data )
{
	if ( element == NULL ) return;

	element->event_handler = handler;
	element->event_data = data;
}
