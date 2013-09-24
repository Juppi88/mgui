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

// --------------------------------------------------

extern vectorscreen_t draw_size;
extern list_t* layers;
extern MGuiRenderer* renderer;

// --------------------------------------------------

static MYLLY_INLINE MGuiElement*	mgui_get_element_at_test_self		( MGuiElement* element, int16 x, int16 y );
static MYLLY_INLINE MGuiElement*	mgui_get_element_at_test_bounds		( MGuiElement* element, int16 x, int16 y );

// --------------------------------------------------

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
	element->z_depth = 1.0f;

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

	if ( element->text != NULL )
		element->text->colour.a = element->colour.a;
}

void mgui_element_destroy( MGuiElement* element )
{
	node_t *node, *tmp;

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

	if ( element->font != NULL )
		mgui_font_destroy( element->font );

	mgui_input_cleanup_references( element );

	// Finally free the element itself
	mem_free( element );
}

void mgui_element_render_cache( MGuiElement* element, bool draw_self )
{
	node_t* node;
	rectangle_t* r;
	static colour_t cache_colour = { 0xFFFFFFFF };

	if ( element == NULL ) return;
	if ( BIT_OFF( element->flags, FLAG_VISIBLE ) ) return;

	// Did we get a request from upstairs to draw ourselves?
	// Do it anyway if we have a cache, otherwise just pass the order on.
	if ( !draw_self && element->cache == NULL )
	{
		element->flags_int &= ~INTFLAG_REFRESH;
		if ( element->children == NULL ) return;
		
		list_foreach( element->children, node )
		{
			mgui_element_render_cache( cast_elem(node), false );
		}
		return;
	}

	// Get element boundaries
	r = element->callbacks->get_clip_region ?
		element->callbacks->get_clip_region( element, &r ), r :
		&element->bounds;

	// Can we just draw the element from the old cache?
	if ( element->cache && BIT_OFF( element->flags_int, INTFLAG_REFRESH ) )
	{
		cache_colour.a = element->colour.a;

		renderer->set_draw_colour( &cache_colour );
		renderer->draw_render_target( element->cache, r->x, r->y, r->w, r->h );
		return;
	}

	// If not, we'll really have to draw everything again.
	if ( element->cache != NULL )
		renderer->enable_render_target( element->cache, r->x, r->y );

	// Set clipping region and toggle clip mode.
	if ( element->flags & FLAG_CLIP )
		renderer->start_clip( r->x, r->y, r->w, r->h );

	// Render the element itself
	if ( element->callbacks->render )
		element->callbacks->render( element );

	// If the element has any children, draw them now.
	if ( element->children )
	{
		list_foreach( element->children, node )
		{
			mgui_element_render_cache( (MGuiElement*)node, true );
		}
	}

	// Disable clip mode.
	if ( element->flags & FLAG_CLIP )
		renderer->end_clip();

	// Disable render target.
	if ( element->cache != NULL )
	{
		element->flags_int &= ~INTFLAG_REFRESH;
		renderer->disable_render_target( element->cache );

		if ( draw_self )
			renderer->draw_render_target( element->cache, r->x, r->y, r->w, r->h );
	}

	// Re-enable parent clipping if necessary.
	element = element->parent;

	if ( element && element->flags & FLAG_CLIP )
	{
		r = element->callbacks->get_clip_region ?
			element->callbacks->get_clip_region( element, &r ), r :
			&element->bounds;

		renderer->start_clip( r->x, r->y, r->w, r->h );
	}
}

void mgui_element_render( MGuiElement* element )
{
	node_t* node;
	rectangle_t* r;
	DRAW_MODE draw_mode = DRAWING_INVALID;
	static colour_t cache_colour = { 0xFFFFFFFF };

	if ( element == NULL ) return;
	if ( BIT_OFF( element->flags, FLAG_VISIBLE ) ) return;

	// Get element boundaries
	r = element->callbacks->get_clip_region ?
		element->callbacks->get_clip_region( element, &r ), r :
		&element->bounds;

	// If the element is a 3D element, toggle special drawing mode.
	// Make sure the element is also a root element (to avoid some dodgy results)
	if ( element->parent == NULL )
	{
		switch ( element->flags & (FLAG_3D_ENTITY|FLAG_DEPTH_TEST) )
		{
		case FLAG_DEPTH_TEST:
			draw_mode = renderer->set_draw_mode( DRAWING_2D_DEPTH );
			renderer->set_draw_depth( element->z_depth );
			break;

		case FLAG_3D_ENTITY:
			draw_mode = renderer->set_draw_mode( DRAWING_3D );
			renderer->set_draw_transform( &element->transform->transform );
			renderer->set_draw_depth( element->z_depth );
			break;
		}
	}

	// Do we have a cache texture?
	if ( element->cache != NULL )
	{
		cache_colour.a = element->colour.a;

		renderer->set_draw_colour( &cache_colour );
		renderer->draw_render_target( element->cache, r->x, r->y, r->w, r->h );

		// This fixes a bug which didn't update the element's cache in some cases
		// after it was made visible. It's not really ideal but it seems to work.
		if ( element->flags_int & INTFLAG_REFRESH )
			mgui_element_request_redraw( element );
	}
	else
	{
		// Set clipping region and toggle clip mode.
		if ( BIT_ON( element->flags, FLAG_CLIP ) )
			renderer->start_clip( r->x, r->y, r->w, r->h );

		// Render the element itself
		if ( element->callbacks->render )
			element->callbacks->render( element );

		// If the element has children, draw them now.
		if ( element->children )
		{
			list_foreach( element->children, node )
			{
				mgui_element_render( cast_elem(node) );
			}
		}

		// Disable clip mode.
		if ( element->flags & FLAG_CLIP )
			renderer->end_clip();
	}

	// Reset draw modes back to original.
	if ( draw_mode != DRAWING_INVALID )
	{
		renderer->set_draw_mode( draw_mode );
		renderer->set_draw_depth( 1.0f );
	}

	if ( element->flags & FLAG_3D_ENTITY )
		renderer->reset_draw_transform();

	// Do post-render processing (effects etc.)
	if ( element->callbacks->post_render )
		element->callbacks->post_render( element );

	// Re-enable parent clipping if necessary.
	element = element->parent;

	if ( element && BIT_ON( element->flags, FLAG_CLIP ) )
	{
		r = element->callbacks->get_clip_region ?
			element->callbacks->get_clip_region( element, &r ), r :
			&element->bounds;

		renderer->start_clip( r->x, r->y, r->w, r->h );
	}
}

void mgui_element_process( MGuiElement* element )
{
	node_t* node;

	if ( element == NULL ) return;
	if ( BIT_OFF( element->flags, FLAG_VISIBLE ) ) return;

	if ( element->callbacks->process )
		element->callbacks->process( element );

	if ( !element->children ) return;

	list_foreach( element->children, node )
	{
		mgui_element_process( cast_elem(node) );
	}
}

void mgui_element_initialize( MGuiElement* element )
{
	node_t* node;
	rectangle_t* r;

	if ( element == NULL ) return;

	// Intialize all resources with the renderer
	if ( element->flags & FLAG_CACHE_TEXTURE )
	{
		if ( element->cache == NULL )
		{
			r = element->callbacks->get_clip_region ?
				element->callbacks->get_clip_region( element, &r ), r :
				&element->bounds;

			element->cache = renderer->create_render_target( r->w, r->h );
		}
	}

	if ( !element->children ) return;

	// Do it for all the child elements as well
	list_foreach( element->children, node )
	{
		mgui_element_initialize( cast_elem(node) );
	}
}

void mgui_element_invalidate( MGuiElement* element )
{
	node_t* node;

	if ( element == NULL ) return;

	// Intialize all resources with the renderer
	if ( element->flags & FLAG_CACHE_TEXTURE )
	{
		if ( element->cache != NULL )
		{
			renderer->destroy_render_target( element->cache );
			element->cache = NULL;
		}
	}

	if ( !element->children ) return;

	// Do it for all the child elements as well
	list_foreach( element->children, node )
	{
		mgui_element_invalidate( cast_elem(node) );
	}
}

void mgui_element_resize_cache( MGuiElement* element )
{
	rectangle_t* r;

	if ( element == NULL ) return;
	if ( renderer == NULL ) return;
	if ( BIT_OFF( element->flags, FLAG_CACHE_TEXTURE ) ) return;

	r = element->callbacks->get_clip_region ?
		element->callbacks->get_clip_region( element, &r ), r :
		&element->bounds;

	if ( element->cache == NULL ||
		 element->cache->width < (uint32)r->w ||
		 element->cache->height < (uint32)r->h )
	{
		if ( element->cache != NULL )
			renderer->destroy_render_target( element->cache );

		element->cache = renderer->create_render_target( r->w, r->h );

		mgui_element_request_redraw( element );
	}
}

void mgui_element_request_redraw( MGuiElement* element )
{
	extern bool refresh_all;
	extern bool redraw_cache;

	refresh_all = true;

	// Invalidate this element and all its predecessors
	if ( element != NULL )
	{
		while ( element )
		{
			if ( element->cache != NULL )
				redraw_cache = true;

			element->flags_int |= INTFLAG_REFRESH;
			element = element->parent;
		}
	}
}

void mgui_element_request_redraw_all( void )
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
	if ( BIT_ON( child->flags, FLAG_3D_ENTITY ) ) return;
	if ( BIT_ON( child->flags, FLAG_DEPTH_TEST ) ) return;
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
		elem->callbacks->on_bounds_change( elem, true, false );

	mgui_element_request_redraw_all();

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
		elem->callbacks->on_bounds_change( elem, false, true );

	if ( elem->flags & FLAG_CACHE_TEXTURE )
		mgui_element_resize_cache( elem );

	mgui_element_request_redraw_all();

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
		elem->callbacks->on_bounds_change( elem, true, false );

	mgui_element_request_redraw_all();

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
		elem->callbacks->on_bounds_change( elem, false, true );

	if ( elem->flags & FLAG_CACHE_TEXTURE )
		mgui_element_resize_cache( elem );

	mgui_element_request_redraw_all();

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

float mgui_get_z_depth( MGuiElement* element )
{
	if ( element == NULL ) return 0.0f;

	return element->z_depth;
}

void mgui_set_z_depth( MGuiElement* element, float depth )
{
	if ( element == NULL ) return;
	if ( BIT_OFF( element->flags, FLAG_DEPTH_TEST ) ) return;

	element->z_depth = depth;
}

void mgui_get_3d_position( MGuiElement* element, vector3_t* pos )
{
	if ( element == NULL ||
		 element->transform == NULL ||
		 pos == NULL )
	{
		return;
	}

	*pos = element->transform->position;
}

void mgui_set_3d_position( MGuiElement* element, const vector3_t* pos )
{
	if ( element == NULL ||
		 element->transform == NULL ||
		 pos == NULL )
	{
		return;
	}

	element->transform->position = *pos;
	mgui_element_update_transform( element );
}

void mgui_get_3d_rotation( MGuiElement* element, vector3_t* rot )
{
	if ( element == NULL ||
		 element->transform == NULL ||
		 rot == NULL )
	{
		return;
	}

	*rot = element->transform->rotation;
}

void mgui_set_3d_rotation( MGuiElement* element, const vector3_t* rot )
{
	if ( element == NULL ||
		 element->transform == NULL ||
		 rot == NULL )
	{
		return;
	}

	element->transform->rotation = *rot;
	mgui_element_update_transform( element );
}

void mgui_get_3d_size( MGuiElement* element, vector2_t* size )
{
	if ( element == NULL ||
		 element->transform == NULL ||
		 size == NULL )
	{
		return;
	}

	*size = element->transform->size;
}

void mgui_set_3d_size( MGuiElement* element, const vector2_t* size )
{
	if ( element == NULL ||
		 element->transform == NULL ||
		 size == NULL )
	{
		return;
	}

	element->transform->size = *size;
	mgui_element_update_transform( element );
}

void mgui_set_3d_transform( MGuiElement* element, const vector3_t* pos, const vector3_t* rot, const vector2_t* size )
{
	if ( element == NULL ||
		 element->transform == NULL ||
		 pos == NULL || rot == NULL || size == NULL )
	{
		return;
	}

	element->transform->position = *pos;
	element->transform->rotation = *rot;
	element->transform->size = *size;

	mgui_element_update_transform( element );
}

void mgui_element_update_transform( MGuiElement* element )
{
	matrix4_t mat1, mat2, mat3, mat4;
	rectangle_t* bounds;
	float x, y, z;

	if ( element == NULL || element->transform == NULL ) return;

	if ( element->callbacks->get_clip_region )
		element->callbacks->get_clip_region( element, &bounds );
	else
		bounds = &element->bounds;

	// Scale
	x = element->transform->size.x / bounds->w;
	y = element->transform->size.y / bounds->h;

	matrix4_scale( &mat4, x, y, 1.0f );

	// Rotate
	if ( !vector3_is_zero( &element->transform->rotation ) )
	{
		matrix4_rotation_x( &mat1, element->transform->rotation.x );
		matrix4_multiply( &mat3, &mat4, &mat1 );

		matrix4_rotation_y( &mat2, element->transform->rotation.y );
		matrix4_multiply( &mat1, &mat3, &mat2 );

		matrix4_rotation_z( &mat3, element->transform->rotation.z );
		matrix4_multiply( &mat4, &mat1, &mat3 );
	}

	// Translate
	x = element->transform->position.x - x * (float)bounds->x;
	y = element->transform->position.y - y * (float)bounds->y;
	z = element->transform->position.z - element->z_depth;

	matrix4_translation( &mat1, x, y, z );
	matrix4_multiply( &element->transform->transform, &mat4, &mat1 );
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

	mgui_text_set_default_colour( elem->text );
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

	mgui_text_set_default_colour( elem->text );
}

uint8 mgui_get_alpha( MGuiElement* elem )
{
	if ( elem == NULL ) return 0;

	return elem->colour.a;
}

void mgui_set_alpha( MGuiElement* element, uint8 alpha )
{
	node_t* node;
	MGuiElement* child;

	if ( element == NULL ) return;

	element->colour.a = alpha;

	if ( element->text )
	{
		element->text->colour.a = alpha;
		mgui_text_set_default_colour( element->text );
	}

	if ( element->callbacks->on_colour_change )
		element->callbacks->on_colour_change( element );

	if ( element->children == NULL ) return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);

		if ( BIT_ON( child->flags, FLAG_INHERIT_ALPHA ) )
		{
			// If this child element is supposed to inherit its parent's alpha, apply it.
			mgui_set_alpha( child, alpha );
		}
	}
}

const char_t* mgui_get_text( MGuiElement* element )
{
	if ( element == NULL || element->text == NULL ) return NULL;

	return element->text->buffer;
}

char_t* mgui_get_text_buffer( MGuiElement* element, char_t* buf, size_t buflen )
{
	if ( buf == NULL ) return NULL;

	if ( element == NULL || element->text == NULL )
	{
		*buf = '\0';
		return buf;
	}

	mstrcpy( buf, element->text->buffer, buflen );
	return buf;
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

void mgui_get_text_size( MGuiElement* element, vectorscreen_t* size )
{
	if ( size == NULL ) return;

	if ( element == NULL || element->text == NULL )
	{
		size->w = 0;
		size->h = 0;
		return;
	}

	size->w = element->text->size.w;
	size->h = element->text->size.h;
}

void mgui_get_text_size_i( MGuiElement* element, uint16* w, uint16* h )
{
	if ( w == NULL || h == NULL ) return;

	if ( element == NULL || element->text == NULL )
	{
		w = h = 0;
		return;
	}

	*w = element->text->size.ux;
	*h = element->text->size.uy;
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

	element->text->alignment = (uint16)alignment;
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
		element->callbacks->on_bounds_change( element, false, true );
	
	mgui_element_request_redraw( element );

	if ( element->flags & FLAG_CACHE_TEXTURE )
		mgui_element_resize_cache( element );
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
	{
		element->text->font = element->font;
		mgui_text_update_dimensions( element->text );
	}

	if ( element->callbacks->on_text_change )
		element->callbacks->on_text_change( element );
}

void mgui_set_font_size( MGuiElement* element, uint8 size )
{
	if ( element == NULL ) return;
	if ( element->font == NULL ) return;

	element->font = mgui_font_set_size( element->font, size );

	if ( element->text && element->text->buffer )
	{
		element->text->font = element->font;
		mgui_text_update_dimensions( element->text );
	}

	if ( element->callbacks->on_text_change )
		element->callbacks->on_text_change( element );
}

void mgui_set_font_flags( MGuiElement* element, uint8 flags )
{
	if ( element == NULL ) return;
	if ( element->font == NULL ) return;

	element->font = mgui_font_set_flags( element->font, flags );

	if ( element->text )
	{
		if ( flags & FFLAG_BOLD ) element->text->flags |= TFLAG_BOLD;
		if ( flags & FFLAG_ITALIC ) element->text->flags |= TFLAG_ITALIC;

		element->text->font = element->font;
		mgui_text_update_dimensions( element->text );
	}

	if ( element->callbacks->on_text_change )
		element->callbacks->on_text_change( element );
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
		if ( flags & FFLAG_BOLD ) element->text->flags |= TFLAG_BOLD;
		if ( flags & FFLAG_ITALIC ) element->text->flags |= TFLAG_ITALIC;

		element->text->font = element->font;
		mgui_text_update_dimensions( element->text );
	}

	if ( element->callbacks->on_text_change )
		element->callbacks->on_text_change( element );
}

uint32 mgui_get_flags( MGuiElement* element )
{
	if ( element == NULL ) return 0;

	return element->flags;
}

void mgui_add_flags( MGuiElement* element, uint32 flags )
{
	uint32 old;
	rectangle_t* r;
	extern bool redraw_cache;

	if ( element == NULL ) return;

	if ( flags & FLAG_TEXT_TAGS )
	{
		// Format tags on editboxes is not supported for the time being.
		if ( element->type == GUI_EDITBOX ) flags &= ~FLAG_TEXT_TAGS;
		else if ( element->text ) element->text->flags |= TFLAG_TAGS;
	}

	if ( flags & FLAG_3D_ENTITY )
	{
		// A 3D element must be a root element.
		if ( element->parent )
			flags &= ~FLAG_3D_ENTITY;

		// Create transform matrix storage.
		else if ( element->transform == NULL )
			element->transform = mem_alloc_clean( sizeof(*element->transform) );
	}

	// A 3D element must be a root element.
	if ( flags & FLAG_DEPTH_TEST && element->parent )
		flags &= ~FLAG_DEPTH_TEST;

	// Enable shadow bit in text flags as well.
	if ( flags & FLAG_TEXT_SHADOW && element->text )
		element->text->flags |= TFLAG_SHADOW;

	// Enable caching into a texture.
	if ( flags & FLAG_CACHE_TEXTURE )
	{
		element->flags_int |= INTFLAG_REFRESH;

		if ( element->cache == NULL && renderer != NULL )
		{
			r = element->callbacks->get_clip_region ?
				element->callbacks->get_clip_region( element, &r ), r :
				&element->bounds;

			element->cache = renderer->create_render_target( r->w, r->h );
		}
	}

	old = element->flags;
	element->flags |= flags;

	if ( element->callbacks->on_flags_change )
		element->callbacks->on_flags_change( element, old );
}

void mgui_remove_flags( MGuiElement* element, uint32 flags )
{
	uint32 old;

	if ( element == NULL ) return;

	old = element->flags;
	element->flags &= ~flags;

	if ( element->text != NULL )
	{
		// Disable text tag bit.
		if ( flags & FLAG_TEXT_TAGS )
			element->text->flags &= ~TFLAG_TAGS;

		// Disable shadow bit.
		if ( flags & FLAG_TEXT_SHADOW )
			element->text->flags &= ~TFLAG_SHADOW;
	}

	// Remove transformation data.
	if ( flags & FLAG_3D_ENTITY )
		SAFE_DELETE( element->transform );

	// Set default z depth.
	if ( flags & FLAG_DEPTH_TEST )
		element->z_depth = 1.0f;

	// Destroy render cache
	if ( flags & FLAG_CACHE_TEXTURE )
	{
		if ( element->cache != NULL && renderer != NULL )
		{
			renderer->destroy_render_target( element->cache );
			element->cache = NULL;
		}
	}

	if ( element->callbacks->on_flags_change )
		element->callbacks->on_flags_change( element, old );
}

void mgui_set_event_handler( MGuiElement* element, mgui_event_handler_t handler, void* data )
{
	if ( element == NULL ) return;

	element->event_handler = handler;
	element->event_data = data;
}
