/**********************************************************************
 *
 * PROJECT:		Mylly GUI - OpenGL Renderer
 * FILE:		OpenGL.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An OpenGL reference renderer for Mylly GUI.
 *				Context initialization on Windows.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#ifdef _WIN32

#include "OpenGL.h"
#include "Renderer.h"

// --------------------------------------------------

static HWND hwnd = NULL;
static HDC dc = NULL;
static HGLRC context = NULL;

// --------------------------------------------------

void mgui_opengl_initialize_system( void* window )
{
	PIXELFORMATDESCRIPTOR pfd;
	int format;
	RECT rect;

	hwnd = (HWND)window;
	dc = GetDC( hwnd );

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = (PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER|PFD_DRAW_TO_WINDOW);
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 32;

	// Attempt to select and set an appropriate pixel format.
	format = ChoosePixelFormat( dc, &pfd );

	if ( format == 0 ) return;

	SetPixelFormat( dc, format, &pfd );

	// Create an OpenGL rendering context.
	context = wglCreateContext( dc );

	wglMakeCurrent( dc, context );

	GetClientRect( hwnd, &rect );
	renderer_resize( rect.right - rect.left, rect.bottom - rect.top );
}

void mgui_opengl_shutdown_system( void )
{
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( context );

	dc = NULL;
	context = NULL;
}

void mgui_opengl_swap_buffers( void )
{
	SwapBuffers( dc );
}

GLuint mgui_opengl_load_bitmap_texture( const char_t* path, uint32* width, uint32* height )
{
	uint32 w = 32, h = 32;
	HBITMAP bitmap;
	GLuint texture;
	BITMAP bm = { 0 };

	// Attempt to load the bitmap.
	bitmap = (HBITMAP)LoadImage( GetModuleHandle( NULL ), path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE|LR_CREATEDIBSECTION );

	if ( bitmap == NULL ) return 0;

	// Get info about the loaded bitmap (dimensions).
	GetObject( bitmap, sizeof(bm), &bm );

	*width = bm.bmWidth;
	*height = bm.bmHeight;

	// Make sure that the dimensions are a power of two (required for the texture).
	while ( w < *width ) w <<= 1;
	while ( h < *height ) h <<= 1;

	// Create a texture and copy the bitmap to it.
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, bm.bmWidth, bm.bmHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.bmBits );
	
	DeleteObject( bitmap );

	*width = w;
	*height = h;

	return texture;
}

#endif /* _WIN32 */
