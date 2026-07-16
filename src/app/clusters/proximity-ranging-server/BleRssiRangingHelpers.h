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
#pragma once

#include <ble/Ble.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/Span.h>

#include <cstdint>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {
namespace BleRssi {

/// Sentinel value reserved by the spec - a generated BLEDeviceID MUST NOT
/// equal this value. Platform adapters should re-roll on collision.
inline constexpr uint64_t kInvalidBleDeviceId = 0;

/**
 * Encode a BLEDeviceID into a Proximity Ranging BLE advertisement payload.
 *
 * Populates @p outPayload with the OpCode, message counter, Tx power, and
 * obfuscated BLEDeviceID fields per the Matter Proximity Ranging spec's
 * advertisement format. The BLEDeviceID is obfuscated with HMAC-SHA256 keyed
 * on @p sessionKey unless the test-only
 * CHIP_CLUSTER_PROXIMITY_RANGING_DISABLE_SECURE_BLE_BEACONING flag is set, in which
 * case it is carried in plaintext and @p sessionKey is ignored.
 *
 * @param bleDeviceId    The local device's 64-bit BLEDeviceID.
 * @param messageCounter Monotonic counter for replay protection.
 * @param txPower        Transmit power in dBm reported in the advertisement.
 * @param sessionKey     Per-session HMAC key.
 * @param outPayload     Output. Initialised in-place; caller need not pre-init.
 *
 * @return CHIP_NO_ERROR on success; a crypto error if HMAC computation fails.
 */
CHIP_ERROR EncodeBeaconPayload(uint64_t bleDeviceId, uint16_t messageCounter, int8_t txPower, ByteSpan sessionKey,
                               Ble::ChipBLEProximityRangingIdentificationInfo & outPayload);

/**
 * Verify that a received BLE Proximity Ranging advertisement matches a
 * candidate BLEDeviceID.
 *
 * Used by adapters in BLE-Scanning role to test whether an observed beacon
 * was emitted by a peer whose BLEDeviceID is @p candidateBleDeviceId. Mirrors
 * EncodeBeaconPayload: it recomputes the expected obfuscated field the same way
 * (HMAC by default, plaintext under
 * CHIP_CLUSTER_PROXIMITY_RANGING_DISABLE_SECURE_BLE_BEACONING) and constant-time
 * compares it against the payload.
 *
 * @param payload              The decoded advertisement payload.
 * @param candidateBleDeviceId The BLEDeviceID to verify against.
 * @param sessionKey           Per-session HMAC key.
 *
 * @return CHIP_NO_ERROR on match, CHIP_ERROR_NOT_FOUND on mismatch; a crypto
 *         error if HMAC computation fails.
 */
CHIP_ERROR DecodeBeaconPayload(const Ble::ChipBLEProximityRangingIdentificationInfo & payload, uint64_t candidateBleDeviceId,
                               ByteSpan sessionKey);

/**
 * Generate a non-zero random 64-bit BLEDeviceID using the platform CSPRNG.
 *
 * The caller is responsible for persisting the generated value across reboots
 * - the spec requires BLEDeviceID stability so peers can correlate beacons
 * over time. This helper deliberately does not touch storage so it remains a
 * pure function over the RNG. See RetrieveGenerateBleDeviceId() for the
 * persistence-aware variant.
 *
 * The implementation makes a small bounded number of attempts to avoid the
 * reserved kInvalidBleDeviceId value; failure is therefore only possible with
 * a pathological RNG.
 *
 * @param outBleDeviceId Output, valid only on CHIP_NO_ERROR.
 *
 * @return CHIP_NO_ERROR on success;
 *         CHIP_ERROR_INTERNAL if every draw returned kInvalidBleDeviceId.
 */
CHIP_ERROR GenerateBleDeviceId(uint64_t & outBleDeviceId);

/**
 * Return a stable BLEDeviceID, generating and persisting one on first use.
 *
 * Reads the BLEDeviceID previously stored under
 * DefaultStorageKeyAllocator::ProximityRangingBleDeviceId(). If no value is
 * present, or the stored value is kInvalidBleDeviceId, a fresh ID is generated
 * via GenerateBleDeviceId() and written back to @p storage so subsequent calls
 * (across reboots) return the same value.
 *
 * Intended for BLE Beacon RSSI adapters that need a per-device identifier
 * stable across reboots without each adapter re-implementing the load /
 * generate / persist sequence.
 *
 * @param storage         Backing store for the persisted BLEDeviceID. Must
 *                        outlive the call.
 * @param outBleDeviceId  Output, valid only on CHIP_NO_ERROR.
 *
 * @return CHIP_NO_ERROR on success;
 *         CHIP_ERROR_INTERNAL if generation failed (see GenerateBleDeviceId);
 *         any error from PersistentStorageDelegate::SyncSetKeyValue if the
 *         newly generated ID could not be persisted.
 */
CHIP_ERROR RetrieveGenerateBleDeviceId(PersistentStorageDelegate & storage, uint64_t & outBleDeviceId);

} // namespace BleRssi
} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
