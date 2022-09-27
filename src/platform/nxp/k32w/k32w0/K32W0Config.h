/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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

#pragma once

#include <functional>

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "PDM.h"
#include "fsl_os_abstraction.h"
#include "pdm_ram_storage_glue.h"
#include "ram_storage.h"
#include <platform/nxp/k32w/common/RamStorage.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

constexpr uint16_t kNvmIdChipConfigData  = 0x5000;
constexpr uint16_t kRamBufferInitialSize = 3072;

constexpr inline uint16_t K32WConfigKey(uint8_t chipId, uint8_t pdmId)
{
    return static_cast<uint16_t>(chipId) << 8 | pdmId;
}

/**
 *
 * This implementation uses the NXP Persistent Data Manager (PDM) library as the
 * underlying storage layer.
 *
 * NOTE: This class is designed to be mixed-in to the concrete subclass of the
 * GenericConfigurationManagerImpl<> template.  When used this way, the class
 * naturally provides implementations for the delegated members referenced by
 * the template class (e.g. the ReadConfigValue() method).
 */
class K32WConfig
{
public:
    // PDM ids used by the CHIP Device Layer
    static constexpr uint8_t kPDMId_ChipFactory = 0x01; /**< PDM id for settings containing persistent config values set at
                                                         * manufacturing time. Retained during factory reset. */
    static constexpr uint8_t kPDMId_ChipConfig = 0x02;  /**< PDM id for settings containing dynamic config values set at runtime.
                                                         *   Cleared during factory reset. */
    static constexpr uint8_t kPDMId_ChipCounter = 0x03; /**< PDM id for settings containing dynamic counter values set at runtime.
                                                         *   Retained during factory reset. */
    static constexpr uint8_t kPDMId_KVSKey = 0x04;      /**< PDM id for settings containing KVS keys set at runtime.
                                                         *   Cleared during factory reset. */
    static constexpr uint8_t kPDMId_KVSValue = 0x05;    /**< PDM id for settings containing KVS values set at runtime.
                                                         *   Cleared during factory reset. */

    using Key = uint32_t;

    // Key definitions for well-known configuration values.
    // Factory Config Keys
    static constexpr Key kConfigKey_SerialNum             = K32WConfigKey(kPDMId_ChipFactory, 0x00);
    static constexpr Key kConfigKey_MfrDeviceId           = K32WConfigKey(kPDMId_ChipFactory, 0x01);
    static constexpr Key kConfigKey_MfrDeviceCert         = K32WConfigKey(kPDMId_ChipFactory, 0x02);
    static constexpr Key kConfigKey_MfrDevicePrivateKey   = K32WConfigKey(kPDMId_ChipFactory, 0x03);
    static constexpr Key kConfigKey_ManufacturingDate     = K32WConfigKey(kPDMId_ChipFactory, 0x04);
    static constexpr Key kConfigKey_SetupPinCode          = K32WConfigKey(kPDMId_ChipFactory, 0x05);
    static constexpr Key kConfigKey_MfrDeviceICACerts     = K32WConfigKey(kPDMId_ChipFactory, 0x06);
    static constexpr Key kConfigKey_HardwareVersion       = K32WConfigKey(kPDMId_ChipFactory, 0x07);
    static constexpr Key kConfigKey_SetupDiscriminator    = K32WConfigKey(kPDMId_ChipFactory, 0x08);
    static constexpr Key kConfigKey_Spake2pIterationCount = K32WConfigKey(kPDMId_ChipFactory, 0x09);
    static constexpr Key kConfigKey_Spake2pSalt           = K32WConfigKey(kPDMId_ChipFactory, 0x0A);
    static constexpr Key kConfigKey_Spake2pVerifier       = K32WConfigKey(kPDMId_ChipFactory, 0x0B);

    // CHIP Config Keys
    static constexpr Key kConfigKey_ServiceConfig      = K32WConfigKey(kPDMId_ChipConfig, 0x01);
    static constexpr Key kConfigKey_PairedAccountId    = K32WConfigKey(kPDMId_ChipConfig, 0x02);
    static constexpr Key kConfigKey_ServiceId          = K32WConfigKey(kPDMId_ChipConfig, 0x03);
    static constexpr Key kConfigKey_LastUsedEpochKeyId = K32WConfigKey(kPDMId_ChipConfig, 0x05);
    static constexpr Key kConfigKey_FailSafeArmed      = K32WConfigKey(kPDMId_ChipConfig, 0x06);
    static constexpr Key kConfigKey_RegulatoryLocation = K32WConfigKey(kPDMId_ChipConfig, 0x07);
    static constexpr Key kConfigKey_CountryCode        = K32WConfigKey(kPDMId_ChipConfig, 0x08);
    static constexpr Key kConfigKey_UniqueId           = K32WConfigKey(kPDMId_ChipConfig, 0x0A);
    static constexpr Key kConfigKey_SoftwareVersion    = K32WConfigKey(kPDMId_ChipConfig, 0x0B);
    static constexpr Key kConfigKey_FirstRunOfOTAImage = K32WConfigKey(kPDMId_ChipConfig, 0x0C);

    // CHIP Counter Keys
    static constexpr Key kCounterKey_RebootCount           = K32WConfigKey(kPDMId_ChipCounter, 0x00);
    static constexpr Key kCounterKey_UpTime                = K32WConfigKey(kPDMId_ChipCounter, 0x01);
    static constexpr Key kCounterKey_TotalOperationalHours = K32WConfigKey(kPDMId_ChipCounter, 0x02);
    static constexpr Key kCounterKey_BootReason            = K32WConfigKey(kPDMId_ChipCounter, 0x03);

    // Set key id limits for each group.
    static constexpr Key kMinConfigKey_ChipFactory = K32WConfigKey(kPDMId_ChipFactory, 0x00);
    static constexpr Key kMaxConfigKey_ChipFactory = K32WConfigKey(kPDMId_ChipFactory, 0xFF);
    static constexpr Key kMinConfigKey_ChipConfig  = K32WConfigKey(kPDMId_ChipConfig, 0x00);
    static constexpr Key kMaxConfigKey_ChipConfig  = K32WConfigKey(kPDMId_ChipConfig, 0xFF);
    static constexpr Key kMinConfigKey_ChipCounter = K32WConfigKey(kPDMId_ChipCounter, 0x00);
    static constexpr Key kMaxConfigKey_ChipCounter = K32WConfigKey(kPDMId_ChipCounter, 0xFF); // Allows 32 Counters to be created.
    static constexpr Key kMinConfigKey_KVSKey      = K32WConfigKey(kPDMId_KVSKey, 0x00);
    static constexpr Key kMaxConfigKey_KVSKey      = K32WConfigKey(kPDMId_KVSKey, 0xFF);
    static constexpr Key kMinConfigKey_KVSValue    = K32WConfigKey(kPDMId_KVSValue, 0x00);
    static constexpr Key kMaxConfigKey_KVSValue    = K32WConfigKey(kPDMId_KVSValue, 0xFF);

    static CHIP_ERROR Init(void);

    // Configuration methods used by the GenericConfigurationManagerImpl<> template.
    template <typename TValue>
    static CHIP_ERROR ReadConfigValue(Key key, TValue & val);
    template <typename TValue>
    static CHIP_ERROR WriteConfigValue(Key key, TValue val);
    template <typename TValue>
    static CHIP_ERROR WriteConfigValueSync(Key key, TValue val);

    static CHIP_ERROR ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen);
    static CHIP_ERROR ReadConfigValueCounter(uint8_t counterIdx, uint32_t & val);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str);
    static CHIP_ERROR WriteConfigValueStr(Key key, const char * str, size_t strLen);
    static CHIP_ERROR WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen);
    static CHIP_ERROR WriteConfigValueCounter(uint8_t counterIdx, uint32_t val);
    static CHIP_ERROR ClearConfigValue(Key key);
    static bool ConfigValueExists(Key key);
    static CHIP_ERROR FactoryResetConfig(void);
    static bool ValidConfigKey(Key key);

    static void RunConfigUnitTest(void);

    // Log error wrappers for OSA mutex lock/unlock.
    static void MutexLock(osaMutexId_t mutexId, uint32_t millisec);
    static void MutexUnlock(osaMutexId_t mutexId);

    static osaMutexId_t pdmMutexHandle;

protected:
    static constexpr uint8_t GetPDMId(uint32_t key);
    static constexpr uint8_t GetRecordKey(uint32_t key);

private:
    static CHIP_ERROR MapPdmStatusToChipError(PDM_teStatus status);
    static CHIP_ERROR MapPdmInitStatusToChipError(int status);
    static void FactoryResetConfigInternal(Key firstKey, Key lastKey);
};

/**
 * Extract a PDM id from a Key value.
 */
inline constexpr uint8_t K32WConfig::GetPDMId(Key key)
{
    return static_cast<uint8_t>(key >> 8);
}

/**
 * Extract an NVM record key from a Key value.
 */
inline constexpr uint8_t K32WConfig::GetRecordKey(Key key)
{
    return static_cast<uint8_t>(key);
}

template <typename TValue>
CHIP_ERROR K32WConfig::ReadConfigValue(Key key, TValue & val)
{
    CHIP_ERROR err;
    uint16_t valLen = sizeof(val);

    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    err = RamStorage::Read(key, 0, (uint8_t *) &val, &valLen);
    SuccessOrExit(err);

exit:
    return err;
}

template <typename TValue>
CHIP_ERROR K32WConfig::WriteConfigValue(Key key, TValue val)
{
    CHIP_ERROR err;
    PDM_teStatus status;
    RamStorage::Buffer buffer;

    MutexLock(pdmMutexHandle, osaWaitForever_c);
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    err = RamStorage::Write(key, (uint8_t *) &val, sizeof(TValue));
    SuccessOrExit(err);

    buffer = RamStorage::GetBuffer();
    status = PDM_eSaveRecordDataInIdleTask(kNvmIdChipConfigData, buffer, buffer->ramBufferLen + kRamDescHeaderSize);
    SuccessOrExit(err = MapPdmStatusToChipError(status));

exit:
    MutexUnlock(pdmMutexHandle);
    return err;
}

template <typename TValue>
CHIP_ERROR K32WConfig::WriteConfigValueSync(Key key, TValue val)
{
    CHIP_ERROR err;
    PDM_teStatus status;
    RamStorage::Buffer buffer;

    MutexLock(pdmMutexHandle, osaWaitForever_c);
    VerifyOrExit(ValidConfigKey(key), err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND);
    err = RamStorage::Write(key, (uint8_t *) &val, sizeof(TValue));
    SuccessOrExit(err);
    // Interrupts are disabled to ensure there is no context switch during the actual
    // writing, thus avoiding race conditions.
    OSA_InterruptDisable();
    buffer = RamStorage::GetBuffer();
    status = PDM_eSaveRecordData(kNvmIdChipConfigData, buffer, buffer->ramBufferLen + kRamDescHeaderSize);
    OSA_InterruptEnable();
    SuccessOrExit(err = MapPdmStatusToChipError(status));

exit:
    MutexUnlock(pdmMutexHandle);
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
