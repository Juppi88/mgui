/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (X11)
 * FILE:		X11.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		An X11 renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_X11_RENDERER_H
#define __MYLLY_GUI_X11_RENDERER_H

#include "MGUI/Renderer/Renderer.h"
#include <X11/Xlib.h>


__BEGIN_DECLS

// Public functions
MYLLY_API MGuiRenderer*	mgui_x11_initialize		( void* syswindow );
MYLLY_API void			mgui_x11_shutdown		( void );

__END_DECLS


#endif /* __MYLLY_GUI_X11_RENDERER_H */
