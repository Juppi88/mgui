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
#include "Platform/Window.h"

static uint32 colour;
extern syswindow_t* window;
extern GC gc;

void mgui_x11_begin( void )
{
	//XClearWindow( window->display, window->wnd );
}

void mgui_x11_end( void )
{
}

void mgui_x11_resize( uint32 w, uint32 h )
{
	UNREFERENCED_PARAM( w );
	UNREFERENCED_PARAM( h );
}

void mgui_x11_set_draw_colour( const colour_t* col )
{
	colour = ( col->a << 24 ) | ( col->r << 16 ) | ( col->g << 8 ) | col->b;
	XSetForeground( window->display, gc, colour );
}

void mgui_x11_start_clip( int32 x, int32 y, uint32 w, uint32 h )
{
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );
	UNREFERENCED_PARAM( w );
	UNREFERENCED_PARAM( h );
}

void mgui_x11_end_clip( void )
{
}

void mgui_x11_draw_rect( int32 x, int32 y, uint32 w, uint32 h )
{
	XFillRectangle( window->display, window->wnd, gc, x, y, w, h );
}

void mgui_x11_draw_triangle( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 )
{
	UNREFERENCED_PARAM( x1 );
	UNREFERENCED_PARAM( x2 );
	UNREFERENCED_PARAM( x3 );
	UNREFERENCED_PARAM( y1 );
	UNREFERENCED_PARAM( y2 );
	UNREFERENCED_PARAM( y3 );
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

void mgui_x11_draw_textured_rect( void* texture, int32 x, int32 y, uint32 w, uint32 h )
{
	UNREFERENCED_PARAM( texture );
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );
	UNREFERENCED_PARAM( w );
	UNREFERENCED_PARAM( h );
}

void* mgui_x11_load_font( const char* name, uint32 size, uint32 flags, uint32 charset, uint32 firstc, uint32 lastc )
{
	return NULL;
}

void mgui_x11_destroy_font( void* font )
{

}

void mgui_x11_draw_text( void* font, const char_t* text, int32 x, int32 y, uint32 flags )
{

}

void mgui_x11_measure_text( void* font, const char_t* text, uint32* w, uint32* h )
{
	*w = 1;
	*h = 1;
}
