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

MGuiLabel* mgui_create_label( MGuiControl* parent )
{
	struct MGuiLabel* label;

	label = mem_alloc_clean( sizeof(*label) );
	mgui_element_create( cast_elem(label), parent, true );

	label->type = GUI_LABEL;
	label->flags |= FLAG_CLIP;

	label->font = mgui_font_create( DEFAULT_FONT, 11, FFLAG_NONE, ANSI_CHARSET );
	label->text->font = label->font;

	// Label callbacks
	label->destroy = mgui_destroy_label;
	label->render = mgui_label_render;

	return cast_elem(label);
}

static void mgui_destroy_label( MGuiElement* label )
{
	UNREFERENCED_PARAM( label );
}

static void mgui_label_render( MGuiElement* label )
{
	skin->draw_label( &label->bounds, &label->colour, label->flags, label->text );
}

void mgui_label_make_text_fit( MGuiLabel* label )
{
	uint16 w, h;
	
	if ( label == NULL ) return;
	if ( label->text == NULL ) return;

	w = label->bounds.w;
	h = label->bounds.h;

	if ( w >= label->text->size.x && h >= label->text->size.y ) return;

	// Dodgy. yes.
	w = label->text->size.x + label->text->pad.left + label->text->pad.right;
	h = (uint16)( 1.3f * ( label->text->size.y + label->text->pad.top + label->text->pad.bottom ) );

	label->bounds.w = math_max( w, label->bounds.w );
	label->bounds.h = math_max( h, label->bounds.h );

	label->set_bounds( label, false, true );
}
