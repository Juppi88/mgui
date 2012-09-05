/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Control.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		'Control' is a blank element which can be used as a
 *				parent (canvas/layer) for real-world elements. All
 *				GUI elements also inherit from a control, it tracks the
 *				relationships to/from an element.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "Control.h"
#include "Element.h"
#include "Window.h"
#include "Platform/Platform.h"
#include <assert.h>

static list_t* controls = NULL;

control_t* mgui_create_control( void )
{
	control_t* control;
	extern vectorscreen_t screen_size;

	if ( !controls )
		controls = list_create();

	control = (control_t*)mem_alloc( sizeof(*control) );
	control->children = list_create();
	control->parent = NULL;
	control->flags = FLAG_VISIBLE;
	control->bounds.x = 0;
	control->bounds.y = 0;
	control->bounds.w = screen_size.x;
	control->bounds.h = screen_size.y;

	list_push( controls, (node_t*)control );

	return control;
}

void mgui_destroy_control( control_t* control )
{
	node_t* node;
	node_t* tmp;
	element_t* element;

	assert( control != NULL );
	assert( controls != NULL );

	// Iterate through all elements on this control and destroy them and their children *evil laugh*
	list_foreach_safe( control->children, node, tmp ) 
	{
		element = cast_elem(node);
		mgui_destroy_element( element );
	}

	list_remove( controls, cast_node(control) );
	list_destroy( control->children );

	mem_free( control );
}

void mgui_add_child( control_t* parent, element_t* child )
{
	assert( parent != NULL );
	assert( child != NULL );

	if ( child->parent ) return;

	list_push( parent->children, cast_node(child) );
	child->parent = parent;
}

void mgui_remove_child( element_t* child )
{
	assert( child != NULL );

	if ( !child->parent ) return;
	if ( !child->parent->children ) return;

	list_remove( child->parent->children, cast_node(child) );
	child->parent = NULL;
}

void mgui_move_forward( element_t* child )
{
	assert( child != NULL );

	if ( !child->parent ) return;
	if ( !child->parent->children ) return;

	list_move_forward( child->parent->children, cast_node(child) );
}

void mgui_move_backward( element_t* child )
{
	assert( child != NULL );

	if ( !child->parent ) return;
	if ( !child->parent->children ) return;

	list_move_backward( child->parent->children, cast_node(child) );
}

void mgui_send_to_top( element_t* child )
{
	assert( child != NULL );

	if ( !child->parent ) return;
	if ( !child->parent->children ) return;

	list_send_to_front( child->parent->children, cast_node(child) );
}

void mgui_send_to_bottom( element_t* child )
{
	assert( child != NULL );

	if ( !child->parent ) return;
	if ( !child->parent->children ) return;

	list_send_to_back( child->parent->children, cast_node(child) );
}

bool mgui_is_child_of( control_t* parent, element_t* child )
{
	assert( parent != NULL );
	assert( child != NULL );

	return ( parent == child->parent );
}

void mgui_do_cleanup( void )
{
	node_t* cnode;
	node_t* enode;
	node_t *tmp1, *tmp2;
	control_t* control;
	element_t* element;

	assert( controls != NULL );

	list_foreach_safe( controls, cnode, tmp1 ) 
	{
		control = cast_ctrl(cnode);

		list_foreach_safe( control->children, enode, tmp2 )
		{
			element = cast_elem(enode);
			mgui_destroy_element( element );
		}

		list_remove( controls, cnode );
		list_destroy( control->children );
		mem_free( control );
	}

	list_destroy( controls );
	controls = NULL;
}

void mgui_render_controls( void )
{
	node_t*	cnode;
	node_t*	enode;
	control_t* control;
	element_t* element;

	assert( controls != NULL );

	// Render all elements on every visible parent controls
	list_foreach( controls, cnode )
	{
		control = cast_ctrl(cnode);
		if ( BIT_OFF(control->flags, FLAG_VISIBLE) ) continue;

		list_foreach( control->children, enode )
		{
			element = cast_elem(enode);
			mgui_element_render( element );
		}
	}
}

void mgui_process_controls( void )
{
	node_t*	cnode;
	node_t*	enode;
	control_t* control;
	element_t* element;
	uint32 ticks;

	ticks = timer_get_ticks();

	assert( controls != NULL );

	list_foreach( controls, cnode )
	{
		control = cast_ctrl(cnode);
		if ( BIT_OFF(control->flags, FLAG_VISIBLE) ) continue;

		list_foreach( control->children, enode )
		{
			element = cast_elem(enode);
			mgui_element_process( element, ticks );
		}
	}
}

static __inline element_t* __mgui_get_element_at_test_self( element_t* element, uint16 x, uint16 y )
{
	struct window_s* window;
	element_t* ret = NULL;

	if ( BIT_ON( element->flags, FLAG_INACTIVE ) ) return NULL;

	switch ( element->type )
	{
	case GUI_WINDOW:
		{
			window = (struct window_s*)element;

			if ( !point_in_rect( &window->window_bounds, x, y ) )
				return NULL;

			if ( window->closebtn )
				if ( ( ret = mgui_get_element_at( cast_ctrl(window->closebtn), x, y ) ) != NULL )
					goto endcheck;

			if ( window->titlebar )
				if ( ( ret = mgui_get_element_at( cast_ctrl(window->titlebar), x, y ) ) != NULL )
					goto endcheck;

			break;
		}

	default:
		if ( !point_in_rect( &element->bounds, x, y ) )
			return NULL;
	}

endcheck:
	ret = ret ? ret : element;

	if ( BIT_OFF( ret->flags, FLAG_MOUSECTRL ) )
		return NULL;

	return ret;
}

element_t* mgui_get_element_at( control_t* parent, uint16 x, uint16 y )
{
	node_t* node;
	element_t* element;
	element_t* ret = NULL;

	assert( controls != NULL );

	if ( !parent )
	{
		if ( list_empty( controls ) ) return NULL;
		parent = cast_ctrl(controls->begin);
	}

	// If this element is invisible, dont bother going any further
	if ( BIT_OFF( parent->flags, FLAG_VISIBLE ) ) return NULL;

	// If our control is an element test it and its sub-elements first
	if ( BIT_ON( parent->flags, FLAG_ELEMENT ) && 
		( ret = __mgui_get_element_at_test_self( cast_elem(parent), x, y ) ) != NULL )
		return ret;

	// Test all the children as well
	if ( parent->children )
	{
		list_foreach( parent->children, node ) 
		{
			element = cast_elem(node);
			if ( ( ret = mgui_get_element_at( cast_ctrl(element), x, y ) ) != NULL )
				return ret;
		}
	}

	return ret;
}
