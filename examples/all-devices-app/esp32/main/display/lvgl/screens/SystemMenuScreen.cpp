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

#include "SystemMenuScreen.h"
#include "CommissioningCodesScreen.h"
#include "DeviceInfoScreen.h"
#include "DeviceOperationsScreen.h"
#include "NavigationStack.h"
#include "Ui.h"

namespace {

void OnQRClicked(lv_event_t *)
{
    NavigationStack::Push("QR Code", ShowCommissioningCodesScreen);
}

void OnStatusClicked(lv_event_t *)
{
    NavigationStack::Push("Status", ShowDeviceInfo);
}

void OnOperationsClicked(lv_event_t *)
{
    NavigationStack::Push("Operations", ShowDeviceOperations);
}

} // namespace

void ShowSystemMenu(lv_obj_t * parent)
{
    Ui::LayoutColumn(parent);

    Ui::CreateTile(parent, LV_SYMBOL_IMAGE, Ui::Accent(), "Commissioning", "QR code and manual pairing code", OnQRClicked);
    Ui::CreateTile(parent, LV_SYMBOL_EYE_OPEN, Ui::Success(), "Status & Diagnostics", "Fabrics, network, memory", OnStatusClicked);
    Ui::CreateTile(parent, LV_SYMBOL_POWER, Ui::Danger(), "Device Operations", "Restart, factory reset", OnOperationsClicked);
}
