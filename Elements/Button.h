/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Button.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI button related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_BUTTON_H
#define __MGUI_BUTTON_H

#include "Element.h"

struct MGuiButton
{
	MGuiElement;	// Inherit all generic element properties
};

MGuiButton*		mgui_create_button		( MGuiElement* parent );
MGuiButton*		mgui_create_button_ex	( MGuiElement* parent, uint16 x, uint16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );

#endif /* __MGUI_BUTTON_H */
