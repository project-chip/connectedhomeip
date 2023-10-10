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
 *          platforms based on the NXP SDK.
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "FreeRTOS.h"
#include <functional>

#define CHIP_PLAT_NO_NVM 0
#define CHIP_PLAT_NVM_FWK 1
#define CHIP_PLAT_LITTLEFS 2
#define CHIP_PLAT_KEY_STORAGE 3

#define DEBUG_NVM 0

#ifndef CHIP_PLAT_NVM_SUPPORT
#define CHIP_PLAT_NVM_SUPPORT CHIP_PLAT_NO_NVM
#endif

#if (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_NVM_FWK)
#include "NVM_Interface.h"
#elif (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_LITTLEFS)
#include "fwk_filesystem.h"
#endif

#if (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_KEY_STORAGE)
#include "fwk_key_storage.h"
#else
#include "ram_storage.h"
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

/* Base for the category calculation when determining the key IDs */
#define CATEGORY_BASE 0x01

constexpr inline uint16_t config_key(uint8_t chipId, uint8_t pdmId)
{
    return static_cast<uint16_t>(chipId) << 8 | pdmId;
}

/**
 * This implementation uses the NVM component from Connectivity Framework or the
 *      The Little FS component depending on CHIP_PLAT_NVM_SUPPORT defined in the build system
 *
 * NOTE: This class is designed to be mixed-in to the concrete subclass of the
 * GenericConfigurationManagerImpl<> template.  When used this way, the class
 * naturally provides implementations for the delegated members referenced by
 * the template class (e.g. the ReadConfigValue() method).
 */
class NXPConfig
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
                                                                       *  Cleared during factory reset. */

    using Key = uint16_t;

    // Key definitions for well-known configuration values.
    // Factory config keys
    static constexpr Key kConfigKey_SerialNum             = config_key(kFileId_ChipFactory, 0x00);
    static constexpr Key kConfigKey_UniqueId              = config_key(kFileId_ChipFactory, 0x01);
    static constexpr Key kConfigKey_MfrDeviceId           = config_key(kFileId_ChipFactory, 0x02);
    static constexpr Key kConfigKey_MfrDeviceCert         = config_key(kFileId_ChipFactory, 0x03);
    static constexpr Key kConfigKey_MfrDevicePrivateKey   = config_key(kFileId_ChipFactory, 0x04);
    static constexpr Key kConfigKey_ManufacturingDate     = config_key(kFileId_ChipFactory, 0x05);
    static constexpr Key kConfigKey_SetupPinCode          = config_key(kFileId_ChipFactory, 0x06);
    static constexpr Key kConfigKey_MfrDeviceICACerts     = config_key(kFileId_ChipFactory, 0x07);
    static constexpr Key kConfigKey_HardwareVersion       = config_key(kFileId_ChipFactory, 0x08);
    static constexpr Key kConfigKey_SetupDiscriminator    = config_key(kFileId_ChipFactory, 0x09);
    static constexpr Key kConfigKey_Spake2pIterationCount = config_key(kFileId_ChipFactory, 0x0A);
    static constexpr Key kConfigKey_Spake2pSalt           = config_key(kFileId_ChipFactory, 0x0B);
    static constexpr Key kConfigKey_Spake2pVerifier       = config_key(kFileId_ChipFactory, 0x0C);

    // CHIP Config Keys
    static constexpr Key kConfigKey_FabricId           = config_key(kFileId_ChipConfig, 0x00);
    static constexpr Key kConfigKey_ServiceConfig      = config_key(kFileId_ChipConfig, 0x01);
    static constexpr Key kConfigKey_PairedAccountId    = config_key(kFileId_ChipConfig, 0x02);
    static constexpr Key kConfigKey_ServiceId          = config_key(kFileId_ChipConfig, 0x03);
    static constexpr Key kConfigKey_FabricSecret       = config_key(kFileId_ChipConfig, 0x04);
    static constexpr Key kConfigKey_LastUsedEpochKeyId = config_key(kFileId_ChipConfig, 0x05);
    static constexpr Key kConfigKey_FailSafeArmed      = config_key(kFileId_ChipConfig, 0x06);

    static constexpr Key kConfigKey_OperationalDeviceId         = config_key(kFileId_ChipConfig, 0x07);
    static constexpr Key kConfigKey_OperationalDeviceCert       = config_key(kFileId_ChipConfig, 0x08);
    static constexpr Key kConfigKey_OperationalDeviceICACerts   = config_key(kFileId_ChipConfig, 0x09);
    static constexpr Key kConfigKey_OperationalDevicePrivateKey = config_key(kFileId_ChipConfig, 0x0A);

    static constexpr Key kConfigKey_RegulatoryLocation = config_key(kFileId_ChipConfig, 0x0B);
    static constexpr Key kConfigKey_CountryCode        = config_key(kFileId_ChipConfig, 0x0C);
    static constexpr Key kConfigKey_Breadcrumb         = config_key(kFileId_ChipConfig, 0x0D);

    // CHIP Counter Keys
    static constexpr Key kCounterKey_RebootCount           = config_key(kFileId_ChipCounter, 0x00);
    static constexpr Key kCounterKey_UpTime                = config_key(kFileId_ChipCounter, 0x01);
    static constexpr Key kCounterKey_TotalOperationalHours = config_key(kFileId_ChipCounter, 0x02);
    static constexpr Key kCounterKey_BootReason            = config_key(kFileId_ChipCounter, 0x03);

    static constexpr Key kConfigKey_GroupKey   = config_key(kFileId_ChipConfig, 0x0E);
    static constexpr Key kConfigKey_GroupKey0  = config_key(kFileId_ChipConfig, 0x0F);
    static constexpr Key kConfigKey_GroupKey1  = config_key(kFileId_ChipConfig, 0x10);
    static constexpr Key kConfigKey_GroupKey2  = config_key(kFileId_ChipConfig, 0x11);
    static constexpr Key kConfigKey_GroupKey3  = config_key(kFileId_ChipConfig, 0x12);
    static constexpr Key kConfigKey_GroupKey4  = config_key(kFileId_ChipConfig, 0x13);
    static constexpr Key kConfigKey_GroupKey5  = config_key(kFileId_ChipConfig, 0x14);
    static constexpr Key kConfigKey_GroupKey6  = config_key(kFileId_ChipConfig, 0x15);
    static constexpr Key kConfigKey_GroupKey7  = config_key(kFileId_ChipConfig, 0x16);
    static constexpr Key kConfigKey_GroupKey8  = config_key(kFileId_ChipConfig, 0x17);
    static constexpr Key kConfigKey_GroupKey9  = config_key(kFileId_ChipConfig, 0x18);
    static constexpr Key kConfigKey_GroupKey10 = config_key(kFileId_ChipConfig, 0x19);
    static constexpr Key kConfigKey_GroupKey11 = config_key(kFileId_ChipConfig, 0x1A);
    static constexpr Key kConfigKey_GroupKey12 = config_key(kFileId_ChipConfig, 0x1B);
    static constexpr Key kConfigKey_GroupKey13 = config_key(kFileId_ChipConfig, 0x1C);
    static constexpr Key kConfigKey_GroupKey14 = config_key(kFileId_ChipConfig, 0x1D);
    static constexpr Key kConfigKey_GroupKey15 = config_key(kFileId_ChipConfig, 0x1E);

    static constexpr Key kConfigKey_GroupKeyBase = kConfigKey_GroupKey0;
    static constexpr Key kConfigKey_GroupKeyMax  = config_key(kFileId_ChipConfig, 0x1E);
    ; // Allows 16 Group Keys to be created.

    // Set key id limits for each group.
    static constexpr Key kMinConfigKey_ChipFactory = config_key(kFileId_ChipFactory, 0x00);
    static constexpr Key kMaxConfigKey_ChipFactory = config_key(kFileId_ChipFactory, 0x08);
    static constexpr Key kMinConfigKey_ChipConfig  = config_key(kFileId_ChipConfig, 0x00);
    static constexpr Key kMaxConfigKey_ChipConfig  = config_key(kFileId_ChipConfig, 0x1E);
    static constexpr Key kMinConfigKey_ChipCounter = config_key(kFileId_ChipCounter, 0x00);
    static constexpr Key kMaxConfigKey_ChipCounter = config_key(kFileId_ChipCounter, 0x1F); // Allows 32 Counters to be created.
    static constexpr Key kMinConfigKey_KVS         = config_key(kFileId_KVS, 0x00);
    static constexpr Key kMaxConfigKey_KVS         = config_key(kFileId_KVS, 0xFF);

    static CHIP_ERROR Init(void);

    // Configuration methods used by the GenericConfigurationManagerImpl<> template.
    static CHIP_ERROR ReadConfigValue(Key key, bool & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint32_t & val);
    static CHIP_ERROR ReadConfigValue(Key key, uint64_t & val);
    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(const char * keyString, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val);
    static CHIP_ERROR WriteConfigValue(Key key, bool val);
    static CHIP_ERROR WriteConfigValue(Key key, uint32_t val);
    static CHIP_ERROR WriteConfigValue(Key key, uint64_t val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR WriteConfigValueBin(const char * keyString, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR WriteConfigValueCounter(uint8_t counterIdx, uint32_t val);
    static CHIP_ERROR ClearConfigValue(Key key);
    static CHIP_ERROR ClearConfigValue(const char * keyString);
    static bool ConfigValueExists(Key key);
    static CHIP_ERROR FactoryResetConfig(void);
    static bool ValidConfigKey(Key key);

    static void RunConfigUnitTest(void);
    static void RunSystemIdleTask(void);

private:
#if (CHIP_PLAT_NVM_SUPPORT == CHIP_PLAT_KEY_STORAGE)
    static CHIP_ERROR MapKeyStorageStatus(ks_error_t ksStatus);
#else
    static CHIP_ERROR MapRamStorageStatus(rsError rsStatus);
#endif
    static int SaveIntKeysToFS(void);
    static int SaveStringKeysToFS(void);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
