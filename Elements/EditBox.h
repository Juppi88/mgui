/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		EditBox.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI editbox related functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_EDITBOX_H
#define __MGUI_EDITBOX_H

#include "Element.h"

struct editbox_s
{
	element_t;							// Inherit all generic element properties

	char_t*			buffer;				// The displayed/masked text buffer
	uint32			cursor_pos;			// Current cursor position
	uint32			cursor_end;			// Cursor end pos (if text has been selected)
	rectangle_t		cursor;				// Cursor bounds
	rectangle_t		selection;			// Selection bounds
	uint32			last_update;		// Cursor animation last update timestamp
	bool			cursor_visible;		// Should the cursor be visible on this cycle?
};

#endif /* __MGUI_EDITBOX_H */
