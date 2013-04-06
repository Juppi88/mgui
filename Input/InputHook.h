/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		InputHook.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI input handler.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_INPUTHOOK_H
#define __MYLLY_GUI_INPUTHOOK_H

#include "MGUI/MGUI.h"
#include "Input/Input.h"

void			mgui_input_initialize_hooks		( void );
void			mgui_input_shutdown_hooks		( void );
void			mgui_input_cleanup_references	( MGuiElement* element );

static bool		mgui_input_handle_char			( input_event_t* event );
static bool		mgui_input_handle_key_up		( input_event_t* event );
static bool		mgui_input_handle_key_down		( input_event_t* event );
static bool		mgui_input_handle_mouse_move	( input_event_t* event );
static bool		mgui_input_handle_mouse_wheel	( input_event_t* event );
static bool		mgui_input_handle_lmb_up		( input_event_t* event );
static bool		mgui_input_handle_lmb_down		( input_event_t* event );

#endif /* __MYLLY_GUI_INPUTHOOK_H */
