/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Sprite.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI sprite related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 * 
 **********************************************************************/

#pragma once
#ifndef __MGUI_SPRITE_H
#define __MGUI_SPRITE_H

#include "Element.h"
#include "Texture.h"

struct MGuiSprite
{
	MGuiElement;
	MGuiTexture*		texture;		// Texture data
	float				u1, v1;			// Texture coordinates 1
	float				u2, v2;			// Texture coordinates 2
	vector2_t			scale;			// Texture scaling
	float				rotation;		// Rotation in radians
	vectorscreen_t		rotation_centre;// Rotation centre
};

MGuiSprite*			mgui_create_sprite					( MGuiElement* parent );
MGuiSprite*			mgui_create_sprite_ex				( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col, const char_t* texture );

const char_t*		mgui_sprite_get_texture				( MGuiSprite* sprite );
void				mgui_sprite_set_texture				( MGuiSprite* sprite, const char_t* texture );
void				mgui_sprite_get_texture_size		( MGuiSprite* sprite, vectorscreen_t* size );
void				mgui_sprite_get_texture_size_i		( MGuiSprite* sprite, uint16* width, uint16* height );
void				mgui_sprite_resize					( MGuiSprite* sprite );
/*void				mgui_sprite_get_scale				( MGuiSprite* sprite, vector2_t* scale );
void				mgui_sprite_set_scale				( MGuiSprite* sprite, const vector2_t* scale );
void				mgui_sprite_get_scale_f				( MGuiSprite* sprite, float* scale_x, float* scale_y );
void				mgui_sprite_set_scale_f				( MGuiSprite* sprite, float scale_x, float scale_y );*/
void				mgui_sprite_get_uv					( MGuiSprite* sprite, float* u1, float* v1, float* u2, float* v2 );
void				mgui_sprite_set_uv					( MGuiSprite* sprite, float u1, float v1, float u2, float v2 );
/*float				mgui_sprite_get_rotation			( MGuiSprite* sprite );
void				mgui_sprite_set_rotation			( MGuiSprite* sprite, float rotation );
void				mgui_sprite_get_rotation_centre		( MGuiSprite* sprite, vectorscreen_t* pos );
void				mgui_sprite_set_rotation_centre		( MGuiSprite* sprite, const vectorscreen_t* pos );
void				mgui_sprite_get_rotation_centre_i	( MGuiSprite* sprite, int16* x, int16* y );
void				mgui_sprite_set_rotation_centre_i	( MGuiSprite* sprite, int16 x, int16 y );*/

#endif /* __MGUI_SPRITE_H */
