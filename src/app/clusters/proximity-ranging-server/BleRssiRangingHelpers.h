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

#include <cstddef>
#include <cstdint>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {
namespace BleRssi {

/// Sentinel value reserved by the spec - a generated BLEDeviceID MUST NOT
/// equal this value. Platform adapters should re-roll on collision.
inline constexpr uint64_t kInvalidBleDeviceId = 0;

/// Length of the ObfuscatedBLEDeviceId field carried in the advertisement
/// payload. An obfuscation function fills exactly this many bytes.
inline constexpr size_t kBleObfuscatedIdLength = sizeof(Ble::ChipBLEProximityRangingIdentificationInfo::ObfuscatedBLEDeviceId);

/**
 * Function type to produce BLE RSSI ObfuscatedBLEDeviceId beacon field from a
 * (BLEDeviceID, messageCounter, sessionKey) tuple.
 *
 * @param bleDeviceId     The BLEDeviceID to obfuscate.
 * @param messageCounter  Beacon message counter mixed into the output.
 * @param sessionKey      Per-session key.
 * @param outObfuscatedId Output span of exactly kBleObfuscatedIdLength bytes.
 *
 * @return CHIP_NO_ERROR on success; an error otherwise (e.g.
 *         CHIP_ERROR_INVALID_ARGUMENT for an empty key, or a crypto error).
 */
using ObfuscateBleDeviceIdFunction = CHIP_ERROR (*)(uint64_t bleDeviceId, uint16_t messageCounter, ByteSpan sessionKey,
                                                    MutableByteSpan outObfuscatedId);

/**
 * Produces the ObfuscatedBLEDeviceId according to the Proximity Ranging cluster specification
 * using HMAC-SHA256(sessionKey, BLEDeviceID (big-endian) || messageCounter (big-endian)).
 *
 * @return CHIP_NO_ERROR on success; CHIP_ERROR_INVALID_ARGUMENT if @p sessionKey
 *         is empty; CHIP_ERROR_BUFFER_TOO_SMALL if @p outObfuscatedId is too small;
 *         a crypto error if HMAC computation fails.
 */
CHIP_ERROR HmacObfuscateBleDeviceId(uint64_t bleDeviceId, uint16_t messageCounter, ByteSpan sessionKey,
                                    MutableByteSpan outObfuscatedId);

/**
 * Produces raw BLEDeviceId in big-endian form and ignores @p sessionKey.
 *
 * @note This leaks the device identifier over air and MUST NOT be used in production; it exists
 * for testing purposes only without requiring shared session key. Both ranging peers need to use this
 * unsecure function when testing.
 */
CHIP_ERROR PlaintextObfuscateBleDeviceId(uint64_t bleDeviceId, uint16_t messageCounter, ByteSpan sessionKey,
                                         MutableByteSpan outObfuscatedId);

/**
 * Encode a BLEDeviceID into a Proximity Ranging BLE advertisement payload.
 *
 * Populates @p outPayload with the OpCode, message counter, Tx power, and
 * obfuscated BLEDeviceID fields per the Matter Proximity Ranging spec's
 * advertisement format. The BLEDeviceID is obfuscated by @p obfuscate, which
 * defaults to the secure HMAC-SHA256 strategy.
 *
 * @param bleDeviceId    The local device's 64-bit BLEDeviceID.
 * @param messageCounter Monotonic counter for replay protection.
 * @param txPower        Transmit power in dBm reported in the advertisement.
 * @param sessionKey     Per-session key passed through to @p obfuscate.
 * @param outPayload     Output. Initialised in-place; caller need not pre-init.
 * @param obfuscate      Obfuscation strategy; defaults to HmacObfuscateBleDeviceId.
 *
 * @return CHIP_NO_ERROR on success; whatever error @p obfuscate returns.
 */
CHIP_ERROR EncodeBeaconPayload(uint64_t bleDeviceId, uint16_t messageCounter, int8_t txPower, ByteSpan sessionKey,
                               Ble::ChipBLEProximityRangingIdentificationInfo & outPayload,
                               ObfuscateBleDeviceIdFunction obfuscate = HmacObfuscateBleDeviceId);

/**
 * Verify that a received BLE Proximity Ranging advertisement matches a
 * candidate BLEDeviceID.
 *
 * Used by adapters in BLE-Scanning role to test whether an observed beacon
 * was emitted by a peer whose BLEDeviceID is @p candidateBleDeviceId. Mirrors
 * EncodeBeaconPayload: it recomputes the expected obfuscated field with the
 * same @p obfuscate strategy and constant-time compares it against the payload.
 * The caller MUST pass the same strategy used to encode.
 *
 * @param payload              The decoded advertisement payload.
 * @param candidateBleDeviceId The BLEDeviceID to verify against.
 * @param sessionKey           Per-session key passed through to @p obfuscate.
 * @param obfuscate            Obfuscation strategy; defaults to HmacObfuscateBleDeviceId.
 *
 * @return CHIP_NO_ERROR on match, CHIP_ERROR_NOT_FOUND on mismatch; whatever
 *         error @p obfuscate returns if the expected field cannot be computed.
 */
CHIP_ERROR DecodeBeaconPayload(const Ble::ChipBLEProximityRangingIdentificationInfo & payload, uint64_t candidateBleDeviceId,
                               ByteSpan sessionKey, ObfuscateBleDeviceIdFunction obfuscate = HmacObfuscateBleDeviceId);

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
