/**********************************************************************
 *
 * PROJECT:		Mylly GUI - GDI+ Renderer
 * FILE:		WinGDIPlus.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A GDI+ reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_GDIPLUS_H
#define __MYLLY_GUI_GDIPLUS_H

#include "MGUI/Renderer/Renderer.h"

__BEGIN_DECLS

MYLLY_API MGuiRenderer*	mgui_gdiplus_initialize		( void* hwnd );
MYLLY_API void			mgui_gdiplus_shutdown		( void );
MYLLY_API void			mgui_gdiplus_begin_scene	( void );
MYLLY_API void			mgui_gdiplus_end_scene		( void );

__END_DECLS

#endif /* __MYLLY_GUI_GDIPLUS_H */
