/**********************************************************************
 *
 * PROJECT:		Mylly GUI - Generic Windows font loader
 * FILE:		FontLoader.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		Shared code for MGUI renderer font loading on Windows.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "FontLoader.h"
#include "Platform/Alloc.h"
#include <math.h>
#include <windef.h>

// --------------------------------------------------

static void		mgui_font_copy_to_texture				( void* src, void* dest, uint32 width, uint32 height, int32 texWidth );
static void		mgui_font_copy_to_texture_with_outline	( void* src, void* dest, uint32 width, uint32 height, int32 texWidth );
static bool		mgui_font_measure_and_print				( HDC tmpDC, bool print, uint8 flags, uint32 first_char, uint32 last_char, MGuiFontInfo* info );

// --------------------------------------------------

bool mgui_load_font( const char* name, uint8 size, uint8 flags, uint8 charset, uint32 firstc, uint32 lastc,
					 MGuiFontInfo* info, create_texture_t callback, void* callback_data )
{
	HFONT old_font, tmp_font;
	void *bitmap_bits, *tex_bits = NULL;
	BITMAPINFO bitmap_info;
	HBITMAP bitmap;
	HGDIOBJ old_bitmap;
	HDC tmp_dc;
	bool retn = false;
	uint32 i, width, height,
		tex_data_len, tex_pitch;

	if ( info == NULL ||
		 callback == NULL ||
		 callback_data == NULL )
		 return retn;

	memset( info, 0, sizeof(*info) );

	tmp_dc = CreateCompatibleDC( NULL );
	SetMapMode( tmp_dc, MM_TEXT );

	tmp_font = CreateFont( -size, 0, 0, 0,
		flags & FFLAG_BOLD ? FW_BOLD : FW_NORMAL,
		flags & FFLAG_ITALIC ? TRUE : FALSE,
		flags & FFLAG_ULINE ? TRUE : FALSE,
		flags & FFLAG_STRIKE ? TRUE : FALSE,
		charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		flags & FFLAG_NOAA ? NONANTIALIASED_QUALITY : CLEARTYPE_QUALITY,
		VARIABLE_PITCH, name );

	if ( tmp_font == NULL )
		return retn;

	tex_data_len = lastc - firstc;
	width = 128;
	height = 128;

	old_font = (HFONT)SelectObject( tmp_dc, tmp_font );

	// Find out a sufficient size for our texture bitmap (must be a power of 2)
	while ( !mgui_font_measure_and_print( tmp_dc, false, flags, firstc, lastc, info ) )
	{
		width <<= 1; info->width = width;
		height <<= 1; info->height = height;
	}

	// Allocate space for character texture coordinates
	info->tex_coords = mem_alloc( tex_data_len * sizeof(float*) );
	info->tex_coords[0] = mem_alloc( tex_data_len * sizeof(float) * 4 );

	for ( i = 1; i < tex_data_len; i++ )
	{
		info->tex_coords[i] = info->tex_coords[0] + i * 4;
	}

	ZeroMemory( &bitmap_info.bmiHeader, sizeof(BITMAPINFOHEADER) );

	bitmap_info.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bitmap_info.bmiHeader.biWidth		= (int)width;
	bitmap_info.bmiHeader.biHeight		= (int)height;
	bitmap_info.bmiHeader.biPlanes		= 1;
	bitmap_info.bmiHeader.biCompression	= BI_RGB;
	bitmap_info.bmiHeader.biBitCount	= 32;

	bitmap = CreateDIBSection( tmp_dc, &bitmap_info, DIB_RGB_COLORS, &bitmap_bits, NULL, 0 );
	if ( !bitmap ) return false;

	old_bitmap = SelectObject( tmp_dc, bitmap );

	SetTextColor( tmp_dc, 0x00FFFFFF );
	SetBkColor( tmp_dc, 0x00000000 );
	SetTextAlign( tmp_dc, TA_TOP );

	// Paint the alphabet onto the selected bitmap
	mgui_font_measure_and_print( tmp_dc, true, flags, firstc, lastc, info );

	// Request a new texture from the renderer.
	callback( callback_data, width, height, &tex_bits, &tex_pitch );

	if ( tex_bits == NULL )
		goto cleanup;

	// Finally transfer the font glyphs to the texture
	if ( flags & FFLAG_OUTLINE )
		mgui_font_copy_to_texture_with_outline( bitmap_bits, tex_bits, width, height, tex_pitch );
	else
		mgui_font_copy_to_texture( bitmap_bits, tex_bits, width, height, tex_pitch );

	retn = true;

cleanup:
	if ( old_bitmap ) SelectObject( tmp_dc, old_bitmap );
	if ( old_font ) SelectObject( tmp_dc, old_font );
	if ( bitmap ) DeleteObject( bitmap );
	if ( tmp_font ) DeleteObject( tmp_font );
	if ( tmp_dc ) DeleteDC( tmp_dc );

	return retn;
}

static void mgui_font_copy_to_texture( void* src, void* dest, uint32 width, uint32 height, int32 texWidth )
{
	uint8 *dstRow, alpha;
	uint16* dst;
	uint32 x, y, *pixels;

	dstRow = (uint8*)dest;

	for ( y = 0; y < height; y++ )
	{
		dst = (uint16*)dstRow;
		pixels = (uint32*)src + width * y;

		for ( x = 0; x < width; x += 4 )
		{
			alpha = (uint8)( ( *pixels++ >> 8 ) & 0xFF );
			*dst++ = (uint16)( alpha << 8 ) | 0x0FFF;

			alpha = (uint8)( ( *pixels++ >> 8 ) & 0xFF );
			*dst++ = (uint16)( alpha << 8 ) | 0x0FFF;

			alpha = (uint8)( ( *pixels++ >> 8 ) & 0xFF );
			*dst++ = (uint16)( alpha << 8 ) | 0x0FFF;

			alpha = (uint8)( ( *pixels++ >> 8 ) & 0xFF );
			*dst++ = (uint16)( alpha << 8 ) | 0x0FFF;
		}

		dstRow += texWidth;
	}
}

static void mgui_font_copy_to_texture_with_outline( void* src, void* dest, uint32 width, uint32 height, int32 texWidth )
{
	uint8* dstRow, alphaMask, shadeMask;
	uint16* dst;
	int32 x, y, x2, y2;
	uint32* pixels;
	float alpha, shade;

	pixels = (uint32*)src;
	dstRow = (uint8*)dest;

	for ( y = 0; y < (int32)height; y++ )
	{
		dst = (uint16*)dstRow;

		for ( x = 0; x < (int32)width; x++ )
		{
			shade = alpha = (float)( pixels[x+y*width] & 0xFF ) / 0xFF;

			for ( x2 = x - 1; x2 < x + 2; x2++ )
			{
				for ( y2 = y - 1; y2 < y + 2; y2++ )
				{
					if ( x2 == x && y2 == y ) continue;
					if ( x2 < 0 || x2 >= (int32)width || y2 < 0 || y2 >= (int32)height ) continue;

					alpha += (float)( pixels[x2+y2*width] & 0xFF ) / 0xFF;
				}
			}

			alpha = alpha > 0 ? 1.0f : 0.0f;

			shadeMask = (uint8)( 0xF * shade ) & 0xF;
			alphaMask = (uint8)( 0xF * alpha ) & 0xF;

			*dst++ = (uint16)( ( alphaMask << 12 ) | ( shadeMask << 8 ) | ( shadeMask << 4 ) | shadeMask );
		}

		dstRow += texWidth;
	}
}

static bool mgui_font_measure_and_print( HDC tmpDC, bool print, uint8 flags, uint32 first_char, uint32 last_char, MGuiFontInfo* info )
{
	SIZE size;
	uint32 x, y, c, idx, padding;
	char_t tmpBuf[] = " ";

	if ( info->spacing == 0 )
	{
		GetTextExtentPoint32( tmpDC, tmpBuf, 1, &size );
		info->spacing = (uint8)ceil( size.cx / 4.0f );
	}

	padding = ( flags & FFLAG_OUTLINE ) ? 1 : 0;

	x = padding + info->spacing;
	y = 2 * padding;

	for ( c = first_char; c < last_char; c++ )
	{
		*tmpBuf = (char_t)c;
		GetTextExtentPoint32( tmpDC, tmpBuf, 1, &size );

		if ( x + size.cx + info->spacing + padding > info->width )
		{
			x = info->spacing;
			y += size.cy + 2 * padding + 1;
		}

		// Check to see if there's room to write the character here
		if ( y + size.cy + 2 * padding > info->height ) return false;

		if ( print )
		{
			// Print the character
			ExtTextOut( tmpDC, x, y, ETO_CLIPPED|ETO_OPAQUE, NULL, tmpBuf, 1, NULL );

			idx = c - first_char;
			info->tex_coords[idx][0] = (float)( x - info->spacing ) / info->width;
			info->tex_coords[idx][1] = (float)( y - padding ) / info->height;
			info->tex_coords[idx][2] = (float)( x + size.cx + padding + info->spacing ) / info->width;
			info->tex_coords[idx][3] = (float)( y + size.cy + padding ) / info->height;
		}

		x += size.cx + 2 * ( padding + info->spacing );
	}

	return true;
}
