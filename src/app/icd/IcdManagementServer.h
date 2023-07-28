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
    void Init(uint32_t idle_interval, uint32_t active_interval, uint16_t active_threshold, uint32_t count,
              uint16_t fabric_clients_supported)
    {
        mIdleInterval           = idle_interval;
        mActiveInterval         = active_interval;
        mActiveThreshold        = active_threshold;
        mIcdCounter             = count;
        mFabricClientsSupported = fabric_clients_supported;
    }

    uint32_t GetIdleModeInterval() { return mIdleInterval; }

    uint32_t GetActiveModeInterval() { return mActiveInterval; }

    uint16_t GetActiveModeThreshold() { return mActiveThreshold; }

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
    uint32_t mIdleInterval           = CHIP_CONFIG_ICD_IDLE_MODE_INTERVAL;
    uint32_t mActiveInterval         = CHIP_CONFIG_ICD_ACTIVE_MODE_INTERVAL;
    uint16_t mActiveThreshold        = CHIP_CONFIG_ICD_ACTIVE_MODE_THRESHOLD;
    uint32_t mIcdCounter             = 0;
    uint16_t mFabricClientsSupported = CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC;
};

} // namespace chip
