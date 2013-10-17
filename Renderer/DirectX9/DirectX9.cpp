/**********************************************************************
 *
 * PROJECT:		Mylly GUI - DirectX 9 Renderer
 * FILE:		DirectX9.cpp
 * LICENCE:		See Licence.txt
 * PURPOSE:		A DirectX 9 reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "DirectX9.h"
#include "Renderer.h"
#include <d3d9.h>
#include <D3dx9core.h>
#include <D3dx9math.h>
#include <DxErr.h>

static MGuiRenderer	renderer;
static bool			initialized	= false;
static HWND			hwnd		= NULL;
IDirect3D9*			d3d			= NULL;
IDirect3DDevice9*	d3dDevice	= NULL;
D3DPRESENT_PARAMETERS params;

MGuiRenderer* mgui_directx9_initialize( void* wnd )
{
	RECT rect;
	HRESULT hr;

	// Create an IDirect3D object.
	d3d = Direct3DCreate9( D3D_SDK_VERSION );
	hwnd = (HWND)wnd;

	GetClientRect( hwnd, &rect );

	ZeroMemory( &params, sizeof(params) );

	// Create a display adapter device with the wanted arguments.
	params.Windowed = TRUE;
	params.SwapEffect = D3DSWAPEFFECT_DISCARD;
	params.BackBufferWidth = rect.right;
	params.BackBufferHeight = rect.bottom;
	params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	params.BackBufferFormat = D3DFMT_X8R8G8B8;

	hr = d3d->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &params, &d3dDevice );

	if ( FAILED( hr ) ) return NULL;

	// Device was created successfully, so let's set up the renderer.
	renderer.properties = REND_SUPPORTS_TEXTTAGS |
						  REND_SUPPORTS_TEXTURES |
						  REND_SUPPORTS_TARGETS |
						  REND_RESET_ON_RESIZE;

	renderer.begin					= CRenderer::Begin;
	renderer.end					= CRenderer::End;
	renderer.resize					= CRenderer::Resize;
	renderer.set_draw_mode			= CRenderer::SetDrawMode;
	renderer.set_draw_colour		= CRenderer::SetDrawColour;
	renderer.set_draw_depth			= CRenderer::SetDrawDepth;
	renderer.set_draw_transform		= CRenderer::SetDrawTransform;
	renderer.reset_draw_transform	= CRenderer::ResetDrawTransform;
	renderer.start_clip				= CRenderer::StartClip;
	renderer.end_clip				= CRenderer::EndClip;
	renderer.draw_rect				= CRenderer::DrawRect;
	renderer.draw_triangle			= CRenderer::DrawTriangle;
	renderer.draw_pixel				= CRenderer::DrawPixel;
	renderer.load_texture			= CRenderer::LoadTexture;
	renderer.destroy_texture		= CRenderer::DestroyTexture;
	renderer.draw_textured_rect		= CRenderer::DrawTexturedRect;
	renderer.load_font				= CRenderer::LoadFont;
	renderer.destroy_font			= CRenderer::DestroyFont;
	renderer.draw_text				= CRenderer::DrawText;
	renderer.measure_text			= CRenderer::MeasureText;
	renderer.create_render_target	= CRenderer::CreateRenderTarget;
	renderer.destroy_render_target	= CRenderer::DestroyRenderTarget;
	renderer.draw_render_target		= CRenderer::DrawRenderTarget;
	renderer.enable_render_target	= CRenderer::EnableRenderTarget;
	renderer.disable_render_target	= CRenderer::DisableRenderTarget;
	renderer.screen_pos_to_world	= CRenderer::ScreenPosToWorld;
	renderer.world_pos_to_screen	= CRenderer::WorldPosToScreen;

	CRenderer::Initialize();

	initialized = true;

	return &renderer;
}

void mgui_directx9_shutdown( void )
{
	if ( !initialized ) return;

	// Clean up the mess we made.
	CRenderer::Shutdown();

	SAFE_RELEASE( d3dDevice );
	SAFE_RELEASE( d3d );

	initialized = false;
}

void mgui_directx9_begin_scene( void )
{
	// If rendering is done every frame, call BeginScene here.
	// Because we're only updating the window when there is something to draw,
	// we're doing it in CRenderer::Begin instead.
}

void mgui_directx9_end_scene( void )
{
	// If rendering is done every frame, call EndScene and Present here.
	// Because we're only updating the window when there is something to draw,
	// we're doing it in CRenderer::End instead.
}
