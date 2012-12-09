/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (DirectX8)
 * FILE:		Drawing.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		DirectX8 GUI drawing functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_DX8_DRAWING_H
#define __MYLLY_GUI_DX8_DRAWING_H

#include "DX8.h"

void	mgui_dx8_drawing_initialize	( void );
void	mgui_dx8_drawing_shutdown	( void );

void	mgui_dx8_begin				( void );
void	mgui_dx8_end				( void );
void	mgui_dx8_resize				( uint w, uint h );

void	mgui_dx8_set_draw_colour	( const colour_t* col );
void	mgui_dx8_start_clip			( uint x, uint y, uint w, uint h );
void	mgui_dx8_end_clip			( void );

void	mgui_dx8_draw_rect			( uint x, uint y, uint w, uint h );
void	mgui_dx8_draw_triangle		( uint x1, uint y1, uint x2, uint y2, uint x3, uint y3 );

void*	mgui_dx8_load_texture		( const char* path );
void	mgui_dx8_destroy_texture	( void* texture );
void	mgui_dx8_draw_textured_rect	( void* texture, uint x, uint y, uint w, uint h );

void*	mgui_dx8_load_font			( const char* name, uint size, uint flags, uint charset, uint firstc, uint lastc );
void	mgui_dx8_destroy_font		( void* font );
void	mgui_dx8_draw_text			( void* font, const char_t* text, uint x, uint y, uint flags );
void	mgui_dx8_measure_text		( void* font, const char_t* text, uint* w, uint* h );

#endif /* __MYLLY_GUI_DX8_DRAWING_H */
