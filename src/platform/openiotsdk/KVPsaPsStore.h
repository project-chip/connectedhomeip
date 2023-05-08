/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          Open IoT SDK key-value storage base on flash TDBStore.
 */

#pragma once

#include <functional>
#include <string.h>

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <platform/PersistedStorage.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include <psa/protected_storage.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Platform::PersistedStorage;

// PSA Protected Storage interface provides 64-bit ID number,
// For Matter we reserved region ranges from 0x00010000 to 0x0001FFFF
// e.g. key = 0x010214
// '01' = Matter region
// '02' = the sub region group base offset (Factory, Config, Counter or KVS)
// '14' = the id offset inside the group.
constexpr Key kMatterPsaPaKeyRegion = 0x010000U;
constexpr inline Key GetPsaPaKey(Key group, uint8_t id)
{
    return kMatterPsaPaKeyRegion | (group) << 8 | id;
}

#define KVS_MAX_ENTRIES 30

#define KVS_KEY_MAP_UPDATE_DELAY_SEC 5

/**
 * This class provides access to configuration information for Open IoT SDK platform
 * stored in PSA Protected Storage.
 */
class KVPsaPsStore
{
public:
    using Key = chip::Platform::PersistedStorage::Key;

    // Persistent config values set at manufacturing time. Retained during factory reset.
    static const Key kMatterFactory_KeyOffset;
    // Persistent config values set at runtime. Cleared during factory reset.
    static const Key kMatterConfig_KeyOffset;
    // Persistent counter values set at runtime. Retained during factory reset.
    static const Key kMatterCounter_KeyOffset;
    // Persistent config values set at runtime. Cleared during factory reset.
    static const Key kMatterKvs_KeyOffset;

    // Key definitions for well-known keys.
    // Factory keys
    static const Key kConfigKey_SerialNum;
    static const Key kConfigKey_MfrDeviceId;
    static const Key kConfigKey_MfrDeviceCert;
    static const Key kConfigKey_MfrDeviceICACerts;
    static const Key kConfigKey_MfrDevicePrivateKey;
    static const Key kConfigKey_HardwareVersion;
    static const Key kConfigKey_ManufacturingDate;
    static const Key kConfigKey_SetupPinCode;
    static const Key kConfigKey_SetupDiscriminator;
    static const Key kConfigKey_Spake2pIterationCount;
    static const Key kConfigKey_Spake2pSalt;
    static const Key kConfigKey_Spake2pVerifier;
    static const Key kConfigKey_VendorId;
    static const Key kConfigKey_ProductId;

    // Config Keys
    static const Key kConfigKey_ServiceConfig;
    static const Key kConfigKey_PairedAccountId;
    static const Key kConfigKey_ServiceId;
    static const Key kConfigKey_LastUsedEpochKeyId;
    static const Key kConfigKey_FailSafeArmed;
    static const Key kConfigKey_WiFiStationSecType;
    static const Key kConfigKey_RegulatoryLocation;
    static const Key kConfigKey_CountryCode;
    static const Key kConfigKey_LocationCapability;
    static const Key kConfigKey_UniqueId;

    // Counter Keys
    static const Key kCounterKey_RebootCount;
    static const Key kCounterKey_UpTime;
    static const Key kCounterKey_TotalOperationalHours;
    static const Key kCounterKey_BootReason;

    // KVS storage Keys
    static const Key kConfigKey_KvsStringKeyMap;
    static const Key kConfigKey_KvsFirstKeySlot;
    static const Key kConfigKey_KvsLastKeySlot;

    // NVS helper variables
    static const Key kMinConfigKey_MatterConfig;
    static const Key kMaxConfigKey_MatterConfig;
    static const Key kMinConfigKey_MatterCounter;
    static const Key kMaxConfigKey_MatterCounter;

    static const Key kMinMatterPsaPaKeyRegion;
    static const Key kMaxMatterPsaPaKeyRegion;

    static CHIP_ERROR Init(void);

    // Config value accessors.
    static CHIP_ERROR ReadConfigValue(Key key, bool & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);
    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen, size_t offset);
    static CHIP_ERROR ReadConfigValueCounter(Key counterId, uint32_t & val);

    static CHIP_ERROR WriteConfigValue(Key key, bool val);
    static CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
    static CHIP_ERROR WriteConfigValue(Key key, uint64_t val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR WriteConfigValueCounter(Key counterId, uint32_t val);

    static CHIP_ERROR ClearConfigValue(Key key);

    // Additional functions
    static CHIP_ERROR FactoryResetConfig(void);
    static void RunConfigUnitTest(void);
    static bool ConfigValueExists(Key key);
    static bool ValidKvsKey(Key key);
    static void KVSKeyMapUpdate(void);
    static void ScheduleKVSKeyMapUpdate(void);

protected:
    using ForEachObjectFunct = std::function<CHIP_ERROR(const Key & key, const size_t & size)>;
    static CHIP_ERROR ForEachObject(Key firstKey, Key lastKey, bool addNewRecord, ForEachObjectFunct funct);

private:
    // NVS helper functions.
    static bool ValidConfigKey(Key key);
    static bool ConfigValueExists(Key key, size_t & size);
    static CHIP_ERROR PsaStatus2ChipError(psa_status_t status);
    static void OnScheduleKVSKeyMapUpdate(System::Layer * systemLayer, void * appState);

    static bool initialized;
};

class KVPsaPsStoreKeyBuilder
{
public:
    KVPsaPsStoreKeyBuilder(const char * key);

    ~KVPsaPsStoreKeyBuilder();

    void AddKey(void);
    void RemoveKey(void);

    KVPsaPsStore::Key GetKey() const;

private:
    KVPsaPsStore::Key keyValue;
    char buffer[PersistentStorageDelegate::kKeyLengthMax + 1];
    bool valid;
    bool existing;
};

inline KVPsaPsStore::Key KVPsaPsStoreKeyBuilder::GetKey() const
{
    return valid ? keyValue : 0;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

using KVStoreConfig     = chip::DeviceLayer::Internal::KVPsaPsStore;
using KVStoreKeyBuilder = chip::DeviceLayer::Internal::KVPsaPsStoreKeyBuilder;
