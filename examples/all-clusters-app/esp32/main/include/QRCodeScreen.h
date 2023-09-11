/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file QRCodeScreen.h
 *
 * Screen which displays a QR code.
 *
 */

#pragma once

#include "sdkconfig.h"
#if CONFIG_HAVE_DISPLAY
#include "Screen.h"
#include "ScreenManager.h"

#include <cstdint>
#include <vector>

class QRCodeScreen : public Screen
{
    std::vector<uint8_t> qrCode;
    std::string title;

public:
    QRCodeScreen(std::string text, std::string title = "QR Code");

    virtual std::string GetTitle() { return title; }

    virtual void Display();
};

#endif // CONFIG_HAVE_DISPLAY
