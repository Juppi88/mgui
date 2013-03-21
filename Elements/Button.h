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

// Buton callbacks
static void		mgui_destroy_button			( MGuiElement* button );
static void		mgui_button_render			( MGuiElement* button );
static void		mgui_button_set_bounds		( MGuiElement* button, bool pos, bool size );
static void		mgui_button_on_mouse_enter	( MGuiElement* button );
static void		mgui_button_on_mouse_leave	( MGuiElement* button );
static void		mgui_button_on_key_press	( MGuiElement* element, uint key, bool down );

#endif /* __MGUI_BUTTON_H */
