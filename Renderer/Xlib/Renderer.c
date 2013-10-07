/**********************************************************************
 *
 * PROJECT:		Mylly GUI - Xlib Renderer
 * FILE:		Renderer.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An Xlib reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "Renderer.h"
#include "Platform/Alloc.h"
#include "Platform/Window.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// --------------------------------------------------

typedef struct {
	MGuiRendFont	data;
	XFontStruct*	font;
} RendFont;

// --------------------------------------------------

static uint16		window_width	= 0;
static uint16		window_height	= 0;
static uint32		colour			= 0;
static colour_t		draw_colour		= { 0 };
static LINE_STATUS	line_status		= LINE_IDLE;
static bool			line_continue	= false;
extern syswindow_t*	window;
extern GC			gc;

// --------------------------------------------------

static void		renderer_draw_buffer		( const RendFont* font, const char_t* text, int32* x, int32* y, uint32 flags, bool measure );
static void		renderer_process_tag		( const MGuiFormatTag* tag );
static void		renderer_process_underline	( const RendFont* font, int32 x, int32 y, int32* x2, int32* y2, colour_t* line_colour );

// --------------------------------------------------

void renderer_begin( void )
{
	XClearWindow( window->display, window->window );
}

void renderer_end( void )
{
}

void renderer_resize( uint32 w, uint32 h )
{
	window_width = (uint16)w;
	window_height = (uint16)h;
}

DRAW_MODE renderer_set_draw_mode( DRAW_MODE mode )
{
	// Only 2D drawing is supported.
	return mode;
}

void renderer_set_draw_colour( const colour_t* col )
{
	colour = ( col->a << 24 ) | ( col->r << 16 ) | ( col->g << 8 ) | col->b;
	draw_colour = *col;

	XSetForeground( window->display, gc, colour );
}

void renderer_set_draw_depth( float z_depth )
{
	// Only 2D drawing is supported.
	UNREFERENCED_PARAM( z_depth );
}

void renderer_set_draw_transform( const matrix4_t* mat )
{
	// Only 2D drawing is supported.
	UNREFERENCED_PARAM( mat );
}

void renderer_reset_draw_transform( void )
{
	// Only 2D drawing is supported.
	return;
}

void renderer_start_clip( int32 x, int32 y, uint32 w, uint32 h )
{
	XRectangle r = { x, y, w, h };
	XSetClipRectangles( window->display, gc, 0, 0, &r, 1, Unsorted );
}

void renderer_end_clip( void )
{
	XSetClipMask( window->display, gc, None );
}

void renderer_draw_rect( int32 x, int32 y, uint32 w, uint32 h )
{
	XFillRectangle( window->display, window->window, gc, x, y, w, h );
}

void renderer_draw_triangle( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 )
{
	XPoint points[] = { { x1, y1 }, { x2, y2 }, { x3, y3 } };
	XFillPolygon( window->display, window->window, gc, points, 3, Convex, CoordModeOrigin );
}

void renderer_draw_pixel( int32 x, int32 y )
{
	XFillRectangle( window->display, window->window, gc, x, y, 1, 1 );
}

MGuiRendTexture* renderer_load_texture( const char_t* path, uint32* width, uint32* height )
{
	// Texture loading is not supported.
	UNREFERENCED_PARAM( path );

	*width = 0;
	*height = 0;

	return NULL;
}

void renderer_destroy_texture( MGuiRendTexture* texture )
{
	UNREFERENCED_PARAM( texture );
}

void renderer_draw_textured_rect( const MGuiRendTexture* texture, int32 x, int32 y, uint32 w, uint32 h, const float uv[] )
{
	UNREFERENCED_PARAM( texture );
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );
	UNREFERENCED_PARAM( w );
	UNREFERENCED_PARAM( h );
	UNREFERENCED_PARAM( uv );
}

MGuiRendFont* renderer_load_font( const char_t* name, uint8 size, uint8 flags, uint8 charset, uint32 firstc, uint32 lastc )
{
	RendFont* font;

	if ( name == NULL ||
		 *name == '\0' )
		 return NULL;

	font = mem_alloc( sizeof(*font) );

	font->data.size = size;
	font->data.flags = flags;
	font->data.charset = charset;
	font->data.first_char = firstc;
	font->data.last_char = lastc;

	font->font = XLoadQueryFont( window->display, name );

	if ( font->font == NULL )
	{
		mem_free( font );
		return NULL;
	}

	return (MGuiRendFont*)font;
}

void renderer_destroy_font( MGuiRendFont* fnt )
{
	RendFont* font = (RendFont*)fnt;

	if ( font == NULL ) return;

	if ( font->font )
		XFreeFont( window->display, font->font );

	mem_free( font );
}

void renderer_draw_text( const MGuiRendFont* fnt, const char_t* text, int32 x, int32 y,
						 uint32 flags, const MGuiFormatTag tags[], uint32 ntags )
{
	const RendFont* font = (const RendFont*)fnt;
	uint32 ntag = 0, idx = 0;
	int32 dx, dy, line_x, line_y;
	colour_t line_colour, default_colour;
	const MGuiFormatTag* tag;
	register const char_t* s;
	char_t* t;
	char_t tmp[1024] = { 0 };

	if ( font == NULL || text == NULL ) return;

	if ( tags != NULL && ntags > 0 )
	{
		tag = &tags[ntag];
		default_colour.hex = draw_colour.hex;

		dx = x; line_x = dx;
		dy = y; line_y = dy;

		for ( s = text, t = tmp; *s; ++s, ++idx )
		{
			if ( tag && tag->index == idx )
			{
				// Tag change, draw the text in the temp buffer
				if ( *tmp != '\0' )
				{
					*t = '\0';
					renderer_draw_buffer( font, tmp, &dx, &dy, flags, true );
				}

				// Process possible format tags for this index
				renderer_process_tag( tag );
				renderer_process_underline( font, dx, dy, &line_x, &line_y, &line_colour );

				if ( ++ntag < ntags ) tag = &tags[ntag];
				else tag = NULL;

				t = tmp;
				*t = '\0';

			}

			*t++ = *s;
		}

		if ( *tmp != '\0' )
		{
			*t = '\0';
			renderer_draw_buffer( font, tmp, &dx, &dy, flags, true );
		}

		// Process tags one last time in case someone forgot the ending tag.
		if ( line_status == LINE_DRAWING )
		{
			line_status = LINE_DRAW;
			renderer_process_underline( font, dx, dy, &line_x, &line_y, &line_colour );
		}

		// Reset back to default colour if the end tag was missing.
		if ( draw_colour.hex != default_colour.hex )
		{
			renderer_set_draw_colour( &default_colour );
		}
	}
	else
	{
		// No tags to process, just draw the damn text.
		renderer_draw_buffer( font, text, &x, &y, flags, false );
	}
}

void renderer_measure_text( const MGuiRendFont* fnt, const char_t* text, uint32* w, uint32* h )
{
	RendFont* font = (RendFont*)fnt;

	if ( font == NULL )
	{
		*w = *h = 1;
		return;
	}

	XSetFont( window->display, gc, font->font->fid );

#ifdef MYLLY_UNICODE
	*w = XTextWidth16( font->font, text, mstrlen( text ) );
#else
	*w = XTextWidth( font->font, text, strlen( text ) );
#endif

	*h = font->data.size;
}

MGuiRendTarget* renderer_create_render_target( uint32 width, uint32 height )
{
	// Render targets are not supported with this renderer.
	UNREFERENCED_PARAM( width );
	UNREFERENCED_PARAM( height );

	return NULL;
}

void renderer_destroy_render_target( MGuiRendTarget* target )
{
	UNREFERENCED_PARAM( target );
}

void renderer_draw_render_target( const MGuiRendTarget* target, int32 x, int32 y, uint32 w, uint32 h )
{
	UNREFERENCED_PARAM( target );
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );
	UNREFERENCED_PARAM( w );
	UNREFERENCED_PARAM( h );
}

void renderer_enable_render_target( const MGuiRendTarget* target, int32 x, int32 y )
{
	UNREFERENCED_PARAM( target );
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );
}

void renderer_disable_render_target( const MGuiRendTarget* target )
{
	UNREFERENCED_PARAM( target );
}

void renderer_screen_pos_to_world( const vector3_t* src, vector3_t* dst )
{
	// 3D drawing is not supported.
	UNREFERENCED_PARAM( src );

	if ( dst != NULL )
	{
		dst->x = dst->y = dst->z = 0;
		return;
	}
}
void renderer_world_pos_to_screen( const vector3_t* src, vector3_t* dst )
{
	// 3D drawing is not supported.
	UNREFERENCED_PARAM( src );

	if ( dst != NULL )
	{
		dst->x = dst->y = dst->z = 0;
		return;
	}
}

static void renderer_draw_buffer( const RendFont* font, const char_t* text, int32* x, int32* y, uint32 flags, bool measure )
{
	int32 y_pos = *y;

	XSetFont( window->display, gc, font->font->fid );
	y_pos += font->data.size;

#ifdef MYLLY_UNICODE
	XDrawString16( window->display, window->window, gc, *x, y_pos, text, mstrlen( text ) );
#else
	XDrawString( window->display, window->window, gc, *x, y_pos, text, strlen( text ) );
#endif

	if ( measure )
	{
#ifdef MYLLY_UNICODE
		*x += XTextWidth16( font->font, text, mstrlen( text ) );
#else
		*x += XTextWidth( font->font, text, strlen( text ) );
#endif
	}
}

static void renderer_process_tag( const MGuiFormatTag* tag )
{
	if ( tag->flags & TAG_COLOUR ||
		 tag->flags & TAG_COLOUR_END )
	{
		renderer_set_draw_colour( &tag->colour );

		if ( line_status == LINE_DRAWING )
		{
			line_status = LINE_DRAW;
			line_continue = true;
		}
	}

	if ( tag->flags & TAG_UNDERLINE )
	{
		line_status = LINE_BEGIN;
	}

	else if ( tag->flags & TAG_UNDERLINE_END )
	{
		switch ( line_status )
		{
		case LINE_DRAWING:
		case LINE_DRAW:
			line_status = LINE_DRAW;
			line_continue = false;
			break;

		default:
			line_status = LINE_IDLE;
			break;
		}
	}
}

static void renderer_process_underline( const RendFont* font, int32 x, int32 y, int32* x2, int32* y2, colour_t* line_colour )
{
	colour_t col;

	switch ( line_status )
	{
	case LINE_BEGIN:
		*x2 = x;
		*y2 = y + font->data.size + 2;

		*line_colour = draw_colour;
		line_status = LINE_DRAWING;
		break;

	case LINE_DRAW:
		col = draw_colour;

		renderer_set_draw_colour( line_colour );
		renderer_draw_rect( *x2, *y2, x - *x2, 1 );

		renderer_set_draw_colour( &col );

		if ( line_continue )
		{
			line_status = LINE_DRAWING;
			line_continue = false;

			*x2 = x;
			*y2 = y + font->data.size;
		}
		else
		{
			line_status = LINE_IDLE;
		}

		*line_colour = draw_colour;
		break;

	default:
		break;
	}
}
