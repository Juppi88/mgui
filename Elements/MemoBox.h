/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		MemoBox.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI memobox related functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 * 
 **********************************************************************/

#pragma once
#ifndef __MGUI_MEMOBOX_H
#define __MGUI_MEMOBOX_H

#include "Element.h"

struct memoline_s
{
	node_t;
	char_t*			text;
	font_t*			font;
	colour_t		colour;
	vectorscreen_t	pos;
};

struct memoraw_s
{
	node_t;
	char_t*		text;
	colour_t	colour;
};

struct memobox_s
{
	element_t;					// Inherit all generic element properties

	float		position;		// Scroll position of the editbox 'screen'
	uint32		max_history;	// Max number of lines to keep as history
	list_t*		lines;			// A list of processed (wrapped) memobox lines
	list_t*		raw_lines;		// A list of unprocessed (raw) memobox lines
	node_t*		first_line;		// First line to render
	uint32		visible_lines;	// Number of lines to be rendered
};

#endif /* __MGUI_MEMOBOX_H */
