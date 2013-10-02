/**********************************************************************
 *
 * PROJECT:		Mylly GUI - GDI+ Renderer
 * FILE:		Renderer.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A GDI+ reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_GDIPLUS_RENDERER_H
#define __MYLLY_GUI_GDIPLUS_RENDERER_H

#include "WinGDIPlus.h"
#include <gdiplus.h>

#define SAFE_RELEASE(x) if ( x ) { (x)->Release(); x = NULL; }

// Used for drawing an underline for text (format tags)
enum LINE_STATUS {
	LINE_IDLE,
	LINE_BEGIN,
	LINE_DRAWING,
	LINE_DRAW,
};

class CFont;

class CRenderer
{
public:
	// Renderer initialization and shutdown
	static void					Initialize				( void );
	static void					Shutdown				( void );

	// Actual renderer methods
	static void					Begin					( void );
	static void					End						( void );
	static void					Resize					( uint32 w, uint32 h );

	static DRAW_MODE			SetDrawMode				( DRAW_MODE mode );
	static void					SetDrawColour			( const colour_t* col );
	static void					SetDrawDepth			( float zDepth );
	static void					SetDrawTransform		( const matrix4_t* mat );
	static void					ResetDrawTransform		( void );

	static void					StartClip				( int32 x, int32 y, uint32 w, uint32 h );
	static void					EndClip					( void );

	static void					DrawRect				( int32 x, int32 y, uint32 w, uint32 h );
	static void					DrawTriangle			( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 );
	static void					DrawPixel				( int32 x, int32 y );

	static MGuiRendTexture*		LoadTexture				( const char_t* path, uint32* width, uint32* height );
	static void					DestroyTexture			( MGuiRendTexture* texture );
	static void					DrawTexturedRect		( const MGuiRendTexture* texture, int32 x, int32 y, uint32 w, uint32 h, const float uv[] );

	static MGuiRendFont*		LoadFont				( const char_t* name, uint8 size, uint8 flags, uint8 charset, uint32 firstc, uint32 lastc );
	static void					DestroyFont				( MGuiRendFont* font );
	static void					DrawText				( const MGuiRendFont* font, const char_t* text, int32 x, int32 y, uint32 flags, const MGuiFormatTag tags[], uint32 ntags );
	static void					MeasureText				( const MGuiRendFont* font, const char_t* text, uint32* w, uint32* h );

	static MGuiRendTarget*		CreateRenderTarget		( uint32 width, uint32 height );
	static void					DestroyRenderTarget		( MGuiRendTarget* target );
	static void					DrawRenderTarget		( const MGuiRendTarget* target, int32 x, int32 y, uint32 w, uint32 h );
	static void					EnableRenderTarget		( const MGuiRendTarget* target, int32 x, int32 y );
	static void					DisableRenderTarget		( const MGuiRendTarget* target );

	static void					ScreenPosToWorld		( const vector3_t* src, vector3_t* dst );
	static void					WorldPosToScreen		( const vector3_t* src, vector3_t* dst );

private:
	static inline void			CalculateOffset			( int32& x, int32& y );
	static void					DrawBuffer				( const CFont* font, const char_t* text, int32& x, int32& y, uint32 flags, bool measure = false );
	static void					ProcessTag				( const MGuiFormatTag* tag );
	static void					ProcessUnderline		( const CFont* font, int32 x, int32 y, int32& x2, int32& y2, colour_t& lineColour );

private:
	static uint32				width;					// GDI+ drawing area width
	static uint32				height;					// GDI+ drawing area height
	static HDC					deviceContext;			// Device context
	static Gdiplus::Bitmap*		bitmap;					// Bitmap we're drawing to
	static Gdiplus::Graphics*	graphics;				// Current graphics instance
	static Gdiplus::Graphics*	rendererGraphics;		// Graphics instance created by this renderer
	static Gdiplus::SolidBrush*	drawBrush;				// Brush used for drawing solid shapes
	static Gdiplus::SolidBrush*	shadowBrush;			// Brush used for drawing text shadows
	static Gdiplus::Color		drawColour;				// Current draw colour
	static LINE_STATUS			lineStatus;				// Current underline status
	static bool					lineContinue;			// Continue drawing a line
	static colour_t				curDrawColour;			// Current drawing colour as a colour_t struct
	static int32				drawOffsetX;			// Current drawing offset (X)
	static int32				drawOffsetY;			// Current drawing offset (Y)
};

#endif /* __MYLLY_GUI_GDIPLUS_RENDERER_H */
