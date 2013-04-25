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

MGuiEditbox*		mgui_create_editbox					( MGuiElement* parent );
MGuiEditbox*		mgui_create_editbox_ex				( MGuiElement* parent, uint16 x, uint16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );

void				mgui_editbox_get_selection			( MGuiEditbox* editbox, char_t* buf, size_t buflen );
void				mgui_editbox_select_text			( MGuiEditbox* editbox, uint32 begin, uint32 end );
uint32				mgui_editbox_get_cursor_pos			( MGuiEditbox* editbox );
void				mgui_editbox_set_cursor_pos			( MGuiEditbox* editbox, uint32 pos );

#endif /* __MGUI_EDITBOX_H */
