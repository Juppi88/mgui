/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Progressbar.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI progress bar related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_PROGRESSBAR_H
#define __MGUI_PROGRESSBAR_H

#include "Element.h"

struct MGuiProgressbar {
	MGuiElement;					// Inherit all generic element properties

	float			value;			// Current value
	float			max_value;		// Max value
	colour_t		colour_start;	// Start colour
	colour_t		colour_end;		// End colour
	colour_t		colour_fg;		// Foreground colour (calculated from colour_start and colour_end)
	colour_t		colour_bg;		// Background colour (ditto)
	float			bg_shade;		// Background shade (ie. percentage of the foreground colour)
	uint8			thickness;		// Border thickness
};

MGuiProgressbar*	mgui_create_progressbar				( MGuiElement* parent );
MGuiProgressbar*	mgui_create_progressbar_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col1, uint32 col2, float max_value );

float				mgui_progressbar_get_value			( MGuiProgressbar* bar );
void				mgui_progressbar_set_value			( MGuiProgressbar* bar, float value );
float				mgui_progressbar_get_max_value		( MGuiProgressbar* bar );
void				mgui_progressbar_set_max_value		( MGuiProgressbar* bar, float value );

void				mgui_progressbar_get_colour			( MGuiProgressbar* bar, colour_t* col_start, colour_t* col_end );
void				mgui_progressbar_set_colour			( MGuiProgressbar* bar, const colour_t* col_start, const colour_t* col_end );
void				mgui_progressbar_get_colour_i		( MGuiProgressbar* bar, uint32* col_start, uint32* col_end );
void				mgui_progressbar_set_colour_i		( MGuiProgressbar* bar, uint32 col_start, uint32 col_end );
float				mgui_progressbar_get_bg_shade		( MGuiProgressbar* bar );
void				mgui_progressbar_set_bg_shade		( MGuiProgressbar* bar, float shade );
uint8				mgui_progressbar_get_thickness		( MGuiProgressbar* bar );
void				mgui_progressbar_set_thickness		( MGuiProgressbar* bar, uint8 thickness );

#endif /* __MGUI_PROGRESSBAR_H */
