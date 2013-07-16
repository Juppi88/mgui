/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		MGUI.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		Public API header for Mylly GUI - A basic GUI library
 *				written in C.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_H
#define __MYLLY_GUI_H

#include "stdtypes.h"
#include "Math/MathDefs.h"

// Interface types
typedef struct MGuiElement	MGuiElement;
typedef struct MGuiRenderer	MGuiRenderer;

#define MGUI_ELEMENT_DECL(x) typedef MGuiElement x

// GUI elements
MGUI_ELEMENT_DECL( MGuiButton );
MGUI_ELEMENT_DECL( MGuiCanvas );
MGUI_ELEMENT_DECL( MGuiEditbox );
MGUI_ELEMENT_DECL( MGuiLabel );
MGUI_ELEMENT_DECL( MGuiMemobox );
MGUI_ELEMENT_DECL( MGuiScrollbar );
MGUI_ELEMENT_DECL( MGuiWindow );

#define cast_elem(x) ((MGuiElement*)x)
#define cast_node(x) ((node_t*)x)

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
	FLAG_NONE				= 0,		/* All flags disabled */
	FLAG_VISIBLE			= 1 << 0,	/* This element is visible and can be processed */
	FLAG_DISABLED			= 1 << 1,	/* This element is inactive (user can't interact with it) */
	FLAG_BACKGROUND			= 1 << 2,	/* The element has a background */
	FLAG_BORDER				= 1 << 3,	/* The element has a border */
	FLAG_SHADOW				= 1 << 4,	/* The element casts a shadow */
	FLAG_DRAGGABLE			= 1 << 5,	/* This element can be dragged */
	FLAG_CLIP				= 1 << 6,	/* Clip text within the element if it exceeds the boundaries */
	FLAG_WRAP				= 1 << 7,	/* Wrap text if it exceeds the boundaries (if applicable) */
	FLAG_AUTO_RESIZE		= 1 << 8,	/* Resize element automatically if parent size changes */
	FLAG_INHERIT_ALPHA		= 1 << 9,	/* Element will inherit alpha from it's parent element */
	FLAG_ANIMATION			= 1 << 10,	/* Enable animations (if applicable) */
	FLAG_TABSTOP			= 1 << 11,	/* Tab press can switch focus to this element */
	FLAG_MOUSECTRL			= 1 << 12,	/* Element triggers mouse input events */
	FLAG_KBCTRL				= 1 << 13,	/* Element triggers keyboard input events and accepts keyboard focus */
	
	// Flags tied to certain element type
	FLAG_WINDOW_TITLEBAR	= 1 << 20,	/* Enable window titlebar */
	FLAG_WINDOW_CLOSEBTN	= 1 << 21,	/* Enable window close button */
	FLAG_EDIT_MASKINPUT		= 1 << 20,	/* Mask user input in editbox */
	FLAG_MEMO_TOPBOTTOM		= 1 << 20,	/* Memobox order is top to bottom */
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

enum MGUI_FONT_CHARSET
{
	CHARSET_ANSI,
	CHARSET_BALTIC,
	CHARSET_CHINESE_TRAD,
	CHARSET_DEFAULT,
	CHARSET_EASTEUR,
	CHARSET_CHINESE_SIMPLE,
	CHARSET_GREEK,
	CHARSET_HANGUL_KOREAN,
	CHARSET_APPLE,
	CHARSET_OEM,
	CHARSET_CYRILLIC,
	CHARSET_JAPANESE,
	CHARSET_SYMBOL,
	CHARSET_TURKISH,
	CHARSET_VIETNAMESE,
};

typedef enum
{
	EVENT_HOVER_ENTER,	/* Mouse enters the element boundaries */
	EVENT_HOVER_LEAVE,	/* Mouse leaves the element boundaries */
	EVENT_CLICK,		/* Element is clicked (lmb down) */
	EVENT_RELEASE,		/* Element is released (lmb up) */
	EVENT_DRAG,			/* Element is dragged */
	EVENT_FOCUS_ENTER,	/* Element receives focus */
	EVENT_FOCUS_EXIT,	/* Element loses focus */
	EVENT_INPUT_CHANGE,	/* User modifies the text of an input element */
	EVENT_INPUT_RETURN,	/* User presses return while an input element is focused */
	EVENT_WINDOW_CLOSE,	/* Window is closed from the close button */
	EVENT_FORCE_DWORD = 0x7FFFFFFF
} MGUI_EVENT;

typedef struct MGuiEvent 
{
	MGUI_EVENT		type;		// Event type
	MGuiElement*	element;	// The element which triggered this event
	void*			data;		// User specified data

	union {
		struct { int16 x, y; } mouse;		// Mouse cursor position
		struct { uint32 key; } keyboard;	// Active keyboard key
	};
} MGuiEvent;

// GUI event hook type
typedef void ( *mgui_event_handler_t )( MGuiEvent* event );


__BEGIN_DECLS

/* Library initialization and processing */
MYLLY_API void				mgui_initialize					( void* wndhandle );
MYLLY_API void				mgui_shutdown					( void );
MYLLY_API void				mgui_process					( void );
MYLLY_API void				mgui_redraw						( void );

MYLLY_API void				mgui_set_renderer				( MGuiRenderer* renderer );
MYLLY_API void				mgui_set_skin					( const char_t* skinimg );

/* Misc functions */
MYLLY_API MGuiElement*		mgui_get_focus					( void );
MYLLY_API void				mgui_set_focus					( MGuiElement* element );

/* Element relations */
MYLLY_API void				mgui_add_child					( MGuiElement* parent, MGuiElement* child );
MYLLY_API void				mgui_remove_child				( MGuiElement* child );
MYLLY_API void				mgui_move_forward				( MGuiElement* child );
MYLLY_API void				mgui_move_backward				( MGuiElement* child );
MYLLY_API void				mgui_send_to_top				( MGuiElement* child );
MYLLY_API void				mgui_send_to_bottom				( MGuiElement* child );
MYLLY_API bool				mgui_is_child_of				( MGuiElement* parent, MGuiElement* child );

/* Element constructors */
MYLLY_API MGuiButton*		mgui_create_button				( MGuiElement* parent );
MYLLY_API MGuiButton*		mgui_create_button_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );
MYLLY_API MGuiEditbox*		mgui_create_canvas				( MGuiElement* parent );
MYLLY_API MGuiEditbox*		mgui_create_editbox				( MGuiElement* parent );
MYLLY_API MGuiEditbox*		mgui_create_editbox_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );
MYLLY_API MGuiLabel*		mgui_create_label				( MGuiElement* parent );
MYLLY_API MGuiLabel*		mgui_create_label_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );
MYLLY_API MGuiMemobox*		mgui_create_memobox				( MGuiElement* parent );
MYLLY_API MGuiMemobox*		mgui_create_memobox_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col );
MYLLY_API MGuiScrollbar*	mgui_create_scrollbar			( MGuiElement* parent );
MYLLY_API MGuiWindow*		mgui_create_window				( MGuiElement* parent );
MYLLY_API MGuiWindow*		mgui_create_window_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );

MYLLY_API void				mgui_element_destroy			( MGuiElement* element );

/* Generic element manipulation */
MYLLY_API void				mgui_get_pos					( MGuiElement* element, vector2_t* pos );
MYLLY_API void				mgui_get_size					( MGuiElement* element, vector2_t* size );
MYLLY_API void				mgui_set_pos					( MGuiElement* element, const vector2_t* pos );
MYLLY_API void				mgui_set_size					( MGuiElement* element, const vector2_t* size );
MYLLY_API void				mgui_get_abs_pos				( MGuiElement* element, vectorscreen_t* pos );
MYLLY_API void				mgui_get_abs_size				( MGuiElement* element, vectorscreen_t* size );
MYLLY_API void				mgui_set_abs_pos				( MGuiElement* element, const vectorscreen_t* pos );
MYLLY_API void				mgui_set_abs_size				( MGuiElement* element, const vectorscreen_t* size );
MYLLY_API void				mgui_get_pos_f					( MGuiElement* element, float* x, float* y );
MYLLY_API void				mgui_get_size_f					( MGuiElement* element, float* w, float* h );
MYLLY_API void				mgui_set_pos_f					( MGuiElement* element, float x, float y );
MYLLY_API void				mgui_set_size_f					( MGuiElement* element, float w, float h );
MYLLY_API void				mgui_get_abs_pos_i				( MGuiElement* element, int16* x, int16* y );
MYLLY_API void				mgui_get_abs_size_i				( MGuiElement* element, uint16* w, uint16* h );
MYLLY_API void				mgui_set_abs_pos_i				( MGuiElement* element, int16 x, int16 y );
MYLLY_API void				mgui_set_abs_size_i				( MGuiElement* element, uint16 w, uint16 h );

MYLLY_API void				mgui_get_colour					( MGuiElement* element, colour_t* col );
MYLLY_API void				mgui_set_colour					( MGuiElement* element, const colour_t* col );
MYLLY_API void				mgui_get_text_colour			( MGuiElement* element, colour_t* col );
MYLLY_API void				mgui_set_text_colour			( MGuiElement* element, const colour_t* col );
MYLLY_API uint32			mgui_get_colour_i				( MGuiElement* element );
MYLLY_API void				mgui_set_colour_i				( MGuiElement* element, uint32 hex );
MYLLY_API uint32			mgui_get_text_colour_i			( MGuiElement* element );
MYLLY_API void				mgui_set_text_colour_i			( MGuiElement* element, uint32 hex );
MYLLY_API uint8				mgui_get_alpha					( MGuiElement* element );
MYLLY_API void				mgui_set_alpha					( MGuiElement* element, uint8 alpha );

MYLLY_API const char_t*		mgui_get_text					( MGuiElement* element );
MYLLY_API uint32			mgui_get_text_len				( MGuiElement* element );
MYLLY_API void				mgui_set_text					( MGuiElement* element, const char_t* fmt, ... );
MYLLY_API void				mgui_set_text_s					( MGuiElement* element, const char_t* text );
MYLLY_API uint32			mgui_get_alignment				( MGuiElement* element );
MYLLY_API void				mgui_set_alignment				( MGuiElement* element, uint32 alignment );
MYLLY_API void				mgui_get_text_padding			( MGuiElement* element, uint8* top, uint8* bottom, uint8* left, uint8* right );
MYLLY_API void				mgui_set_text_padding			( MGuiElement* element, uint8 top, uint8 bottom, uint8 left, uint8 right );

MYLLY_API const char_t*		mgui_get_font_name				( MGuiElement* element );
MYLLY_API uint8				mgui_get_font_size				( MGuiElement* element );
MYLLY_API uint8				mgui_get_font_flags				( MGuiElement* element );
MYLLY_API void				mgui_set_font_name				( MGuiElement* element, const char_t* font );
MYLLY_API void				mgui_set_font_size				( MGuiElement* element, uint8 size );
MYLLY_API void				mgui_set_font_flags				( MGuiElement* element, uint8 flags );
MYLLY_API void				mgui_set_font					( MGuiElement* element, const char_t* font, uint8 size, uint8 flags, uint8 charset );

MYLLY_API uint32			mgui_get_flags					( MGuiElement* element );
MYLLY_API void				mgui_set_flags					( MGuiElement* element, uint32 flags );
MYLLY_API void				mgui_add_flags					( MGuiElement* element, uint32 flags );
MYLLY_API void				mgui_remove_flags				( MGuiElement* element, uint32 flags );

MYLLY_API void				mgui_set_event_handler			( MGuiElement* element, mgui_event_handler_t handler, void* data );

/* Editbox functions */
MYLLY_API void				mgui_editbox_get_selection		( MGuiEditbox* editbox, char_t* buf, size_t buflen );
MYLLY_API void				mgui_editbox_select_text		( MGuiEditbox* editbox, uint32 begin, uint32 end );
MYLLY_API uint32			mgui_editbox_get_cursor_pos		( MGuiEditbox* editbox );
MYLLY_API void				mgui_editbox_set_cursor_pos		( MGuiEditbox* editbox, uint32 pos );

/* Label functions */
MYLLY_API void				mgui_label_make_text_fit		( MGuiLabel* label );

/* Memobox functions */
MYLLY_API void				mgui_memobox_add_line			( MGuiMemobox* memobox, const char* fmt, ... );
MYLLY_API void				mgui_memobox_add_line_col		( MGuiMemobox* memobox, const char* fmt, const colour_t* col, ... );
MYLLY_API void				mgui_memobox_add_line_s			( MGuiMemobox* memobox, const char* text );
MYLLY_API void				mgui_memobox_add_line_col_s		( MGuiMemobox* memobox, const char* text, const colour_t* col );
MYLLY_API void				mgui_memobox_clear				( MGuiMemobox* memobox );
MYLLY_API float				mgui_memobox_get_display_pos	( MGuiMemobox* memobox );
MYLLY_API void				mgui_memobox_set_display_pos	( MGuiMemobox* memobox, float pos );
MYLLY_API uint32			mgui_memobox_get_lines			( MGuiMemobox* memobox );
MYLLY_API void				mgui_memobox_set_lines			( MGuiMemobox* memobox, uint32 lines );
MYLLY_API uint32			mgui_memobox_get_num_lines		( MGuiMemobox* memobox );
MYLLY_API uint32			mgui_memobox_get_history		( MGuiMemobox* memobox );
MYLLY_API void				mgui_memobox_set_history		( MGuiMemobox* memobox, uint32 lines );
MYLLY_API uint32			mgui_memobox_get_margin			( MGuiMemobox* memobox );
MYLLY_API void				mgui_memobox_set_margin			( MGuiMemobox* memobox, uint32 margin );

/* Scrollbar functions */
MYLLY_API float				mgui_scrollbar_get_bar_pos		( MGuiScrollbar* scrollbar );
MYLLY_API void				mgui_scrollbar_set_bar_pos		( MGuiScrollbar* scrollbar, float pos );
MYLLY_API float				mgui_scrollbar_get_bar_size		( MGuiScrollbar* scrollbar );
MYLLY_API void				mgui_scrollbar_set_bar_size		( MGuiScrollbar* scrollbar, float size );
MYLLY_API float				mgui_scrollbar_get_nudge		( MGuiScrollbar* scrollbar );
MYLLY_API void				mgui_scrollbar_set_nudge		( MGuiScrollbar* scrollbar, float amount );
MYLLY_API void				mgui_scrollbar_get_track_colour	( MGuiScrollbar* scrollbar, colour_t* col );
MYLLY_API void				mgui_scrollbar_set_track_colour	( MGuiScrollbar* scrollbar, const colour_t* col );

/* Window functions */
MYLLY_API void				mgui_window_get_title_col		( MGuiWindow* window, colour_t* col );
MYLLY_API void				mgui_window_set_title_col		( MGuiWindow* window, const colour_t* col );
MYLLY_API uint32			mgui_window_get_title_col_i		( MGuiWindow* window );
MYLLY_API void				mgui_window_set_title_col_i		( MGuiWindow* window, uint32 hex );
MYLLY_API void				mgui_window_get_drag_offset		( MGuiWindow* window, vectorscreen_t* pos );

__END_DECLS

#endif /* __MYLLY_GUI_H */
