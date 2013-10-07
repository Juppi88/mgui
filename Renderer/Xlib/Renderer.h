/**********************************************************************
 *
 * PROJECT:		Mylly GUI - Xlib Renderer
 * FILE:		Renderer.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		An Xlib reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_XLIB_RENDERER_H
#define __MYLLY_GUI_XLIB_RENDERER_H

#include "MGUI/Renderer/Renderer.h"

// Used for drawing an underline for text (format tags)
typedef enum {
	LINE_IDLE,
	LINE_BEGIN,
	LINE_DRAWING,
	LINE_DRAW,
} LINE_STATUS;

void				renderer_begin						( void );
void				renderer_end						( void );
void				renderer_resize						( uint32 width, uint32 height );

DRAW_MODE			renderer_set_draw_mode				( DRAW_MODE mode );
void				renderer_set_draw_colour			( const colour_t* col );
void				renderer_set_draw_depth				( float z_depth );
void				renderer_set_draw_transform			( const matrix4_t* mat );	
void				renderer_reset_draw_transform		( void );

void				renderer_start_clip					( int32 x, int32 y, uint32 w, uint32 h );
void				renderer_end_clip					( void );

void				renderer_draw_rect					( int32 x, int32 y, uint32 w, uint32 h );
void				renderer_draw_triangle				( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 );
void				renderer_draw_pixel					( int32 x, int32 y );

MGuiRendTexture*	renderer_load_texture				( const char_t* path, uint32* width, uint32* height );
void				renderer_destroy_texture			( MGuiRendTexture* texture );
void				renderer_draw_textured_rect			( const MGuiRendTexture* texture, int32 x, int32 y, uint32 w, uint32 h, const float uv[] );

MGuiRendFont*		renderer_load_font					( const char_t* font, uint8 size, uint8 flags, uint8 charset,
														  uint32 firstc, uint32 lastc );

void				renderer_destroy_font				( MGuiRendFont* font );

void				renderer_draw_text					( const MGuiRendFont* font, const char_t* text, int32 x, int32 y,
														  uint32 flags, const MGuiFormatTag tags[], uint32 ntags );

void				renderer_measure_text				( const MGuiRendFont* font, const char_t* text, uint32* w, uint32* h );

MGuiRendTarget*		renderer_create_render_target		( uint32 width, uint32 height );
void				renderer_destroy_render_target		( MGuiRendTarget* target );
void				renderer_draw_render_target			( const MGuiRendTarget* target, int32 x, int32 y, uint32 w, uint32 h );
void				renderer_enable_render_target		( const MGuiRendTarget* target, int32 x, int32 y );
void				renderer_disable_render_target		( const MGuiRendTarget* target );

void				renderer_screen_pos_to_world		( const vector3_t* src, vector3_t* dst );
void				renderer_world_pos_to_screen		( const vector3_t* src, vector3_t* dst );

#endif /* __MYLLY_GUI_XLIB_RENDERER_H */
