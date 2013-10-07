/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (Xlib)
 * FILE:		Xlib.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		An Xlib renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_XLIB_H
#define __MYLLY_GUI_XLIB_H

#include "MGUI/Renderer/Renderer.h"

__BEGIN_DECLS

MYLLY_API MGuiRenderer*	mgui_xlib_initialize	( void* window );
MYLLY_API void			mgui_xlib_shutdown		( void );
MYLLY_API void			mgui_xlib_begin_scene	( void );
MYLLY_API void			mgui_xlib_end_scene		( void );

__END_DECLS

#endif /* __MYLLY_GUI_X11_RENDERER_H */
