/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (GDI+)
 * FILE:		GDI.cpp
 * LICENCE:		See Licence.txt
 * PURPOSE:		A GDI+ renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#include "GDI.h"
#include "Drawing.h"
#include <gdiplus.h>

static MGuiRenderer	renderer;
static ULONG_PTR		token		= NULL;
HWND					hwnd		= NULL;

MGuiRenderer* mgui_gdiplus_initialize( void* syswindow )
{
	Gdiplus::GdiplusStartupInput si;

	renderer.device_context = NULL;

	renderer.begin				= mgui_gdiplus_begin;
	renderer.end				= mgui_gdiplus_end;
	renderer.resize				= mgui_gdiplus_resize;
	renderer.set_draw_colour	= mgui_gdiplus_set_draw_colour;
	renderer.start_clip			= mgui_gdiplus_start_clip;
	renderer.end_clip			= mgui_gdiplus_end_clip;
	renderer.draw_rect			= mgui_gdiplus_draw_rect;
	renderer.draw_triangle		= mgui_gdiplus_draw_triangle;
	renderer.load_texture		= mgui_gdiplus_load_texture;
	renderer.destroy_texture	= mgui_gdiplus_destroy_texture;
	renderer.draw_textured_rect	= mgui_gdiplus_draw_textured_rect;
	renderer.load_font			= mgui_gdiplus_load_font;
	renderer.destroy_font		= mgui_gdiplus_destroy_font;
	renderer.draw_text			= mgui_gdiplus_draw_text;
	renderer.measure_text		= mgui_gdiplus_measure_text;

	Gdiplus::GdiplusStartup( &token, &si, NULL );

	hwnd = (HWND)syswindow;

	return &renderer;
}

void mgui_gdiplus_shutdown( void )
{
	Gdiplus::GdiplusShutdown( token );
}
