/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (OpenGL)
 * FILE:		OpenGLX11.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An OpenGL renderer for Mylly GUI.
 *				X11 related functions
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#ifndef _WIN32

#include "OpenGL.h"

void* mgui_opengl_create_context( void* syswindow )
{
	return NULL;
}

void mgui_opengl_destroy_context( void* context )
{

}

#endif /* _WIN32 */
