/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file Screen.h
 *
 * Simple screen.
 *
 */

#pragma once

#include "Display.h"

#if CONFIG_HAVE_DISPLAY

#include "ScreenManager.h"

#include <string>

class Screen
{
public:
    enum FocusType
    {
        NONE,
        BLUR,
        UNBLUR,
        NEXT,
        PREVIOUS
    };

    virtual ~Screen() = default;

    virtual std::string GetTitle() { return "Untitled"; }

    virtual std::string GetButtonText(int id);

    virtual void Display() {}

    virtual void Enter(bool pushed) {}

    virtual void Exit(bool popped) {}

    virtual bool IsFocusable() { return false; }

    virtual void Focus(FocusType focus) {}

    virtual void Action() {}
};

#endif // CONFIG_HAVE_DISPLAY
