/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (OpenGL)
 * FILE:		Drawing.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		OpenGL GUI drawing functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "Drawing.h"
#include <assert.h>

static uint32			num_vertices = 0;				// Number of vetrices stored into the temp buffer
static vertex_t			vertices[MYLLY_GUI_MAX_VERT];	// Temp buffer for vertices

static colour_t			colour;							// Current drawing colour
static const colour_t	shadow_col = { 0, 0, 0, 125 };	// Text shadow colour

#define SHADOW_OFFSET 1 // Text shadow offset in pixels


void mgui_opengl_swap_buffers( void );

void mgui_opengl_drawing_init( void )
{
	size_t i;

	for ( i = 0; i < MYLLY_GUI_MAX_VERT; i++ )
	{
		vertices[i].z = 0.5f;
	}
}

void mgui_opengl_begin( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glAlphaFunc( GL_GREATER, 1.0f );	
	glEnable( GL_BLEND );
}

static void __flush( void )
{
	if ( !num_vertices ) return;

	glVertexPointer( 3, GL_FLOAT, sizeof(vertex_t), &vertices[0].x );
	glEnableClientState( GL_VERTEX_ARRAY );

	glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof(vertex_t), &vertices[0].r );
	glEnableClientState( GL_COLOR_ARRAY );

	glTexCoordPointer( 2, GL_FLOAT, sizeof(vertex_t), &vertices[0].u );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glDrawArrays( GL_TRIANGLES, 0, (GLsizei)num_vertices );
	glFlush();

	num_vertices = 0;
}

void mgui_opengl_end( void )
{
	__flush();
	mgui_opengl_swap_buffers();
}

void mgui_opengl_resize( uint w, uint h )
{
	if ( h == 0 ) h = 1;

	glViewport( 0, 0, w, h );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	//gluPerspective( 45.0f, (GLfloat)w / (GLfloat)h, 0.1f, 100.0f );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

void mgui_opengl_set_draw_colour( const colour_t* col )
{
	colour = *col;
}

void mgui_opengl_start_clip( uint x, uint y, uint w, uint h )
{
	GLint view[4];
	
	__flush();

	// Translate OpenGL coordinates to MGUI coords
	glGetIntegerv( GL_VIEWPORT, &view[0] );
	y = view[3] - ( y + h );

	glScissor( x, y, w, h );
	glEnable( GL_SCISSOR_TEST );
}

void mgui_opengl_end_clip( void )
{
	__flush();

	glDisable( GL_SCISSOR_TEST );
}

static __inline void __add_vertex2( uint x, uint y, float u, float v )
{
	if ( num_vertices >= MYLLY_GUI_MAX_VERT-1 ) __flush();

	vertices[num_vertices].x = (float)x;
	vertices[num_vertices].y = (float)y;
	vertices[num_vertices].u = u;
	vertices[num_vertices].v = v;
	vertices[num_vertices].r = colour.r;
	vertices[num_vertices].g = colour.g;
	vertices[num_vertices].b = colour.b;
	vertices[num_vertices].a = colour.a;

	num_vertices++;
}

static __inline void __add_vertex( uint x, uint y )
{
	__add_vertex2( x, y, 0.0f, 0.0f );
}

void mgui_opengl_draw_rect( uint x, uint y, uint w, uint h )
{
	__add_vertex( x, y );
	__add_vertex( x+w, y );
	__add_vertex( x, y+h );

	__add_vertex( x+w, y );
	__add_vertex( x+w, y+h );
	__add_vertex( x, y+h );
}

void mgui_opengl_draw_triangle( uint x1, uint y1, uint x2, uint y2, uint x3, uint y3 )
{
	__add_vertex( x1, y1 );
	__add_vertex( x2, y2 );
	__add_vertex( x3, y3 );
}

void* mgui_opengl_load_texture( const char* path )
{
	UNREFERENCED_PARAM(path);
	return NULL;
}

void mgui_opengl_destroy_texture( void* texture )
{
	UNREFERENCED_PARAM(texture);
}

void mgui_opengl_draw_textured_rect( void* texture, uint x, uint y, uint w, uint h )
{
	UNREFERENCED_PARAM(texture);
	UNREFERENCED_PARAM(x);
	UNREFERENCED_PARAM(y);
	UNREFERENCED_PARAM(w);
	UNREFERENCED_PARAM(h);
}

// Font loading related functions are platform dependent,
// hence they're located in OpenGLWin/X11.c

static __inline uint32 __render_char( oglfont_t* font, uint32 c, uint32 x, uint32 y, uint32 flags )
{
	float x1, y1, x2, y2;
	float space, offset;
	uint32 w, h;

	x1 = font->coords[c][0];
	x2 = font->coords[c][2];
	y1 = 1.0f - font->coords[c][1];
	y2 = 1.0f - font->coords[c][3];

	space = ( font->flags & FFLAG_ITALIC ) ? (x2-x1) : 0;
	offset = space * font->width;

	w = (uint32)( ( x2 - x1 + space ) * font->width );
	h = (uint32)( ( y2 - y1 ) * font->height );
	y -= h;

	if ( flags & FFLAG_SHADOW )
	{
		glColor4ubv( (const GLubyte*)&shadow_col );

		glTexCoord2f( x1, y2 );
		glVertex2i( x + SHADOW_OFFSET, y + SHADOW_OFFSET );

		glTexCoord2f( x2+space, y2 );
		glVertex2i( x+w + SHADOW_OFFSET, y + SHADOW_OFFSET );

		glTexCoord2f( x2+space, y1 );
		glVertex2i( x+w + SHADOW_OFFSET, y+h + SHADOW_OFFSET );

		glTexCoord2f( x1, y1 );
		glVertex2i( x + SHADOW_OFFSET, y+h + SHADOW_OFFSET );

		glColor4ubv( (const GLubyte*)&colour );
	}

	glTexCoord2f( x1, y2 );
	glVertex2i( x, y );

	glTexCoord2f( x2+space, y2 );
	glVertex2i( x+w, y );

	glTexCoord2f( x2+space, y1 );
	glVertex2i( x+w, y+h );

	glTexCoord2f( x1, y1 );
	glVertex2i( x, y+h );

	return (uint32)( w - offset );
}

void mgui_opengl_draw_text( void* font, const char_t* text, uint x, uint y, uint flags )
{
	uint32 dx, dy, c;
	register const char_t* s;
	oglfont_t* fnt = font;

	assert( font != NULL );

	if ( !text ) return;

	__flush();

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, fnt->texture );
	glBegin( GL_QUADS );

	glColor4ubv( (const GLubyte*)&colour );

	dx = x;
	dy = y - fnt->size;

	for ( s = text; *s; s += sizeof(char_t) )
	{
		c = *(uchar_t*)s;

		if ( *s == '\n' )
		{
			dy += fnt->size;
			dx = x;
		}	
		else if ( c < fnt->first_char || c > fnt->last_char )
		{
			continue;
		}
		else
		{
			dx += __render_char( fnt, c - fnt->first_char, dx, dy, flags );		
		}
	}

	glEnd();
	glDisable( GL_TEXTURE_2D );
}

void mgui_opengl_measure_text( void* font, const char_t* text, uint* x_out, uint* y_out )
{
	float x, y, xout;
	float tmp1, tmp2;
	register const char_t* s;
	uint32 c;
	oglfont_t* fnt = font;

	assert( font != NULL );
	assert( text != NULL );

	x = xout = 0;
	y = (float)fnt->size;

	for ( s = text; *s; s += sizeof(char_t) )
	{
		c = *(uchar_t*)s;

		if ( c == '\n' )
		{
			y += fnt->size;
			xout = math_max( xout, x );
			x = 0;
		}	
		else if ( c < fnt->first_char || c > fnt->last_char )
		{
			continue;
		}
		else
		{
			c -= fnt->first_char;
			tmp1 = fnt->coords[c][0];
			tmp2 = fnt->coords[c][2];

			x += (tmp2-tmp1) * fnt->width;
		}
	}	

	xout = math_max( xout, x );

	*x_out = (uint32)xout;
	*y_out = (uint32)y;
}
