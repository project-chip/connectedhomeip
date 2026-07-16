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
#include <lib/support/BufferReader.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/Span.h>

#include <cstring>

/**
 * Build-time switch selecting how the BLEDeviceID is carried in the beacon
 * advertisement payload.
 *
 * By default (undefined or 0) the BLEDeviceID is obfuscated with
 * HMAC-SHA256(sessionKey, BLEDeviceID || messageCounter) so passive observers
 * cannot recover or correlate the raw identifier across beacons.
 *
 * When defined as 1, obfuscation is disabled and the raw BLEDeviceID is placed
 * in the advertisement in plaintext. This exists only to let two ranging
 * devices interoperate during bring-up/interop without a shared session key;
 * both peers must set this flag. It leaks the device identifier on air and MUST
 * NOT be enabled in production.
 */
#ifndef CHIP_CLUSTER_PROXIMITY_RANGING_DISABLE_SECURE_BLE_BEACONING
#define CHIP_CLUSTER_PROXIMITY_RANGING_DISABLE_SECURE_BLE_BEACONING 0
#endif

/// Number of CSPRNG attempts GenerateBleDeviceId will make before
/// giving up. The probability of three consecutive zero draws from a
/// well-seeded 64-bit RNG is 2^-192, so this is effectively a safety net for
/// pathological RNG implementations.
static constexpr uint8_t kBleDeviceIdGenerationAttempts = 3;

/// Length of the ObfuscatedBLEDeviceId field carried in the advertisement payload.
/// Shared by both the secure (HMAC) and plaintext beaconing paths.
constexpr size_t kBleObfuscatedIdLength = sizeof(chip::Ble::ChipBLEProximityRangingIdentificationInfo::ObfuscatedBLEDeviceId);

namespace {

/**
 * Fill @p outObfuscatedId with the value that goes into the beacon's
 * ObfuscatedBLEDeviceId field for a given (BLEDeviceID, messageCounter) following
 * the Proximity Ranging cluster specification.
 *
 * When CHIP_CLUSTER_PROXIMITY_RANGING_DISABLE_SECURE_BLE_BEACONING is set for
 * insecure testing, the raw bleDeviceId in ble-endian form is set into the obfuscatedId field.
 */
CHIP_ERROR ComputeObfuscatedBleDeviceId(uint64_t bleDeviceId, uint16_t messageCounter, chip::ByteSpan sessionKey,
                                        uint8_t (&outObfuscatedId)[kBleObfuscatedIdLength])
{
#if CHIP_CLUSTER_PROXIMITY_RANGING_DISABLE_SECURE_BLE_BEACONING
    // Test-only: publish the BLEDeviceID in plaintext, no session key needed.
    (void) sessionKey;
    memset(outObfuscatedId, 0, kBleObfuscatedIdLength);
    chip::Encoding::BigEndian::Put64(outObfuscatedId, bleDeviceId);
    return CHIP_NO_ERROR;
#else
    // HMAC message: BLEDeviceID (8 bytes, big-endian) || BLERBCMessageCounter (2 bytes, big-endian).
    uint8_t message[sizeof(bleDeviceId) + sizeof(messageCounter)] = {};
    chip::Encoding::BigEndian::Put64(message, bleDeviceId);
    chip::Encoding::BigEndian::Put16(message + sizeof(bleDeviceId), messageCounter);

    uint8_t hmacTag[chip::Crypto::kSHA256_Hash_Length] = {};
    chip::Crypto::HMAC_sha hmac;
    ReturnErrorOnFailure(
        hmac.HMAC_SHA256(sessionKey.data(), sessionKey.size(), message, sizeof(message), hmacTag, sizeof(hmacTag)));

    // The ObfuscatedBLEDeviceId field is 16 bytes; take the first 16 bytes of the 32-byte HMAC output.
    static_assert(sizeof(hmacTag) >= kBleObfuscatedIdLength,
                  "HMAC-SHA256 output must be at least as large as ObfuscatedBLEDeviceId");
    memcpy(outObfuscatedId, hmacTag, kBleObfuscatedIdLength);
    return CHIP_NO_ERROR;
#endif // CHIP_CLUSTER_PROXIMITY_RANGING_DISABLE_SECURE_BLE_BEACONING
}

} // namespace

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

    uint8_t obfuscatedId[kBleObfuscatedIdLength] = {};
    ReturnErrorOnFailure(ComputeObfuscatedBleDeviceId(bleDeviceId, messageCounter, sessionKey, obfuscatedId));
    outPayload.SetObfuscatedBLEDeviceId(obfuscatedId);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeBeaconPayload(const Ble::ChipBLEProximityRangingIdentificationInfo & payload, uint64_t candidateBleDeviceId,
                               ByteSpan sessionKey)
{
    // Recompute the expected obfuscated ID from the candidate BLEDeviceID and the
    // message counter carried in the received payload, then compare against the
    // ObfuscatedBLEDeviceId in the payload to verify the advertiser's identity.
    uint8_t expected[kBleObfuscatedIdLength] = {};
    ReturnErrorOnFailure(ComputeObfuscatedBleDeviceId(candidateBleDeviceId, payload.GetMsgCounter(), sessionKey, expected));

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
