/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (DirectX8)
 * FILE:		Drawing.cpp
 * LICENCE:		See Licence.txt
 * PURPOSE:		DirectX8 GUI drawing functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "Drawing.h"
#include "Math/Colour.h"
#include "Platform/Platform.h"
#include <d3d8.h>
#include <D3dx8core.h>
#include <D3dx8math.h>
#include <DxErr.h>
#include <assert.h>

#define MAX_VERT				(1024)
#define D3DFVF_PRIMITIVES		(D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
#define D3DFVF_VERTEXFORMAT2D	(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

typedef struct
{
	float	x, y, z, rhw;
	uint32	col;
	float	u, v;
} vertex_t;

typedef struct
{
	uint32		width;
	uint32		height;
	uint32		spacing;
	uint32		flags;
	uint32		size;
	uint32		first_char;
	uint32		last_char;
	uint32		data_len;
	float**		coords;
	IDirect3DTexture8 * texture;
} dxfont_t;

extern IDirect3DDevice8*		D3DDevice;

static uint32					num_vertices = 0;	// Number of vetrices stored into the temp buffer
static uint32					colour		= 0;	// Current drawing colour
static IDirect3DTexture8*		texture		= NULL;	// Current texture
static IDirect3DVertexBuffer8*	vertex_buf	= NULL;	// Pointer to vertex buffer
static vertex_t*				vertex		= NULL;	// Pointer to current vertex
static DWORD					fvf			= 0;	// Storage for vertex shader
//static DWORD					state		= 0;


static void __flush( void )
{
	if ( num_vertices == 0 ) return;

	//D3DDevice->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1 );
	//D3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, num_vertices / 3, &vertices[0], sizeof(vertex_t) );

	vertex_buf->Unlock();

	D3DDevice->SetStreamSource( 0, vertex_buf, sizeof(vertex_t) );
	D3DDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, num_vertices/3 );

	vertex_buf->Lock( 0, 0, (BYTE**)&vertex, D3DLOCK_DISCARD );

	num_vertices = 0;
}

static __inline void __add_vertex( uint x, uint y )
{
	vertex->x = (float)x;
	vertex->y = (float)y;
	vertex->z = 1.0f;
	vertex->rhw = 1.0f;
	vertex->col = colour;

	vertex++;
	num_vertices++;
}

static __inline void __add_vertex_tex( uint x, uint y, float u, float v )
{
	vertex->x = -0.5f + (float)x;
	vertex->y = -0.5f + (float)y;
	vertex->x = -0.5f + (float)x;
	vertex->y = -0.5f + (float)y;
	vertex->z = 1.0f;
	vertex->rhw = 1.0f;
	vertex->col = colour;

	vertex++;
	num_vertices++;
}

void mgui_dx8_drawing_initialize( void )
{
	if ( FAILED( D3DDevice->CreateVertexBuffer( MAX_VERT * sizeof(vertex_t), D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_VERTEXFORMAT2D, D3DPOOL_DEFAULT, &vertex_buf ) ) ) 
		return;
}

void mgui_dx8_drawing_shutdown( void )
{
	vertex_buf->Release();
}

void mgui_dx8_begin( void )
{
	D3DDevice->BeginScene();
	D3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 0, 0 ), 1, 0 );

	D3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	D3DDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	D3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	D3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_MODULATE );
	D3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );
	D3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_CURRENT );

	D3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_MODULATE );
	D3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE );
	D3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2,	D3DTA_CURRENT );

	D3DDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	D3DDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

	D3DDevice->GetVertexShader( &fvf );
	D3DDevice->SetVertexShader( D3DFVF_VERTEXFORMAT2D );

	vertex_buf->Lock( 0, 0, (BYTE**)&vertex, D3DLOCK_DISCARD );
}

void mgui_dx8_end( void )
{
	__flush();

	D3DDevice->SetVertexShader( fvf );

	D3DDevice->EndScene();
	D3DDevice->Present( NULL, NULL, NULL, NULL );
}

void mgui_dx8_resize( uint w, uint h )
{
	UNREFERENCED_PARAM( w );
	UNREFERENCED_PARAM( h );
	// TODO: !
}

void mgui_dx8_set_draw_colour( const colour_t* col )
{
	colour = (col->a << 24) | (col->r << 16) | (col->g << 8) | col->b;
}

void mgui_dx8_start_clip( uint x, uint y, uint w, uint h )
{
	/*RECT r = { x, y, w, h };

	__flush();

	D3DDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
	D3DDevice->SetScissorRect( &r );*/
}

void mgui_dx8_end_clip( void )
{
	/*__flush();
	D3DDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );*/
}

void mgui_dx8_draw_rect( uint x, uint y, uint w, uint h )
{
	if ( num_vertices >= MAX_VERT-7 ) __flush();

	if ( texture )
	{
		__flush();

		D3DDevice->SetTexture( 0, NULL );
		texture = NULL;
	}

	__add_vertex( x, y );
	__add_vertex( x+w, y );
	__add_vertex( x, y+h );

	__add_vertex( x+w, y );
	__add_vertex( x+w, y+h );
	__add_vertex( x, y+h );
}

void mgui_dx8_draw_triangle( uint x1, uint y1, uint x2, uint y2, uint x3, uint y3 )
{
	if ( num_vertices >= MAX_VERT-4 ) __flush();

	__add_vertex( x1, y1 );
	__add_vertex( x2, y2 );
	__add_vertex( x3, y3 );
}

void* mgui_dx8_load_texture( const char* path )
{
	return NULL;
}

void mgui_dx8_destroy_texture( void* texture )
{

}

void mgui_dx8_draw_textured_rect( void* texture, uint x, uint y, uint w, uint h )
{

}

static bool __measure_font( HDC tmpdc, dxfont_t* font, bool print )
{
	SIZE size;
	uint32 x, y;
	uint32 c, idx;
	char_t tmp[2] = _TEXT("x");

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
#include <fstream>
using namespace std;
void CaptureScreen(HWND window, const char* filename,HBITMAP mappi)
{
	// get screen rectangle
	RECT windowRect;
	GetWindowRect(window, &windowRect);

	// bitmap dimensions
	int bitmap_dx = windowRect.right - windowRect.left;
	int bitmap_dy = windowRect.bottom - windowRect.top;

	// create file
	ofstream file(filename, ios::binary);
	if(!file) return;

	// save bitmap file headers
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;
	file.write((char*)&fileHeader, sizeof(fileHeader));
	file.write((char*)&infoHeader, sizeof(infoHeader));

	fileHeader.bfType      = 0x4d42;
	fileHeader.bfSize      = 0;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	infoHeader.biSize          = sizeof(infoHeader);
	infoHeader.biWidth         = bitmap_dx;
	infoHeader.biHeight        = bitmap_dy;
	infoHeader.biPlanes        = 1;
	infoHeader.biBitCount      = 24;
	infoHeader.biCompression   = BI_RGB;
	infoHeader.biSizeImage     = 0;
	infoHeader.biXPelsPerMeter = 0;
	infoHeader.biYPelsPerMeter = 0;
	infoHeader.biClrUsed       = 0;
	infoHeader.biClrImportant  = 0;

	file.write((char*)&fileHeader, sizeof(fileHeader));
	file.write((char*)&infoHeader, sizeof(infoHeader));

	// dibsection information
	BITMAPINFO info;
	info.bmiHeader = infoHeader; 

	// ------------------
	// THE IMPORTANT CODE
	// ------------------
	// create a dibsection and blit the window contents to the bitmap
	HDC winDC = GetWindowDC(window);
	HDC memDC = CreateCompatibleDC(winDC);
	BYTE* memory = 0;
	HBITMAP bitmap = CreateDIBSection(winDC, &info, DIB_RGB_COLORS, (void**)&memory, 0, 0);
	SelectObject(memDC, bitmap);
	BitBlt(memDC, 0, 0, bitmap_dx, bitmap_dy, winDC, 0, 0, SRCCOPY);
	DeleteDC(memDC);
	ReleaseDC(window, winDC);

	// save dibsection data
	int bytes = (((24*bitmap_dx + 31) & (~31))/8)*bitmap_dy;
	file.write((const char*)memory, bytes);

	// HA HA, forgot paste in the DeleteObject lol, happy now ;)?
	DeleteObject(bitmap);
}

void* mgui_dx8_load_font( const char* name, uint32 size, uint32 flags, uint32 charset, uint32 firstc, uint32 lastc )
{
	HFONT newfnt, oldfnt;
	DWORD* bm_bits;
	BITMAPINFO bmi;
	HBITMAP bitmap;
	HGDIOBJ bmold;
	BITMAP bm;
	dxfont_t* font;
	HDC tmpdc;
	uint32 i, *pixels;
	float ratio;
	D3DLOCKED_RECT d3drect;

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

	font = (dxfont_t*)mem_alloc( sizeof(*font) );
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

	D3DDevice->CreateTexture( font->width, font->height, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &font->texture );

	if ( !font->texture )
	{
		mem_free( font );
		return NULL;
	}

	// Create a table for character texture coordinates
	font->coords	= (float**)mem_alloc( font->data_len * sizeof(float*) );
	font->coords[0]	= (float*)mem_alloc( 4 * font->data_len * sizeof(float) );

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

	GetObject( bitmap, sizeof(BITMAP), &bm );

	//------------------------------------------------------------------------------------------------
	extern HWND wnd;
///CaptureScreen( wnd, "paskaa.bmp" );

	ofstream file("paskaa.bmp", ios::binary);

	// save bitmap file headers
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	fileHeader.bfType      = 0x4d42;
	fileHeader.bfSize      = 0;
	fileHeader.bfReserved1 = 0;
	fileHeader.bfReserved2 = 0;
	fileHeader.bfOffBits   = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	file.write((char*)&fileHeader, sizeof(fileHeader));
	file.write((char*)&bmi.bmiHeader, sizeof(bmi.bmiHeader));

//	int bytes = (((24*font->width + 31) & (~31))/8)*font->height;
	file.write((const char*)&bm_bits, bm.bmWidthBytes*bm.bmHeight);


	// Hacky method to convert the background colour (black) to alpha
#if 1
	for ( i = 0, pixels = (uint32*)bm.bmBits; i < font->width * font->height; i++ )
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
		}
	}
#endif

	font->texture->LockRect( 0, &d3drect, 0, 0 );

	GetDIBits( tmpdc, bitmap, 0, 0, d3drect.pBits, &bmi, DIB_RGB_COLORS );

	font->texture->UnlockRect( 0 );

	SelectObject( tmpdc, bmold );
	SelectObject( tmpdc, oldfnt );
	DeleteObject( bitmap );
	DeleteObject( newfnt );
	DeleteDC( tmpdc );

	return font;
}

void mgui_dx8_destroy_font( void* font )
{
	dxfont_t* dxfont = (dxfont_t*)font;

	if ( dxfont->texture )
		dxfont->texture->Release();

	mem_free( dxfont->coords[0] );
	mem_free( dxfont->coords );
	mem_free( font );
}

static __inline uint32 __render_char( dxfont_t* font, uint32 c, uint32 x, uint32 y, uint32 flags )
{
	float x1, y1, x2, y2;
	float space, offset;
	uint32 w, h;

	if ( num_vertices >= MAX_VERT-13 ) __flush();

	x1 = font->coords[c][0];
	x2 = font->coords[c][2];
	y1 = font->coords[c][1];
	y2 = font->coords[c][3];

	space = ( font->flags & FFLAG_ITALIC ) ? (x2-x1) : 0;
	offset = space * font->width;

	w = (uint32)( ( x2 - x1 + space ) * font->width );
	h = (uint32)( ( y2 - y1 ) * font->height );
	y -= h;

	if ( flags & FFLAG_SHADOW )
	{
		/*glColor4ubv( (const GLubyte*)&shadow_col );

		glTexCoord2f( x1, y2 );
		glVertex2i( x + SHADOW_OFFSET, y + SHADOW_OFFSET );

		glTexCoord2f( x2+space, y2 );
		glVertex2i( x+w + SHADOW_OFFSET, y + SHADOW_OFFSET );

		glTexCoord2f( x2+space, y1 );
		glVertex2i( x+w + SHADOW_OFFSET, y+h + SHADOW_OFFSET );

		glTexCoord2f( x1, y1 );
		glVertex2i( x + SHADOW_OFFSET, y+h + SHADOW_OFFSET );

		glColor4ubv( (const GLubyte*)&colour );*/
	}

	__add_vertex_tex( x, y, x1, y2 );
	__add_vertex_tex( x+w, y, x2+space, y2 );
	__add_vertex_tex( x+w, y+h, x2+space, y1 );

	__add_vertex_tex( x, y, x1, y2 );
	__add_vertex_tex( x, y+h, x1, y1 );
	__add_vertex_tex( x+w, y+h, x2+space, y1 );

	/*__add_vertex_tex( x, y, x1, y2 );
	__add_vertex_tex( x+w, y, x2+space, y2 );
	__add_vertex_tex( x, y+h, x1, y1 );

	__add_vertex_tex( x+w, y, x2+space, y2 );
	__add_vertex_tex( x+w, y+h, x2+space, y1 );
	__add_vertex_tex( x, y+h, x1, y1 );*/

	return (uint32)( w - offset );
}

void mgui_dx8_draw_text( void* font, const char_t* text, uint x, uint y, uint flags )
{
	uint32 dx, dy, c;
	register const char_t* s;
	dxfont_t* fnt = (dxfont_t*)font;

	assert( font != NULL );

	if ( !text ) return;

	__flush();

	D3DDevice->SetTexture( 0, fnt->texture );
	texture = fnt->texture;

	dx = x;
	dy = y;

	for ( s = text; *s; s += sizeof(char_t) )
	{
		c = *(uchar_t*)s;

		if ( c < fnt->first_char || c > fnt->last_char )
		{
			continue;
		}
		else
		{
			dx += __render_char( fnt, c - fnt->first_char, dx, dy, flags );		
		}
	}
}

void mgui_dx8_measure_text( void* font, const char_t* text, uint* x_out, uint* y_out )
{
	float x, y, xout;
	float tmp1, tmp2;
	register const char_t* s;
	uint32 c;
	dxfont_t* fnt = (dxfont_t*)font;

	assert( font != NULL );
	assert( text != NULL );

	x = xout = 0;
	y = (float)fnt->size;

	for ( s = text; *s; s += sizeof(char_t) )
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
