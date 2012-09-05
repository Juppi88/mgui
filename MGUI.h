/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		MGUI.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		Public API header for Mylly GUI - A basic GUI library
 *				written in C.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_H
#define __MYLLY_GUI_H

#include "Defines.h"
#include "Types/Types.h"

// Interface types
typedef struct control_s	control_t;
typedef struct element_s	element_t;

#define MGUI_ELEMENT_DECL(x) typedef element_t x

// GUI elements
MGUI_ELEMENT_DECL( button_t );
MGUI_ELEMENT_DECL( editbox_t );
MGUI_ELEMENT_DECL( label_t );
MGUI_ELEMENT_DECL( window_t );
/*typedef struct button_s			button_t;
typedef struct checkbox_s		checkbox_t;
typedef struct droplist_s		droplist_t;
typedef struct gridlist_s		gridlist_t;
typedef struct inputbox_s		inputbox_t;
typedef struct label_s			label_t;
typedef struct memobox_s		memobox_t;
typedef struct menubar_s		menubar_t;
typedef struct progbar_s		progbar_t;
typedef struct radiobutton_s	radiobutton_t;
typedef struct scrollbar_s		scrollbar_t;
typedef struct slider_s			slider_t;
typedef struct sprite_s			sprite_t;
typedef struct tab_s			tab_t;
typedef struct window_s			window_t;*/

#define cast_elem(x) ((element_t*)x)
#define cast_ctrl(x) ((control_t*)x)
#define cast_node(x) ((node_t*)x)

typedef enum
{
	GUIREND_NULL,
	GUIREND_OPENGL,
	GUIREND_GDIPLUS,
	GUIREND_DX8,
	GUIREND_DX9,
	GUIREND_DX10,
	GUIREND_DX11,
} MGUI_RENDERER;

enum MGUI_ALIGNMENT
{
	ALIGN_NONE		= 0x00,
	ALIGN_LEFT		= 0x01,
	ALIGN_RIGHT		= 0x02,
	ALIGN_TOP		= 0x04,
	ALIGN_BOTTOM	= 0x08,
	ALIGN_CENTERV	= 0x10,
	ALIGN_CENTERH	= 0x20,
	ALIGN_CENTER	= (ALIGN_CENTERV|ALIGN_CENTERH),
};

enum MGUI_FLAGS
{
	FLAG_NONE		= 0x0000,	/* All flags disabled */
	FLAG_VISIBLE	= 0x0001,	/* Is this element visible */
	FLAG_DISABLED	= 0x0002,	/* Is this element disabled */
	FLAG_BACKGROUND	= 0x0004,	/* Does the element have a background */
	FLAG_BORDER		= 0x0008,	/* Element has a border */
	FLAG_SHADOW		= 0x0010,	/* Does the element cast a shadow */
	FLAG_CLIP		= 0x0020,	/* Can this element be clipped if necessary */
	FLAG_WRAP		= 0x0040,	/* Wrap the text if possible */
};

enum MGUI_FONT_FLAGS
{
	FFLAG_NONE		= 0x00, /* All flags disabled */
	FFLAG_BOLD		= 0x01,	/* Bold font */
	FFLAG_ITALIC	= 0x02,	/* Italic font */
	FFLAG_ULINE		= 0x04,	/* Underlined font */
	FFLAG_STRIKE	= 0x08,	/* Strike out */
	FFLAG_SHADOW	= 0x10,	/* Cast a shadow */
	FFLAG_COLOUR	= 0x20,	/* Accept colour tags */
	FFLAG_NOAA		= 0x40,	/* Disable edge smoothing (if possible) */
};

typedef enum
{
	EVENT_HOVER_ENTER,	/* Mouse enters the element boundaries */
	EVENT_HOVER_LEAVE,	/* Mouse leaves the element boundaries */
	EVENT_CLICK,		/* Element is clicked (lmb down) */
	EVENT_RELEASE,		/* Element is released (lmb up) */
	EVENT_DRAG,			/* Element is dragged */
	EVENT_INPUT_CHANGE,	/* User modifies the text of an input element */
	EVENT_INPUT_RETURN,	/* User presses return while an input element is focused */
	EVENT_WINDOW_CLOSE,	/* Window is closed from the close button */
} MGUI_EVENT;

typedef struct 
{
	MGUI_EVENT		type;		// Event type
	element_t*		element;	// The element which triggered this event
	void*			data;		// User specified data

	union
	{
		struct {
			uint16	x;			// Mouse x co-ordinate
			uint16	y;			// Mouse y co-ordinate
		} mouse;

		struct {
			uint32	key;		// Keyboard key
		} keyboard;
	};
} guievent_t;

// GUI event hook type
typedef void ( *mgui_event_handler_t )( guievent_t* event );

__BEGIN_DECLS

/* Library initialization and processing */
MYLLY_API void				mgui_initialize				( MGUI_RENDERER renderer, void* window );
MYLLY_API void				mgui_shutdown				( void );
MYLLY_API void				mgui_process				( void );
MYLLY_API void				mgui_redraw					( void );
MYLLY_API void				mgui_set_renderer			( MGUI_RENDERER renderer );
MYLLY_API void				mgui_set_skin				( const char_t* skinimg );

/* Misc functions */
MYLLY_API element_t*		mgui_get_focus				( void );
MYLLY_API void				mgui_set_focus				( element_t* element );

/* Element relations */
MYLLY_API void				mgui_add_child				( control_t* parent, element_t* child );
MYLLY_API void				mgui_remove_child			( element_t* child );
MYLLY_API void				mgui_move_forward			( element_t* child );
MYLLY_API void				mgui_move_backward			( element_t* child );
MYLLY_API void				mgui_send_to_top			( element_t* child );
MYLLY_API void				mgui_send_to_bottom			( element_t* child );
MYLLY_API bool				mgui_is_child_of			( control_t* parent, element_t* child );

/* Controls/canvases */
MYLLY_API control_t*		mgui_create_control			( void );
MYLLY_API void				mgui_destroy_control		( control_t* control );

/* Element constructors */
MYLLY_API button_t*			mgui_create_button			( control_t* parent );
MYLLY_API editbox_t*		mgui_create_editbox			( control_t* parent );
MYLLY_API label_t*			mgui_create_label			( control_t* parent );
MYLLY_API window_t*			mgui_create_window			( control_t* parent );
MYLLY_API void				mgui_destroy_element		( element_t* element );

/* Generic element manipulation */
MYLLY_API void				mgui_get_pos				( element_t* element, vector2_t* pos );
MYLLY_API void				mgui_set_pos				( element_t* element, float x, float y );
MYLLY_API void				mgui_set_pos_v				( element_t* element, const vector2_t* pos );
MYLLY_API void				mgui_get_size				( element_t* element, vector2_t* size );
MYLLY_API void				mgui_set_size				( element_t* element, float w, float h );
MYLLY_API void				mgui_set_size_v				( element_t* element, const vector2_t* size );

MYLLY_API uint32			mgui_get_colour				( element_t* element );
MYLLY_API void				mgui_set_colour				( element_t* element, uint32 colour );
MYLLY_API uint8				mgui_get_alpha				( element_t* element );
MYLLY_API void				mgui_set_alpha				( element_t* element, uint8 alpha );
MYLLY_API uint32			mgui_get_text_colour		( element_t* element );
MYLLY_API void				mgui_set_text_colour		( element_t* element, uint32 colour );

MYLLY_API const char_t*		mgui_get_text				( element_t* element );
MYLLY_API uint32			mgui_get_text_len			( element_t* element );
MYLLY_API void				mgui_set_text				( element_t* element, const char_t* fmt, ... );
MYLLY_API void				mgui_set_text_s				( element_t* element, const char_t* text );
MYLLY_API uint32			mgui_get_alignment			( element_t* element );
MYLLY_API void				mgui_set_alignment			( element_t* element, uint32 alignment );
MYLLY_API void				mgui_get_text_padding		( element_t* element, uint8* top, uint8* bottom, uint8* left, uint8* right );
MYLLY_API void				mgui_set_text_padding		( element_t* element, uint8 top, uint8 bottom, uint8 left, uint8 right );

MYLLY_API const char_t*		mgui_get_font_name			( element_t* element );
MYLLY_API uint8				mgui_get_font_size			( element_t* element );
MYLLY_API uint8				mgui_get_font_flags			( element_t* element );
MYLLY_API void				mgui_set_font_name			( element_t* element, const char_t* font );
MYLLY_API void				mgui_set_font_size			( element_t* element, uint8 size );
MYLLY_API void				mgui_set_font_flags			( element_t* element, uint8 flags );
MYLLY_API void				mgui_set_font				( element_t* element, const char_t* font, uint8 size, uint8 flags, uint8 charset );

MYLLY_API uint32			mgui_get_flags				( element_t* element );
MYLLY_API void				mgui_set_flags				( element_t* element, const uint32 flags );
MYLLY_API void				mgui_add_flags				( element_t* element, const uint32 flags );
MYLLY_API void				mgui_remove_flags			( element_t* element, const uint32 flags );

MYLLY_API void				mgui_set_event_handler		( element_t* element, mgui_event_handler_t handler, void* data );

/* Editbox functions */
MYLLY_API void				mgui_editbox_get_selection	( editbox_t* editbox, char_t* buf, size_t buflen );
MYLLY_API void				mgui_editbox_select_text	( editbox_t* editbox, uint32 begin, uint32 end );
MYLLY_API uint32			mgui_editbox_get_cursor_pos	( editbox_t* editbox );
MYLLY_API void				mgui_editbox_set_cursor_pos	( editbox_t* editbox, uint32 pos );

/* Label functions */
MYLLY_API void				mgui_label_make_text_fit	( label_t* label );

/* Window functions */
MYLLY_API bool				mgui_window_get_titlebar	( window_t* window );
MYLLY_API void				mgui_window_set_titlebar	( window_t* window, bool enabled );
MYLLY_API uint32			mgui_window_get_title_col	( window_t* window );
MYLLY_API void				mgui_window_set_title_col	( window_t* window, uint32 colour );
MYLLY_API bool				mgui_window_get_closebtn	( window_t* window );
MYLLY_API void				mgui_window_set_closebtn	( window_t* window, bool enabled );
MYLLY_API void				mgui_window_get_drag_offset	( window_t* window, vectorscreen_t* pos );

__END_DECLS

#endif /* __MYLLY_GUI_H */
