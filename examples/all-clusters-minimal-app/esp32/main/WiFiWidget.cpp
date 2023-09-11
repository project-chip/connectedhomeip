/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file WiFiWidget.cpp
 *
 * Implements a WiFi Widget controller that display the state of bluetooth
 * connection on display.
 */

#include "WiFiWidget.h"

#include "esp_log.h"
#include "esp_system.h"
#if CONFIG_HAVE_DISPLAY
#include "ScreenManager.h"
#endif
extern const char * TAG;

void WiFiWidget::Init()
{
#if CONFIG_HAVE_DISPLAY
    mVLED = -1;
#endif // CONFIG_HAVE_DISPLAY

    mState = false;
}

void WiFiWidget::Set(bool state)
{
    bool stateChange = (mState != state);
    mState           = state;
    if (stateChange)
    {
#if CONFIG_HAVE_DISPLAY
        if (mVLED != -1)
        {
            ScreenManager::SetVLED(mVLED, mState);
        }
#endif // CONFIG_HAVE_DISPLAY
    }
}

#if CONFIG_HAVE_DISPLAY
void WiFiWidget::SetVLED(int id)
{
    mVLED = id;
    if (mVLED != -1)
    {
        ScreenManager::SetVLED(mVLED, mState);
    }
}
#endif // CONFIG_HAVE_DISPLAY
