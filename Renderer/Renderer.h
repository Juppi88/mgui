/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Renderer.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		An interface for the GUI renderer.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_RENDERER_H
#define __MYLLY_GUI_RENDERER_H

#include "MGUI/MGUI.h"

struct MGuiRenderer
{
	void*		device_context;

	void		( *begin )				( void );
	void		( *end )				( void );
	void		( *resize )				( uint32 w, uint32 h );

	void		( *set_draw_colour )	( const colour_t* col );
	void		( *start_clip )			( int32 x, int32 y, uint32 w, uint32 h );
	void		( *end_clip )			( void );

	void		( *draw_rect )			( int32 x, int32 y, uint32 w, uint32 h );
	void		( *draw_triangle )		( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 );

	void*		( *load_texture )		( const char_t* path );
	void		( *destroy_texture )	( void* texture );
	void		( *draw_textured_rect )	( void* texture, int32 x, int32 y, uint32 w, uint32 h );

	void*		( *load_font )			( const char_t* font, uint32 size, uint32 flags, uint32 charset, uint32 firstc, uint32 lastc );
	void		( *destroy_font )		( void* font );
	void		( *draw_text )			( void* font, const char_t* text, int32 x, int32 y, uint32 flags );
	void		( *measure_text )		( void* font, const char_t* text, uint32* w, uint32* h );
};


// Redraw system
// TODO: Move this the hell out of here
//#define MGUI_USE_REDRAW

#ifdef MGUI_USE_REDRAW
	#define mgui_force_redraw mgui_redraw
	extern	bool redraw;
#else
	#define mgui_force_redraw()
#endif

#endif /* __MYLLY_GUI_RENDERER_H */
