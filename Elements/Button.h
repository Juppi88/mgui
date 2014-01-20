/**
 *
 * @file		Button.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI button related functions.
 *
 * @details		Functions and structures related to GUI buttons.
 *
 **/

#pragma once
#ifndef __MGUI_BUTTON_H
#define __MGUI_BUTTON_H

#include "Element.h"

/**
 * @brief GUI button.
 *
 * @details A button is an element that provides the user a simple way to
 * trigger an event by clicking the element.
 */
struct MGuiButton {
	MGuiElement;	///< Inherit MGuiElement members.
};

MGuiButton*		mgui_create_button		( MGuiElement* parent );
MGuiButton*		mgui_create_button_ex	( MGuiElement* parent, int16 x, int16 y, uint16 w, uint16 h, uint32 flags, uint32 col, const char_t* text );

#endif /* __MGUI_BUTTON_H */
