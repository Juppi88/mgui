/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Label.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI label related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 * 
 **********************************************************************/

#include "Label.h"
#include "Skin.h"
#include "Platform/Alloc.h"

// Label callback handlers
static void			mgui_destroy_label			( MGuiElement* label );
static void			mgui_label_render			( MGuiElement* label );

MGuiLabel* mgui_create_label( MGuiElement* parent )
{
	struct MGuiLabel* label;

	label = mem_alloc_clean( sizeof(*label) );
	mgui_element_create( cast_elem(label), parent );

	label->type = GUI_LABEL;
	label->flags |= FLAG_CLIP;

	label->font = mgui_font_create( DEFAULT_FONT, 11, FFLAG_NONE, ANSI_CHARSET );
	label->text->font = label->font;

	// Label callbacks
	label->destroy = mgui_destroy_label;
	label->render = mgui_label_render;

	return cast_elem(label);
}

MGuiLabel* mgui_create_label_ex( MGuiElement* parent, uint16 x, uint16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text )
{
	MGuiLabel* label;

	label = mgui_create_label( parent );

	mgui_set_abs_pos_i( label, x, y );
	mgui_set_abs_size_i( label, w, h );
	mgui_add_flags( label, flags );
	mgui_set_colour_i( label, col );
	mgui_set_text_s( label, text );

	return label;
}

static void mgui_destroy_label( MGuiElement* label )
{
	UNREFERENCED_PARAM( label );
}

static void mgui_label_render( MGuiElement* label )
{
	label->skin->draw_label( label );
}

void mgui_label_make_text_fit( MGuiLabel* label )
{
	uint16 w, h;
	
	if ( label == NULL ) return;
	if ( label->text == NULL ) return;

	w = label->bounds.w;
	h = label->bounds.h;

	if ( w >= label->text->size.x && h >= label->text->size.y ) return;

	// Dodgy. yes. - Edit: Even worse now. Really need to add precise dimensions to the font struct... For now let's just stab ourselves in the eye.
	w = label->text->size.x + label->text->pad.left + label->text->pad.right + label->text->size.y;
	h = (uint16)( 1.5f * ( label->text->size.y + label->text->pad.top + label->text->pad.bottom ) );

	label->bounds.w = math_max( w, label->bounds.w );
	label->bounds.h = math_max( h, label->bounds.h );

	label->set_bounds( label, false, true );
	mgui_text_update_position( label->text );
}
