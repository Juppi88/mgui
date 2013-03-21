/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Renderer.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI renderer initialization.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "Renderer.h"

#ifdef MGUI_OPENGL
#include "Renderer/OpenGL/OpenGL.h"
#endif
#ifdef MGUI_DIRECTX8
#include "Renderer/DirectX8/DX8.h"
#endif
#ifdef MGUI_DIRECTX9
#include "Renderer/DirectX9/DX9.h"
#endif
#ifdef MGUI_DIRECTX10
#include "Renderer/DirectX10/DX10.h"
#endif
#ifdef MGUI_DIRECTX11
#include "Renderer/DirectX9/DX11.h"
#endif
#ifdef MGUI_GDIPLUS
#include "Renderer/GDIPlus/GDI.h"
#endif

MGuiRenderer* render;

void mgui_render_initialize( MGUI_RENDERER type, void* window )
{
	switch ( type )
	{
#ifdef MGUI_OPENGL
	case GUIREND_OPENGL:
		render = mgui_opengl_initialize( window );
		break;
#endif
#ifdef MGUI_DIRECTX8
	case GUIREND_DX8:
		render = mgui_dx8_initialize( window );
		break;
#endif
#ifdef MGUI_DIRECTX9
	case GUIREND_DX9:
		render = mgui_dx9_initialize( window );
		break;
#endif
#ifdef MGUI_DIRECTX10
	case GUIREND_DX10:
		render = mgui_dx10_initialize( window );
		break;
#endif
#ifdef MGUI_DIRECTX11
	case GUIREND_DX11:
		render = mgui_dx11_initialize( window );
		break;
#endif
#ifdef MGUI_GDIPLUS
	case GUIREND_GDIPLUS:
		render = mgui_gdiplus_initialize( window );
		break;
#endif
	default:
		render = NULL;
		break;
	}
}

void mgui_render_shutdown( void )
{
	if ( !render ) return;

	switch ( render->type )
	{
#ifdef MGUI_OPENGL
	case GUIREND_OPENGL:
		mgui_opengl_shutdown();
		break;
#endif
#ifdef MGUI_DIRECTX8
	case GUIREND_DX8:
		mgui_dx8_shutdown();
		break;
#endif
#ifdef MGUI_DIRECTX9
	case GUIREND_DX9:
		mgui_dx9_shutdown();
		break;
#endif
#ifdef MGUI_DIRECTX10
	case GUIREND_DX10:
		mgui_dx10_shutdown();
		break;
#endif
#ifdef MGUI_DIRECTX11
	case GUIREND_DX11:
		mgui_dx11_shutdown();
		break;
#endif
#ifdef MGUI_GDIPLUS
	case GUIREND_GDIPLUS:
		mgui_gdiplus_shutdown();
		break;
#endif
	}
}
