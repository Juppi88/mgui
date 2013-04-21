/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		SkinSimple.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		An implementation of a basic, textureless skin.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_SKINSIMPLE_H
#define __MYLLY_GUI_SKINSIMPLE_H

#include "Skin.h"

MGuiSkin*		mgui_setup_skin_simple				( void );

static void		skin_simple_draw_panel				( const rectangle_t* r, const colour_t* col );
static void		skin_simple_draw_border				( const rectangle_t* r, const colour_t* col, uint32 borders, uint32 thickness );
static void		skin_simple_draw_generic_button		( const rectangle_t* r, const colour_t* col, uint32 flags );
static void		skin_simple_draw_shadow				( const rectangle_t* r, uint32 offset );
static void		skin_simple_draw_button				( MGuiElement* element );
static void		skin_simple_draw_editbox			( MGuiElement* element );
static void		skin_simple_draw_label				( MGuiElement* element );
static void		skin_simple_draw_memobox			( MGuiElement* element );
static void		skin_simple_draw_scrollbar			( MGuiElement* element );
static void		skin_simple_draw_scrollbar_button	( const rectangle_t* r, const colour_t* col, const colour_t* arrowcol, uint32 flags, uint32 dir );
static void		skin_simple_draw_window				( MGuiElement* element );
static void		skin_simple_draw_window_titlebar	( MGuiElement* element );

#endif /* __MYLLY_GUI_SKINSIMPLE_H */
