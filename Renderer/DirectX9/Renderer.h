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

#pragma once
#ifndef __MYLLY_GUI_DIRECTX9_RENDERER_H
#define __MYLLY_GUI_DIRECTX9_RENDERER_H

#include "DirectX9.h"
#include <d3d9.h>
#include <d3dx9.h>

#define SAFE_RELEASE(x) if ( x ) { (x)->Release(); x = NULL; }

// Forward declarations for some internal structs and classes.
class CFont;
class CTexture;
class CRenderTarget;
struct Vertex;
struct Vertex3D;

// Used for drawing an underline for text (format tags)
enum LINE_STATUS {
	LINE_IDLE,
	LINE_BEGIN,
	LINE_DRAWING,
	LINE_DRAW,
};

class CRenderer
{
public:
	// Renderer initialization and shutdown
	static void				Initialize				( void );
	static void				Shutdown				( void );

	// Actual renderer methods
	static void				Begin					( void );
	static void				End						( void );
	static void				Resize					( uint32 w, uint32 h );

	static DRAW_MODE		SetDrawMode				( DRAW_MODE mode );
	static void				SetDrawColour			( const colour_t* col );
	static void				SetDrawDepth			( float zDepth );
	static void				SetDrawTransform		( const matrix4_t* mat );
	static void				ResetDrawTransform		( void );

	static void				StartClip				( int32 x, int32 y, uint32 w, uint32 h );
	static void				EndClip					( void );

	static void				DrawRect				( int32 x, int32 y, uint32 w, uint32 h );
	static void				DrawTriangle			( int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3 );
	static void				DrawPixel				( int32 x, int32 y );

	static MGuiRendTexture*	LoadTexture				( const char_t* path, uint32* width, uint32* height );
	static void				DestroyTexture			( MGuiRendTexture* texture );
	static void				DrawTexturedRect		( const MGuiRendTexture* texture, int32 x, int32 y, uint32 w, uint32 h, const float uv[] );

	static MGuiRendFont*	LoadFont				( const char_t* name, uint8 size, uint8 flags, uint8 charset, uint32 firstc, uint32 lastc );
	static void				DestroyFont				( MGuiRendFont* font );
	static void				DrawText				( const MGuiRendFont* font, const char_t* text, int32 x, int32 y, uint32 flags, const MGuiFormatTag tags[], uint32 ntags );
	static void				MeasureText				( const MGuiRendFont* font, const char_t* text, uint32* w, uint32* h );

	static MGuiRendTarget*	CreateRenderTarget		( uint32 width, uint32 height );
	static void				DestroyRenderTarget		( MGuiRendTarget* target );
	static void				DrawRenderTarget		( const MGuiRendTarget* target, int32 x, int32 y, uint32 w, uint32 h );
	static void				EnableRenderTarget		( const MGuiRendTarget* target, int32 x, int32 y );
	static void				DisableRenderTarget		( const MGuiRendTarget* target );

	static void				ScreenPosToWorld		( const vector3_t* src, vector3_t* dst );
	static void				WorldPosToScreen		( const vector3_t* src, vector3_t* dst );

private:
	static inline void		BeginDraw				( void );
	static inline void		EndDraw					( void );

	static inline void		IncreaseZPlane			( void );

	static inline void		AddVertex				( int32 x, int32 y );
	static inline void		AddVertex2D				( int32 x, int32 y, float z );
	static inline void		AddVertex3D				( int32 x, int32 y, float z );
	static inline void		AddVertexTextured		( int32 x, int32 y, float u, float v );
	static inline void		AddVertexTextured2D		( int32 x, int32 y, float u, float v, float z );
	static inline void		AddVertexTextured3D		( int32 x, int32 y, float u, float v, float z );

	static inline uint32	AddChar					( uint32 c, int32 x, int32 y, uint32 flags );

	static void				ProcessTag				( const MGuiFormatTag* tag );
	static void				ProcessUnderline		( const CFont* font, int32 x, int32 y, int32& x2, int32& y2, colour_t& lineColour );

	static void				CheckVertexBuffer		( uint32 required );
	static inline void		UpdateVertexFormat		( bool textured = false );

	static void				Flush					( bool endDrawing = false );

	static void				CreateFontTexture		( void* fontData, uint32 width, uint32 height, void** texture, uint32* texturePitch );

private:
	static DRAW_MODE				drawMode;		// Current drawing mode
	static uint32					colour;			// Current drawing colour
	static colour_t					drawColour;		// Current drawing colour as a colour struct
	static D3DMATRIX				transform;		// Old transform matrix
	static IDirect3DTexture9*		renderTexture;	// Current texture
	static IDirect3DVertexBuffer9*	vertexBuffer;	// Pointer to vertex buffer
	static IDirect3DVertexBuffer9*	vertexBuffer3d;	// Pointer to 3D vertex buffer
	static bool						lockBuffer;		// Should the vertex buffer be locked
	static bool						lockBuffer3d;	// Should the 3D vertex buffer be locked
	static Vertex*					vertex;			// Pointer to current vertex
	static Vertex3D*				vertex3d;		// Pointer to current 3D vertex
	static uint32					numVertices;	// Number of vetrices stored into the temp buffer
	static uint32					numVertices3d;	// Number of 3D vetrices stored into the temp buffer
	static DWORD					fvfOld;			// Storage for vertex shader
	static DWORD					fvf;			// Storage for current vertex shader
	static IDirect3DStateBlock9*	normState;		// Norm state
	static IDirect3DStateBlock9*	drawState;		// Draw state
	static IDirect3DStateBlock9*	drawState3d;	// Draw state for 3D mode
	static rectangle_t				clip;			// Clip rectangle
	static bool						clipping;		// Is clipping enabled?
	static const CFont*				renderFont;		// Temp storage for current font
	static LINE_STATUS				lineStatus;		// Current underline status
	static bool						lineContinue;	// Continue drawing a line
	static float					zIndex;			// Current Z-index (used with 3D rendering)
	static float					zBias;			// Added z-bias (a dodgy fix for 3D z-fighting)
	static D3DVIEWPORT9				viewport;		// Default viewport
	static int32					drawOffsetX;	// X offset when drawing onto a texture
	static int32					drawOffsetY;	// Y offset when drawing onto a texture
	static CRenderTarget*			renderTarget;	// Current render target
	static uint32					screenWidth;	// Render target (window or screen) width
	static uint32					screenHeight;	// Render target (window or screen) height
};

#endif /* __MYLLY_GUI_DIRECTX9_RENDERER_H */
