/**
 *
 * @file		Sprite.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI sprite related functions.
 *
 * @details		Functions and structures related to GUI sprites.
 *
 **/

#include "Sprite.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

extern MGuiRenderer* renderer;

// --------------------------------------------------

// Sprite callback handlers
static void mgui_sprite_render( MGuiSprite* sprite );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_sprite_render,
	NULL, /* post_render */
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
	NULL, /* on_mouse_move */
	NULL, /* on_mouse_wheel */
	NULL, /* on_character */
	NULL  /* on_key_press */
};

// --------------------------------------------------

/**
 * @brief Creates a sprite.
 *
 * @details This function creates a GUI sprite. If the parent element
 * is NULL, the sprite will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @returns A pointer to the created sprite
 */
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

/**
 * @brief Creates a sprite (extended).
 *
 * @details This function creates a GUI sprite with the given parameters.
 * If the parent element is NULL, the sprite will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @param x The absolute x coordinate relative to the parent
 * @param y The absolute y coordinate relative to the parent
 * @param flags Any additional flags that will be applied as a bitmask (see @ref MGUI_FLAGS)
 * @param col The colour of the sprite as a 32bit hex integer
 * @param texture Filename of the texture file to be loaded
 *
 * @returns A pointer to the created sprite
 */
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

static void mgui_sprite_render( MGuiSprite* sprite )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;
	rectangle_t* r;

	if ( sprite == NULL ) return;
	if ( _sprite->texture == NULL ) return;

	r = &sprite->bounds;

	renderer->set_draw_colour( &sprite->colour );
	renderer->draw_textured_rect( _sprite->texture->data, r->x, r->y, r->w, r->h, _sprite->uv );
}

/**
 * @brief Returns the name of the texture a sprite is using.
 *
 * @details This function returns the name of the texture a sprite is using.
 *
 * @param sprite The sprite to get the texture of
 * @returns Name of the texture used
 */
const char_t* mgui_sprite_get_texture( MGuiSprite* sprite )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL )
		return NULL;

	if ( _sprite->texture == NULL )
		return NULL;

	return _sprite->texture->filename;
}

/**
 * @brief Sets the texture of a sprite.
 *
 * @details This function sets and loads a texture for a sprite.
 *
 * @param sprite The sprite to set the texture of
 * @param texture The filename of the texture to be loaded
 */
void mgui_sprite_set_texture( MGuiSprite* sprite, const char_t* texture )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL || texture == NULL )
		return;

	if ( _sprite->texture )
		mgui_texture_destroy( _sprite->texture );

	_sprite->texture = mgui_texture_create( texture );
}

/**
 * @brief Returns the size of the texture a sprite is using.
 *
 * @details This function returns the absolute size of the texture
 * a sprite is using.
 *
 * @param sprite The sprite to get the texture size of
 * @param size A pointer to a vectorscreen_t struct that will receive the size of the texture
 */
void mgui_sprite_get_texture_size( MGuiSprite* sprite, vectorscreen_t* size )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL || size == NULL )
		return;

	if ( _sprite->texture == NULL )
	{
		size->ux = 0;
		size->uy = 0;
		return;
	}

	size->ux = _sprite->texture->width;
	size->uy = _sprite->texture->height;
}

/**
 * @brief Returns the size of the texture a sprite is using.
 *
 * @details This function returns the absolute size of the texture
 * a sprite is using.
 *
 * @param sprite The sprite to get the texture size of
 * @param width A pointer to a 16bit integer that will receive the width of the texture
 * @param height A pointer to a 16bit integer that will receive the height of the texture
 */
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

/**
 * @brief Resizes a sprite to match the size of the texture.
 *
 * @details This function resizes a sprite to match the size of the texture
 * the sprite is using.
 *
 * @param sprite The sprite to resize
 */
void mgui_sprite_resize( MGuiSprite* sprite )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL  )
		return;

	if ( _sprite->texture == NULL )
		return;

	mgui_set_abs_size_i( sprite, _sprite->texture->width, _sprite->texture->height );
}

/**
 * @brief Returns the texture coordinates of a sprite.
 *
 * @details This function returns the texture coordinates used by a sprite.
 * Texture coordinates are scaled between 0 and 1.
 *
 * @param sprite The sprite to get the texture coordinates of
 * @param u1, v1 Pointers to floats that will receive the coordinates for the top left corner
 * @param u2, v2 Pointers to floats that will receive the coordinates for the bottom right corner
 */
void mgui_sprite_get_uv( MGuiSprite* sprite, float* u1, float* v1, float* u2, float* v2 )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( u1 == NULL || v1 == NULL ||
		 u2 == NULL || v2 == NULL )
		 return;

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

/**
 * @brief Sets the texture coordinates of a sprite.
 *
 * @details This function sets the texture coordinates used by a sprite.
 * This is useful if you only want to use a portion of the texture.
 * Texture coordinates are scaled between 0 and 1.
 *
 * @param sprite The sprite to set the texture coordinates of
 * @param u1, v1 Coordinates for the top left corner
 * @param u2, v2 Coordinates for the bottom right corner
 */
void mgui_sprite_set_uv( MGuiSprite* sprite, float u1, float v1, float u2, float v2 )
{
	struct MGuiSprite* _sprite = (struct MGuiSprite*)sprite;

	if ( sprite == NULL )
		return;

	_sprite->uv[0] = u1;
	_sprite->uv[1] = v1;
	_sprite->uv[2] = u2;
	_sprite->uv[3] = v2;

	mgui_element_request_redraw( sprite );
}
