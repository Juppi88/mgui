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

#pragma once
#ifndef __MYLLY_GUI_CONTROL_H
#define __MYLLY_GUI_CONTROL_H

#include "MGUI.h"
#include "Math/MathDefs.h"
#include "Types/List.h"

struct MGuiControl
{
	node_t;							// Next and previous controls (linked list node)
	uint32				flags;		// Element flags
	rectangle_t			bounds;		// Absolute rectangle for this control (in pixels)
	MGuiControl*		parent;		// Parent control
	list_t*				children;	// List of children elements
};

MGuiControl*	mgui_create_control		( void );
void			mgui_destroy_control	( MGuiControl* control );

void			mgui_do_cleanup			( void );
void			mgui_render_controls	( void );
void			mgui_process_controls	( uint32 ticks );

void			mgui_add_child			( MGuiControl* parent, MGuiElement* child );
void			mgui_remove_child		( MGuiElement* child );
void			mgui_move_forward		( MGuiElement* child );
void			mgui_move_backward		( MGuiElement* child );
void			mgui_send_to_top		( MGuiElement* child );
void			mgui_send_to_bottom		( MGuiElement* child );
bool			mgui_is_child_of		( MGuiControl* parent, MGuiElement* child );

MGuiElement*	mgui_get_element_at				( MGuiControl* parent, uint16 x, uint16 y );
MGuiElement*	mgui_get_element_at_test_self	( MGuiElement* element, uint16 x, uint16 y );

#endif /* __MYLLY_GUI_CONTROL_H */
