/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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
 *          Texas Instruments cc13x2_26x2 SoCs.
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <ti/common/nv/nvintf.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

class CC13X2_26X2Config
{
public:
    struct Key;

    // TODO: Define a proper system ID in upstream driver
    static constexpr uint16_t kNvinf_sysid_chip = 13;

    static constexpr uint16_t kNVinf_itemid_chipConfig    = 1;
    static constexpr uint16_t kNVinf_itemid_chipFactory   = 2;
    static constexpr uint16_t kNVinf_itemid_chipCounters  = 3;
    static constexpr uint16_t kNVinf_itemid_chipKVS_key   = 4;
    static constexpr uint16_t kNVinf_itemid_chipKVS_value = 5;

    static const uint16_t kCC13X2_26X2Matter_SysID = kNvinf_sysid_chip;

    static const uint16_t kCC13X2_26X2Matter_ItemID_ChipConfig    = kNVinf_itemid_chipConfig;
    static const uint16_t kCC13X2_26X2Matter_ItemID_ChipFactory   = kNVinf_itemid_chipFactory;
    static const uint16_t kCC13X2_26X2Matter_ItemID_ChipCounters  = kNVinf_itemid_chipCounters;
    static const uint16_t kCC13X2_26X2Matter_ItemID_ChipKVS_key   = kNVinf_itemid_chipKVS_key;
    static const uint16_t kCC13X2_26X2Matter_ItemID_ChipKVS_value = kNVinf_itemid_chipKVS_value;

    // Key definitions for well-known keys.
    static const Key kConfigKey_SerialNum;
    static const Key kConfigKey_UniqueId;
    static const Key kConfigKey_MfrDeviceId;
    static const Key kConfigKey_MfrDeviceCert;
    static const Key kConfigKey_MfrDeviceICACerts;
    static const Key kConfigKey_MfrDevicePrivateKey;
    static const Key kConfigKey_HardwareVersion;
    static const Key kConfigKey_ManufacturingDate;
    static const Key kConfigKey_SetupPinCode;
    static const Key kConfigKey_SetupDiscriminator;
    static const Key kConfigKey_ServiceConfig;
    static const Key kConfigKey_PairedAccountId;
    static const Key kConfigKey_ServiceId;
    static const Key kConfigKey_LastUsedEpochKeyId;
    static const Key kConfigKey_FailSafeArmed;
    static const Key kConfigKey_WiFiStationSecType;
    static const Key kConfigKey_RegulatoryLocation;
    static const Key kConfigKey_CountryCode;
    static const Key kConfigKey_KVS_key;   // special key for KVS system, key storage
    static const Key kConfigKey_KVS_value; // special key for KVS system, value storage
    static const Key kConfigKey_Spake2pIterationCount;
    static const Key kConfigKey_Spake2pSalt;
    static const Key kConfigKey_Spake2pVerifier;
    static const Key kConfigKey_BootCount;
    static const Key kConfigKey_TotalOperationalHours;
    static const Key kConfigKey_LifeTimeCounter;

    static CHIP_ERROR Init(void);

    // Config value accessors.
    static CHIP_ERROR ReadConfigValue(Key key, bool & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);
    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR WriteConfigValue(Key key, bool val);
    static CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
    static CHIP_ERROR WriteConfigValue(Key key, uint64_t val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);
    static CHIP_ERROR FactoryResetConfig(void);

    static void RunConfigUnitTest(void);

    // internal to the platform for KeyValueStoreManagerImpl.cpp
    static CHIP_ERROR ReadKVS(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset_bytes);
    static CHIP_ERROR WriteKVS(const char * key, const void * value, size_t value_size);
    static CHIP_ERROR ClearKVS(const char * key);
};

struct CC13X2_26X2Config::Key
{
    NVINTF_itemID_t nvID;

    bool operator==(const Key & other) const;
};

inline bool CC13X2_26X2Config::Key::operator==(const Key & other) const
{
    return (nvID.systemID == other.nvID.systemID && nvID.itemID == other.nvID.itemID && nvID.subID == other.nvID.subID);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
