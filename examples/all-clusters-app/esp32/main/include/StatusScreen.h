/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
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

#include <sdkconfig.h>

#if CONFIG_HAVE_DISPLAY
#include "ListScreen.h"
#include "ScreenManager.h"

#include <cstdint>
#include <vector>

class StatusScreen : public ListScreen
{

public:
    StatusScreen();

    virtual ~StatusScreen() {}
};

#endif // CONFIG_HAVE_DISPLAY
