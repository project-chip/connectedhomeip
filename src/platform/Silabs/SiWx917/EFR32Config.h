/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          platforms based on the Silicon Labs SDK.
 */

#pragma once

#include <functional>

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "nvm3.h"
#include "nvm3_hal_flash.h"

#ifndef KVS_MAX_ENTRIES
#define KVS_MAX_ENTRIES 75 // Available key slot count for Kvs Key mapping.
#endif

// Delay before Key/Value is actually saved in NVM
#define EFR32_KVS_SAVE_DELAY_SECONDS 5

static_assert((KVS_MAX_ENTRIES <= 255), "Implementation supports up to 255 Kvs entries");
static_assert((KVS_MAX_ENTRIES >= 30), "Mininimal Kvs entries requirement is not met");

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *
 * This implementation uses the Silicon Labs EFR32 NVM3 flash data storage library
 * as the underlying storage layer.
 *
 * NOTE: This class is designed to be mixed-in to the concrete subclass of the
 * GenericConfigurationManagerImpl<> template.  When used this way, the class
 * naturally provides implementations for the delegated members referenced by
 * the template class (e.g. the ReadConfigValue() method).
 */

// Silabs NVM3 objects use a 20-bit number,
// NVM3 Key 19:16 Stack region
// NVM3 Key 15:0 Available NVM3 keys 0x0000 -> 0xFFFF.
// Matter stack reserved region ranges from 0x087200 to 0x087FFF
// e.g. key = 0x087201
// '08' = Matter nvm3 region
// '72' = the sub region group base offset (Factory, Config, Counter or KVS)
// '01' = the id offset inside the group.
constexpr uint32_t kMatterNvm3KeyDomain  = 0x080000U;
constexpr uint32_t kMatterNvm3KeyLoLimit = 0x087200U; // Do not modify without Silabs GSDK team approval
constexpr uint32_t kMatterNvm3KeyHiLimit = 0x087FFFU; // Do not modify without Silabs GSDK team approval
constexpr inline uint32_t EFR32ConfigKey(uint8_t keyBaseOffset, uint8_t id)
{
    return kMatterNvm3KeyDomain | static_cast<uint32_t>(keyBaseOffset) << 8 | id;
}

class EFR32Config
{
public:
    // Definitions for Silicon Labs EFR32 NVM3 driver:-

    using Key = uint32_t;

    // NVM3 key base offsets used by the CHIP Device Layer.
    // ** Key base can range from 0x72 to 0x7F **
    // Persistent config values set at manufacturing time. Retained during factory reset.
    static constexpr uint8_t kMatterFactory_KeyBase = 0x72;
    // Persistent config values set at runtime. Cleared during factory reset.
    static constexpr uint8_t kMatterConfig_KeyBase = 0x73;
    // Persistent counter values set at runtime. Retained during factory reset.
    static constexpr uint8_t kMatterCounter_KeyBase = 0x74;
    // Persistent config values set at runtime. Cleared during factory reset.
    static constexpr uint8_t kMatterKvs_KeyBase = 0x75;

    // Key definitions for well-known configuration values.
    // Factory config keys
    static constexpr Key kConfigKey_SerialNum             = EFR32ConfigKey(kMatterFactory_KeyBase, 0x00);
    static constexpr Key kConfigKey_MfrDeviceId           = EFR32ConfigKey(kMatterFactory_KeyBase, 0x01);
    static constexpr Key kConfigKey_MfrDeviceCert         = EFR32ConfigKey(kMatterFactory_KeyBase, 0x02);
    static constexpr Key kConfigKey_MfrDevicePrivateKey   = EFR32ConfigKey(kMatterFactory_KeyBase, 0x03);
    static constexpr Key kConfigKey_ManufacturingDate     = EFR32ConfigKey(kMatterFactory_KeyBase, 0x04);
    static constexpr Key kConfigKey_SetupPayloadBitSet    = EFR32ConfigKey(kMatterFactory_KeyBase, 0x05);
    static constexpr Key kConfigKey_MfrDeviceICACerts     = EFR32ConfigKey(kMatterFactory_KeyBase, 0x06);
    static constexpr Key kConfigKey_SetupDiscriminator    = EFR32ConfigKey(kMatterFactory_KeyBase, 0x07);
    static constexpr Key kConfigKey_Spake2pIterationCount = EFR32ConfigKey(kMatterFactory_KeyBase, 0x08);
    static constexpr Key kConfigKey_Spake2pSalt           = EFR32ConfigKey(kMatterFactory_KeyBase, 0x09);
    static constexpr Key kConfigKey_Spake2pVerifier       = EFR32ConfigKey(kMatterFactory_KeyBase, 0x0A);
    static constexpr Key kConfigKey_ProductId             = EFR32ConfigKey(kMatterFactory_KeyBase, 0x0B);
    static constexpr Key kConfigKey_VendorId              = EFR32ConfigKey(kMatterFactory_KeyBase, 0x0C);
    static constexpr Key kConfigKey_VendorName            = EFR32ConfigKey(kMatterFactory_KeyBase, 0x0D);
    static constexpr Key kConfigKey_ProductName           = EFR32ConfigKey(kMatterFactory_KeyBase, 0x0E);
    static constexpr Key kConfigKey_HardwareVersionString = EFR32ConfigKey(kMatterFactory_KeyBase, 0x0F);
    static constexpr Key kConfigKey_UniqueId              = EFR32ConfigKey(kMatterFactory_KeyBase, 0x1F);
    // Matter Config Keys
    static constexpr Key kConfigKey_ServiceConfig      = EFR32ConfigKey(kMatterConfig_KeyBase, 0x01);
    static constexpr Key kConfigKey_PairedAccountId    = EFR32ConfigKey(kMatterConfig_KeyBase, 0x02);
    static constexpr Key kConfigKey_ServiceId          = EFR32ConfigKey(kMatterConfig_KeyBase, 0x03);
    static constexpr Key kConfigKey_LastUsedEpochKeyId = EFR32ConfigKey(kMatterConfig_KeyBase, 0x05);
    static constexpr Key kConfigKey_FailSafeArmed      = EFR32ConfigKey(kMatterConfig_KeyBase, 0x06);
    static constexpr Key kConfigKey_GroupKey           = EFR32ConfigKey(kMatterConfig_KeyBase, 0x07);
    static constexpr Key kConfigKey_HardwareVersion    = EFR32ConfigKey(kMatterConfig_KeyBase, 0x08);
    static constexpr Key kConfigKey_RegulatoryLocation = EFR32ConfigKey(kMatterConfig_KeyBase, 0x09);
    static constexpr Key kConfigKey_CountryCode        = EFR32ConfigKey(kMatterConfig_KeyBase, 0x0A);
    static constexpr Key kConfigKey_WiFiSSID           = EFR32ConfigKey(kMatterConfig_KeyBase, 0x0C);
    static constexpr Key kConfigKey_WiFiPSK            = EFR32ConfigKey(kMatterConfig_KeyBase, 0x0D);
    static constexpr Key kConfigKey_WiFiSEC            = EFR32ConfigKey(kMatterConfig_KeyBase, 0x0E);
    static constexpr Key kConfigKey_GroupKeyBase       = EFR32ConfigKey(kMatterConfig_KeyBase, 0x0F);
    static constexpr Key kConfigKey_LockUser           = EFR32ConfigKey(kMatterConfig_KeyBase, 0x10);
    static constexpr Key kConfigKey_Credential         = EFR32ConfigKey(kMatterConfig_KeyBase, 0x11);
    static constexpr Key kConfigKey_LockUserName       = EFR32ConfigKey(kMatterConfig_KeyBase, 0x12);
    static constexpr Key kConfigKey_CredentialData     = EFR32ConfigKey(kMatterConfig_KeyBase, 0x13);
    static constexpr Key kConfigKey_UserCredentials    = EFR32ConfigKey(kMatterConfig_KeyBase, 0x14);
    static constexpr Key kConfigKey_WeekDaySchedules   = EFR32ConfigKey(kMatterConfig_KeyBase, 0x15);
    static constexpr Key kConfigKey_YearDaySchedules   = EFR32ConfigKey(kMatterConfig_KeyBase, 0x16);
    static constexpr Key kConfigKey_HolidaySchedules   = EFR32ConfigKey(kMatterConfig_KeyBase, 0x17);
    static constexpr Key kConfigKey_OpKeyMap           = EFR32ConfigKey(kMatterConfig_KeyBase, 0x20);

    static constexpr Key kConfigKey_GroupKeyMax =
        EFR32ConfigKey(kMatterConfig_KeyBase, 0x1E); // Allows 16 Group Keys to be created.

    // Matter Counter Keys
    static constexpr Key kConfigKey_BootCount             = EFR32ConfigKey(kMatterCounter_KeyBase, 0x00);
    static constexpr Key kConfigKey_TotalOperationalHours = EFR32ConfigKey(kMatterCounter_KeyBase, 0x01);
    static constexpr Key kConfigKey_LifeTimeCounter       = EFR32ConfigKey(kMatterCounter_KeyBase, 0x02);

    // Matter KVS storage Keys
    static constexpr Key kConfigKey_KvsStringKeyMap = EFR32ConfigKey(kMatterKvs_KeyBase, 0x00);
    static constexpr Key kConfigKey_KvsFirstKeySlot = EFR32ConfigKey(kMatterKvs_KeyBase, 0x01);
    static constexpr Key kConfigKey_KvsLastKeySlot  = EFR32ConfigKey(kMatterKvs_KeyBase, KVS_MAX_ENTRIES);

    // Set key id limits for each group.
    static constexpr Key kMinConfigKey_MatterFactory = EFR32ConfigKey(kMatterFactory_KeyBase, 0x00);
    static constexpr Key kMaxConfigKey_MatterFactory = EFR32ConfigKey(kMatterFactory_KeyBase, 0x1F);
    static constexpr Key kMinConfigKey_MatterConfig  = EFR32ConfigKey(kMatterConfig_KeyBase, 0x00);
    static constexpr Key kMaxConfigKey_MatterConfig  = EFR32ConfigKey(kMatterConfig_KeyBase, 0x20);

    // Allows 32 Counters to be created.
    static constexpr Key kMinConfigKey_MatterCounter = EFR32ConfigKey(kMatterCounter_KeyBase, 0x00);
    static constexpr Key kMaxConfigKey_MatterCounter = EFR32ConfigKey(kMatterCounter_KeyBase, 0x1F);

    static constexpr Key kMinConfigKey_MatterKvs = EFR32ConfigKey(kMatterKvs_KeyBase, 0x00);
    static constexpr Key kMaxConfigKey_MatterKvs = EFR32ConfigKey(kMatterKvs_KeyBase, 0xFF);

    static CHIP_ERROR Init(void);
    static void DeInit(void);

    // Configuration methods used by the GenericConfigurationManagerImpl<> template.
    static CHIP_ERROR ReadConfigValue(Key key, bool & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);
    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen, size_t offset);
    static CHIP_ERROR ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val);
    static CHIP_ERROR WriteConfigValue(Key key, bool val);
    static CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
    static CHIP_ERROR WriteConfigValue(Key key, uint64_t val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR WriteConfigValueCounter(uint8_t counterIdx, uint32_t val);
    static CHIP_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);
    static bool ConfigValueExists(Key key, size_t & dataLen);
    static CHIP_ERROR FactoryResetConfig(void);
    static bool ValidConfigKey(Key key);

    static void RunConfigUnitTest(void);
    static void RepackNvm3Flash(void);

protected:
    using ForEachRecordFunct = std::function<CHIP_ERROR(const Key & nvm3Key, const size_t & length)>;
    static CHIP_ERROR ForEachRecord(Key firstKey, Key lastKey, bool addNewRecord, ForEachRecordFunct funct);

private:
    static CHIP_ERROR MapNvm3Error(Ecode_t nvm3Res);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
