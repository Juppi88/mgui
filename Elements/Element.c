/**
 *
 * @file		Element.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		MGUI element functions.
 *
 * @details		Element function implementations. MGuiElement is the base struct for all GUI widgets.
 *
 **/

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
extern MGuiRenderer* renderer;
extern list_t* layers;

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

	if ( parent != NULL )
	{
		// Add this element to it's parents children
		mgui_add_child( parent, element );

		// Set alpha to parent's value, if the parent is an element
		element->colour.a = parent->colour.a;
	}
	else if ( BIT_OFF( element->flags_int, INTFLAG_NOPARENT ) )
	{
		// Add this element to the main layer list
		list_push( layers, cast_node(element) );
		element->flags_int |= INTFLAG_LAYER;
	}

	if ( element->text != NULL )
		element->text->colour.a = element->colour.a;
}

/**
 * @brief Destroy an element and its children.
 *
 * @details This function destroys and frees the memory used by an element
 * and its children.
 *
 * @param element The element to be destroyed
 */
void mgui_element_destroy( MGuiElement* element )
{
	node_t *node, *tmp;

	if ( element == NULL )
		return;

	mgui_remove_child( element );

	// Destroy child elements if any.
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
	extern uint32 params;
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

		// Draw the element from the cache.
		if ( draw_self || BIT_ON( params, MGUI_USE_DRAW_EVENT ) )
		{
			renderer->set_draw_colour( &cache_colour );
			renderer->draw_render_target( element->cache, r->x, r->y, r->w, r->h );
		}
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
	if ( element->flags & FLAG_CACHE_TEXTURE && element->cache == NULL )
	{
		r = element->callbacks->get_clip_region ?
			element->callbacks->get_clip_region( element, &r ), r :
			&element->bounds;

		element->cache = renderer->create_render_target( r->w, r->h );
	}

	if ( element->children == NULL ) return;

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

	// Invalidate all resources with the renderer
	if ( element->cache != NULL )
	{
		renderer->destroy_render_target( element->cache );
		element->cache = NULL;
	}

	if ( element->children == NULL ) return;

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

	// Check that the element is actually visible and active
	if ( BIT_OFF( element->flags, FLAG_VISIBLE ) ||
		 BIT_ON( element->flags, FLAG_DISABLED ) )
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

/**
 * @brief Adds a child element to another element.
 *
 * @details This function adds an element to another one as a child.
 * 3D elements can not be linked to other elements. If the parent is
 * NULL, the child element will become a layer.
 *
 * @param parent The parent element
 * @param child The element that is to be added as a child
 */
void mgui_add_child( MGuiElement* parent, MGuiElement* child )
{
	if ( child == NULL ) return;
	if ( child->parent != NULL ) return;
	if ( BIT_ON( child->flags, FLAG_3D_ENTITY ) ) return;
	if ( BIT_ON( child->flags, FLAG_DEPTH_TEST ) ) return;

	// If the child object is a layer, remove it from the layer list first.
	if ( BIT_ON( child->flags_int, INTFLAG_LAYER ) )
	{
		child->flags_int &= ~INTFLAG_LAYER;
		list_remove( layers, cast_node(child) );
	}

	if ( parent != NULL )
	{
		if ( parent->children == NULL )
			parent->children = list_create();

		list_push( parent->children, cast_node(child) );
		child->parent = parent;

		if ( child->type != GUI_NONE )
			mgui_element_update_child_pos( child );
	}
	else
	{
		list_push( layers, cast_node(child) );
		child->flags_int |= INTFLAG_LAYER;
	}

	mgui_element_request_redraw( parent );
}

/**
 * @brief Removes a child element from its parent.
 *
 * @details This function removes an element from its parent.
 * Note that when you remove an element from its parent, it will
 * no longer be rendered. The element itself will not be removed.
 *
 * @param child The element to be removed from its parent
 */
void mgui_remove_child( MGuiElement* child )
{
	if ( child == NULL )
		return;

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

/**
 * @brief Moves an element forward in the list of its siblings.
 *
 * @details This function moves an elements forward among its sibling
 * elements. This will make the element be drawn on top of the elements that
 * come after it.
 *
 * @param child The element to be moved forward
 */
void mgui_move_forward( MGuiElement* child )
{
	if ( child == NULL )
		return;

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

/**
 * @brief Moves an element backward in the list of its siblings.
 *
 * @details This function moves an elements backward among its sibling
 * elements. This will make the element be drawn behind the elements that
 * come before it.
 *
 * @param child The element to be moved backward
 */
void mgui_move_backward( MGuiElement* child )
{
	if ( child == NULL )
		return;

	if ( child->parent && child->parent->children )
	{
		list_move_forward( child->parent->children, cast_node(child) );
	}
	else if ( BIT_ON( child->flags_int, INTFLAG_LAYER ) )
	{
		list_move_forward( layers, cast_node(child) );
	}
}

/**
 * @brief Moves an element on top of its siblings.
 *
 * @details This function moves an elements on top of its sibling
 * elements. This will make the element be drawn on top of all of
 * its sibling elements
 *
 * @param child The element to be moved to the top
 */
void mgui_send_to_top( MGuiElement* child )
{
	if ( child == NULL )
		return;

	if ( child->parent && child->parent->children )
	{
		list_send_to_back( child->parent->children, cast_node(child) );
	}
	else if ( BIT_ON( child->flags_int, INTFLAG_LAYER ) )
	{
		list_send_to_back( layers, cast_node(child) );
	}
}

/**
 * @brief Moves an element behind its siblings.
 *
 * @details This function moves an elements behind its sibling
 * elements. This will make the element be drawn behind all of
 * its sibling elements
 *
 * @param child The element to be moved to the bottom
 */
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

/**
 * @brief Returns whether an element is a child of another element.
 *
 * @details This function returns whether two elements have a
 * parent-child relationship.
 *
 * @param parent The parent element
 * @param child The potential child element
 * @returns true if child and parent are related, false otherwise
 */
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

	if ( elem->parent != NULL )
	{
		r = &elem->parent->bounds;

		elem->bounds.x = r->x + (int16)( elem->pos.x * r->w );
		elem->bounds.y = r->y + (int16)( elem->pos.y * r->h );
		elem->offset.x = elem->bounds.x - r->x;
		elem->offset.y = elem->bounds.y - r->y;
	}
	else
	{
		elem->bounds.x = (int16)( elem->pos.x * draw_size.x );
		elem->bounds.y = (int16)( elem->pos.y * draw_size.y );
	}

	if ( elem->text != NULL )
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

	if ( elem->parent != NULL )
	{
		elem->bounds.w = (uint16)( elem->size.x * elem->parent->bounds.w );
		elem->bounds.h = (uint16)( elem->size.y * elem->parent->bounds.h );
		elem->offset.x = elem->bounds.x - elem->parent->bounds.x;
		elem->offset.y = elem->bounds.y - elem->parent->bounds.y;
	}
	else
	{
		elem->bounds.w = (uint16)( elem->size.x * draw_size.x );
		elem->bounds.h = (uint16)( elem->size.y * draw_size.y );
	}

	if ( elem->text != NULL )
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

	if ( elem->parent != NULL )
	{
		r = &elem->parent->bounds;

		elem->pos.x = (float)( elem->bounds.x - r->x ) / r->w;
		elem->pos.y = (float)( elem->bounds.y - r->y ) / r->h;
		elem->offset.x = elem->bounds.x - r->x;
		elem->offset.y = elem->bounds.y - r->y;
	}
	else
	{
		elem->pos.x = (float)elem->bounds.x / draw_size.x;
		elem->pos.y = (float)elem->bounds.y / draw_size.y;
	}

	if ( elem->text != NULL )
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

	if ( elem->parent != NULL )
	{
		elem->size.x = (float)elem->bounds.w / elem->parent->bounds.w;
		elem->size.y = (float)elem->bounds.h / elem->parent->bounds.h;
		elem->offset.x = elem->bounds.x - elem->parent->bounds.x;
		elem->offset.y = elem->bounds.y - elem->parent->bounds.y;
	}
	else
	{
		elem->size.x = (float)elem->bounds.w / draw_size.x;
		elem->size.y = (float)elem->bounds.h / draw_size.y;
	}

	if ( elem->text != NULL )
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

void mgui_element_update_child_pos( MGuiElement* elem )
{
	node_t* node;
	rectangle_t* r;
	bool size_changed = false;

	if ( elem == NULL || elem->parent == NULL ) return;

	r = &elem->parent->bounds;

	if ( elem->flags & FLAG_AUTO_RESIZE )
	{
		elem->bounds.x = r->x + (int16)( elem->pos.x * r->w );
		elem->bounds.y = r->y + (int16)( elem->pos.y * r->h );
		elem->bounds.w = (uint16)( elem->size.x * r->w );
		elem->bounds.h = (uint16)( elem->size.y * r->h );

		elem->offset.x = (int16)( elem->pos.x * r->w );
		elem->offset.y = (int16)( elem->pos.y * r->h );

		size_changed = true;
	}
	else
	{
		elem->bounds.x = r->x + elem->offset.x;
		elem->bounds.y = r->y + elem->offset.y;
	}

	if ( elem->callbacks->on_bounds_change )
		elem->callbacks->on_bounds_change( elem, true, size_changed );

	if ( elem->text != NULL )
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

/**
 * @brief Returns the relative position of an element.
 *
 * @details This function returns the position of an element relative to
 * its parent or the screen/window. A relative position is scaled between
 * 0 and 1. The origin is at the top left corner.
 *
 * @param element The element to retrieve the position of
 * @param pos A pointer to a vector2_t struct that will receive the position
 */
void mgui_get_pos( MGuiElement* element, vector2_t* pos )
{
	if ( element == NULL || pos == NULL )
		return;

	pos->x = element->pos.x;
	pos->y = element->pos.y;
}

/**
 * @brief Sets the relative position of an element.
 *
 * @details This function sets the position of an element relative to
 * its parent or the screen/window. A relative position is scaled between
 * 0 and 1. The origin is at the top left corner.
 *
 * @param element The element to set the position of
 * @param pos A pointer to a vector2_t struct containing the position
 */
void mgui_set_pos( MGuiElement* element, const vector2_t* pos )
{
	if ( element == NULL || pos == NULL )
		return;

	element->pos.x = pos->x;
	element->pos.y = pos->y;

	mgui_element_update_abs_pos( element );
}

/**
 * @brief Returns the relative size of an element.
 *
 * @details This function returns the size of an element relative to
 * its parent or the screen/window. A relative size is scaled
 * between 0 and 1.
 *
 * @param element The element to retrieve the size of
 * @param size A pointer to a vector2_t struct that will receive the size
 */
void mgui_get_size( MGuiElement* element, vector2_t* size )
{
	if ( element == NULL || size == NULL )
		return;

	size->x = element->size.x;
	size->y = element->size.y;
}

/**
 * @brief Sets the relative size of an element.
 *
 * @details This function sets the size of an element relative to
 * its parent or the screen/window. A relative size is scaled
 * between 0 and 1.
 *
 * @param element The element to set the size of
 * @param size A pointer to a vector2_t struct containing the new size
 */
void mgui_set_size( MGuiElement* element, const vector2_t* size )
{
	if ( element == NULL || size == NULL )
		return;

	element->size.x = size->x;
	element->size.y = size->y;

	mgui_element_update_abs_size( element );
}

/**
 * @brief Returns the absolute position of an element.
 *
 * @details This function returns the absolute position of an element
 * relative to its parent or the screen/window. An absolute position
 * is given in pixels. The origin is at the top left corner.
 *
 * @param element The element to retrieve the position of
 * @param pos A pointer to a vectorscreen_t struct that will receive the position
 */
void mgui_get_abs_pos( MGuiElement* element, vectorscreen_t* pos )
{
	if ( element == NULL || pos == NULL )
		return;

	if ( element->parent != NULL )
	{
		pos->x = element->bounds.x - element->parent->bounds.x;
		pos->y = element->bounds.y - element->parent->bounds.y;
	}
	else
	{
		pos->x = element->bounds.x;
		pos->y = element->bounds.y;
	}
}

/**
 * @brief Sets the absolute position of an element.
 *
 * @details This function sets the absolute position of an element
 * relative to its parent or the screen/window. An absolute position
 * is given in pixels. The origin is at the top left corner.
 *
 * @param element The element to set the position of
 * @param pos A pointer to a vectorscreen_t struct containing the position
 */
void mgui_set_abs_pos( MGuiElement* element, const vectorscreen_t* pos )
{
	if ( element == NULL || pos == NULL )
		return;

	if ( element->parent != NULL )
	{
		element->bounds.x = element->parent->bounds.x + pos->x;
		element->bounds.y = element->parent->bounds.y + pos->y;
	}
	else
	{
		element->bounds.x = pos->x;
		element->bounds.y = pos->y;
	}

	mgui_element_update_rel_pos( element );
}

/**
 * @brief Returns the absolute size of an element.
 *
 * @details This function returns the absolute size of an element.
 * An absolute size is given in pixels.
 *
 * @param element The element to retrieve the size of
 * @param size A pointer to a vectorscreen_t struct that will receive the size
 */
void mgui_get_abs_size( MGuiElement* element, vectorscreen_t* size )
{
	if ( element == NULL || size == NULL )
		return;

	size->x = element->bounds.w;
	size->y = element->bounds.h;
}

/**
 * @brief Sets the absolute size of an element.
 *
 * @details This function returns the absolute size of an element.
 * An absolute size is given in pixels.
 *
 * @param element The element to retrieve the size of
 * @param size A pointer to a vectorscreen_t struct
 */
void mgui_set_abs_size( MGuiElement* element, const vectorscreen_t* size )
{
	if ( element == NULL || size == NULL )
		return;

	element->bounds.w = size->x;
	element->bounds.h = size->y;

	mgui_element_update_rel_size( element );
}

/**
 * @brief Returns the relative position of an element.
 *
 * @details This function returns the position of an element relative to
 * its parent or the screen/window. A relative position is scaled between
 * 0 and 1. The origin is at the top left corner.
 *
 * @param element The element to retrieve the position of
 * @param x A pointer to a float that will receive the x coordinate
 * @param y A pointer to a float that will receive the y coordinate
 */
void mgui_get_pos_f( MGuiElement* element, float* x, float* y )
{
	if ( element == NULL || x == NULL || y == NULL )
		return;

	*x = element->pos.x;
	*y = element->pos.y;
}

/**
 * @brief Sets the relative position of an element.
 *
 * @details This function sets the position of an element relative to
 * its parent or the screen/window. A relative position is scaled between
 * 0 and 1. The origin is at the top left corner.
 *
 * @param element The element to set the position of
 * @param x The new x coordinate of the element
 * @param y The new y coordinate of the element
 */
void mgui_set_pos_f( MGuiElement* element, float x, float y )
{
	if ( element == NULL )
		return;

	element->pos.x = x;
	element->pos.y = y;

	mgui_element_update_abs_pos( element );
}

/**
 * @brief Returns the relative size of an element.
 *
 * @details This function returns the size of an element relative to
 * its parent or the screen/window. A relative size is scaled
 * between 0 and 1.
 *
 * @param element The element to retrieve the size of
 * @param w A pointer to a float that will receive the width of the element
 * @param h A pointer to a float that will receive the height of the element
 */
void mgui_get_size_f( MGuiElement* element, float* w, float* h )
{
	if ( element == NULL || w == NULL || h == NULL )
		return;

	*w = element->size.x;
	*h = element->size.y;
}

/**
 * @brief Sets the relative size of an element.
 *
 * @details This function sets the size of an element relative to
 * its parent or the screen/window. A relative size is scaled
 * between 0 and 1.
 *
 * @param element The element to set the size of
 * @param w The new width for the element
 * @param h The new height for the element
 */
void mgui_set_size_f( MGuiElement* element, float w, float h )
{
	if ( element == NULL )
		return;

	element->size.x = w;
	element->size.y = h;

	mgui_element_update_abs_size( element );
}

/**
 * @brief Returns the absolute position of an element.
 *
 * @details This function returns the absolute position of an element
 * relative to its parent or the screen/window. An absolute position
 * is given in pixels. The origin is at the top left corner.
 *
 * @param element The element to retrieve the position of
 * @param x A pointer to an int16 that will receive the x coordinate
 * @param y A pointer to an int16 that will receive the y coordinate
 */
void mgui_get_abs_pos_i( MGuiElement* element, int16* x, int16* y )
{
	if ( element == NULL || x == NULL || y == NULL )
		return;

	if ( element->parent != NULL )
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

/**
 * @brief Sets the absolute position of an element.
 *
 * @details This function sets the absolute position of an element
 * relative to its parent or the screen/window. An absolute position
 * is given in pixels. The origin is at the top left corner.
 *
 * @param element The element to set the position of
 * @param x The new x coordinate for the element
 * @param y The new y coordinate for the element
 */
void mgui_set_abs_pos_i( MGuiElement* element, int16 x, int16 y )
{
	if ( element == NULL )
		return;

	if ( element->parent != NULL )
	{
		element->bounds.x = element->parent->bounds.x + x;
		element->bounds.y = element->parent->bounds.y + y;
	}
	else
	{
		element->bounds.x = x;
		element->bounds.y = y;
	}

	mgui_element_update_rel_pos( element );
}

/**
 * @brief Returns the absolute size of an element.
 *
 * @details This function returns the absolute size of an element.
 * An absolute size is given in pixels.
 *
 * @param element The element to retrieve the size of
 * @param w A pointer to an uint16 that will receive the width
 * @param h A pointer to an uint16 that will receive the height
 */
void mgui_get_abs_size_i( MGuiElement* element, uint16* w, uint16* h )
{
	if ( element == NULL || w == NULL || h == NULL )
		return;

	*w = element->bounds.w;
	*h = element->bounds.h;
}

/**
 * @brief Sets the absolute size of an element.
 *
 * @details This function returns the absolute size of an element.
 * An absolute size is given in pixels.
 *
 * @param element The element to retrieve the size of
 * @param w The new width for the element
 * @param h The new height for the element
 */
void mgui_set_abs_size_i( MGuiElement* element, uint16 w, uint16 h )
{
	if ( element == NULL )
		return;

	element->bounds.w = w;
	element->bounds.h = h;

	mgui_element_update_rel_size( element );
}

/**
 * @brief Returns the draw depth of an element.
 *
 * @details This function returns the current draw depth of an
 * element. Draw depth controls the depth of the element on the
 * screen in a 3D environment. The draw depth is usually scaled
 * between 0 and 1, 1 being on the top. This value is only valid
 * if @ref FLAG_DEPTH_TEST has been enabled (see @ref MGUI_FLAGS,
 * @ref mgui_add_flags).
 *
 * @param element The element to get the draw depth of
 * @returns The current draw depth
 */
float mgui_get_z_depth( MGuiElement* element )
{
	if ( element == NULL )
		return 0.0f;

	return element->z_depth;
}

/**
 * @brief Sets the draw depth of an element.
 *
 * @details This function changes the current draw depth of an
 * element. Draw depth controls the depth of the element on the
 * screen in a 3D environment. The draw depth is usually scaled
 * between 0 and 1, 1 being on the top. This value is only valid
 * if @ref FLAG_DEPTH_TEST has been enabled (see @ref MGUI_FLAGS,
 * @ref mgui_add_flags).
 *
 * @param element The element to set the draw depth of
 * @param depth The new z-depth
 */
void mgui_set_z_depth( MGuiElement* element, float depth )
{
	if ( element == NULL )
		return;

	if ( BIT_OFF( element->flags, FLAG_DEPTH_TEST ) )
		return;

	element->z_depth = depth;
}

/**
 * @brief Returns the position of an element in 3D space.
 *
 * @details This function returns the position of an element
 * in 3D space. The value is only valid if @ref FLAG_3D_ENTITY
 * has been set (see @ref MGUI_FLAGS, @ref mgui_add_flags).
 *
 * @param element The element to retrieve the position of
 * @param pos A pointer to a vector3_t struct that will receive the position
 */
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

/**
 * @brief Sets the position of an element in 3D space.
 *
 * @details This function changes the position of an element
 * in 3D space. The value is only valid if @ref FLAG_3D_ENTITY
 * has been set (see @ref MGUI_FLAGS, @ref mgui_add_flags).
 *
 * @param element The element to set the position of
 * @param pos A pointer to a vector3_t struct that contains the new position
 */
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

/**
 * @brief Returns the rotation of an element in 3D space.
 *
 * @details This function returns the orientation of an element
 * in 3D space. The angles returned are in radians. This value is
 * only valid if @ref FLAG_3D_ENTITY has been set (see @ref MGUI_FLAGS,
 * @ref mgui_add_flags).
 *
 * @param element The element to retrieve the rotation of
 * @param rot A pointer to a vector3_t struct that will receive the rotation
 */
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

/**
 * @brief Sets the rotation of an element in 3D space.
 *
 * @details This function changes the orientation of an element
 * in 3D space. The angles used are in radians. This value is only
 * valid if @ref FLAG_3D_ENTITY has been set (see @ref MGUI_FLAGS,
 * @ref mgui_add_flags).
 *
 * @param element The element to set the rotation of
 * @param rot A pointer to a vector3_t struct that will receive the rotation
 */
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

/**
 * @brief Returns the size of an element in 3D space.
 *
 * @details This function returns the size of an element in 3D space.
 * This function uses 3D world coordinates instead of pixels. The value
 * is only valid if @ref FLAG_3D_ENTITY has been set (see @ref MGUI_FLAGS,
 * @ref mgui_add_flags).
 *
 * @param element The element to retrieve the size of
 * @param size A pointer to a vector2_t struct that will receive the size (width, height)
 */
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

/**
 * @brief Sets the size of an element in 3D space.
 *
 * @details This function changes the size of an element in 3D space.
 * This function uses 3D world coordinates instead of pixels. The value
 * is only valid if @ref FLAG_3D_ENTITY has been set (see @ref MGUI_FLAGS,
 * @ref mgui_add_flags).
 *
 * @param element The element to set the size of
 * @param size A pointer to a vector2_t struct that contains the size (width, height)
 */
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

/**
 * @brief Sets the transform of an element in 3D space.
 *
 * @details This function changes the transform (position, rotation, size)
 * of an element in 3D space. This function uses 3D world coordinates instead
 * of pixels. The values are only valid if @ref FLAG_3D_ENTITY has been set
 * (see @ref MGUI_FLAGS, @ref mgui_add_flags).
 *
 * @param element The element to set the transform of
 * @param pos A pointer to a vector3_t struct that contains the new position
 * @param rot A pointer to a vector3_t struct that contains the new rotation
 * @param size A pointer to a vector2_t struct that contains the new size (width, height)
 */
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

	// First, scale the size.
	x = element->transform->size.x / bounds->w;
	y = element->transform->size.y / bounds->h;

	matrix4_scale( &mat4, x, y, 1.0f );

	// Rotate the element.
	if ( !vector3_is_zero( &element->transform->rotation ) )
	{
		matrix4_rotation_x( &mat1, element->transform->rotation.x );
		matrix4_multiply( &mat3, &mat4, &mat1 );

		matrix4_rotation_y( &mat2, element->transform->rotation.y );
		matrix4_multiply( &mat1, &mat3, &mat2 );

		matrix4_rotation_z( &mat3, element->transform->rotation.z );
		matrix4_multiply( &mat4, &mat1, &mat3 );
	}

	// Translate to the new position.
	x = element->transform->position.x - x * (float)bounds->x;
	y = element->transform->position.y - y * (float)bounds->y;
	z = element->transform->position.z - element->z_depth;

	matrix4_translation( &mat1, x, y, z );
	matrix4_multiply( &element->transform->transform, &mat4, &mat1 );
}

/**
 * @brief Returns the colour of an element.
 *
 * @details This function returns the main colour of an element. The main
 * colour is usually the background of the element.
 *
 * @param element The element to retrieve the colour of
 * @param col A pointer to a colour_t struct that will receive the colour
 */
void mgui_get_colour( MGuiElement* element, colour_t* col )
{
	if ( element == NULL || col == NULL )
		return;

	*col = element->colour;
}

/**
 * @brief Sets the colour of an element.
 *
 * @details This function changes the main colour of an element. The main
 * colour is usually the background of the element.
 *
 * @param element The element to set the colour of
 * @param col A pointer to a colour_t struct that contains the new colour
 */
void mgui_set_colour( MGuiElement* element, const colour_t* col )
{
	if ( element == NULL || col == NULL )
		return;

	element->colour = *col;
	mgui_set_alpha( element, col->a );
}

/**
 * @brief Returns the text colour of an element.
 *
 * @details This function returns the colour of the text within an element.
 *
 * @param element The element to retrieve the colour of
 * @param col A pointer to a colour_t struct that will receive the colour
 */
void mgui_get_text_colour( MGuiElement* element, colour_t* col )
{
	if ( element == NULL || col == NULL || element->text == NULL )
		return;

	*col = element->text->colour;
}

/**
 * @brief Sets the text colour of an element.
 *
 * @details This function changes the colour of the text within an element.
 *
 * @param element The element to set the colour of
 * @param col A pointer to a colour_t struct that contains the new colour
 */
void mgui_set_text_colour( MGuiElement* element, const colour_t* col )
{
	if ( element == NULL || col == NULL || element->text == NULL )
		return;

	element->text->colour = *col;
	element->text->colour.a = element->colour.a;

	mgui_text_set_default_colour( element->text );
}

/**
 * @brief Returns the colour of an element as a 32bit integer.
 *
 * @details This function returns the main colour of an element. The main
 * colour is usually the background of the element. The colour is returned
 * as a 32bit hex integer in format 0xRRGGBBAA.
 *
 * @param element The element to retrieve the colour of
 * @returns The colour as a 32bit integer
 */
uint32 mgui_get_colour_i( MGuiElement* element )
{
	if ( element == NULL )
		return 0;

	return element->colour.hex;
}

/**
 * @brief Sets the colour of an element.
 *
 * @details This function changes the main colour of an element. The main
 * colour is usually the background of the element. The colour is passed
 * as a 32bit hex integer in 0xRRGGBBAA format.
 *
 * @param element The element to set the colour of
 * @param col The new colour as a 32bit integer
 */
void mgui_set_colour_i( MGuiElement* element, uint32 col )
{
	if ( element == NULL )
		return;

	element->colour.hex = col;
	mgui_set_alpha( element, element->colour.a );
}

/**
 * @brief Returns the colour of the text within an element.
 *
 * @details This function returns the colour of the text within an element.
 * The colour is returned as a 32bit hex integer in format 0xRRGGBBAA.
 *
 * @param element The element to retrieve the colour of
 * @returns The colour as a 32bit integer
 */
uint32 mgui_get_text_colour_i( MGuiElement* element )
{
	if ( element == NULL || element->text == NULL )
		return 0;

	return element->text->colour.hex;
}

/**
 * @brief Sets the text colour of an element.
 *
 * @details This function changes the colour of the text within an element.
 * The new colour is passed as a 32bit hex integer in 0xRRGGBBAA format.
 *
 * @param element The element to set the colour of
 * @param col The new colour as a 32bit integer
 */
void mgui_set_text_colour_i( MGuiElement* element, uint32 col )
{
	if ( element == NULL || element->text == NULL )
		return;

	element->text->colour.hex = col;
	element->text->colour.a = element->colour.a;

	mgui_text_set_default_colour( element->text );
}

/**
 * @brief Returns the alpha value of an element.
 *
 * @details This function returns the alpha value (opacity) of an element.
 *
 * @param element The element to retrieve the alpha value of
 * @returns The alpha value as an 8bit integer
 */
uint8 mgui_get_alpha( MGuiElement* element )
{
	if ( element == NULL )
		return 0;

	return element->colour.a;
}

/**
 * @brief Sets the alpha value of an element.
 *
 * @details This function changes the alpha value (opacity) of an element.
 *
 * @param element The element to change the alpha of
 * @param alpha The new alpha value as an 8bit integer
 */
void mgui_set_alpha( MGuiElement* element, uint8 alpha )
{
	node_t* node;
	MGuiElement* child;

	if ( element == NULL )
		return;

	element->colour.a = alpha;

	if ( element->text != NULL )
	{
		element->text->colour.a = alpha;
		mgui_text_set_default_colour( element->text );
	}

	if ( element->callbacks->on_colour_change )
		element->callbacks->on_colour_change( element );

	if ( element->children == NULL )
		return;

	list_foreach( element->children, node )
	{
		child = cast_elem(node);

		// If this child element is supposed to inherit its parent's alpha, apply it.
		if ( BIT_ON( child->flags, FLAG_INHERIT_ALPHA ) )
			mgui_set_alpha( child, alpha );
	}
}

/**
 * @brief Returns the text of the element.
 *
 * @details This function returns a pointer to the text buffer of an element.
 *
 * @param element The element to retrieve the text of
 * @returns A pointer to the element's text buffer
 */
const char_t* mgui_get_text( MGuiElement* element )
{
	if ( element == NULL || element->text == NULL )
		return NULL;

	return element->text->buffer;
}

/**
 * @brief Copies the text of an element into a buffer.
 *
 * @details This function copies the contents of an element's text
 * buffer into another text buffer.
 *
 * @param element The element to retrieve the text of
 * @param buf A pointer to the text buffer that will receive the text
 * @param buflen The size of the buffer in characters
 * @returns A pointer to the buffer that was passed to the function
 */
char_t* mgui_get_text_buffer( MGuiElement* element, char_t* buf, size_t buflen )
{
	if ( buf == NULL )
		return NULL;

	if ( element == NULL || element->text == NULL )
	{
		*buf = '\0';
		return buf;
	}

	mstrcpy( buf, element->text->buffer, buflen );
	return buf;
}

/**
 * @brief Returns the length of the text within an element's buffer.
 *
 * @details This function returns the length of an element's text buffer
 * in characters.
 *
 * @param element The element to retrieve the text length of
 * @returns The length of the text buffer
 */
uint32 mgui_get_text_len( MGuiElement* element )
{
	if ( element->text == NULL )
		return 0;

	return element->text->len;
}

/**
 * @brief Sets the text of an element.
 *
 * @details This function changes the text of an element. The input
 * can be formatted using standard printf formatting.
 *
 * @param element The element to set the text of
 * @param fmt A standard C printf format string
 * @param ... Any additional arguments
 */
void mgui_set_text( MGuiElement* element, const char_t* fmt, ... )
{
	va_list	marker;

	if ( element == NULL || fmt == NULL )
		return;

	if ( element->text == NULL )
		return;

	va_start( marker, fmt );
	mgui_text_set_buffer_va( element->text, fmt, marker );
	va_end( marker );

	if ( element->callbacks->on_text_change )
		element->callbacks->on_text_change( element );
}

/**
 * @brief Sets the text of an element safely.
 *
 * @details This function changes the text of an element.
 *
 * @param element The element to set the text of
 * @param text The new text value
 */
void mgui_set_text_s( MGuiElement* element, const char_t* text )
{
	if ( element == NULL || text == NULL )
		return;

	if ( element->text == NULL )
		return;

	mgui_text_set_buffer_s( element->text, text );

	if ( element->callbacks->on_text_change )
		element->callbacks->on_text_change( element );
}

/**
 * @brief Returns the size of the text within an element.
 *
 * @details This function returns the absolute size of the text
 * within an element in pixels (when applicable).
 *
 * @param element The element to retrieve the text size of
 * @param size A pointer to a vectorscreen_t struct that will receive the size of the text
 */
void mgui_get_text_size( MGuiElement* element, vectorscreen_t* size )
{
	if ( size == NULL )
		return;

	if ( element == NULL || element->text == NULL )
	{
		size->w = 0;
		size->h = 0;
		return;
	}

	size->w = element->text->size.w;
	size->h = element->text->size.h;
}

/**
 * @brief Returns the size of the text within an element.
 *
 * @details This function returns the absolute size of the text
 * within an element in pixels (when applicable).
 *
 * @param element The element to retrieve the text size of
 * @param w A pointer to a uint16 that will receive the width of the text
 * @param h A pointer to a uint16 that will receive the height of the text
 */
void mgui_get_text_size_i( MGuiElement* element, uint16* w, uint16* h )
{
	if ( w == NULL || h == NULL )
		return;

	if ( element == NULL || element->text == NULL )
	{
		w = h = 0;
		return;
	}

	*w = element->text->size.ux;
	*h = element->text->size.uy;
}

/**
 * @brief Returns the text alignment within an element.
 *
 * @details This function returns the type of text alignment within
 * an element. This value is a bitfield.
 *
 * @param element The element to retrieve the alignment of
 * @returns Current alignment as a bitfield
 * @sa MGUI_ALIGNMENT
 */
uint32 mgui_get_alignment( MGuiElement* element )
{
	if ( element == NULL )
		return ALIGN_CENTER;

	if ( element->text == NULL )
		return ALIGN_CENTER;

	return element->text->alignment;
}

/**
 * @brief Sets the text alignment within an element.
 *
 * @details This function changes the type of text alignment within
 * an element. This value is a bitfield.
 *
 * @param element The element to set the alignment of
 * @param alignment New alignment as a bitfield
 * @sa MGUI_ALIGNMENT
 */
void mgui_set_alignment( MGuiElement* element, uint32 alignment )
{
	if ( element == NULL )
		return;

	if ( element->text == NULL )
		return;

	element->text->alignment = (uint16)alignment;
	mgui_text_update_position( element->text );
}

/**
 * @brief Returns the amount of text padding.
 *
 * @details This function returns the amount of padding used around text
 * within an element, in pixels.
 *
 * @param element The element to retrieve text padding of
 * @param top A pointer to an 8bit integer that will receive the amount of top padding
 * @param bottom A pointer to an 8bit integer that will receive the amount of bottom padding
 * @param left A pointer to an 8bit integer that will receive the amount of left padding
 * @param right A pointer to an 8bit integer that will receive the amount of right padding
 */
void mgui_get_text_padding( MGuiElement* element, uint8* top, uint8* bottom, uint8* left, uint8* right )
{
	if ( element == NULL )
		return;

	if ( element->text == NULL )
		return;

	*top = element->text->pad.top;
	*bottom = element->text->pad.bottom;
	*left = element->text->pad.left;
	*right = element->text->pad.right;
}

/**
 * @brief Sets the amount of text padding.
 *
 * @details This function changes the amount of padding used around text
 * within an element, in pixels.
 *
 * @param element The element to set text padding of
 * @param top The amount of top padding in pixels as an 8bit integer
 * @param bottom The amount of bottom padding in pixels as an 8bit integer
 * @param left The amount of left padding in pixels as an 8bit integer
 * @param right The amount of right padding in pixels as an 8bit integer
 */
void mgui_set_text_padding( MGuiElement* element, uint8 top, uint8 bottom, uint8 left, uint8 right )
{
	if ( element == NULL )
		return;

	if ( element->text == NULL )
		return;

	element->text->pad.top = top;
	element->text->pad.bottom = bottom;
	element->text->pad.left = left;
	element->text->pad.right = right;

	if ( element->callbacks->on_bounds_change )
		element->callbacks->on_bounds_change( element, false, true );

	if ( element->callbacks->on_text_change )
		element->callbacks->on_text_change( element );
	
	mgui_element_request_redraw( element );

	if ( element->flags & FLAG_CACHE_TEXTURE )
		mgui_element_resize_cache( element );
}

/**
 * @brief Returns the name of the font for an element.
 *
 * @details This function returns the name of the font used within
 * an element.
 *
 * @param element The element to retrieve the font name of
 * @returns A pointer to the name of the font, or NULL if this element does not have text
 */
const char_t* mgui_get_font_name( MGuiElement* element )
{
	if ( element == NULL || element->font == NULL )
		return NULL;

	return element->font->name;
}

/**
 * @brief Returns the size of the font within an element.
 *
 * @details This function returns the size of the font used within
 * an element.
 *
 * @param element The element to retrieve the font size of
 * @returns Size of the font as an 8bit integer, or 0 if this element does not have text
 */
uint8 mgui_get_font_size( MGuiElement* element )
{
	if ( element == NULL || element->font == NULL )
		return 0;

	return element->font->size;
}

/**
 * @brief Returns the font style flags of an element.
 *
 * @details This function returns currently enabled font style flags for
 * the given element. The return value is a bitfield composed of @ref
 * MGUI_FONT_FLAGS values.
 *
 * @param element The element to retrieve the font flags of
 * @returns Currently enabled font flags as an 8bit integer bitfield, or 0 if this element does not have text
 * @sa MGUI_FONT_FLAGS
 */
uint8 mgui_get_font_flags( MGuiElement* element )
{
	if ( element == NULL || element->font == NULL )
		return FFLAG_NONE;

	return element->font->flags;
}

/**
 * @brief Sets the font of an element.
 *
 * @details This function changes the font used by an element.
 *
 * @param element The element to set the font of
 * @param font The name of the font to be used
 */
void mgui_set_font_name( MGuiElement* element, const char_t* font )
{
	if ( element == NULL || font == NULL )
		return;

	if ( element->font == NULL )
		return;

	element->font = mgui_font_set_font( element->font, font );

	if ( element->text && element->text->buffer )
	{
		element->text->font = element->font;
		mgui_text_update_dimensions( element->text );
	}

	if ( element->callbacks->on_text_change )
		element->callbacks->on_text_change( element );
}

/**
 * @brief Sets the size of the font for an element.
 *
 * @details This function changes the size of the font used by an element.
 *
 * @param element The element to set the font size of
 * @param size The new size for the font
 */
void mgui_set_font_size( MGuiElement* element, uint8 size )
{
	if ( element == NULL || element->font == NULL )
		return;

	element->font = mgui_font_set_size( element->font, size );

	if ( element->text && element->text->buffer )
	{
		element->text->font = element->font;
		mgui_text_update_dimensions( element->text );
	}

	if ( element->callbacks->on_text_change )
		element->callbacks->on_text_change( element );
}

/**
 * @brief Sets the style flags for the font used by an element.
 *
 * @details This function changes the style flags of the font
 * used by an element. The value passed to the function is a
 * bitfield composed of @ref MGUI_FONT_FLAGS values.
 *
 * @param element The element to set the font flags of
 * @param flags The new style flags for the font
 * @sa MGUI_FONT_FLAGS
 */
void mgui_set_font_flags( MGuiElement* element, uint8 flags )
{
	if ( element == NULL || element->font == NULL )
		return;

	element->font = mgui_font_set_flags( element->font, flags );

	if ( element->text != NULL )
	{
		if ( flags & FFLAG_BOLD )
			element->text->flags |= TFLAG_BOLD;

		if ( flags & FFLAG_ITALIC )
			element->text->flags |= TFLAG_ITALIC;

		element->text->font = element->font;
		mgui_text_update_dimensions( element->text );
	}

	if ( element->callbacks->on_text_change )
		element->callbacks->on_text_change( element );
}

/**
 * @brief Changes all the font properies of an element at once.
 *
 * @details This function changes all font properties (font name, size, flags)
 * for the font used by an element.
 *
 * @param element The element to change the font properties of
 * @param font The name of the font to be loaded
 * @param size The size of the font to be loaded
 * @param flags The new style flags for the font. A bitfield composed of @ref MGUI_FONT_FLAGS values
 * @param charset Requested charset from @ref MGUI_FONT_CHARSET, can be 0
 *
 * @sa MGUI_FONT_FLAGS
 * @sa MGUI_FONT_CHARSET
 */
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

/**
 * @brief Returns the property flags of an element.
 *
 * @details This function returns the property flags of an element.
 * The return value is a bitfield composed of @ref MGUI_FLAGS values.
 *
 * @param element The element to retrieve property flags of
 * @returns The properties of the element as a 32bit integer
 * @sa MGUI_FLAGS
 */
uint32 mgui_get_flags( MGuiElement* element )
{
	if ( element == NULL )
		return 0;

	return element->flags;
}

/**
 * @brief Enables the given property flags for an element.
 *
 * @details This function will enable the given property flags for an element.
 * The properties are passed as a 32bit integer bitfield composed of @ref MGUI_FLAGS values.
 *
 * @param element The element to set the property flags of
 * @param flags The properties that will be enabled for this element
 * @sa MGUI_FLAGS
 */
void mgui_add_flags( MGuiElement* element, uint32 flags )
{
	uint32 old;
	rectangle_t* r;
	extern bool redraw_cache;

	if ( element == NULL )
		return;

	if ( flags & FLAG_TEXT_TAGS )
	{
		// Format tags on editboxes is not supported for the time being.
		if ( element->type == GUI_EDITBOX )
			flags &= ~FLAG_TEXT_TAGS;

		else if ( element->text != NULL )
			element->text->flags |= TFLAG_TAGS;
	}

	if ( flags & FLAG_3D_ENTITY )
	{
		// A 3D element must be a root element.
		if ( element->parent != NULL )
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

/**
 * @brief Disables the given property flags for an element.
 *
 * @details This function will disable the given property flags for an element.
 * The properties are passed as a 32bit integer bitfield composed of @ref MGUI_FLAGS values.
 *
 * @param element The element to set the property flags of
 * @param flags The properties that will be disabled for this element
 * @sa MGUI_FLAGS
 */
void mgui_remove_flags( MGuiElement* element, uint32 flags )
{
	uint32 old;

	if ( element == NULL )
		return;

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

	// Destroy render cache.
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

/**
 * @brief Registers an event handler for an element.
 *
 * @details This function registers a per-element event handler that will be
 * called when the element triggers certain events. The event handler is of type
 * @ref mgui_event_handler_t, and the event specific data is passed to it in a
 * structure. The function also accepts user data that can be used to identify
 * the element in question.
 *
 * @param element The element to register an event handler for
 * @param handler An event handler function
 * @param data User data that can be bound to the element
 *
 * @sa MGuiEvent
 * @sa mgui_event_handler_t
 */
void mgui_set_event_handler( MGuiElement* element, mgui_event_handler_t handler, void* data )
{
	if ( element == NULL )
		return;

	element->event_handler = handler;
	element->event_data = data;
}
