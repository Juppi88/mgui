/**********************************************************************
 *
 * PROJECT:		Mylly GUI - DirectX 8 Renderer
 * FILE:		DirectX8.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A DirectX 8 reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_DIRECTX8_H
#define __MYLLY_GUI_DIRECTX8_H

#include "MGUI/Renderer/Renderer.h"

__BEGIN_DECLS

MYLLY_API MGuiRenderer*	mgui_directx8_initialize	( void* hwnd );
MYLLY_API void			mgui_directx8_shutdown		( void );
MYLLY_API void			mgui_directx8_begin_scene	( void );
MYLLY_API void			mgui_directx8_end_scene		( void );

__END_DECLS

#endif /* __MYLLY_GUI_DIRECTX_8_H */
