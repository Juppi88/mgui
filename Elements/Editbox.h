/**
 *
 * @file		Editbox.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI editbox related functions.
 *
 * @details		Functions and structures related to GUI editboxes.
 *
 **/

#pragma once
#ifndef __MGUI_EDITBOX_H
#define __MGUI_EDITBOX_H

#include "Element.h"

/**
 * @brief GUI editbox.
 *
 * @details Editbox is an element that allows the user to input text information
 * to be used by the program.
 */
struct MGuiEditbox {
	MGuiElement;					///< Inherit MGuiElement members.

	char_t*			buffer;			///< Pointer to the text buffer that is rendered (masked or unmasked, depending on flags)
	uint32			cursor_pos;		///< Current cursor position (an offset from the beginning of the string in characters)
	uint32			cursor_end;		///< Cursor end position (if text has been selected)
	rectangle_t		cursor;			///< Boundaries for the cursor rectangle
	rectangle_t		selection;		///< Boundaries for the selection rectangle
	uint32			last_update;	///< Animation timestamp (indicates the last animation cycle)
	bool			cursor_visible;	///< Is the cursor currently visible
};

MGuiEditbox* mgui_create_editbox				( MGuiElement* parent );
MGuiEditbox* mgui_create_editbox_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );

bool		mgui_editbox_has_text_selected	( MGuiEditbox* editbox );
void		mgui_editbox_get_selection		( MGuiEditbox* editbox, char_t* buf, size_t buflen );
void		mgui_editbox_select_text		( MGuiEditbox* editbox, uint32 begin, uint32 end );
uint32		mgui_editbox_get_cursor_pos		( MGuiEditbox* editbox );
void		mgui_editbox_set_cursor_pos		( MGuiEditbox* editbox, uint32 pos );

#endif /* __MGUI_EDITBOX_H */
