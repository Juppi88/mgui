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
#include "Input/Input.h"

struct MGuiButton
{
	MGuiElement;	// Inherit all generic element properties
};

MGuiButton*		mgui_create_button			( MGuiControl* parent );

#endif /* __MGUI_BUTTON_H */
