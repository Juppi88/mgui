/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		SkinTextured.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		An implementation of a textured GUI skin.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "SkinTextured.h"
#include "Element.h"
#include "Editbox.h"
#include "Listbox.h"
#include "Memobox.h"
#include "Progressbar.h"
#include "Scrollbar.h"
#include "Window.h"
#include "WindowButton.h"
#include "WindowTitlebar.h"
#include "Renderer.h"
#include "Texture.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

extern MGuiRenderer* renderer;

// --------------------------------------------------

typedef enum {
	TEX_TOPLEFT,		// Top left corner
	TEX_TOP,			// Top border
	TEX_TOPRIGHT,		// Top right corner
	TEX_RIGHT,			// Right border
	TEX_BOTTOMRIGHT,	// Bottom right corner
	TEX_BOTTOM,			// Bottom border
	TEX_BOTTOMLEFT,		// Bottom left corner
	TEX_LEFT,			// Left border
	TEX_CENTRE,			// Centre area
	NUM_REGIONS
} TEXTURE_REGION;

typedef enum {
	MARGIN_TOP,
	MARGIN_BOTTOM,
	MARGIN_LEFT,
	MARGIN_RIGHT
} MARGIN;

typedef enum {
	BUTTON_IDLE,
	BUTTON_HOVERED,
	BUTTON_PRESSED,
	NUM_STATES
} BUTTON_STATE;

typedef enum {
	ARROWDIR_LEFT,
	ARROWDIR_RIGHT,
	ARROWDIR_DOWN,
	ARROWDIR_UP,
	NUM_ARROWS
} SCROLL_ARROW;

// --------------------------------------------------

typedef struct {
	float	uv[4];				// Texture coordinates
	uint16	size[2];			// Size of the texture in pixels
} MGuiTex;

// --------------------------------------------------

typedef struct {
	float	uv[NUM_REGIONS][4];	// Texture coordinates for each region
	uint8	margin[4];			// Border texture size in pixels
	uint16	size[2];			// Overall texture size in pixels
} MGuiTexBorder;

// --------------------------------------------------

typedef struct {
	MGuiSkin			api;				// Element drawing functions
	MGuiTexture*		texture;			// Pointer to texture
	
	// Texture coordinates for each element
	struct {
		MGuiTexBorder	panel;				// Generic panel
		MGuiTexBorder	button;				// Buttons
		MGuiTexBorder	button_hover;
		MGuiTexBorder	button_pressed;
		MGuiTexBorder	button_inactive;
		MGuiTexBorder	checkbox;			// Checkboxes
		MGuiTexBorder	checkbox_checked;
		MGuiTexBorder	checkbox_off;
		MGuiTexBorder	checkbox_off_checked;
		MGuiTexBorder	editbox;			// Editboxes
		MGuiTexBorder	editbox_focus;
		MGuiTexBorder	editbox_inactive;
		MGuiTexBorder	label;				// Labels
		MGuiTexBorder	progbar;			// Progressbars
		MGuiTexBorder	progbar_bg;
		MGuiTexBorder	scroll_bg_vert;		// Scrollbars
		MGuiTexBorder	scroll_bg_horiz;
		MGuiTexBorder	scroll_bar_vert[NUM_STATES];
		MGuiTexBorder	scroll_bar_horiz[NUM_STATES];
		MGuiTexBorder	scroll_buttons[NUM_ARROWS][NUM_STATES];
		MGuiTexBorder	window;				// Windows
		MGuiTexBorder	window_resizable;
		MGuiTexBorder	window_titlebar;
		MGuiTex			window_closebtn;
		MGuiTex			window_closebtn_hover;
		MGuiTex			window_closebtn_pressed;
	} textures;
} MGuiTexturedSkin;

// --------------------------------------------------

static void		skin_textured_setup_primitive			( MGuiTexture* texture, MGuiTex* primitive, uint32 x, uint32 y, uint32 x2, uint32 y2 );
static void		skin_textured_setup_primitive_bordered	( MGuiTexture* texture, MGuiTexBorder* primitive, uint32 x, uint32 y, uint32 x2, uint32 y2, uint8 top, uint8 bottom, uint8 left, uint8 right );
static void		skin_textured_draw_panel				( MGuiTexture* texture, const MGuiTex* prim, const rectangle_t* r, const colour_t* col );
static void		skin_textured_draw_bordered_panel		( MGuiTexture* texture, const MGuiTexBorder* prim, const rectangle_t* r, const colour_t* col, uint32 borders, bool panel );
static void		skin_textured_draw_button				( MGuiElement* element );
static void		skin_textured_draw_checkbox				( MGuiElement* element );
static void		skin_textured_draw_editbox				( MGuiElement* element );
static void		skin_textured_draw_label				( MGuiElement* element );
static void		skin_textured_draw_listbox				( MGuiElement* element );
static void		skin_textured_draw_memobox				( MGuiElement* element );
static void		skin_textured_draw_progressbar			( MGuiElement* element );
static void		skin_textured_draw_scrollbar			( MGuiElement* element );
static void		skin_textured_draw_window				( MGuiElement* element );
static void		skin_textured_draw_window_closebtn		( struct MGuiWindowButton* button );

// --------------------------------------------------

MGuiSkin* mgui_setup_skin_textured( const char_t* path )
{
	MGuiTexturedSkin* skin;
	MGuiTexture* texture;

	// Try to load the skin first, if it fails there's no point in going further
	texture = mgui_texture_create( path );

	if ( texture == NULL ) return NULL;

	// Well, the texture exists so let's create the skin instance
	skin = mem_alloc_clean( sizeof(*skin) );

	skin->api.draw_button		= skin_textured_draw_button;
	skin->api.draw_checkbox		= skin_textured_draw_checkbox;
	skin->api.draw_editbox		= skin_textured_draw_editbox;
	skin->api.draw_label		= skin_textured_draw_label;
	skin->api.draw_listbox		= skin_textured_draw_listbox;
	skin->api.draw_memobox		= skin_textured_draw_memobox;
	skin->api.draw_progressbar	= skin_textured_draw_progressbar;
	skin->api.draw_scrollbar	= skin_textured_draw_scrollbar;
	skin->api.draw_window		= skin_textured_draw_window;

	skin->texture = texture;

	// Generic panel
	skin_textured_setup_primitive_bordered( texture, &skin->textures.panel, 129, 65, 192, 128, 3, 3, 3, 3 );

	// Button textures
	skin_textured_setup_primitive_bordered( texture, &skin->textures.button, 193, 65, 224, 87, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.button_hover, 225, 65, 256, 87, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.button_pressed, 193, 88, 224, 109, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.button_inactive, 225, 88, 256, 109, 2, 2, 2, 2 );

	// Checkbox textures
	skin_textured_setup_primitive_bordered( texture, &skin->textures.checkbox, 13, 129, 25, 141, 1, 1, 1, 1 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.checkbox_checked, 0, 129, 12, 141, 1, 1, 1, 1 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.checkbox_off_checked, 26, 129, 38, 141, 1, 1, 1, 1 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.checkbox_off, 39, 129, 51, 141, 1, 1, 1, 1 );

	// Editbox textures
	skin_textured_setup_primitive_bordered( texture, &skin->textures.editbox, 0, 65, 64, 86, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.editbox_focus, 0, 87, 64, 108, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.editbox_inactive, 0, 109, 64, 128, 3, 3, 3, 3 );

	// Label textures
	skin_textured_setup_primitive_bordered( texture, &skin->textures.label, 65, 65, 128, 107, 2, 2, 2, 2 );

	// Progressbar textures
	skin_textured_setup_primitive_bordered( texture, &skin->textures.progbar, 65, 108, 97, 128, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.progbar_bg, 98, 108, 128, 128, 2, 2, 2, 2 );

	// Scrollbar textures
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_bg_vert, 113, 129, 128, 192, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_bg_horiz, 0, 241, 64, 256, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_bar_vert[BUTTON_IDLE], 65, 129, 80, 192, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_bar_vert[BUTTON_HOVERED], 81, 129, 96, 192, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_bar_vert[BUTTON_PRESSED], 97, 129, 112, 192, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_bar_horiz[BUTTON_IDLE], 0, 193, 64, 208, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_bar_horiz[BUTTON_HOVERED], 0, 209, 64, 224, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_bar_horiz[BUTTON_PRESSED], 0, 225, 64, 240, 2, 2, 2, 2 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_LEFT][BUTTON_IDLE], 0, 142, 15, 157, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_LEFT][BUTTON_HOVERED], 0, 158, 15, 173, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_LEFT][BUTTON_PRESSED], 0, 174, 15, 189, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_RIGHT][BUTTON_IDLE], 16, 142, 31, 157, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_RIGHT][BUTTON_HOVERED], 16, 158, 31, 173, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_RIGHT][BUTTON_PRESSED], 16, 174, 31, 189, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_DOWN][BUTTON_IDLE], 32, 142, 47, 157, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_DOWN][BUTTON_HOVERED], 32, 158, 47, 173, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_DOWN][BUTTON_PRESSED], 32, 174, 47, 189, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_UP][BUTTON_IDLE], 48, 142, 63, 157, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_UP][BUTTON_HOVERED], 48, 158, 63, 173, 3, 3, 3, 3 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.scroll_buttons[ARROWDIR_UP][BUTTON_PRESSED], 48, 174, 63, 189, 3, 3, 3, 3 );

	// Window textures
	skin_textured_setup_primitive_bordered( texture, &skin->textures.window, 0, 26, 64, 64, 6, 6, 6, 6 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.window_resizable, 65, 26, 128, 64, 12, 12, 12, 12 );
	skin_textured_setup_primitive_bordered( texture, &skin->textures.window_titlebar,	0, 0, 128, 25, 5, 2, 5, 5 );
	skin_textured_setup_primitive( texture, &skin->textures.window_closebtn, 193, 110, 213, 128 );
	skin_textured_setup_primitive( texture, &skin->textures.window_closebtn_hover, 214, 110, 234, 128 );
	skin_textured_setup_primitive( texture, &skin->textures.window_closebtn_pressed, 235, 110, 255, 128 );

	return (MGuiSkin*)skin;
}

static MYLLY_INLINE void skin_textured_init_coordinates( uint32 tex_width, uint32 tex_height, float uv[4], uint32 x, uint32 y, uint32 w, uint32 h )
{
	uv[0] = (float)x / tex_width;
	uv[1] = (float)y / tex_height;
	uv[2] = (float)( x + w ) / tex_width;
	uv[3] = (float)( y + h ) / tex_height;
}

static void skin_textured_setup_primitive( MGuiTexture* texture, MGuiTex* primitive, uint32 x, uint32 y, uint32 x2, uint32 y2 )
{
	uint32 width = texture->width;
	uint32 height = texture->height;
	uint32 w = x2 - x, h = y2 - y;

	primitive->size[0] = (uint16)w;
	primitive->size[1] = (uint16)h;

	skin_textured_init_coordinates( width, height, primitive->uv, x, y, w, h );
}

static void skin_textured_setup_primitive_bordered( MGuiTexture* texture, MGuiTexBorder* primitive, uint32 x, uint32 y, uint32 x2, uint32 y2, uint8 top, uint8 bottom, uint8 left, uint8 right )
{
	uint32 width = texture->width;
	uint32 height = texture->height;
	uint32 w = x2 - x, h = y2 - y;

	primitive->size[0] = (uint16)w;
	primitive->size[1] = (uint16)h;

	primitive->margin[MARGIN_TOP] = top;
	primitive->margin[MARGIN_BOTTOM] = bottom;
	primitive->margin[MARGIN_LEFT] = left;
	primitive->margin[MARGIN_RIGHT] = right;

	skin_textured_init_coordinates( width, height, primitive->uv[TEX_TOP], x+left, y, w-left-right, top );
	skin_textured_init_coordinates( width, height, primitive->uv[TEX_TOPRIGHT], x+w-right, y, right, top );
	skin_textured_init_coordinates( width, height, primitive->uv[TEX_RIGHT], x+w-right, y+top, right, h-top-bottom );
	skin_textured_init_coordinates( width, height, primitive->uv[TEX_BOTTOMRIGHT], x+w-right, y+h-bottom, right, bottom );
	skin_textured_init_coordinates( width, height, primitive->uv[TEX_BOTTOM], x+left, y+h-bottom, w-left-right, bottom );
	skin_textured_init_coordinates( width, height, primitive->uv[TEX_BOTTOMLEFT], x, y+h-bottom, left, bottom );
	skin_textured_init_coordinates( width, height, primitive->uv[TEX_LEFT], x, y+top, left, h-top-bottom );
	skin_textured_init_coordinates( width, height, primitive->uv[TEX_TOPLEFT], x, y, left, top );
	skin_textured_init_coordinates( width, height, primitive->uv[TEX_CENTRE], x+left, y+top, w-left-right, h-top-bottom );
}

static void skin_textured_draw_panel( MGuiTexture* texture, const MGuiTex* prim, const rectangle_t* r, const colour_t* col )
{
	renderer->set_draw_colour( col );
	renderer->draw_textured_rect( texture->data, r->x, r->y, r->w, r->h, prim->uv );
}

static void skin_textured_draw_bordered_panel( MGuiTexture* texture, const MGuiTexBorder* prim, const rectangle_t* r,
											   const colour_t* col, uint32 borders, bool panel )
{
	const float* uv;
	uint32 top = ( borders & BORDER_TOP ) ? prim->margin[MARGIN_TOP] : 0,
		   bottom = ( borders & BORDER_BOTTOM ) ? prim->margin[MARGIN_BOTTOM] : 0,
		   left = ( borders & BORDER_LEFT ) ? prim->margin[MARGIN_LEFT] : 0,
		   right = ( borders & BORDER_RIGHT ) ? prim->margin[MARGIN_RIGHT] : 0;

	renderer->set_draw_colour( col );

	// Left border
	if ( left != 0 )
	{
		uv = prim->uv[TEX_LEFT];
		renderer->draw_textured_rect( texture->data, r->x, r->y + top, left, r->h - top - bottom, uv );
	}

	// Right border
	if ( right != 0 )
	{
		uv = prim->uv[TEX_RIGHT];
		renderer->draw_textured_rect( texture->data, r->x + r->w - right, r->y + top, right, r->h - top - bottom, uv );
	}

	// Top border + corners
	if ( top != 0 )
	{
		uv = prim->uv[TEX_TOP];
		renderer->draw_textured_rect( texture->data, r->x + left, r->y, r->w - left - right, top, uv );

		uv = prim->uv[TEX_TOPLEFT];
		renderer->draw_textured_rect( texture->data, r->x, r->y, left, top, uv );

		uv = prim->uv[TEX_TOPRIGHT];
		renderer->draw_textured_rect( texture->data, r->x + r->w - right, r->y, right, top, uv );
	}

	// Bottom border + corners
	if ( bottom != 0 )
	{
		uv = prim->uv[TEX_BOTTOM];
		renderer->draw_textured_rect( texture->data, r->x + left, r->y + r->h - bottom, r->w - left - right, bottom, uv );

		uv = prim->uv[TEX_BOTTOMLEFT];
		renderer->draw_textured_rect( texture->data, r->x, r->y + r->h - bottom, left, bottom, uv );

		uv = prim->uv[TEX_BOTTOMRIGHT];
		renderer->draw_textured_rect( texture->data, r->x + r->w - right, r->y + r->h - bottom, right, bottom, uv );
	}

	// Background panel
	if ( panel )
	{
		uv = prim->uv[TEX_CENTRE];
		renderer->draw_textured_rect( texture->data, r->x + left, r->y + top, r->w - left - right, r->h - top - bottom, uv );
	}
}

static void skin_textured_draw_button( MGuiElement* element )
{
	MGuiTexturedSkin* skin = (MGuiTexturedSkin*)element->skin;
	MGuiText* text = element->text;
	MGuiTexBorder* primitive;
	colour_t col;
	int32 x, y;

	// Button inactive
	if ( element->flags & FLAG_DISABLED )
		primitive = &skin->textures.button_inactive;

	// Button pressed
	else if ( element->flags_int & INTFLAG_PRESSED )
		primitive = &skin->textures.button_pressed;

	// Button hovered
	else if ( element->flags_int & INTFLAG_HOVER )
		primitive = &skin->textures.button_hover;

	// Button idle
	else primitive = &skin->textures.button;

	// Draw button
	skin_textured_draw_bordered_panel( skin->texture, primitive, &element->bounds, &element->colour,
									   element->flags & FLAG_BORDER ? BORDER_ALL : BORDER_NONE, element->flags & FLAG_BACKGROUND );
	
	// Draw text
	if ( text != NULL )
	{
		x = text->pos.x, y = text->pos.y;
		col = text->colour;

		if ( element->flags & FLAG_DISABLED )
		{
			col.a /= 2;
		}
		else if ( element->flags_int & INTFLAG_PRESSED )
		{
			x += 1;
			y += 1;
		}
		else if ( element->flags_int & INTFLAG_HOVER )
		{
			colour_multiply( &col, &col, 1.2f );
			col.a = text->colour.a;
		}

		renderer->set_draw_colour( &col );
		renderer->draw_text( text->font->data, text->buffer, x, y,
							 text->flags, text->tags, text->num_tags );
	}
}

static void skin_textured_draw_checkbox( MGuiElement* element )
{
	MGuiTexturedSkin* skin = (MGuiTexturedSkin*)element->skin;
	MGuiTexBorder* primitive;

	if ( element->flags & FLAG_DISABLED )
	{
		// Checkbox toggled
		if ( element->flags & FLAG_CHECKBOX_CHECKED )
			primitive = &skin->textures.checkbox_off_checked;

		// Checkbox idle
		else primitive = &skin->textures.checkbox_off;
	}
	else
	{
		// Checkbox toggled
		if ( element->flags & FLAG_CHECKBOX_CHECKED )
			primitive = &skin->textures.checkbox_checked;

		// Checkbox idle
		else primitive = &skin->textures.checkbox;
	}

	skin_textured_draw_bordered_panel( skin->texture, primitive, &element->bounds, &element->colour,
									   element->flags & FLAG_BORDER ? BORDER_ALL : BORDER_NONE, element->flags & FLAG_BACKGROUND );
}

static void skin_textured_draw_editbox( MGuiElement* element )
{
	MGuiTexturedSkin* skin = (MGuiTexturedSkin*)element->skin;
	struct MGuiEditbox* editbox = (struct MGuiEditbox*)element;
	MGuiText* text = element->text;
	MGuiTexBorder* primitive;
	colour_t col;
	char_t* tmp;

	// Editbox inactive
	if ( editbox->flags & FLAG_DISABLED )
		primitive = &skin->textures.editbox_inactive;

	// Editbox has focus
	else if ( editbox->flags_int & INTFLAG_FOCUS )
		primitive = &skin->textures.editbox_focus;

	// Editbox is idle
	else primitive = &skin->textures.editbox;

	// Draw editbox
	skin_textured_draw_bordered_panel( skin->texture, primitive, &editbox->bounds, &editbox->colour,
									   editbox->flags & FLAG_BORDER ? BORDER_ALL : BORDER_NONE, editbox->flags & FLAG_BACKGROUND );

	// Draw selection
	if ( editbox->flags_int & INTFLAG_FOCUS )
	{
		// Draw the cursor, also make sure it's within the editbox boundaries
		if ( ( editbox->cursor_visible || BIT_OFF( editbox->flags, FLAG_ANIMATION ) ) &&
			   editbox->cursor.x < editbox->bounds.x + editbox->bounds.w )
		{
			renderer->set_draw_colour( &editbox->text->colour );
			renderer->draw_rect( editbox->cursor.x, editbox->cursor.y, editbox->cursor.w, editbox->cursor.h );
		}

		// Draw the selection. Invert the background colour.
		if ( mgui_editbox_has_text_selected( (MGuiEditbox*)editbox ) )
		{
			col = editbox->colour; col.a = 90;
			colour_invert( &col, &col );

			renderer->set_draw_colour( &col );
			renderer->draw_rect( editbox->selection.x, editbox->selection.y, editbox->selection.w, editbox->selection.h );
		}
	}

	if ( text != NULL )
	{
		col = text->colour;

		if ( editbox->flags & FLAG_DISABLED )
			col.a /= 2;

		// This is a really ugly hack to make mgui_get_text return the correct buffer:
		// We replace the MGuiText buffer with our own (with masked input etc cool)
		// while we render, and put the original buffer back afterwards
		tmp = editbox->text->buffer;
		editbox->text->buffer = editbox->buffer;

		renderer->set_draw_colour( &col );
		renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y,
							 text->flags, text->tags, text->num_tags );

		// Really ugly hack cleanup
		editbox->text->buffer = tmp;
	}
}

static void skin_textured_draw_label( MGuiElement* element )
{
	MGuiTexturedSkin* skin = (MGuiTexturedSkin*)element->skin;
	MGuiText* text = element->text;

	// Draw background and borders
	if ( element->flags & (FLAG_BORDER|FLAG_BACKGROUND) )
	{
		skin_textured_draw_bordered_panel( skin->texture, &skin->textures.label, &element->bounds, &element->colour,
										   element->flags & FLAG_BORDER ? BORDER_ALL : BORDER_NONE, element->flags & FLAG_BACKGROUND );
	}

	// Draw text
	if ( text != NULL )
	{
		renderer->set_draw_colour( &text->colour );
		renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y,
							 text->flags, text->tags, text->num_tags );
	}
}

static void skin_textured_draw_listbox( MGuiElement* element )
{
	MGuiTexturedSkin* skin = (MGuiTexturedSkin*)element->skin;
	struct MGuiListbox* listbox = (struct MGuiListbox*)element;
	MGuiListboxItem* item;
	node_t* next;
	uint32 count;

	// Draw listbox background and border
	if ( listbox->flags & (FLAG_BORDER|FLAG_BACKGROUND) )
	{
		skin_textured_draw_bordered_panel( skin->texture, &skin->textures.panel, &listbox->bounds, &listbox->colour,
										   listbox->flags & FLAG_BORDER ? BORDER_ALL : BORDER_NONE, listbox->flags & FLAG_BACKGROUND );
	}

	if ( list_empty( listbox->items ) ) return;

	// Draw (visible) items.
	renderer->set_draw_colour( &listbox->text->colour );
	item = listbox->first_visible;

	for ( count = 0; count < listbox->max_visible; ++count )
	{
		if ( item == NULL ) break;

		// If this item is selected, draw the background first.
		if ( item->selected )
		{
			skin_textured_draw_bordered_panel( skin->texture, &skin->textures.label, &item->bounds,
											   &listbox->select_colour, BORDER_ALL, true );

			renderer->set_draw_colour( &listbox->text->colour );
		}

		// Draw the text.
		renderer->draw_text( listbox->font->data, item->text, item->text_bounds.x, item->text_bounds.y, listbox->text->flags, item->tags, item->ntags );

		next = item->node.next;
		item = ( next != list_end( listbox->items ) ) ? (MGuiListboxItem*)next : NULL;
	}
}

static void skin_textured_draw_memobox( MGuiElement* element )
{
	MGuiTexturedSkin* skin = (MGuiTexturedSkin*)element->skin;
	struct MGuiMemobox* memo = (struct MGuiMemobox*)element;
	struct MGuiMemoLine* line;
	node_t* node;
	uint32 i, count, colour = 0;

	// Draw memobox background and border
	if ( memo->flags & (FLAG_BORDER|FLAG_BACKGROUND) )
	{
		skin_textured_draw_bordered_panel( skin->texture, &skin->textures.panel, &memo->bounds, &memo->colour,
										   memo->flags & FLAG_BORDER ? BORDER_ALL : BORDER_NONE, memo->flags & FLAG_BACKGROUND );
	}

	// Draw memobox lines
	if ( memo->lines->size == 0 ) return;

	count = memo->visible_lines;
	if ( count == 0 ) count = 0xFFFFFFFF;

	for ( node = memo->first_line, i = 0;
		  node != list_end(memo->lines) && i < count && node;
		  node = node->prev, i++ )
	{
		line = (struct MGuiMemoLine*)node;

		if ( line->colour.hex != colour )
		{
			renderer->set_draw_colour( &line->colour );
			colour = line->colour.hex;
		}

		renderer->draw_text( line->font->data, line->text, line->pos.x, line->pos.y,
							 line->font->flags & FFLAG_ITALIC ? TFLAG_ITALIC : 0,
							 line->tags, line->ntags );
	}
}

static void skin_textured_draw_progressbar( MGuiElement* element )
{
	struct MGuiProgressbar* progbar = (struct MGuiProgressbar*)element;
	MGuiTexturedSkin* skin = (MGuiTexturedSkin*)element->skin;
	rectangle_t fg, bg;
	float percentage;
	uint16 width;

	percentage = progbar->value / progbar->max_value;
	percentage = math_clampf( percentage, 0, 1 );

	// Special case (progress = 100%)
	if ( percentage == 1 )
	{
		skin_textured_draw_bordered_panel( skin->texture, &skin->textures.progbar, &progbar->bounds, &progbar->colour_fg,
										   progbar->flags & FLAG_BORDER ? BORDER_ALL : BORDER_NONE, progbar->flags & FLAG_BACKGROUND );
		return;
	}

	// Special case (progress = 0%)
	if ( percentage == 0 )
	{
		skin_textured_draw_bordered_panel( skin->texture, &skin->textures.progbar_bg, &progbar->bounds, &progbar->colour_bg,
										   progbar->flags & FLAG_BORDER ? BORDER_ALL : BORDER_NONE, progbar->flags & FLAG_BACKGROUND );
		return;
	}

	// Progress is between 0% and 100%, draw bar and background
	width = (uint16)( percentage * progbar->bounds.uw );

	fg = progbar->bounds;
	fg.uw = width;

	bg.x = fg.x + width;
	bg.y = fg.y;
	bg.uw = progbar->bounds.uw - width;
	bg.uh = fg.uh;

	skin_textured_draw_bordered_panel( skin->texture, &skin->textures.progbar, &fg, &progbar->colour_fg,
									   progbar->flags & FLAG_BORDER ? BORDER_ALL : BORDER_NONE, progbar->flags & FLAG_BACKGROUND );

	skin_textured_draw_bordered_panel( skin->texture, &skin->textures.progbar_bg, &bg, &progbar->colour_bg,
									   progbar->flags & FLAG_BORDER ? BORDER_ALL&(~BORDER_LEFT) : BORDER_NONE, progbar->flags & FLAG_BACKGROUND );
}

static void skin_textured_draw_scrollbar( MGuiElement* element )
{
	struct MGuiScrollbar* bar = (struct MGuiScrollbar*)element;
	MGuiTexturedSkin* skin = (MGuiTexturedSkin*)element->skin;
	const MGuiTexBorder* primitive;
	bool horiz = BIT_ON( bar->flags, FLAG_SCROLLBAR_HORIZ );
	uint32 button, state;

	// Background
	primitive = horiz ? &skin->textures.scroll_bg_horiz : &skin->textures.scroll_bg_vert;

	skin_textured_draw_bordered_panel( skin->texture, primitive, &bar->background,
									   &bar->bg_colour, BORDER_ALL, true );

	// Button 1 (left/up)
	button = horiz ? ARROWDIR_LEFT : ARROWDIR_UP;

	if ( bar->scroll_flags & SCROLL_BUTTON1_PRESSED )
		state = BUTTON_PRESSED;

	else if ( bar->scroll_flags & SCROLL_BUTTON1_HOVER )
		state = BUTTON_HOVERED;

	else state = BUTTON_IDLE;

	skin_textured_draw_bordered_panel( skin->texture, &skin->textures.scroll_buttons[button][state],
									   &bar->button1, &bar->colour, BORDER_ALL, true );

	// Button 2 (right/down)
	button = horiz ? ARROWDIR_RIGHT : ARROWDIR_DOWN;

	if ( bar->scroll_flags & SCROLL_BUTTON2_PRESSED )
		state = BUTTON_PRESSED;

	else if ( bar->scroll_flags & SCROLL_BUTTON2_HOVER )
		state = BUTTON_HOVERED;

	else state = BUTTON_IDLE;

	skin_textured_draw_bordered_panel( skin->texture, &skin->textures.scroll_buttons[button][state],
									   &bar->button2, &bar->colour, BORDER_ALL, true );

	// Scrollbar
	if ( bar->scroll_flags & SCROLL_BAR_PRESSED )
		state = BUTTON_PRESSED;

	else if ( bar->scroll_flags & SCROLL_BAR_HOVER )
		state = BUTTON_HOVERED;

	else state = BUTTON_IDLE;

	primitive = horiz ? &skin->textures.scroll_bar_horiz[state] : &skin->textures.scroll_bar_vert[state];

	skin_textured_draw_bordered_panel( skin->texture, primitive, &bar->bar,
									   &bar->colour, BORDER_ALL, true );
}

static void skin_textured_draw_window( MGuiElement* element )
{
	struct MGuiWindow* window = (struct MGuiWindow*)element;
	MGuiTexturedSkin* skin = (MGuiTexturedSkin*)element->skin;
	const MGuiTexBorder* primitive;
	MGuiText* text;
	bool titlebar = false;

	primitive = ( window->flags & FLAG_WINDOW_RESIZABLE ) ? &skin->textures.window_resizable : &skin->textures.window;

	renderer->set_draw_colour( &window->colour );

	// Draw titlebar
	if ( ( window->flags & FLAG_WINDOW_TITLEBAR ) && window->titlebar != NULL )
	{
		skin_textured_draw_bordered_panel( skin->texture, &skin->textures.window_titlebar, &window->titlebar->bounds,
										   &window->titlebar->colour, BORDER_ALL, true );

		titlebar = true;
		text = window->titlebar->text;

		renderer->set_draw_colour( &text->colour );
		renderer->draw_text( text->font->data, text->buffer, text->pos.x, text->pos.y,
							 text->flags, text->tags, text->num_tags );
	}

	// Draw closebutton
	if ( ( window->flags & FLAG_WINDOW_CLOSEBTN ) && window->closebtn != NULL )
	{
		skin_textured_draw_window_closebtn( window->closebtn );
	}

	// Draw borders and background panel
	if ( window->flags & FLAG_BORDER )
	{
		skin_textured_draw_bordered_panel( skin->texture, primitive, &window->bounds, &window->colour,
										   titlebar ? BORDER_ALL&(~BORDER_TOP) : BORDER_ALL, BIT_ON( window->flags, FLAG_BACKGROUND ) );
	}

	// Draw only the background panel
	else if ( window->flags & FLAG_BACKGROUND )
	{
		skin_textured_draw_bordered_panel( skin->texture, primitive, &window->bounds,
										   &window->colour, BORDER_NONE, true );
	}
}

static void skin_textured_draw_window_closebtn( struct MGuiWindowButton* button )
{
	MGuiTexturedSkin* skin = (MGuiTexturedSkin*)button->window->skin;
	MGuiTex* texture;

	// Button pressed
	if ( button->flags_int & INTFLAG_PRESSED )
		texture = &skin->textures.window_closebtn_pressed;

	// Button hovered
	else if ( button->flags_int & INTFLAG_HOVER )
		texture = &skin->textures.window_closebtn_hover;

	// Button idle
	else texture = &skin->textures.window_closebtn;

	skin_textured_draw_panel( skin->texture, texture, &button->bounds, &button->colour );
}
