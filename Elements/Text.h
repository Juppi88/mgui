/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Text.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A struct for text buffer and its properties.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_TEXT_H
#define __MGUI_TEXT_H

#include "MGUI.h"
#include "Font.h"
#include <stdarg.h>

typedef struct  
{
	char_t*					buffer;		// Text buffer for the actual text
	size_t					len;		// The length of the buffer (in characters)
	size_t					bufsize;	// Length of allocated buffer (in characters)
	vectorscreen_t			pos;		// Rendering position
	vectorscreen_t			size;		// Size of the text (width, height)
	rectangle_t*			bounds;		// Containing boundaries
	uint32					alignment;	// Alignment flags
	font_t*					font;		// Font used for rendering
	colour_t				colour;		// Rendering colour

	struct 
	{
		uint8				top;
		uint8				bottom;
		uint8				left;
		uint8				right;
	} pad;								// Text padding 
} text_t;

// Helper functions
text_t*		mgui_text_create			( void );
void		mgui_text_destroy			( text_t* text );

void		mgui_text_set_buffer		( text_t* text, const char_t* fmt, ... );
void		mgui_text_set_buffer_s		( text_t* text, const char_t* str );
void		mgui_text_set_buffer_va		( text_t* text, const char_t* fmt, va_list list );

void		mgui_text_update_dimensions	( text_t* text );
void		mgui_text_update_position	( text_t* text );

uint32		mgui_text_get_closest_char	( text_t* text, uint16 x, uint16 y );
void		mgui_text_get_char_pos		( text_t* text, uint32 idx, uint16* x, uint16* y );

#endif /* __MGUI_TEXT_H */
