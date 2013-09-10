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

void		mgui_opengl_drawing_init		( void );

void		mgui_opengl_begin				( void );
void		mgui_opengl_end					( void );
void		mgui_opengl_resize				( uint32 w, uint32 h );

DRAW_MODE	mgui_opengl_set_draw_mode		( DRAW_MODE mode );
void		mgui_opengl_set_draw_colour		( const colour_t* col );
void		mgui_opengl_set_draw_depth		( float z_depth );
void		mgui_opengl_set_draw_transform	( const matrix4_t* transform );
void		mgui_opengl_reset_draw_transform( void );

void		mgui_opengl_start_clip			( int32 x, int32 y, uint32 w, uint32 h );
void		mgui_opengl_end_clip			( void );

void		mgui_opengl_draw_rect			( int32 x, int32 y, uint32 w, uint32 h );
void		mgui_opengl_draw_triangle		( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 );
void		mgui_opengl_draw_pixel			( int32 x, int32 y );

void*		mgui_opengl_load_texture		( const char* path, uint32* width, uint32* height );
void		mgui_opengl_destroy_texture		( void* texture );
void		mgui_opengl_draw_textured_rect	( const void* texture, int32 x, int32 y, uint32 w, uint32 h,
											  float u1, float v1, float u2, float v2 );

void*		mgui_opengl_load_font			( const char* name, uint32 size, uint32 flags, uint32 charset,
											  uint32 firstc, uint32 lastc );

void		mgui_opengl_destroy_font		( void* font );

void		mgui_opengl_draw_text			( const void* font, const char_t* text, int32 x, int32 y,
											  uint32 flags, const MGuiFormatTag tags[], uint32 ntags );

void		mgui_opengl_measure_text		( const void* font, const char_t* text, uint32* w, uint32* h );

void		mgui_opengl_screen_pos_to_world	( const vector3_t* src, vector3_t* dst );
void		mgui_opengl_world_pos_to_screen	( const vector3_t* src, vector3_t* dst );

#endif /* __MYLLY_GUI_OPENGL_DRAWING_H */
