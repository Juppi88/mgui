/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Font.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A renderer font storage.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_FONT_H
#define __MGUI_FONT_H

#include "MGUI.h"
#include "Renderer.h"
#include "Types/List.h"

typedef struct {
	node_t			node;		// Linked list node
	MGuiRendFont*	data;		// Renderer font data
	char_t*			name;		// The name of the font
	uint8			size;		// Size (height)
	uint8			flags;		// Font flags, defined in MGUI.h
	uint8			charset;	// Character set
	char_t			first_char;	// First character in range
	char_t			last_char;	// Last character in range
	uint32			refcount;	// Reference count
} MGuiFont;

void		mgui_fontmgr_initialize		( void );
void		mgui_fontmgr_shutdown		( void );
void		mgui_fontmgr_initialize_all	( void );
void		mgui_fontmgr_invalidate_all	( void );

MGuiFont*	mgui_font_create			( const char_t* name, uint8 size, uint8 flags, uint8 charset );
MGuiFont*	mgui_font_create_range		( const char_t* name, uint8 size, uint8 flags, uint8 charset, char_t firstc, char_t lastc );
void		mgui_font_destroy			( MGuiFont* font );

MGuiFont*	mgui_font_set_font			( MGuiFont* font, const char_t* name );
MGuiFont*	mgui_font_set_size			( MGuiFont* font, uint8 size );
MGuiFont*	mgui_font_set_flags			( MGuiFont* font, uint8 flags );
MGuiFont*	mgui_font_set_charset		( MGuiFont* font, uint8 charset );

void		mgui_font_reinitialize		( MGuiFont* font );

#endif /* __MGUI_FONT_H */
