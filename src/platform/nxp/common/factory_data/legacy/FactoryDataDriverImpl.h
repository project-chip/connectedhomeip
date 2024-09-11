/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <platform/nxp/common/factory_data/legacy/FactoryDataDriver.h>

extern "C" {
#include "HWParameter.h"
}

namespace chip {
namespace DeviceLayer {

/**
 * This class implements the FactoryDataDriver
 */

class FactoryDataDriverImpl : public FactoryDataDriver
{
public:
    static FactoryDataDriverImpl sInstance;

    FactoryDataDriverImpl()  = default;
    ~FactoryDataDriverImpl() = default;

    bool DoesBackupExist(uint16_t *) override;
    CHIP_ERROR Init() override;
    CHIP_ERROR DeleteBackup(void) override;
    CHIP_ERROR InitRamBackup(void) override;
    CHIP_ERROR ClearRamBackup(void) override;
    CHIP_ERROR ReadBackupInRam(void) override;
    CHIP_ERROR BackupFactoryData(void) override;
    CHIP_ERROR UpdateFactoryData(void) override;

    static StorageKeyName GetFactoryBackupKey() { return StorageKeyName::FromConst("nxp/fact-bkp"); }

private:
    extendedAppFactoryData_t * mFactoryData = nullptr;
};

} // namespace DeviceLayer
} // namespace chip
