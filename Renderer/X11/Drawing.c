/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (X11)
 * FILE:		Drawing.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		X11 GUI drawing functions.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "Drawing.h"
#include "Platform/Alloc.h"
#include "Platform/Window.h"
#include "Stringy/Stringy.h"

static uint32 colour;
static uint16 window_width;
static uint16 window_height;
extern syswindow_t* window;
extern GC gc;

struct X11Font
{
	XFontStruct* font;
	uint32 size;
	uint32 flags;
};

void mgui_x11_begin( void )
{
	XClearWindow( window->display, window->window );
}

void mgui_x11_end( void )
{
}

void mgui_x11_resize( uint32 w, uint32 h )
{
	window_width = (uint16)w;
	window_height = (uint16)h;
}

void mgui_x11_set_draw_colour( const colour_t* col )
{
	colour = ( col->a << 24 ) | ( col->r << 16 ) | ( col->g << 8 ) | col->b;
	XSetForeground( window->display, gc, colour );
}

void mgui_x11_start_clip( int32 x, int32 y, uint32 w, uint32 h )
{
	XRectangle r = { x, y, w, h };
	XSetClipRectangles( window->display, gc, 0, 0, &r, 1, Unsorted );
}

void mgui_x11_end_clip( void )
{
	XSetClipMask( window->display, gc, None );
}

void mgui_x11_draw_rect( int32 x, int32 y, uint32 w, uint32 h )
{
	XFillRectangle( window->display, window->window, gc, x, y, w, h );
}

void mgui_x11_draw_triangle( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 )
{
	XPoint points[] = { { x1, y1 }, { x2, y2 }, { x3, y3 } };
	XFillPolygon( window->display, window->window, gc, points, 3, Convex, CoordModeOrigin );
}

void* mgui_x11_load_texture( const char_t* path )
{
	UNREFERENCED_PARAM( path );
	return NULL;
}

void mgui_x11_destroy_texture( void* texture )
{
	UNREFERENCED_PARAM( texture );
}

void mgui_x11_draw_textured_rect( const void* texture, int32 x, int32 y, uint32 w, uint32 h )
{
	UNREFERENCED_PARAM( texture );
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );
	UNREFERENCED_PARAM( w );
	UNREFERENCED_PARAM( h );
}

void* mgui_x11_load_font( const char* name, uint32 size, uint32 flags, uint32 charset, uint32 firstc, uint32 lastc )
{
	struct X11Font* font;

	UNREFERENCED_PARAM( charset );
	UNREFERENCED_PARAM( firstc );
	UNREFERENCED_PARAM( lastc );

	if ( !name || !*name ) return NULL;

	font = mem_alloc_clean( sizeof(*font) );

	font->font = XLoadQueryFont( window->display, name );
	font->size = size;
	font->flags = flags;

	if ( font->font == NULL )
	{
		mem_free( font );
		return NULL;
	}

	return (void*)font;
}

void mgui_x11_destroy_font( void* font )
{
	struct X11Font* fnt = (struct X11Font*)font;

	if ( font == NULL ) return;

	if ( fnt->font ) XFreeFont( window->display, fnt->font );
	mem_free( font );
}

void mgui_x11_draw_text( const void* font, const char_t* text, int32 x, int32 y, uint32 flags, const MGuiFormatTag tags[], uint32 ntags )
{
	const struct X11Font* fnt = (const struct X11Font*)font;

	UNREFERENCED_PARAM( tags );
	UNREFERENCED_PARAM( ntags );

	if ( font == NULL ) return;

	XSetFont( window->display, gc, fnt->font->fid );
	y += fnt->size;

#ifdef MYLLY_UNICODE
	XDrawString16( window->display, window->window, gc, x, y, text, mstrlen( text ) );
#else
	XDrawString( window->display, window->window, gc, x, y, text, strlen( text ) );
#endif
}

void mgui_x11_measure_text( const void* font, const char_t* text, uint32* w, uint32* h )
{
	const struct X11Font* fnt = (const struct X11Font*)font;

	if ( font == NULL )
	{
		*w = 1;
		*h = 1;
		return;
	}

	XSetFont( window->display, gc, fnt->font->fid );

#ifdef MYLLY_UNICODE
	*w = XTextWidth16( fnt->font, text, mstrlen( text ) );
#else
	*w = XTextWidth( fnt->font, text, strlen( text ) );
#endif

	*h = fnt->size;
}
