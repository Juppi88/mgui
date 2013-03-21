/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		WindowButton.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI window button (close) related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_WINDOWBUTTON_H
#define __MGUI_WINDOWBUTTON_H

#include "Element.h"
#include "Window.h"
#include "Input/Input.h"

struct MGuiWindowButton
{
	MGuiElement;			// Inherit all generic element properties
	MGuiWindow*	window;		// Parent window
};

MGuiWindowButton*	mgui_create_windowbutton			( MGuiWindow* parent );
void				mgui_destroy_windowbutton			( MGuiWindowButton* button );

// Window button callback handlers
static void			mgui_windowbutton_set_bounds		( MGuiElement* button, bool pos, bool size );
static void			mgui_windowbutton_set_colour		( MGuiElement* button );
static void			mgui_windowbutton_on_mouse_enter	( MGuiElement* button );
static void			mgui_windowbutton_on_mouse_leave	( MGuiElement* button );
static void			mgui_windowbutton_on_mouse_click	( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
static void			mgui_windowbutton_on_mouse_release	( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );

#endif /* __MGUI_WINDOWBUTTON_H */
