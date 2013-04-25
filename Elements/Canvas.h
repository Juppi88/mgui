/**********************************************************************
 *
 * PROJECT:		Mylly GUI
 * FILE:		Canvas.h
 * LICENCE:		See Licence.txt
 * PURPOSE:		GUI canvas related functions. Canvas is an invisible
 *				element that can be used as a parent for everything else.
 *
 *				(c) Tuomo Jauhiainen 2012-13
 *
 **********************************************************************/

#pragma once
#ifndef __MGUI_CANVAS_H
#define __MGUI_CANVAS_H

#include "Element.h"

struct MGuiCanvas
{
	MGuiElement;	// Inherit all generic element properties
};

MGuiCanvas*		mgui_create_canvas		( MGuiElement* parent );

#endif /* __MGUI_CANVAS_H */
