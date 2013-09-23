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

struct MGuiTitlebar {
	MGuiElement;			// Inherit all generic element properties
	MGuiWindow*	window;		// Parent window
};

MGuiTitlebar* mgui_create_titlebar( MGuiWindow* parent );

#endif /* __MGUI_WINDOWTITLEBAR_H */
