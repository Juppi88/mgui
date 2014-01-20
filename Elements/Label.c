/**
 *
 * @file		Label.c
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI label related functions.
 *
 * @details		Functions and structures related to GUI labels.
 *
 **/

#include "Label.h"
#include "Skin.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

// Label callback handlers
static void		mgui_label_render			( MGuiElement* label );
static void		mgui_label_on_text_change	( MGuiElement* label );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_label_render,
	NULL, /* post_render */
	NULL, /* process */
	NULL, /* get_clip_region */
	NULL, /* on_bounds_change */
	NULL, /* on_flags_change */
	NULL, /* on_colour_change */
	mgui_label_on_text_change,
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
 * @brief Creates a label.
 *
 * @details This function creates a GUI label. If the parent element
 * is NULL, the label will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @returns A pointer to the created label
 */
MGuiLabel* mgui_create_label( MGuiElement* parent )
{
	struct MGuiLabel* label;
	extern MGuiFont* default_font;

	label = mem_alloc_clean( sizeof(*label) );
	mgui_element_create( cast_elem(label), parent );

	label->type = GUI_LABEL;
	label->flags &= ~FLAG_CLIP;

	label->colour.a = 255;
	label->text->colour.a = 255;

	label->font = default_font;
	label->text->font = default_font;

	// Label callbacks
	label->callbacks = &callbacks;

	return cast_elem(label);
}

/**
 * @brief Creates a label (extended).
 *
 * @details This function creates a GUI label with the given parameters.
 * If the parent element is NULL, the label will become a layer.
 *
 * @param parent The parent element, or NULL if the element is to be created without a parent
 * @param x The absolute x coordinate relative to the parent
 * @param y The absolute y coordinate relative to the parent
 * @param flags Any additional flags that will be applied as a bitmask (see @ref MGUI_FLAGS)
 * @param col The text colour of the label as a 32bit hex integer
 * @param text The text to be displayed
 *
 * @returns A pointer to the created label
 */
MGuiLabel* mgui_create_label_ex( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col, const char_t* text )
{
	MGuiLabel* label;

	label = mgui_create_label( parent );

	mgui_set_abs_pos_i( label, x, y );
	mgui_add_flags( label, flags );
	mgui_set_text_colour_i( label, col );
	mgui_set_text_s( label, text );

	return label;
}

static void mgui_label_render( MGuiElement* label )
{
	label->skin->draw_label( label );
}

static void mgui_label_on_text_change( MGuiElement* label )
{
	mgui_label_make_text_fit( label );
}

/**
 * @brief Forces the size of the label to match the size of the text.
 *
 * @details This function resizes the label to make its size match
 * the size of the text exactly.
 *
 * @param label The label to resize
 */
void mgui_label_make_text_fit( MGuiLabel* label )
{
	uint16 w, h, padding;

	if ( label == NULL ) return;
	if ( label->text == NULL ) return;
	if ( label->font == NULL ) return;

	w = label->bounds.w;
	h = label->bounds.h;

	padding = ( label->font->flags & FFLAG_OUTLINE ) ? 2 : 0;

	if ( w >= label->text->size.x && h >= label->text->size.y ) return;

	// Dodgy. yes. - Edit: Even worse now. Really need to add precise dimensions to the font struct... For now let's just stab ourselves in the eye.
	w = label->text->size.x + label->text->pad.left + label->text->pad.right + label->text->size.y + padding;
	h = (uint16)( 1.5f * ( label->text->size.y + label->text->pad.top + label->text->pad.bottom ) ) + padding;

	label->bounds.w = math_max( w, label->bounds.w );
	label->bounds.h = math_max( h, label->bounds.h );

	mgui_text_update_position( label->text );
	mgui_element_request_redraw( label );
}
