/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Texture.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		Internal texture loading.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "Texture.h"
#include "Renderer.h"
#include "Stringy/Stringy.h"
#include "Platform/Alloc.h"

extern MGuiRenderer* renderer;
static list_t* textures = NULL;

static MGuiTexture* mgui_texture_find( const char_t* name );

void mgui_texturemgr_initialize( void )
{
	textures = list_create();
}

void mgui_texturemgr_shutdown( void )
{
	node_t *node, *tmp;

	list_foreach_safe( textures, node, tmp )
	{
		mgui_texture_destroy( (MGuiTexture*)node );
	}

	list_destroy( textures );
	textures = NULL;
}

void mgui_texturemgr_initialize_all( void )
{
	node_t* node;
	MGuiTexture* texture;
	uint32 width, height;

	if ( renderer == NULL ) return;

	list_foreach( textures, node )
	{
		texture = (MGuiTexture*)node;

		if ( texture->data == NULL )
		{
			texture->data = renderer->load_texture( texture->filename, &width, &height );
			texture->width = (uint16)width;
			texture->height = (uint16)height;
		}
	}
}

void mgui_texturemgr_invalidate_all( void )
{
	node_t* node;
	MGuiTexture* texture;

	if ( renderer == NULL ) return;

	list_foreach( textures, node )
	{
		texture = (MGuiTexture*)node;

		if ( texture->data )
		{
			renderer->destroy_texture( texture->data );
			texture->data = NULL;
		}
	}
}

MGuiTexture* mgui_texture_create( const char_t* texture_file )
{
	MGuiTexture* texture;
	uint32 width = 0, height = 0;

	if ( texture_file == NULL ) return NULL;

	texture = mgui_texture_find( texture_file );
	if ( texture )
	{
		texture->refcount++;
		return texture;
	}

	texture = mem_alloc_clean( sizeof(*texture) );
	texture->filename = str_dup( texture_file, 0 );
	texture->refcount = 1;

	if ( renderer != NULL )
	{
		texture->data = renderer->load_texture( texture_file, &width, &height );
		texture->width = (uint16)width;
		texture->height = (uint16)height;
	}

	list_push( textures, &texture->node );

	return texture;
}

void mgui_texture_destroy( MGuiTexture* texture )
{
	if ( texture == NULL ) return;
	if ( --(texture->refcount) ) return;

	if ( texture->data )
		renderer->destroy_texture( texture->data );

	SAFE_DELETE( texture->filename );

	list_remove( textures, &texture->node );
	mem_free( texture );
}

static MGuiTexture* mgui_texture_find( const char_t* name )
{
	MGuiTexture* texture;
	node_t* node;

	if ( name == NULL ) return NULL;

	list_foreach( textures, node )
	{
		texture = (MGuiTexture*)node;

#ifdef _WIN32
		if ( mstrcaseequal( texture->filename, name ) )
#else
		if ( mstrequal( texture->filename, name ) )
#endif
		{
			return texture;
		}
	}

	return NULL;
}
