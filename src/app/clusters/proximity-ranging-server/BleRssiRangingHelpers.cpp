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

#include <crypto/CHIPCryptoPAL.h>
#include <crypto/RandUtils.h>
#include <lib/core/CHIPEncoding.h>
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

CHIP_ERROR HmacObfuscateBleDeviceId(uint64_t bleDeviceId, uint16_t messageCounter, ByteSpan sessionKey,
                                    MutableByteSpan outObfuscatedId)
{
    VerifyOrReturnError(outObfuscatedId.size() >= kBleObfuscatedIdLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    // An empty key is accepted by some HMAC implementations but is never
    // meaningful here and would silently produce a keyless tag.
    VerifyOrReturnError(!sessionKey.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    // HMAC message: BLEDeviceID (8 bytes, big-endian) || BLERBCMessageCounter (2 bytes, big-endian).
    uint8_t message[sizeof(bleDeviceId) + sizeof(messageCounter)] = {};
    Encoding::BigEndian::Put64(message, bleDeviceId);
    Encoding::BigEndian::Put16(message + sizeof(bleDeviceId), messageCounter);

    uint8_t hmacTag[Crypto::kSHA256_Hash_Length] = {};
    Crypto::HMAC_sha hmac;
    ReturnErrorOnFailure(
        hmac.HMAC_SHA256(sessionKey.data(), sessionKey.size(), message, sizeof(message), hmacTag, sizeof(hmacTag)));

    // The ObfuscatedBLEDeviceId field is 16 bytes; take the first 16 bytes of the 32-byte HMAC output.
    static_assert(sizeof(hmacTag) >= kBleObfuscatedIdLength,
                  "HMAC-SHA256 output must be at least as large as ObfuscatedBLEDeviceId");
    memcpy(outObfuscatedId.data(), hmacTag, kBleObfuscatedIdLength);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PlaintextObfuscateBleDeviceId(uint64_t bleDeviceId, uint16_t messageCounter, ByteSpan sessionKey,
                                         MutableByteSpan outObfuscatedId)
{
    VerifyOrReturnError(outObfuscatedId.size() >= kBleObfuscatedIdLength, CHIP_ERROR_BUFFER_TOO_SMALL);
    // Insecure bring-up path: publish the BLEDeviceID in plaintext, no key needed.
    (void) messageCounter;
    (void) sessionKey;
    memset(outObfuscatedId.data(), 0, outObfuscatedId.size());
    Encoding::BigEndian::Put64(outObfuscatedId.data(), bleDeviceId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR EncodeBeaconPayload(uint64_t bleDeviceId, uint16_t messageCounter, int8_t txPower, ByteSpan sessionKey,
                               Ble::ChipBLEProximityRangingIdentificationInfo & outPayload, ObfuscateBleDeviceIdFunction obfuscate)
{
    outPayload.Init();
    outPayload.SetMsgCounter(messageCounter);
    outPayload.SetTxPower(txPower);

    uint8_t obfuscatedId[kBleObfuscatedIdLength] = {};
    ReturnErrorOnFailure(obfuscate(bleDeviceId, messageCounter, sessionKey, MutableByteSpan(obfuscatedId)));
    outPayload.SetObfuscatedBLEDeviceId(obfuscatedId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeBeaconPayload(const Ble::ChipBLEProximityRangingIdentificationInfo & payload, uint64_t candidateBleDeviceId,
                               ByteSpan sessionKey, ObfuscateBleDeviceIdFunction obfuscate)
{
    // Recompute the expected obfuscated ID from the candidate BLEDeviceID and the
    // message counter carried in the received payload, then compare against the
    // ObfuscatedBLEDeviceId in the payload to verify the advertiser's identity.
    uint8_t expected[kBleObfuscatedIdLength] = {};
    ReturnErrorOnFailure(obfuscate(candidateBleDeviceId, payload.GetMsgCounter(), sessionKey, MutableByteSpan(expected)));

    // Use a constant-time comparison to avoid leaking, via timing, how many
    // leading bytes of the tag matched.
    if (!Crypto::IsBufferContentEqualConstantTime(payload.GetObfuscatedBLEDeviceId(), expected, kBleObfuscatedIdLength))
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
