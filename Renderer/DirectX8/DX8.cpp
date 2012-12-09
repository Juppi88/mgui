/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (DirectX8)
 * FILE:		DX8.cpp
 * LICENCE:		See Licence.txt
 * PURPOSE:		A DirectX8 renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "DX8.h"
#include "Drawing.h"
#include <d3d8.h>
#include <D3dx8core.h>
#include <D3dx8math.h>
#include <DxErr.h>

static mgui_renderer_t	renderer;
/*static*/ HWND				wnd;

LPDIRECT3D8				D3D = NULL;
IDirect3DDevice8*		D3DDevice = NULL;
D3DPRESENT_PARAMETERS	D3DParams;

mgui_renderer_t* mgui_dx8_initialize( void* syswindow )
{
	RECT rect;
	HRESULT res;

	D3D = Direct3DCreate8( D3D_SDK_VERSION );
	wnd = (HWND)syswindow;

	GetClientRect( wnd, &rect );

	ZeroMemory( &D3DParams, sizeof(D3DParams) );

	D3DParams.Windowed = TRUE;
	D3DParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	D3DParams.BackBufferWidth = rect.right;
	D3DParams.BackBufferHeight = rect.bottom;
	D3DParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	D3DParams.BackBufferFormat = D3DFMT_X8R8G8B8;

	res = D3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &D3DParams, &D3DDevice );

	if ( FAILED(res) ) return NULL;

	renderer.type = GUIREND_DX8;
	renderer.device_context = NULL;

	renderer.begin				= mgui_dx8_begin;
	renderer.end				= mgui_dx8_end;
	renderer.resize				= mgui_dx8_resize;
	renderer.set_draw_colour	= mgui_dx8_set_draw_colour;
	renderer.start_clip			= mgui_dx8_start_clip;
	renderer.end_clip			= mgui_dx8_end_clip;
	renderer.draw_rect			= mgui_dx8_draw_rect;
	renderer.draw_triangle		= mgui_dx8_draw_triangle;
	renderer.load_texture		= mgui_dx8_load_texture;
	renderer.destroy_texture	= mgui_dx8_destroy_texture;
	renderer.draw_textured_rect	= mgui_dx8_draw_textured_rect;
	renderer.load_font			= mgui_dx8_load_font;
	renderer.destroy_font		= mgui_dx8_destroy_font;
	renderer.draw_text			= mgui_dx8_draw_text;
	renderer.measure_text		= mgui_dx8_measure_text;

	mgui_dx8_drawing_initialize();

	return &renderer;
}

void mgui_dx8_shutdown( void )
{
	mgui_dx8_drawing_shutdown();
}
