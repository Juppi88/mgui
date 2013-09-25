/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Checkbox.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI checkbox related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 * 
 **********************************************************************/

#pragma once
#ifndef __MGUI_CHECKBOX_H
#define __MGUI_CHECKBOX_H

#include "Element.h"

struct MGuiCheckbox {
	MGuiElement;	// Inherit all generic element properties
};

MGuiCheckbox*	mgui_create_checkbox	( MGuiElement* parent );
MGuiCheckbox*	mgui_create_checkbox_ex	( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col );

#endif /* __MGUI_CHECKBOX_H */
