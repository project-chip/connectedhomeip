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
#include "CommonAppDelegate.h"
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace chip {
namespace NXP {
namespace App {

void CommonAppDelegate::OnCommissioningWindowOpened()
{
    ChipLogProgress(AppServer, "Commissioning window opened");
    // Wi-Fi PAF publish is now handled automatically by
    // CommissioningWindowManager::StartAdvertisement() via
    // ConnectivityMgr().SetWiFiPAFAdvertisingEnabled(true, ...).
    // No manual publish needed here.
}

// Default implementation - singleton instance of the CommonAppDelegate
static CommonAppDelegate sCommonAppDelegate;

::AppDelegate & GetAppDelegate()
{
    return sCommonAppDelegate;
}

} // namespace App
} // namespace NXP
} // namespace chip
