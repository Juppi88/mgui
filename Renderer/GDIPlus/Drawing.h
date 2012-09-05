/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (GDI+)
 * FILE:		Drawing.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GDI+ GUI drawing functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_GDIPLUS_DRAWING_H
#define __MYLLY_GUI_GDIPLUS_DRAWING_H

#include "GDI.h"

void	mgui_gdiplus_drawing_init		( void );

void	mgui_gdiplus_begin				( void );
void	mgui_gdiplus_end				( void );
void	mgui_gdiplus_resize				( uint w, uint h );

void	mgui_gdiplus_set_draw_colour	( const colour_t* col );
void	mgui_gdiplus_start_clip			( uint x, uint y, uint w, uint h );
void	mgui_gdiplus_end_clip			( void );

void	mgui_gdiplus_draw_rect			( uint x, uint y, uint w, uint h );
void	mgui_gdiplus_draw_triangle		( uint x1, uint y1, uint x2, uint y2, uint x3, uint y3 );

void*	mgui_gdiplus_load_texture		( const char* path );
void	mgui_gdiplus_destroy_texture	( void* texture );
void	mgui_gdiplus_draw_textured_rect	( void* texture, uint x, uint y, uint w, uint h );

void*	mgui_gdiplus_load_font			( const char* name, uint size, uint flags, uint charset, uint firstc, uint lastc );
void	mgui_gdiplus_destroy_font		( void* font );
void	mgui_gdiplus_draw_text			( void* font, const char_t* text, uint x, uint y, uint flags );
void	mgui_gdiplus_measure_text		( void* font, const char_t* text, uint* w, uint* h );

#endif /* __MYLLY_GUI_GDIPLUS_DRAWING_H */
