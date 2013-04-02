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

	float		position;		// Scroll position of the editbox 'screen'
	uint32		max_history;	// Max number of lines to keep as history
	list_t*		lines;			// A list of processed (wrapped) memobox lines
	list_t*		raw_lines;		// A list of unprocessed (raw) memobox lines
	node_t*		first_line;		// First line to render
	uint32		visible_lines;	// Number of lines to be rendered
};

MGuiMemobox*	mgui_create_memobox				( MGuiControl* parent );

void			mgui_memobox_add_line			( MGuiMemobox* memobox, const char* fmt, ... );
void			mgui_memobox_add_line_col		( MGuiMemobox* memobox, const char* fmt, const colour_t*, ... );
void			mgui_memobox_add_line_s			( MGuiMemobox* memobox, const char* text );
void			mgui_memobox_add_line_col_s		( MGuiMemobox* memobox, const char* text, const colour_t* );
void			mgui_memobox_clear				( MGuiMemobox* memobox );
float			mgui_memobox_get_display_pos	( MGuiMemobox* memobox );
void			mgui_memobox_set_display_pos	( MGuiMemobox* memobox, float pos );
bool			mgui_memobox_get_top_to_bottom	( MGuiMemobox* memobox );
void			mgui_memobox_set_top_to_bottom	( MGuiMemobox* memobox, bool enable );
uint32			mgui_memobox_get_lines			( MGuiMemobox* memobox );
uint32			mgui_memobox_get_history		( MGuiMemobox* memobox );
void			mgui_memobox_set_history		( MGuiMemobox* memobox, uint32 lines );

static void		mgui_memobox_update_display_positions_topbottom	( struct MGuiMemobox* memobox );
static void		mgui_memobox_update_display_positions_bottomtop	( struct MGuiMemobox* memobox );
static void		mgui_memobox_update_display_positions			( struct MGuiMemobox* memobox );
static void		mgui_memobox_wrap_line							( struct MGuiMemobox* memobox, struct MGuiMemoRaw* raw );
static void		mgui_memobox_process_new_line					( struct MGuiMemobox* memobox, struct MGuiMemoRaw* raw );

// Memobox callbacks
static void		mgui_destroy_memobox			( MGuiElement* memobox );
static void		mgui_memobox_render				( MGuiElement* memobox );
static void		mgui_memobox_set_bounds			( MGuiElement* memobox, bool pos, bool size );
static void		mgui_memobox_on_mouse_click		( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
static void		mgui_memobox_on_mouse_release	( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
static void		mgui_memobox_on_mouse_drag		( MGuiElement* element, uint16 x, uint16 y );
static void		mgui_memobox_on_key_press		( MGuiElement* element, uint key, bool down );


#endif /* __MGUI_MEMOBOX_H */
