/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Label.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI label related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 * 
 **********************************************************************/

#pragma once
#ifndef __MGUI_LABEL_H
#define __MGUI_LABEL_H

#include "Element.h"

struct MGuiLabel
{
	MGuiElement;	// Inherit all generic element properties
};

MGuiLabel*			mgui_create_label			( MGuiElement* parent );
MGuiLabel*			mgui_create_label_ex		( MGuiElement* parent, uint16 x, uint16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );

void				mgui_label_make_text_fit	( MGuiLabel* label );

#endif /* __MGUI_LABEL_H */
