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
#include <devices/proximity-ranger/RangingAdapter.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/Span.h>

/**
 * Base class for BLE RSSI ranging adapters.
 *
 * Provides beacon payload encoding/decoding and technology identification.
 * Platform subclasses implement session management (StartSession, StopSession,
 * StopAllSessions, GetActiveSessionIds) and BLE hardware control directly.
 */
class BleRssiRangingAdapter : public chip::app::Clusters::ProximityRanging::RangingAdapter
{
public:
    /// A BLE Device ID of zero indicates the ID has not been generated or is invalid.
    static constexpr uint64_t kInvalidBleDeviceId = 0;
    /**
     * Encode a proximity ranging beacon advertisement payload.
     *
     * Obfuscates the BLEDeviceId using the provided session key and populates
     * the output struct with the encoded payload ready for BLE advertising.
     *
     * @param bleDeviceId    The device's 64-bit BLE Device ID to obfuscate.
     * @param messageCounter The 16-bit message counter for replay protection.
     * @param txPower        The transmit power in dBm.
     * @param sessionKey     The session key used for HMAC obfuscation.
     * @param outPayload     The encoded beacon payload struct.
     * @return CHIP_NO_ERROR on success, or an error if encoding fails.
     */
    static CHIP_ERROR EncodeBeaconPayload(uint64_t bleDeviceId, uint16_t messageCounter, int8_t txPower, chip::ByteSpan sessionKey,
                                          chip::Ble::ChipBLEProximityRangingIdentificationInfo & outPayload);

    /**
     * Decode and verify a proximity ranging beacon advertisement payload.
     *
     * Verifies that the received beacon payload matches the expected candidate
     * BLEDeviceId by computing the obfuscation with the provided session key
     * and comparing against the payload's obfuscated field.
     *
     * @param payload            The received beacon payload to verify.
     * @param candidateBleDeviceId The expected peer BLE Device ID to verify against.
     * @param sessionKey         The session key used for HMAC verification.
     * @return CHIP_NO_ERROR if the payload matches the candidate, or
     *         CHIP_ERROR_NOT_FOUND if the obfuscated ID does not match.
     */
    static CHIP_ERROR DecodeBeaconPayload(const chip::Ble::ChipBLEProximityRangingIdentificationInfo & payload,
                                          uint64_t candidateBleDeviceId, chip::ByteSpan sessionKey);

    ~BleRssiRangingAdapter() override;

    /**
     * Initialize the adapter with persistent storage for BLE Device ID.
     * Retrieves persisted ID (if available) or generates random value if none already exist.
     * Must be called before @e GetDeviceId to ensure persistence, otherwise new BLE Device ID will be generated.
     * @note Providing no persistent storage results in random ID generation after every reboot.
     */
    CHIP_ERROR Init(chip::PersistentStorageDelegate * store);

    virtual std::optional<uint64_t> GetDeviceId() override;

    chip::app::Clusters::ProximityRanging::RangingTechEnum GetTechnology() const override
    {
        return chip::app::Clusters::ProximityRanging::RangingTechEnum::kBLEBeaconRSSIRanging;
    }

    chip::app::Clusters::ProximityRanging::Structs::RangingCapabilitiesStruct::Type GetCapabilities() const override
    {
        chip::app::Clusters::ProximityRanging::Structs::RangingCapabilitiesStruct::Type capabilities = {};
        capabilities.technology    = chip::app::Clusters::ProximityRanging::RangingTechEnum::kBLEBeaconRSSIRanging;
        capabilities.frequencyBand = chip::BitMask<chip::app::Clusters::ProximityRanging::RadioBandBitmap>(
            chip::app::Clusters::ProximityRanging::RadioBandBitmap::k2g4);
        capabilities.periodicRangingSupport = true;
        return capabilities;
    }

protected:
    uint64_t mBleDeviceId                     = kInvalidBleDeviceId;
    chip::PersistentStorageDelegate * mpStore = nullptr;

private:
    CHIP_ERROR GenerateBleDeviceId();
};
