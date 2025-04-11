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
#include <platform/CHIPDeviceError.h>

#include "nvm3.h"
#include "nvm3_hal_flash.h"

#ifndef KVS_MAX_ENTRIES
#define KVS_MAX_ENTRIES 255 // Available key slot count for Kvs Key mapping.
#endif

// Delay before Key/Value is actually saved in NVM
#ifndef SL_KVS_SAVE_DELAY_SECONDS
#define SL_KVS_SAVE_DELAY_SECONDS 2
#endif

static_assert((KVS_MAX_ENTRIES <= 511), "Implementation supports up to 511 Kvs entries");
static_assert((KVS_MAX_ENTRIES >= 30), "Mininimal Kvs entries requirement is not met");

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 *
 * This implementation uses the Silicon Labs Silabs NVM3 flash data storage library
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
inline constexpr uint32_t kUserNvm3KeyDomainLoLimit = 0x000000U; // User Domain NVM3 Key Range lower limit
inline constexpr uint32_t kUserNvm3KeyDomainHiLimit = 0x00FFFFU; // User Domain NVM3 Key Range Maximum limit
inline constexpr uint32_t kMatterNvm3KeyDomain      = 0x080000U;
inline constexpr uint32_t kMatterNvm3KeyLoLimit     = 0x087200U; // Do not modify without Silabs GSDK team approval
inline constexpr uint32_t kMatterNvm3KeyHiLimit     = 0x087FFFU; // Do not modify without Silabs GSDK team approval
constexpr inline uint32_t SilabsConfigKey(uint8_t keyBaseOffset, uint8_t id)
{
    return kMatterNvm3KeyDomain | static_cast<uint32_t>(keyBaseOffset) << 8 | id;
}

class SilabsConfig
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
    static constexpr uint8_t kMatterKvs_KeyBase       = 0x75;
    static constexpr uint8_t kMatterKvs_ExtendedRange = 0x76;

    // Key definitions for well-known configuration values.
    // Factory config keys
    static constexpr Key kConfigKey_SerialNum              = SilabsConfigKey(kMatterFactory_KeyBase, 0x00);
    static constexpr Key kConfigKey_MfrDeviceId            = SilabsConfigKey(kMatterFactory_KeyBase, 0x01);
    static constexpr Key kConfigKey_MfrDeviceCert          = SilabsConfigKey(kMatterFactory_KeyBase, 0x02);
    static constexpr Key kConfigKey_MfrDevicePrivateKey    = SilabsConfigKey(kMatterFactory_KeyBase, 0x03);
    static constexpr Key kConfigKey_ManufacturingDate      = SilabsConfigKey(kMatterFactory_KeyBase, 0x04);
    static constexpr Key kConfigKey_SetupPayloadBitSet     = SilabsConfigKey(kMatterFactory_KeyBase, 0x05);
    static constexpr Key kConfigKey_MfrDeviceICACerts      = SilabsConfigKey(kMatterFactory_KeyBase, 0x06);
    static constexpr Key kConfigKey_SetupDiscriminator     = SilabsConfigKey(kMatterFactory_KeyBase, 0x07);
    static constexpr Key kConfigKey_Spake2pIterationCount  = SilabsConfigKey(kMatterFactory_KeyBase, 0x08);
    static constexpr Key kConfigKey_Spake2pSalt            = SilabsConfigKey(kMatterFactory_KeyBase, 0x09);
    static constexpr Key kConfigKey_Spake2pVerifier        = SilabsConfigKey(kMatterFactory_KeyBase, 0x0A);
    static constexpr Key kConfigKey_ProductId              = SilabsConfigKey(kMatterFactory_KeyBase, 0x0B);
    static constexpr Key kConfigKey_VendorId               = SilabsConfigKey(kMatterFactory_KeyBase, 0x0C);
    static constexpr Key kConfigKey_VendorName             = SilabsConfigKey(kMatterFactory_KeyBase, 0x0D);
    static constexpr Key kConfigKey_ProductName            = SilabsConfigKey(kMatterFactory_KeyBase, 0x0E);
    static constexpr Key kConfigKey_HardwareVersionString  = SilabsConfigKey(kMatterFactory_KeyBase, 0x0F);
    static constexpr Key KConfigKey_ProductLabel           = SilabsConfigKey(kMatterFactory_KeyBase, 0x10);
    static constexpr Key kConfigKey_ProductURL             = SilabsConfigKey(kMatterFactory_KeyBase, 0x11);
    static constexpr Key kConfigKey_PartNumber             = SilabsConfigKey(kMatterFactory_KeyBase, 0x12);
    static constexpr Key kConfigKey_CACerts                = SilabsConfigKey(kMatterFactory_KeyBase, 0x13);
    static constexpr Key kConfigKey_DeviceCerts            = SilabsConfigKey(kMatterFactory_KeyBase, 0x14);
    static constexpr Key kConfigKey_hostname               = SilabsConfigKey(kMatterFactory_KeyBase, 0x15);
    static constexpr Key kConfigKey_clientid               = SilabsConfigKey(kMatterFactory_KeyBase, 0x16);
    static constexpr Key kConfigKey_Test_Event_Trigger_Key = SilabsConfigKey(kMatterFactory_KeyBase, 0x17);
    static constexpr Key kConfigKey_HardwareVersion        = SilabsConfigKey(kMatterFactory_KeyBase, 0x18);
    // kConfigKey_PersistentUniqueId is the inputkey in the generating of the Rotating Device ID
    // SHALL NOT be the same as the UniqueID attribute exposed in the Basic Information cluster.
    static constexpr Key kConfigKey_PersistentUniqueId = SilabsConfigKey(kMatterFactory_KeyBase, 0x1F);
    static constexpr Key kConfigKey_Creds_KeyId        = SilabsConfigKey(kMatterFactory_KeyBase, 0x20);
    static constexpr Key kConfigKey_Creds_Base_Addr    = SilabsConfigKey(kMatterFactory_KeyBase, 0x21);
    static constexpr Key kConfigKey_Creds_DAC_Offset   = SilabsConfigKey(kMatterFactory_KeyBase, 0x22);
    static constexpr Key kConfigKey_Creds_DAC_Size     = SilabsConfigKey(kMatterFactory_KeyBase, 0x23);
    static constexpr Key kConfigKey_Creds_PAI_Offset   = SilabsConfigKey(kMatterFactory_KeyBase, 0x24);
    static constexpr Key kConfigKey_Creds_PAI_Size     = SilabsConfigKey(kMatterFactory_KeyBase, 0x25);
    static constexpr Key kConfigKey_Creds_CD_Offset    = SilabsConfigKey(kMatterFactory_KeyBase, 0x26);
    static constexpr Key kConfigKey_Creds_CD_Size      = SilabsConfigKey(kMatterFactory_KeyBase, 0x27);
    static constexpr Key kConfigKey_Provision_Request  = SilabsConfigKey(kMatterFactory_KeyBase, 0x28);
    static constexpr Key kConfigKey_Provision_Version  = SilabsConfigKey(kMatterFactory_KeyBase, 0x29);
    static constexpr Key kOtaTlvEncryption_KeyId       = SilabsConfigKey(kMatterFactory_KeyBase, 0x30);

    // Matter Config Keys
    static constexpr Key kConfigKey_ServiceConfig      = SilabsConfigKey(kMatterConfig_KeyBase, 0x01);
    static constexpr Key kConfigKey_PairedAccountId    = SilabsConfigKey(kMatterConfig_KeyBase, 0x02);
    static constexpr Key kConfigKey_ServiceId          = SilabsConfigKey(kMatterConfig_KeyBase, 0x03);
    static constexpr Key kConfigKey_LastUsedEpochKeyId = SilabsConfigKey(kMatterConfig_KeyBase, 0x05);
    static constexpr Key kConfigKey_FailSafeArmed      = SilabsConfigKey(kMatterConfig_KeyBase, 0x06);
    static constexpr Key kConfigKey_GroupKey           = SilabsConfigKey(kMatterConfig_KeyBase, 0x07);
    static constexpr Key kConfigKey_RegulatoryLocation = SilabsConfigKey(kMatterConfig_KeyBase, 0x09);
    static constexpr Key kConfigKey_CountryCode        = SilabsConfigKey(kMatterConfig_KeyBase, 0x0A);
    static constexpr Key kConfigKey_WiFiSSID           = SilabsConfigKey(kMatterConfig_KeyBase, 0x0C);
    static constexpr Key kConfigKey_WiFiPSK            = SilabsConfigKey(kMatterConfig_KeyBase, 0x0D);
    static constexpr Key kConfigKey_WiFiSEC            = SilabsConfigKey(kMatterConfig_KeyBase, 0x0E);
    static constexpr Key kConfigKey_GroupKeyBase       = SilabsConfigKey(kMatterConfig_KeyBase, 0x0F);
    static constexpr Key kConfigKey_LockUser           = SilabsConfigKey(kMatterConfig_KeyBase, 0x10);
    static constexpr Key kConfigKey_Credential         = SilabsConfigKey(kMatterConfig_KeyBase, 0x11);
    static constexpr Key kConfigKey_LockUserName       = SilabsConfigKey(kMatterConfig_KeyBase, 0x12);
    static constexpr Key kConfigKey_CredentialData     = SilabsConfigKey(kMatterConfig_KeyBase, 0x13);
    static constexpr Key kConfigKey_UserCredentials    = SilabsConfigKey(kMatterConfig_KeyBase, 0x14);
    static constexpr Key kConfigKey_WeekDaySchedules   = SilabsConfigKey(kMatterConfig_KeyBase, 0x15);
    static constexpr Key kConfigKey_YearDaySchedules   = SilabsConfigKey(kMatterConfig_KeyBase, 0x16);
    static constexpr Key kConfigKey_HolidaySchedules   = SilabsConfigKey(kMatterConfig_KeyBase, 0x17);
    // UniqueId exposed in the Basic Information cluster. It is cleared on factoryreset
    // We will generate a random ID, if none was previously provided.
    static constexpr Key kConfigKey_UniqueId              = SilabsConfigKey(kMatterConfig_KeyBase, 0x18);
    static constexpr Key kConfigKey_OpKeyMap              = SilabsConfigKey(kMatterConfig_KeyBase, 0x20);
    static constexpr Key kConfigKey_BootCount             = SilabsConfigKey(kMatterConfig_KeyBase, 0x21);
    static constexpr Key kConfigKey_TotalOperationalHours = SilabsConfigKey(kMatterConfig_KeyBase, 0x22);

    static constexpr Key kConfigKey_GroupKeyMax =
        SilabsConfigKey(kMatterConfig_KeyBase, 0x1E); // Allows 16 Group Keys to be created.

    // Matter Counter Keys
    static constexpr Key kConfigKey_LifeTimeCounter  = SilabsConfigKey(kMatterCounter_KeyBase, 0x02);
    static constexpr Key kConfigKey_MigrationCounter = SilabsConfigKey(kMatterCounter_KeyBase, 0x03);

    // Matter KVS storage Keys
    static constexpr Key kConfigKey_KvsStringKeyMap = SilabsConfigKey(kMatterKvs_KeyBase, 0x00);
    static constexpr Key kConfigKey_KvsFirstKeySlot = SilabsConfigKey(kMatterKvs_KeyBase, 0x01);
    static constexpr Key kConfigKey_KvsLastKeySlot =
        SilabsConfigKey(kMatterKvs_KeyBase + (KVS_MAX_ENTRIES >> 8), KVS_MAX_ENTRIES & UINT8_MAX);

    // Set key id limits for each group.
    static constexpr Key kMinConfigKey_MatterFactory = SilabsConfigKey(kMatterFactory_KeyBase, 0x00);
    static constexpr Key kMaxConfigKey_MatterFactory = SilabsConfigKey(kMatterFactory_KeyBase, 0x30);
    static constexpr Key kMinConfigKey_MatterConfig  = SilabsConfigKey(kMatterConfig_KeyBase, 0x00);
    static constexpr Key kMaxConfigKey_MatterConfig  = SilabsConfigKey(kMatterConfig_KeyBase, 0x22);

    // Allows 32 Counters to be created.
    static constexpr Key kMinConfigKey_MatterCounter = SilabsConfigKey(kMatterCounter_KeyBase, 0x00);
    static constexpr Key kMaxConfigKey_MatterCounter = SilabsConfigKey(kMatterCounter_KeyBase, 0x1F);

    static constexpr Key kMinConfigKey_MatterKvs = kConfigKey_KvsStringKeyMap;
    static constexpr Key kMaxConfigKey_MatterKvs = SilabsConfigKey(kMatterKvs_ExtendedRange, 0xFF);
    static_assert(kConfigKey_KvsLastKeySlot <= kMaxConfigKey_MatterKvs,
                  "Configured KVS_MAX_ENTRIES overflows the reserved KVS Key range");

    static CHIP_ERROR Init(void);
    static void DeInit(void);

    // Configuration methods used by the GenericConfigurationManagerImpl<> template.
    static CHIP_ERROR ReadConfigValue(Key key, bool & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint16_t & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);
    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen, size_t offset);
    static CHIP_ERROR ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val);
    static CHIP_ERROR WriteConfigValue(Key key, bool val);
    static CHIP_ERROR WriteConfigValue(Key key, uint16_t val);
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
