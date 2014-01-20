/**
 *
 * @file		Listbox.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI listbox related functions.
 *
 * @details		Functions and structures related to GUI listboxes.
 *
 **/

#pragma once
#ifndef __MGUI_LISTBOX_H
#define __MGUI_LISTBOX_H

#include "Element.h"
#include "Scrollbar.h"

/**
 * @brief GUI listbox.
 * @details Listbox is an element that allows the user to select one or more items from a list.
 */
struct MGuiListbox {
	MGuiElement;							///< Inherit MGuiElement members
	list_t*					items;			///< List of items on this listbox
	MGuiListboxItem*		first_visible;	///< First visible item that will be rendered
	uint32					max_visible;	///< Maximum number of visible items this list can display at once
	MGuiListboxItem*		first_selected;	///< First selected item
	uint32					selected;		///< Total number of selected items
	colour_t				select_colour;	///< Background colour used for selected items
	mgui_listbox_sort_t		sort;			///< Item comparison function used for automatic sorting
	struct MGuiScrollbar*	scrollbar;		///< The scrollbar element that is shown if the list gets too big
	int16					scroll_offset;	///< Position of the scrollbar if it is visible
	uint16					height;			///< Total height of all the items in pixels
};

/**
 * @brief Lstbox item.
 * @details This is a container for a single item in an MGuiListbox.
 */
struct MGuiListboxItem {
	node_t			node;		///< Linked list node
	MGuiListbox*	parent;		///< Listbox this item belongs to
	char_t*			text;		///< Pointer to the text buffer
	MGuiFormatTag*	tags;		///< An array of parsed format tags
	uint32			ntags;		///< Number of format tags
	rectangle_t		bounds;		///< Absolute bounding rectangle (in pixels)
	rectangle_t		text_bounds;///< Bounding rectangle for text (in pixels)
	vectorscreen_t	pos;		///< Item position relative to the position of the listbox (in pixels)
	void*			data;		///< Pointer to user assigned data
	bool			selected;	///< Has this item been selected by the user
};

MGuiListbox*	mgui_create_listbox					( MGuiElement* parent );
MGuiListbox*	mgui_create_listbox_ex				( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, uint32 select_colour );

MGuiListboxItem* mgui_listbox_add_item				( MGuiListbox* listbox, const char_t* text );
void			mgui_listbox_remove_item			( MGuiListbox* listbox, MGuiListboxItem* item );
void			mgui_listbox_clean					( MGuiListbox* listbox );
void			mgui_listbox_set_sort_function		( MGuiListbox* listbox, mgui_listbox_sort_t func );
uint32			mgui_listbox_get_item_count			( MGuiListbox* listbox );
uint32			mgui_listbox_get_selected_count		( MGuiListbox* listbox );

MGuiListboxItem* mgui_listbox_get_first_item		( MGuiListbox* listbox );
MGuiListboxItem* mgui_listbox_get_next_item			( MGuiListboxItem* item );
MGuiListboxItem* mgui_listbox_get_selected_item		( MGuiListbox* listbox );
MGuiListboxItem* mgui_listbox_get_next_selected_item( MGuiListboxItem* item );

const char_t*	mgui_listbox_get_item_text			( MGuiListboxItem* item );
void			mgui_listbox_set_item_text			( MGuiListboxItem* item, const char_t* text );
void*			mgui_listbox_get_item_data			( MGuiListboxItem* item );
void			mgui_listbox_set_item_data			( MGuiListboxItem* item, void* data );

void			mgui_listbox_get_selected_colour	( MGuiListbox* listbox, colour_t* col );
void			mgui_listbox_set_selected_colour	( MGuiListbox* listbox, const colour_t* col );
uint32			mgui_listbox_get_selected_colour_i	( MGuiListbox* listbox );
void			mgui_listbox_set_selected_colour_i	( MGuiListbox* listbox, uint32 hex );

#endif /* __MGUI_LISTBOX_H */
