/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Sprite.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI sprite related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 * 
 **********************************************************************/

#include "Sprite.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

extern MGuiRenderer* renderer;

// --------------------------------------------------

// Sprite callback handlers
static void mgui_sprite_render( MGuiElement* sprite );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_sprite_render,
	NULL, /* process */
	NULL, /* get_clip_region */
	NULL, /* on_bounds_change */
	NULL, /* on_flags_change */
	NULL, /* on_colour_change */
	NULL, /* on_text_change */
	NULL, /* on_mouse_enter */
	NULL, /* on_mouse_leave */
	NULL, /* on_mouse_click */
	NULL, /* on_mouse_release */
	NULL, /* on_mouse_drag */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

// --------------------------------------------------

MGuiSprite* mgui_create_sprite( MGuiElement* parent )
{
	struct MGuiSprite* sprite;

	sprite = mem_alloc_clean( sizeof(*sprite) );
	sprite->flags_int |= INTFLAG_NOTEXT;

	mgui_element_create( cast_elem(sprite), parent );

	sprite->type = GUI_SPRITE;

	sprite->uv[0] = 0.0f; sprite->uv[1] = 0.0f;
	sprite->uv[2] = 1.0f; sprite->uv[3] = 1.0f;
	sprite->scale.x = 1.0f;
	sprite->scale.y = 1.0f;

	// Sprite callbacks
	sprite->callbacks = &callbacks;

	return cast_elem(sprite);
}

MGuiSprite* mgui_create_sprite_ex( MGuiElement* parent, int16 x, int16 y,
								   uint32 flags, uint32 col, const char_t* texture )
{
	MGuiSprite* sprite;

	sprite = mgui_create_sprite( parent );

	mgui_set_abs_pos_i( sprite, x, y );
	mgui_add_flags( sprite, flags );
	mgui_set_colour_i( sprite, col );
	mgui_sprite_set_texture( sprite, texture );
	mgui_sprite_resize( sprite );

	return sprite;
}

static void mgui_sprite_render( MGuiElement* sprite )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;
	rectangle_t* r;

	if ( sprite == NULL ) return;
	if ( _sprite->texture == NULL ) return;

	r = &sprite->bounds;

	renderer->set_draw_colour( &sprite->colour );
	renderer->draw_textured_rect( _sprite->texture->data, r->x, r->y, r->w, r->h, _sprite->uv );
}

const char_t* mgui_sprite_get_texture( MGuiSprite* sprite )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL ) return NULL;
	if ( _sprite->texture == NULL ) return NULL;

	return _sprite->texture->filename;
}

void mgui_sprite_set_texture( MGuiSprite* sprite, const char_t* texture )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL || texture == NULL ) return;

	if ( _sprite->texture )
		mgui_texture_destroy( _sprite->texture );

	_sprite->texture = mgui_texture_create( texture );
}

void mgui_sprite_get_texture_size( MGuiSprite* sprite, vectorscreen_t* size )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL || size == NULL ) return;

	if ( _sprite->texture == NULL )
	{
		size->ux = 0;
		size->uy = 0;
		return;
	}

	size->ux = _sprite->texture->width;
	size->uy = _sprite->texture->height;
}

void mgui_sprite_get_texture_size_i( MGuiSprite* sprite, uint16* width, uint16* height )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL || width == NULL || height == NULL ) return;

	if ( _sprite->texture == NULL )
	{
		*width = 0;
		*height = 0;
		return;
	}

	*width = _sprite->texture->width;
	*height = _sprite->texture->height;
}

void mgui_sprite_resize( MGuiSprite* sprite )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL  ) return;
	if ( _sprite->texture == NULL ) return;

	mgui_set_abs_size_i( sprite, _sprite->texture->width, _sprite->texture->height );
}

void mgui_sprite_get_uv( MGuiSprite* sprite, float* u1, float* v1, float* u2, float* v2 )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL )
	{
		*u1 = *v1 = 0.0f;
		*u2 = *v2 = 0.0f;
		return;
	}

	*u1 = _sprite->uv[0];
	*v1 = _sprite->uv[1];
	*u2 = _sprite->uv[2];
	*v2 = _sprite->uv[3];
}

void mgui_sprite_set_uv( MGuiSprite* sprite, float u1, float v1, float u2, float v2 )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL ) return;

	_sprite->uv[0] = u1;
	_sprite->uv[1] = v1;
	_sprite->uv[2] = u2;
	_sprite->uv[3] = v2;
}
