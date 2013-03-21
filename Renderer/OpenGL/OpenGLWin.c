/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (OpenGL)
 * FILE:		OpenGLWin.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An OpenGL renderer for Mylly GUI.
 *				Windows related functions
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#ifdef _WIN32

#include "OpenGL.h"
#include "Platform/Alloc.h"
#include "Platform/Window.h"
#include <malloc.h>
#include <assert.h>
#include <math.h>

HWND hwnd;
HDC dc;

void* mgui_opengl_create_context( void* syswindow )
{
	PIXELFORMATDESCRIPTOR pfd;
	int format;
	RECT rect;
	HGLRC context;

	hwnd = (HWND)syswindow;
	dc = GetDC(hwnd);

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = (PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER|PFD_DRAW_TO_WINDOW);
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 32;

	format = ChoosePixelFormat( dc, &pfd );

	if ( format == 0 )
	{
		return NULL;
	}    

	SetPixelFormat( dc, format, &pfd );

	context = wglCreateContext( dc );

	wglMakeCurrent( dc, context );

	if ( !GetClientRect( hwnd, &rect ) ) return context;

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( rect.left, rect.right, rect.bottom, rect.top, -1.0, 1.0 );
	glMatrixMode( GL_MODELVIEW );
	glViewport( 0, 0, rect.right - rect.left, rect.bottom - rect.top );

	return context;
}

void mgui_opengl_destroy_context( void* context )
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( context );

	dc = NULL;
}

void mgui_opengl_swap_buffers( void )
{
	SwapBuffers( dc );
}

static bool __measure_font( HDC tmpdc, MGuiGLFont* font, bool print )
{
	SIZE size;
	uint32 x, y;
	uint32 c, idx;
	char_t tmp[2] = _MTEXT(" ");

	GetTextExtentPoint32( tmpdc, tmp, 1, &size );

	font->spacing = (int32)ceil( size.cy * 0.5f );

	x = font->spacing;
	y = 0;

	for ( c = font->first_char; c <= font->last_char; c++ )
	{
		tmp[0] = (char_t)c;
		GetTextExtentPoint32( tmpdc, tmp, 1, &size );

		if ( x + size.cx + font->spacing > font->width )
		{
			x = font->spacing;
			y += size.cy + 1;
		}

		// Check to see if there's room to write the character here
		if ( y + size.cy > font->height ) return false;

		if ( print )
		{
			// Print the character
			ExtTextOut( tmpdc, x, y, ETO_OPAQUE, NULL, tmp, 1, NULL );

			idx = c - font->first_char;
			font->coords[idx][0] = (float)x / font->width;
			font->coords[idx][1] = (float)y / font->height;
			font->coords[idx][2] = (float)( x + size.cx ) / font->width;
			font->coords[idx][3] = (float)( y + size.cy ) / font->height;
		}

		x += size.cx + ( 2 * font->spacing );
	}

	return true;
}

void* mgui_opengl_load_font( const char* name, uint32 size, uint32 flags, uint32 charset, uint32 firstc, uint32 lastc )
{
	HFONT newfnt, oldfnt;
	DWORD* bm_bits;
	BITMAPINFO bmi;
	HBITMAP bitmap;
	HGDIOBJ bmold;
	BITMAP bm;
	MGuiGLFont* font;
	HDC tmpdc;
	uint32 i, *pixels;
	float ratio;

	tmpdc = CreateCompatibleDC( NULL );
	SetMapMode( tmpdc, MM_TEXT );

	newfnt = CreateFont( -(int32)size, 0, 0, 0,
		flags & FFLAG_BOLD ? FW_BOLD : FW_NORMAL,
		flags & FFLAG_ITALIC ? TRUE : FALSE,
		flags & FFLAG_ULINE ? TRUE : FALSE,
		flags & FFLAG_STRIKE ? TRUE : FALSE,
		charset, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		flags & FFLAG_NOAA ? NONANTIALIASED_QUALITY : CLEARTYPE_QUALITY,
		VARIABLE_PITCH, name );

	if ( !newfnt ) return NULL;

	firstc = firstc > 32 ? firstc : 32;
	lastc = lastc > firstc ? lastc : 255;

	font = mem_alloc( sizeof(*font) );
	font->first_char = firstc;
	font->last_char = lastc++;
	font->data_len = lastc - firstc;
	font->width = 128;
	font->height = 128;
	font->size = size;
	font->flags = flags;

	oldfnt = (HFONT)SelectObject( tmpdc, newfnt );

	// Find out a sufficient size for our texture bitmap (needs to be power of 2)
	while ( !__measure_font( tmpdc, font, false ) )
	{
		font->width <<= 1;
		font->height <<= 1;
	}

	// Create a table for character texture coordinates
	font->coords	= mem_alloc( font->data_len * sizeof(float*) );
	font->coords[0]	= mem_alloc( 4 * font->data_len * sizeof(float) );

	for ( i = 1; i < font->data_len; i++ )
		font->coords[i] = font->coords[0] + i * 4;

	ZeroMemory( &bmi.bmiHeader, sizeof(BITMAPINFOHEADER) );
	bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth       = (int)font->width;
	bmi.bmiHeader.biHeight      = (int)font->height;
	bmi.bmiHeader.biPlanes      = 1;
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biBitCount    = 32;

	bitmap = CreateDIBSection( tmpdc, &bmi, DIB_RGB_COLORS, (void**)&bm_bits, NULL, 0 );
	bmold = SelectObject( tmpdc, bitmap );

	SetTextColor( tmpdc, 0x00FFFFFF );
	SetBkColor( tmpdc, 0x00000000 );
	SetTextAlign( tmpdc, TA_TOP );

	// Paint the alphabet onto the selected bitmap
	__measure_font( tmpdc, font, true );

	glGenTextures( 1, &font->texture );
	
	GetObject( bitmap, sizeof(BITMAP), &bm );
	glBindTexture( GL_TEXTURE_2D, font->texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	// Hacky method to convert the background colour (black) to alpha
	for ( i = 0, pixels = bm.bmBits; i < font->width * font->height; i++ )
	{
		if ( pixels[i] == 0x0 ) /* BLACK */
		{
			pixels[i] &= ~-1;
		}

		else if ( pixels[i] == 0xFFFFFF ) /* WHITE */
		{
			pixels[i] |= 0xFF << 24;
		}

		else /* GREY - or at least close enough */
		{
			ratio = ( (float)( ( pixels[i] & 0xFF0000 ) >> 16 ) / 0xFF
					+ (float)( ( pixels[i] & 0x00FF00 ) >> 8 ) / 0xFF
					+ (float)( pixels[i] & 0x0000FF ) / 0xFF ) / 3;

			pixels[i] = 0x00FFFFFF | ( (uint32)( ratio * 0xFF ) << 24 );
			//pixels[i] = 0x00FFFFFF | ( ( 0xFF * pixels[i] / 0xFFFFFF ) << 24 );
		}
	}

	// Its actually BGRA, but we're rendering white
	gluBuild2DMipmaps( GL_TEXTURE_2D, 4, font->width, font->height, GL_RGBA, GL_UNSIGNED_BYTE, bm.bmBits );

	SelectObject( tmpdc, bmold );
	SelectObject( tmpdc, oldfnt );
	DeleteObject( bitmap );
	DeleteObject( newfnt );
	DeleteDC( tmpdc );

	return font;
}

void mgui_opengl_destroy_font( void* font )
{
	MGuiGLFont* fnt = font;

	assert( font != NULL );

	if ( fnt->texture )
		glDeleteTextures( 1, &fnt->texture );

	mem_free( fnt->coords[0] );
	mem_free( fnt->coords );
	mem_free( font );
}

#endif /* _WIN32 */
