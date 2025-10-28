/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "Reboot.h"

#include <platform/KeyValueStoreManager.h>

#include <zephyr/settings/settings.h>
#include <zephyr/sys/reboot.h>

using namespace ::chip::DeviceLayer::PersistedStorage;

namespace chip {
namespace DeviceLayer {

// Define non-volatile storage key for RebootReason.
static constexpr char kRebootReason[] = "RebootReason";

void Reboot(SoftwareRebootReason reason)
{
    KeyValueStoreMgr().Put(kRebootReason, &reason, sizeof(reason));

    sys_reboot(SYS_REBOOT_WARM);
}

SoftwareRebootReason GetSoftwareRebootReason()
{
    SoftwareRebootReason reason = SoftwareRebootReason::kOther;

    settings_subsys_init();

    if (KeyValueStoreMgr().Get(kRebootReason, &reason, sizeof(reason)) == CHIP_NO_ERROR)
    {
        KeyValueStoreMgr().Delete(kRebootReason);
    }

    return reason;
}

} // namespace DeviceLayer
} // namespace chip
