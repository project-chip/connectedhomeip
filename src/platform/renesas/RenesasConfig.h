/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *    @file
 *          Utilities for accessing persisted device configuration on
 *          CYW30739 platforms.
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>


namespace chip {
namespace DeviceLayer {
namespace Internal {

#define PLATFORM_NVRAM_SSID_MATTER_BASE 0
#define PLATFORM_NVRAM_VSID_MATTER_BASE 0x100

constexpr inline uint32_t RenesasConfigKey(uint16_t keyBaseOffset, uint8_t id)
{
    return keyBaseOffset | id;
}

/**
 * Provides functions and definitions for accessing device configuration information.
 */
class RenesasConfig
{
public:
    using Key = uint32_t;

    static constexpr uint16_t kChipFactory_KeyBase  = PLATFORM_NVRAM_SSID_MATTER_BASE;
    static constexpr uint16_t kChipConfig_KeyBase   = PLATFORM_NVRAM_VSID_MATTER_BASE + 0x0000;
    static constexpr uint16_t kChipKvsValue_KeyBase = PLATFORM_NVRAM_VSID_MATTER_BASE + 0x0100;

    // Key definitions for well-known keys.
    static constexpr uint16_t kChipKvsKey_KeyBase   = PLATFORM_NVRAM_VSID_MATTER_BASE + 0x0200;

    // Key definitions for well-known keys.
    // Factory config keys
    static constexpr Key kConfigKey_SerialNum             = RenesasConfigKey(kChipFactory_KeyBase, 0x00);
    static constexpr Key kConfigKey_MfrDeviceId           = RenesasConfigKey(kChipFactory_KeyBase, 0x01);
    static constexpr Key kConfigKey_MfrDeviceCert         = RenesasConfigKey(kChipFactory_KeyBase, 0x02);
    static constexpr Key kConfigKey_MfrDevicePrivateKey   = RenesasConfigKey(kChipFactory_KeyBase, 0x03);
    static constexpr Key kConfigKey_ManufacturingDate     = RenesasConfigKey(kChipFactory_KeyBase, 0x04);
    static constexpr Key kConfigKey_SetupPinCode          = RenesasConfigKey(kChipFactory_KeyBase, 0x05);
    static constexpr Key kConfigKey_MfrDeviceICACerts     = RenesasConfigKey(kChipFactory_KeyBase, 0x06);
    static constexpr Key kConfigKey_SetupDiscriminator    = RenesasConfigKey(kChipFactory_KeyBase, 0x07);
    static constexpr Key kConfigKey_Spake2pIterationCount = RenesasConfigKey(kChipFactory_KeyBase, 0x08);
    static constexpr Key kConfigKey_Spake2pSalt           = RenesasConfigKey(kChipFactory_KeyBase, 0x09);
    static constexpr Key kConfigKey_Spake2pVerifier       = RenesasConfigKey(kChipFactory_KeyBase, 0x0a);
    static constexpr Key kConfigKey_DAC                   = RenesasConfigKey(kChipFactory_KeyBase, 0x0b);
    static constexpr Key kConfigKey_DACKey                = RenesasConfigKey(kChipFactory_KeyBase, 0x0c);
    static constexpr Key kConfigKey_PAICert               = RenesasConfigKey(kChipFactory_KeyBase, 0x0d);
    static constexpr Key kConfigKey_CertDeclaration       = RenesasConfigKey(kChipFactory_KeyBase, 0x0e);
    // CHIP Config Keys
    static constexpr Key kConfigKey_ServiceConfig      = RenesasConfigKey(kChipConfig_KeyBase, 0x00);
    static constexpr Key kConfigKey_PairedAccountId    = RenesasConfigKey(kChipConfig_KeyBase, 0x01);
    static constexpr Key kConfigKey_ServiceId          = RenesasConfigKey(kChipConfig_KeyBase, 0x02);
    static constexpr Key kConfigKey_LastUsedEpochKeyId = RenesasConfigKey(kChipConfig_KeyBase, 0x03);
    static constexpr Key kConfigKey_FailSafeArmed      = RenesasConfigKey(kChipConfig_KeyBase, 0x04);
    static constexpr Key kConfigKey_GroupKey           = RenesasConfigKey(kChipConfig_KeyBase, 0x05);
    static constexpr Key kConfigKey_HardwareVersion    = RenesasConfigKey(kChipConfig_KeyBase, 0x06);
    static constexpr Key kConfigKey_RegulatoryLocation = RenesasConfigKey(kChipConfig_KeyBase, 0x07);
    static constexpr Key kConfigKey_CountryCode        = RenesasConfigKey(kChipConfig_KeyBase, 0x08);
    static constexpr Key kConfigKey_RebootCount        = RenesasConfigKey(kChipConfig_KeyBase, 0x09);
    static constexpr Key kConfigKey_UniqueId           = RenesasConfigKey(kChipConfig_KeyBase, 0x0a);
    static constexpr Key kConfigKey_LockUser           = RenesasConfigKey(kChipConfig_KeyBase, 0x0b);
    static constexpr Key kConfigKey_Credential         = RenesasConfigKey(kChipConfig_KeyBase, 0x0c);
    static constexpr Key kConfigKey_LockUserName       = RenesasConfigKey(kChipConfig_KeyBase, 0x0d);
    static constexpr Key kConfigKey_CredentialData     = RenesasConfigKey(kChipConfig_KeyBase, 0x0e);
    static constexpr Key kConfigKey_UserCredentials    = RenesasConfigKey(kChipConfig_KeyBase, 0x0f);
    static constexpr Key kConfigKey_WeekDaySchedules   = RenesasConfigKey(kChipConfig_KeyBase, 0x10);
    static constexpr Key kConfigKey_YearDaySchedules   = RenesasConfigKey(kChipConfig_KeyBase, 0x11);
    static constexpr Key kConfigKey_HolidaySchedules   = RenesasConfigKey(kChipConfig_KeyBase, 0x12);
    static constexpr Key kConfigKey_BootReason         = RenesasConfigKey(kChipConfig_KeyBase, 0x13);

    // Set key id limits for each group.
    static constexpr Key kMinConfigKey_ChipFactory = RenesasConfigKey(kChipFactory_KeyBase, 0x00);
    static constexpr Key kMaxConfigKey_ChipFactory = RenesasConfigKey(kChipFactory_KeyBase, 0xff);
    static constexpr Key kMinConfigKey_ChipConfig  = RenesasConfigKey(kChipConfig_KeyBase, 0x00);
    static constexpr Key kMaxConfigKey_ChipConfig  = RenesasConfigKey(kChipConfig_KeyBase, 0xff);

    static CHIP_ERROR Init(void);

    // Config value accessors.
    template <typename T>
    static CHIP_ERROR ReadConfigValue(Key key, T & val);

    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, void * buf, size_t bufSize, size_t & outLen);
    template <typename T>
    static CHIP_ERROR WriteConfigValue(Key key, T val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const void * data, size_t dataLen);
    static CHIP_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);
    static CHIP_ERROR FactoryResetConfig(void);
    static void RunConfigUnitTest(void);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
