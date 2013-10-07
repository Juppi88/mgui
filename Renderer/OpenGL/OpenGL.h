/**********************************************************************
 *
 * PROJECT:		Mylly GUI - OpenGL Renderer
 * FILE:		OpenGL.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		An OpenGL reference renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_OPENGL_H
#define __MYLLY_GUI_OPENGL_H

#include "MGUI/Renderer/Renderer.h"

__BEGIN_DECLS

MYLLY_API MGuiRenderer*	mgui_opengl_initialize		( void* window );
MYLLY_API void			mgui_opengl_shutdown		( void );
MYLLY_API void			mgui_opengl_begin_scene		( void );
MYLLY_API void			mgui_opengl_end_scene		( void );

__END_DECLS

#endif /* __MYLLY_GUI_OPENGL_H */
