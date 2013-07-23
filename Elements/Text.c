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
	size_t len, size;
	char_t tmp[1024];
	va_list	marker;
	uint32 tags;

	if ( text == NULL ) return;

	va_start( marker, fmt );
	len = msnprintf( tmp, lengthof(tmp), fmt, marker );
	va_end( marker );

	// Do we need to reallocate memory for the new buffer?
	if ( len + 1 > text->bufsize )
	{
		// delete old buffers
		if ( text->buffer )
			mem_free( text->buffer );

		if ( text->buffer_tags )
			mem_free( text->buffer_tags );

		// Allocate new buffers
		if ( text->flags & TFLAG_TAGS )
		{
			size = len * sizeof(char_t) + 1;

			text->buffer = mem_alloc( size );
			text->buffer_tags = mstrdup( tmp, len );
			text->bufsize = len + 1;

			tags = mgui_text_strip_format_tags( text->buffer_tags, text->buffer, size );
			mgui_text_parse_format_tags( text, tags );

			text->len = mstrlen( text->buffer );
		}
		else
		{
			text->buffer = mstrdup( tmp, len );
			text->buffer_tags = NULL;
			text->len = len;
			text->bufsize = len + 1;
		}
	}
	else
	{
		// TODO:
	}

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
	size_t len;
	char_t tmp[1024];

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

static bool parse_colour_tag( const char_t* text, uint32* in )
{
	register const char* s = text;
	uint32 order = 6;
	uint32 multiplier = 1 << 28;

	*in = 0;

	for ( ; order; ++s, --order )
	{
		if ( *s >= '0' && *s <= '9' )
		{
			*in += ( *s - '0' ) * multiplier;

			multiplier >>= 4;
			continue;
		}
		if ( *s >= 'a' && *s <= 'f' )
		{
			*in += ( *s - 'a' + 0xA ) * multiplier;

			multiplier >>= 4;
			continue;
		}

		return false;
	}

	if ( *s != ']' ) return false;

	return true;
}

static bool parse_end_tag( const char_t* text, char_t* in )
{
	register const char* s = text;

	*in = *s;

	if ( *s != 'd' && *s != 'u' ) return false;
	if ( *++s != ']' ) return false;

	return true;
}

uint32 mgui_text_strip_format_tags( const char_t* text, char_t* buf, size_t buflen )
{
	const char_t* s = text;
	char* d = buf;
	size_t l = buflen;
	uint32 tags = 0;

	if ( text == NULL || buf == NULL ) return 0;

	for ( ; l && *s; --l )
	{
		if ( s[0] == '[' && s[1] == '#' )
		{
			// Colour tag
			if ( is_valid_colour_tag( s + 2 ) )
			{
				s += 9; tags++;
				continue;
			}

			// Underline tag
			if ( is_valid_uline_tag( s + 2 ) )
			{
				s += 8; tags++;
				continue;
			}

			// Format end tag
			if ( is_valid_end_tag( s + 2 ) )
			{
				s += 4; tags++;
				continue;
			}
		}

		*d++ = *s++;
	}

	*d = '\0';
	return tags;
}

void mgui_text_parse_format_tags( MGuiText* text, uint32 num_tags )
{
	const char* s;
	size_t l, i = 0, ntag = 0;
	uint32 hex;
	char_t c;
	MGuiFormatTag* tag;

	if ( text == NULL || text->buffer_tags == NULL ) return;

	// Allocate space for format tags if required
	if ( text->num_tags < num_tags )
	{
		if ( text->tags )
		{
			mem_free( text->tags );
		}

		text->tags = mem_alloc_clean( sizeof(MGuiFormatTag) * num_tags );
		text->num_tags = num_tags;
	}

	s = text->buffer_tags;
	l = text->len_tags;

	for ( ; l && *s; --l )
	{
		if ( s[0] == '[' && s[1] == '#' )
		{
			// Colour tag
			if ( parse_colour_tag( s + 2 , &hex ) )
			{
				tag = &text->tags[ntag];

				if ( tag->index == i )
				{
					tag->flags |= TAG_COLOUR;
					tag->colour.hex = hex;
					tag->colour.a = text->colour.a;
				}
				else
				{
					tag = &text->tags[++ntag];

					tag->index = (uint16)i;
					tag->flags = TAG_COLOUR;
					tag->colour.hex = hex;
					tag->colour.a = text->colour.a;
				}

				s += 9;
				continue;
			}

			// Underline tag
			if ( is_valid_uline_tag( s + 2 ) )
			{
				tag = &text->tags[ntag];

				if ( tag->index == i )
				{
					tag->flags |= TAG_UNDERLINE;
				}
				else
				{
					tag = &text->tags[++ntag];
					tag->index = (uint16)i;
					tag->flags = TAG_UNDERLINE;
				}

				s += 8;
				continue;
			}

			// Format end tag
			if ( parse_end_tag( s + 2, &c ) )
			{
				tag = &text->tags[ntag];

				if ( tag->index == i )
				{
					switch ( c )
					{
					case 'd':
						tag->flags &= ~TAG_COLOUR;
						tag->flags |= TAG_COLOUR_END;
						break;
					case 'u':
						tag->flags &= ~TAG_UNDERLINE;
						tag->flags |= TAG_UNDERLINE_END;
						break;
					}
				}
				else
				{
					tag = &text->tags[++ntag];
					tag->index = (uint16)i;
					
					switch ( c )
					{
					case 'd':
						tag->flags = TAG_COLOUR_END;
						break;
					case 'u':
						tag->flags = TAG_UNDERLINE_END;
						break;
					}
				}

				s += 4;
				continue;
			}
		}

		i++;
	}
}
