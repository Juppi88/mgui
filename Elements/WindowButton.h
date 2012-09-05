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

struct windowbutton_s
{
	element_t;							// Inherit all generic element properties
	struct window_s*	window;			// Parent window
};

struct windowbutton_s*	mgui_create_windowbutton	( struct window_s* parent );
void					mgui_destroy_windowbutton	( struct windowbutton_s* button );

#endif /* __MGUI_WINDOWBUTTON_H */
