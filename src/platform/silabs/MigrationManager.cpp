/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "MigrationManager.h"
#include <platform/CHIPDeviceLayer.h>
#include <platform/silabs/SilabsConfig.h>
#include <stdio.h>

using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::DeviceLayer::PersistedStorage;

namespace chip {
namespace DeviceLayer {
namespace Silabs {

namespace {
typedef void (*func_ptr)();
typedef struct
{
    uint32_t migrationGroup;
    func_ptr migrationFunc;
} migrationData_t;

#define COUNT_OF(A) (sizeof(A) / sizeof((A)[0]))
static migrationData_t migrationTable[] = {
    { .migrationGroup = 1, .migrationFunc = MigrateKvsMap },
    { .migrationGroup = 2, .migrationFunc = MigrateDacProvider },
    { .migrationGroup = 3, .migrationFunc = MigrateCounterConfigs },
    // add any additional migration neccesary. migrationGroup should stay equal if done in the same commit or increment by 1 for
    // each new entry.
};

} // namespace

void MigrationManager::applyMigrations()
{
    uint32_t lastMigationGroupDone = 0;
    SilabsConfig::ReadConfigValue(SilabsConfig::kConfigKey_MigrationCounter, lastMigationGroupDone);

    uint32_t completedMigrationGroup = lastMigationGroupDone;
    for (uint32_t i = 0; i < COUNT_OF(migrationTable); i++)
    {
        if (lastMigationGroupDone < migrationTable[i].migrationGroup)
        {
            (*migrationTable[i].migrationFunc)();
            completedMigrationGroup = max(migrationTable[i].migrationGroup, completedMigrationGroup);
        }
    }
    SilabsConfig::WriteConfigValue(SilabsConfig::kConfigKey_MigrationCounter, completedMigrationGroup);
}
void MigrationManager::MigrateUint32(uint32_t old_key, uint32_t new_key)
{
    uint32_t value = 0;
    if (SilabsConfig::ConfigValueExists(old_key) && (CHIP_NO_ERROR == SilabsConfig::ReadConfigValue(old_key, value)))
    {
        if (CHIP_NO_ERROR == SilabsConfig::WriteConfigValue(new_key, value))
        {
            // Free memory of old key location
            SilabsConfig::ClearConfigValue(old_key);
        }
    }
}

void MigrateCounterConfigs(void)
{
    constexpr uint32_t kOldConfigKey_BootCount             = SilabsConfigKey(SilabsConfig::kMatterCounter_KeyBase, 0x00);
    constexpr uint32_t kOldConfigKey_TotalOperationalHours = SilabsConfigKey(SilabsConfig::kMatterCounter_KeyBase, 0x01);

    MigrationManager::MigrateUint32(kOldConfigKey_BootCount, SilabsConfig::kConfigKey_BootCount);
    MigrationManager::MigrateUint32(kOldConfigKey_TotalOperationalHours, SilabsConfig::kConfigKey_TotalOperationalHours);
}

MigrationManager & MigrationManager::GetMigrationInstance()
{
    static MigrationManager sMigrationManager;
    return sMigrationManager;
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
