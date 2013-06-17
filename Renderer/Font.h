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
#ifndef __MYLLY_GUI_FONT_H
#define __MYLLY_GUI_FONT_H

#include "MGUI.h"
#include "Types/List.h"

#ifdef _WIN32
#define CHARSET_ANSI			ANSI_CHARSET
#define CHARSET_BALTIC			BALTIC_CHARSET
#define CHARSET_CHINESE_TRAD	CHINESEBIG5_CHARSET
#define CHARSET_DEFAULT			DEFAULT_CHARSET
#define CHARSET_EASTEUR			EASTEUROPE_CHARSET
#define CHARSET_CHINESE_SIMPLE	GB2312_CHARSET
#define CHARSET_GREEK			GREEK_CHARSET
#define CHARSET_HANGUL_KOREAN	HANGUL_CHARSET
#define CHARSET_APPLE			MAC_CHARSET
#define CHARSET_OEM				OEM_CHARSET
#define CHARSET_CYRILLIC		RUSSIAN_CHARSET
#define CHARSET_JAPANESE		SHIFTJIS_CHARSET
#define CHARSET_SYMBOL			SYMBOL_CHARSET
#define CHARSET_TURKISH			TURKISH_CHARSET
#define CHARSET_VIETNAMESE		VIETNAMESE_CHARSET
#else
#endif /* _WIN32 */

typedef struct MGuiFont
{
	node_t		node;		// Linked list node
	void*		data;		// Renderer font data
	char_t*		name;		// The name of the font
	uint8		size;		// Size (height)
	uint8		flags;		// Font flags, defined in MGUI.h
	uint8		charset;	// Character set
	char_t		first_char;	// First character in range
	char_t		last_char;	// Last character in range
	uint32		refcount;	// Reference count
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

#endif /* __MYLLY_GUI_FONT_H */
