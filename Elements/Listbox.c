/**
 *
 * @file		Listbox.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI listbox related functions.
 *
 * @details		Functions and structures related to GUI listboxes.
 *
 **/

#include "Listbox.h"
#include "Skin.h"
#include "Platform/Alloc.h"
#include "Stringy/Stringy.h"

// --------------------------------------------------

// Listbox callback handlers
static void				mgui_listbox_destroy			( MGuiElement* listbox );
static void				mgui_listbox_render				( MGuiElement* listbox );
static void				mgui_listbox_on_bounds_change	( MGuiElement* listbox, bool pos, bool size );
static void				mgui_listbox_on_flags_change	( MGuiElement* listbox, uint32 old );
static void				mgui_listbox_on_colour_change	( MGuiElement* listbox );
static void				mgui_listbox_on_text_change		( MGuiElement* listbox );
static void				mgui_listbox_on_mouse_click		( MGuiElement* listbox, int16 x, int16 y, MOUSEBTN mousebtn );

static void				mgui_listbox_on_scroll			( const MGuiEvent* event );
static void				mgui_listbox_push_item			( struct MGuiListbox* listbox, MGuiListboxItem* item );
static void				mgui_listbox_update_positions	( struct MGuiListbox* listbox, MGuiListboxItem* begin );
static void				mgui_listbox_update_scrollbar	( struct MGuiListbox* listbox );
static void				mgui_listbox_needs_scrollbar	( struct MGuiListbox* listbox );
static MGuiListboxItem*	mgui_listbox_get_first_visible	( struct MGuiListbox* listbox );
static MGuiListboxItem*	mgui_listbox_get_item_at		( struct MGuiListbox* listbox, int16 x, int16 y );
static void				mgui_listbox_remove_selected	( struct MGuiListbox* listbox );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	mgui_listbox_destroy,
	mgui_listbox_render,
	NULL, /* post_render */
	NULL, /* process */
	NULL, /* get_clip_region */
	mgui_listbox_on_bounds_change,
	mgui_listbox_on_flags_change,
	mgui_listbox_on_colour_change,
	mgui_listbox_on_text_change,
	NULL, /* on_mouse_enter */
	NULL, /* on_mouse_leave */
	mgui_listbox_on_mouse_click,
	NULL, /* on_mouse_release */
	NULL, /* on_mouse_drag */
	NULL, /* on_mouse_move */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

// --------------------------------------------------

/**
 * @brief Creates a listbox.
 *
 * @details This function creates a GUI listbox. If the parent element
 * is NULL, the listbox will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @returns A pointer to the created listbox
 */
MGuiListbox* mgui_create_listbox( MGuiElement* parent )
{
	struct MGuiListbox* listbox;
	MGuiScrollbar* scrollbar;
	extern MGuiFont* default_font;

	listbox = mem_alloc_clean( sizeof(*listbox) );
	mgui_element_create( cast_elem(listbox), parent );

	listbox->type = GUI_LISTBOX;
	listbox->flags |= (FLAG_BACKGROUND|FLAG_BORDER|FLAG_MOUSECTRL|FLAG_KBCTRL|FLAG_CLIP|FLAG_SCROLLABLE);

	listbox->font = default_font;
	listbox->text->font = default_font;
	listbox->text->pad.bottom = 5;
	listbox->text->pad.top = 5;
	listbox->text->pad.left = 5;
	listbox->text->pad.right = 5;

	// Create the list for items.
	listbox->items = list_create();

	// Create a scrollbar and make it invisible for now
	scrollbar = mgui_create_scrollbar( cast_elem(listbox) );

	mgui_remove_flags( scrollbar, FLAG_VISIBLE );
	mgui_set_event_handler( scrollbar, mgui_listbox_on_scroll, listbox );
	mgui_scrollbar_set_step_size( scrollbar, (float)default_font->size + listbox->text->pad.top + listbox->text->pad.bottom );

	listbox->scrollbar = (struct MGuiScrollbar*)scrollbar;

	// Listbox callbacks
	listbox->callbacks = &callbacks;

	return cast_elem(listbox);
}

/**
 * @brief Creates a listbox (extended).
 *
 * @details This function creates a GUI listbox with the given parameters.
 * If the parent element is NULL, the listbox will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @param x The absolute x coordinate relative to the parent
 * @param y The absolute y coordinate relative to the parent
 * @param w The absolute width of the listbox
 * @param h The absolute height of the listbox
 * @param flags Any additional flags that will be applied as a bitmask (see @ref MGUI_FLAGS)
 * @param col The background colour of the listbox as a 32bit hex integer
 * @param select_col The background colour of a selected item as a 32bit hex integer
 *
 * @returns A pointer to the created listbox
 */
MGuiListbox* mgui_create_listbox_ex( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, uint32 select_col )
{
	MGuiListbox* listbox;

	listbox = mgui_create_listbox( parent );

	mgui_set_abs_pos_i( listbox, x, y );
	mgui_set_abs_size_i( listbox, w, h );
	mgui_add_flags( listbox, flags );
	mgui_set_colour_i( listbox, col );
	mgui_listbox_set_selected_colour_i( listbox, select_col );

	return listbox;
}

static void mgui_listbox_destroy( MGuiElement* listbox )
{
	struct MGuiListbox* list = (struct MGuiListbox*)listbox;

	// NOTE: We don't have to delete the scrollbar here because mgui_element_destroy will handle it!
	list->scrollbar = NULL;

	mgui_listbox_clean( listbox );
	list_destroy( list->items );
}

static void mgui_listbox_render( MGuiElement* listbox )
{
	listbox->skin->draw_listbox( listbox );
}

static void mgui_listbox_on_bounds_change( MGuiElement* listbox, bool pos, bool size )
{
	struct MGuiListbox* list = (struct MGuiListbox*)listbox;
	MGuiScrollbar* scrollbar = cast_elem(list->scrollbar);

	UNREFERENCED_PARAM( pos );

	if ( size )
	{
		// Re-calculate the maximum number of visible items.
		list->max_visible = list->bounds.h / ( list->text->pad.top + list->text->pad.bottom + list->font->size );

		// Update scrollbar boundaries.
		mgui_set_abs_pos_i( scrollbar, list->bounds.w - 16, 0 );
		mgui_set_abs_size_i( scrollbar, 16, list->bounds.h );
	}

	mgui_listbox_update_positions( list, (MGuiListboxItem*)list_begin( list->items ) );
}

static void mgui_listbox_on_flags_change( MGuiElement* listbox, uint32 old )
{
	UNREFERENCED_PARAM( old );
	mgui_listbox_needs_scrollbar( (struct MGuiListbox*)listbox );
}

static void mgui_listbox_on_colour_change( MGuiElement* listbox )
{
	struct MGuiListbox* list = (struct MGuiListbox*)listbox;

	// Update scrollbar colour.
	if ( list->scrollbar != NULL )
		mgui_set_colour( cast_elem(list->scrollbar), &list->colour );
}

static void mgui_listbox_on_text_change( MGuiElement* listbox )
{
	struct MGuiListbox* list = (struct MGuiListbox*)listbox;

	// Update scrollbar step.
	mgui_scrollbar_set_step_size( cast_elem(list->scrollbar), (float)list->font->size + list->text->pad.top + list->text->pad.bottom );
}

static void mgui_listbox_on_mouse_click( MGuiElement* listbox, int16 x, int16 y, MOUSEBTN mousebtn )
{
	struct MGuiListbox* list = (struct MGuiListbox*)listbox;
	MGuiListboxItem* item;
	MGuiEvent event;

	if ( mousebtn != MOUSE_LBUTTON ) return;

	mgui_element_request_redraw( listbox );

	item = mgui_listbox_get_item_at( list, x, y );
	if ( item == NULL )
	{
		mgui_listbox_remove_selected( list );
		return;
	}

	if ( BIT_OFF( list->flags, FLAG_LISTBOX_MULTISELECT ) ||
		 !input_get_key_state( MKEY_CONTROL ) )
	{
		mgui_listbox_remove_selected( list );
	}

	item->selected = true;
	list->selected++;

	if ( listbox->event_handler )
	{
		event.type = EVENT_LISTBOX_SELECT;
		event.list.element = cast_elem(listbox);
		event.list.data = listbox->event_data;
		event.list.item = item;

		listbox->event_handler( &event );
	}
}

static void mgui_listbox_on_scroll( const MGuiEvent* event )
{
	struct MGuiListbox* listbox;
	struct MGuiScrollbar* scrollbar;
	float percentage;
	MGuiEvent list_event;

	if ( event->type != EVENT_SCROLL ) return;

	listbox = (struct MGuiListbox*)event->scroll.data;
	scrollbar = (struct MGuiScrollbar*)event->scroll.element;

	percentage = scrollbar->bar_position / scrollbar->content_size;

	// If we can fit all the items into the box, we don't have to offset anything.
	if ( listbox->height < listbox->bounds.h )
		listbox->scroll_offset = 0;

	// If the scrollbar is at the bottom, make sure the last item is displayed properly. (This is ugly, but will have to do for now.)
	else if ( percentage == 1 )
		listbox->scroll_offset = listbox->height - listbox->bounds.h + listbox->font->size + listbox->text->pad.top + listbox->text->pad.bottom;

	// Otherwise calculate the item offset.
	else listbox->scroll_offset = (int16)( ( listbox->height - listbox->bounds.h ) * percentage );

	// Update item positions and find out the first visible item.
	mgui_listbox_update_positions( listbox, (MGuiListboxItem*)list_begin( listbox->items ) );

	listbox->first_visible = mgui_listbox_get_first_visible( listbox );

	// Call the listbox's own scroll event handler here.
	if ( listbox->event_handler )
	{
		list_event.type = EVENT_SCROLL;
		list_event.scroll.element = cast_elem(listbox);
		list_event.scroll.data = listbox->event_data;
		list_event.scroll.position = 0;
		list_event.scroll.change = 0;

		listbox->event_handler( &list_event );
	}
}

static void mgui_listbox_push_item( struct MGuiListbox* listbox, MGuiListboxItem* item )
{
	// Add the item to the list.
	list_push( listbox->items, &item->node );

	// If we have a sort function, let's sort the items.
	// TODO: Actually do this

	// Calculate the position of the new item, and update the other items.
	mgui_listbox_update_positions( listbox, item );

	listbox->first_visible = mgui_listbox_get_first_visible( listbox ); 
}

static void mgui_listbox_update_positions( struct MGuiListbox* listbox, MGuiListboxItem* begin )
{
	node_t *node, *prev;
	MGuiListboxItem* item;
	int16 y = 0, item_height, scroll_width = 0;

	node = &begin->node;
	prev = node->prev;

	if ( listbox->scrollbar->flags & FLAG_VISIBLE )
		scroll_width = listbox->scrollbar->bounds.w;

	if ( prev != list_end( listbox->items ) )
	{
		item = (MGuiListboxItem*)prev;
		y = item->pos.y + item->bounds.h;
	}

	item_height = listbox->font->size + listbox->text->pad.top + listbox->text->pad.bottom;

	y -= listbox->scroll_offset;

	// Since we can't trust the renderer to do smooth text clipping, we'll make sure that
	// the first visible item will always be at the top.
	y += listbox->scroll_offset % item_height;

	for ( ; node != list_end( listbox->items ); node = node->next )
	{
		item = (MGuiListboxItem*)node;

		// Update position within the listbox.
		item->pos.x = 0;
		item->pos.y = y;

		// Update absolute boundaries.
		item->bounds.x = listbox->bounds.x;
		item->bounds.y = listbox->bounds.y + y;
		item->bounds.w = listbox->bounds.w - scroll_width;
		item->bounds.h = item_height;

		// Update absolute text position.
		item->text_bounds.x = item->bounds.x + listbox->text->pad.left;
		item->text_bounds.y = item->bounds.y + listbox->text->pad.top;

		y = item->pos.y + item->bounds.h;
	}
}

static void mgui_listbox_update_scrollbar( struct MGuiListbox* listbox )
{
	float content = (float)( listbox->height - listbox->bounds.h );
	MGuiScrollbar* scrollbar = cast_elem( listbox->scrollbar );

	mgui_scrollbar_set_content_size( scrollbar, content );
	mgui_scrollbar_set_bar_size( scrollbar, 0.9f - content / listbox->height );
	mgui_listbox_needs_scrollbar( listbox );
}

static void mgui_listbox_needs_scrollbar( struct MGuiListbox* listbox )
{
	if ( listbox->height > listbox->bounds.h &&
		 BIT_OFF( listbox->scrollbar->flags, FLAG_VISIBLE ) &&
		 BIT_ON( listbox->flags, FLAG_SCROLLABLE ) )
	{
		// Make the scrollbar visible and make the item area smaller.
		mgui_add_flags( cast_elem(listbox->scrollbar), FLAG_VISIBLE );
		mgui_listbox_update_positions( listbox, (MGuiListboxItem*)list_begin( listbox->items ) );
	}

	else if ( ( listbox->height <= listbox->bounds.h || BIT_OFF( listbox->flags, FLAG_SCROLLABLE ) ) &&
			    BIT_ON( listbox->scrollbar->flags, FLAG_VISIBLE ) )
	{
		// Make the scrollbar invisible and make the item area bigger.
		mgui_remove_flags( cast_elem(listbox->scrollbar), FLAG_VISIBLE );
		mgui_listbox_update_positions( listbox, (MGuiListboxItem*)list_begin( listbox->items ) );

		listbox->scroll_offset = 0;
	}
}

static MGuiListboxItem* mgui_listbox_get_first_visible( struct MGuiListbox* listbox )
{
	node_t* node;
	MGuiListboxItem* item;

	if ( list_empty( listbox->items ) ) return NULL;

	list_foreach( listbox->items, node )
	{
		item = (MGuiListboxItem*)node;
		if ( item->bounds.y >= listbox->bounds.y )
			return item;
	}

	return NULL;
}

static MGuiListboxItem* mgui_listbox_get_item_at( struct MGuiListbox* listbox, int16 x, int16 y )
{
	MGuiListboxItem* item;
	node_t* node;

	if ( list_empty( listbox->items ) ||
		 listbox->first_visible == NULL )
		 return NULL;

	item = listbox->first_visible;

	for ( node = &item->node; node != list_end( listbox->items ); node = node->next )
	{
		item = (MGuiListboxItem*)node;
		if ( rect_is_point_in( &item->bounds, x, y ) )
		{
			return item;
		}
	}

	return NULL;
}

static void mgui_listbox_remove_selected( struct MGuiListbox* listbox )
{
	node_t* node;
	MGuiListboxItem* item;

	list_foreach( listbox->items, node )
	{
		item = (MGuiListboxItem*)node;
		item->selected = false;
	}

	listbox->selected = 0;
}

/**
 * @brief Adds a new item to a listbox.
 *
 * @details This function adds a new item to the listbox and returns a pointer
 * to the created item.
 *
 * @param listbox The listbox to add an item to
 * @param text Text that will go on the item
 * @returns Pointer to an MGuiListboxItem container
 */
MGuiListboxItem* mgui_listbox_add_item( MGuiListbox* listbox, const char_t* text )
{
	MGuiListboxItem* item;
	struct MGuiListbox* list = (struct MGuiListbox*)listbox;

	if ( list == NULL ||
		 text == NULL )
		 return NULL;

	item = mem_alloc_clean( sizeof(*item) );
	item->parent = listbox;
	
	mgui_listbox_set_item_text( item, text );
	mgui_listbox_push_item( list, item );
	mgui_element_request_redraw( listbox );

	// Check whether we need the scrollbar.
	list->height += item->bounds.h;
	mgui_listbox_update_scrollbar( list );

	mgui_listbox_needs_scrollbar( list );

	return item;
}

/**
 * @brief Removes an item from a listbox.
 *
 * @details This function removes a previously added item from a listbox.
 *
 * @param listbox The listbox to remove an item from
 * @param item The item to be removed
 */
void mgui_listbox_remove_item( MGuiListbox* listbox, MGuiListboxItem* item )
{
	struct MGuiListbox* list = (struct MGuiListbox*)listbox;

	if ( list == NULL || item == NULL ) return;

	list_remove( list->items, (node_t*)item );

	SAFE_DELETE( item->text );
	SAFE_DELETE( item->tags );
	mem_free( item );

	list->first_visible = mgui_listbox_get_first_visible( list );

	if ( list->scrollbar != NULL )
	{
		// Check whether we need the scrollbar.
		list->height -= item->bounds.h;
		mgui_listbox_update_scrollbar( list );

		// Update item positions.
		mgui_listbox_update_positions( list, (MGuiListboxItem*)list_begin( list->items ) );
		mgui_element_request_redraw( listbox );
	}
}

/**
 * @brief Removes all items from a listbox.
 *
 * @details This function cleans up a listbox (all items are removed).
 *
 * @param listbox The listbox to clean
 */
void mgui_listbox_clean( MGuiListbox* listbox )
{
	struct MGuiListbox* list = (struct MGuiListbox*)listbox;
	MGuiListboxItem* item;
	node_t *node, *tmp;

	if ( list == NULL ) return;
	if ( list_empty( list->items ) ) return;

	list_foreach_safe( list->items, node, tmp )
	{
		item = (MGuiListboxItem*)node;

		list_remove( list->items, node );

		SAFE_DELETE( item->text );
		SAFE_DELETE( item->tags );
		mem_free( item );
	}
}

/**
 * @brief Set a comparison function for autosort.
 *
 * @details This function sets am item comparison function that will
 * be used for autosorting.
 *
 * @param listbox The listbox to add the sort function to
 * @param func A function that compares two listbox items (see @ref mgui_listbox_sort_t)
 * @sa mgui_listbox_sort_t
 */
void mgui_listbox_set_sort_function( MGuiListbox* listbox, mgui_listbox_sort_t func )
{
	struct MGuiListbox* list;

	if ( listbox == NULL ) return;

	list = (struct MGuiListbox*)listbox;
	list->sort = func;
}

/**
 * @brief Returns the number of items in a listbox.
 *
 * @details This function returns the number of added items in a listbox.
 *
 * @param listbox The listbox to get the item count of
 * @returns The number of items
 */
uint32 mgui_listbox_get_item_count( MGuiListbox* listbox )
{
	struct MGuiListbox* list;

	if ( listbox == NULL ) return 0;

	list = (struct MGuiListbox*)listbox;
	return list->items->size;
}

/**
 * @brief Returns the number of selected items in a listbox.
 *
 * @details This function returns the number of selected items in a listbox.
 *
 * @param listbox The listbox to get the item count of
 * @returns The number of selected items
 */
uint32 mgui_listbox_get_selected_count( MGuiListbox* listbox )
{
	struct MGuiListbox* list;

	if ( listbox == NULL ) return 0;

	list = (struct MGuiListbox*)listbox;
	return list->selected;
}

/**
 * @brief Returns the first item in a listbox.
 *
 * @details This function returns the first item in the listbox.
 *
 * @param listbox The listbox to get the first item of
 * @returns Pointer to the first item, or NULL if the listbox has no  items
 */
MGuiListboxItem* mgui_listbox_get_first_item( MGuiListbox* listbox )
{
	struct MGuiListbox* list;

	if ( listbox == NULL ) return NULL;

	list = (struct MGuiListbox*)listbox;
	if ( list_empty( list->items ) ) return NULL;

	return (MGuiListboxItem*)list_begin( list->items );
}

/**
 * @brief Returns the next item in a listbox.
 *
 * @details This function returns the next item from another listbox item.
 *
 * @param item A pointer to a listbox item
 * @returns Pointer to the next item, or NULL if the passed item was the last one
 */
MGuiListboxItem* mgui_listbox_get_next_item( MGuiListboxItem* item )
{
	struct MGuiListbox* list;
	node_t* next;

	if ( item == NULL ) return NULL;

	list = (struct MGuiListbox*)item->parent;
	next = item->node.next;

	return ( next != list_end( list->items ) ) ? (MGuiListboxItem*)next : NULL;
}

/**
 * @brief Returns the first selected item in a listbox.
 *
 * @details This function returns the first selected item in the listbox.
 *
 * @param listbox The listbox to get the first selected item of
 * @returns Pointer to the first selected item, or NULL if the listbox has no selected items
 */
MGuiListboxItem* mgui_listbox_get_selected_item( MGuiListbox* listbox )
{
	struct MGuiListbox* list;
	MGuiListboxItem* item;
	node_t* node;

	if ( listbox == NULL ) return NULL;

	list = (struct MGuiListbox*)listbox;

	list_foreach( list->items, node )
	{
		item = (MGuiListboxItem*)node;
		if ( item->selected ) return item;
	}

	return NULL;
}

/**
 * @brief Returns the next selected item in a listbox.
 *
 * @details This function returns the next selected item
 * from another listbox item.
 *
 * @param item A pointer to a listbox item
 * @returns Pointer to the next selected item, or NULL if the passed item was the last one
 */
MGuiListboxItem* mgui_listbox_get_next_selected_item( MGuiListboxItem* item )
{
	struct MGuiListbox* list;
	node_t* node;

	if ( item == NULL )
		return NULL;

	list = (struct MGuiListbox*)item->parent;
	
	for ( node = item->node.next; node != list_end( list->items ); node = item->node.next )
	{
		item = (MGuiListboxItem*)node;
		if ( item->selected ) return item;
	}

	return NULL;
}

/**
 * @brief Returns the text bound to a listbox item.
 *
 * @details This function returns the text bound to a certain listbox item.
 *
 * @param item Pointer to a listbox item
 * @returns Pointer to a text buffer that contains the text
 */
const char_t* mgui_listbox_get_item_text( MGuiListboxItem* item )
{
	return ( item != NULL ) ? item->text : NULL;
}

/**
 * @brief Sets the text bound to a listbox item.
 *
 * @details This function changes the text bound to a listbox item.
 *
 * @param item Pointer to a listbox item
 * @param text The new text value
 */
void mgui_listbox_set_item_text( MGuiListboxItem* item, const char_t* text )
{
	if ( item == NULL || text == NULL || item->parent == NULL )
		 return;

	// Free old buffer and tags.
	SAFE_DELETE( item->tags );
	SAFE_DELETE( item->text );

	// Allocate the new buffer and calculate its size. If format tags are enabled, process them too.
	if ( item->parent->flags & FLAG_TEXT_TAGS )
	{
		item->ntags = mgui_text_parse_and_get_line( text, item->parent->font, &item->parent->text->colour, 0, &item->text, &item->tags );
	}
	else
	{
		item->text = mstrdup( text, 0 );
		item->ntags = 0;
	}

	mgui_text_measure_buffer( item->parent->font, item->text, &item->text_bounds.uw, &item->text_bounds.uh );
}

/**
 * @brief Returns the user data bound to a listbox item.
 *
 * @details This function returns the user data bound to a listbox item.
 *
 * @param item Pointer to a listbox item
 * @returns Pointer to the user data that has been bound to the item, or NULL if there is no data
 */
void* mgui_listbox_get_item_data( MGuiListboxItem* item )
{
	return ( item != NULL ) ? item->data : NULL;
}

/**
 * @brief Sets the user data bound to a listbox item.
 *
 * @details This function sets or changes the user data
 * that is bound to a listbox item.
 *
 * @param item Pointer to a listbox item
 * @param data Pointer to the user data
 */
void mgui_listbox_set_item_data( MGuiListboxItem* item, void* data )
{
	if ( item == NULL ) return;
	item->data = data;
}

/**
 * @brief Returns the background colour of a selected item.
 *
 * @details This function returns the background colour of a listbox item
 * that has been selected by the user.
 *
 * @param listbox The listbox to get the selection colour of
 * @param col A pointer to a colour_t struct that will receive the colour
 */
void mgui_listbox_get_selected_colour( MGuiListbox* listbox, colour_t* col )
{
	struct MGuiListbox* list;

	if ( listbox == NULL || col == NULL ) return;

	list = (struct MGuiListbox*)listbox;
	*col = list->select_colour;
}

/**
 * @brief Sets the background colour of a selected item.
 *
 * @details This function changes the background colour of a listbox item
 * that has been selected by the user.
 *
 * @param listbox The listbox to change the selection colour of
 * @param col A pointer to a colour_t struct that contains the new colour
 */
void mgui_listbox_set_selected_colour( MGuiListbox* listbox, const colour_t* col )
{
	struct MGuiListbox* list;

	if ( listbox == NULL || col == NULL ) return;

	list = (struct MGuiListbox*)listbox;

	list->select_colour = *col;
	list->select_colour.a = list->colour.a;

	mgui_element_request_redraw( listbox );
}

/**
 * @brief Returns the background colour of a selected item.
 *
 * @details This function returns the background colour of a listbox
 * item that has been selected by the user. The colour is returned as
 * a 32bit hex integer in 0xRRGGBBAA format.
 *
 * @param listbox The listbox to get the selection colour of
 * @returns The background colour as a 32bit integer
 */
uint32 mgui_listbox_get_selected_colour_i( MGuiListbox* listbox )
{
	struct MGuiListbox* list;

	if ( listbox == NULL ) return 0;

	list = (struct MGuiListbox*)listbox;
	return list->select_colour.hex;
}

/**
 * @brief Sets the background colour of a selected item.
 *
 * @details This function changes the background colour of a listbox
 * item that has been selected by the user. The colour is passed as
 * a 32bit hex integer in 0xRRGGBBAA format.
 *
 * @param listbox The listbox to change the selection colour of
 * @param col The new colour as a 32bit integer
 */
void mgui_listbox_set_selected_colour_i( MGuiListbox* listbox, uint32 col )
{
	struct MGuiListbox* list;

	if ( listbox == NULL ) return;

	list = (struct MGuiListbox*)listbox;

	list->select_colour.hex = col;
	list->select_colour.a = list->colour.a;

	mgui_element_request_redraw( listbox );
}
