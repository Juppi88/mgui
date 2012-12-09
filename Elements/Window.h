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

typedef struct _MGuiTitlebar MGuiTitlebar;
typedef struct _MGuiWindowButton MGuiWindowButton;

struct _MGuiWindow
{
	MGuiElement;						// Inherit all generic element properties

	rectangle_t			window_bounds;	// Window boundaries including the titlebar
	vectorscreen_t		click_offset;	// Position offset when dragging this element
	MGuiTitlebar*		titlebar;		// Window titlebar
	MGuiWindowButton*	closebtn;		// Close button
};

#endif /* __MGUI_WINDOW_H */
