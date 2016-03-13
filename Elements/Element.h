/**
 *
 * @file		Element.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		MGUI element functions.
 *
 * @details		Element function implementations. MGuiElement is the base struct for all GUI widgets.
 *
 **/

#pragma once
#ifndef __MGUI_ELEMENT_H
#define __MGUI_ELEMENT_H

#include "MGUI.h"
#include "Text.h"
#include "Skin.h"
#include "Renderer.h"
#include "Input/Input.h"

/* The following are internal flags and should not be used by the library user */
enum MGUI_INTERNAL_FLAGS {
	INTFLAG_REFRESH		= 1 << 0,	/* Cache texture needs refreshing */
	INTFLAG_FOCUS		= 1 << 1,	/* This element has captured focus */
	INTFLAG_HOVER		= 1 << 2,	/* The element is hovered over by a mouse cursor */
	INTFLAG_PRESSED		= 1 << 3,	/* The element is pressed down */
	INTFLAG_NOTEXT		= 1 << 4,	/* Element has no text */
	INTFLAG_LAYER		= 1 << 5,	/* This element is a main GUI layer */
	INTFLAG_NOPARENT	= 1 << 6,	/* This element has no parent */
};

/* The following values are used only internally. */
typedef enum MGUI_TYPE {
	GUI_NONE,
	GUI_BUTTON,
	GUI_CANVAS,
	GUI_CHECKBOX,
	GUI_DROPLIST,
	GUI_EDITBOX,
	GUI_GRIDLIST,
	GUI_LABEL,
	GUI_LISTBOX,
	GUI_MEMOBOX,
	GUI_MENUBAR,
	GUI_PROGRESSBAR,
	GUI_RADIOBUTTON,
	GUI_SCROLLBAR,
	GUI_SLIDER,
	GUI_SPRITE,
	GUI_TAB,
	GUI_TITLEBAR,
	GUI_WINDOW,
	GUI_WINDOWBUTTON,
	GUI_NUM_TYPES,
	GUI_FORCE_DWORD = 0x7FFFFFFF
} MGUI_TYPE;

/**
 * @brief GUI element.
 *
 * @details This is the base struct for all real world element types.
 * All the other element types are inherited from this generic container.
 */
struct MGuiElement {
	node_t;									///< Linked list node, points to next and previous elements
	uint32					flags;			///< Element property flags (see @ref MGUI_FLAGS)
	uint32					flags_int;		///< Internal flags, used by element processing and rendering
	rectangle_t				bounds;			///< Absolute bounding rectangle of this element (in pixels)
	vectorscreen_t			offset;			///< Offset from parent's position
	float					z_depth;		///< Draw depth index (valid if @ref FLAG_DEPTH_TEST is ebabled and supported)
	MGuiElement*			parent;			///< Pointer to parent element, NULL if this element is a layer
	list_t*					children;		///< List of children elements
	MGUI_TYPE				type;			///< Element type identifier
	vector2_t				pos;			///< Relative position (within parent element)
	vector2_t				size;			///< Relative size (within parent element)
	colour_t				colour;			///< The main colour of the element (usually the background)
	MGuiText*				text;			///< A pointer to a text buffer container, can be NULL if the element type does not support text
	MGuiFont*				font;			///< Default font used to render all the text in this element
	MGuiSkin*				skin;			///< Skin to be used for rendering
	MGuiRendTarget*			cache;			///< Pointer to a texture cache (valid only if @ref FLAG_CACHE_TEXTURE is enabled and supported)
	mgui_event_handler_t	event_handler;	///< User event handler function
	void*					event_data;		///< User-specified data to be passed via event_handler

	/**
	 * @brief Transform information for 3D elements.
	 */

	struct MGuiTransform {
		vector3_t	position;				///< Position of the element in 3D space
		vector3_t	rotation;				///< Rotation of the element in 3D space
		vector2_t	size;					///< Size of the element in 3D space
		matrix4_t	transform;				///< A transform matrix used for rendering this element
	} *transform;							///< Transform information for 3D elements. Valid if @ref FLAG_3D_ENTITY is enabled.

	/**
	 * @brief Internal callback functions.
	 */

	struct MGuiCallbacks {
		void		( *destroy )			( MGuiElement* element );
		void		( *render )				( MGuiElement* element );
		void		( *post_render )		( MGuiElement* element );
		void		( *process )			( MGuiElement* element );

		void		( *get_clip_region )	( MGuiElement* element, rectangle_t** rect );

		void		( *on_bounds_change )	( MGuiElement* element, bool pos, bool size );
		void		( *on_flags_change )	( MGuiElement* element, uint32 old );
		void		( *on_colour_change )	( MGuiElement* element );
		void		( *on_text_change )		( MGuiElement* element );
		void		( *on_mouse_enter )		( MGuiElement* element );
		void		( *on_mouse_leave )		( MGuiElement* element );
		void		( *on_mouse_click )		( MGuiElement* element, int16 x, int16 y, MOUSEBTN button );
		void		( *on_mouse_release )	( MGuiElement* element, int16 x, int16 y, MOUSEBTN button );
		void		( *on_mouse_drag )		( MGuiElement* element, int16 x, int16 y );
		void		( *on_mouse_move )		( MGuiElement* element, int16 x, int16 y );
		void		( *on_mouse_wheel )		( MGuiElement* element, float diff );
		bool		( *on_character )		( MGuiElement* element, char_t c );
		bool		( *on_key_press )		( MGuiElement* element, uint32 key, bool down );
	} *callbacks;							///< Internal callback functions.
};

// Generic element functions
void			mgui_element_create				( MGuiElement* element, MGuiElement* parent );
void			mgui_element_destroy			( MGuiElement* element );
void			mgui_element_render				( MGuiElement* element );
void			mgui_element_render_cache		( MGuiElement* element, bool draw_self );
void			mgui_element_process			( MGuiElement* element );
void			mgui_element_initialize			( MGuiElement* element );
void			mgui_element_invalidate			( MGuiElement* element );

void			mgui_element_resize_cache		( MGuiElement* element );
void			mgui_element_request_redraw		( MGuiElement* element );
void			mgui_element_request_redraw_all	( void );

MGuiElement*	mgui_get_element_at				( int16 x, int16 y );

void			mgui_add_child					( MGuiElement* parent, MGuiElement* child );
void			mgui_remove_child				( MGuiElement* child );
void			mgui_move_forward				( MGuiElement* child );
void			mgui_move_backward				( MGuiElement* child );
void			mgui_send_to_top				( MGuiElement* child );
void			mgui_send_to_bottom				( MGuiElement* child );
bool			mgui_is_child_of				( MGuiElement* parent, MGuiElement* child );

void			mgui_element_update_abs_pos		( MGuiElement* element );
void			mgui_element_update_abs_size	( MGuiElement* element );
void			mgui_element_update_rel_pos		( MGuiElement* element );
void			mgui_element_update_rel_size	( MGuiElement* element );
void			mgui_element_update_child_pos	( MGuiElement* element );

void			mgui_get_pos					( MGuiElement* element, vector2_t* pos );
void			mgui_get_size					( MGuiElement* element, vector2_t* size );
void			mgui_set_pos					( MGuiElement* element, const vector2_t* pos );
void			mgui_set_size					( MGuiElement* element, const vector2_t* size );
void			mgui_get_abs_pos				( MGuiElement* element, vectorscreen_t* pos );
void			mgui_get_abs_size				( MGuiElement* element, vectorscreen_t* size );
void			mgui_set_abs_pos				( MGuiElement* element, const vectorscreen_t* pos );
void			mgui_set_abs_size				( MGuiElement* element, const vectorscreen_t* size );
void			mgui_get_pos_f					( MGuiElement* element, float* x, float* y );
void			mgui_get_size_f					( MGuiElement* element, float* w, float* h );
void			mgui_set_pos_f					( MGuiElement* element, float x, float y );
void			mgui_set_size_f					( MGuiElement* element, float w, float h );
void			mgui_get_abs_pos_i				( MGuiElement* element, int16* x, int16* y );
void			mgui_get_abs_size_i				( MGuiElement* element, uint16* w, uint16* h );
void			mgui_set_abs_pos_i				( MGuiElement* element, int16 x, int16 y );
void			mgui_set_abs_size_i				( MGuiElement* element, uint16 w, uint16 h );

float			mgui_get_z_depth				( MGuiElement* element );
void			mgui_set_z_depth				( MGuiElement* element, float depth );

void			mgui_get_3d_position			( MGuiElement* element, vector3_t* pos );
void			mgui_set_3d_position			( MGuiElement* element, const vector3_t* pos );
void			mgui_get_3d_rotation			( MGuiElement* element, vector3_t* rot );
void			mgui_set_3d_rotation			( MGuiElement* element, const vector3_t* rot );
void			mgui_get_3d_size				( MGuiElement* element, vector2_t* size );
void			mgui_set_3d_size				( MGuiElement* element, const vector2_t* size );
void			mgui_set_3d_transform			( MGuiElement* element, const vector3_t* pos, const vector3_t* rot, const vector2_t* size );

void			mgui_element_update_transform	( MGuiElement* element );

void			mgui_get_colour					( MGuiElement* element, colour_t* col );
void			mgui_set_colour					( MGuiElement* element, const colour_t* col );
void			mgui_get_text_colour			( MGuiElement* element, colour_t* col );
void			mgui_set_text_colour			( MGuiElement* element, const colour_t* col );
void			mgui_get_text_size				( MGuiElement* element, vectorscreen_t* size );
void			mgui_get_text_size_i			( MGuiElement* element, uint16* w, uint16* h );
uint32			mgui_get_colour_i				( MGuiElement* element );
void			mgui_set_colour_i				( MGuiElement* element, uint32 hex );
uint32			mgui_get_text_colour_i			( MGuiElement* element );
void			mgui_set_text_colour_i			( MGuiElement* element, uint32 hex );
uint8			mgui_get_alpha					( MGuiElement* element );
void			mgui_set_alpha					( MGuiElement* element, uint8 alpha );

const char_t*	mgui_get_text					( MGuiElement* element );
char_t*			mgui_get_text_buffer			( MGuiElement* element, char_t* buf, size_t buflen );
uint32			mgui_get_text_len				( MGuiElement* element );
void			mgui_set_text					( MGuiElement* element, const char_t* fmt, ... );
void			mgui_set_text_s					( MGuiElement* element, const char_t* text );
uint32			mgui_get_alignment				( MGuiElement* element );
void			mgui_set_alignment				( MGuiElement* element, uint32 alignment );
void			mgui_get_text_padding			( MGuiElement* element, uint8* top, uint8* bottom, uint8* left, uint8* right );
void			mgui_set_text_padding			( MGuiElement* element, uint8 top, uint8 bottom, uint8 left, uint8 right );

const char_t*	mgui_get_font_name				( MGuiElement* element );
uint8			mgui_get_font_size				( MGuiElement* element );
uint8			mgui_get_font_flags				( MGuiElement* element );
void			mgui_set_font_name				( MGuiElement* element, const char_t* font );
void			mgui_set_font_size				( MGuiElement* element, uint8 size );
void			mgui_set_font_flags				( MGuiElement* element, uint8 flags );
void			mgui_set_font					( MGuiElement* element, const char_t* font, uint8 size, uint8 flags, uint8 charset );

uint32			mgui_get_flags					( MGuiElement* element );
void			mgui_add_flags					( MGuiElement* element, uint32 flags );
void			mgui_remove_flags				( MGuiElement* element, uint32 flags );

void			mgui_set_event_handler			( MGuiElement* element, mgui_event_handler_t handler, void* data );

#endif /* __MGUI_ELEMENT_H */
