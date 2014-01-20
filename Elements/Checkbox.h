/**
 *
 * @file		Checkbox.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI checkbox related functions.
 *
 * @details		Functions and structures related to GUI checkboxes.
 *
 **/

#pragma once
#ifndef __MGUI_CHECKBOX_H
#define __MGUI_CHECKBOX_H

#include "Element.h"

/**
 * @brief GUI checkbox.
 * @details Checkbox is a data input element that can be toggled by clicking it.
 */
struct MGuiCheckbox {
	MGuiElement;	///< Inherit MGuiElement members.
};

MGuiCheckbox*	mgui_create_checkbox	( MGuiElement* parent );
MGuiCheckbox*	mgui_create_checkbox_ex	( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col );

#endif /* __MGUI_CHECKBOX_H */
