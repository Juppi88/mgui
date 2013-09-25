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

#ifdef _MYLLY_SHARED_LIB
#define MGUI_EXPORT MYLLY_EXPORT
#else
#define MGUI_EXPORT MYLLY_API
#endif

// Interface types
typedef struct MGuiElement	MGuiElement;
typedef struct MGuiRenderer	MGuiRenderer;

#define MGUI_ELEMENT_DECL(x) typedef MGuiElement x

// GUI elements
MGUI_ELEMENT_DECL( MGuiButton );
MGUI_ELEMENT_DECL( MGuiCanvas );
MGUI_ELEMENT_DECL( MGuiCheckbox );
MGUI_ELEMENT_DECL( MGuiEditbox );
MGUI_ELEMENT_DECL( MGuiLabel );
MGUI_ELEMENT_DECL( MGuiMemobox );
MGUI_ELEMENT_DECL( MGuiProgressBar );
MGUI_ELEMENT_DECL( MGuiScrollBar );
MGUI_ELEMENT_DECL( MGuiSprite );
MGUI_ELEMENT_DECL( MGuiWindow );

#define cast_elem(x) ((MGuiElement*)x)
#define cast_node(x) ((node_t*)x)

enum MGUI_PARAMETERS
{
	MGUI_NO_PARAMS		= 0x0,	/* Don't use any special parameters */
	MGUI_USE_DRAW_EVENT	= 0x1,	/* MGUI will refresh only when the user calls mgui_force_redraw */
	MGUI_PROCESS_INPUT	= 0x2,	/* Process input within the GUI library without external hooks */
	MGUI_HOOK_INPUT		= 0x4,	/* Hook window messages and process input within the GUI library */
};

enum MGUI_ALIGNMENT
{
	ALIGN_LEFT		= 1 << 0,
	ALIGN_RIGHT		= 1 << 1,
	ALIGN_TOP		= 1 << 2,
	ALIGN_BOTTOM	= 1 << 3,
	ALIGN_CENTERV	= 1 << 4,
	ALIGN_CENTERH	= 1 << 5,
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
	FLAG_TEXT_SHADOW		= 1 << 14,	/* Text will cast a shadow */
	FLAG_TEXT_TAGS			= 1 << 15,	/* Text can be formatted using tags */
	FLAG_DEPTH_TEST			= 1 << 16,	/* Enable depth testing (useful for 3D text) */
	FLAG_3D_ENTITY			= 1 << 17,	/* This element is fully 3D, use mgui_set_3d_* to manipulate */
	FLAG_CACHE_TEXTURE		= 1 << 18,	/* Use a cache texture */

	/* Checkbox */
	FLAG_CHECKBOX_CHECKED	= 1 << 24,	/* Checkbox is selected */

	/* Editbox */
	FLAG_EDITBOX_MASKINPUT	= 1 << 24,	/* Mask user input in editbox */

	/* Memobox */
	FLAG_MEMOBOX_TOPBOTTOM	= 1 << 24,	/* Memobox order is top to bottom */

	/* Window */
	FLAG_WINDOW_TITLEBAR	= 1 << 24,	/* Enable window titlebar */
	FLAG_WINDOW_CLOSEBTN	= 1 << 25,	/* Enable window close button */
	FLAG_WINDOW_RESIZABLE	= 1 << 26,	/* Window can be resized by the user */
};

enum MGUI_FONT_FLAGS
{
	FFLAG_NONE		= 0,		/* All flags disabled */
	FFLAG_BOLD		= 1 << 0,	/* Bold font */
	FFLAG_ITALIC	= 1 << 1,	/* Italic font */
	FFLAG_ULINE		= 1 << 2,	/* Underlined font */
	FFLAG_STRIKE	= 1 << 3,	/* Strike out */
	FFLAG_NOAA		= 1 << 4,	/* Disable edge smoothing (if possible) */
	FFLAG_OUTLINE	= 1 << 5,	/* Create outlined font */
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

// --------------------------------------------------
// Event system
// --------------------------------------------------

typedef enum {
	EVENT_HOVER_ENTER,		/* Mouse enters the element boundaries (MGuiMouseEvent) */
	EVENT_HOVER_LEAVE,		/* Mouse leaves the element boundaries (MGuiMouseEvent) */
	EVENT_CLICK,			/* Element is clicked (lmb down) (MGuiMouseEvent) */
	EVENT_RELEASE,			/* Element is released (lmb up) (MGuiMouseEvent) */
	EVENT_DRAG,				/* Element is dragged (MGuiMouseEvent) */
	EVENT_FOCUS_ENTER,		/* Element receives focus (MGuiAnyEvent) */
	EVENT_FOCUS_EXIT,		/* Element loses focus (MGuiAnyEvent) */
	EVENT_CHECKBOX_TOGGLE,	/* Checkbox is toggled (MGuiAnyEvent) */
	EVENT_INPUT_CHANGE,		/* User modifies the text of an input element such as an editbox (MGuiAnyEvent) */
	EVENT_INPUT_RETURN,		/* User presses return while an input element is focused (MGuiKeyEvent) */
	EVENT_WINDOW_CLOSE,		/* Window is closed from the close button (MGuiAnyEvent) */
	EVENT_WINDOW_RESIZE,	/* Window is resized by the user (MGuiResizeEvent) */
	EVENT_FORCE_DWORD = 0x7FFFFFFF
} MGUI_EVENT;

typedef struct {
	MGUI_EVENT		type;		// Event type
	MGuiElement*	element;	// The element which triggered this event
	void*			data;		// User specified data
} MGuiAnyEvent;

typedef struct {
	MGUI_EVENT		type;		// Event type
	MGuiElement*	element;	// The element which triggered this event
	void*			data;		// User specified data
	int16			cursor_x;	// Mouse cursor x co-ordinate
	int16			cursor_y;	// Mouse cursor y co-ordinate
} MGuiMouseEvent;

typedef struct {
	MGUI_EVENT		type;		// Event type
	MGuiElement*	element;	// The element which triggered this event
	void*			data;		// User specified data
	uint32			key;		// Active keyboard key
} MGuiKeyEvent;

typedef struct {
	MGUI_EVENT		type;		// Event type
	MGuiElement*	element;	// The element which triggered this event
	void*			data;		// User specified data
	uint16			width;		// New width of the element
	uint16			height;		// New height of the element
} MGuiResizeEvent;

typedef union {
	MGUI_EVENT		type;		// Type of the event, always the first member
	MGuiAnyEvent	any;		// Generic event without extra data
	MGuiKeyEvent	keyboard;	// Keyboard event
	MGuiMouseEvent	mouse;		// Mouse cursor event
	MGuiResizeEvent	resize;		// Window resize event
} MGuiEvent;

// GUI event hook type
typedef void ( *mgui_event_handler_t )( const MGuiEvent* event );


__BEGIN_DECLS

/* Library initialization and processing */
MGUI_EXPORT void			mgui_initialize					( void* wndhandle, uint32 params );
MGUI_EXPORT void			mgui_shutdown					( void );
MGUI_EXPORT void			mgui_pre_process				( void );
MGUI_EXPORT void			mgui_process					( void );
MGUI_EXPORT void			mgui_force_redraw				( void );

MGUI_EXPORT void			mgui_set_renderer				( MGuiRenderer* renderer );
MGUI_EXPORT void			mgui_set_skin					( const char_t* skinimg );

/* Misc functions */
MGUI_EXPORT MGuiElement*	mgui_get_focus					( void );
MGUI_EXPORT void			mgui_set_focus					( MGuiElement* element );
MGUI_EXPORT uint32			mgui_text_strip_format_tags		( const char_t* text, char_t* buf, size_t buflen );

MGUI_EXPORT void			mgui_screen_pos_to_world		( const vector3_t* src, vector3_t* dst );
MGUI_EXPORT void			mgui_world_pos_to_screen		( const vector3_t* src, vector3_t* dst );

/* Element relations */
MGUI_EXPORT void			mgui_add_child					( MGuiElement* parent, MGuiElement* child );
MGUI_EXPORT void			mgui_remove_child				( MGuiElement* child );
MGUI_EXPORT void			mgui_move_forward				( MGuiElement* child );
MGUI_EXPORT void			mgui_move_backward				( MGuiElement* child );
MGUI_EXPORT void			mgui_send_to_top				( MGuiElement* child );
MGUI_EXPORT void			mgui_send_to_bottom				( MGuiElement* child );
MGUI_EXPORT bool			mgui_is_child_of				( MGuiElement* parent, MGuiElement* child );

/* Element constructors */
MGUI_EXPORT MGuiButton*		mgui_create_button				( MGuiElement* parent );
MGUI_EXPORT MGuiButton*		mgui_create_button_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );
MGUI_EXPORT MGuiEditbox*	mgui_create_canvas				( MGuiElement* parent );
MGUI_EXPORT MGuiCheckbox*	mgui_create_checkbox			( MGuiElement* parent );
MGUI_EXPORT MGuiCheckbox*	mgui_create_checkbox_ex			( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col );
MGUI_EXPORT MGuiEditbox*	mgui_create_editbox				( MGuiElement* parent );
MGUI_EXPORT MGuiEditbox*	mgui_create_editbox_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );
MGUI_EXPORT MGuiLabel*		mgui_create_label				( MGuiElement* parent );
MGUI_EXPORT MGuiLabel*		mgui_create_label_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );
MGUI_EXPORT MGuiMemobox*	mgui_create_memobox				( MGuiElement* parent );
MGUI_EXPORT MGuiMemobox*	mgui_create_memobox_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col );
MGUI_EXPORT MGuiProgressBar* mgui_create_progressbar		( MGuiElement* parent );
MGUI_EXPORT MGuiProgressBar* mgui_create_progressbar_ex		( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col1, uint32 col2, float max_value );
MGUI_EXPORT MGuiScrollBar*	mgui_create_scrollbar			( MGuiElement* parent );
MGUI_EXPORT MGuiSprite*		mgui_create_sprite				( MGuiElement* parent );
MGUI_EXPORT MGuiSprite*		mgui_create_sprite_ex			( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col, const char_t* texture );
MGUI_EXPORT MGuiWindow*		mgui_create_window				( MGuiElement* parent );
MGUI_EXPORT MGuiWindow*		mgui_create_window_ex			( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );

MGUI_EXPORT void			mgui_element_destroy			( MGuiElement* element );

/* Generic element manipulation */
MGUI_EXPORT void			mgui_get_pos					( MGuiElement* element, vector2_t* pos );
MGUI_EXPORT void			mgui_get_size					( MGuiElement* element, vector2_t* size );
MGUI_EXPORT void			mgui_set_pos					( MGuiElement* element, const vector2_t* pos );
MGUI_EXPORT void			mgui_set_size					( MGuiElement* element, const vector2_t* size );
MGUI_EXPORT void			mgui_get_abs_pos				( MGuiElement* element, vectorscreen_t* pos );
MGUI_EXPORT void			mgui_get_abs_size				( MGuiElement* element, vectorscreen_t* size );
MGUI_EXPORT void			mgui_set_abs_pos				( MGuiElement* element, const vectorscreen_t* pos );
MGUI_EXPORT void			mgui_set_abs_size				( MGuiElement* element, const vectorscreen_t* size );
MGUI_EXPORT void			mgui_get_pos_f					( MGuiElement* element, float* x, float* y );
MGUI_EXPORT void			mgui_get_size_f					( MGuiElement* element, float* w, float* h );
MGUI_EXPORT void			mgui_set_pos_f					( MGuiElement* element, float x, float y );
MGUI_EXPORT void			mgui_set_size_f					( MGuiElement* element, float w, float h );
MGUI_EXPORT void			mgui_get_abs_pos_i				( MGuiElement* element, int16* x, int16* y );
MGUI_EXPORT void			mgui_get_abs_size_i				( MGuiElement* element, uint16* w, uint16* h );
MGUI_EXPORT void			mgui_set_abs_pos_i				( MGuiElement* element, int16 x, int16 y );
MGUI_EXPORT void			mgui_set_abs_size_i				( MGuiElement* element, uint16 w, uint16 h );

MGUI_EXPORT float			mgui_get_z_depth				( MGuiElement* element );
MGUI_EXPORT void			mgui_set_z_depth				( MGuiElement* element, float depth );

MGUI_EXPORT void			mgui_get_3d_position			( MGuiElement* element, vector3_t* pos );
MGUI_EXPORT void			mgui_set_3d_position			( MGuiElement* element, const vector3_t* pos );
MGUI_EXPORT void			mgui_get_3d_rotation			( MGuiElement* element, vector3_t* rot );
MGUI_EXPORT void			mgui_set_3d_rotation			( MGuiElement* element, const vector3_t* rot );
MGUI_EXPORT void			mgui_get_3d_size				( MGuiElement* element, vector2_t* size );
MGUI_EXPORT void			mgui_set_3d_size				( MGuiElement* element, const vector2_t* size );
MGUI_EXPORT void			mgui_set_3d_transform			( MGuiElement* element, const vector3_t* pos, const vector3_t* rot, const vector2_t* size );

MGUI_EXPORT void			mgui_get_colour					( MGuiElement* element, colour_t* col );
MGUI_EXPORT void			mgui_set_colour					( MGuiElement* element, const colour_t* col );
MGUI_EXPORT void			mgui_get_text_colour			( MGuiElement* element, colour_t* col );
MGUI_EXPORT void			mgui_set_text_colour			( MGuiElement* element, const colour_t* col );
MGUI_EXPORT uint32			mgui_get_colour_i				( MGuiElement* element );
MGUI_EXPORT void			mgui_set_colour_i				( MGuiElement* element, uint32 hex );
MGUI_EXPORT uint32			mgui_get_text_colour_i			( MGuiElement* element );
MGUI_EXPORT void			mgui_set_text_colour_i			( MGuiElement* element, uint32 hex );
MGUI_EXPORT uint8			mgui_get_alpha					( MGuiElement* element );
MGUI_EXPORT void			mgui_set_alpha					( MGuiElement* element, uint8 alpha );

MGUI_EXPORT const char_t*	mgui_get_text					( MGuiElement* element );
MGUI_EXPORT char_t*			mgui_get_text_buffer			( MGuiElement* element, char_t* buf, size_t buflen );
MGUI_EXPORT uint32			mgui_get_text_len				( MGuiElement* element );
MGUI_EXPORT void			mgui_set_text					( MGuiElement* element, const char_t* fmt, ... );
MGUI_EXPORT void			mgui_set_text_s					( MGuiElement* element, const char_t* text );
MGUI_EXPORT void			mgui_get_text_size				( MGuiElement* element, vectorscreen_t* size );
MGUI_EXPORT void			mgui_get_text_size_i			( MGuiElement* element, uint16* w, uint16* h );
MGUI_EXPORT uint32			mgui_get_alignment				( MGuiElement* element );
MGUI_EXPORT void			mgui_set_alignment				( MGuiElement* element, uint32 alignment );
MGUI_EXPORT void			mgui_get_text_padding			( MGuiElement* element, uint8* top, uint8* bottom, uint8* left, uint8* right );
MGUI_EXPORT void			mgui_set_text_padding			( MGuiElement* element, uint8 top, uint8 bottom, uint8 left, uint8 right );

MGUI_EXPORT const char_t*	mgui_get_font_name				( MGuiElement* element );
MGUI_EXPORT uint8			mgui_get_font_size				( MGuiElement* element );
MGUI_EXPORT uint8			mgui_get_font_flags				( MGuiElement* element );
MGUI_EXPORT void			mgui_set_font_name				( MGuiElement* element, const char_t* font );
MGUI_EXPORT void			mgui_set_font_size				( MGuiElement* element, uint8 size );
MGUI_EXPORT void			mgui_set_font_flags				( MGuiElement* element, uint8 flags );
MGUI_EXPORT void			mgui_set_font					( MGuiElement* element, const char_t* font, uint8 size, uint8 flags, uint8 charset );

MGUI_EXPORT uint32			mgui_get_flags					( MGuiElement* element );
MGUI_EXPORT void			mgui_add_flags					( MGuiElement* element, uint32 flags );
MGUI_EXPORT void			mgui_remove_flags				( MGuiElement* element, uint32 flags );

MGUI_EXPORT void			mgui_set_event_handler			( MGuiElement* element, mgui_event_handler_t handler, void* data );

/* Editbox functions */
MGUI_EXPORT bool			mgui_editbox_has_text_selected	( MGuiEditbox* editbox );
MGUI_EXPORT void			mgui_editbox_get_selection		( MGuiEditbox* editbox, char_t* buf, size_t buflen );
MGUI_EXPORT void			mgui_editbox_select_text		( MGuiEditbox* editbox, uint32 begin, uint32 end );
MGUI_EXPORT uint32			mgui_editbox_get_cursor_pos		( MGuiEditbox* editbox );
MGUI_EXPORT void			mgui_editbox_set_cursor_pos		( MGuiEditbox* editbox, uint32 pos );

/* Label functions */
MGUI_EXPORT void			mgui_label_make_text_fit		( MGuiLabel* label );

/* Memobox functions */
MGUI_EXPORT void			mgui_memobox_add_line			( MGuiMemobox* memobox, const char* fmt, ... );
MGUI_EXPORT void			mgui_memobox_add_line_col		( MGuiMemobox* memobox, const char* fmt, const colour_t* col, ... );
MGUI_EXPORT void			mgui_memobox_add_line_s			( MGuiMemobox* memobox, const char* text );
MGUI_EXPORT void			mgui_memobox_add_line_col_s		( MGuiMemobox* memobox, const char* text, const colour_t* col );
MGUI_EXPORT void			mgui_memobox_clear				( MGuiMemobox* memobox );
MGUI_EXPORT float			mgui_memobox_get_display_pos	( MGuiMemobox* memobox );
MGUI_EXPORT void			mgui_memobox_set_display_pos	( MGuiMemobox* memobox, float pos );
MGUI_EXPORT uint32			mgui_memobox_get_lines			( MGuiMemobox* memobox );
MGUI_EXPORT void			mgui_memobox_set_lines			( MGuiMemobox* memobox, uint32 lines );
MGUI_EXPORT uint32			mgui_memobox_get_num_lines		( MGuiMemobox* memobox );
MGUI_EXPORT uint32			mgui_memobox_get_history		( MGuiMemobox* memobox );
MGUI_EXPORT void			mgui_memobox_set_history		( MGuiMemobox* memobox, uint32 lines );
MGUI_EXPORT uint32			mgui_memobox_get_margin			( MGuiMemobox* memobox );
MGUI_EXPORT void			mgui_memobox_set_margin			( MGuiMemobox* memobox, uint32 margin );

/* Progressbar functions */
MGUI_EXPORT float			mgui_progressbar_get_value		( MGuiProgressBar* bar );
MGUI_EXPORT void			mgui_progressbar_set_value		( MGuiProgressBar* bar, float value );
MGUI_EXPORT float			mgui_progressbar_get_max_value	( MGuiProgressBar* bar );
MGUI_EXPORT void			mgui_progressbar_set_max_value	( MGuiProgressBar* bar, float value );
MGUI_EXPORT void			mgui_progressbar_get_colour		( MGuiProgressBar* bar, colour_t* col_start, colour_t* col_end );
MGUI_EXPORT void			mgui_progressbar_set_colour		( MGuiProgressBar* bar, const colour_t* col_start, const colour_t* col_end );
MGUI_EXPORT void			mgui_progressbar_get_colour_i	( MGuiProgressBar* bar, uint32* col_start, uint32* col_end );
MGUI_EXPORT void			mgui_progressbar_set_colour_i	( MGuiProgressBar* bar, uint32 col_start, uint32 col_end );
MGUI_EXPORT float			mgui_progressbar_get_bg_shade	( MGuiProgressBar* bar );
MGUI_EXPORT void			mgui_progressbar_set_bg_shade	( MGuiProgressBar* bar, float shade );
MGUI_EXPORT uint8			mgui_progressbar_get_thickness	( MGuiProgressBar* bar );
MGUI_EXPORT void			mgui_progressbar_set_thickness	( MGuiProgressBar* bar, uint8 thickness );

/* Scrollbar functions */
MGUI_EXPORT float			mgui_scrollbar_get_bar_pos		( MGuiScrollBar* scrollbar );
MGUI_EXPORT void			mgui_scrollbar_set_bar_pos		( MGuiScrollBar* scrollbar, float pos );
MGUI_EXPORT float			mgui_scrollbar_get_bar_size		( MGuiScrollBar* scrollbar );
MGUI_EXPORT void			mgui_scrollbar_set_bar_size		( MGuiScrollBar* scrollbar, float size );
MGUI_EXPORT float			mgui_scrollbar_get_nudge		( MGuiScrollBar* scrollbar );
MGUI_EXPORT void			mgui_scrollbar_set_nudge		( MGuiScrollBar* scrollbar, float amount );
MGUI_EXPORT void			mgui_scrollbar_get_track_colour	( MGuiScrollBar* scrollbar, colour_t* col );
MGUI_EXPORT void			mgui_scrollbar_set_track_colour	( MGuiScrollBar* scrollbar, const colour_t* col );

/* Sprite functions */
MGUI_EXPORT const char_t*	mgui_sprite_get_texture			( MGuiSprite* sprite );
MGUI_EXPORT void			mgui_sprite_set_texture			( MGuiSprite* sprite, const char_t* texture );
MGUI_EXPORT void			mgui_sprite_get_texture_size	( MGuiSprite* sprite, vectorscreen_t* size );
MGUI_EXPORT void			mgui_sprite_get_texture_size_i	( MGuiSprite* sprite, uint16* width, uint16* height );
MGUI_EXPORT void			mgui_sprite_resize				( MGuiSprite* sprite );
MGUI_EXPORT void			mgui_sprite_get_uv				( MGuiSprite* sprite, float* u1, float* v1, float* u2, float* v2 );
MGUI_EXPORT void			mgui_sprite_set_uv				( MGuiSprite* sprite, float u1, float v1, float u2, float v2 );

/* Window functions */
MGUI_EXPORT void			mgui_window_get_title_col		( MGuiWindow* window, colour_t* col );
MGUI_EXPORT void			mgui_window_set_title_col		( MGuiWindow* window, const colour_t* col );
MGUI_EXPORT uint32			mgui_window_get_title_col_i		( MGuiWindow* window );
MGUI_EXPORT void			mgui_window_set_title_col_i		( MGuiWindow* window, uint32 hex );
MGUI_EXPORT void			mgui_window_get_drag_offset		( MGuiWindow* window, vectorscreen_t* pos );

__END_DECLS

#endif /* __MYLLY_GUI_H */
