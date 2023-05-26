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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemMutex.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// *** CAUTION ***: Changing the values of these keys can *break* existing devices.
const KVPsaPsStore::Key KVPsaPsStore::kMatterFactory_KeyOffset = 0x0;
const KVPsaPsStore::Key KVPsaPsStore::kMatterConfig_KeyOffset  = 0x1;
const KVPsaPsStore::Key KVPsaPsStore::kMatterCounter_KeyOffset = 0x2;
const KVPsaPsStore::Key KVPsaPsStore::kMatterKvs_KeyOffset     = 0x3;

// Keys stored in the Matter factory group
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_SerialNum             = GetPsaPaKey(kMatterFactory_KeyOffset, 0x00);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_MfrDeviceId           = GetPsaPaKey(kMatterFactory_KeyOffset, 0x01);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_MfrDeviceCert         = GetPsaPaKey(kMatterFactory_KeyOffset, 0x02);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_MfrDeviceICACerts     = GetPsaPaKey(kMatterFactory_KeyOffset, 0x03);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_MfrDevicePrivateKey   = GetPsaPaKey(kMatterFactory_KeyOffset, 0x04);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_HardwareVersion       = GetPsaPaKey(kMatterFactory_KeyOffset, 0x05);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_ManufacturingDate     = GetPsaPaKey(kMatterFactory_KeyOffset, 0x06);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_SetupPinCode          = GetPsaPaKey(kMatterFactory_KeyOffset, 0x07);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_SetupDiscriminator    = GetPsaPaKey(kMatterFactory_KeyOffset, 0x08);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_Spake2pIterationCount = GetPsaPaKey(kMatterFactory_KeyOffset, 0x09);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_Spake2pSalt           = GetPsaPaKey(kMatterFactory_KeyOffset, 0x0A);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_Spake2pVerifier       = GetPsaPaKey(kMatterFactory_KeyOffset, 0x0B);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_VendorId              = GetPsaPaKey(kMatterFactory_KeyOffset, 0x0C);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_ProductId             = GetPsaPaKey(kMatterFactory_KeyOffset, 0x0D);

// Keys stored in the Matter config group
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_ServiceConfig      = GetPsaPaKey(kMatterConfig_KeyOffset, 0x00);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_PairedAccountId    = GetPsaPaKey(kMatterConfig_KeyOffset, 0x01);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_ServiceId          = GetPsaPaKey(kMatterConfig_KeyOffset, 0x02);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_LastUsedEpochKeyId = GetPsaPaKey(kMatterConfig_KeyOffset, 0x03);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_FailSafeArmed      = GetPsaPaKey(kMatterConfig_KeyOffset, 0x04);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_WiFiStationSecType = GetPsaPaKey(kMatterConfig_KeyOffset, 0x05);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_RegulatoryLocation = GetPsaPaKey(kMatterConfig_KeyOffset, 0x06);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_CountryCode        = GetPsaPaKey(kMatterConfig_KeyOffset, 0x07);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_LocationCapability = GetPsaPaKey(kMatterConfig_KeyOffset, 0x08);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_UniqueId           = GetPsaPaKey(kMatterConfig_KeyOffset, 0x09);

// Keys stored in the Matter counters group
const KVPsaPsStore::Key KVPsaPsStore::kCounterKey_RebootCount           = GetPsaPaKey(kMatterCounter_KeyOffset, 0x00);
const KVPsaPsStore::Key KVPsaPsStore::kCounterKey_UpTime                = GetPsaPaKey(kMatterCounter_KeyOffset, 0x01);
const KVPsaPsStore::Key KVPsaPsStore::kCounterKey_TotalOperationalHours = GetPsaPaKey(kMatterCounter_KeyOffset, 0x02);
const KVPsaPsStore::Key KVPsaPsStore::kCounterKey_BootReason            = GetPsaPaKey(kMatterCounter_KeyOffset, 0x03);

// Keys stored in the Matter key-value group
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_KvsStringKeyMap = GetPsaPaKey(kMatterKvs_KeyOffset, 0x00);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_KvsFirstKeySlot = GetPsaPaKey(kMatterKvs_KeyOffset, 0x01);
const KVPsaPsStore::Key KVPsaPsStore::kConfigKey_KvsLastKeySlot  = GetPsaPaKey(kMatterKvs_KeyOffset, KVS_MAX_ENTRIES);

// NVS helper variables
const KVPsaPsStore::Key KVPsaPsStore::kMinConfigKey_MatterConfig  = GetPsaPaKey(kMatterConfig_KeyOffset, 0x00);
const KVPsaPsStore::Key KVPsaPsStore::kMaxConfigKey_MatterConfig  = GetPsaPaKey(kMatterConfig_KeyOffset, 0xFF);
const KVPsaPsStore::Key KVPsaPsStore::kMinConfigKey_MatterCounter = GetPsaPaKey(kMatterCounter_KeyOffset, 0x00);
const KVPsaPsStore::Key KVPsaPsStore::kMaxConfigKey_MatterCounter = GetPsaPaKey(kMatterCounter_KeyOffset, 0xFF);

const KVPsaPsStore::Key KVPsaPsStore::kMinMatterPsaPaKeyRegion = GetPsaPaKey(kMatterFactory_KeyOffset, 0x00);
const KVPsaPsStore::Key KVPsaPsStore::kMaxMatterPsaPaKeyRegion = GetPsaPaKey(kMatterKvs_KeyOffset, 0xFF);

char mKvsStoredKeyString[KVS_MAX_ENTRIES][PersistentStorageDelegate::kKeyLengthMax + 1];
chip::System::Mutex mKvsStoredKeyMutex;

bool KVPsaPsStore::initialized = false;

CHIP_ERROR KVPsaPsStore::Init(void)
{
    if (initialized)
    {
        return CHIP_NO_ERROR;
    }

    memset(mKvsStoredKeyString, 0, sizeof(mKvsStoredKeyString));
    size_t outLen;
    CHIP_ERROR err = ReadConfigValueBin(kConfigKey_KvsStringKeyMap, reinterpret_cast<uint8_t *>(mKvsStoredKeyString),
                                        sizeof(mKvsStoredKeyString), outLen);

    if ((err != CHIP_NO_ERROR) && (err != CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)) // Initial boot
    {
        return err;
    }

    chip::System::Mutex::Init(mKvsStoredKeyMutex);
    initialized = true;

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::Shutdown(void)
{
    KVSKeyMapUpdate();

    initialized = false;

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::ReadConfigValue(Key key, bool & val)
{
    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    size_t actual_size = 0;
    CHIP_ERROR err     = PsaStatus2ChipError(psa_ps_get(key, 0, sizeof(val), reinterpret_cast<void *>(&val), &actual_size));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    if (actual_size != sizeof(val))
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::ReadConfigValue(Key key, uint32_t & val)
{

    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    size_t actual_size = 0;
    CHIP_ERROR err     = PsaStatus2ChipError(psa_ps_get(key, 0, sizeof(val), reinterpret_cast<void *>(&val), &actual_size));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    if (actual_size != sizeof(val))
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::ReadConfigValue(Key key, uint64_t & val)
{
    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    size_t actual_size = 0;
    CHIP_ERROR err     = PsaStatus2ChipError(psa_ps_get(key, 0, sizeof(val), reinterpret_cast<void *>(&val), &actual_size));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    if (actual_size != sizeof(val))
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::ReadConfigValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    // Note: The system expect adding the terminator char.
    CHIP_ERROR err = ReadConfigValueBin(key, reinterpret_cast<uint8_t *>(buf), (bufSize - 1), outLen);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    buf[outLen] = 0;

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    return ReadConfigValueBin(key, buf, bufSize, outLen, 0);
}

CHIP_ERROR KVPsaPsStore::ReadConfigValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen, size_t offset)
{
    size_t keySize;
    if (!ConfigValueExists(key, keySize))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    if (buf != NULL)
    {
        size_t actual_size = 0;
        size_t readLength  = bufSize >= (keySize - offset) ? keySize - offset : bufSize;
        CHIP_ERROR err     = PsaStatus2ChipError(psa_ps_get(key, offset, readLength, reinterpret_cast<void *>(buf), &actual_size));
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }

        outLen = actual_size;
        if (bufSize < keySize - offset)
        {
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::ReadConfigValueCounter(Key counterId, uint32_t & val)
{
    Key key = kMinConfigKey_MatterCounter + counterId;

    if (!ConfigValueExists(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    size_t actual_size = 0;
    CHIP_ERROR err     = PsaStatus2ChipError(psa_ps_get(key, 0, sizeof(val), reinterpret_cast<void *>(&val), &actual_size));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    if (actual_size != sizeof(val))
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::WriteConfigValue(Key key, bool val)
{
    if (!ValidConfigKey(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    CHIP_ERROR err = PsaStatus2ChipError(psa_ps_set(key, sizeof(val), reinterpret_cast<void *>(&val), PSA_STORAGE_FLAG_NONE));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::WriteConfigValue(Key key, uint32_t val)
{
    if (!ValidConfigKey(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    CHIP_ERROR err = PsaStatus2ChipError(psa_ps_set(key, sizeof(val), reinterpret_cast<void *>(&val), PSA_STORAGE_FLAG_NONE));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::WriteConfigValue(Key key, uint64_t val)
{
    if (!ValidConfigKey(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    CHIP_ERROR err = PsaStatus2ChipError(psa_ps_set(key, sizeof(val), reinterpret_cast<void *>(&val), PSA_STORAGE_FLAG_NONE));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::WriteConfigValueStr(Key key, const char * str)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), (str != nullptr) ? strlen(str) : 0);
}

CHIP_ERROR KVPsaPsStore::WriteConfigValueStr(Key key, const char * str, size_t strLen)
{
    return WriteConfigValueBin(key, reinterpret_cast<const uint8_t *>(str), (strLen > 0) ? strLen : 1);
}

CHIP_ERROR KVPsaPsStore::WriteConfigValueBin(Key key, const uint8_t * data, size_t dataLen)
{
    if (!ValidConfigKey(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    // Two different behavior: If the pointer is not null, the value is updated
    // or create. If the pointer is null, the key is removed if it exist.
    if (data != nullptr)
    {
        CHIP_ERROR err = PsaStatus2ChipError(
            psa_ps_set(key, dataLen, const_cast<void *>(reinterpret_cast<const void *>(data)), PSA_STORAGE_FLAG_NONE));
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
    }
    else if (ConfigValueExists(key))
    {
        return ClearConfigValue(key);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::WriteConfigValueCounter(Key counterId, uint32_t val)
{
    Key key = kMinConfigKey_MatterCounter + counterId;

    if (!ValidConfigKey(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    CHIP_ERROR err = PsaStatus2ChipError(psa_ps_set(key, sizeof(val), reinterpret_cast<void *>(&val), PSA_STORAGE_FLAG_NONE));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::ClearConfigValue(Key key)
{
    if (!ValidConfigKey(key))
    {
        return CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    }

    CHIP_ERROR err = PsaStatus2ChipError(psa_ps_remove(key));
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::FactoryResetConfig()
{
    // Deletes all Config and KVS group objects.
    // Factory and Counter type objects are NOT deleted.

    if (Init() != CHIP_NO_ERROR)
    {
        return CHIP_ERROR_INTERNAL;
    }

    // Iterate over all the Matter Config objects and delete each one.
    CHIP_ERROR err = ForEachObject(kMinConfigKey_MatterConfig, kMaxConfigKey_MatterConfig, false,
                                   [](const Key & key, const size_t & size) -> CHIP_ERROR { return ClearConfigValue(key); });
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    // Iterate over all the Matter KVS objects and delete each one (including KVS key map).
    err = ForEachObject(kConfigKey_KvsStringKeyMap, kConfigKey_KvsLastKeySlot, false,
                        [](const Key & key, const size_t & size) -> CHIP_ERROR { return ClearConfigValue(key); });
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    mKvsStoredKeyMutex.Lock();
    memset(mKvsStoredKeyString, 0, sizeof(mKvsStoredKeyString));
    mKvsStoredKeyMutex.Unlock();

    return err;
}

void KVPsaPsStore::RunConfigUnitTest()
{
    // Run common unit test.
    ::chip::DeviceLayer::Internal::RunConfigUnitTest<KVPsaPsStore>();
}

bool KVPsaPsStore::ValidConfigKey(Key key)
{
    // Returns true if the key is in the Matter reserved key range.
    return ((key >= kMinMatterPsaPaKeyRegion) && (key <= kMaxMatterPsaPaKeyRegion));
}

bool KVPsaPsStore::ValidKvsKey(Key key)
{
    // Returns true if the key is in the KVS group range.
    return ((key >= kConfigKey_KvsFirstKeySlot) && (key <= kConfigKey_KvsLastKeySlot));
}

void KVPsaPsStore::KVSKeyMapUpdate()
{
    OnScheduleKVSKeyMapUpdate(nullptr, nullptr);
}

void KVPsaPsStore::ScheduleKVSKeyMapUpdate()
{
    CHIP_ERROR err = SystemLayer().StartTimer(
        std::chrono::duration_cast<System::Clock::Timeout>(System::Clock::Seconds32(KVS_KEY_MAP_UPDATE_DELAY_SEC)),
        OnScheduleKVSKeyMapUpdate, NULL);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Start KVS key map update timer failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void KVPsaPsStore::OnScheduleKVSKeyMapUpdate(System::Layer * systemLayer, void * appState)
{
    mKvsStoredKeyMutex.Lock();
    CHIP_ERROR err = WriteConfigValueBin(kConfigKey_KvsStringKeyMap, reinterpret_cast<const uint8_t *>(mKvsStoredKeyString),
                                         sizeof(mKvsStoredKeyString));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "KVS key map update failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    mKvsStoredKeyMutex.Unlock();
}

bool KVPsaPsStore::ConfigValueExists(Key key, size_t & size)
{
    if (!ValidConfigKey(key))
    {
        return false;
    }

    psa_storage_info_t info;
    psa_status_t status = psa_ps_get_info(key, &info);
    if (status == PSA_SUCCESS)
    {
        size = info.size;
    }

    return (status == PSA_SUCCESS);
}

bool KVPsaPsStore::ConfigValueExists(Key key)
{
    size_t size;
    return ConfigValueExists(key, size);
}

CHIP_ERROR KVPsaPsStore::ForEachObject(Key firstKey, Key lastKey, bool addNewRecord, ForEachObjectFunct funct)
{
    CHIP_ERROR err;
    // Iterates through the specified range of object key ids.
    // Invokes the callers CB function when appropriate.

    for (Key key = firstKey; key <= lastKey; ++key)
    {
        // Check if object with current key exists.
        psa_storage_info_t info;
        psa_status_t status = psa_ps_get_info(key, &info);
        switch (status)
        {
        case PSA_SUCCESS:
            if (!addNewRecord)
            {
                // Invoke the caller's function
                // (for retrieve,store,delete,enumerate GroupKey operations).
                err = funct(key, info.size);
            }
            break;
        case PSA_ERROR_DOES_NOT_EXIST:
            if (addNewRecord)
            {
                // Invoke caller's function
                // (for add GroupKey operation).
                err = funct(key, info.size);
            }
            break;
        default:
            err = PsaStatus2ChipError(status);
            break;
        }

        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KVPsaPsStore::PsaStatus2ChipError(psa_status_t status)
{
    CHIP_ERROR err;

    switch (status)
    {
    case PSA_SUCCESS:
        err = CHIP_NO_ERROR;
        break;
    case PSA_ERROR_DOES_NOT_EXIST:
        err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
        break;
    default:
        err = CHIP_ERROR(ChipError::Range::kPlatform, (status * (-1)));
        break;
    }

    return err;
}

#define CONVERT_INDEX_TO_KEY(index) (KVPsaPsStore::kConfigKey_KvsFirstKeySlot + index)
#define CONVERT_KEY_TO_INDEX(key) (key - KVPsaPsStore::kConfigKey_KvsFirstKeySlot)

KVPsaPsStoreKeyBuilder::KVPsaPsStoreKeyBuilder(const char * key)
{
    uint8_t firstEmptyKeySlot = KVS_MAX_ENTRIES;

    if (KVPsaPsStore::Init() != CHIP_NO_ERROR)
    {
        return;
    }

    valid    = false;
    existing = false;
    mKvsStoredKeyMutex.Lock();

    // Check if key already exists, find the first empty slot in the same time
    for (uint8_t keyIndex = 0; keyIndex < KVS_MAX_ENTRIES; keyIndex++)
    {
        if (strcmp(key, mKvsStoredKeyString[keyIndex]) == 0)
        {
            keyValue = CONVERT_INDEX_TO_KEY(keyIndex);
            if (KVPsaPsStore::ValidKvsKey(keyValue))
            {
                valid    = true;
                existing = true;
            }
            return;
        }

        if ((keyIndex < firstEmptyKeySlot) && (mKvsStoredKeyString[keyIndex][0] == 0))
        {
            firstEmptyKeySlot = keyIndex;
        }
    }

    // Key does not exist, reserve a slot for it
    keyValue = CONVERT_INDEX_TO_KEY(firstEmptyKeySlot);
    if (KVPsaPsStore::ValidKvsKey(keyValue))
    {
        memset(buffer, 0, sizeof(buffer));
        size_t keyLength = strlen(key) <= (sizeof(buffer) - 1) ? strlen(key) : (sizeof(buffer) - 1);
        if (keyLength < strlen(key))
        {
            ChipLogError(DeviceLayer, "String key length is too long. Truncated to %d bytes", (sizeof(buffer) - 1));
        }
        memcpy(buffer, key, keyLength);
        buffer[keyLength] = 0;

        valid = true;
    }
}

KVPsaPsStoreKeyBuilder::~KVPsaPsStoreKeyBuilder()
{
    mKvsStoredKeyMutex.Unlock();
}

void KVPsaPsStoreKeyBuilder::AddKey()
{
    if (!valid || existing)
    {
        return;
    }

    auto keyIndex = CONVERT_KEY_TO_INDEX(keyValue);
    memset(mKvsStoredKeyString[keyIndex], 0, sizeof(mKvsStoredKeyString[keyIndex]));
    Platform::CopyString(mKvsStoredKeyString[keyIndex], buffer);
    KVPsaPsStore::ScheduleKVSKeyMapUpdate();
}

void KVPsaPsStoreKeyBuilder::RemoveKey()
{
    if (!valid)
    {
        return;
    }

    auto keyIndex = CONVERT_KEY_TO_INDEX(keyValue);
    memset(mKvsStoredKeyString[keyIndex], 0, sizeof(mKvsStoredKeyString[keyIndex]));
    KVPsaPsStore::ScheduleKVSKeyMapUpdate();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
