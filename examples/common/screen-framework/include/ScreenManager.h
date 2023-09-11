/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file ScreenManager.h
 *
 * Simple screen manager.
 *
 */

#pragma once

#include "Display.h"
#include "Screen.h"

#if CONFIG_HAVE_DISPLAY

extern uint16_t ScreenFontHeight;
extern uint16_t ScreenTitleSafeTop;
extern uint16_t ScreenTitleSafeBottom;

extern color_t ScreenNormalColor;
extern color_t ScreenFocusColor;

class Screen;

namespace ScreenManager {

void Init();

void Display();

void ButtonPressed(int id);

void PushScreen(Screen * screen);

void PopScreen();

void FocusBack();

int AddVLED(color_t color);

void SetVLED(int id, bool on);

void ToggleVLED(int id);

} // namespace ScreenManager

#endif // CONFIG_HAVE_DISPLAY
