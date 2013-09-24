/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Texture.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		Internal texture loading.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_TEXTURE_H
#define __MGUI_TEXTURE_H

#include "MGUI.h"
#include "Renderer.h"
#include "Types/List.h"

typedef struct {
	node_t			node;		// Linked list node
	MGuiRendTexture*data;		// Renderer texture data
	char_t*			filename;	// Texture file name
	uint16			width;		// Texture width
	uint16			height;		// Texture height
	uint32			refcount;	// Reference count
} MGuiTexture;

void		mgui_texturemgr_initialize		( void );
void		mgui_texturemgr_shutdown		( void );
void		mgui_texturemgr_initialize_all	( void );
void		mgui_texturemgr_invalidate_all	( void );

MGuiTexture*mgui_texture_create				( const char_t* texture_file );
void		mgui_texture_destroy			( MGuiTexture* texture );

#endif /* __MGUI_TEXTURE_H */
