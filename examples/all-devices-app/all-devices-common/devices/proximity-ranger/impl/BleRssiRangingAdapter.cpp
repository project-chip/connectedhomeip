/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include "BleRssiRangingAdapter.h"

#include <crypto/RandUtils.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <string.h>

BleRssiRangingAdapter::~BleRssiRangingAdapter() = default;

CHIP_ERROR BleRssiRangingAdapter::Init(chip::PersistentStorageDelegate * store)
{
    mpStore = store;

    chip::StorageKeyName key = chip::DefaultStorageKeyAllocator::ProximityRangingBleDeviceId();

    if (mpStore != nullptr)
    {
        uint16_t size = sizeof(mBleDeviceId);
        if (mpStore->SyncGetKeyValue(key.KeyName(), &mBleDeviceId, size) != CHIP_NO_ERROR || size != sizeof(mBleDeviceId))
        {
            mBleDeviceId = kInvalidBleDeviceId;
        }
    }

    if (mBleDeviceId == kInvalidBleDeviceId)
    {
        ReturnErrorOnFailure(GenerateBleDeviceId());
        // Persist newly generated BLE Device ID if storage is provided
        if (mpStore != nullptr)
        {
            ReturnErrorOnFailure(mpStore->SyncSetKeyValue(key.KeyName(), &mBleDeviceId, sizeof(mBleDeviceId)));
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BleRssiRangingAdapter::EncodeBeaconPayload(uint64_t bleDeviceId, uint16_t messageCounter, int8_t txPower,
                                                      chip::ByteSpan sessionKey,
                                                      chip::Ble::ChipBLEProximityRangingIdentificationInfo & outPayload)
{
    outPayload.Init();
    outPayload.SetMsgCounter(messageCounter);
    outPayload.SetTxPower(txPower);

    // TODO: Apply HMAC-based obfuscation using sessionKey. For now, encode the
    // BLEDeviceId in plain big-endian form in the first 8 bytes of the field.
    uint8_t plain[16] = {};
    chip::Encoding::BigEndian::Put64(plain, bleDeviceId);
    outPayload.SetObfuscatedBLEDeviceId(plain);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BleRssiRangingAdapter::DecodeBeaconPayload(const chip::Ble::ChipBLEProximityRangingIdentificationInfo & payload,
                                                      uint64_t candidateBleDeviceId, chip::ByteSpan sessionKey)
{
    // TODO: Apply HMAC-based verification using sessionKey. For now, compare
    // the candidate directly against the plain-encoded BLEDeviceId.
    uint8_t expected[16] = {};
    chip::Encoding::BigEndian::Put64(expected, candidateBleDeviceId);

    if (memcmp(payload.GetObfuscatedBLEDeviceId(), expected, sizeof(expected)) != 0)
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BleRssiRangingAdapter::GenerateBleDeviceId()
{
    static constexpr uint8_t kMaxGenerationAttempts = 3;
    for (uint8_t i = 0; i < kMaxGenerationAttempts; i++)
    {
        uint64_t id = chip::Crypto::GetRandU64();
        if (id != kInvalidBleDeviceId)
        {
            mBleDeviceId = id;

            if (mpStore == nullptr)
            {
                ChipLogError(DeviceLayer, "Warning: no persistent storage, BleDeviceId randomly generated on each reboot");
            }
            return CHIP_NO_ERROR;
        }
    }
    ChipLogError(DeviceLayer, "GenerateBleDeviceId failed generation after 3 attempts");
    return CHIP_ERROR_INTERNAL;
}

std::optional<uint64_t> BleRssiRangingAdapter::GetDeviceId()
{
    if (mBleDeviceId == kInvalidBleDeviceId)
    {
        RETURN_SAFELY_IGNORED GenerateBleDeviceId();
    }
    return mBleDeviceId;
}
