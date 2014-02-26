/**
 *
 * @file		Text.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		A structure for a text buffer and its properties.
 *
 * @details		Functions to manipulate MGUI strings.
 *
 **/

#include "Text.h"
#include "Renderer.h"
#include "Skin.h"
#include "Stringy/Stringy.h"
#include "Platform/Alloc.h"
#include <stdio.h>

// --------------------------------------------------

extern MGuiRenderer* renderer;

// --------------------------------------------------

static bool is_valid_colour_tag( const char* text );
static bool is_valid_uline_tag( const char* text );
static bool is_valid_end_tag( const char* text );
static bool parse_colour_tag( const char_t* text, uint32* in );
static bool parse_end_tag( const char_t* text, char_t* in );
static bool mgui_text_parse_tag( const char_t** ptext, MGuiFormatTag tags[], uint32* ntag, uint32* index, const colour_t* def );
static void mgui_text_parse_format_tags2( MGuiText* text, uint32 num_tags );

// --------------------------------------------------

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

	SAFE_DELETE( text->buffer );
	SAFE_DELETE( text->buffer_tags );

	mem_free( text );
}

static void mgui_text_update_buffers( MGuiText* text, const char_t* tmp, size_t len )
{
	uint32 tags;
	size_t size;

	if ( text == NULL ) return;

	text->len = len;

	// Do we need to reallocate memory for the new buffer?
	if ( len + 1 > text->bufsize )
	{
		// Delete old buffers
		SAFE_DELETE( text->buffer );
		SAFE_DELETE( text->buffer_tags );

		// Allocate new buffers
		if ( text->flags & TFLAG_TAGS )
		{
			size = len * sizeof(char_t) + 1;

			text->buffer = mem_alloc( size );
			text->buffer_tags = mstrdup( tmp, len );
			text->bufsize = len + 1;

			tags = mgui_text_strip_format_tags( text->buffer_tags, text->buffer, text->bufsize );
			mgui_text_parse_format_tags2( text, tags );

			text->len = mstrlen( text->buffer );
		}
		else
		{
			text->buffer = mstrdup( tmp, len );
			text->buffer_tags = NULL;
			text->bufsize = len + 1;
		}
	}
	else
	{
		// It's safe to use the old buffers
		if ( text->flags & TFLAG_TAGS )
		{
			mstrcpy( text->buffer_tags, tmp, text->bufsize );

			tags = mgui_text_strip_format_tags( text->buffer_tags, text->buffer, text->bufsize );
			mgui_text_parse_format_tags2( text, tags );

			text->len = mstrlen( text->buffer );
		}
		else
		{
			mstrcpy( text->buffer, tmp, text->bufsize );
		}
	}

	mgui_text_update_dimensions( text );
}

void mgui_text_set_buffer( MGuiText* text, const char_t* fmt, ... )
{
	va_list	marker;
	int32 len;
	char_t tmp[1024];
	
	if ( text == NULL ) return;

	va_start( marker, fmt );
	len = msnprintf( tmp, lengthof(tmp), fmt, marker );
	va_end( marker );

	if ( len < 0 ) len = mstrlen( tmp );

	mgui_text_update_buffers( text, tmp, len );
}

void mgui_text_set_buffer_s( MGuiText* text, const char_t* str )
{
	if ( text == NULL ) return;

	mgui_text_update_buffers( text, str, mstrlen( str ) );
}

void mgui_text_set_buffer_va( MGuiText* text, const char_t* fmt, va_list list )
{
	int32 len;
	char_t tmp[1024];

	if ( text == NULL ) return;

	len = msnprintf( tmp, lengthof(tmp), fmt, list );
	if ( len < 0 ) len = mstrlen( tmp );

	mgui_text_update_buffers( text, tmp, len );
}

void mgui_text_update_dimensions( MGuiText* text )
{
	uint32 w, h;

	if ( text == NULL ) return;
	if ( text->buffer == NULL ) return;

	renderer->measure_text( text->font->data, text->buffer, &w, &h );

	text->size.x = (uint16)w;
	text->size.y = (uint16)h;

	mgui_text_update_position( text );
}

void mgui_text_update_position( MGuiText* text )
{
	int16 x, y, w, h;

	if ( text == NULL || text->bounds == NULL )
		return;

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

void mgui_text_set_default_colour( MGuiText* text )
{
	uint32 ntag;
	MGuiFormatTag* tag;

	if ( text == NULL ) return;
	if ( text->tags == NULL || text->num_tags == 0 ) return;

	for ( ntag = 0; ntag < text->num_tags; ntag++ )
	{
		tag = &text->tags[ntag];

		if ( tag->flags & TAG_COLOUR_END )
			tag->colour = text->colour;

		else if ( tag->flags & TAG_COLOUR )
			tag->colour.a = text->colour.a;
	}
}

void mgui_text_measure_buffer( MGuiFont* font, const char_t* text, uint16* width, uint16* height )
{
	uint32 w, h;

	if ( width == NULL || height == NULL )
		return;

	if ( text == NULL || font == NULL || renderer == NULL )
	{
		*width = 0;
		*height = 0;
		return;
	}

	renderer->measure_text( font->data, text, &w, &h );

	*width = (uint16)w;
	*height = (uint16)h;
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

	if ( *s != 'd' && *s != 'u' && *s != 'e' ) return false;
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

	if ( *s != 'd' && *s != 'u' && *s != 'e' ) return false;
	if ( *++s != ']' ) return false;

	return true;
}

/**
 * @brief Strips format tags from a string.
 *
 * @details This function strips format tags from a text buffer
 * and returns the number of format tags found.
 *
 * @param text A pointer to a buffer that is to be stripped
 * @param buf A pointer to a buffer that will receive the stripped string
 * @param buflen The size of the buffer in characters
 * @returns The number of stripped tags
 */
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

static bool mgui_text_parse_tag( const char_t** ptext, MGuiFormatTag tags[], uint32* ntag, uint32* index, const colour_t* def )
{
	MGuiFormatTag* tag;
	uint32 hex;
	char_t c;
	register const char_t* s = *ptext;

	if ( s[0] != '[' || s[1] != '#' ) { return false; }

	// Colour tag
	if ( parse_colour_tag( s + 2 , &hex ) )
	{
		tag = &tags[*ntag];

		if ( tag->index != *index && tag->flags != TAG_NONE )
		{
			tag = &tags[++*ntag];
			tag->flags = TAG_NONE;
		}

		tag->index = (uint16)*index;
		tag->flags |= TAG_COLOUR;
		tag->colour.hex = hex;
		tag->colour.a = def->a;

		*ptext = s + 9;
		return true;
	}

	// Underline tag
	if ( is_valid_uline_tag( s + 2 ) )
	{
		tag = &tags[*ntag];

		if ( tag->index != *index && tag->flags != TAG_NONE )
		{
			tag = &tags[++*ntag];
			tag->flags = TAG_NONE;
		}
		
		tag->index = (uint16)*index;
		tag->flags |= TAG_UNDERLINE;

		*ptext = s + 8;
		return true;
	}

	// Format end tag
	if ( parse_end_tag( s + 2, &c ) )
	{
		tag = &tags[*ntag];

		if ( tag->index != *index && tag->flags != TAG_NONE )
		{
			tag = &tags[++*ntag];
			tag->flags = TAG_NONE;
		}

		tag->index = (uint16)*index;

		switch ( c )
		{
		case 'd':
			tag->flags &= ~TAG_COLOUR;
			tag->flags |= TAG_COLOUR_END;
			tag->colour = *def;
			break;

		case 'u':
			tag->flags &= ~TAG_UNDERLINE;
			tag->flags |= TAG_UNDERLINE_END;
			break;

		case 'e':
			tag->flags &= ~TAG_COLOUR;
			tag->flags |= TAG_COLOUR_END;
			tag->flags &= ~TAG_UNDERLINE;
			tag->flags |= TAG_UNDERLINE_END;

			tag->colour = *def;
			break;
		}

		*ptext = s + 4;
		return true;
	}

	return false;
}

void mgui_text_parse_format_tags( const char_t* text, const colour_t* def, MGuiFormatTag* tags, uint32 ntags )
{
	const char* s;
	size_t l, i = 0, ntag = 0;

	if ( text == NULL || def == NULL || tags == NULL ) return;

	s = text;
	l = ntags;

	while ( l && *s )
	{
		if ( mgui_text_parse_tag( &s, tags, &ntag, &i, def ) )
		{
			--l;
			continue;
		}

		++s;
		++i;
	}
}

static void mgui_text_parse_format_tags2( MGuiText* text, uint32 num_tags )
{
	if ( text == NULL ) return;
	if ( text->buffer_tags == NULL ) return;

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

	mgui_text_parse_format_tags( text->buffer_tags, &text->colour, text->tags, num_tags );
}

uint32 mgui_text_parse_and_get_line( const char_t* text, MGuiFont* font, const colour_t* def, uint32 max_width, char_t** buf_in, MGuiFormatTag** tags_in )
{
	uint32 width = 0, w, h, i = 0, flags;
	uint32 space = 0, ntag = 0, len = 0, index = 0;
	bool has_tags = false;
	const char_t *s, *last_space = NULL;
	char_t *t, tmp[2];

	static uint32 pad = 0;
	static char_t tmpbuf[1024];
	static MGuiFormatTag tmptags[32], prev_tag;
	static const char_t* ptr = NULL;

	if ( font == NULL || buf_in == NULL )
		goto cleanup;

	if ( max_width == 0 )
		max_width = (uint32)-1;

	if ( text == NULL )
	{
		// Continue processing an old string.
		s = ptr;

		if ( ptr == NULL ) goto cleanup;
		if ( prev_tag.index < (uint16)-1 )
		{
			tmptags[0].colour.hex = prev_tag.colour.hex;
			tmptags[0].flags = prev_tag.flags;
			tmptags[0].index = 0;

			has_tags = true;
		}
	}
	else
	{
		// This is a new string. Start from the beginning.
		ptr = NULL;
		s = text;

		tmptags[0].index = 0;
		tmptags[0].flags = TAG_NONE;
		tmptags[0].colour = *def;

		prev_tag.index = (uint16)-1;
		prev_tag.flags = 0;
		prev_tag.colour.hex = 0;

		// Measure padding between two characters.
		renderer->measure_text( font->data, _MTEXT("XX"), &pad, &h );
		renderer->measure_text( font->data, _MTEXT("X"), &w, &h );

		pad -= 2 * w;
	}	

	tmpbuf[0] = tmp[1] = '\0';
	t = tmpbuf;

	for ( ; *s; )
	{
		if ( *s == ' ' )
		{
			// Remove leading spaces
			if ( len == 0 )
			{
				while ( *s == ' ' ) ++s;
			}

			space = len;
			last_space = s;
		}

		// Parse and remove format tags. This will store the result into the temp buffer automatically.
		if ( tags_in && mgui_text_parse_tag( &s, tmptags, &ntag, &index, def ) )
		{
			has_tags = true;
			continue;
		}

		// Measure the width of the current character and add it to the total line width.
		tmp[0] = *s;

		renderer->measure_text( font->data, tmp, &w, &h );
		width += w + pad;

		// Do we have anough text for a new line?
		if ( width > max_width || *s == '\n' )
		{
			break;
		}

		*t++ = *s++;
		len++;
		index++;
	}

	if ( *s ==  '\n' )
	{
		// Ignore line breaks.
		++s;
	}
	else if ( space >= 5 && *s != '\0' )
	{
		// If the new line is less than 5 characters, ignore spacing.
		len = space;
		s = last_space ? last_space : s;
	}

	*buf_in = mstrdup( tmpbuf, len );
	ptr = *s ? s : NULL;

	// Allocate permanent memory for tags and copy the tags
	if ( tags_in && has_tags )
	{
		*tags_in = mem_alloc( ( ntag + 1 ) * sizeof(MGuiFormatTag) );

		for ( i = 0; i < ntag+1; ++i )
		{
			(*tags_in)[i] = tmptags[i];
		}
	}

	// Remember the last tag, just in case we continue processing this string 
	if ( has_tags )
	{
		flags = tmptags[ntag].flags;
		prev_tag.index = tmptags[ntag].index;

		if ( flags & TAG_COLOUR )
		{
			prev_tag.flags |= TAG_COLOUR;
			prev_tag.flags &= ~TAG_COLOUR_END;
			prev_tag.colour.hex = tmptags[ntag].colour.hex;
		}
		else if ( flags & TAG_COLOUR_END )
		{
			prev_tag.flags &= ~TAG_COLOUR;
			prev_tag.flags |= TAG_COLOUR_END;
			prev_tag.colour.hex = tmptags[ntag].colour.hex;
		}

		if ( flags & TAG_UNDERLINE )
		{
			prev_tag.flags |= TAG_UNDERLINE;
			prev_tag.flags &= ~TAG_UNDERLINE_END;
		}
		else if ( flags & TAG_UNDERLINE_END )
		{
			prev_tag.flags &= ~TAG_UNDERLINE;
			prev_tag.flags |= TAG_UNDERLINE_END;
		}
	}

	return ntag + 1;

cleanup:
	*buf_in = NULL;
	return 0;
}
