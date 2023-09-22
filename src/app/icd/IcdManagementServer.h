/**
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

#pragma once

#include <app/util/basic-types.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {

using chip::Protocols::InteractionModel::Status;

class IcdManagementServer
{
public:
    uint32_t GetIdleModeInterval() { return mIdleInterval_s; }

    uint32_t GetActiveModeInterval() { return mActiveInterval_ms; }

    uint16_t GetActiveModeThreshold() { return mActiveThreshold_ms; }

    uint32_t GetICDCounter() { return mIcdCounter; }

    void SetICDCounter(uint32_t count) { mIcdCounter = count; }

    uint16_t GetClientsSupportedPerFabric() { return mFabricClientsSupported; }

    Status RegisterClient(PersistentStorageDelegate & storage, FabricIndex fabric_index, chip::NodeId node_id,
                          uint64_t monitored_subject, chip::ByteSpan key, Optional<chip::ByteSpan> verification_key, bool is_admin);

    Status UnregisterClient(PersistentStorageDelegate & storage, FabricIndex fabric_index, chip::NodeId node_id,
                            Optional<chip::ByteSpan> verificationKey, bool is_admin);

    Status StayActiveRequest(FabricIndex fabric_index);

    static IcdManagementServer & GetInstance() { return mInstance; }

private:
    IcdManagementServer() = default;

    static IcdManagementServer mInstance;

    static_assert((CHIP_CONFIG_ICD_IDLE_MODE_INTERVAL_SEC) <= 64800,
                  "Spec requires the IdleModeInterval to be equal or inferior to 64800s.");
    static_assert((CHIP_CONFIG_ICD_IDLE_MODE_INTERVAL_SEC) >= 1,
                  "Spec requires the IdleModeInterval to be equal or greater to 1s.");
    uint32_t mIdleInterval_s = CHIP_CONFIG_ICD_IDLE_MODE_INTERVAL_SEC; // in seconds.

    static_assert((CHIP_CONFIG_ICD_ACTIVE_MODE_INTERVAL_MS) <= (CHIP_CONFIG_ICD_IDLE_MODE_INTERVAL_SEC * 1000),
                  "Spec requires the IdleModeInterval be equal or greater to the ActiveModeInterval.");
    static_assert((CHIP_CONFIG_ICD_ACTIVE_MODE_INTERVAL_MS) >= 300,
                  "Spec requires the ActiveModeThreshold to be equal or greater to 300ms");
    uint32_t mActiveInterval_ms = CHIP_CONFIG_ICD_ACTIVE_MODE_INTERVAL_MS; // in milliseconds

    static_assert((CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS) >= 300,
                  "Spec requires the ActiveModeThreshold to be equal or greater to 300ms.");
    uint16_t mActiveThreshold_ms = CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD_MS; // in milliseconds

    uint32_t mIcdCounter = 0;

    static_assert((CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC) >= 1,
                  "Spec requires the minimum of supported clients per fabric be equal or greater to 1.");
    uint16_t mFabricClientsSupported = CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC;
};

} // namespace chip
