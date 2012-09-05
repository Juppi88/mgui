/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (OpenGL)
 * FILE:		OpenGL.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		An OpenGL renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_OPENGL_RENDERER_H
#define __MYLLY_GUI_OPENGL_RENDERER_H

#include "Renderer.h"
#include <gl/gl.h>
#include <gl/glu.h>

#define MYLLY_GUI_MAX_VERT 1024

typedef struct
{
	float x, y, z;
	float u, v;
	uint8 r, g, b, a;
} vertex_t;

typedef struct oglfont_s
{
	uint32		width;
	uint32		height;
	uint32		spacing;
	uint32		flags;
	uint32		texture;
	uint32		size;
	uint32		first_char;
	uint32		last_char;
	uint32		data_len;
	float**		coords;
} oglfont_t;

// Functions to create and destroy device context
void*	mgui_opengl_create_context		( void* syswindow );
void	mgui_opengl_destroy_context		( void* context );


__BEGIN_DECLS

// Public functions
MYLLY_API mgui_renderer_t*	mgui_opengl_initialize			( void* syswindow );
MYLLY_API void				mgui_opengl_shutdown			( void );

__END_DECLS


#endif /* __MYLLY_GUI_OPENGL_RENDERER_H */
