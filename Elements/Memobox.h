/**
 *
 * @file		Memobox.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI memobox related functions.
 *
 * @details		Functions and structures related to GUI memoboxes.
 *
 **/

#pragma once
#ifndef __MGUI_MEMOBOX_H
#define __MGUI_MEMOBOX_H

#include "Element.h"
#include "Scrollbar.h"

/**
 * @brief Formatted memobox line.
 *
 * @details MGuiMemoLine is a data container for a parsed and formatted
 * line of text in a memobox.
 */
struct MGuiMemoLine {
	node_t;					///< Linked list node
	char_t*			text;	///< Pointer to a text buffer that contains the line without format tags
	MGuiFont*		font;	///< Pointer to a font data structure that is used to render the line
	colour_t		colour;	///< Default colour for this memobox line
	MGuiFormatTag*	tags;	///< An array of parsed format tags
	uint32			ntags;	///< Number of format tags in the array above
	vectorscreen_t	pos;	///< Absolute position on the screen
};

/**
 * @brief Unparsed memobox line.
 * @details MGuiMemoRaw is a container for an unparsed line of text in a memobox.
 */
struct MGuiMemoRaw {
	node_t;					///< Linked list node
	char_t*		text;		///< Pointer to a text buffer that contains the unparsed line
	colour_t	colour;		///< Default colour for the text
};

/**
 * @brief GUI memobox.
 * @details Memobox is a multiline read-only textbox.
 */
struct MGuiMemobox {
	MGuiElement;							///< Inherit MGuiElement members
	float					position;		///< Current scroll position inside the renderable area
	uint8					margin;			///< Margin between two memobox lines in pixels
	uint8					max_history;	///< Maximum number of raw input lines to be stored as history
	uint8					num_lines;		///< Maximum number of visible lines to be shown in the memobox at once
	uint8					visible_lines;	///< Current number of visible lines
	list_t*					lines;			///< List of processed and wrapped memobox lines (see @ref MGuiMemoLine)
	list_t*					raw_lines;		///< List of unprocessed (raw) memobox lines (see @ref MGuiMemoRaw)
	node_t*					first_line;		///< First visible line to be rendered
	struct MGuiScrollbar*	scrollbar;		///< The scrollbar element that is shown if the memobox gets too big
};

MGuiMemobox* mgui_create_memobox	( MGuiElement* parent );
MGuiMemobox* mgui_create_memobox_ex	( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col );

void	mgui_memobox_add_line		( MGuiMemobox* memobox, const char* fmt, ... );
void	mgui_memobox_add_line_col	( MGuiMemobox* memobox, const char* fmt, const colour_t*, ... );
void	mgui_memobox_add_line_s		( MGuiMemobox* memobox, const char* text );
void	mgui_memobox_add_line_col_s	( MGuiMemobox* memobox, const char* text, const colour_t* );
void	mgui_memobox_clear			( MGuiMemobox* memobox );
float	mgui_memobox_get_display_pos( MGuiMemobox* memobox );
void	mgui_memobox_set_display_pos( MGuiMemobox* memobox, float pos );
uint32	mgui_memobox_get_lines		( MGuiMemobox* memobox );
void	mgui_memobox_set_lines		( MGuiMemobox* memobox, uint32 lines );
uint32	mgui_memobox_get_num_lines	( MGuiMemobox* memobox );
uint32	mgui_memobox_get_history	( MGuiMemobox* memobox );
void	mgui_memobox_set_history	( MGuiMemobox* memobox, uint32 lines );
uint32	mgui_memobox_get_margin		( MGuiMemobox* memobox );
void	mgui_memobox_set_margin		( MGuiMemobox* memobox, uint32 margin );

#endif /* __MGUI_MEMOBOX_H */
