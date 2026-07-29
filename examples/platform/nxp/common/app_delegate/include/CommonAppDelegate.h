/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <app/server/AppDelegate.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace NXP {
namespace App {

class CommonAppDelegate : public ::AppDelegate
{
public:
    virtual void OnCommissioningWindowOpened() override;
};

/**
 * Returns the application-specific implementation of the CommonAppDelegate object.
 */
extern ::AppDelegate & GetAppDelegate();

} // namespace App
} // namespace NXP
} // namespace chip
