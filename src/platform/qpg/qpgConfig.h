/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *          platforms based on the Qorvo QPG platforms.
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "FreeRTOS.h"
#include "qvCHIP.h"

#include <functional>

namespace chip {
namespace DeviceLayer {
namespace Internal {

/* Base for the category calculation when determining the key IDs */
#define CATEGORY_BASE 0x01

constexpr inline uint16_t QorvoConfigKey(uint8_t categoryId, uint8_t id)
{
    return static_cast<uint16_t>(((categoryId) << 6) | (id & 0x3F));
}

/**
 * This implementation uses the Qorvo NVM component as the underlying storage layer.
 *
 * NOTE: This class is designed to be mixed-in to the concrete subclass of the
 * GenericConfigurationManagerImpl<> template.  When used this way, the class
 * naturally provides implementations for the delegated members referenced by
 * the template class (e.g. the ReadConfigValue() method).
 */
class QPGConfig
{
public:
    // Category ids used by the CHIP Device Layer
    static constexpr uint8_t kFileId_ChipFactory = CATEGORY_BASE;    /**< Category containing persistent config values set at
                                                                      * manufacturing    time. Retained during factory reset. */
    static constexpr uint8_t kFileId_ChipConfig = CATEGORY_BASE + 1; /**< Catyegory containing dynamic config values set at runtime.
                                                                      *   Cleared during factory reset. */
    static constexpr uint8_t kFileId_ChipCounter = CATEGORY_BASE + 2; /**< Category containing dynamic counter values set at
                                                                       * runtime. Retained during factory reset. */

    using Key = uint16_t;

    // Key definitions for well-known configuration values.
    static constexpr Key kConfigKey_SerialNum             = QorvoConfigKey(kFileId_ChipFactory, 0x00);
    static constexpr Key kConfigKey_MfrDeviceId           = QorvoConfigKey(kFileId_ChipFactory, 0x01);
    static constexpr Key kConfigKey_MfrDeviceCert         = QorvoConfigKey(kFileId_ChipFactory, 0x02);
    static constexpr Key kConfigKey_MfrDevicePrivateKey   = QorvoConfigKey(kFileId_ChipFactory, 0x03);
    static constexpr Key kConfigKey_ManufacturingDate     = QorvoConfigKey(kFileId_ChipFactory, 0x04);
    static constexpr Key kConfigKey_SetupPinCode          = QorvoConfigKey(kFileId_ChipFactory, 0x05);
    static constexpr Key kConfigKey_MfrDeviceICACerts     = QorvoConfigKey(kFileId_ChipFactory, 0x06);
    static constexpr Key kConfigKey_SetupDiscriminator    = QorvoConfigKey(kFileId_ChipFactory, 0x07);
    static constexpr Key kConfigKey_Spake2pIterationCount = QorvoConfigKey(kFileId_ChipFactory, 0x08);
    static constexpr Key kConfigKey_Spake2pSalt           = QorvoConfigKey(kFileId_ChipFactory, 0x09);
    static constexpr Key kConfigKey_Spake2pVerifier       = QorvoConfigKey(kFileId_ChipFactory, 0x0A);

    static constexpr Key kConfigKey_ServiceConfig      = QorvoConfigKey(kFileId_ChipConfig, 0x01);
    static constexpr Key kConfigKey_PairedAccountId    = QorvoConfigKey(kFileId_ChipConfig, 0x02);
    static constexpr Key kConfigKey_ServiceId          = QorvoConfigKey(kFileId_ChipConfig, 0x03);
    static constexpr Key kConfigKey_LastUsedEpochKeyId = QorvoConfigKey(kFileId_ChipConfig, 0x05);
    static constexpr Key kConfigKey_FailSafeArmed      = QorvoConfigKey(kFileId_ChipConfig, 0x06);
    static constexpr Key kConfigKey_GroupKey           = QorvoConfigKey(kFileId_ChipConfig, 0x07);
    static constexpr Key kConfigKey_HardwareVersion    = QorvoConfigKey(kFileId_ChipConfig, 0x08);
    static constexpr Key kConfigKey_RegulatoryLocation = QorvoConfigKey(kFileId_ChipConfig, 0x09);
    static constexpr Key kConfigKey_CountryCode        = QorvoConfigKey(kFileId_ChipConfig, 0x0A);
    static constexpr Key kConfigKey_UniqueId           = QorvoConfigKey(kFileId_ChipConfig, 0x0C);

    static constexpr Key kConfigKey_GroupKeyBase = QorvoConfigKey(kFileId_ChipConfig, 0x0F);
    static constexpr Key kConfigKey_GroupKeyMax  = QorvoConfigKey(kFileId_ChipConfig, 0x1E); // Allows 16 Group Keys to be created.

    static constexpr Key kCounterKey_RebootCount           = QorvoConfigKey(kFileId_ChipCounter, 0x01);
    static constexpr Key kCounterKey_BootReason            = QorvoConfigKey(kFileId_ChipCounter, 0x02);
    static constexpr Key kCounterKey_TotalOperationalHours = QorvoConfigKey(kFileId_ChipCounter, 0x03);

    static constexpr Key kConfigKey_CounterKeyBase = QorvoConfigKey(kFileId_ChipCounter, 0x00);
    static constexpr Key kConfigKey_CounterKeyMax =
        QorvoConfigKey(kFileId_ChipCounter, 0x1F); // Allows 32 Counter Keys to be created.

    // Set key id limits for each group.
    static constexpr Key kMinConfigKey_ChipFactory = kConfigKey_SerialNum;
    static constexpr Key kMaxConfigKey_ChipFactory = kConfigKey_Spake2pVerifier;
    static constexpr Key kMinConfigKey_ChipConfig  = kConfigKey_ServiceConfig;
    static constexpr Key kMaxConfigKey_ChipConfig  = kConfigKey_GroupKeyMax;
    static constexpr Key kMinConfigKey_ChipCounter = kConfigKey_CounterKeyBase;
    static constexpr Key kMaxConfigKey_ChipCounter = kConfigKey_CounterKeyMax; // Allows 32 Counters to be created.

    static CHIP_ERROR Init(void);

    // Configuration methods used by the GenericConfigurationManagerImpl<> template.
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

protected:
    using ForEachRecordFunct = std::function<CHIP_ERROR(const Key & key, const size_t & length)>;
    static CHIP_ERROR ForEachRecord(uint16_t fileId, uint16_t recordKey, bool addNewRecord, ForEachRecordFunct funct);
    static CHIP_ERROR MapNVMError(qvStatus_t status);

private:
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
