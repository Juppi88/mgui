/**********************************************************************
 *
 * PROJECT:		Mylly GUI - DirectX 9 Renderer
 * FILE:		Renderer.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A DirectX 9 reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "Renderer.h"
#include "../Shared/Windows/FontLoader.h"
#include "Platform/Alloc.h"
#include "Stringy/Stringy.h"

// --------------------------------------------------

#define MAX_VERT				(1024)
#define D3DFVF_PRIMITIVE		(D3DFVF_XYZRHW|D3DFVF_DIFFUSE)
#define D3DFVF_TEXTURED			(D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)
#define D3DFVF_PRIMITIVE_3D		(D3DFVF_XYZ|D3DFVF_DIFFUSE)
#define D3DFVF_TEXTURED_3D		(D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// --------------------------------------------------

class CFont : MGuiRendFont
{
	friend class CRenderer;

public:
	CFont(){}
	~CFont(){}

private:
	uint8				spacing;
	uint32				width;
	uint32				height;
	IDirect3DTexture9*	texture;
	float**				texCoords;
	uint32				texDataLen;
};

// --------------------------------------------------

class CTexture
{
	friend class CRenderer;

public:
	CTexture( void ) {}
	~CTexture( void ) {}

private:
	IDirect3DTexture9*	texture;
	uint32				width;
	uint32				height;
};

// --------------------------------------------------

class CRenderTarget : MGuiRendTarget
{
	friend class CRenderer;

public:
	CRenderTarget( void ) {}
	~CRenderTarget( void ) {}

private:
	IDirect3DTexture9*	texture;
	IDirect3DSurface9*	surface;
	IDirect3DSurface9*	oldSurface;
	CRenderTarget*		oldCache;
	int32				oldX;
	int32				oldY;
};

// --------------------------------------------------

struct Vertex {
	float	x, y, z, rhw;
	uint32	col;
	float	u, v;
};

struct Vertex3D {
	float	x, y, z;
	uint32	col;
	float	u, v;
};

// --------------------------------------------------

extern IDirect3DDevice9* d3dDevice;

// --------------------------------------------------

DRAW_MODE				CRenderer::drawMode			= DRAWING_2D;	// Current drawing mode
uint32					CRenderer::colour			= 0;			// Current drawing colour
colour_t				CRenderer::drawColour		= 0;			// Current drawing colour as a colour struct
D3DMATRIX				CRenderer::transform;						// Old transform matrix
IDirect3DTexture9*		CRenderer::renderTexture	= NULL;			// Current texture
IDirect3DVertexBuffer9*	CRenderer::vertexBuffer		= NULL;			// Pointer to vertex buffer
IDirect3DVertexBuffer9*	CRenderer::vertexBuffer3d	= NULL;			// Pointer to 3D vertex buffer
bool					CRenderer::lockBuffer		= true;			// Should the vertex buffer be locked
bool					CRenderer::lockBuffer3d		= true;			// Should the 3D vertex buffer be locked
Vertex*					CRenderer::vertex			= NULL;			// Pointer to current vertex
Vertex3D*				CRenderer::vertex3d			= NULL;			// Pointer to current 3D vertex
uint32					CRenderer::numVertices		= 0;			// Number of vetrices stored into the temp buffer
uint32					CRenderer::numVertices3d	= 0;			// Number of 3D vetrices stored into the temp buffer
DWORD					CRenderer::fvfOld			= 0;			// Storage for vertex shader
DWORD					CRenderer::fvf				= D3DFVF_PRIMITIVE;
IDirect3DStateBlock9*	CRenderer::normState		= 0;			// Norm state
IDirect3DStateBlock9*	CRenderer::drawState		= 0;			// Draw state
IDirect3DStateBlock9*	CRenderer::drawState3d		= 0;			// Draw state for 3D mode
rectangle_t				CRenderer::clip;							// Clip rectangle
bool					CRenderer::clipping			= false;		// Is clipping enabled?
const CFont*			CRenderer::renderFont		= NULL;			// Temp storage for current font
LINE_STATUS				CRenderer::lineStatus		= LINE_IDLE;	// Current underline status
bool					CRenderer::lineContinue		= false;		// Continue drawing a line
float					CRenderer::zIndex			= 1.0f;			// Current Z-index (used with 3D rendering)
float					CRenderer::zBias			= 0.0f;			// Added z-bias (a dodgy fix for 3D z-fighting)
D3DVIEWPORT9			CRenderer::viewport;						// Default viewport
int32					CRenderer::drawOffsetX		= 0;			// X offset when drawing onto a texture
int32					CRenderer::drawOffsetY		= 0;			// Y offset when drawing onto a texture
CRenderTarget*			CRenderer::renderTarget		= NULL;			// Current render target
uint32					CRenderer::screenWidth		= 0;			// Render target (window or screen) width
uint32					CRenderer::screenHeight		= 0;			// Render target (window or screen) height

// --------------------------------------------------

void CRenderer::Initialize( void )
{
	// Create vertex buffers for 2D and 3D primitives
	d3dDevice->CreateVertexBuffer( MAX_VERT * sizeof(Vertex), D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_TEXTURED, D3DPOOL_DEFAULT, &vertexBuffer, NULL );
	d3dDevice->CreateVertexBuffer( MAX_VERT * sizeof(Vertex3D), D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC, D3DFVF_TEXTURED_3D, D3DPOOL_DEFAULT, &vertexBuffer3d, NULL );

	// Create a norm state block.
	d3dDevice->CreateStateBlock( D3DSBT_ALL, &normState );

	// Initialize our state blocks and set the states we're going to need for drawing the UI stuff.

	// State block for drawing 2D primitives
	d3dDevice->BeginStateBlock();
	{
		d3dDevice->SetPixelShader( NULL );
		d3dDevice->SetVertexShader( NULL );
		d3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
		d3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRS_ZENABLE, 0 );
		d3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		d3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		d3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		d3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		d3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		d3dDevice->SetRenderState( D3DRS_STENCILENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRS_CLIPPING, TRUE );
		d3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE );
		d3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	}
	d3dDevice->EndStateBlock( &drawState );

	// State block for drawing 3D primitives
	d3dDevice->BeginStateBlock();
	{
		d3dDevice->SetPixelShader( NULL );
		d3dDevice->SetVertexShader( NULL );
		d3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
		d3dDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
		d3dDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
		d3dDevice->SetRenderState( D3DRS_ZFUNC, D3DCMP_LESSEQUAL );
		d3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
		d3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		d3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
		d3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		d3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );
		d3dDevice->SetRenderState( D3DRS_STENCILENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRS_CLIPPING, TRUE );
		d3dDevice->SetRenderState( D3DRS_CLIPPLANEENABLE, FALSE );
		d3dDevice->SetRenderState( D3DRS_VERTEXBLEND, D3DVBF_DISABLE );
		d3dDevice->SetRenderState( D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
		d3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	}
	d3dDevice->EndStateBlock( &drawState3d );
}

void CRenderer::Shutdown( void )
{
	SAFE_RELEASE( vertexBuffer );
	SAFE_RELEASE( vertexBuffer3d );

	SAFE_RELEASE( normState );
	SAFE_RELEASE( drawState );
	SAFE_RELEASE( drawState3d );
}

void CRenderer::Begin( void )
{
	// Begin the scene.
	d3dDevice->BeginScene();
	d3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB( 0, 0, 0 ), 1, 0 );

	// Capture the current state block.
	normState->Capture();

	// Get the viewport in case we need to alter it (Z-fighting in 3D mode)
	d3dDevice->GetViewport( &viewport );
	zBias = 0.0f;

	// Begin drawing our UI scene.
	BeginDraw();
}

void CRenderer::End( void )
{
	Flush( true );

	// Restore the previous state block.
	normState->Apply();

	// Restore old viewport.
	if ( drawMode == DRAWING_3D )
		d3dDevice->SetViewport( &viewport );

	// Frame complete!
	d3dDevice->EndScene();
	d3dDevice->Present( NULL, NULL, NULL, NULL );
}

void CRenderer::Resize( uint32 w, uint32 h )
{
	extern D3DPRESENT_PARAMETERS params;

	screenWidth = w;
	screenHeight = h;

	params.BackBufferWidth = w;
	params.BackBufferHeight = h;
	
	CRenderer::Shutdown();
	d3dDevice->Reset( &params );
	CRenderer::Initialize();
}

DRAW_MODE CRenderer::SetDrawMode( DRAW_MODE mode )
{
	DRAW_MODE oldMode = drawMode;

	if ( mode != drawMode )
	{
		if ( drawMode == DRAWING_3D )
		{
			d3dDevice->SetViewport( &viewport );
		}

		Flush();
		drawMode = mode;
	}

	return oldMode;
}

void CRenderer::SetDrawColour( const colour_t* col )
{
	if ( col == NULL ) return;

	if ( renderTarget != NULL )
	{
		// This is a cache refresh, do not apply alpha, cached rendering will handle that.
		colour = 0xFF000000 | (col->r << 16) | (col->g << 8) | col->b;

		drawColour.hex = col->hex;
		drawColour.a = 0xFF;
	}
	else
	{
		colour = (col->a << 24) | (col->r << 16) | (col->g << 8) | col->b;
		drawColour.hex = col->hex;
	}
}

void CRenderer::SetDrawDepth( float zDepth )
{
	zIndex = zDepth;
}

void CRenderer::SetDrawTransform( const matrix4_t* mat )
{
	d3dDevice->GetTransform( D3DTS_WORLD, &transform );
	d3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)mat );
}

void CRenderer::ResetDrawTransform( void )
{
	d3dDevice->SetTransform( D3DTS_WORLD, &transform );
}

void CRenderer::StartClip( int32 x, int32 y, uint32 w, uint32 h )
{
	RECT r;

	Flush();

	clip.x = (int16)x;
	clip.y = (int16)y;
	clip.w = (uint16)w;
	clip.h = (uint16)h;
	clipping = true;

	r.left = (LONG)x - drawOffsetX;
	r.right = (LONG)( x + w );
	r.top = (LONG)y - drawOffsetY;
	r.bottom = (LONG)( y + h );

	d3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, TRUE );
	d3dDevice->SetScissorRect( &r );
}

void CRenderer::EndClip( void )
{
	Flush();

	clip.x = 0;
	clip.y = 0;
	clip.w = 0;
	clip.h = 0;
	clipping = false;

	d3dDevice->SetRenderState( D3DRS_SCISSORTESTENABLE, FALSE );
}

void CRenderer::DrawRect( int32 x, int32 y, uint32 w, uint32 h )
{
	if ( renderTexture )
	{
		Flush();
		renderTexture = NULL;
	}

	CheckVertexBuffer( 6 );
	UpdateVertexFormat( false );

	IncreaseZPlane();

	AddVertex( x, y );
	AddVertex( x+w, y );
	AddVertex( x, y+h );

	AddVertex( x+w, y );
	AddVertex( x+w, y+h );
	AddVertex( x, y+h );
}

void CRenderer::DrawTriangle( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 )
{
	if ( renderTexture )
	{
		Flush();
		renderTexture = NULL;
	}

	CheckVertexBuffer( 3 );
	UpdateVertexFormat( false );

	IncreaseZPlane();

	AddVertex( x1, y1 );
	AddVertex( x2, y2 );
	AddVertex( x3, y3 );
}

void CRenderer::DrawPixel( int32 x, int32 y )
{
	if ( renderTexture )
	{
		Flush();
		renderTexture = NULL;
	}

	CheckVertexBuffer( 6 );
	UpdateVertexFormat( false );

	IncreaseZPlane();

	AddVertex( x, y );
	AddVertex( x+1, y );
	AddVertex( x, y+1 );

	AddVertex( x+1, y );
	AddVertex( x+1, y+1 );
	AddVertex( x, y+1 );
}

MGuiRendTexture* CRenderer::LoadTexture( const char_t* path, uint32* width, uint32* height )
{
	CTexture* texture = NULL;
	D3DSURFACE_DESC desc;
	IDirect3DTexture9* data;
	HRESULT hr;

	// I was hoping I wouldn't have to use D3DX for this :(
	hr = D3DXCreateTextureFromFileEx( d3dDevice, path, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN,
									  D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &data );

	if ( SUCCEEDED( hr ) )
	{
		data->GetLevelDesc( 0, &desc );

		texture = new CTexture();

		texture->texture = data;
		texture->width = desc.Width;
		texture->height = desc.Height;

		if ( width ) *width = desc.Width;
		if ( height ) *height = desc.Height;
	}

	return (MGuiRendTexture*)texture;
}

void CRenderer::DestroyTexture( MGuiRendTexture* data )
{
	CTexture* texture = (CTexture*)data;

	if ( data == NULL ) return;

	SAFE_RELEASE( texture->texture );
	delete texture;
}

void CRenderer::DrawTexturedRect( const MGuiRendTexture* data, int32 x, int32 y, uint32 w, uint32 h, const float uv[] )
{
	CTexture* texture = (CTexture*)data;

	if ( data == NULL ) return;

	if ( renderTexture == NULL || renderTexture != texture->texture )
	{
		Flush();
		renderTexture = texture->texture;
	}

	CheckVertexBuffer( 6 );
	UpdateVertexFormat( true );

	IncreaseZPlane();

	AddVertexTextured( x, y, uv[0], uv[1] );
	AddVertexTextured( x+w, y, uv[2], uv[1] );
	AddVertexTextured( x, y+h, uv[0], uv[3] );

	AddVertexTextured( x+w, y, uv[2], uv[1] );
	AddVertexTextured( x+w, y+h, uv[2], uv[3] );
	AddVertexTextured( x, y+h, uv[0], uv[3] );
}

MGuiRendFont* CRenderer::LoadFont( const char_t* name, uint8 size, uint8 flags, uint8 charset, uint32 firstc, uint32 lastc )
{
	CFont* font;
	MGuiFontInfo info;

	font = new CFont();

	firstc = firstc > 0x20 ? firstc : 0x20;
	lastc = lastc > firstc ? lastc : 0xFF;

	lastc++;

	font->size = size;
	font->flags = flags;
	font->charset = charset;
	font->first_char = firstc;
	font->last_char = lastc;
	font->texDataLen = lastc - firstc;

	bool ret = mgui_load_font( name, size, flags, charset, firstc, lastc, &info, CreateFontTexture, (void*)font );
	if ( !ret )
	{
		delete font;
		return NULL;
	}

	font->texture->UnlockRect( 0 );

	font->texCoords = info.tex_coords;
	font->width = info.width;
	font->height = info.height;
	font->spacing = info.spacing;

	return (MGuiRendFont*)font;
}

void CRenderer::DestroyFont( MGuiRendFont* fnt )
{
	CFont* font;

	if ( fnt == NULL ) return;

	font = (CFont*)fnt;

	if ( font->texCoords != NULL )
	{
		mem_free( font->texCoords[0] );
		mem_free( font->texCoords );
	}

	SAFE_RELEASE( font->texture );

	delete font;
}

void CRenderer::DrawText( const MGuiRendFont* fnt, const char_t* text, int32 x, int32 y, uint32 flags, const MGuiFormatTag tags[], uint32 ntags )
{
	int32 dx, dy, lineX, lineY;
	uint32 c, ntag = 0, idx = 0;
	colour_t lineColour, defaultColour;
	register const char_t* s;
	const MGuiFormatTag* tag = NULL;
	const CFont* font = (const CFont*)fnt;

	if ( fnt == NULL || text == NULL ) return;

	lineStatus = LINE_IDLE;

	if ( renderTexture == NULL || renderTexture != font->texture )
	{
		Flush();
		renderTexture = font->texture;
	}

	IncreaseZPlane();
	UpdateVertexFormat( true );

	renderFont = font;
	defaultColour.hex = drawColour.hex;

	if ( tags && ntags > 0 ) tag = &tags[ntag];

	dx = x; lineX = dx;
	dy = y; lineY = dy;

	for ( s = text; *s; ++s, ++idx )
	{
		c = *(uchar_t*)s;

		// Process possible format tags for this index
		if ( tag && tag->index == idx )
		{
			ProcessTag( tag );
			ProcessUnderline( font, dx, dy, lineX, lineY, lineColour );

			if ( ++ntag < ntags ) tag = &tags[ntag];
			else tag = NULL;
		}

		if ( c < font->first_char || c > font->last_char )
		{
			continue;
		}
		else
		{
			dx += AddChar( c - font->first_char, dx, dy, flags );		
		}
	}

	Flush();

	// Finish the underline in case the end tag was missing
	if ( lineStatus == LINE_DRAWING )
	{
		lineStatus = LINE_DRAW;
		ProcessUnderline( font, dx, dy, lineX, lineY, lineColour );
	}

	// Reset back to default colour if the end tag was missing.
	if ( drawColour.hex != defaultColour.hex )
	{
		SetDrawColour( &defaultColour );
	}

	UpdateVertexFormat( false );

	renderTexture = NULL;
	renderFont = NULL;
}

void CRenderer::MeasureText( const MGuiRendFont* fnt, const char_t* text, uint32* x_out, uint32* y_out )
{
	float x, y, xout;
	float tmp1, tmp2;
	uint32 c;
	register const char_t* s;
	const CFont* font = (const CFont*)fnt;

	if ( font == NULL || text == NULL )
	{
		*x_out = 1;
		*y_out = 1;
		return;
	}

	x = xout = 0;
	y = (float)font->size;

	for ( s = text; *s; s++ )
	{
		c = *(uchar_t*)s;

		if ( c < font->first_char || c > font->last_char )
		{
			continue;
		}
		else
		{
			c -= font->first_char;
			tmp1 = font->texCoords[c][0];
			tmp2 = font->texCoords[c][2];

			x += ( tmp2 - tmp1 ) * font->width - 2 * font->spacing;
		}
	}	

	xout = math_max( xout, x );

	*x_out = (uint32)xout;
	*y_out = (uint32)y;
}

MGuiRendTarget* CRenderer::CreateRenderTarget( uint32 width, uint32 height )
{
	CRenderTarget* cache;
	uint32 w = 32, h = 32;
	HRESULT res;
	extern D3DPRESENT_PARAMETERS params;

	// This seems to fuck up big time if AA is enabled
	if ( params.MultiSampleType > D3DMULTISAMPLE_NONE ) return NULL;

	// Find suitable size for the actual render target (ideally a power of two)
	while ( w < width || h < height )
	{
		w <<= 1;
		h <<= 1;
	}

	cache = new CRenderTarget();

	cache->width = w;
	cache->height = h;
	cache->oldCache = NULL;
	cache->oldX = 0;
	cache->oldY = 0;
	cache->texture = NULL;
	cache->surface = NULL;
	cache->oldSurface = NULL;

	res = d3dDevice->CreateTexture( w, h, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &cache->texture, 0 );

	if ( FAILED( res ) ) goto cleanup;

	res = cache->texture->GetSurfaceLevel( 0, &cache->surface );
	if ( FAILED( res ) ) goto cleanup;

	return (MGuiRendTarget*)cache;

cleanup:
	SAFE_RELEASE( cache->surface );
	SAFE_RELEASE( cache->texture );

	delete cache;
	return NULL;
}

void CRenderer::DestroyRenderTarget( MGuiRendTarget* target )
{
	CRenderTarget* cache = (CRenderTarget*)target;

	if ( target == NULL ) return;

	SAFE_RELEASE( cache->surface );
	SAFE_RELEASE( cache->texture );

	delete cache;
}

void CRenderer::DrawRenderTarget( const MGuiRendTarget* target, int32 x, int32 y, uint32 w, uint32 h )
{
	CRenderTarget* cache = (CRenderTarget*)target;
	float u, v;

	if ( cache == NULL ) return;
	if ( cache->texture == NULL ) return;

	if ( renderTexture == NULL || renderTexture != cache->texture )
	{
		Flush();
		renderTexture = cache->texture;
	}

	CheckVertexBuffer( 6 );
	UpdateVertexFormat( true );

	IncreaseZPlane();

	u = (float)w / cache->width;
	v = (float)h / cache->height;

	AddVertexTextured( x, y, 0, 0 );
	AddVertexTextured( x+w, y, u, 0 );
	AddVertexTextured( x, y+h, 0, v );

	AddVertexTextured( x+w, y, u, 0 );
	AddVertexTextured( x+w, y+h, u, v );
	AddVertexTextured( x, y+h, 0, v );
}

void CRenderer::EnableRenderTarget( const MGuiRendTarget* target, int32 x, int32 y )
{
	CRenderTarget* cache = (CRenderTarget*)target;

	if ( target == NULL ) return;

	d3dDevice->GetRenderTarget( 0, &cache->oldSurface );

	if ( cache->oldSurface != cache->surface )
	{
		d3dDevice->SetRenderTarget( 0, cache->surface );
		d3dDevice->Clear( 0, 0, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0,0,0,0), 1.0f, 0 );

		cache->oldCache = renderTarget;
		cache->oldX = drawOffsetX;
		cache->oldY = drawOffsetY;

		drawOffsetX = x;
		drawOffsetY = y;
		renderTarget = cache;
		return;
	}
}

void CRenderer::DisableRenderTarget( const MGuiRendTarget* target )
{
	CRenderTarget* cache = (CRenderTarget*)target;

	if ( target == NULL ) return;

	if ( cache->oldCache != NULL )
	{
		d3dDevice->SetRenderTarget( 0, cache->oldCache->surface );

		drawOffsetX = cache->oldX;
		drawOffsetY = cache->oldY;
		renderTarget = cache->oldCache;

		SAFE_RELEASE( cache->oldSurface );
		return;
	}

	d3dDevice->SetRenderTarget( 0, cache->oldSurface );
	SAFE_RELEASE( cache->oldSurface );

	drawOffsetX = 0;
	drawOffsetY = 0;
	renderTarget = NULL;
}

void CRenderer::ScreenPosToWorld( const vector3_t* src, vector3_t* dst )
{
	matrix4_t tmp, tmp2;
	vector3_t vec;
	D3DMATRIX matView, matProj;

	if ( src == NULL || dst == NULL ) return;

	d3dDevice->GetTransform( D3DTS_PROJECTION, &matProj );
	d3dDevice->GetTransform( D3DTS_VIEW, &matView );

	matrix4_multiply( &tmp, (matrix4_t*)&matView, (matrix4_t*)&matProj );

	if ( matrix4_inverse( &tmp2, &tmp ) == 0.0f )
	{
		dst->x = dst->y = dst->z = 0.0f;
		return;
	}

	vec.x = 2.0f * ( src->x - viewport.X ) / viewport.Width - 1.0f;
	vec.y = 1.0f - 2.0f * ( src->y - viewport.Y ) / viewport.Height;
	vec.z = ( src->z - viewport.MinZ ) / ( viewport.MaxZ - viewport.MinZ );

	vector3_transform_coord( dst, &vec, &tmp2 );
}

void CRenderer::WorldPosToScreen( const vector3_t* src, vector3_t* dst )
{
	matrix4_t tmp;
	D3DMATRIX matView, matProj;

	if ( src == NULL || dst == NULL ) return;

	d3dDevice->GetTransform( D3DTS_PROJECTION, &matProj );
	d3dDevice->GetTransform( D3DTS_VIEW, &matView );

	matrix4_multiply( &tmp, (matrix4_t*)&matView, (matrix4_t*)&matProj );
	vector3_transform_coord( dst, src, (matrix4_t*)&tmp );

	dst->x = viewport.X + ( 1.0f + dst->x ) * viewport.Width / 2.0f;
	dst->y = viewport.Y + ( 1.0f - dst->y ) * viewport.Height / 2.0f;
	dst->z = viewport.MinZ + dst->z * ( viewport.MaxZ - viewport.MinZ );
}

void CRenderer::BeginDraw( void )
{
	if ( !vertex && lockBuffer )
	{
		vertexBuffer->Lock( 0, 0, (void**)&vertex, D3DLOCK_DISCARD );
		lockBuffer = false;
	}

	if ( !vertex3d && lockBuffer3d )
	{
		vertexBuffer3d->Lock( 0, 0, (void**)&vertex3d, D3DLOCK_DISCARD );
		lockBuffer3d = false;
	}
}

void CRenderer::EndDraw( void )
{
	switch ( drawMode )
	{
	case DRAWING_2D_DEPTH:
	case DRAWING_3D:
		drawState3d->Apply();
		break;

	default:
		drawState->Apply();
		break;
	}

	if ( numVertices )
	{
		vertexBuffer->Unlock();

		vertex = NULL;
		lockBuffer = true;

		d3dDevice->GetFVF( &fvfOld );
		d3dDevice->SetFVF( fvf );
		d3dDevice->SetTexture( 0, renderTexture );
		d3dDevice->SetStreamSource( 0, vertexBuffer, 0, sizeof(Vertex) );
		d3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, numVertices/3 );
		d3dDevice->SetFVF( fvfOld );
	}
	else if ( numVertices3d )
	{
		vertexBuffer3d->Unlock();

		vertex3d = NULL;
		lockBuffer3d = true;

		d3dDevice->GetFVF( &fvfOld );
		d3dDevice->SetFVF( fvf );
		d3dDevice->SetTexture( 0, renderTexture );
		d3dDevice->SetStreamSource( 0, vertexBuffer3d, 0, sizeof(Vertex3D) );
		d3dDevice->DrawPrimitive( D3DPT_TRIANGLELIST, 0, numVertices3d/3 );
		d3dDevice->SetFVF( fvfOld );
	}
}

// This here is a really dodgy fix for z-fighting in 3D mode. Because D3DRS_ZBIAS doesn't
// seem to work the same way everywhere (and it wouldn't work that well for us anyway),
// we'll move the viewport slightly in 3D mode to avoid z-fighting issues.
void CRenderer::IncreaseZPlane( void )
{
	static const float viewportBias	= 0.0000152588f;
	D3DVIEWPORT9 newViewport;

	if ( drawMode != DRAWING_3D ) return;

	newViewport = viewport;

	zBias += viewportBias;

	newViewport.MinZ -= zBias;
	newViewport.MaxZ -= zBias;

	d3dDevice->SetViewport( &newViewport );
}

inline void CRenderer::AddVertex( int32 x, int32 y )
{
	if ( renderTarget != NULL )
	{
		// Apply texture offset in case this is a cache refresh
		x -= drawOffsetX;
		y -= drawOffsetY;
	}

	switch ( drawMode )
	{
	case DRAWING_2D:
		AddVertex2D( x, y, 1.0f );
		break;

	case DRAWING_2D_DEPTH:
		AddVertex2D( x, y, zIndex );
		break;

	case DRAWING_3D:
		AddVertex3D( x, y, zIndex );
		break;
	}
}

inline void CRenderer::AddVertex2D( int32 x, int32 y, float z )
{
	vertex->x = (float)x;
	vertex->y = (float)y;
	vertex->z = z;
	vertex->rhw = 1.0f;
	vertex->col = colour;

	vertex++;
	numVertices++;
}

inline void CRenderer::AddVertex3D( int32 x, int32 y, float z )
{
	vertex3d->x = (float)x;
	vertex3d->y = (float)y;
	vertex3d->z = z;
	vertex3d->col = colour;

	vertex3d++;
	numVertices3d++;
}

inline void CRenderer::AddVertexTextured( int32 x, int32 y, float u, float v )
{
	if ( renderTarget != NULL )
	{
		// Offset drawing so our primitives start from (0,0)
		x -= drawOffsetX;
		y -= drawOffsetY;
	}

	switch ( drawMode )
	{
	case DRAWING_2D:
		AddVertexTextured2D( x, y, u, v, 1.0f );
		break;

	case DRAWING_2D_DEPTH:
		AddVertexTextured2D( x, y, u, v, zIndex );
		break;

	case DRAWING_3D:
		AddVertexTextured3D( x, y, u, v, zIndex );
		break;
	}
}

inline void CRenderer::AddVertexTextured2D( int32 x, int32 y, float u, float v, float z )
{
	vertex->x = -0.5f + (float)x;
	vertex->y = -0.5f + (float)y;
	vertex->z = z;

	vertex->u = u;
	vertex->v = v;

	vertex->rhw = 1.0f;
	vertex->col = colour;

	vertex++;
	numVertices++;
}

inline void CRenderer::AddVertexTextured3D( int32 x, int32 y, float u, float v, float z )
{
	vertex3d->x = -0.5f + (float)x;
	vertex3d->y = -0.5f + (float)y;
	vertex3d->z = z;

	vertex3d->u = u;
	vertex3d->v = v;

	vertex3d->col = colour;

	vertex3d++;
	numVertices3d++;
}

uint32 inline CRenderer::AddChar( uint32 c, int32 x, int32 y, uint32 flags )
{
	float tx1, ty1, tx2, ty2;
	uint32 w, h, spacing;
	colour_t col;
	rectangle_t r;

	CheckVertexBuffer( 12 );

	tx1 = renderFont->texCoords[c][0];
	tx2 = renderFont->texCoords[c][2];
	ty1 = 1.0f - renderFont->texCoords[c][1];
	ty2 = 1.0f - renderFont->texCoords[c][3];

	w = (uint32)( ( tx2 - tx1 ) * renderFont->width );
	h = (uint32)( ( ty1 - ty2 ) * renderFont->height );
	spacing = (uint32)renderFont->spacing * 2;

	r.x = (int16)x, r.y = (int16)y, r.w = (uint16)w, r.h = (uint16)h;

	if ( clipping && ( x < (int32)clip.x || (int32)(x+w) > (int32)clip.x+clip.w+spacing+1 || y < (int32)clip.y || (int32)(y+h) > (int32)clip.y+clip.h ) )
	{
		return ( w - spacing );
	}

	if ( flags & TFLAG_SHADOW )
	{
		colour_t shadowCol = colour_t( 0, 0, 0, drawColour.a );
		static const int32 shadowOffset = 1;

		col.hex = drawColour.hex;
		SetDrawColour( &shadowCol );

		AddVertexTextured( x+shadowOffset, y+shadowOffset, tx1, ty1 );
		AddVertexTextured( x+w+shadowOffset, y+shadowOffset, tx2, ty1 );
		AddVertexTextured( x+shadowOffset, y+h+shadowOffset, tx1, ty2 );

		AddVertexTextured( x+w+shadowOffset, y+shadowOffset, tx2, ty1 );
		AddVertexTextured( x+w+shadowOffset, y+h+shadowOffset, tx2, ty2 );
		AddVertexTextured( x+shadowOffset, y+h+shadowOffset, tx1, ty2 );

		SetDrawColour( &col );
	}

	AddVertexTextured( x, y, tx1, ty1 );
	AddVertexTextured( x+w, y, tx2, ty1 );
	AddVertexTextured( x, y+h, tx1, ty2 );

	AddVertexTextured( x+w, y, tx2, ty1 );
	AddVertexTextured( x+w, y+h, tx2, ty2 );
	AddVertexTextured( x, y+h, tx1, ty2 );

	return ( w - 2 * renderFont->spacing );
}

void CRenderer::ProcessTag( const MGuiFormatTag* tag )
{
	colour_t col;

	if ( tag->flags & TAG_COLOUR ||
		tag->flags & TAG_COLOUR_END )
	{
		SetDrawColour( &tag->colour );

		if ( lineStatus == LINE_DRAWING )
		{
			lineStatus = LINE_DRAW;
			lineContinue = true;
		}
	}

	if ( tag->flags & TAG_UNDERLINE )
	{
		lineStatus = LINE_BEGIN;
	}

	else if ( tag->flags & TAG_UNDERLINE_END )
	{
		switch ( lineStatus )
		{
		case LINE_DRAWING:
		case LINE_DRAW:
			lineStatus = LINE_DRAW;
			lineContinue = false;
			break;

		default:
			lineStatus = LINE_IDLE;
			break;
		}
	}
}

void CRenderer::ProcessUnderline( const CFont* font, int32 x, int32 y, int32& x2, int32& y2, colour_t& lineColour )
{
	colour_t col;

	switch ( lineStatus )
	{
	case LINE_BEGIN:
		x2 = x;
		y2 = y + font->size + 2;

		lineColour = drawColour;
		lineStatus = LINE_DRAWING;
		break;

	case LINE_DRAW:
		Flush();

		col = drawColour;
		SetDrawColour( &lineColour );

		DrawRect( x2, y2, x - x2, 1 );
		Flush();

		UpdateVertexFormat( true );
		renderTexture = font->texture;

		SetDrawColour( &col );

		if ( lineContinue )
		{
			lineStatus = LINE_DRAWING;
			lineContinue = false;

			x2 = x;
			y2 = y + font->size;
		}
		else
		{
			lineStatus = LINE_IDLE;
		}

		lineColour = colour;
		break;
	}
}

inline void CRenderer::CheckVertexBuffer( uint32 required )
{
	switch ( drawMode )
	{
	case DRAWING_2D:
	case DRAWING_2D_DEPTH:
		if ( numVertices >= MAX_VERT - required - 1 ) Flush();
		break;

	case DRAWING_3D:
		if ( numVertices3d >= MAX_VERT - required - 1 ) Flush();
		break;
	}
}

inline void CRenderer::UpdateVertexFormat( bool textured )
{
	if ( textured )
	{
		switch ( drawMode )
		{
		case DRAWING_2D:
		case DRAWING_2D_DEPTH:
			fvf = D3DFVF_TEXTURED;
			break;

		case DRAWING_3D:
			fvf = D3DFVF_TEXTURED_3D;
			break;
		}
	}
	else
	{
		switch ( drawMode )
		{
		case DRAWING_2D:
		case DRAWING_2D_DEPTH:
			fvf = D3DFVF_PRIMITIVE;
			break;

		case DRAWING_3D:
			fvf = D3DFVF_PRIMITIVE_3D;
			break;
		}
	}
}

void CRenderer::Flush( bool endDrawing )
{
	if ( numVertices == 0 && numVertices3d == 0 ) return;

	EndDraw();

	if ( !endDrawing ) BeginDraw();

	numVertices = 0;
	numVertices3d = 0;
}

void CRenderer::CreateFontTexture( void* fontData, uint32 width, uint32 height, void** texture, uint32* texturePitch )
{
	HRESULT hr;
	D3DLOCKED_RECT lockRect;
	CFont* font = (CFont*)fontData;

	// Create a texture which will contain the font glyphs
	hr = d3dDevice->CreateTexture( width, height, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, &font->texture, 0 );
	if ( FAILED( hr ) ) return;

	font->texture->LockRect( 0, &lockRect, 0, 0 );

	*texture = lockRect.pBits;
	*texturePitch = (uint32)lockRect.Pitch;
}
