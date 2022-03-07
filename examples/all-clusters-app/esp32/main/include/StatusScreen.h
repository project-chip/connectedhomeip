/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
