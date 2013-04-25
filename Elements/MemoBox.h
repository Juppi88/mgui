/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		MemoBox.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI memobox related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 * 
 **********************************************************************/

#pragma once
#ifndef __MGUI_MEMOBOX_H
#define __MGUI_MEMOBOX_H

#include "Element.h"

struct MGuiMemoLine
{
	node_t;
	char_t*			text;
	MGuiFont*		font;
	colour_t		colour;
	vectorscreen_t	pos;
};

struct MGuiMemoRaw
{
	node_t;
	char_t*		text;
	colour_t	colour;
};

struct MGuiMemobox
{
	MGuiElement;				// Inherit all generic element properties

	float		position;		// Scroll position on the editbox render area
	uint8		margin;			// Margin between two memobox lines
	uint8		max_history;	// Max number of lines to keep as history
	uint8		num_lines;		// Max number of lines to show in the memobox at once
	uint8		visible_lines;	// Number of lines to be rendered
	list_t*		lines;			// A list of processed (wrapped) memobox lines
	list_t*		raw_lines;		// A list of unprocessed (raw) memobox lines
	node_t*		first_line;		// First visible line to be rendered
};

MGuiMemobox*	mgui_create_memobox				( MGuiElement* parent );

void			mgui_memobox_add_line			( MGuiMemobox* memobox, const char* fmt, ... );
void			mgui_memobox_add_line_col		( MGuiMemobox* memobox, const char* fmt, const colour_t*, ... );
void			mgui_memobox_add_line_s			( MGuiMemobox* memobox, const char* text );
void			mgui_memobox_add_line_col_s		( MGuiMemobox* memobox, const char* text, const colour_t* );
void			mgui_memobox_clear				( MGuiMemobox* memobox );
float			mgui_memobox_get_display_pos	( MGuiMemobox* memobox );
void			mgui_memobox_set_display_pos	( MGuiMemobox* memobox, float pos );
uint32			mgui_memobox_get_lines			( MGuiMemobox* memobox );
void			mgui_memobox_set_lines			( MGuiMemobox* memobox, uint32 lines );
uint32			mgui_memobox_get_num_lines		( MGuiMemobox* memobox );
uint32			mgui_memobox_get_history		( MGuiMemobox* memobox );
void			mgui_memobox_set_history		( MGuiMemobox* memobox, uint32 lines );
uint32			mgui_memobox_get_margin			( MGuiMemobox* memobox );
void			mgui_memobox_set_margin			( MGuiMemobox* memobox, uint32 margin );

#endif /* __MGUI_MEMOBOX_H */
