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
 *          platforms based on the  mw320 SDK.
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "FreeRTOS.h"

#include <functional>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/* Base for the category calculation when determining the key IDs */
#define CATEGORY_BASE 0x01

constexpr inline uint16_t ConfigKey(uint8_t chipId, uint8_t id)
{
    return static_cast<uint16_t>(((chipId) << 8) | (id & 0x3F));
}

/**
 * This implementation uses the  NVM component as the underlying storage layer.
 *
 * NOTE: This class is designed to be mixed-in to the concrete subclass of the
 * GenericConfigurationManagerImpl<> template.  When used this way, the class
 * naturally provides implementations for the delegated members referenced by
 * the template class (e.g. the ReadConfigValue() method).
 */
class MW320Config
{
public:
    // Category ids used by the CHIP Device Layer
    static constexpr uint8_t kFileId_ChipFactory = CATEGORY_BASE;    /**< Category containing persistent config values set at
                                                                      * manufacturing    time. Retained during factory reset. */
    static constexpr uint8_t kFileId_ChipConfig = CATEGORY_BASE + 1; /**< Catyegory containing dynamic config values set at runtime.
                                                                      *   Cleared during factory reset. */
    static constexpr uint8_t kFileId_ChipCounter = CATEGORY_BASE + 2; /**< Category containing dynamic counter values set at
                                                                       * runtime. Retained during factory reset. */
    static constexpr uint8_t kFileId_KVS = CATEGORY_BASE + 3;         /**< Category containing KVS set at runtime.
                                                                       *   Cleared during factory reset. */
    static constexpr uint8_t kPDMId_ChipCounter = CATEGORY_BASE + 4;  /**< Category containing KVS set at runtime.
                                                                       *   Cleared during factory reset. */

    using Key = uint32_t;

    // Key definitions for well-known configuration values.
    // Factory Config Keys
    static constexpr Key kConfigKey_SerialNum             = ConfigKey(kFileId_ChipFactory, 0x00);
    static constexpr Key kConfigKey_MfrDeviceId           = ConfigKey(kFileId_ChipFactory, 0x01);
    static constexpr Key kConfigKey_MfrDeviceCert         = ConfigKey(kFileId_ChipFactory, 0x02);
    static constexpr Key kConfigKey_MfrDevicePrivateKey   = ConfigKey(kFileId_ChipFactory, 0x03);
    static constexpr Key kConfigKey_ManufacturingDate     = ConfigKey(kFileId_ChipFactory, 0x04);
    static constexpr Key kConfigKey_SetupPinCode          = ConfigKey(kFileId_ChipFactory, 0x05);
    static constexpr Key kConfigKey_MfrDeviceICACerts     = ConfigKey(kFileId_ChipFactory, 0x06);
    static constexpr Key kConfigKey_HardwareVersion       = ConfigKey(kFileId_ChipFactory, 0x07);
    static constexpr Key kConfigKey_SetupDiscriminator    = ConfigKey(kFileId_ChipFactory, 0x08);
    static constexpr Key kConfigKey_Spake2pIterationCount = ConfigKey(kFileId_ChipFactory, 0x09);
    static constexpr Key kConfigKey_Spake2pSalt           = ConfigKey(kFileId_ChipFactory, 0x0A);
    static constexpr Key kConfigKey_Spake2pVerifier       = ConfigKey(kFileId_ChipFactory, 0x0B);

    // CHIP Config Keys
    static constexpr Key kConfigKey_FabricId                    = ConfigKey(kFileId_ChipConfig, 0x00);
    static constexpr Key kConfigKey_ServiceConfig               = ConfigKey(kFileId_ChipConfig, 0x01);
    static constexpr Key kConfigKey_PairedAccountId             = ConfigKey(kFileId_ChipConfig, 0x02);
    static constexpr Key kConfigKey_ServiceId                   = ConfigKey(kFileId_ChipConfig, 0x03);
    static constexpr Key kConfigKey_FabricSecret                = ConfigKey(kFileId_ChipConfig, 0x04);
    static constexpr Key kConfigKey_LastUsedEpochKeyId          = ConfigKey(kFileId_ChipConfig, 0x05);
    static constexpr Key kConfigKey_FailSafeArmed               = ConfigKey(kFileId_ChipConfig, 0x06);
    static constexpr Key kConfigKey_OperationalDeviceId         = ConfigKey(kFileId_ChipConfig, 0x07);
    static constexpr Key kConfigKey_OperationalDeviceCert       = ConfigKey(kFileId_ChipConfig, 0x08);
    static constexpr Key kConfigKey_OperationalDeviceICACerts   = ConfigKey(kFileId_ChipConfig, 0x09);
    static constexpr Key kConfigKey_OperationalDevicePrivateKey = ConfigKey(kFileId_ChipConfig, 0x0A);
    static constexpr Key kConfigKey_RegulatoryLocation          = ConfigKey(kFileId_ChipConfig, 0x0B);
    static constexpr Key kConfigKey_CountryCode                 = ConfigKey(kFileId_ChipConfig, 0x0C);
    // static constexpr Key kConfigKey_Breadcrumb           = ConfigKey(kFileId_ChipConfig, 0x0D);
    static constexpr Key kConfigKey_UniqueId = ConfigKey(kFileId_ChipConfig, 0x0D);

    // CHIP Counter Keys
    static constexpr Key kCounterKey_RebootCount           = ConfigKey(kPDMId_ChipCounter, 0x00);
    static constexpr Key kCounterKey_UpTime                = ConfigKey(kPDMId_ChipCounter, 0x01);
    static constexpr Key kCounterKey_TotalOperationalHours = ConfigKey(kPDMId_ChipCounter, 0x02);
    static constexpr Key kCounterKey_BootReason            = ConfigKey(kPDMId_ChipCounter, 0x03);

    static constexpr Key kConfigKey_GroupKey   = ConfigKey(kFileId_ChipConfig, 0x0E);
    static constexpr Key kConfigKey_GroupKey0  = ConfigKey(kFileId_ChipConfig, 0x0F);
    static constexpr Key kConfigKey_GroupKey1  = ConfigKey(kFileId_ChipConfig, 0x10);
    static constexpr Key kConfigKey_GroupKey2  = ConfigKey(kFileId_ChipConfig, 0x11);
    static constexpr Key kConfigKey_GroupKey3  = ConfigKey(kFileId_ChipConfig, 0x12);
    static constexpr Key kConfigKey_GroupKey4  = ConfigKey(kFileId_ChipConfig, 0x13);
    static constexpr Key kConfigKey_GroupKey5  = ConfigKey(kFileId_ChipConfig, 0x14);
    static constexpr Key kConfigKey_GroupKey6  = ConfigKey(kFileId_ChipConfig, 0x15);
    static constexpr Key kConfigKey_GroupKey7  = ConfigKey(kFileId_ChipConfig, 0x16);
    static constexpr Key kConfigKey_GroupKey8  = ConfigKey(kFileId_ChipConfig, 0x17);
    static constexpr Key kConfigKey_GroupKey9  = ConfigKey(kFileId_ChipConfig, 0x18);
    static constexpr Key kConfigKey_GroupKey10 = ConfigKey(kFileId_ChipConfig, 0x19);
    static constexpr Key kConfigKey_GroupKey11 = ConfigKey(kFileId_ChipConfig, 0x1A);
    static constexpr Key kConfigKey_GroupKey12 = ConfigKey(kFileId_ChipConfig, 0x1B);
    static constexpr Key kConfigKey_GroupKey13 = ConfigKey(kFileId_ChipConfig, 0x1C);
    static constexpr Key kConfigKey_GroupKey14 = ConfigKey(kFileId_ChipConfig, 0x1D);
    static constexpr Key kConfigKey_GroupKey15 = ConfigKey(kFileId_ChipConfig, 0x1E);

    static constexpr Key kConfigKey_GroupKeyBase = kConfigKey_GroupKey0;
    static constexpr Key kConfigKey_GroupKeyMax  = ConfigKey(kFileId_ChipConfig, 0x1E);
    ; // Allows 16 Group Keys to be created.

    // Set key id limits for each group.
    static constexpr Key kMinConfigKey_ChipFactory = ConfigKey(kFileId_ChipFactory, 0x00);
    static constexpr Key kMaxConfigKey_ChipFactory = ConfigKey(kFileId_ChipFactory, 0x0B);
    static constexpr Key kMinConfigKey_ChipConfig  = ConfigKey(kFileId_ChipConfig, 0x00);
    static constexpr Key kMaxConfigKey_ChipConfig  = ConfigKey(kFileId_ChipConfig, 0x1E);
    static constexpr Key kMinConfigKey_ChipCounter = ConfigKey(kFileId_ChipCounter, 0x00);
    static constexpr Key kMaxConfigKey_ChipCounter = ConfigKey(kFileId_ChipCounter, 0x1F); // Allows 32 Counters to be created.
    static constexpr Key kMinConfigKey_KVS         = ConfigKey(kFileId_KVS, 0x00);
    static constexpr Key kMaxConfigKey_KVS         = ConfigKey(kFileId_KVS, 0xFF);

    static CHIP_ERROR Init(void);

    // Configuration methods used by the GenericConfigurationManagerImpl<> template.
    static CHIP_ERROR ReadConfigValue(Key key, bool & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);
    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueCounter(const char * index, uint32_t & val);
    static CHIP_ERROR WriteConfigValue(Key key, bool val);
    static CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
    static CHIP_ERROR WriteConfigValue(Key key, uint64_t val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR WriteConfigValueCounter(uint8_t counterIdx, uint32_t val);
    static CHIP_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);
    static CHIP_ERROR FactoryResetConfig(void);
    static bool ValidConfigKey(Key key);

    static void RunConfigUnitTest(void);

protected:
    using ForEachRecordFunct = std::function<CHIP_ERROR(const Key & key, const size_t & length)>;
    static CHIP_ERROR ForEachRecord(Key firstKey, Key lastKey, bool addNewRecord, ForEachRecordFunct funct);

private:
    static CHIP_ERROR FactoryResetConfigInternal(Key firstKey, Key lastKey);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
