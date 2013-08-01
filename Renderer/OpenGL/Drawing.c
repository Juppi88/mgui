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
static uint8			colour[4];						// Current drawing colour
static uint8			shadow_col[4] = { 0,0,0,0x7F };	// Text shadow colour

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

void mgui_opengl_resize( uint32 w, uint32 h )
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
	colour[0] = col->r;
	colour[1] = col->g;
	colour[2] = col->b;
	colour[3] = col->a;
}

void mgui_opengl_start_clip( int32 x, int32 y, uint32 w, uint32 h )
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

static __inline void __add_vertex2( int32 x, int32 y, float u, float v )
{
	if ( num_vertices >= MYLLY_GUI_MAX_VERT-1 ) __flush();

	vertices[num_vertices].x = (float)x;
	vertices[num_vertices].y = (float)y;
	vertices[num_vertices].u = u;
	vertices[num_vertices].v = v;
	vertices[num_vertices].r = colour[0];
	vertices[num_vertices].g = colour[1];
	vertices[num_vertices].b = colour[2];
	vertices[num_vertices].a = colour[3];

	num_vertices++;
}

static __inline void __add_vertex( int32 x, int32 y )
{
	__add_vertex2( x, y, 0.0f, 0.0f );
}

void mgui_opengl_draw_rect( int32 x, int32 y, uint32 w, uint32 h )
{
	__add_vertex( x, y );
	__add_vertex( x+w, y );
	__add_vertex( x, y+h );

	__add_vertex( x+w, y );
	__add_vertex( x+w, y+h );
	__add_vertex( x, y+h );
}

void mgui_opengl_draw_triangle( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 )
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

void mgui_opengl_draw_textured_rect( const void* texture, int32 x, int32 y, uint32 w, uint32 h )
{
	UNREFERENCED_PARAM(texture);
	UNREFERENCED_PARAM(x);
	UNREFERENCED_PARAM(y);
	UNREFERENCED_PARAM(w);
	UNREFERENCED_PARAM(h);
}

// Font loading related functions are platform dependent,
// hence they're located in OpenGLWin/X11.c

static __inline uint32 __render_char( const MGuiGLFont* font, uint32 c, int32 x, int32 y, uint32 flags )
{
	float x1, y1, x2, y2;
	float space, offset;
	uint32 w, h;

	x1 = font->coords[c][0];
	x2 = font->coords[c][2];
	y1 = 1.0f - font->coords[c][1];
	y2 = 1.0f - font->coords[c][3];

	space = 0;//( font->flags & FFLAG_ITALIC ) ? (x2-x1) : 0;
	offset = space * font->width;

	w = (uint32)( ( x2 - x1 + space ) * font->width );
	h = (uint32)( ( y2 - y1 ) * font->height );
	y -= h;

	if ( flags & FLAG_TEXT_SHADOW )
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

static void mgui_opengl_process_tag( const MGuiFormatTag* tag )
{
	if ( tag->flags & TAG_COLOUR ||
		 tag->flags & TAG_COLOUR_END )
	{
		mgui_opengl_set_draw_colour( &tag->colour );
		glColor4ubv( (const GLubyte*)&colour );
	}
	if ( tag->flags & TAG_UNDERLINE )
	{
		// TODO:
	}
	else if ( tag->flags & TAG_UNDERLINE_END )
	{
		// TODO:
	}
}

void mgui_opengl_draw_text( const void* font, const char_t* text, int32 x, int32 y,
						    uint32 flags, const MGuiFormatTag tags[], uint32 ntags )
{
	int32 dx = x, dy = y - 2;
	uint32 c, ntag = 0, idx = 0;
	register const char_t* s;
	const MGuiFormatTag* tag = NULL;
	const MGuiGLFont* fnt = (const MGuiGLFont*)font;

	if ( font == NULL || text == NULL ) return;

	__flush();

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, fnt->texture );
	glBegin( GL_QUADS );

	glColor4ubv( (const GLubyte*)&colour );

	if ( tags ) tag = &tags[ntag];

	for ( s = text; *s; ++s, ++idx )
	{
		c = *(uchar_t*)s;

		// Process tags for this index
		if ( tag && tag->index == idx )
		{
			mgui_opengl_process_tag( tag );

			if ( ++ntag < ntags ) tag = &tags[ntag];
			else tag = NULL;
		}

		if ( c < fnt->first_char || c > fnt->last_char )
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

void mgui_opengl_measure_text( const void* font, const char_t* text, uint32* x_out, uint32* y_out )
{
	float x, y, xout;
	float tmp1, tmp2;
	register const char_t* s;
	uint32 c;
	const MGuiGLFont* fnt = (const MGuiGLFont*)font;

	if ( font == NULL || text == NULL )
	{
		*x_out = 1;
		*y_out = 1;
		return;
	}

	x = xout = 0;
	y = (float)fnt->size;

	for ( s = text; *s; ++s )
	{
		c = *(uchar_t*)s;

		if ( c < fnt->first_char || c > fnt->last_char )
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
