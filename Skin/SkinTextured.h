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
static void		skin_textured_draw_button			( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text );
static void		skin_textured_draw_editbox			( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text );
static void		skin_textured_draw_label			( const rectangle_t* r, const colour_t* col, uint32 flags, const MGuiText* text );
static void		skin_textured_draw_memobox			( const rectangle_t* r, const colour_t* col, uint32 flags );
static void		skin_textured_draw_memobox_lines	( const rectangle_t* r, uint32 flags, list_t* lines, node_t* first, uint32 count );
static void		skin_textured_draw_scrollbar		( const rectangle_t* r, const colour_t* col, uint32 flags );
static void		skin_textured_draw_scrollbar_bar	( const rectangle_t* r, const colour_t* col, uint32 flags );
static void		skin_textured_draw_scrollbar_button	( const rectangle_t* r, const colour_t* col, uint32 flags, const colour_t* arrowcol, uint32 direction );
static void		skin_textured_draw_window			( const rectangle_t* r, const colour_t* col, uint32 flags );
static void		skin_textured_draw_window_titlebar	( const rectangle_t* r, const colour_t* col, const MGuiText* text );

#endif /* __MYLLY_GUI_SKINTEXTURED_H */
