/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (OpenGL)
 * FILE:		Drawing.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		OpenGL GUI drawing functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_OPENGL_DRAWING_H
#define __MYLLY_GUI_OPENGL_DRAWING_H

#include "OpenGL.h"

void	mgui_opengl_drawing_init		( void );

void	mgui_opengl_begin				( void );
void	mgui_opengl_end					( void );
void	mgui_opengl_resize				( uint w, uint h );

void	mgui_opengl_set_draw_colour		( const colour_t* col );
void	mgui_opengl_start_clip			( uint x, uint y, uint w, uint h );
void	mgui_opengl_end_clip			( void );

void	mgui_opengl_draw_rect			( uint x, uint y, uint w, uint h );
void	mgui_opengl_draw_triangle		( uint x1, uint y1, uint x2, uint y2, uint x3, uint y3 );

void*	mgui_opengl_load_texture		( const char* path );
void	mgui_opengl_destroy_texture		( void* texture );
void	mgui_opengl_draw_textured_rect	( void* texture, uint x, uint y, uint w, uint h );

void*	mgui_opengl_load_font			( const char* name, uint size, uint flags, uint charset, uint firstc, uint lastc );
void	mgui_opengl_destroy_font		( void* font );
void	mgui_opengl_draw_text			( void* font, const char_t* text, uint x, uint y, uint flags );
void	mgui_opengl_measure_text		( void* font, const char_t* text, uint* w, uint* h );

#endif /* __MYLLY_GUI_OPENGL_DRAWING_H */
