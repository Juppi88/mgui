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
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "Control.h"
#include "Element.h"
#include "Window.h"
#include "Platform/Alloc.h"

static list_t* controls = NULL;

MGuiControl* mgui_create_control( void )
{
	MGuiControl* control;
	extern vectorscreen_t draw_size;

	if ( !controls )
		controls = list_create();

	control = (MGuiControl*)mem_alloc( sizeof(*control) );
	control->children = list_create();
	control->parent = NULL;
	control->flags = FLAG_VISIBLE;
	control->bounds.x = 0;
	control->bounds.y = 0;
	control->bounds.w = draw_size.x;
	control->bounds.h = draw_size.y;

	list_push( controls, (node_t*)control );

	return control;
}

void mgui_destroy_control( MGuiControl* control )
{
	node_t* node;
	node_t* tmp;
	MGuiElement* element;

	if ( control == NULL || controls == NULL ) return;

	// Iterate through all elements on this control and destroy them and their children *evil laugh*
	list_foreach_safe( control->children, node, tmp ) 
	{
		element = cast_elem(node);
		mgui_element_destroy( element );
	}

	list_remove( controls, cast_node(control) );
	list_destroy( control->children );

	mem_free( control );
}

void mgui_do_cleanup( void )
{
	node_t* cnode;
	node_t* enode;
	node_t *tmp1, *tmp2;
	MGuiControl* control;
	MGuiElement* element;

	if ( controls == NULL ) return;

	list_foreach_safe( controls, cnode, tmp1 ) 
	{
		control = cast_ctrl(cnode);

		list_foreach_safe( control->children, enode, tmp2 )
		{
			element = cast_elem(enode);
			mgui_element_destroy( element );
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
	MGuiControl* control;
	MGuiElement* element;

	if ( controls == NULL ) return;

	// Render all elements on every visible parent control
	list_foreach( controls, cnode )
	{
		control = cast_ctrl(cnode);
		if ( BIT_OFF( control->flags, FLAG_VISIBLE ) ) continue;

		list_foreach( control->children, enode )
		{
			element = cast_elem(enode);
			mgui_element_render( element );
		}
	}
}

void mgui_process_controls( uint32 ticks )
{
	node_t*	cnode;
	node_t*	enode;
	MGuiControl* control;
	MGuiElement* element;

	if ( controls == NULL ) return;

	list_foreach( controls, cnode )
	{
		control = cast_ctrl(cnode);
		if ( BIT_OFF( control->flags, FLAG_VISIBLE ) ) continue;

		list_foreach( control->children, enode )
		{
			element = cast_elem(enode);
			mgui_element_process( element, ticks );
		}
	}
}

void mgui_add_child( MGuiControl* parent, MGuiElement* child )
{
	if ( parent == NULL || child == NULL ) return;

	if ( child->parent ) return;

	list_push( parent->children, cast_node(child) );
	child->parent = parent;
}

void mgui_remove_child( MGuiElement* child )
{
	if ( child == NULL ) return;

	if ( !child->parent ) return;
	if ( !child->parent->children ) return;

	list_remove( child->parent->children, cast_node(child) );
	child->parent = NULL;
}

void mgui_move_forward( MGuiElement* child )
{
	if ( child == NULL ) return;

	if ( !child->parent ) return;
	if ( !child->parent->children ) return;

	// Yes, this is a bit backwards (no pun intended), but the list is rendered from front to back.
	list_move_backward( child->parent->children, cast_node(child) );
}

void mgui_move_backward( MGuiElement* child )
{
	if ( child == NULL ) return;

	if ( !child->parent ) return;
	if ( !child->parent->children ) return;

	list_move_forward( child->parent->children, cast_node(child) );
}

void mgui_send_to_top( MGuiElement* child )
{
	if ( child == NULL ) return;

	if ( !child->parent ) return;
	if ( !child->parent->children ) return;

	list_send_to_back( child->parent->children, cast_node(child) );
}

void mgui_send_to_bottom( MGuiElement* child )
{
	if ( child == NULL ) return;

	if ( !child->parent ) return;
	if ( !child->parent->children ) return;

	list_send_to_front( child->parent->children, cast_node(child) );
}

bool mgui_is_child_of( MGuiControl* parent, MGuiElement* child )
{
	if ( parent == NULL || child == NULL ) return false;

	return ( parent == child->parent );
}

MGuiElement* mgui_get_element_at( MGuiControl* parent, uint16 x, uint16 y )
{
	node_t* node;
	MGuiElement* element;
	MGuiElement* ret = NULL;

	if ( controls == NULL ) return NULL;

	if ( !parent )
	{
		if ( list_empty( controls ) ) return NULL;
		parent = cast_ctrl(controls->begin);
	}

	// If this element is invisible, dont bother going any further
	if ( BIT_OFF( parent->flags, FLAG_VISIBLE ) ) return NULL;

	// If our control is an element test it and its sub-elements first
	if ( BIT_ON( parent->flags, FLAG_ELEMENT ) && 
	   ( ret = mgui_get_element_at_test_self( cast_elem(parent), x, y ) ) != NULL )
		return ret;

	// Test all the children as well
	if ( parent->children )
	{
		list_foreach_r( parent->children, node ) 
		{
			element = cast_elem(node);
			if ( ( ret = mgui_get_element_at( cast_ctrl(element), x, y ) ) != NULL )
				return ret;
		}
	}

	return ret;
}

__inline MGuiElement* mgui_get_element_at_test_self( MGuiElement* element, uint16 x, uint16 y )
{
	struct MGuiWindow* window;
	MGuiElement* ret = NULL;

	if ( BIT_ON( element->flags, FLAG_INACTIVE ) ) return NULL;

	switch ( element->type )
	{
	case GUI_WINDOW:
		{
			window = (struct MGuiWindow*)element;

			if ( !rect_is_point_in( &window->window_bounds, x, y ) )
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
		if ( !rect_is_point_in( &element->bounds, x, y ) )
			return NULL;
	}

endcheck:
	ret = ret ? ret : element;

	if ( BIT_OFF( ret->flags, FLAG_MOUSECTRL ) )
		return NULL;

	return ret;
}
