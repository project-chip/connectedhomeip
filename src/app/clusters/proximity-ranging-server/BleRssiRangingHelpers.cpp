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

// BleRssiRangingHelpers - utilities shared by all platform adapters that
// implement the BLE Beacon RSSI Ranging technology. These helpers encapsulate
// the spec-defined advertisement payload encoding and BLEDeviceID generation
// so each adapter does not have to re-implement them.
//
// Per docs/guides/writing_clusters.md ("Provide Helper Abstractions"), this
// keeps adapter code small and uniform: the platform layer wires in radio
// transmit/receive, and these helpers handle the value transforms.

#include "BleRssiRangingHelpers.h"

#include <crypto/RandUtils.h>
#include <lib/support/BufferReader.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Span.h>

#include <cstring>

/// Number of CSPRNG attempts GenerateBleDeviceId will make before
/// giving up. The probability of three consecutive zero draws from a
/// well-seeded 64-bit RNG is 2^-192, so this is effectively a safety net for
/// pathological RNG implementations.
static constexpr uint8_t kBleDeviceIdGenerationAttempts = 3;

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {
namespace BleRssi {

CHIP_ERROR EncodeBeaconPayload(uint64_t bleDeviceId, uint16_t messageCounter, int8_t txPower, ByteSpan sessionKey,
                               Ble::ChipBLEProximityRangingIdentificationInfo & outPayload)
{
    outPayload.Init();
    outPayload.SetMsgCounter(messageCounter);
    outPayload.SetTxPower(txPower);

    // TODO(spec): Apply HMAC-SHA256 obfuscation keyed on `sessionKey` over
    // (bleDeviceId || messageCounter) and copy the truncated tag into the
    // 16-byte ObfuscatedBLEDeviceId field. Until that lands, encode the
    // BLEDeviceID in plain big-endian form in the first 8 bytes so adapter
    // and cert-test plumbing can be exercised end-to-end.
    (void) sessionKey;

    uint8_t plain[16] = {};
    Encoding::BigEndian::Put64(plain, bleDeviceId);
    outPayload.SetObfuscatedBLEDeviceId(plain);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeBeaconPayload(const Ble::ChipBLEProximityRangingIdentificationInfo & payload, uint64_t candidateBleDeviceId,
                               ByteSpan sessionKey)
{
    // TODO(spec): mirror EncodeBeaconPayload - when HMAC obfuscation lands,
    // recompute the expected tag from `candidateBleDeviceId`, `payload`'s
    // message counter, and `sessionKey`, then constant-time compare against
    // payload.ObfuscatedBLEDeviceId.
    (void) sessionKey;

    uint8_t expected[16] = {};
    Encoding::BigEndian::Put64(expected, candidateBleDeviceId);

    if (memcmp(payload.GetObfuscatedBLEDeviceId(), expected, sizeof(expected)) != 0)
    {
        return CHIP_ERROR_NOT_FOUND;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenerateBleDeviceId(uint64_t & outBleDeviceId)
{
    for (uint8_t i = 0; i < kBleDeviceIdGenerationAttempts; ++i)
    {
        uint64_t id = Crypto::GetRandU64();
        if (id != kInvalidBleDeviceId)
        {
            outBleDeviceId = id;
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR RetrieveGenerateBleDeviceId(PersistentStorageDelegate & storage, uint64_t & outBleDeviceId)
{
    StorageKeyName key = DefaultStorageKeyAllocator::ProximityRangingBleDeviceId();
    uint16_t size      = sizeof(outBleDeviceId);
    if (storage.SyncGetKeyValue(key.KeyName(), &outBleDeviceId, size) != CHIP_NO_ERROR || size != sizeof(outBleDeviceId))
    {
        outBleDeviceId = BleRssi::kInvalidBleDeviceId;
    }

    if (outBleDeviceId == BleRssi::kInvalidBleDeviceId)
    {
        ReturnErrorOnFailure(GenerateBleDeviceId(outBleDeviceId));
        ReturnErrorOnFailure(storage.SyncSetKeyValue(key.KeyName(), &outBleDeviceId, sizeof(outBleDeviceId)));
    }
    return CHIP_NO_ERROR;
}

} // namespace BleRssi
} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
