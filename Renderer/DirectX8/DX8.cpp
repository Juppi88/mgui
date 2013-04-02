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

static MGuiRenderer		renderer;
static HWND				wnd			= NULL;
LPDIRECT3D8				d3d			= NULL;
IDirect3DDevice8*		d3dDevice	= NULL;
D3DPRESENT_PARAMETERS	d3dParams;

MGuiRenderer* mgui_dx8_initialize( void* syswindow )
{
	RECT rect;
	HRESULT res;

	d3d = Direct3DCreate8( D3D_SDK_VERSION );
	wnd = (HWND)syswindow;

	GetClientRect( wnd, &rect );

	ZeroMemory( &d3dParams, sizeof(d3dParams) );

	d3dParams.Windowed = TRUE;
	d3dParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dParams.BackBufferWidth = rect.right;
	d3dParams.BackBufferHeight = rect.bottom;
	d3dParams.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	d3dParams.BackBufferFormat = D3DFMT_X8R8G8B8;

	res = d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, wnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dParams, &d3dDevice );

	if ( FAILED(res) ) return NULL;

	renderer.type = GUIREND_DX8;
	renderer.device_context = NULL;

	renderer.begin				= CMGuiDX8Drawing::Begin;
	renderer.end				= CMGuiDX8Drawing::End;
	renderer.resize				= CMGuiDX8Drawing::Resize;
	renderer.set_draw_colour	= CMGuiDX8Drawing::SetDrawColour;
	renderer.start_clip			= CMGuiDX8Drawing::StartClip;
	renderer.end_clip			= CMGuiDX8Drawing::EndClip;
	renderer.draw_rect			= CMGuiDX8Drawing::DrawRect;
	renderer.draw_triangle		= CMGuiDX8Drawing::DrawTriangle;
	renderer.load_texture		= CMGuiDX8Drawing::LoadTexture;
	renderer.destroy_texture	= CMGuiDX8Drawing::DestroyTexture;
	renderer.draw_textured_rect	= CMGuiDX8Drawing::DrawTexturedRect;
	renderer.load_font			= CMGuiDX8Drawing::LoadFont;
	renderer.destroy_font		= CMGuiDX8Drawing::DestroyFont;
	renderer.draw_text			= CMGuiDX8Drawing::DrawText;
	renderer.measure_text		= CMGuiDX8Drawing::MeasureText;

	CMGuiDX8Drawing::Initialize();

	return &renderer;
}

void mgui_dx8_shutdown( void )
{
	CMGuiDX8Drawing::Shutdown();
}
