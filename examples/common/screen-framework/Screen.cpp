/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file Screen.cpp
 *
 * Simple screen.
 *
 */

#include "Screen.h"

#if CONFIG_HAVE_DISPLAY

namespace {

const char * buttonText[] = { "Previous", "Next", "Action" };

};

std::string Screen::GetButtonText(int id)
{
    return buttonText[id - 1];
}

#endif // CONFIG_HAVE_DISPLAY
