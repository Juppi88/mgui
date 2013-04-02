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

MGuiFont* mgui_font_create( const char_t* name, uint8 size, uint8 flags, uint8 charset )
{
	return mgui_font_create_range( name, size, flags, charset, 0, 0 );
}

MGuiFont* mgui_font_create_range( const char_t* name, uint8 size, uint8 flags, uint8 charset, char_t firstc, char_t lastc )
{
	MGuiFont* font;
	size_t len;

	len = mstrsize( name );

	font = (MGuiFont*)mem_alloc( sizeof(*font) );
	font->name = mem_alloc( len );
	font->size = size;
	font->flags = flags;
	font->charset = charset;

	mstrcpy( font->name, name, len );
	font->data = renderer->load_font( name, size, flags, charset, firstc, lastc );

	return font;
}

MGuiFont* mgui_font_create_default( void )
{
	return mgui_font_create_range( DEFAULT_FONT, 10, FFLAG_NONE, CHARSET_ANSI, 0, 0 );
}

void mgui_font_destroy( MGuiFont* font )
{
	assert( font != NULL );

	if ( font->data )
		renderer->destroy_font( font->data );

	if ( font->name )
		mem_free( font->name );

	mem_free( font );
}

void mgui_font_set_font( MGuiFont* font, const char_t* name )
{
	size_t len;

	assert( font != NULL );

	if ( font->name )
		mem_free( font->name );

	len = mstrsize( name );
	font->name = mem_alloc( len );
	mstrcpy( font->name, name, len );

	if ( font->data )
		renderer->destroy_font( font->data );

	font->data = renderer->load_font( name, font->size, font->flags, font->charset, 0, 0 );
}

void mgui_font_set_size( MGuiFont* font, uint8 size )
{
	assert( font != NULL );

	font->size = size;

	if ( font->data )
		renderer->destroy_font( font->data );

	font->data = renderer->load_font( font->name, font->size, font->flags, font->charset, 0, 0 );
}

void mgui_font_set_flags( MGuiFont* font, uint8 flags )
{
	assert( font != NULL );

	font->flags = flags;

	if ( font->data )
		renderer->destroy_font( font->data );

	font->data = renderer->load_font( font->name, font->size, font->flags, font->charset, 0, 0 );
}

void mgui_font_set_charset( MGuiFont* font, uint8 charset )
{
	assert( font != NULL );

	font->charset = charset;

	if ( font->data )
		renderer->destroy_font( font->data );

	font->data = renderer->load_font( font->name, font->size, font->flags, font->charset, 0, 0 );
}
