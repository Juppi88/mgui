/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Skin.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A skin for the GUI. This can be either a simple layout
 *				using basic coloured rectangles, or a pre-defined skin
 *				texture.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_SKIN_H
#define __MYLLY_GUI_SKIN_H

#include "MGUI.h"

// Default element style
#define COL_BACKGROUND		0xAFAF9FFF
#define COL_ELEMENT			0xAFAF9FFF
#define COL_ELEMENT_DARK	0x989884FF
#define COL_TEXT			0x303030FF
#define COL_TITLEBAR		0x121314FF
#define COL_TITLETEXT		0xE0E0E0FF

#ifdef _WIN32
	#define DEFAULT_FONT _MTEXT("Tahoma")
#else
	#define DEFAULT_FONT _MTEXT("fixed")
#endif

// Element borders
#define BORDER_NONE		0x0
#define BORDER_LEFT		0x1
#define BORDER_RIGHT	0x2
#define BORDER_TOP		0x4
#define BORDER_BOTTOM	0x8
#define BORDER_ALL		(BORDER_LEFT|BORDER_RIGHT|BORDER_TOP|BORDER_BOTTOM)

// Arrow directions
#define ARROW_UP		0x1
#define ARROW_DOWN		0x2
#define ARROW_LEFT		0x3
#define ARROW_RIGHT		0x4

// Skin interface
typedef struct MGuiSkin {
	void	( *draw_button )		( MGuiElement* element );
	void	( *draw_checkbox )		( MGuiElement* element );
	void	( *draw_editbox )		( MGuiElement* element );
	void	( *draw_label )			( MGuiElement* element );
	void	( *draw_listbox )		( MGuiElement* element );
	void	( *draw_memobox )		( MGuiElement* element );
	void	( *draw_progressbar )	( MGuiElement* element );
	void	( *draw_scrollbar )		( MGuiElement* element );
	void	( *draw_window )		( MGuiElement* element );
} MGuiSkin;


// Pointer to the currently used skin
extern MGuiSkin* skin;

#endif /* __MYLLY_GUI_SKIN_H */
