/**********************************************************************
 *
 * PROJECT:		Mylly GUI Renderer (GDI+)
 * FILE:		GDI.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A GDI+ renderer for Mylly GUI.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_GDIPLUS_RENDERER_H
#define __MYLLY_GUI_GDIPLUS_RENDERER_H

#include "Renderer.h"

typedef struct gdifont_s
{
	uint32		size;
	uint32		flags;
	void*		font;
} MGuiGDIFont;

__BEGIN_DECLS

// Public functions
MYLLY_API MGuiRenderer*	mgui_gdiplus_initialize			( void* syswindow );
MYLLY_API void			mgui_gdiplus_shutdown			( void );

__END_DECLS

#endif /* __MYLLY_GUI_GDIPLUS_RENDERER_H */
