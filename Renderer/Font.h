/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Font.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		A renderer font storage.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_FONT_H
#define __MYLLY_GUI_FONT_H

#include "MGUI.h"

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

typedef struct
{
	void*		data;		// Renderer font data
	char_t*		name;		// The name of the font
	uint8		size;		// Size (height)
	uint8		flags;		// Font flags, defined in MGUI.h
	uint8		charset;	// Character set
} font_t;

font_t*		mgui_font_create			( const char_t* name, uint8 size, uint8 flags, uint8 charset );
font_t*		mgui_font_create_range		( const char_t* name, uint8 size, uint8 flags, uint8 charset, char_t firstc, char_t lastc );
font_t*		mgui_font_create_default	( void );
void		mgui_font_destroy			( font_t* font );
void		mgui_font_set_font			( font_t* font, const char_t* name );
void		mgui_font_set_size			( font_t* font, uint8 size );
void		mgui_font_set_flags			( font_t* font, uint8 flags );
void		mgui_font_set_charset		( font_t* font, uint8 charset );

#endif /* __MYLLY_GUI_FONT_H */
