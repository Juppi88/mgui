/**********************************************************************
 *
 * PROJECT:		Mylly GUI - GDI+ Renderer
 * FILE:		WinGDIPlus.cpp
 * LICENCE:		See Licence.txt
 * PURPOSE:		A GDI+ reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#include "WinGDIPlus.h"
#include "Renderer.h"

static MGuiRenderer	renderer;
static bool			initialized	= false;
static ULONG_PTR	token		= 0;
HWND				hwnd		= NULL;

MGuiRenderer* mgui_gdiplus_initialize( void* wnd )
{
	Gdiplus::GdiplusStartupInput si;
	Gdiplus::Status status;

	hwnd = (HWND)wnd;

	status = Gdiplus::GdiplusStartup( &token, &si, NULL );
	if ( status != Gdiplus::Ok ) return NULL;

	// GDI+ was initialized successfully, so let's set up the renderer.
	renderer.properties = REND_SUPPORTS_TEXTTAGS |
						  REND_SUPPORTS_TEXTURES |
						  REND_SUPPORTS_TARGETS;
						  
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

void mgui_gdiplus_shutdown( void )
{
	if ( !initialized ) return;

	// Clean up the mess we made.
	CRenderer::Shutdown();

	Gdiplus::GdiplusShutdown( token );
	initialized = false;
}

void mgui_gdiplus_begin_scene( void )
{
}

void mgui_gdiplus_end_scene( void )
{
}
