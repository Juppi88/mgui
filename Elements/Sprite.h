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

#pragma once
#ifndef __MGUI_SPRITE_H
#define __MGUI_SPRITE_H

#include "Element.h"
#include "Texture.h"

/**
 * @brief GUI sprite.
 *
 * @details Sprite is a static image that can be displayed
 * anywhere in the window.
 */
struct MGuiSprite {
	MGuiElement;						///< Inherit MGuiElement members
	MGuiTexture*		texture;		///< Container for texture data
	float				uv[4];			///< An array of texture coordinates for drawing
	vector2_t			scale;			///< Texture scale
	float				rotation;		///< Sprite rotation in radians
	vectorscreen_t		rotation_centre;///< Rotation centre
};

MGuiSprite*	mgui_create_sprite				( MGuiElement* parent );
MGuiSprite*	mgui_create_sprite_ex			( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col, const char_t* texture );

const char_t* mgui_sprite_get_texture		( MGuiSprite* sprite );
void		mgui_sprite_set_texture			( MGuiSprite* sprite, const char_t* texture );
void		mgui_sprite_get_texture_size	( MGuiSprite* sprite, vectorscreen_t* size );
void		mgui_sprite_get_texture_size_i	( MGuiSprite* sprite, uint16* width, uint16* height );
void		mgui_sprite_resize				( MGuiSprite* sprite );
/*void		mgui_sprite_get_scale			( MGuiSprite* sprite, vector2_t* scale );
void		mgui_sprite_set_scale			( MGuiSprite* sprite, const vector2_t* scale );
void		mgui_sprite_get_scale_f			( MGuiSprite* sprite, float* scale_x, float* scale_y );
void		mgui_sprite_set_scale_f			( MGuiSprite* sprite, float scale_x, float scale_y );*/
void		mgui_sprite_get_uv				( MGuiSprite* sprite, float* u1, float* v1, float* u2, float* v2 );
void		mgui_sprite_set_uv				( MGuiSprite* sprite, float u1, float v1, float u2, float v2 );
/*float		mgui_sprite_get_rotation		( MGuiSprite* sprite );
void		mgui_sprite_set_rotation		( MGuiSprite* sprite, float rotation );
void		mgui_sprite_get_rotation_centre	( MGuiSprite* sprite, vectorscreen_t* pos );
void		mgui_sprite_set_rotation_centre	( MGuiSprite* sprite, const vectorscreen_t* pos );
void		mgui_sprite_get_rotation_centre_i( MGuiSprite* sprite, int16* x, int16* y );
void		mgui_sprite_set_rotation_centre_i( MGuiSprite* sprite, int16 x, int16 y );*/

#endif /* __MGUI_SPRITE_H */
