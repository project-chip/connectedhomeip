/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file StatusScreen.h
 *
 * Screen which displays a QR code.
 *
 */

#pragma once

#include "ListScreen.h"
#include "ScreenManager.h"

#if CONFIG_HAVE_DISPLAY

#include <cstdint>
#include <vector>

class StatusScreen : public ListScreen
{

public:
    StatusScreen();

    virtual ~StatusScreen() {}
};

#endif // CONFIG_HAVE_DISPLAY
