/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Text.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		A struct for text buffer and its properties.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "Text.h"
#include "Renderer.h"
#include "Skin.h"
#include "Stringy/Stringy.h"
#include "Platform/Alloc.h"
#include <stdio.h>
#include <assert.h>

extern MGuiRenderer* renderer;

static bool is_valid_colour_tag( const char* text );
static bool is_valid_uline_tag( const char* text );
static bool is_valid_end_tag( const char* text );

MGuiText* mgui_text_create( void )
{
	MGuiText* text;
	text = (MGuiText*)mem_alloc_clean( sizeof(*text) );

	text->alignment = ALIGN_CENTER;
	text->colour.hex = COL_TEXT;

	return text;
}

void mgui_text_destroy( MGuiText* text )
{
	if ( text == NULL ) return;

	if ( text->buffer )
		mem_free( text->buffer );

	mem_free( text );
}

void mgui_text_set_buffer( MGuiText* text, const char_t* fmt, ... )
{
	size_t	len;
	char_t	tmp[512];
	va_list	marker;

	if ( text == NULL ) return;

	if ( text->buffer )
		mem_free( text->buffer );

	va_start( marker, fmt );
	len = msnprintf( tmp, lengthof(tmp), fmt, marker );
	va_end( marker );

	len = math_max( len, text->bufsize );

	text->buffer = mstrdup( tmp, len );
	text->len = mstrlen( text->buffer );
	text->bufsize = len + 1;

	mgui_text_update_dimensions( text );
}

void mgui_text_set_buffer_s( MGuiText* text, const char_t* str )
{
	size_t len;

	if ( text == NULL ) return;

	if ( text->buffer )
		mem_free( text->buffer );

	len = mstrlen( str );
	len = math_max( len, text->bufsize );

	text->buffer = mstrdup( str, len );
	text->len = mstrlen( text->buffer );
	text->bufsize = len + 1;

	mgui_text_update_dimensions( text );
}

void mgui_text_set_buffer_va( MGuiText* text, const char_t* fmt, va_list list )
{
	size_t	len;
	char_t	tmp[512];

	if ( text == NULL ) return;

	if ( text->buffer )
		mem_free( text->buffer );

	len = msnprintf( tmp, lengthof(tmp), fmt, list );
	len = math_max( len, text->bufsize );

	text->buffer = mstrdup( tmp, len );
	text->len = mstrlen( text->buffer );
	text->bufsize = len + 1;

	mgui_text_update_dimensions( text );
}

void mgui_text_update_dimensions( MGuiText* text )
{
	uint32 w, h;

	if ( text == NULL ) return;

	renderer->measure_text( text->font->data, text->buffer, &w, &h );
	h -= 2;

	text->size.x = (uint16)w;
	text->size.y = (uint16)h;

	mgui_text_update_position( text );
}

void mgui_text_update_position( MGuiText* text )
{
	uint16 x, y, w, h;

	if ( text == NULL ) return;
	if ( text->bounds == NULL ) return;

	x = text->bounds->x + text->pad.left;
	y = text->bounds->y + text->pad.top - text->font->size;
	w = text->bounds->w - text->pad.left - text->pad.right;
	h = text->bounds->h - text->pad.top - text->pad.bottom;

	switch ( text->alignment )
	{
	case ALIGN_LEFT:
	case ALIGN_LEFT|ALIGN_CENTERV:
		text->pos.x = x;
		text->pos.y = y + text->size.y + ( h - text->size.y ) / 2;
		break;

	case ALIGN_LEFT|ALIGN_TOP:
		text->pos.x = x;
		text->pos.y = y + text->size.y;
		break;

	case ALIGN_TOP:
	case ALIGN_TOP|ALIGN_CENTERH:
		text->pos.x = x + ( w - text->size.x ) / 2;
		text->pos.y = y + text->size.y;
		break;

	case ALIGN_RIGHT|ALIGN_TOP:
		text->pos.x = x + w - text->size.x;
		text->pos.y = y + text->size.y;
		break;

	case ALIGN_RIGHT:
	case ALIGN_RIGHT|ALIGN_CENTERV:
		text->pos.x = x + w - text->size.x;
		text->pos.y = y + text->size.y + ( h - text->size.y ) / 2;
		break;

	case ALIGN_RIGHT|ALIGN_BOTTOM:
		text->pos.x = x + w - text->size.x;
		text->pos.y = y + h;
		break;

	case ALIGN_BOTTOM:
	case ALIGN_BOTTOM|ALIGN_CENTERH:
		text->pos.x = x + ( w - text->size.x ) / 2;
		text->pos.y = y + h;
		break;

	case ALIGN_LEFT|ALIGN_BOTTOM:
		text->pos.x = x;
		text->pos.y = y + h;
		break;

	case ALIGN_CENTER:
	case ALIGN_CENTERH:
	case ALIGN_CENTERV:
	default:
		text->pos.x = x + ( w - text->size.x ) / 2;
		text->pos.y = y + text->size.y + ( h - text->size.y ) / 2;
		break;
	}
}

uint32 mgui_text_get_closest_char( MGuiText* text, uint16 x, uint16 y )
{
	uint32 dist, ch, i, tmp;
	uint16 cx, cy;

	if ( text == NULL ) return 0;

	// We only calculate the dist along the x axis for now
	UNREFERENCED_PARAM( y );

	dist = 0xFFFF;
	ch = 0;

	assert( text != NULL );

	for ( i = 0; i < text->len + 1; i++ )
	{
		mgui_text_get_char_pos( text, i, &cx, &cy );
		tmp = math_abs( cx - x );

		if ( tmp < dist )
		{
			dist = tmp;
			ch = i;
		}
	}

	return ch;
}

void mgui_text_get_char_pos( MGuiText* text, uint32 idx, uint16* x, uint16* y )
{
	char_t tmp[512];
	uint32 w, h;

	if ( text == NULL ) return;

	if ( text->len == 0 || idx == 0 )
	{
		*x = 0;
		*y = text->font->size;
		return;
	}

	mstrcpy( tmp, text->buffer, math_min( idx+1, lengthof(tmp) ) );
	renderer->measure_text( text->font->data, tmp, &w, &h );

	*x = (uint16)w;
	*y = (uint16)h;
}

static bool is_valid_colour_tag( const char* text )
{
	register const char* s = text;
	size_t i;

	for ( i = 0; i < 6; s++, i++ )
	{
		if ( *s >= '0' && *s <= '9' ) continue;
		if ( *s >= 'a' && *s <= 'f' ) continue;
		return false;
	}

	if ( *s != ']' ) return false;

	return true;
}

static bool is_valid_uline_tag( const char* text )
{
	register const char* s = text;

	if ( *s++ != 'u' ) return false;
	if ( *s++ != 'l' ) return false;
	if ( *s++ != 'i' ) return false;
	if ( *s++ != 'n' ) return false;
	if ( *s++ != 'e' ) return false;
	if ( *s != ']' ) return false;

	return true;
}

static bool is_valid_end_tag( const char* text )
{
	register const char* s = text;

	if ( *s != 'd' && *s != 'u' ) return false;
	if ( *++s != ']' ) return false;

	return true;
}

void mgui_text_strip_format_tags( const char_t* text, char_t* buf, size_t buflen )
{
	const char_t* s = text;
	char* d = buf;
	size_t l = buflen;

	if ( text == NULL || buf == NULL ) return;

	for ( ; l && *s; --l )
	{
		if ( s[0] == '[' && s[1] == '#' )
		{
			// Colour tag
			if ( is_valid_colour_tag( s + 2 ) )
			{
				s += 9;
				continue;
			}

			// Underline tag
			if ( is_valid_uline_tag( s + 2 ) )
			{
				s += 8;
				continue;
			}

			// Format end tag
			if ( is_valid_end_tag( s + 2 ) )
			{
				s += 4;
				continue;
			}
		}

		*d++ = *s++;
	}

	*d = '\0';
}

void mgui_text_parse_format_tags( MGuiText* text )
{
	if ( text == NULL ) return;
}
