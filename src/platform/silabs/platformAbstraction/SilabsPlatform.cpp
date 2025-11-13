/*
 *    Copyright (c) 2025 Project CHIP Authors
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

// Common platform methods go here

#include <lib/support/CodeUtils.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/silabs/MigrationManager.h>
#include <platform/silabs/SilabsConfig.h>
#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

CHIP_ERROR SilabsPlatform::VerifyIfUpdated()
{
    bool performedUpdate = false;
    CHIP_ERROR err =
        Internal::SilabsConfig::ReadConfigValue(Internal::SilabsConfig::kConfigKey_MatterUpdateReboot, performedUpdate);
    VerifyOrReturnLogError(CHIP_NO_ERROR == err || CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND == err, err);
    if (performedUpdate)
    {
        Internal::SilabsConfig::ClearConfigValue(Internal::SilabsConfig::kConfigKey_MatterUpdateReboot);
        mRebootCause = to_underlying(BootReasonType::kSoftwareUpdateCompleted);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR SilabsPlatform::NvmInit()
{
    ReturnErrorOnFailure(Internal::SilabsConfig::Init());
    Silabs::MigrationManager::GetMigrationInstance().ApplyMigrations();
    return CHIP_NO_ERROR;
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
