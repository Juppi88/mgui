/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (DirectX8)
 * FILE:		Drawing.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		DirectX8 GUI drawing functions.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_DX8_DRAWING_H
#define __MYLLY_GUI_DX8_DRAWING_H

#include "DX8.h"
#include <d3d8.h>
#include <D3dx8core.h>
#include <D3dx8math.h>
#include <DxErr.h>

struct vertex_t;

class CMGuiDX8Drawing
{
public:
	static void				Initialize				( void );
	static void				Shutdown				( void );

	static void				Begin					( void );
	static void				End						( void );
	static void				Resize					( uint32 w, uint32 h );

	static void				SetDrawColour			( const colour_t* col );
	static void				StartClip				( uint32 x, uint32 y, uint32 w, uint32 h );
	static void				EndClip					( void );

	static void				DrawRect				( uint32 x, uint32 y, uint32 w, uint32 h );
	static void				DrawTriangle			( uint32 x1, uint32 y1, uint32 x2, uint32 y2, uint32 x3, uint32 y3 );

	static void*			LoadTexture				( const char_t* path );
	static void				DestroyTexture			( void* texture );
	static void				DrawTexturedRect		( void* texture, uint32 x, uint32 y, uint32 w, uint32 h );

	static void*			LoadFont				( const char_t* name, uint32 size, uint32 flags, uint32 charset, uint32 firstc, uint32 lastc );
	static void				DestroyFont				( void* texture );
	static void				DrawText				( void* font, const char_t* text, uint x, uint y, uint flags );
	static void				MeasureText				( void* font, const char_t* text, uint32* w, uint32* h );

private:
	static inline void		AddVertex				( uint32 x, uint32 y );
	static inline void		AddVertexTex			( uint32 x, uint32 y, float u, float v );
	static void				Flush					( void );

private:
	static uint32					num_vertices;	// Number of vetrices stored into the temp buffer
	static uint32					colour;			// Current drawing colour
	static IDirect3DTexture8*		texture;		// Current texture
	static IDirect3DVertexBuffer8*	vertex_buf;		// Pointer to vertex buffer
	static vertex_t*				vertex;			// Pointer to current vertex
	static DWORD					fvf;			// Storage for vertex shader
	static RECT						clip;			// Temp storage for clipping
};

#endif /* __MYLLY_GUI_DX8_DRAWING_H */
