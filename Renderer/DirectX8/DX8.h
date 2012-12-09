/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (DirectX8)
 * FILE:		DX8.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A DirectX8 renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_DX8_RENDERER_H
#define __MYLLY_GUI_DX8_RENDERER_H

#include "Renderer.h"

__BEGIN_DECLS

MYLLY_API mgui_renderer_t*	mgui_dx8_initialize			( void* syswindow );
MYLLY_API void				mgui_dx8_shutdown			( void );

__END_DECLS

#endif /* __MYLLY_GUI_DX8_RENDERER_H */
