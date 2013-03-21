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

#include "MGUI.h"

typedef struct MGuiRenderer
{
	MGUI_RENDERER	type;
	void*			device_context;

	void		( *begin )				( void );
	void		( *end )				( void );
	void		( *resize )				( uint w, uint h );

	void		( *set_draw_colour )	( const colour_t* col );
	void		( *start_clip )			( uint x, uint y, uint w, uint h );
	void		( *end_clip )			( void );

	void		( *draw_rect )			( uint x, uint y, uint w, uint h );
	void		( *draw_triangle )		( uint x1, uint y1, uint x2, uint y2, uint x3, uint y3 );

	void*		( *load_texture )		( const char_t* path );
	void		( *destroy_texture )	( void* texture );
	void		( *draw_textured_rect )	( void* texture, uint x, uint y, uint w, uint h );

	void*		( *load_font )			( const char_t* font, uint size, uint flags, uint charset, uint firstc, uint lastc );
	void		( *destroy_font )		( void* font );
	void		( *draw_text )			( void* font, const char_t* text, uint x, uint y, uint flags );
	void		( *measure_text )		( void* font, const char_t* text, uint* w, uint* h );
} MGuiRenderer;


void		mgui_render_initialize		( MGUI_RENDERER type, void* syswindow );
void		mgui_render_shutdown		( void );

// Pointer to the current renderer
extern	MGuiRenderer* render;

// Redraw system
// TODO: Move this the hell out of here
#define MGUI_USE_REDRAW

#ifdef MGUI_USE_REDRAW
	#define mgui_force_redraw mgui_redraw
	extern	bool redraw;
#else
	#define mgui_force_redraw
#endif



#endif /* __MYLLY_GUI_RENDERER_H */
