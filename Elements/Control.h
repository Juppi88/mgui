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

#pragma once
#ifndef __MYLLY_GUI_CONTROL_H
#define __MYLLY_GUI_CONTROL_H

#include "MGUI.h"

struct control_s
{
	node_t;							// Next and previous controls (linked list node)
	uint32				flags;		// Element flags
	rectangle_t			bounds;		// Absolute rectangle for this control (in pixels)
	control_t*			parent;		// Parent control
	list_t*				children;	// List of children elements
};

void		mgui_do_cleanup			( void );
void		mgui_render_controls	( void );
void		mgui_process_controls	( uint32 ticks );
element_t*	mgui_get_element_at		( control_t* parent, uint16 x, uint16 y );

#endif /* __MYLLY_GUI_CONTROL_H */
