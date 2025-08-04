/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file QRCodeScreen.h
 *
 * Screen which displays a QR code.
 *
 */

#pragma once

#include "Screen.h"
#include "ScreenManager.h"

#if CONFIG_HAVE_DISPLAY

#include <cstdint>
#include <string>
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
