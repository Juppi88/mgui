/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		WindowTitlebar.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI window titlebar.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_WINDOWTITLEBAR_H
#define __MGUI_WINDOWTITLEBAR_H

#include "Element.h"
#include "Window.h"

#define TITLE_H 26

struct MGuiTitlebar
{
	MGuiElement;			// Inherit all generic element properties
	MGuiWindow*	window;		// Parent window
};

MGuiTitlebar*	mgui_create_titlebar			( MGuiWindow* parent );
void			mgui_destroy_titlebar			( MGuiTitlebar* titlebar );

// Window titlebar callback handlers
static void		mgui_titlebar_on_mouse_click	( MGuiElement* element, MOUSEBTN button, uint16 x, uint16 y );
static void		mgui_titlebar_on_mouse_drag		( MGuiElement* element, uint16 x, uint16 y );

#endif /* __MGUI_WINDOWTITLEBAR_H */
