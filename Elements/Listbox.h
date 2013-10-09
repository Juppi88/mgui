/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Listbox.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI listbox related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_LISTBOX_H
#define __MGUI_LISTBOX_H

#include "Element.h"
#include "Scrollbar.h"

struct MGuiListbox {
	MGuiElement;							// Inherit all generic element properties
	list_t*					items;			// List of items on this listbox
	MGuiListboxItem*		first_visible;	// First visible item that is rendered
	uint32					max_visible;	// Maximum number of visible items at once
	MGuiListboxItem*		first_selected;	// First selected item
	uint32					selected;		// Number of selected items
	colour_t				select_colour;	// Background colour for selected items
	mgui_listbox_sort_t		sort;			// Sort function, used by item sorting
	struct MGuiScrollbar*	scrollbar;		// Scrollbar that is shown if the list gets too big
	int16					scroll_offset;	// Position of the scrollbar if it is visible
	uint16					height;			// Total height of all the items in pixels
};

struct MGuiListboxItem {
	node_t			node;		// Linked list node
	MGuiListbox*	parent;		// Parent element
	char_t*			text;		// Text buffer
	MGuiFormatTag*	tags;		// An array of parsed format tags
	uint32			ntags;		// Number of format tags
	rectangle_t		bounds;		// Absolute boundaries (in pixels)
	rectangle_t		text_bounds;// Text boundaries (in pixels)
	vectorscreen_t	pos;		// Item position inside the listbox (in pixels)
	void*			data;		// Pointer to user data
	bool			selected;	// Has the user selected this item?
};

MGuiListbox*		mgui_create_listbox					( MGuiElement* parent );
MGuiListbox*		mgui_create_listbox_ex				( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, uint32 select_colour );

MGuiListboxItem*	mgui_listbox_add_item				( MGuiListbox* listbox, const char_t* text );
void				mgui_listbox_remove_item			( MGuiListbox* listbox, MGuiListboxItem* item );
void				mgui_listbox_clean					( MGuiListbox* listbox );
void				mgui_listbox_set_sort_function		( MGuiListbox* listbox, mgui_listbox_sort_t func );
uint32				mgui_listbox_get_item_count			( MGuiListbox* listbox );
uint32				mgui_listbox_get_selected_count		( MGuiListbox* listbox );

MGuiListboxItem*	mgui_listbox_get_first_item			( MGuiListbox* listbox );
MGuiListboxItem*	mgui_listbox_get_next_item			( MGuiListboxItem* item );
MGuiListboxItem*	mgui_listbox_get_selected_item		( MGuiListbox* listbox );
MGuiListboxItem*	mgui_listbox_get_next_selected_item	( MGuiListboxItem* item );

const char_t*		mgui_listbox_get_item_text			( MGuiListboxItem* item );
void				mgui_listbox_set_item_text			( MGuiListboxItem* item, const char_t* text );
void*				mgui_listbox_get_item_data			( MGuiListboxItem* item );
void				mgui_listbox_set_item_data			( MGuiListboxItem* item, void* data );

void				mgui_listbox_get_selected_colour	( MGuiListbox* listbox, colour_t* col );
void				mgui_listbox_set_selected_colour	( MGuiListbox* listbox, const colour_t* col );
uint32				mgui_listbox_get_selected_colour_i	( MGuiListbox* listbox );
void				mgui_listbox_set_selected_colour_i	( MGuiListbox* listbox, uint32 hex );

#endif /* __MGUI_LISTBOX_H */
