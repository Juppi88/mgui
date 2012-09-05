/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Skin.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A skin for the GUI. This can be either a simple layout
 *				using basic coloured rectangles, or a pre-defined skin
 *				texture.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_SKIN
#define __MYLLY_GUI_SKIN

#include "MGUI.h"
#include "Text.h"

// Some default element values
#define COL_BACKGROUND		0xAFAF9FFF
#define COL_ELEMENT			0xAFAF9FFF
#define COL_ELEMENT_DARK	0x989884FF
#define COL_TEXT			0x303030FF
#define COL_TITLEBAR		0x121314FF
#define COL_TITLETEXT		0xE0E0E0FF

#ifdef _WIN32
	#define DEFAULT_FONT _TEXT("Tahoma")
#else
	#define DEFAULT_FONT _TEXT("DejaVu Sans Mono")
#endif

// Element borders
#define BORDER_LEFT		0x1
#define BORDER_RIGHT	0x2
#define BORDER_TOP		0x4
#define BORDER_BOTTOM	0x8
#define BORDER_ALL		(BORDER_LEFT|BORDER_RIGHT|BORDER_TOP|BORDER_BOTTOM)

// Skin interface 
typedef struct skin_s
{
	void			( *draw_panel )					( const rectangle_t* r, const colour_t* col );
	void			( *draw_border )				( const rectangle_t* r, const colour_t* col, uint8 borders, uint8 thickness );
	void			( *draw_shadow )				( const rectangle_t* r, uint offset );

	void			( *draw_button )				( const rectangle_t* r, const colour_t* col, uint32 flags, const text_t* text );
	void			( *draw_editbox )				( const rectangle_t* r, colour_t* col, uint32 flags, const text_t* text );
	void			( *draw_label )					( const rectangle_t* r, colour_t* col, uint32 flags, const text_t* text );
	void			( *draw_window )				( const rectangle_t* r, const colour_t* col, uint32 flags );
	void			( *draw_window_titlebar )		( const rectangle_t* r, const colour_t* col, const text_t* text );
}
skin_t;

// Functions to initialize a skin
skin_t* mgui_setup_skin_simple( void );
skin_t* mgui_setup_skin_textured( const char_t* texture );

// Pointer to the currently used skin
extern skin_t* skin;

#endif /* __MYLLY_GUI_SKIN */
