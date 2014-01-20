/**
 *
 * @file		Label.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI label related functions.
 *
 * @details		Functions and structures related to GUI labels.
 *
 **/

#pragma once
#ifndef __MGUI_LABEL_H
#define __MGUI_LABEL_H

#include "Element.h"

/**
 * @brief GUI label.
 * @details Label is a container for static text that can not be interacted with.
 */
struct MGuiLabel {
	MGuiElement;	///< Inherit MGuiElement members.
};

MGuiLabel*	mgui_create_label		( MGuiElement* parent );
MGuiLabel*	mgui_create_label_ex	( MGuiElement* parent, int16 x, int16 y, uint32 flags, uint32 col, const char_t* text );

void		mgui_label_make_text_fit ( MGuiLabel* label );

#endif /* __MGUI_LABEL_H */
