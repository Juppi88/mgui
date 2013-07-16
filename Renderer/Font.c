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

MGuiFont*	default_font = NULL;	// Default font for all elements
MGuiFont*	wndbutton_font = NULL;	// Font used for the close button X

static MGuiFont* mgui_font_find( const char_t* name, uint8 size, uint8 flags, uint8 charset, char_t firstc, char_t lastc );

void mgui_fontmgr_initialize( void )
{
	fonts = list_create();

	default_font = mgui_font_create_range( DEFAULT_FONT, 10, FFLAG_NONE, CHARSET_ANSI, 0, 0 );
	wndbutton_font = mgui_font_create_range( DEFAULT_FONT, 10, FFLAG_NONE, CHARSET_ANSI, 'X', 'X' );
}

void mgui_fontmgr_shutdown( void )
{
	node_t *node, *tmp;
	MGuiFont* font;

	if ( default_font ) mgui_font_destroy( default_font );
	if ( wndbutton_font ) mgui_font_destroy( wndbutton_font );

	list_foreach_safe( fonts, node, tmp )
	{
		font = (MGuiFont*)node;
		mgui_font_destroy( font );
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
			font->data = renderer->load_font( font->name, font->size, font->flags, font->charset, font->first_char, font->last_char );
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
	else
	{
		len = mstrsize( name );

		font = (MGuiFont*)mem_alloc_clean( sizeof(*font) );
		font->name = mem_alloc( len );
		font->size = size;
		font->flags = flags;
		font->charset = charset;
		font->first_char = firstc;
		font->last_char = lastc;
		font->refcount = 1;

		mstrcpy( font->name, name, len );

		if ( renderer )
			font->data = renderer->load_font( name, size, flags, charset, firstc, lastc );

		list_push( fonts, &font->node );

		return font;
	}
}

void mgui_font_destroy( MGuiFont* font )
{
	if ( font == NULL ) return;
	if ( --(font->refcount) ) return;

	if ( font->data )
		renderer->destroy_font( font->data );

	if ( font->name )
		mem_free( font->name );

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
			 font->last_char == lastc )
			 return font;
	}

	return NULL;
}

MGuiFont* mgui_font_set_font( MGuiFont* font, const char_t* name )
{
	size_t len;
	MGuiFont* fnt;

	if ( font == NULL ) return NULL;

	if ( font->refcount > 1 )
	{
		// We need to create a new font or find a matching font that exists, because the old font is still being used.
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

	if ( font->refcount > 1 )
	{
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

	if ( font->refcount > 1 )
	{
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

	if ( font->refcount > 1 )
	{
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
