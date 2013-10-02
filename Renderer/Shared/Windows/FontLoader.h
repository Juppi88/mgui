/**********************************************************************
 *
 * PROJECT:		Mylly GUI - Generic Windows font loader
 * FILE:		FontLoader.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		Shared code for MGUI renderer font loading on Windows.
 *
 *				(c) Tuomo Jauhiainen 2013
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_FONTLOADER_H
#define __MYLLY_GUI_FONTLOADER_H

#include "MGUI/Renderer/Renderer.h"

#define MAX_FONT_LEN 32

/**
 * A struct which will be filled with info about the loaded font.
 */
typedef struct {
	float**	tex_coords;
	uint8	spacing;
	uint32	width;
	uint32	height;
} MGuiFontInfo;

__BEGIN_DECLS

/**
 * Callback prototype for texture creation request from the font loader.
 * The texture should be in A4R4G4B4 (16bit) format.
 *
 * @arg data: pointer to font data instance supplied to mgui_load_font
 * @arg width: Requested texture width
 * @arg height: Requested texture height
 * @arg texture: (Return value) Pointer to the created texture
 * @arg texture_pitch: (Return value) Bytes per row in the texture
 */
typedef void ( *create_texture_t )( void* data, uint32 width, uint32 height, void** texture, uint32* texture_pitch );


/**
 * mgui_font_load will load the requested font and draw it onto a texture. The correct texture will be
 * passed on to the function via a create_texture_t callback.
 *
 * @arg name: Name of the requested font
 * @arg size: Size of the font
 * @arg flags: Font flags (see FFLAG_* in MGUI.h)
 * @arg charset: Requested charset (Windows charsets, not MGUI!)
 * @arg firstc: First character in range
 * @arg lastc: Last character in range
 * @arg info: (Return value) Struct containing info about the loaded font
 * @arg callback: Callback which will be used to request the creation of a sufficient texture for the font
 * @arg callback_data: Pointer to font data that is passed to the callback function
 * @returns: true if the font was created successfully, false otherwise
 */
bool mgui_load_font( const char* name, uint8 size, uint8 flags, uint8 charset, uint32 firstc, uint32 lastc,
					 MGuiFontInfo* info, create_texture_t callback, void* callback_data );

__END_DECLS

#endif /* __MYLLY_GUI_FONTLOADER_H */
