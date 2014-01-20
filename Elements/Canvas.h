/**
 *
 * @file		Canvas.h
 * @copyright	Tuomo Jauhiainen 2012-2014
 * @licence		See Licence.txt
 * @brief		GUI canvas related functions.
 *
 * @details		Functions and structures related to GUI canvases.
 *
 **/

#pragma once
#ifndef __MGUI_CANVAS_H
#define __MGUI_CANVAS_H

#include "Element.h"

/**
 * @brief GUI canvas.
 *
 * @details This is an invisible element that can be used
 * as a parent for other elements.
 */
struct MGuiCanvas {
	MGuiElement;	///< Inherit MGuiElement members.
};

MGuiCanvas*	mgui_create_canvas( MGuiElement* parent );

#endif /* __MGUI_CANVAS_H */
