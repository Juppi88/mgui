/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		InputHook.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI input handler.
 *
 *				(c) Tuomo Jauhiainen 2012
 *
 **********************************************************************/

#pragma once
#ifndef __MYLLY_GUI_INPUTHOOK_H
#define __MYLLY_GUI_INPUTHOOK_H

#include "MGUI.h"

void	mgui_input_initialize_hooks		( void );
void	mgui_input_shutdown_hooks		( void );
void	mgui_input_cleanup_references	( element_t* element );

#endif /* __MYLLY_GUI_INPUTHOOK_H */
