/**********************************************************************
 *
 * PROJECT:		Mylly GUI - OpenGL Renderer
 * FILE:		Renderer.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An OpenGL reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "Renderer.h"
#include "Platform/Alloc.h"
#include "../Shared/Windows/FontLoader.h"

// --------------------------------------------------

#define MAX_VERT (1024)

// --------------------------------------------------

typedef struct {
	MGuiRendFont data;
	uint8		spacing;
	uint32		width;
	uint32		height;
	void*		texture_bits;
	GLuint		texture;
	float**		tex_coords;
	uint32		tex_data_len;
} Font;

// --------------------------------------------------

typedef struct {
	MGuiRendTexture	data;
	GLuint			texture;
	void*			texture_bits;
} Texture;

// --------------------------------------------------

typedef struct {
	MGuiRendTarget	data;
	GLuint			frame_buffer;
	GLuint			texture;
	GLuint			old_buffer;
	int32			x_offset;
	int32			y_offset;
} RenderTarget;

// --------------------------------------------------

typedef struct {
	float x, y, z;
	float u, v;
	uint8 r, g, b, a;
} Vertex;

// --------------------------------------------------

static uint32		num_vertices			= 0;			// Number of vetrices stored into the buffer
static Vertex*		vertex					= NULL;			// Pointer to current vertex
static Vertex		vertex_buffer[MAX_VERT];				// Vertex buffer
static DRAW_MODE	draw_mode				= DRAWING_2D;	// Current draw mode
static GLbyte		colour[4]				= { 0,0,0,0 };	// Current drawing colour in OpenGL format
static colour_t		draw_colour				= { 0 };		// Current drawing colour
static float		draw_depth				= 1.0f;			// Current drawing z depth
static GLuint		draw_texture			= 0;			// Current texture pointer
static rectangle_t	clip_rect;								// Clip rectangle
static bool			is_clipping				= false;		// Is clipping mode active?
static LINE_STATUS	line_status				= LINE_IDLE;	// Text underline status
static bool			line_continue			= false;		// Continue drawing an underline
static int32		x_offset				= 0;			// Current drawing offset (X)
static int32		y_offset				= 0;			// Current drawing offset (Y)

// --------------------------------------------------

static void					renderer_flush					( void );
MYLLY_INLINE static void	renderer_add_vertex				( int32 x, int32 y );
MYLLY_INLINE static void	renderer_add_vertex_2d			( int32 x, int32 y, float z );
MYLLY_INLINE static void	renderer_add_vertex_tex			( int32 x, int32 y, float u, float v );
MYLLY_INLINE static void	renderer_add_vertex_tex_2d		( int32 x, int32 y, float z, float u, float v );
MYLLY_INLINE static void	renderer_check_buffer_for_space	( uint32 vertices );
MYLLY_INLINE static uint32	renderer_draw_char				( const Font* font, uint32 c, int32 x, int32 y, uint32 flags );
static void					renderer_process_tag			( const MGuiFormatTag* tag );
static void					renderer_process_underline		( const Font* font, int32 x, int32 y, int32* x2, int32* y2, colour_t* line_colour );
static void					renderer_create_font_texture	( void* data, uint32 width, uint32 height, void** texture, uint32* texture_pitch );
static void					renderer_create_bitmap_texture	( void* data, uint32 width, uint32 height, void** texture, uint32* texture_pitch );

// These are platform dependent functions, so they're defined in OpenGLWin32.c.
void						mgui_opengl_swap_buffers		( void );
GLuint						mgui_opengl_load_bitmap_texture	( const char_t* path, uint32* width, uint32* height );

// --------------------------------------------------

void renderer_initialize( void )
{
}

void renderer_shutdown( void )
{
}

void renderer_begin( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glAlphaFunc( GL_GREATER, 1.0f );
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnable( GL_BLEND );

	vertex = &vertex_buffer[0];
	num_vertices = 0;
}

void renderer_end( void )
{
	renderer_flush();
	glDisableClientState( GL_VERTEX_ARRAY );

	// Do platform specific processing (swap buffers).
	mgui_opengl_swap_buffers();
}

void renderer_resize( uint32 width, uint32 height )
{
	glViewport( 0, 0, width, height );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

DRAW_MODE renderer_set_draw_mode( DRAW_MODE mode )
{
	DRAW_MODE old = draw_mode;

	switch ( mode )
	{
	case DRAWING_2D:
	case DRAWING_2D_DEPTH:
		draw_mode = mode;
		break;

	default:
		// We only support 2D drawing at the moment.
		// TODO: Add support for 3D rendering
		break;
	}
	
	return old;
}

void renderer_set_draw_colour( const colour_t* col )
{
	colour[0] = col->r;
	colour[1] = col->g;
	colour[2] = col->b;
	colour[3] = col->a;

	draw_colour = *col;
}

void renderer_set_draw_depth( float z_depth )
{
	draw_depth = z_depth;
}

void renderer_set_draw_transform( const matrix4_t* mat )
{
	// We only support 2D drawing at the moment.
	// TODO: Add support for 3D rendering
	UNREFERENCED_PARAM( mat );
}

void renderer_reset_draw_transform( void )
{
	// We only support 2D drawing at the moment.
	// TODO: Add support for 3D rendering
}

void renderer_start_clip( int32 x, int32 y, uint32 w, uint32 h )
{
	GLint view[4];

	renderer_flush();

	// Translate OpenGL coordinates to MGUI units
	glGetIntegerv( GL_VIEWPORT, &view[0] );
	y = view[3] - ( y + h );

	glScissor( x, y, w, h );
	glEnable( GL_SCISSOR_TEST );

	clip_rect.x = (int16)x;
	clip_rect.y = (int16)y;
	clip_rect.w = (uint16)w;
	clip_rect.h = (uint16)h;
	is_clipping = true;
}

void renderer_end_clip( void )
{
	renderer_flush();

	glDisable( GL_SCISSOR_TEST );
	is_clipping = false;
}

void renderer_draw_rect( int32 x, int32 y, uint32 w, uint32 h )
{
	if ( draw_texture != 0 )
	{
		renderer_flush();

		glDisable( GL_TEXTURE_2D );
		draw_texture = 0;
	}

	renderer_check_buffer_for_space( 6 );

	renderer_add_vertex( x, y );
	renderer_add_vertex( x+w, y );
	renderer_add_vertex( x, y+h );

	renderer_add_vertex( x+w, y );
	renderer_add_vertex( x+w, y+h );
	renderer_add_vertex( x, y+h );
}

void renderer_draw_triangle( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 )
{
	if ( draw_texture != 0 )
	{
		renderer_flush();

		glDisable( GL_TEXTURE_2D );
		draw_texture = 0;
	}

	renderer_check_buffer_for_space( 3 );

	renderer_add_vertex( x1, y1 );
	renderer_add_vertex( x2, y2 );
	renderer_add_vertex( x3, y3 );
}

void renderer_draw_pixel( int32 x, int32 y )
{
	renderer_draw_rect( x, y, 1, 1 );
}

MGuiRendTexture* renderer_load_texture( const char_t* path, uint32* width, uint32* height )
{
	Texture* texture;
	GLuint tex;

	if ( path == NULL ||
		 width == NULL ||
		 height == NULL )
		 return NULL;

	texture = mem_alloc_clean( sizeof(*texture) );
	
	// Request bitmap loading.
	tex = mgui_opengl_load_bitmap_texture( path, width, height );
	if ( tex == 0 )
	{
		// Something broke, clean up and exit.
		mem_free( texture );
		return NULL;
	}

	texture->data.width = *width;
	texture->data.height = *height;
	texture->texture = tex;

	return (MGuiRendTexture*)texture;
}

void renderer_destroy_texture( MGuiRendTexture* tex )
{
	Texture* texture = (Texture*)tex;

	if ( texture == NULL ) return;

	glDeleteTextures( 1, &texture->texture );
	mem_free( texture );
}

void renderer_draw_textured_rect( const MGuiRendTexture* tex, int32 x, int32 y, uint32 w, uint32 h, const float uv[] )
{
	Texture* texture = (Texture*)tex;

	if ( texture == NULL ) return;

	if ( draw_texture == 0 || draw_texture != texture->texture )
	{
		renderer_flush();

		draw_texture = texture->texture;

		glBindTexture( GL_TEXTURE_2D, texture->texture );
		glEnable( GL_TEXTURE_2D );
	}

	renderer_add_vertex_tex( x, y, uv[0], 1 - uv[1] );
	renderer_add_vertex_tex( x+w, y, uv[2], 1 - uv[1] );
	renderer_add_vertex_tex( x, y+h, uv[0], 1 - uv[3] );

	renderer_add_vertex_tex( x+w, y, uv[2], 1 - uv[1] );
	renderer_add_vertex_tex( x+w, y+h, uv[2], 1 - uv[3] );
	renderer_add_vertex_tex( x, y+h, uv[0], 1 - uv[3] );
}

MGuiRendFont* renderer_load_font( const char_t* name, uint8 size, uint8 flags, uint8 charset, uint32 firstc, uint32 lastc )
{
	bool ret;
	Font* font;
	MGuiFontInfo info;

	font = mem_alloc_clean( sizeof(*font) );

	firstc = firstc > 0x20 ? firstc : 0x20;
	lastc = lastc > firstc ? lastc : 0xFF;

	lastc++;

	font->data.size = size;
	font->data.flags = flags;
	font->data.charset = charset;
	font->data.first_char = firstc;
	font->data.last_char = lastc;
	font->tex_data_len = lastc - firstc;

	ret = mgui_load_font( name, size, flags, charset, firstc, lastc, &info, renderer_create_font_texture, (void*)font );
	if ( !ret )
	{
		mem_free( font );
		return NULL;
	}

	font->tex_coords = info.tex_coords;
	font->width = info.width;
	font->height = info.height;
	font->spacing = info.spacing;

	// Copy the font glyphs into an OpenGL texture.
	glGenTextures( 1, &font->texture );
	glBindTexture( GL_TEXTURE_2D, font->texture );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA4, font->width, font->height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_REV, font->texture_bits );
	
	// We no longer need the raw glyphs.
	SAFE_DELETE( font->texture_bits );

	return (MGuiRendFont*)font;
}

void renderer_destroy_font( MGuiRendFont* fnt )
{
	Font* font = (Font*)fnt;

	if ( font == NULL ) return;

	if ( font->tex_coords != NULL )
	{
		mem_free( font->tex_coords[0] );
		mem_free( font->tex_coords );
	}

	glDeleteTextures( 1, &font->texture );

	mem_free( font );
}

void renderer_draw_text( const MGuiRendFont* fnt, const char_t* text, int32 x, int32 y,
						 uint32 flags, const MGuiFormatTag tags[], uint32 ntags )
{
	int32 dx, dy, line_x, line_y;
	uint32 c, ntag = 0, idx = 0;
	colour_t line_colour, default_colour;
	register const char_t* s;
	const MGuiFormatTag* tag = NULL;
	const Font* font = (const Font*)fnt;

	if ( font == NULL || text == NULL ) return;

	line_status = LINE_IDLE;

	if ( draw_texture == 0 || draw_texture != font->texture )
	{
		renderer_flush();

		draw_texture = font->texture;

		glBindTexture( GL_TEXTURE_2D, font->texture );
		glEnable( GL_TEXTURE_2D );
	}

	if ( tags && ntags > 0 ) tag = &tags[ntag];

	dx = x; line_x = dx;
	dy = y; line_y = dy;
	default_colour.hex = draw_colour.hex;

	for ( s = text; *s; ++s, ++idx )
	{
		c = *(uchar_t*)s;

		// Process possible format tags for this index
		if ( tag && tag->index == idx )
		{
			renderer_process_tag( tag );
			renderer_process_underline( font, dx, dy, &line_x, &line_y, &line_colour );

			if ( ++ntag < ntags ) tag = &tags[ntag];
			else tag = NULL;
		}

		if ( c < font->data.first_char || c > font->data.last_char )
		{
			continue;
		}
		else
		{
			dx += renderer_draw_char( font, c - font->data.first_char, dx, dy, flags );		
		}
	}

	renderer_flush();

	// Finish the underline in case the end tag was missing
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

void renderer_measure_text( const MGuiRendFont* fnt, const char_t* text, uint32* width, uint32* height )
{
	float x, y, xout;
	float tmp1, tmp2;
	uint32 c;
	register const char_t* s;
	const Font* font = (const Font*)fnt;

	if ( font == NULL || text == NULL )
	{
		*width = 1;
		*height = 1;
		return;
	}

	x = xout = 0;
	y = (float)font->data.size;

	for ( s = text; *s; s++ )
	{
		c = *(uchar_t*)s;

		if ( c < font->data.first_char || c > font->data.last_char )
		{
			continue;
		}
		else
		{
			c -= font->data.first_char;
			tmp1 = font->tex_coords[c][0];
			tmp2 = font->tex_coords[c][2];

			x += ( tmp2 - tmp1 ) * font->width - 2 * font->spacing;
		}
	}	

	xout = math_max( xout, x );

	*width = (uint32)xout;
	*height = (uint32)y;
}


MGuiRendTarget* renderer_create_render_target( uint32 width, uint32 height )
{
	RenderTarget* target;
	GLenum draw_buffer;
	uint32 w = 32, h = 32;

	if ( !GLEW_EXT_framebuffer_object ) return NULL;

	target = mem_alloc_clean( sizeof(*target) );

	// Find suitable size for the actual render target (ideally a power of two)
	while ( w < width || h < height )
	{
		w <<= 1;
		h <<= 1;
	}

	target->data.width = w;
	target->data.height = h;

	// Create the framebuffer
	glGenFramebuffersEXT( 1, &target->frame_buffer );
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, target->frame_buffer );

	// Create the texture we're going to render to
	glGenTextures( 1, &target->texture );
	glBindTexture( GL_TEXTURE_2D, target->texture );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	glFramebufferTextureEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, target->texture, 0 );

	draw_buffer = GL_COLOR_ATTACHMENT0_EXT;
	glDrawBuffers( 1, &draw_buffer );

	// Check that the framebuffer is ok
	if ( glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) != GL_FRAMEBUFFER_COMPLETE_EXT )
	{
		glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );

		mem_free( target );
		return NULL;
	}

	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
	return (MGuiRendTarget*)target;
}

void renderer_destroy_render_target( MGuiRendTarget* target )
{
	RenderTarget* buffer = (RenderTarget*)target;

	if ( target == NULL ) return;

	glDeleteTextures( 1, &buffer->texture );

	if ( GLEW_EXT_framebuffer_object )
		glDeleteFramebuffersEXT( 1, &buffer->frame_buffer );

	mem_free( buffer );
}

void renderer_draw_render_target( const MGuiRendTarget* target, int32 x, int32 y, uint32 w, uint32 h )
{
	RenderTarget* buffer = (RenderTarget*)target;
	float u, v;
	
	if ( buffer == NULL ) return;
	if ( buffer->texture == 0 ) return;

	if ( draw_texture == 0 || draw_texture != buffer->texture )
	{
		renderer_flush();
		draw_texture = buffer->texture;

		glBindTexture( GL_TEXTURE_2D, buffer->texture );
		glEnable( GL_TEXTURE_2D );
	}

	renderer_check_buffer_for_space( 6 );

	u = (float)w / buffer->data.width;
	v = (float)h / buffer->data.height;

	renderer_add_vertex_tex( x, y, 0, v );
	renderer_add_vertex_tex( x+w, y, u, v );
	renderer_add_vertex_tex( x, y+h, 0, 0 );

	renderer_add_vertex_tex( x+w, y, u, v );
	renderer_add_vertex_tex( x+w, y+h, u, 0 );
	renderer_add_vertex_tex( x, y+h, 0, 0 );
}

void renderer_enable_render_target( const MGuiRendTarget* target, int32 x, int32 y )
{
	RenderTarget* buffer = (RenderTarget*)target;

	if ( target == NULL ) return;
	if ( !GLEW_EXT_framebuffer_object ) return;

	// Store old framebuffer.
	glGetIntegerv( GL_FRAMEBUFFER_BINDING_EXT, (GLint*)&buffer->old_buffer );

	// Enable our framebuffer.
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, buffer->frame_buffer );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	buffer->x_offset = x_offset;
	buffer->y_offset = y_offset;
	x_offset = x;
	y_offset = y;
}

void renderer_disable_render_target( const MGuiRendTarget* target )
{
	RenderTarget* buffer = (RenderTarget*)target;

	if ( target == NULL ) return;
	if ( !GLEW_EXT_framebuffer_object ) return;

	// Restore old framebuffer.
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, buffer->old_buffer );

	x_offset = buffer->x_offset;
	y_offset = buffer->y_offset;
}

void renderer_screen_pos_to_world( const vector3_t* src, vector3_t* dst )
{
	// We don't support 3D drawing at the moment.
	// TODO: Add support for 3D rendering.
	UNREFERENCED_PARAM( src );

	if ( dst != NULL )
	{
		dst->x = dst->y = dst->z = 0;
		return;
	}
}
void renderer_world_pos_to_screen( const vector3_t* src, vector3_t* dst )
{
	// We don't support 3D drawing at the moment.
	// TODO: Add support for 3D rendering.
	UNREFERENCED_PARAM( src );

	if ( dst != NULL )
	{
		dst->x = dst->y = dst->z = 0;
		return;
	}
}

void renderer_flush( void )
{
	if ( num_vertices == 0 ) return;

	glVertexPointer( 3, GL_FLOAT, sizeof(Vertex), &vertex_buffer[0].x );
	glEnableClientState( GL_VERTEX_ARRAY );

	glColorPointer( 4, GL_UNSIGNED_BYTE, sizeof(Vertex), &vertex_buffer[0].r );
	glEnableClientState( GL_COLOR_ARRAY );

	glTexCoordPointer( 2, GL_FLOAT, sizeof(Vertex), &vertex_buffer[0].u );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glDrawArrays( GL_TRIANGLES, 0, (GLsizei)num_vertices );
	glFlush();

	vertex = &vertex_buffer[0];
	num_vertices = 0;
}

MYLLY_INLINE void renderer_add_vertex( int32 x, int32 y )
{
	x -= x_offset;
	y -= y_offset;

	// We only support 2D drawing at the moment.
	// TODO: Add support for 3D rendering
	switch ( draw_mode )
	{
	case DRAWING_2D:
		renderer_add_vertex_2d( x, y, 1.0f );
		break;

	case DRAWING_2D_DEPTH:
		renderer_add_vertex_2d( x, y, draw_depth );
		break;
	}
}

MYLLY_INLINE void renderer_add_vertex_2d( int32 x, int32 y, float z )
{
	vertex->x = (float)x;
	vertex->y = (float)y;
	vertex->z = z;
	vertex->u = 0;
	vertex->v = 0;
	vertex->r = colour[0];
	vertex->g = colour[1];
	vertex->b = colour[2];
	vertex->a = colour[3];

	num_vertices++;
	vertex++;
}

MYLLY_INLINE void renderer_add_vertex_tex( int32 x, int32 y, float u, float v )
{
	x -= x_offset;
	y -= y_offset;

	// We only support 2D drawing at the moment.
	// TODO: Add support for 3D rendering
	switch ( draw_mode )
	{
	case DRAWING_2D:
		renderer_add_vertex_tex_2d( x, y, 1.0f, u, v );
		break;

	case DRAWING_2D_DEPTH:
		renderer_add_vertex_tex_2d( x, y, draw_depth, u, v );
		break;
	}
}

MYLLY_INLINE void renderer_add_vertex_tex_2d( int32 x, int32 y, float z, float u, float v )
{
	vertex->x = (float)x;
	vertex->y = (float)y;
	vertex->z = z;
	vertex->u = u;
	vertex->v = v;
	vertex->r = colour[0];
	vertex->g = colour[1];
	vertex->b = colour[2];
	vertex->a = colour[3];

	num_vertices++;
	vertex++;
}

MYLLY_INLINE void renderer_check_buffer_for_space( uint32 vertices )
{
	if ( num_vertices + vertices >= MAX_VERT-1 )
	{
		// Vertex buffer is almost full, flush it.
		renderer_flush();
	}
}

MYLLY_INLINE uint32 renderer_draw_char( const Font* font, uint32 c, int32 x, int32 y, uint32 flags )
{
	float tx1, ty1, tx2, ty2;
	uint32 w, h, spacing;
	colour_t col;
	rectangle_t r;
	static colour_t shadow_colour = { 0 };
	static int32 shadow_offset = 1;

	renderer_check_buffer_for_space( 12 );

	tx1 = font->tex_coords[c][0];
	tx2 = font->tex_coords[c][2];
	ty1 = 1.0f - font->tex_coords[c][1];
	ty2 = 1.0f - font->tex_coords[c][3];

	w = (uint32)( ( tx2 - tx1 ) * font->width );
	h = (uint32)( ( ty1 - ty2 ) * font->height );
	spacing = (uint32)font->spacing * 2;

	r.x = (int16)x, r.y = (int16)y, r.w = (uint16)w, r.h = (uint16)h;

	if ( is_clipping && (
		 x < (int32)clip_rect.x || (int32)(x+w) > (int32)clip_rect.x+clip_rect.w+spacing+1 ) )
	{
		return ( w - spacing );
	}

	if ( flags & TFLAG_SHADOW )
	{
		shadow_colour.a = draw_colour.a;

		col.hex = draw_colour.hex;
		renderer_set_draw_colour( &shadow_colour );

		renderer_add_vertex_tex( x+shadow_offset, y+shadow_offset, tx1, ty1 );
		renderer_add_vertex_tex( x+w+shadow_offset, y+shadow_offset, tx2, ty1 );
		renderer_add_vertex_tex( x+shadow_offset, y+h+shadow_offset, tx1, ty2 );

		renderer_add_vertex_tex( x+w+shadow_offset, y+shadow_offset, tx2, ty1 );
		renderer_add_vertex_tex( x+w+shadow_offset, y+h+shadow_offset, tx2, ty2 );
		renderer_add_vertex_tex( x+shadow_offset, y+h+shadow_offset, tx1, ty2 );

		renderer_set_draw_colour( &col );
	}

	renderer_add_vertex_tex( x, y, tx1, ty1 );
	renderer_add_vertex_tex( x+w, y, tx2, ty1 );
	renderer_add_vertex_tex( x, y+h, tx1, ty2 );

	renderer_add_vertex_tex( x+w, y, tx2, ty1 );
	renderer_add_vertex_tex( x+w, y+h, tx2, ty2 );
	renderer_add_vertex_tex( x, y+h, tx1, ty2 );

	return ( w - 2 * font->spacing );
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

static void renderer_process_underline( const Font* font, int32 x, int32 y, int32* x2, int32* y2, colour_t* line_colour )
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

		renderer_flush();
		renderer_set_draw_colour( line_colour );

		renderer_draw_rect( *x2, *y2, x - *x2, 1 );
		renderer_flush();

		draw_texture = font->texture;
		glEnable( GL_TEXTURE_2D );

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
	}
}

static void renderer_create_font_texture( void* data, uint32 width, uint32 height, void** texture, uint32* texture_pitch )
{
	Font* font = (Font*)data;

	// We only have to allocate enough memory for the glyphs here. The font loader will
	// print the font glyphs into that memory, and we'll copy the data to an OpenGL
	// texture once the font loader exits.
	*texture = mem_alloc( width * height * sizeof(uint16) );
	*texture_pitch = width * sizeof(uint16);

	font->texture_bits = *texture;
}

static void renderer_create_bitmap_texture( void* data, uint32 width, uint32 height, void** texture, uint32* texture_pitch )
{
	Texture* tex = (Texture*)data;

	// We only have to allocate enough memory for the glyphs here. The font loader will
	// print the font glyphs into that memory, and we'll copy the data to an OpenGL
	// texture once the font loader exits.
	*texture = mem_alloc( width * height * sizeof(uint32) );
	*texture_pitch = width * sizeof(uint32);

	tex->texture_bits = *texture;
	tex->data.width = width;
	tex->data.height = height;
}
