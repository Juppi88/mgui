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
#include "Math/Matrix4.h"

typedef enum {
	DRAWING_2D,		// Draw 2D entities on top of everything else
	DRAWING_2DZ,	// Draw 2D entities, set the Z index
	DRAWING_3D,		// Draw 3D entities (use with set_draw_transform)
} DRAW_MODE;

enum {
	TAG_NONE			= 0,		// No special characteristics
	TAG_COLOUR			= 1 << 0,	// Tag specifies a custom colour
	TAG_COLOUR_END		= 1 << 1,	// Tag returns to default colour
	TAG_UNDERLINE		= 1 << 2,	// Enable underlining
	TAG_UNDERLINE_END	= 1 << 3,	// Enable underlining
};

typedef struct MGuiFormatTag
{
	uint16		index;		// Text buffer start index
	uint16		flags;		// Tag flags (see enum above)
	colour_t	colour;		// The colour that should be used with this tag
} MGuiFormatTag;

struct MGuiRenderer
{
	void		( *begin )					( void );
	void		( *end )					( void );
	void		( *resize )					( uint32 w, uint32 h );

	void		( *set_draw_mode )			( DRAW_MODE mode );
	void		( *set_draw_colour )		( const colour_t* col );
	void		( *set_draw_transform )		( const matrix4_t* mat );
	void		( *reset_draw_transform )	( void );

	void		( *start_clip )				( int32 x, int32 y, uint32 w, uint32 h );
	void		( *end_clip )				( void );

	void		( *draw_rect )				( int32 x, int32 y, uint32 w, uint32 h, float z_index );
	void		( *draw_triangle )			( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, float z_index );

	void*		( *load_texture )			( const char_t* path );
	void		( *destroy_texture )		( void* texture );
	void		( *draw_textured_rect )		( const void* texture, int32 x, int32 y, uint32 w, uint32 h, float z_index );

	void*		( *load_font )				( const char_t* font, uint32 size, uint32 flags, uint32 charset,
											  uint32 firstc, uint32 lastc );

	void		( *destroy_font )			( void* font );

	void		( *draw_text )				( const void* font, const char_t* text, int32 x, int32 y, float z_index,
											  uint32 flags, const MGuiFormatTag tags[], uint32 ntags );

	void		( *measure_text )			( const void* font, const char_t* text, uint32* w, uint32* h );
};

#endif /* __MYLLY_GUI_RENDERER_H */
