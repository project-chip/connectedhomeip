/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *    @file
 *          Utilities for accessing persisted device configuration on
 *          on STM32 platforms.
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include "flash_wb.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

/* Base for the category calculation when determining the key IDs */

class STM32Config {
public:
    using Key = uint32_t;

    // Key definitions for well-known keys.
    // Factory config keys
    static constexpr Key kConfigKey_SerialNum = 0;
    static constexpr Key kConfigKey_MfrDeviceId = 1;
    static constexpr Key kConfigKey_MfrDeviceCert = 2;
    static constexpr Key kConfigKey_MfrDevicePrivateKey = 3;
    static constexpr Key kConfigKey_ManufacturingDate = 4;
    static constexpr Key kConfigKey_SetupPinCode = 5;
    static constexpr Key kConfigKey_MfrDeviceICACerts = 6;
    static constexpr Key kConfigKey_SetupDiscriminator = 7;
    static constexpr Key kConfigKey_Spake2pIterationCount = 8;
    static constexpr Key kConfigKey_Spake2pSalt = 9;
    static constexpr Key kConfigKey_Spake2pVerifier = 10;
    // CHIP Config Keys
    static constexpr Key kConfigKey_FabricId = 30;
    static constexpr Key kConfigKey_ServiceConfig = 31;
    static constexpr Key kConfigKey_PairedAccountId = 32;
    static constexpr Key kConfigKey_ServiceId = 33;
    static constexpr Key kConfigKey_FabricSecret = 34;
    static constexpr Key kConfigKey_LastUsedEpochKeyId = 35;
    static constexpr Key kConfigKey_FailSafeArmed = 36;
    static constexpr Key kConfigKey_GroupKey = 37;
    static constexpr Key kConfigKey_HardwareVersion = 38;
    static constexpr Key kConfigKey_RegulatoryLocation = 39;
    static constexpr Key kConfigKey_CountryCode = 40;
    static constexpr Key kConfigKey_HourFormat = 41;
    static constexpr Key kConfigKey_CalendarType = 42;
    static constexpr Key kConfigKey_Breadcrumb = 43;
    static constexpr Key kConfigKey_UniqueId = 44;
    static constexpr Key kConfigKey_ProductRevision = 45;
    // Counter keys
    static const Key kCounterKey_RebootCount = 60;
    static const Key kCounterKey_UpTime = 61;
    static const Key kCounterKey_TotalOperationalHours = 62;
    static const Key kCounterKey_BootReason = 63;
    // Set key id limits for each group.
    static constexpr Key kConfigKey_Base = kConfigKey_SerialNum;
    static constexpr Key kConfigKey_Max = kConfigKey_UniqueId;

    static CHIP_ERROR Init();

    // Config value accessors.
    static CHIP_ERROR ReadConfigValue(Key key, bool &val);
    static CHIP_ERROR ReadConfigValue(Key key, uint32_t &val);
    static CHIP_ERROR ReadConfigValue(Key key, uint64_t &val);
    static CHIP_ERROR ReadConfigValueStr(Key key, char *buf, size_t bufSize, size_t &outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t *buf, size_t bufSize, size_t &outLen);
    static CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char *str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char *str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t *data, size_t dataLen);
    static CHIP_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);
    static CHIP_ERROR FactoryResetConfig(void);
    static void RunConfigUnitTest(void);
    static CHIP_ERROR PrintError(NVM_StatusTypeDef err);

};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
