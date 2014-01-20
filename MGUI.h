/**
 *
 * @file		MGUI.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		MGUI main header.
 *
 * @details		The main header containing the public API for MGUI.
 *
 **/

#pragma once
#ifndef __MGUI_H
#define __MGUI_H

#include "stdtypes.h"
#include "Math/MathDefs.h"

#ifdef MYLLY_SHARED_LIB
#define MGUI_EXPORT MYLLY_EXPORT
#else
#define MGUI_EXPORT MYLLY_API
#endif

// Interface types
typedef struct MGuiElement		MGuiElement;
typedef struct MGuiRenderer		MGuiRenderer;
typedef struct MGuiListboxItem	MGuiListboxItem;

#define MGUI_ELEMENT_DECL(x) typedef MGuiElement x

// GUI elements
MGUI_ELEMENT_DECL( MGuiButton );
MGUI_ELEMENT_DECL( MGuiCanvas );
MGUI_ELEMENT_DECL( MGuiCheckbox );
MGUI_ELEMENT_DECL( MGuiEditbox );
MGUI_ELEMENT_DECL( MGuiLabel );
MGUI_ELEMENT_DECL( MGuiListbox );
MGUI_ELEMENT_DECL( MGuiMemobox );
MGUI_ELEMENT_DECL( MGuiProgressbar );
MGUI_ELEMENT_DECL( MGuiScrollbar );
MGUI_ELEMENT_DECL( MGuiSprite );
MGUI_ELEMENT_DECL( MGuiWindow );

#define cast_elem(x) ((MGuiElement*)x)
#define cast_node(x) ((node_t*)x)

/**
 * @brief Mylly GUI initialization parameters.
 * @sa mgui_initialize
 */
enum MGUI_PARAMETERS {
	MGUI_NO_PARAMS		= 0x0,	///< Don't use any special parameters
	MGUI_USE_DRAW_EVENT	= 0x1,	///< MGUI will refresh the screen only when there is something to draw
	MGUI_PROCESS_INPUT	= 0x2,	///< Listen to window messages within MGUI (uses Lib-Input)
	MGUI_HOOK_INPUT		= 0x4,	///< Hook window messages and process input within the GUI library (uses Lib-Input)
};

/**
 * @brief Specifies different values for text alignment.
 *
 * @details The contents of this enum are different values that can be
 * used to align text within an element. These can also be used as bitmasks.
 *
 * @sa mgui_get_alignment, mgui_set_alignment
 */
enum MGUI_ALIGNMENT {
	ALIGN_LEFT		= 1 << 0,	///< Align text to left
	ALIGN_RIGHT		= 1 << 1,	///< Align text to right
	ALIGN_TOP		= 1 << 2,	///< Align text to the top of the element
	ALIGN_BOTTOM	= 1 << 3,	///< Align text to the bottom of the element
	ALIGN_CENTERV	= 1 << 4,	///< Align text to center vertically
	ALIGN_CENTERH	= 1 << 5,	///< Align text to center horizontally
	ALIGN_CENTER	= (ALIGN_CENTERV|ALIGN_CENTERH), ///< Align text to center
};

/**
 * @enum MGUI_FLAGS
 *
 * @brief Element property flags.
 * @details These are different element properties that can be turned on or off.
 * Depending on the element type some of these might be enabled by default.
 */
enum MGUI_FLAGS {
	FLAG_NONE				= 0,		///< All flags disabled
	FLAG_VISIBLE			= 1 << 0,	///< This element is visible and it will be processed
	FLAG_DISABLED			= 1 << 1,	///< This element is inactive (user can't interact with it)
	FLAG_BACKGROUND			= 1 << 2,	///< The element has a visible background
	FLAG_BORDER				= 1 << 3,	///< The element has a visible border
	FLAG_SHADOW				= 1 << 4,	///< The element casts a visible shadow/
	FLAG_DRAGGABLE			= 1 << 5,	///< This element can be dragged by the user
	FLAG_CLIP				= 1 << 6,	///< Clip text within the element if it exceeds its boundaries
	FLAG_WRAP				= 1 << 7,	///< Wrap text if it exceeds boundaries of the element (if applicable)
	FLAG_AUTO_RESIZE		= 1 << 8,	///< Resize and reposition this element automatically if the parent size changes
	FLAG_INHERIT_ALPHA		= 1 << 9,	///< Element will inherit alpha from its parent element
	FLAG_ANIMATION			= 1 << 10,	///< Enable animations (if applicable)
	FLAG_TABSTOP			= 1 << 11,	///< Tab press can switch focus to this element
	FLAG_MOUSECTRL			= 1 << 12,	///< Element triggers mouse input events
	FLAG_KBCTRL				= 1 << 13,	///< Element triggers keyboard input events and can accept keyboard focus
	FLAG_SCROLLABLE			= 1 << 14,	///< Element will create a scrollbar if the content size exceeds boundaries
	FLAG_TEXT_SHADOW		= 1 << 15,	///< The text within the element will cast a shadow
	FLAG_TEXT_TAGS			= 1 << 16,	///< Text can be formatted using certain format tags
	FLAG_DEPTH_TEST			= 1 << 17,	///< Enable depth testing (useful for 3D text)
	FLAG_3D_ENTITY			= 1 << 18,	///< This element is drawn into 3D space, use mgui_set_3d_* to transform
	FLAG_CACHE_TEXTURE		= 1 << 19,	///< Cache this element into a texture if possible

	FLAG_CHECKBOX_CHECKED	= 1 << 24,	///< (Checkbox) Checkbox is selected (toggle)
	FLAG_EDITBOX_MASKINPUT	= 1 << 24,	///< (Editbox) Mask user's input in the editbox
	FLAG_LISTBOX_MULTISELECT =1 << 24,	///< (Listbox) Listbox allows multiple rows to be selected at once
	FLAG_LISTBOX_SORTING	= 1 << 25,	///< (Listbox) Listbox uses automatic sorting
	FLAG_MEMOBOX_TOPBOTTOM	= 1 << 24,	///< (Memobox) Memobox outputs lines from top to bottom
	FLAG_SCROLLBAR_HORIZ	= 1 << 24,	///< (Scrollbar) Scrollbar is horizontal (instead of vertical)
	FLAG_WINDOW_TITLEBAR	= 1 << 24,	///< (Window) Enables window titlebar
	FLAG_WINDOW_CLOSEBTN	= 1 << 25,	///< (Window) Enables window close button
	FLAG_WINDOW_RESIZABLE	= 1 << 26,	///< (Window) Window can be resized by the user
};

/**
 * @enum MGUI_FONT_FLAGS
 *
 * @brief Font style flags.
 * @details These are different font styles that can be turned on or off.
 * The values of this enum can be used as a bitmask.
 */
enum MGUI_FONT_FLAGS {
	FFLAG_NONE		= 0,		///< All flags disabled
	FFLAG_BOLD		= 1 << 0,	///< Bold font
	FFLAG_ITALIC	= 1 << 1,	///< Italic (cursive) font
	FFLAG_ULINE		= 1 << 2,	///< Underlined font
	FFLAG_STRIKE	= 1 << 3,	///< Strike out
	FFLAG_NOAA		= 1 << 4,	///< Disable edge smoothing (if possible)
	FFLAG_OUTLINE	= 1 << 5,	///< Create an outlined font
};

/**
 * @enum MGUI_FONT_CHARSET
 *
 * @brief Font character sets.
 * @details These values can be used to set the character set of non-unicode strings on Windows.
 */
enum MGUI_FONT_CHARSET {
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

/**
 * @brief Element event types.
 *
 * @details These are the different event types than an element can
 * trigger. The type of the event will always be passed on as the
 * first member of @ref MGuiEvent.
 *
 * @sa MGuiEvent, mgui_set_event_handler
 */
typedef enum {
	EVENT_HOVER_ENTER,		///< Mouse enters the boundaries of an element (@ref MGuiMouseEvent)
	EVENT_HOVER_LEAVE,		///< Mouse leaves the boundaries of an element (@ref MGuiMouseEvent)
	EVENT_CLICK,			///< Element is clicked (left mouse button down) (@ref MGuiMouseEvent)
	EVENT_RELEASE,			///< Element is released (left mouse button up) (@ref MGuiMouseEvent)
	EVENT_DRAG,				///< Element is dragged (@ref MGuiMouseEvent)
	EVENT_FOCUS_ENTER,		///< Element receives keyboard focus (@ref MGuiAnyEvent)
	EVENT_FOCUS_EXIT,		///< Element loses keyboard focus (@ref MGuiAnyEvent)
	EVENT_CHECKBOX_TOGGLE,	///< Checkbox is toggled (@ref MGuiAnyEvent)
	EVENT_INPUT_RETURN,		///< User presses return while an input element is focused (@ref MGuiKeyEvent)
	EVENT_LISTBOX_SELECT,	///< A listbox item was selected (@ref MGuiListEvent)
	EVENT_SCROLL,			///< Scrollable element was scrolled (@ref MGuiScrollEvent)
	EVENT_WINDOW_CLOSE,		///< Window is closed using the close button (@ref MGuiAnyEvent)
	EVENT_WINDOW_RESIZE,	///< Window is resized by the user (@ref MGuiResizeEvent)
	EVENT_FORCE_DWORD = 0x7FFFFFFF
} MGUI_EVENT;

/**
 * @brief Generic event data.
 * @sa MGuiEvent
 */
typedef struct {
	MGUI_EVENT		type;		///< Type of the event (@ref MGUI_EVENT)
	MGuiElement*	element;	///< The element which triggered this event
	void*			data;		///< User specified data
} MGuiAnyEvent;

/**
 * @brief Keyboard event data.
 * @sa MGuiEvent
 */
typedef struct {
	MGUI_EVENT		type;		///< Type of the event (@ref MGUI_EVENT)
	MGuiElement*	element;	///< The element which triggered this event
	void*			data;		///< User specified data
	uint32			key;		///< Active keyboard key
} MGuiKeyEvent;

/**
 * @brief Mouse event data.
 * @sa MGuiEvent
 */
typedef struct {
	MGUI_EVENT		type;		///< Type of the event (@ref MGUI_EVENT)
	MGuiElement*	element;	///< The element which triggered this event
	void*			data;		///< User specified data
	int16			cursor_x;	///< Mouse cursor x co-ordinate
	int16			cursor_y;	///< Mouse cursor y co-ordinate
} MGuiMouseEvent;

/**
 * @brief Listbox select event data.
 * @sa MGuiEvent
 */
typedef struct {
	MGUI_EVENT		type;		///< Type of the event (@ref MGUI_EVENT)
	MGuiElement*	element;	///< The element which triggered this event
	void*			data;		///< User specified data
	MGuiListboxItem* item;		///< Selected listbox item
} MGuiListEvent;

/**
 * @brief Window resize event data.
 * @sa MGuiEvent
 */
typedef struct {
	MGUI_EVENT		type;		///< Type of the event (@ref MGUI_EVENT)
	MGuiElement*	element;	///< The element which triggered this event
	void*			data;		///< User specified data
	uint16			width;		///< New width of the element
	uint16			height;		///< New height of the element
} MGuiResizeEvent;

/**
 * @brief Scrollable element event data.
 * @sa MGuiEvent
 */
typedef struct {
	MGUI_EVENT		type;		///< Type of the event (@ref MGUI_EVENT)
	MGuiElement*	element;	///< The element which triggered this event
	void*			data;		///< User specified data
	float			position;	///< Position after scrolling
	float			change;		///< Change in position
} MGuiScrollEvent;

/**
 * @brief Element event parameters.
 *
 * @details This struct contains data from an event triggered by an element.
 * The first member of the struct will always be the type of the event (see
 * @ref MGUI_EVENT). The structure is a union of smaller structs that contain
 * data specific to different event types (such as data from the keyboard).
 *
 * @sa MGUI_EVENT, mgui_set_event_handler
 */
typedef union {
	MGUI_EVENT		type;		///< Type of the event, always the first member
	MGuiAnyEvent	any;		///< Generic event data that applies to any event
	MGuiKeyEvent	keyboard;	///< Keyboard event data
	MGuiMouseEvent	mouse;		///< Mouse cursor event data
	MGuiListEvent	list;		///< Listbox selection event
	MGuiResizeEvent	resize;		///< Window resize event
	MGuiScrollEvent	scroll;		///< Scrollable element event
} MGuiEvent;

/**
 * @brief GUI element event handler.
 *
 * @details This is the prototype for a function that handles
 * element events.
 *
 * @param event A pointer to a structure that contains the event data
 * @sa mgui_set_event_handler
 */
typedef void ( *mgui_event_handler_t )( const MGuiEvent* event );

/**
 * @brief Listbox item comparison function.
 *
 * @details This is the prototype for a function that compares
 * two listbox items.
 *
 * @param item1 First listbox item
 * @param item2 Second listbox item
 * @returns -1 if item1 < item2, 1 if item1 > item2, 0 if item1 = item2

 * @sa mgui_listbox_set_sort_function
 */
typedef int ( *mgui_listbox_sort_t )( const MGuiListboxItem* item1, const MGuiListboxItem* item2 );


__BEGIN_DECLS

/**
 * @defgroup mgui Core functions
 * @{
 */
MGUI_EXPORT void	mgui_initialize				( void* wndhandle, uint32 params );
MGUI_EXPORT void	mgui_shutdown				( void );
MGUI_EXPORT void	mgui_pre_process			( void );
MGUI_EXPORT void	mgui_process				( void );
MGUI_EXPORT void	mgui_force_redraw			( void );
MGUI_EXPORT void	mgui_resize					( uint16 width, uint16 height );
MGUI_EXPORT void	mgui_set_renderer			( MGuiRenderer* renderer );
MGUI_EXPORT void	mgui_set_skin				( const char_t* skinimg );

MGUI_EXPORT MGuiElement* mgui_get_focus				( void );
MGUI_EXPORT void	mgui_set_focus				( MGuiElement* element );
MGUI_EXPORT uint32	mgui_text_strip_format_tags	( const char_t* text, char_t* buf, size_t buflen );
MGUI_EXPORT void	mgui_screen_pos_to_world	( const vector3_t* src, vector3_t* dst );
MGUI_EXPORT void	mgui_world_pos_to_screen	( const vector3_t* src, vector3_t* dst );

/**
 * @}
 * @defgroup element-constructors Element constructors
 * @{
 * @details Functions to create and destroy different element types.
 * Includes extended versions for most constructors.
 */
MGUI_EXPORT MGuiButton*		mgui_create_button			( MGuiElement* parent );
MGUI_EXPORT MGuiButton*		mgui_create_button_ex		( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );
MGUI_EXPORT MGuiCanvas*		mgui_create_canvas			( MGuiElement* parent );
MGUI_EXPORT MGuiCheckbox*	mgui_create_checkbox		( MGuiElement* parent );
MGUI_EXPORT MGuiCheckbox*	mgui_create_checkbox_ex		( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col );
MGUI_EXPORT MGuiEditbox*	mgui_create_editbox			( MGuiElement* parent );
MGUI_EXPORT MGuiEditbox*	mgui_create_editbox_ex		( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );
MGUI_EXPORT MGuiLabel*		mgui_create_label			( MGuiElement* parent );
MGUI_EXPORT MGuiLabel*		mgui_create_label_ex		( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col, const char_t* text );
MGUI_EXPORT MGuiListbox*	mgui_create_listbox			( MGuiElement* parent );
MGUI_EXPORT MGuiListbox*	mgui_create_listbox_ex		( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, uint32 select_col );
MGUI_EXPORT MGuiMemobox*	mgui_create_memobox			( MGuiElement* parent );
MGUI_EXPORT MGuiMemobox*	mgui_create_memobox_ex		( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col );
MGUI_EXPORT MGuiProgressbar* mgui_create_progressbar	( MGuiElement* parent );
MGUI_EXPORT MGuiProgressbar* mgui_create_progressbar_ex	( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col1, uint32 col2, float max_value );
MGUI_EXPORT MGuiScrollbar*	mgui_create_scrollbar		( MGuiElement* parent );
MGUI_EXPORT MGuiScrollbar*	mgui_create_scrollbar_ex	( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col );
MGUI_EXPORT MGuiSprite*		mgui_create_sprite			( MGuiElement* parent );
MGUI_EXPORT MGuiSprite*		mgui_create_sprite_ex		( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col, const char_t* texture );
MGUI_EXPORT MGuiWindow*		mgui_create_window			( MGuiElement* parent );
MGUI_EXPORT MGuiWindow*		mgui_create_window_ex		( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );

MGUI_EXPORT void			mgui_element_destroy		( MGuiElement* element );

/**
 * @}
 * @defgroup element-relations Element relations
 * @{
 * @details Functions that can be used to specify or change the relations
 * between elements. If an element has no parent, it is called a layer.
 */
MGUI_EXPORT void	mgui_add_child		( MGuiElement* parent, MGuiElement* child );
MGUI_EXPORT void	mgui_remove_child	( MGuiElement* child );
MGUI_EXPORT void	mgui_move_forward	( MGuiElement* child );
MGUI_EXPORT void	mgui_move_backward	( MGuiElement* child );
MGUI_EXPORT void	mgui_send_to_top	( MGuiElement* child );
MGUI_EXPORT void	mgui_send_to_bottom	( MGuiElement* child );
MGUI_EXPORT bool	mgui_is_child_of	( MGuiElement* parent, MGuiElement* child );

/**
 * @}
 * @defgroup element Element functions
 * @{
 * @details Functions to get or set generic element properties. These can be used on all
 * MGUI element types (although certain element types might choose to ignore the call).
 */
MGUI_EXPORT void	mgui_get_pos			( MGuiElement* element, vector2_t* pos );
MGUI_EXPORT void	mgui_get_size			( MGuiElement* element, vector2_t* size );
MGUI_EXPORT void	mgui_set_pos			( MGuiElement* element, const vector2_t* pos );
MGUI_EXPORT void	mgui_set_size			( MGuiElement* element, const vector2_t* size );
MGUI_EXPORT void	mgui_get_abs_pos		( MGuiElement* element, vectorscreen_t* pos );
MGUI_EXPORT void	mgui_get_abs_size		( MGuiElement* element, vectorscreen_t* size );
MGUI_EXPORT void	mgui_set_abs_pos		( MGuiElement* element, const vectorscreen_t* pos );
MGUI_EXPORT void	mgui_set_abs_size		( MGuiElement* element, const vectorscreen_t* size );
MGUI_EXPORT void	mgui_get_pos_f			( MGuiElement* element, float* x, float* y );
MGUI_EXPORT void	mgui_get_size_f			( MGuiElement* element, float* w, float* h );
MGUI_EXPORT void	mgui_set_pos_f			( MGuiElement* element, float x, float y );
MGUI_EXPORT void	mgui_set_size_f			( MGuiElement* element, float w, float h );
MGUI_EXPORT void	mgui_get_abs_pos_i		( MGuiElement* element, int16* x, int16* y );
MGUI_EXPORT void	mgui_get_abs_size_i		( MGuiElement* element, uint16* w, uint16* h );
MGUI_EXPORT void	mgui_set_abs_pos_i		( MGuiElement* element, int16 x, int16 y );
MGUI_EXPORT void	mgui_set_abs_size_i		( MGuiElement* element, uint16 w, uint16 h );

MGUI_EXPORT float	mgui_get_z_depth		( MGuiElement* element );
MGUI_EXPORT void	mgui_set_z_depth		( MGuiElement* element, float depth );
MGUI_EXPORT void	mgui_get_3d_position	( MGuiElement* element, vector3_t* pos );
MGUI_EXPORT void	mgui_set_3d_position	( MGuiElement* element, const vector3_t* pos );
MGUI_EXPORT void	mgui_get_3d_rotation	( MGuiElement* element, vector3_t* rot );
MGUI_EXPORT void	mgui_set_3d_rotation	( MGuiElement* element, const vector3_t* rot );
MGUI_EXPORT void	mgui_get_3d_size		( MGuiElement* element, vector2_t* size );
MGUI_EXPORT void	mgui_set_3d_size		( MGuiElement* element, const vector2_t* size );
MGUI_EXPORT void	mgui_set_3d_transform	( MGuiElement* element, const vector3_t* pos, const vector3_t* rot, const vector2_t* size );

MGUI_EXPORT void	mgui_get_colour			( MGuiElement* element, colour_t* col );
MGUI_EXPORT void	mgui_set_colour			( MGuiElement* element, const colour_t* col );
MGUI_EXPORT void	mgui_get_text_colour	( MGuiElement* element, colour_t* col );
MGUI_EXPORT void	mgui_set_text_colour	( MGuiElement* element, const colour_t* col );
MGUI_EXPORT uint32	mgui_get_colour_i		( MGuiElement* element );
MGUI_EXPORT void	mgui_set_colour_i		( MGuiElement* element, uint32 hex );
MGUI_EXPORT uint32	mgui_get_text_colour_i	( MGuiElement* element );
MGUI_EXPORT void	mgui_set_text_colour_i	( MGuiElement* element, uint32 hex );
MGUI_EXPORT uint8	mgui_get_alpha			( MGuiElement* element );
MGUI_EXPORT void	mgui_set_alpha			( MGuiElement* element, uint8 alpha );

MGUI_EXPORT const char_t* mgui_get_text		( MGuiElement* element );
MGUI_EXPORT char_t*	mgui_get_text_buffer	( MGuiElement* element, char_t* buf, size_t buflen );
MGUI_EXPORT uint32	mgui_get_text_len		( MGuiElement* element );
MGUI_EXPORT void	mgui_set_text			( MGuiElement* element, const char_t* fmt, ... );
MGUI_EXPORT void	mgui_set_text_s			( MGuiElement* element, const char_t* text );
MGUI_EXPORT void	mgui_get_text_size		( MGuiElement* element, vectorscreen_t* size );
MGUI_EXPORT void	mgui_get_text_size_i	( MGuiElement* element, uint16* w, uint16* h );
MGUI_EXPORT uint32	mgui_get_alignment		( MGuiElement* element );
MGUI_EXPORT void	mgui_set_alignment		( MGuiElement* element, uint32 alignment );
MGUI_EXPORT void	mgui_get_text_padding	( MGuiElement* element, uint8* top, uint8* bottom, uint8* left, uint8* right );
MGUI_EXPORT void	mgui_set_text_padding	( MGuiElement* element, uint8 top, uint8 bottom, uint8 left, uint8 right );

MGUI_EXPORT const char_t* mgui_get_font_name( MGuiElement* element );
MGUI_EXPORT uint8	mgui_get_font_size		( MGuiElement* element );
MGUI_EXPORT uint8	mgui_get_font_flags		( MGuiElement* element );
MGUI_EXPORT void	mgui_set_font_name		( MGuiElement* element, const char_t* font );
MGUI_EXPORT void	mgui_set_font_size		( MGuiElement* element, uint8 size );
MGUI_EXPORT void	mgui_set_font_flags		( MGuiElement* element, uint8 flags );
MGUI_EXPORT void	mgui_set_font			( MGuiElement* element, const char_t* font, uint8 size, uint8 flags, uint8 charset );

MGUI_EXPORT uint32	mgui_get_flags			( MGuiElement* element );
MGUI_EXPORT void	mgui_add_flags			( MGuiElement* element, uint32 flags );
MGUI_EXPORT void	mgui_remove_flags		( MGuiElement* element, uint32 flags );

MGUI_EXPORT void	mgui_set_event_handler	( MGuiElement* element, mgui_event_handler_t handler, void* data );

/** @}
 *  @defgroup editbox Editbox functions
 *  @{
 *  @ingroup element
 */
MGUI_EXPORT bool	mgui_editbox_has_text_selected	( MGuiEditbox* editbox );
MGUI_EXPORT void	mgui_editbox_get_selection		( MGuiEditbox* editbox, char_t* buf, size_t buflen );
MGUI_EXPORT void	mgui_editbox_select_text		( MGuiEditbox* editbox, uint32 begin, uint32 end );
MGUI_EXPORT uint32	mgui_editbox_get_cursor_pos		( MGuiEditbox* editbox );
MGUI_EXPORT void	mgui_editbox_set_cursor_pos		( MGuiEditbox* editbox, uint32 pos );

/** @}
 *  @defgroup label Label functions
 *  @{
 *  @ingroup element
 */
MGUI_EXPORT void	mgui_label_make_text_fit	( MGuiLabel* label );

/** @}
 *  @defgroup listbox Listbox functions
 *  @{
 *  @ingroup element
 */
MGUI_EXPORT MGuiListboxItem* mgui_listbox_add_item				( MGuiListbox* listbox, const char_t* text );
MGUI_EXPORT void			mgui_listbox_remove_item			( MGuiListbox* listbox, MGuiListboxItem* item );
MGUI_EXPORT void			mgui_listbox_clean					( MGuiListbox* listbox );
MGUI_EXPORT void			mgui_listbox_set_sort_function		( MGuiListbox* listbox, mgui_listbox_sort_t func );
MGUI_EXPORT uint32			mgui_listbox_get_item_count			( MGuiListbox* listbox );
MGUI_EXPORT uint32			mgui_listbox_get_selected_count		( MGuiListbox* listbox );
MGUI_EXPORT MGuiListboxItem* mgui_listbox_get_first_item		( MGuiListbox* listbox );
MGUI_EXPORT MGuiListboxItem* mgui_listbox_get_next_item			( MGuiListboxItem* item );
MGUI_EXPORT MGuiListboxItem* mgui_listbox_get_selected_item		( MGuiListbox* listbox );
MGUI_EXPORT MGuiListboxItem* mgui_listbox_get_next_selected_item( MGuiListboxItem* item );
MGUI_EXPORT const char_t*	mgui_listbox_get_item_text			( MGuiListboxItem* item );
MGUI_EXPORT void			mgui_listbox_set_item_text			( MGuiListboxItem* item, const char_t* text );
MGUI_EXPORT void*			mgui_listbox_get_item_data			( MGuiListboxItem* item );
MGUI_EXPORT void			mgui_listbox_set_item_data			( MGuiListboxItem* item, void* data );
MGUI_EXPORT void			mgui_listbox_get_selected_colour	( MGuiListbox* listbox, colour_t* col );
MGUI_EXPORT void			mgui_listbox_set_selected_colour	( MGuiListbox* listbox, const colour_t* col );
MGUI_EXPORT uint32			mgui_listbox_get_selected_colour_i	( MGuiListbox* listbox );
MGUI_EXPORT void			mgui_listbox_set_selected_colour_i	( MGuiListbox* listbox, uint32 hex );

/** @}
 *  @defgroup memobox Memobox functions
 *  @{
 *  @ingroup element
 */
MGUI_EXPORT void	mgui_memobox_add_line			( MGuiMemobox* memobox, const char* fmt, ... );
MGUI_EXPORT void	mgui_memobox_add_line_col		( MGuiMemobox* memobox, const char* fmt, const colour_t* col, ... );
MGUI_EXPORT void	mgui_memobox_add_line_s			( MGuiMemobox* memobox, const char* text );
MGUI_EXPORT void	mgui_memobox_add_line_col_s		( MGuiMemobox* memobox, const char* text, const colour_t* col );
MGUI_EXPORT void	mgui_memobox_clear				( MGuiMemobox* memobox );
MGUI_EXPORT float	mgui_memobox_get_display_pos	( MGuiMemobox* memobox );
MGUI_EXPORT void	mgui_memobox_set_display_pos	( MGuiMemobox* memobox, float pos );
MGUI_EXPORT uint32	mgui_memobox_get_lines			( MGuiMemobox* memobox );
MGUI_EXPORT void	mgui_memobox_set_lines			( MGuiMemobox* memobox, uint32 lines );
MGUI_EXPORT uint32	mgui_memobox_get_num_lines		( MGuiMemobox* memobox );
MGUI_EXPORT uint32	mgui_memobox_get_history		( MGuiMemobox* memobox );
MGUI_EXPORT void	mgui_memobox_set_history		( MGuiMemobox* memobox, uint32 lines );
MGUI_EXPORT uint32	mgui_memobox_get_margin			( MGuiMemobox* memobox );
MGUI_EXPORT void	mgui_memobox_set_margin			( MGuiMemobox* memobox, uint32 margin );

/** @}
 *  @defgroup progressbar Progressbar functions
 *  @{
 *  @ingroup element
 */
MGUI_EXPORT float	mgui_progressbar_get_value		( MGuiProgressbar* bar );
MGUI_EXPORT void	mgui_progressbar_set_value		( MGuiProgressbar* bar, float value );
MGUI_EXPORT float	mgui_progressbar_get_max_value	( MGuiProgressbar* bar );
MGUI_EXPORT void	mgui_progressbar_set_max_value	( MGuiProgressbar* bar, float value );
MGUI_EXPORT void	mgui_progressbar_get_colour		( MGuiProgressbar* bar, colour_t* col_start, colour_t* col_end );
MGUI_EXPORT void	mgui_progressbar_set_colour		( MGuiProgressbar* bar, const colour_t* col_start, const colour_t* col_end );
MGUI_EXPORT void	mgui_progressbar_get_colour_i	( MGuiProgressbar* bar, uint32* col_start, uint32* col_end );
MGUI_EXPORT void	mgui_progressbar_set_colour_i	( MGuiProgressbar* bar, uint32 col_start, uint32 col_end );
MGUI_EXPORT float	mgui_progressbar_get_bg_shade	( MGuiProgressbar* bar );
MGUI_EXPORT void	mgui_progressbar_set_bg_shade	( MGuiProgressbar* bar, float shade );
MGUI_EXPORT uint8	mgui_progressbar_get_thickness	( MGuiProgressbar* bar );
MGUI_EXPORT void	mgui_progressbar_set_thickness	( MGuiProgressbar* bar, uint8 thickness );

/** @}
 *  @defgroup scrollbar Scrollbar functions
 *  @{
 *  @ingroup element
 */
MGUI_EXPORT void	mgui_scrollbar_set_params		( MGuiScrollbar* scrollbar, float content, float step, float position, float size );
MGUI_EXPORT float	mgui_scrollbar_get_content_size	( MGuiScrollbar* scrollbar );
MGUI_EXPORT void	mgui_scrollbar_set_content_size	( MGuiScrollbar* scrollbar, float size );
MGUI_EXPORT float	mgui_scrollbar_get_step_size	( MGuiScrollbar* scrollbar );
MGUI_EXPORT void	mgui_scrollbar_set_step_size	( MGuiScrollbar* scrollbar, float size );
MGUI_EXPORT float	mgui_scrollbar_get_bar_pos		( MGuiScrollbar* scrollbar );
MGUI_EXPORT void	mgui_scrollbar_set_bar_pos		( MGuiScrollbar* scrollbar, float position );
MGUI_EXPORT float	mgui_scrollbar_get_bar_size		( MGuiScrollbar* scrollbar );
MGUI_EXPORT void	mgui_scrollbar_set_bar_size		( MGuiScrollbar* scrollbar, float size );
MGUI_EXPORT float	mgui_scrollbar_get_bg_shade		( MGuiScrollbar* scrollbar );
MGUI_EXPORT void	mgui_scrollbar_set_bg_shade		( MGuiScrollbar* scrollbar, float shade );

/** @}
 *  @defgroup sprite Sprite functions
 *  @{
 *  @ingroup element
 */
MGUI_EXPORT const char_t* mgui_sprite_get_texture	( MGuiSprite* sprite );
MGUI_EXPORT void	mgui_sprite_set_texture			( MGuiSprite* sprite, const char_t* texture );
MGUI_EXPORT void	mgui_sprite_get_texture_size	( MGuiSprite* sprite, vectorscreen_t* size );
MGUI_EXPORT void	mgui_sprite_get_texture_size_i	( MGuiSprite* sprite, uint16* width, uint16* height );
MGUI_EXPORT void	mgui_sprite_resize				( MGuiSprite* sprite );
MGUI_EXPORT void	mgui_sprite_get_uv				( MGuiSprite* sprite, float* u1, float* v1, float* u2, float* v2 );
MGUI_EXPORT void	mgui_sprite_set_uv				( MGuiSprite* sprite, float u1, float v1, float u2, float v2 );

/** @}
 *  @defgroup window Window functions
 *  @{
 *  @ingroup element
 */
MGUI_EXPORT void	mgui_window_get_title_colour	( MGuiWindow* window, colour_t* col );
MGUI_EXPORT void	mgui_window_set_title_colour	( MGuiWindow* window, const colour_t* col );
MGUI_EXPORT uint32	mgui_window_get_title_colour_i	( MGuiWindow* window );
MGUI_EXPORT void	mgui_window_set_title_colour_i	( MGuiWindow* window, uint32 hex );
MGUI_EXPORT void	mgui_window_get_drag_offset		( MGuiWindow* window, vectorscreen_t* pos );

/** @} */

__END_DECLS

#endif /* __MGUI_H */
