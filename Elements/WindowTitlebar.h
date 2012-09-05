/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		WindowTitlebar.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI window titlebar.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_WINDOWTITLEBAR_H
#define __MGUI_WINDOWTITLEBAR_H

#include "Element.h"
#include "Window.h"

#define TITLE_H 26

struct titlebar_s
{
	element_t;							// Inherit all generic element properties

	struct window_s*	window;			// Parent window
};

struct titlebar_s*	mgui_create_titlebar		( struct window_s* parent );
void				mgui_destroy_titlebar		( struct titlebar_s* titlebar );

#endif /* __MGUI_WINDOWTITLEBAR_H */
