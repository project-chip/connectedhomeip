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

#include <string.h>

#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <platform/PersistedStorage.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/testing/ConfigUnitTest.h>

#include <iotsdk/TDBStore.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * This class provides access to configuration information for Open IoT SDK platform
 * stored in Lightweight Key Value storage over a flash block device.
 */
class KVBlockDeviceStore
{
public:
    using Key = chip::Platform::PersistedStorage::Key;

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

    // Initialization
    static CHIP_ERROR Init(void);

    // Config value accessors
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

private:
    static iotsdk::storage::TDBStore * tdb;

    // NVS Namespace helper functions
    static CHIP_ERROR ConstructCounterKey(Key id, char * buf, size_t bufSize);
    static CHIP_ERROR ClearNamespace(const char * ns);
};

class KVBlockDeviceStoreKeyBuilder
{
public:
    KVBlockDeviceStoreKeyBuilder(const char * key);

    void AddKey(void) {}
    void RemoveKey(void) {}

    KVBlockDeviceStore::Key GetKey() const;

private:
    char buffer[PersistentStorageDelegate::kKeyLengthMax + 1] = "chip-kvs-";
    bool valid;
    // Mbed KV storage does not accept these characters in the key definition
    const char * illegalCharacters = " */?:;\"|<>\\";
};

inline KVBlockDeviceStore::Key KVBlockDeviceStoreKeyBuilder::GetKey() const
{
    return valid ? buffer : nullptr;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

using KVStoreConfig     = chip::DeviceLayer::Internal::KVBlockDeviceStore;
using KVStoreKeyBuilder = chip::DeviceLayer::Internal::KVBlockDeviceStoreKeyBuilder;
