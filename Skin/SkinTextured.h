/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		SkinTextured.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		An implementation of a basic textured skin.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_SKINTEXTURED_H
#define __MYLLY_GUI_SKINTEXTURED_H

#include "Skin.h"

MGuiSkin*		mgui_setup_skin_textured			( const char_t* texture );

static void		skin_textured_draw_panel			( const rectangle_t* r, const colour_t* col );
static void		skin_textured_draw_border			( const rectangle_t* r, const colour_t* col, uint32 borders, uint32 thickness );
static void		skin_textured_draw_shadow			( const rectangle_t* r, uint offset );
static void		skin_textured_draw_button			( MGuiElement* element );
static void		skin_textured_draw_editbox			( MGuiElement* element );
static void		skin_textured_draw_label			( MGuiElement* element );
static void		skin_textured_draw_memobox			( MGuiElement* element );
static void		skin_textured_draw_scrollbar		( MGuiElement* element );
static void		skin_textured_draw_window			( MGuiElement* element );
static void		skin_textured_draw_window_titlebar	( MGuiElement* element );

#endif /* __MYLLY_GUI_SKINTEXTURED_H */
