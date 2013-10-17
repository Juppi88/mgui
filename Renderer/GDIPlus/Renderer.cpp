/**********************************************************************
 *
 * PROJECT:		Mylly GUI - GDI+ Renderer
 * FILE:		Renderer.cpp
 * LICENCE:		See Licence.txt
 * PURPOSE:		A GDI+ reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "Renderer.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

uint32					CRenderer::width			= 0;
uint32					CRenderer::height			= 0;
HDC						CRenderer::deviceContext	= NULL;
Gdiplus::Bitmap*		CRenderer::bitmap			= NULL;
Gdiplus::Graphics*		CRenderer::graphics			= NULL;
Gdiplus::Graphics*		CRenderer::rendererGraphics	= NULL;
Gdiplus::SolidBrush*	CRenderer::drawBrush		= NULL;
Gdiplus::SolidBrush*	CRenderer::shadowBrush		= NULL;
Gdiplus::Color			CRenderer::drawColour		= Gdiplus::Color( 0, 0, 0 );
LINE_STATUS				CRenderer::lineStatus		= LINE_IDLE;
bool					CRenderer::lineContinue		= false;
colour_t				CRenderer::curDrawColour	= colour_t( 0, 0, 0 );
int32					CRenderer::drawOffsetX		= 0;
int32					CRenderer::drawOffsetY		= 0;

// --------------------------------------------------

class CFont : MGuiRendFont
{
	friend class CRenderer;

public:
	CFont(){}
	~CFont(){}

private:
	Gdiplus::Font*	font;
};

// --------------------------------------------------

class CTexture : MGuiRendTexture
{
	friend class CRenderer;

public:
	CTexture(){}
	~CTexture(){}

private:
	Gdiplus::Bitmap*	bitmap;
};

// --------------------------------------------------

class CRenderTarget : MGuiRendTarget
{
	friend class CRenderer;

public:
	CRenderTarget(){}
	~CRenderTarget(){}

private:
	Gdiplus::Bitmap*	bitmap;
	Gdiplus::Graphics*	graphics;
	Gdiplus::Graphics*	oldGraphics;
	int32				oldX;
	int32				oldY;
};

// --------------------------------------------------

void CRenderer::Initialize( void )
{
	RECT r;
	extern HWND hwnd;

	GetClientRect( hwnd, &r );

	width = r.right - r.left;
	height = r.bottom - r.top;

	deviceContext = GetDC( hwnd );
	Gdiplus::Graphics gfx( deviceContext );

	bitmap = new Gdiplus::Bitmap( width, height, &gfx );
	rendererGraphics = Gdiplus::Graphics::FromImage( bitmap );
	graphics = rendererGraphics;

	drawBrush = new Gdiplus::SolidBrush( drawColour );
	shadowBrush = new Gdiplus::SolidBrush( Gdiplus::Color( 127, 0, 0, 0 ) );
}

void CRenderer::Shutdown( void )
{
	extern HWND hwnd;

	SAFE_DELETE_CPP( drawBrush );
	SAFE_DELETE_CPP( shadowBrush );
	SAFE_DELETE_CPP( bitmap );
	SAFE_DELETE_CPP( rendererGraphics );

	ReleaseDC( hwnd, deviceContext );
}

void CRenderer::Begin( void )
{
}

void CRenderer::End( void )
{
	Gdiplus::Graphics gfx( deviceContext );
	gfx.DrawImage( bitmap, 0, 0 );

	graphics->Clear( Gdiplus::Color( 255, 0, 0, 0 ) );
}

void CRenderer::Resize( uint32 w, uint32 h )
{
	width = w;
	height = h;

	if ( graphics == rendererGraphics )
		graphics = NULL;

	// Re-create the graphics context
	delete rendererGraphics;
	delete bitmap;

	Gdiplus::Graphics gfx( deviceContext );

	bitmap = new Gdiplus::Bitmap( width, height, &gfx );
	rendererGraphics = Gdiplus::Graphics::FromImage( bitmap );

	if ( graphics == NULL )
		graphics = rendererGraphics;
}

DRAW_MODE CRenderer::SetDrawMode( DRAW_MODE mode )
{
	// 3D drawing is not supported in GDI+.
	return mode;
}

void CRenderer::SetDrawColour( const colour_t* col )
{
	drawColour = Gdiplus::Color( col->a, col->r, col->g, col->b );
	drawBrush->SetColor( drawColour );

	curDrawColour = *col;
}

void CRenderer::SetDrawDepth( float zDepth )
{
	// 3D drawing is not supported in GDI+.
	UNREFERENCED_PARAM( zDepth );
}

void CRenderer::SetDrawTransform( const matrix4_t* mat )
{
	// 3D drawing is not supported in GDI+.
	UNREFERENCED_PARAM( mat );
}

void CRenderer::ResetDrawTransform( void )
{
	// 3D drawing is not supported in GDI+.
	return;
}

void CRenderer::StartClip( int32 x, int32 y, uint32 w, uint32 h )
{
	CalculateOffset( x, y );
	graphics->SetClip( Gdiplus::Rect( x, y, w, h ) );
}

void CRenderer::EndClip( void )
{
	graphics->ResetClip();
}

void CRenderer::DrawRect( int32 x, int32 y, uint32 w, uint32 h )
{
	CalculateOffset( x, y );
	graphics->FillRectangle( drawBrush, x, y, (INT)w, (INT)h );
}

void CRenderer::DrawTriangle( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 )
{
	CalculateOffset( x1, y1 );
	CalculateOffset( x2, y2 );
	CalculateOffset( x3, y3 );

	Gdiplus::Point points[] = { Gdiplus::Point( x1, y1 ), Gdiplus::Point( x2, y2 ), Gdiplus::Point( x3, y3 ) };

	graphics->FillPolygon( drawBrush, points, 3 );
}

void CRenderer::DrawPixel( int32 x, int32 y )
{
	CalculateOffset( x, y );
	graphics->FillRectangle( drawBrush, x, y, 1, 1 );
}

MGuiRendTexture* CRenderer::LoadTexture( const char_t* path, uint32* width, uint32* height )
{
	CTexture* texture;
	wchar_t* fileName;

	if ( path == NULL ||
		 width == NULL ||
		 height == NULL )
		 return NULL;

#ifndef MGUI_UNICODE
	wchar_t file[MAX_PATH];
	mbstowcs( file, path, sizeof(file)/sizeof(char_t) );

	fileName = file;
#else
	fileName = path;
#endif

	texture = new CTexture();

	texture->bitmap = Gdiplus::Bitmap::FromFile( fileName, FALSE );
	if ( texture->bitmap == NULL )
	{
		delete texture;
		return NULL;
	}

	*width = texture->width = texture->bitmap->GetWidth();
	*height = texture->height = texture->bitmap->GetHeight();

	return texture;
}

void CRenderer::DestroyTexture( MGuiRendTexture* tex )
{
	CTexture* texture = (CTexture*)tex;

	if ( texture == NULL ) return;
		 
	SAFE_DELETE_CPP( texture->bitmap );
	delete texture;
}

void CRenderer::DrawTexturedRect( const MGuiRendTexture* tex, int32 x, int32 y, uint32 w, uint32 h, const float uv[] )
{
	Gdiplus::RectF target, source;
	Gdiplus::ImageAttributes attr;
	uint32 width, height;
	CTexture* texture = (CTexture*)tex;

	if ( texture == NULL ) return;
	if ( texture->bitmap == NULL ) return;

	CalculateOffset( x, y );

	width = texture->width;
	height = texture->height;

	target = Gdiplus::RectF( (float)x, (float)y, (float)w, (float)h );
	source = Gdiplus::RectF( width * uv[0], height * uv[1], width * ( uv[2] - uv[0] ), height * ( uv[3] - uv[1] ) );

	Gdiplus::ColorMatrix mat = {
		(float)curDrawColour.r / 255, 0, 0, 0, 0,
		0, (float)curDrawColour.g / 255, 0, 0, 0,
		0, 0, (float)curDrawColour.b / 255, 0, 0,
		0, 0, 0, (float)curDrawColour.a / 255, 0,
		0, 0, 0, 0, 1
	};

	attr.SetColorMatrix( &mat );

	graphics->DrawImage( texture->bitmap, target, source.X, source.Y, source.Width, source.Height, Gdiplus::UnitPixel, &attr );
}

MGuiRendFont* CRenderer::LoadFont( const char_t* name, uint8 size, uint8 flags, uint8 charset, uint32 firstc, uint32 lastc )
{
	CFont* font;
	int style = 0;

	UNREFERENCED_PARAM( charset );
	UNREFERENCED_PARAM( firstc );
	UNREFERENCED_PARAM( lastc );

	font = new CFont();

	font->size = size;
	font->flags = flags;
	font->first_char = 0;
	font->last_char = 0;
	font->charset = 0;

	if ( flags & FFLAG_BOLD )	style |= Gdiplus::FontStyleBold;
	if ( flags & FFLAG_ITALIC )	style |= Gdiplus::FontStyleItalic;
	if ( flags & FFLAG_ULINE )	style |= Gdiplus::FontStyleUnderline;
	if ( flags & FFLAG_STRIKE )	style |= Gdiplus::FontStyleStrikeout;

#ifndef MGUI_UNICODE
	wchar_t fontName[128];
	mbstowcs( fontName, name, sizeof(fontName)/sizeof(char_t) );

	font->font = new Gdiplus::Font( fontName, (float)size, style, Gdiplus::UnitPixel, NULL );
#else
	font->font = new Gdiplus::Font( name, (float)size, style, Gdiplus::UnitPixel, NULL );
#endif

	return (MGuiRendFont*)font;
}

void CRenderer::DestroyFont( MGuiRendFont* fnt )
{
	CFont* font = (CFont*)fnt;

	if ( font == NULL ) return;

	SAFE_DELETE_CPP( font->font );
	delete fnt;
}

void CRenderer::DrawText( const MGuiRendFont* fnt, const char_t* text, int32 x, int32 y, uint32 flags, const MGuiFormatTag tags[], uint32 ntags )
{
	const CFont* font = (const CFont*)fnt;
	uint32 ntag = 0, idx = 0;
	int32 dx, dy, lineX, lineY;
	colour_t lineColour, defaultColour;
	const MGuiFormatTag* tag;
	register const char_t* s;
	char_t* t;
	uchar_t c;
	char_t tmp[1024] = { 0 };

	if ( font == NULL || text == NULL ) return;

	CalculateOffset( x, y );

	if ( tags != NULL && ntags > 0 )
	{
		tag = &tags[ntag];
		defaultColour.hex = curDrawColour.hex;

		dx = x; lineX = dx;
		dy = y; lineY = dy;

		for ( s = text, t = tmp; *s; ++s, ++idx )
		{
			c = *(uchar_t*)s;

			if ( tag && tag->index == idx )
			{
				// Tag change, draw the text in the temp buffer
				if ( *tmp != '\0' )
				{
					// Silly GDI+ seems to ignore trailing spaces. This is not a good way to fix it! (But I'm doing it anyway)
					// TODO: Figure out a better fix for this!
					if ( *(s-1) == ' ' ) { *t++ = ' '; }

					*t = '\0';
					DrawBuffer( font, tmp, dx, dy, flags, true );
				}

				// Process possible format tags for this index
				ProcessTag( tag );
				ProcessUnderline( font, dx, dy, lineX, lineY, lineColour );

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
			DrawBuffer( font, tmp, dx, dy, flags, true );
		}

		// Process tags one last time in case someone forgot the ending tag.
		if ( lineStatus == LINE_DRAWING )
		{
			lineStatus = LINE_DRAW;
			ProcessUnderline( font, dx, dy, lineX, lineY, lineColour );
		}

		// Reset back to default colour if the end tag was missing.
		if ( curDrawColour.hex != defaultColour.hex )
		{
			SetDrawColour( &defaultColour );
		}
	}
	else
	{
		// No tags to process, just draw the damn text.
		DrawBuffer( font, text, x, y, flags );
	}
}

void CRenderer::MeasureText( const MGuiRendFont* fnt, const char_t* text, uint32* w, uint32* h )
{
	extern HWND hwnd;
	Gdiplus::SizeF size;
	wchar_t* out;
	CFont* font = (CFont*)fnt;

	if ( w == NULL || h == NULL )
		return;

	if ( font == NULL || 
		 text == NULL )
	{
		*w = *h = 1;
		return;
	}

	Gdiplus::StringFormat format( Gdiplus::StringFormat::GenericDefault() );

	format.SetAlignment( Gdiplus::StringAlignmentNear );
	format.SetLineAlignment( Gdiplus::StringAlignmentNear );
	format.SetFormatFlags( Gdiplus::StringFormatFlagsMeasureTrailingSpaces );

#ifndef MGUI_UNICODE
	wchar_t wideText[1024];
	mbstowcs( wideText, text, sizeof(wideText) / sizeof(wchar_t) );
	out = wideText;
#else
	out = text;
#endif

	graphics->MeasureString( out, -1, font->font, Gdiplus::SizeF( 10000, 10000 ), &format, &size );

	*w = (uint32)size.Width;
	*h = (uint32)font->size;
}

MGuiRendTarget* CRenderer::CreateRenderTarget( uint32 width, uint32 height )
{
	CRenderTarget* cache;

	cache = new CRenderTarget();

	cache->width = width;
	cache->height = height;

	cache->bitmap = new Gdiplus::Bitmap( width, height );
	cache->graphics = Gdiplus::Graphics::FromImage( cache->bitmap );
	cache->oldGraphics = NULL;
	
	return (MGuiRendTarget*)cache;
}

void CRenderer::DestroyRenderTarget( MGuiRendTarget* target )
{
	CRenderTarget* cache = (CRenderTarget*)target;

	if ( cache == NULL ) return;

	SAFE_DELETE_CPP( cache->graphics );
	SAFE_DELETE_CPP( cache->bitmap );
	delete cache;
}

void CRenderer::DrawRenderTarget( const MGuiRendTarget* target, int32 x, int32 y, uint32 w, uint32 h )
{
	CRenderTarget* cache = (CRenderTarget*)target;
	Gdiplus::RectF area;

	if ( cache == NULL ) return;
	if ( cache->bitmap == NULL ) return;

	area = Gdiplus::RectF( (float)x, (float)y, (float)w, (float)h );
	
	graphics->DrawImage( cache->bitmap, area, 0, 0, (Gdiplus::REAL)w, (Gdiplus::REAL)h, Gdiplus::UnitPixel );
}

void CRenderer::EnableRenderTarget( const MGuiRendTarget* target, int32 x, int32 y )
{
	CRenderTarget* cache = (CRenderTarget*)target;

	if ( cache == NULL ) return;
	if ( cache->graphics == NULL ) return;

	cache->oldGraphics = graphics;
	graphics = cache->graphics;

	cache->oldX = drawOffsetX;
	cache->oldY = drawOffsetY;
	drawOffsetX = x;
	drawOffsetY = y;

	graphics->Clear( Gdiplus::Color( 255, 0, 0, 0 ) );
}

void CRenderer::DisableRenderTarget( const MGuiRendTarget* target )
{
	CRenderTarget* cache = (CRenderTarget*)target;

	if ( cache == NULL ) return;

	graphics = cache->oldGraphics;

	if ( graphics == rendererGraphics )
	{
		drawOffsetX = cache->oldX;
		drawOffsetY = cache->oldY;
	}
	else
	{
		drawOffsetX = 0;
		drawOffsetY = 0;
	}
}

void CRenderer::ScreenPosToWorld( const vector3_t* src, vector3_t* dst )
{
	// 3D drawing is not supported in GDI+.
	UNREFERENCED_PARAM( src );

	if ( dst != NULL )
	{
		dst->x = dst->y = dst->z = 0;
		return;
	}
}

void CRenderer::WorldPosToScreen( const vector3_t* src, vector3_t* dst )
{
	// 3D drawing is not supported in GDI+.
	UNREFERENCED_PARAM( src );

	if ( dst != NULL )
	{
		dst->x = dst->y = dst->z = 0;
		return;
	}
}

inline void CRenderer::CalculateOffset( int32& x, int32& y )
{
	if ( graphics != rendererGraphics )
	{
		x -= drawOffsetX;
		y -= drawOffsetY;
	}
}

void CRenderer::DrawBuffer( const CFont* font, const char_t* text, int32& x, int32& y, uint32 flags, bool measure )
{
	static const int32 SHADOW_OFFSET = 1;
	Gdiplus::SizeF size;
	Gdiplus::RectF shadow, rect;
	Gdiplus::StringFormat format( Gdiplus::StringFormat::GenericDefault() );
	wchar_t* out;

	format.SetAlignment( Gdiplus::StringAlignmentNear );
	format.SetLineAlignment( Gdiplus::StringAlignmentNear );
	format.SetFormatFlags( Gdiplus::StringFormatFlagsMeasureTrailingSpaces );

	rect = Gdiplus::RectF( (float)x, (float)y-1, 1000, 1000 );

#ifndef MGUI_UNICODE
	wchar_t wideText[1024];
	mbstowcs( wideText, text, sizeof(wideText) / sizeof(wchar_t) );
	out = wideText;
#else
	out = text;
#endif

	if ( flags & FLAG_TEXT_SHADOW )
	{
		shadow = Gdiplus::RectF( (float)x + SHADOW_OFFSET, (float)( y + SHADOW_OFFSET - 1 ), 1000, 1000 );
		graphics->DrawString( out, -1, font->font, shadow, &format, shadowBrush );
	}

	graphics->DrawString( out, -1, font->font, rect, &format, drawBrush );

	if ( measure )
	{
		graphics->MeasureString( out, -1, font->font, Gdiplus::SizeF( 10000, 10000 ), &format, &size );
		x += (int32)size.Width - (int32)size.Height/3; // This is slightly (only slightly?) dodgy, but that gorram GDI+ likes to add padding to the size.
	}
}

void CRenderer::ProcessTag( const MGuiFormatTag* tag )
{
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
		x2 = x + font->size / 3;
		y2 = y + font->size + 2;

		lineColour = curDrawColour;
		lineStatus = LINE_DRAWING;
		break;

	case LINE_DRAW:
		col = curDrawColour;
		SetDrawColour( &lineColour );

		DrawRect( x2, y2, x - x2 + font->size / 3, 1 );

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

		lineColour = curDrawColour;
		break;
	}
}
