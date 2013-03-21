/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		EditBox.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI editbox related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_EDITBOX_H
#define __MGUI_EDITBOX_H

#include "Element.h"

struct MGuiEditbox
{
	MGuiElement;					// Inherit all generic element properties

	char_t*			buffer;			// The displayed/masked text buffer
	uint32			cursor_pos;		// Current cursor position
	uint32			cursor_end;		// Cursor end pos (if text has been selected)
	rectangle_t		cursor;			// Cursor bounds
	rectangle_t		selection;		// Selection bounds
	uint32			last_update;	// Cursor animation last update timestamp
	bool			cursor_visible;	// Should the cursor be visible on this cycle?
};

MGuiEditbox*		mgui_create_editbox					( MGuiControl* parent );

void				mgui_editbox_get_selection			( MGuiEditbox* editbox, char_t* buf, size_t buflen );
void				mgui_editbox_select_text			( MGuiEditbox* editbox, uint32 begin, uint32 end );
uint32				mgui_editbox_get_cursor_pos			( MGuiEditbox* editbox );
void				mgui_editbox_set_cursor_pos			( MGuiEditbox* editbox, uint32 pos );

static void			mgui_editbox_refresh_cursor_bounds	( struct MGuiEditbox* editbox );
static void			mgui_editbox_erase_text				( struct MGuiEditbox* editbox, uint32 begin, uint32 end );
static void			mgui_editbox_insert_text			( struct MGuiEditbox* editbox, const char_t* text, size_t len );
static void			mgui_editbox_select_all				( struct MGuiEditbox* editbox );
static void			mgui_editbox_cut_selection			( struct MGuiEditbox* editbox );
static void			mgui_editbox_copy_selection			( struct MGuiEditbox* editbox );
static void			mgui_editbox_paste_selection		( struct MGuiEditbox* editbox );
static void			mgui_editbox_press_backspace		( struct MGuiEditbox* editbox );
static void			mgui_editbox_press_delete			( struct MGuiEditbox* editbox );
static void			mgui_editbox_press_return			( struct MGuiEditbox* editbox );
static void			mgui_editbox_move_left				( struct MGuiEditbox* editbox );
static void			mgui_editbox_move_right				( struct MGuiEditbox* editbox );
static void			mgui_editbox_press_home				( struct MGuiEditbox* editbox );
static void			mgui_editbox_press_end				( struct MGuiEditbox* editbox );


// Editbox callback handlers
static void			mgui_destroy_editbox				( MGuiElement* editbox );
static void			mgui_editbox_render					( MGuiElement* element );
static void			mgui_editbox_process				( MGuiElement* element, uint32 ticks );
static void			mgui_editbox_set_text				( MGuiElement* element );
static void			mgui_editbox_on_mouse_click			( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
static void			mgui_editbox_on_mouse_release		( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
static void			mgui_editbox_on_mouse_drag			( MGuiElement* element, uint16 x, uint16 y );
static void			mgui_editbox_on_character			( MGuiElement* element, char_t c );
static void			mgui_editbox_on_key_press			( MGuiElement* element, uint key, bool down );

#endif /* __MGUI_EDITBOX_H */
