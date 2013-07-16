/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (X11)
 * FILE:		Drawing.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		X11 GUI drawing functions.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_X11_DRAWING_H
#define __MYLLY_GUI_X11_DRAWING_H

#include "X11.h"

void	mgui_x11_drawing_init		( void );

void	mgui_x11_begin				( void );
void	mgui_x11_end				( void );
void	mgui_x11_resize				( uint32 w, uint32 h );

void	mgui_x11_set_draw_colour	( const colour_t* col );
void	mgui_x11_start_clip			( int32 x, int32 y, uint32 w, uint32 h );
void	mgui_x11_end_clip			( void );

void	mgui_x11_draw_rect			( int32 x, int32 y, uint32 w, uint32 h );
void	mgui_x11_draw_triangle		( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 );

void*	mgui_x11_load_texture		( const char* path );
void	mgui_x11_destroy_texture	( void* texture );
void	mgui_x11_draw_textured_rect	( void* texture, int32 x, int32 y, uint32 w, uint32 h );

void*	mgui_x11_load_font			( const char* name, uint32 size, uint32 flags, uint32 charset, uint32 firstc, uint32 lastc );
void	mgui_x11_destroy_font		( void* font );
void	mgui_x11_draw_text			( void* font, const char_t* text, int32 x, int32 y, uint32 flags );
void	mgui_x11_measure_text		( void* font, const char_t* text, uint32* w, uint32* h );

#endif /* __MYLLY_GUI_X11_DRAWING_H */
