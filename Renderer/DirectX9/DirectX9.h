/**********************************************************************
 *
 * PROJECT:		Mylly GUI - DirectX 9 Renderer
 * FILE:		DirectX9.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A DirectX 9 reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_DIRECTX9_H
#define __MYLLY_GUI_DIRECTX9_H

#include "MGUI/Renderer/Renderer.h"

__BEGIN_DECLS

MYLLY_API MGuiRenderer*	mgui_directx9_initialize	( void* hwnd );
MYLLY_API void			mgui_directx9_shutdown		( void );
MYLLY_API void			mgui_directx9_begin_scene	( void );
MYLLY_API void			mgui_directx9_end_scene		( void );

__END_DECLS

#endif /* __MYLLY_GUI_DIRECTX9_H */
