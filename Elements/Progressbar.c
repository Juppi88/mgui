/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Progressbar.c
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI progress bar related functions.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#include "Progressbar.h"
#include "Skin.h"
#include "Platform/Alloc.h"

// --------------------------------------------------

// Progressbar callback handlers and static methods
static void		mgui_progressbar_render				( MGuiProgressbar* bar );
static void		mgui_progressbar_on_colour_change	( MGuiProgressbar* bar );
static void		mgui_progressbar_update_colours		( struct MGuiProgressbar* bar );

// --------------------------------------------------

static struct MGuiCallbacks callbacks =
{
	NULL, /* destroy */
	mgui_progressbar_render,
	NULL, /* post_render */
	NULL, /* process */
	NULL, /* get_clip_region */
	NULL, /* on_bounds_change */
	NULL, /* on_flags_change */
	mgui_progressbar_on_colour_change,
	NULL, /* on_text_change */
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

MGuiProgressbar* mgui_create_progressbar( MGuiElement* parent )
{
	struct MGuiProgressbar* bar;

	bar = mem_alloc_clean( sizeof(*bar) );
	bar->flags_int |= INTFLAG_NOTEXT;

	mgui_element_create( cast_elem(bar), parent );

	bar->flags |= FLAG_BACKGROUND|FLAG_BORDER;

	bar->type = GUI_PROGRESSBAR;
	bar->value = 1;
	bar->max_value = 1;
	bar->bg_shade = 0.5f;
	bar->thickness = 1;

	// Progressbar callbacks
	bar->callbacks = &callbacks;

	return cast_elem(bar);
}

MGuiProgressbar* mgui_create_progressbar_ex( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col1, uint32 col2, float max_value )
{
	MGuiProgressbar* bar;

	bar = mgui_create_progressbar( parent );

	mgui_set_abs_pos_i( bar, x, y );
	mgui_set_abs_size_i( bar, w, h );
	mgui_add_flags( bar, flags );
	mgui_progressbar_set_colour_i( bar, col1, col2 );
	mgui_progressbar_set_max_value( bar, max_value );
	
	return bar;
}

static void mgui_progressbar_render( MGuiProgressbar* bar )
{
	bar->skin->draw_progressbar( bar );
}

static void mgui_progressbar_on_colour_change( MGuiProgressbar* bar )
{
	struct MGuiProgressbar* progbar;
	uint8 alpha;

	progbar = (struct MGuiProgressbar*)bar;

	// Only apply new alpha here, progressbars have
	// separate methods for setting the actual colours.
	alpha = progbar->colour.a;

	progbar->colour_start.a = alpha;
	progbar->colour_end.a = alpha;
	progbar->colour_fg.a = alpha;
	progbar->colour_bg.a = alpha;

	mgui_element_request_redraw( bar );
}

static void mgui_progressbar_update_colours( struct MGuiProgressbar* bar )
{
	float percentage;

	if ( bar == NULL ) return;

	percentage = bar->value / bar->max_value;
	percentage = math_clampf( percentage, 0, 1 );

	colour_lerp_no_alpha( &bar->colour_fg, &bar->colour_start, &bar->colour_end, percentage );
	colour_multiply( &bar->colour_bg, &bar->colour_fg, bar->bg_shade );

	mgui_progressbar_on_colour_change( cast_elem(bar) );
}

float mgui_progressbar_get_value( MGuiProgressbar* bar )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ) return 0;

	progbar = (struct MGuiProgressbar*)bar;
	return progbar->value;
}

void mgui_progressbar_set_value( MGuiProgressbar* bar, float value )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;

	value = math_clampf( value, 0, progbar->max_value );
	progbar->value = value;

	mgui_progressbar_update_colours( progbar );
}

float mgui_progressbar_get_max_value( MGuiProgressbar* bar )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ) return 0;

	progbar = (struct MGuiProgressbar*)bar;
	return progbar->max_value;
}

void mgui_progressbar_set_max_value( MGuiProgressbar* bar, float value )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;
	progbar->max_value = value;

	progbar->value = math_clampf( progbar->value, 0, value );

	mgui_progressbar_update_colours( progbar );
}

void mgui_progressbar_get_colour( MGuiProgressbar* bar, colour_t* col_start, colour_t* col_end )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ||
		 col_start == NULL ||
		 col_end == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;

	*col_start = progbar->colour_start;
	*col_end = progbar->colour_end;
}

void mgui_progressbar_set_colour( MGuiProgressbar* bar, const colour_t* col_start, const colour_t* col_end )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ||
		 col_start == NULL ||
		 col_end == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;

	progbar->colour_start = *col_start;
	progbar->colour_end = *col_end;

	mgui_progressbar_update_colours( progbar );
}

void mgui_progressbar_get_colour_i( MGuiProgressbar* bar, uint32* col_start, uint32* col_end )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ||
		 col_start == NULL ||
		 col_end == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;

	*col_start = progbar->colour_start.hex;
	*col_end = progbar->colour_end.hex;
}

void mgui_progressbar_set_colour_i( MGuiProgressbar* bar, uint32 col_start, uint32 col_end )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;

	progbar->colour_start.hex = col_start;
	progbar->colour_end.hex = col_end;

	mgui_progressbar_update_colours( progbar );
}

float mgui_progressbar_get_bg_shade( MGuiProgressbar* bar )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ) return 0;

	progbar = (struct MGuiProgressbar*)bar;
	return progbar->bg_shade;
}

void mgui_progressbar_set_bg_shade( MGuiProgressbar* bar, float shade )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;

	shade = math_clampf( shade, 0, 1 );
	progbar->bg_shade = shade;

	mgui_progressbar_update_colours( progbar );
}

uint8 mgui_progressbar_get_thickness( MGuiProgressbar* bar )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ) return 0;

	progbar = (struct MGuiProgressbar*)bar;
	return progbar->thickness;
}

void mgui_progressbar_set_thickness( MGuiProgressbar* bar, uint8 thickness )
{
	struct MGuiProgressbar* progbar;

	if ( bar == NULL ) return;

	progbar = (struct MGuiProgressbar*)bar;
	progbar->thickness = thickness;

	mgui_element_request_redraw( bar );
}
