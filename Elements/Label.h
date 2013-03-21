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

MGuiLabel*			mgui_create_label			( MGuiControl* parent );
void				mgui_label_make_text_fit	( MGuiLabel* label );

// Label callback handlers
static void			mgui_destroy_label			( MGuiElement* label );
static void			mgui_label_render			( MGuiElement* label );

#endif /* __MGUI_LABEL_H */
