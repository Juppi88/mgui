/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Window.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI window related functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_WINDOW_H
#define __MGUI_WINDOW_H

#include "Element.h"
#include "WindowButton.h"
#include "WindowTitlebar.h"

struct window_s
{
	element_t;								// Inherit all generic element properties

	rectangle_t				window_bounds;	// Window boundaries including the titlebar
	vectorscreen_t			click_offset;	// Position offset when dragging this element
	struct titlebar_s*		titlebar;		// Window titlebar
	struct windowbutton_s*	closebtn;		// Close button
};

#endif /* __MGUI_WINDOW_H */
