/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (GDI+)
 * FILE:		Drawing.cpp
 * LICENCE:		See Licence.txt
 * PURPOSE:		GDI+ GUI drawing functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "Drawing.h"
#include "stdtypes.h"
#include "Stringy/Stringy.h"
#include <gdiplus.h>

static Gdiplus::Graphics*	graphics	= NULL;
static Gdiplus::Bitmap*		bitmap		= NULL;
static HDC					hdc			= NULL;
static int					width		= 0;
static int					height		= 0;
extern HWND					hwnd;
static Gdiplus::Color		colour;
static Gdiplus::Color		shadow_col	= Gdiplus::Color( 125, 0, 0, 0 );

#define SHADOW_OFFSET 1 // Text shadow offset in pixels

#define SAFE_DELETE(x) \
	if ( x ) {         \
		delete x;      \
		x = NULL;      \
	}


static void __mgui_gdiplus_destroy_buffer( void )
{
	SAFE_DELETE( bitmap );
	SAFE_DELETE( graphics );
}

void mgui_gdiplus_begin( void )
{
	RECT r;
	int w, h;

	hdc = GetDC( hwnd );
	Gdiplus::Graphics gfx( hdc );

	GetClientRect( hwnd, &r );

	w = r.right - r.left;
	h = r.bottom - r.top;

	if ( width != w || height != h ) __mgui_gdiplus_destroy_buffer();

	width = w;
	height = h;

	if ( bitmap ) return;

	bitmap = new Gdiplus::Bitmap( width, height, &gfx );
	graphics = Gdiplus::Graphics::FromImage( bitmap );
}

void mgui_gdiplus_end( void )
{
	Gdiplus::Graphics gfx( hdc );
	gfx.DrawImage( bitmap, 0, 0 );

	// Draw background
	graphics->Clear( Gdiplus::Color( 255, 0, 0, 0 ) );

	ReleaseDC( hwnd, hdc );
	hdc = NULL;
}

void mgui_gdiplus_resize( uint32 w, uint32 h )
{
	UNREFERENCED_PARAM(w);
	UNREFERENCED_PARAM(h);
}

void mgui_gdiplus_set_draw_colour( const colour_t* col )
{
	colour = Gdiplus::Color( col->a, col->r, col->g, col->b );
}

void mgui_gdiplus_start_clip( int32 x, int32 y, uint32 w, uint32 h )
{
	graphics->SetClip( Gdiplus::Rect( x, y, w, h ), Gdiplus::CombineModeReplace );
}

void mgui_gdiplus_end_clip( void )
{
	graphics->ResetClip();
}

void mgui_gdiplus_draw_rect( int32 x, int32 y, uint32 w, uint32 h )
{
	Gdiplus::SolidBrush brush( colour );

	graphics->FillRectangle( &brush, (INT)x, (INT)y, (INT)w, (INT)h );
}

void mgui_gdiplus_draw_triangle( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 )
{
	Gdiplus::SolidBrush brush( colour );
	Gdiplus::Point points[] = { Gdiplus::Point(x1, y1), Gdiplus::Point(x2, y2), Gdiplus::Point(x3, y3) };

	graphics->FillPolygon( &brush, points, 3 );
}

void* mgui_gdiplus_load_texture( const char* path )
{
	UNREFERENCED_PARAM(path);
	return NULL;
}

void mgui_gdiplus_destroy_texture( void* texture )
{
	UNREFERENCED_PARAM(texture);
}

void mgui_gdiplus_draw_textured_rect( void* texture, int32 x, int32 y, uint32 w, uint32 h )
{
	UNREFERENCED_PARAM(texture);
	UNREFERENCED_PARAM(x);
	UNREFERENCED_PARAM(y);
	UNREFERENCED_PARAM(w);
	UNREFERENCED_PARAM(h);
}

void* mgui_gdiplus_load_font( const char* name, uint32 size, uint32 flags, uint32 charset, uint32 firstc, uint32 lastc )
{
	MGuiGDIFont* font;
	int style = 0;

	UNREFERENCED_PARAM(charset);
	UNREFERENCED_PARAM(firstc);
	UNREFERENCED_PARAM(lastc);

	font = new MGuiGDIFont;

	font->size = size;
	font->flags = flags;

	if ( BIT_ON( flags, FFLAG_BOLD ) )		style |= Gdiplus::FontStyleBold;
	if ( BIT_ON( flags, FFLAG_ITALIC ) )	style |= Gdiplus::FontStyleItalic;
	if ( BIT_ON( flags, FFLAG_ULINE ) )		style |= Gdiplus::FontStyleUnderline;
	if ( BIT_ON( flags, FFLAG_STRIKE ) )	style |= Gdiplus::FontStyleStrikeout;

#ifndef MGUI_UNICODE
	// BS GDI+ doesnt support character strings
	wchar_t wname[128];
	mbstowcs( wname, name, lengthof(wname) );
	
	font->font = new Gdiplus::Font( wname, (float)size, style, Gdiplus::UnitPixel, NULL );
#else
	font->font = new Gdiplus::Font( name, (float)size, style, Gdiplus::UnitPixel, NULL );
#endif

	return font;
}

void mgui_gdiplus_destroy_font( void* font )
{
	MGuiGDIFont* fnt;
	fnt = (MGuiGDIFont*)font;

	if ( fnt->font )
	{
		Gdiplus::Font* f = (Gdiplus::Font*)fnt->font;
		delete f;
	}

	delete fnt;
}

void mgui_gdiplus_draw_text( void* font, const char_t* text, int32 x, int32 y, uint32 flags )
{
	MGuiGDIFont* fnt;

	fnt = (MGuiGDIFont*)font;

	if ( font == NULL || text == NULL ) return;

	Gdiplus::StringFormat format( Gdiplus::StringFormat::GenericDefault() );
	format.SetAlignment( Gdiplus::StringAlignmentNear );
	format.SetLineAlignment( Gdiplus::StringAlignmentNear );

	Gdiplus::SolidBrush brush( colour );

	y -= 1;
	Gdiplus::RectF r( (float)x, (float)y, 1000, 1000 );

#ifndef MGUI_UNICODE
	wchar_t wtext[512];
	mbstowcs( wtext, text, lengthof(wtext) );

	if ( flags & FLAG_TEXT_SHADOW )
	{
		Gdiplus::RectF rshadow( (float)x + SHADOW_OFFSET, (float)( y + SHADOW_OFFSET ), 1000, 1000 );
		Gdiplus::SolidBrush shadowbrush( shadow_col );

		graphics->DrawString( wtext, -1, (Gdiplus::Font*)fnt->font, rshadow, &format, &shadowbrush );
	}

	graphics->DrawString( wtext, -1, (Gdiplus::Font*)fnt->font, r, &format, &brush );
#else
	if ( flags & FFLAG_SHADOW )
	{
		Gdiplus::RectF rshadow( (float)x + SHADOW_OFFSET, (float)( y + SHADOW_OFFSET ), 1000, 1000 );
		Gdiplus::SolidBrush shadowbrush( shadow_col );

		graphics->DrawString( text, len-1, (Gdiplus::Font*)fnt->font, rshadow, &format, &shadowbrush );
	}

	graphics->DrawString( text, -1, (Gdiplus::Font*)fnt->font, r, &format, &brush );
#endif /* MGUI_UNICODE */
}

void mgui_gdiplus_measure_text( void* font, const char_t* text, uint32* w, uint32* h )
{
	Gdiplus::SizeF size;
	Gdiplus::Graphics gfx(hwnd);
	MGuiGDIFont* fnt;

	fnt = (MGuiGDIFont*)font;

	if ( !text ) return;

	Gdiplus::StringFormat format( Gdiplus::StringFormat::GenericDefault() );
	format.SetAlignment( Gdiplus::StringAlignmentNear );
	format.SetLineAlignment( Gdiplus::StringAlignmentNear );
	format.SetFormatFlags( Gdiplus::StringFormatFlagsMeasureTrailingSpaces );

#ifndef MGUI_UNICODE
	wchar_t wtext[512];
	mbstowcs( wtext, text, lengthof(wtext) );

	gfx.MeasureString( wtext, -1, (Gdiplus::Font*)fnt->font, Gdiplus::SizeF( 10000, 10000 ), &format, &size );
#else
	gfx.MeasureString( text, -1, (Gdiplus::Font*)fnt->font, Gdiplus::SizeF( 10000, 10000 ), &format, &size );
#endif
	
	*w = (uint32)size.Width;
	*h = (uint32)fnt->size;
}
