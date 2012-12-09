/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		WindowButton.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI window button (close) related functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_WINDOWBUTTON_H
#define __MGUI_WINDOWBUTTON_H

#include "Element.h"
#include "Window.h"
#include "Input/Input.h"

struct _MGuiWindowButton
{
	MGuiElement;			// Inherit all generic element properties
	MGuiWindow*	window;		// Parent window
};

MGuiWindowButton*	mgui_create_windowbutton	( MGuiWindow* parent );
void				mgui_destroy_windowbutton	( MGuiWindowButton* button );

#endif /* __MGUI_WINDOWBUTTON_H */
