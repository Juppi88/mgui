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

#include "Font.h"
#include "Skin.h"
#include "Renderer.h"
#include "Stringy/Stringy.h"
#include "Platform/Alloc.h"
#include <assert.h>

extern MGuiRenderer* renderer;
static list_t* fonts;

MGuiFont*			default_font = NULL;	// Default font for all elements
MGuiFont*			wndbutton_font = NULL;	// Font used for the close button X

static void			mgui_font_destroy_unconditional( MGuiFont* font );
static MGuiFont*	mgui_font_find			( const char_t* name, uint8 size, uint8 flags, uint8 charset, char_t firstc, char_t lastc );
static uint8		mgui_font_get_charset	( uint32 charset );

void mgui_fontmgr_initialize( void )
{
	fonts = list_create();

	default_font = mgui_font_create_range( DEFAULT_FONT, 11, FFLAG_NONE, CHARSET_ANSI, 0, 0 );
	wndbutton_font = mgui_font_create_range( DEFAULT_FONT, 10, FFLAG_NONE, CHARSET_ANSI, 'X', 'X' );
}

void mgui_fontmgr_shutdown( void )
{
	node_t *node, *tmp;

	list_foreach_safe( fonts, node, tmp )
	{
		mgui_font_destroy_unconditional( (MGuiFont*)node );
	}

	list_destroy( fonts );
	fonts = NULL;
}

void mgui_fontmgr_initialize_all( void )
{
	node_t* node;
	MGuiFont* font;

	if ( renderer == NULL ) return;

	list_foreach( fonts, node )
	{
		font = (MGuiFont*)node;

		if ( font->data == NULL )
			font->data = renderer->load_font( font->name, font->size, font->flags, mgui_font_get_charset( font->charset ), font->first_char, font->last_char );
	}
}

void mgui_fontmgr_invalidate_all( void )
{
	node_t* node;
	MGuiFont* font;

	if ( renderer == NULL ) return;

	list_foreach( fonts, node )
	{
		font = (MGuiFont*)node;

		if ( font->data )
		{
			renderer->destroy_font( font->data );
			font->data = NULL;
		}
	}
}

MGuiFont* mgui_font_create( const char_t* name, uint8 size, uint8 flags, uint8 charset )
{
	return mgui_font_create_range( name, size, flags, charset, 0, 0 );
}

MGuiFont* mgui_font_create_range( const char_t* name, uint8 size, uint8 flags, uint8 charset, char_t firstc, char_t lastc )
{
	MGuiFont* font;
	size_t len;

	font = mgui_font_find( name, size, flags, charset, firstc, lastc );

	if ( font && font != default_font && font != wndbutton_font )
	{
		font->refcount++;
		return font;
	}
	
	len = mstrsize( name );

	font = (MGuiFont*)mem_alloc_clean( sizeof(*font) );
	font->name = mem_alloc( len2size(len) );
	font->size = size;
	font->flags = flags;
	font->charset = charset;
	font->first_char = firstc;
	font->last_char = lastc;
	font->refcount = 1;
	font->data = NULL;

	mstrcpy( font->name, name, len );

	if ( renderer != NULL )
		font->data = renderer->load_font( name, size, flags, mgui_font_get_charset( charset ), firstc, lastc );

	list_push( fonts, &font->node );

	return font;
}

void mgui_font_destroy( MGuiFont* font )
{
	if ( font == default_font ||
		 font == wndbutton_font )
		 return;

	mgui_font_destroy_unconditional( font );
}

static void mgui_font_destroy_unconditional( MGuiFont* font )
{
	if ( font == NULL ) return;
	if ( --(font->refcount) ) return;

	if ( font->data != NULL && renderer != NULL )
		renderer->destroy_font( font->data );

	SAFE_DELETE( font->name );

	list_remove( fonts, &font->node );
	mem_free( font );
}

MGuiFont* mgui_font_find( const char_t* name, uint8 size, uint8 flags, uint8 charset, char_t firstc, char_t lastc )
{
	node_t* node;
	MGuiFont* font;

	list_foreach( fonts, node )
	{
		font = (MGuiFont*)node;

		if ( mstrequal( font->name, name ) &&
			 font->size == size &&
			 font->flags == flags &&
			 font->charset == charset &&
			 font->first_char == firstc &&
			 font->last_char == lastc &&
			 font != default_font &&
			 font != wndbutton_font )
			 return font;
	}

	return NULL;
}

MGuiFont* mgui_font_set_font( MGuiFont* font, const char_t* name )
{
	size_t len;
	MGuiFont* fnt;

	if ( font == NULL ) return NULL;

	if ( font->refcount > 1 ||
		 font == default_font ||
		 font == wndbutton_font )
	{
		// We need to create a new font or find a matching font that exists, because the old font is still being used.
		if ( font != default_font && font != wndbutton_font )
			font->refcount--;

		fnt = mgui_font_find( name, font->size, font->flags, font->charset, font->first_char, font->last_char );
		if ( fnt )
		{
			// Woohoo! A matching font! Let's use it
			fnt->refcount++;
			return fnt;
		}
		else
		{
			// D'oh, we need to create a new one for this.
			fnt = mgui_font_create_range( name, font->size, font->flags, font->charset, font->first_char, font->last_char );
			return fnt;
		}
	}

	// Ok, it seems we're the only ones using this font so it's perfectly safe to modify it
	if ( font->name )
		mem_free( font->name );

	len = mstrsize( name );
	font->name = mem_alloc( len );
	mstrcpy( font->name, name, len );

	mgui_font_reinitialize( font );

	return font;
}

MGuiFont* mgui_font_set_size( MGuiFont* font, uint8 size )
{
	MGuiFont* fnt;

	if ( font == NULL ) return NULL;

	if ( font->refcount > 1 ||
		 font == default_font ||
		 font == wndbutton_font )
	{
		if ( font != default_font && font != wndbutton_font )
			font->refcount--;

		fnt = mgui_font_find( font->name, size, font->flags, font->charset, font->first_char, font->last_char );
		if ( fnt )
		{
			fnt->refcount++;
			return fnt;
		}
		else
		{
			fnt = mgui_font_create_range( font->name, size, font->flags, font->charset, font->first_char, font->last_char );
			return fnt;
		}
	}

	font->size = size;
	mgui_font_reinitialize( font );

	return font;
}

MGuiFont* mgui_font_set_flags( MGuiFont* font, uint8 flags )
{
	MGuiFont* fnt;

	if ( font == NULL ) return NULL;

	if ( font->refcount > 1 ||
		 font == default_font ||
		 font == wndbutton_font )
	{
		if ( font != default_font && font != wndbutton_font )
			font->refcount--;

		fnt = mgui_font_find( font->name, font->size, flags, font->charset, font->first_char, font->last_char );
		if ( fnt )
		{
			fnt->refcount++;
			return fnt;
		}
		else
		{
			fnt = mgui_font_create_range( font->name, font->size, flags, font->charset, font->first_char, font->last_char );
			return fnt;
		}
	}

	font->flags = flags;
	mgui_font_reinitialize( font );

	return font;
}

MGuiFont* mgui_font_set_charset( MGuiFont* font, uint8 charset )
{
	MGuiFont* fnt;

	if ( font == NULL ) return NULL;

	if ( font->refcount > 1 ||
		 font == default_font ||
		 font == wndbutton_font )
	{
		if ( font != default_font && font != wndbutton_font )
			font->refcount--;

		fnt = mgui_font_find( font->name, font->size, font->flags, charset, font->first_char, font->last_char );
		if ( fnt )
		{
			fnt->refcount++;
			return fnt;
		}
		else
		{
			fnt = mgui_font_create_range( font->name, font->size, font->flags, charset, font->first_char, font->last_char );
			return fnt;
		}
	}

	font->charset = charset;
	mgui_font_reinitialize( font );

	return font;
}

void mgui_font_reinitialize( MGuiFont* font )
{
	if ( font == NULL || renderer == NULL ) return;

	if ( font->data )
		renderer->destroy_font( font->data );

	font->data = renderer->load_font( font->name, font->size, font->flags, font->charset, font->first_char, font->last_char );
}

static uint8 mgui_font_get_charset( uint32 charset )
{
	switch ( charset )
	{
#ifdef _WIN32
		case CHARSET_ANSI:
			return ANSI_CHARSET;

		case CHARSET_BALTIC:
			return BALTIC_CHARSET;

		case CHARSET_CHINESE_TRAD:
			return CHINESEBIG5_CHARSET;

		case CHARSET_DEFAULT:
			return DEFAULT_CHARSET;

		case CHARSET_EASTEUR:
			return EASTEUROPE_CHARSET;

		case CHARSET_CHINESE_SIMPLE:
			return GB2312_CHARSET;

		case CHARSET_GREEK:
			return GREEK_CHARSET;

		case CHARSET_HANGUL_KOREAN:
			return HANGUL_CHARSET;

		case CHARSET_APPLE:
			return MAC_CHARSET;

		case CHARSET_OEM:
			return OEM_CHARSET;

		case CHARSET_CYRILLIC:
			return RUSSIAN_CHARSET;

		case CHARSET_JAPANESE:
			return SHIFTJIS_CHARSET;

		case CHARSET_SYMBOL:
			return SYMBOL_CHARSET;

		case CHARSET_TURKISH:
			return TURKISH_CHARSET;

		case CHARSET_VIETNAMESE:
			return VIETNAMESE_CHARSET;
#else
#endif /* _WIN32 */
		default:
			return 0;
	}
}
