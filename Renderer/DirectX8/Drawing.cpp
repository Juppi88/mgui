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
#include "Platform/Alloc.h"
#include "Platform/Window.h"
#include "Stringy/Stringy.h"
#include <assert.h>

#define MAX_VERT				(1024)
#define D3DFVF_PRIMITIVES		(D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
#define D3DFVF_VERTEXFORMAT2D	(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

struct vertex_t
{
	float	x, y, z, rhw;
	uint32	col;
	float	u, v;
};

extern IDirect3DDevice8* d3dDevice;

uint32					CMGuiDX8Drawing::num_vertices	= 0;	// Number of vetrices stored into the temp buffer
uint32					CMGuiDX8Drawing::colour			= 0;	// Current drawing colour
IDirect3DTexture8*		CMGuiDX8Drawing::texture		= NULL;	// Current texture
IDirect3DVertexBuffer8*	CMGuiDX8Drawing::vertex_buf		= NULL;	// Pointer to vertex buffer
vertex_t*				CMGuiDX8Drawing::vertex			= NULL;	// Pointer to current vertex
DWORD					CMGuiDX8Drawing::fvf			= 0;	// Storage for vertex shader
RECT					CMGuiDX8Drawing::clip;					// Temp storage for clipping

void CMGuiDX8Drawing::Initialize( void )
{
	d3dDevice->CreateVertexBuffer( MAX_VERT * sizeof(vertex_t), D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,
		D3DFVF_VERTEXFORMAT2D, D3DPOOL_DEFAULT, &vertex_buf );
}

void CMGuiDX8Drawing::Shutdown( void )
{
	vertex_buf->Release();
}

void CMGuiDX8Drawing::Begin( void )
{
	d3dDevice->BeginScene();
	d3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 0, 0 ), 1, 0 );

	d3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	d3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	d3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,		D3DTOP_MODULATE );
	d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1,	D3DTA_TEXTURE );
	d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2,	D3DTA_CURRENT );

	d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,		D3DTOP_MODULATE );
	d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1,	D3DTA_TEXTURE );
	d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2,	D3DTA_CURRENT );

	d3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	d3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

	d3dDevice->GetVertexShader( &fvf );
	d3dDevice->SetVertexShader( D3DFVF_VERTEXFORMAT2D );

	vertex_buf->Lock( 0, 0, (BYTE**)&vertex, D3DLOCK_DISCARD );
}

void CMGuiDX8Drawing::End( void )
{
	Flush();

	d3dDevice->SetVertexShader( fvf );

	d3dDevice->EndScene();
	d3dDevice->Present( NULL, NULL, NULL, NULL );
}

void CMGuiDX8Drawing::Resize( uint32 w, uint32 h )
{
	UNREFERENCED_PARAM( w );
	UNREFERENCED_PARAM( h );
	// TODO: !
}

void CMGuiDX8Drawing::SetDrawColour( const colour_t* col )
{
	if ( col == NULL ) return;

	colour = (col->a << 24) | (col->r << 16) | (col->g << 8) | col->b;
}

void CMGuiDX8Drawing::StartClip( uint32 x, uint32 y, uint32 w, uint32 h )
{
	// DX8 doesn't seem to have a built-in scissor feature,
	// so we'll have to figure out something else here.
	// TODO: Make this work for more than just text.

	Flush();

	clip.left = x;
	clip.top = y;
	clip.right = x + w;
	clip.bottom = y + h;
}

void CMGuiDX8Drawing::EndClip( void )
{
	Flush();

	clip.left = 0;
	clip.top = 0;
	clip.right = 0;
	clip.bottom = 0;
}

void CMGuiDX8Drawing::DrawRect( uint32 x, uint32 y, uint32 w, uint32 h )
{
	if ( num_vertices >= MAX_VERT-7 ) Flush();

	if ( texture )
	{
		Flush();

		d3dDevice->SetTexture( 0, NULL );
		texture = NULL;
	}

	AddVertex( x, y );
	AddVertex( x+w, y );
	AddVertex( x, y+h );

	AddVertex( x+w, y );
	AddVertex( x+w, y+h );
	AddVertex( x, y+h );
}

void CMGuiDX8Drawing::DrawTriangle( uint32 x1, uint32 y1, uint32 x2, uint32 y2, uint32 x3, uint32 y3 )
{
	if ( num_vertices >= MAX_VERT-4 ) Flush();

	AddVertex( x1, y1 );
	AddVertex( x2, y2 );
	AddVertex( x3, y3 );
}

void* CMGuiDX8Drawing::LoadTexture( const char_t* path )
{
	UNREFERENCED_PARAM( path );

	return NULL;
}

void CMGuiDX8Drawing::DestroyTexture( void* texture )
{
	UNREFERENCED_PARAM( texture );
}

void CMGuiDX8Drawing::DrawTexturedRect( void* texture, uint32 x, uint32 y, uint32 w, uint32 h )
{
	UNREFERENCED_PARAM( texture );
	UNREFERENCED_PARAM( x );
	UNREFERENCED_PARAM( y );
	UNREFERENCED_PARAM( w );
	UNREFERENCED_PARAM( h );
}

void* CMGuiDX8Drawing::LoadFont( const char_t* name, uint32 size, uint32 flags, uint32 charset, uint32 firstc, uint32 lastc )
{
	LOGFONT font;
	LPD3DXFONT d3dxFont;

	UNREFERENCED_PARAM( firstc );
	UNREFERENCED_PARAM( lastc );

	memset( &font, 0, sizeof(font) );

	font.lfHeight = -1 * size;
	font.lfWidth = 0;
	font.lfWeight = BIT_ON( flags, FFLAG_BOLD ) ? FW_BOLD : FW_NORMAL;
	font.lfItalic = BIT_ON( flags, FFLAG_ITALIC ) ? 1 : 0;
	font.lfUnderline = BIT_ON( flags, FFLAG_ULINE ) ? 1 : 0;
	font.lfStrikeOut = BIT_ON( flags, FFLAG_STRIKE ) ? 1 : 0;
	font.lfCharSet = (BYTE)charset;
	font.lfOutPrecision = OUT_DEFAULT_PRECIS;
	font.lfQuality = BIT_ON( flags, FFLAG_NOAA ) ? DEFAULT_QUALITY : CLEARTYPE_QUALITY;
	font.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

	mstrcpy( font.lfFaceName, name, sizeof(font.lfFaceName) );

	D3DXCreateFontIndirect( d3dDevice, &font, &d3dxFont );

	return (void*)d3dxFont;
}

void CMGuiDX8Drawing::DestroyFont( void* font )
{
	LPD3DXFONT d3dxFont;

	if ( font == NULL ) return;

	d3dxFont = (LPD3DXFONT)font;
	d3dxFont->Release();
}

void CMGuiDX8Drawing::DrawText( void* font, const char_t* text, uint32 x, uint32 y, uint32 flags )
{
	LPD3DXFONT d3dxFont;
	RECT r;

	UNREFERENCED_PARAM( flags );

	if ( font == NULL ) return;

	Flush();

	d3dxFont = (LPD3DXFONT)font;

	r.left = x;
	r.top = y;
	r.right = clip.right;
	r.bottom = clip.bottom;

	// OK, this seems to be a _really_ crappy way to draw text (->massive memory usage).
	// TODO: Write our own font rendering system.
	if ( r.right )
	{
		d3dxFont->DrawText( text, -1, &r, DT_LEFT|DT_TOP|DT_SINGLELINE, colour );
	}
	else
	{
		d3dxFont->DrawText( text, -1, &r, DT_LEFT|DT_TOP|DT_NOCLIP|DT_SINGLELINE, colour );
	}
}

void CMGuiDX8Drawing::MeasureText( void* font, const char_t* text, uint32* x_out, uint32* y_out )
{
	LPD3DXFONT d3dxFont;
	RECT r;

	if ( font == NULL || text == NULL ) return;
	if ( x_out == NULL || y_out == NULL ) return;

	d3dxFont = (LPD3DXFONT)font;

	r.left = 0;
	r.right = 0;
	r.top = 0;
	r.bottom = 0;

	if ( !*text )
	{
		d3dxFont->DrawText( "X", -1, &r, DT_CALCRECT, colour );

		*x_out = 0;
		*y_out = r.bottom;

		return;
	}

	d3dxFont->DrawText( text, -1, &r, DT_CALCRECT|DT_LEFT|DT_TOP|DT_SINGLELINE, colour );

	*x_out = r.right;
	*y_out = r.bottom;
}

inline void CMGuiDX8Drawing::AddVertex( uint32 x, uint32 y )
{
	vertex->x = (float)x;
	vertex->y = (float)y;
	vertex->z = 1.0f;
	vertex->rhw = 1.0f;
	vertex->col = colour;

	vertex++;
	num_vertices++;
}

inline void CMGuiDX8Drawing::AddVertexTex( uint32 x, uint32 y, float u, float v )
{
	vertex->x = -0.5f + (float)x;
	vertex->y = -0.5f + (float)y;
	vertex->v = -0.5f + (float)u;
	vertex->u = -0.5f + (float)v;
	vertex->z = 1.0f;
	vertex->rhw = 1.0f;
	vertex->col = colour;

	vertex++;
	num_vertices++;
}

void CMGuiDX8Drawing::Flush( void )
{
	if ( num_vertices == 0 ) return;

	vertex_buf->Unlock();

	d3dDevice->SetStreamSource( 0, vertex_buf, sizeof(vertex_t) );
	d3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, num_vertices/3 );

	vertex_buf->Lock( 0, 0, (BYTE**)&vertex, D3DLOCK_DISCARD );

	num_vertices = 0;
}
