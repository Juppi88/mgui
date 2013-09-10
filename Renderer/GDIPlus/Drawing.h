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
void	mgui_gdiplus_resize				( uint32 w, uint32 h );

void	mgui_gdiplus_set_draw_colour	( const colour_t* col );
void	mgui_gdiplus_start_clip			( int32 x, int32 y, uint32 w, uint32 h );
void	mgui_gdiplus_end_clip			( void );

void	mgui_gdiplus_draw_rect			( int32 x, int32 y, uint32 w, uint32 h );
void	mgui_gdiplus_draw_triangle		( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 );

void*	mgui_gdiplus_load_texture		( const char* path, uint32* width, uint32* height );
void	mgui_gdiplus_destroy_texture	( void* texture );
void	mgui_gdiplus_draw_textured_rect	( const void* texture, int32 x, int32 y, uint32 w, uint32 h,
										  float u1, float v1, float u2, float v2 );

void*	mgui_gdiplus_load_font			( const char* name, uint32 size, uint32 flags, uint32 charset,
										  uint32 firstc, uint32 lastc );

void	mgui_gdiplus_destroy_font		( void* font );

void	mgui_gdiplus_draw_text			( const void* font, const char_t* text, int32 x, int32 y,
										  uint32 flags, const MGuiFormatTag tags[], uint32 ntags );

void	mgui_gdiplus_measure_text		( const void* font, const char_t* text, uint32* w, uint32* h );

#endif /* __MYLLY_GUI_GDIPLUS_DRAWING_H */
