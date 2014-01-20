/**
 *
 * @file		Text.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		A structure for a text buffer and its properties.
 *
 * @details		Functions to manipulate MGUI strings.
 *
 **/

#pragma once
#ifndef __MGUI_TEXT_H
#define __MGUI_TEXT_H

#include "MGUI.h"
#include "Font.h"
#include "Renderer.h"
#include <stdarg.h>

typedef struct MGuiText
{
	char_t*			buffer;			// Text buffer for the actual text
	char_t*			buffer_tags;	// Text buffer with tags still enabled
	size_t			len;			// The length of the buffer (in characters)
	size_t			len_tags;		// The length of the buffer with tags (in characters)
	size_t			bufsize;		// Length of allocated buffer (in characters)
	vectorscreen_t	pos;			// Rendering position
	vectorscreen_t	size;			// Size of the text (width, height)
	rectangle_t*	bounds;			// Containing boundaries
	uint16			flags;			// Text flags (see enum above)
	uint16			alignment;		// Alignment flags
	MGuiFont*		font;			// Font used for rendering
	colour_t		colour;			// Default text colour
	MGuiFormatTag*	tags;			// Text format tag array or NULL if text has format tags disabled
	uint32			num_tags;		// Number of colour tags in the array

	struct { uint8 top, bottom, left, right; } pad;	// Text padding 
} MGuiText;

// Helper functions
MGuiText*	mgui_text_create				( void );
void		mgui_text_destroy				( MGuiText* text );

void		mgui_text_set_buffer			( MGuiText* text, const char_t* fmt, ... );
void		mgui_text_set_buffer_s			( MGuiText* text, const char_t* str );
void		mgui_text_set_buffer_va			( MGuiText* text, const char_t* fmt, va_list list );

void		mgui_text_update_dimensions		( MGuiText* text );
void		mgui_text_update_position		( MGuiText* text );

uint32		mgui_text_get_closest_char		( MGuiText* text, uint16 x, uint16 y );
void		mgui_text_get_char_pos			( MGuiText* text, uint32 idx, uint16* x, uint16* y );

void		mgui_text_set_default_colour	( MGuiText* text );

void		mgui_text_measure_buffer		( MGuiFont* font, const char_t* text, uint16* width, uint16* height );

uint32		mgui_text_strip_format_tags		( const char_t* text, char_t* buf, size_t buflen );
void		mgui_text_parse_format_tags		( const char_t* text, const colour_t* def, MGuiFormatTag* tags, uint32 ntags );
uint32		mgui_text_parse_and_get_line	( const char_t* text, MGuiFont* font, const colour_t* def, uint32 max_width, char_t** buf_in, MGuiFormatTag** tags_in );

#endif /* __MGUI_TEXT_H */
