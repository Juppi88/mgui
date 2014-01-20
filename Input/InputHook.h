/**
 *
 * @file		InputHook.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		MGUI input handler.
 *
 * @details		User input hooking and processing.
 *
 **/

#pragma once
#ifndef __MYLLY_GUI_INPUTHOOK_H
#define __MYLLY_GUI_INPUTHOOK_H

#include "MGUI/MGUI.h"
#include "Input/Input.h"

void			mgui_input_initialize_hooks		( void );
void			mgui_input_shutdown_hooks		( void );
void			mgui_input_cleanup_references	( MGuiElement* element );

#endif /* __MYLLY_GUI_INPUTHOOK_H */
