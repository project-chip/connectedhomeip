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

#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/icd/ICDData.h>
#include <app/icd/ICDMonitoringTable.h>
#include <app/util/basic-types.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>

using chip::Protocols::InteractionModel::Status;

class ICDManagementServer
{
public:
    ICDManagementServer() = default;

    static void Init(chip::PersistentStorageDelegate & storage, chip::Crypto::SymmetricKeystore * symmetricKeystore,
                     chip::ICDData & icdData);
    Status RegisterClient(chip::FabricIndex fabric_index, chip::NodeId node_id, uint64_t monitored_subject, chip::ByteSpan key,
                          chip::Optional<chip::ByteSpan> verification_key, bool is_admin, uint32_t & icdCounter);

    Status UnregisterClient(chip::FabricIndex fabric_index, chip::NodeId node_id, chip::Optional<chip::ByteSpan> verificationKey,
                            bool is_admin);

    Status StayActiveRequest(chip::FabricIndex fabric_index);

    CHIP_ERROR CheckAdmin(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                          bool & is_admin);

private:
    /**
     * @brief Triggers table update events to notify subscribers that an entry was added or removed
     *        from the ICDMonitoringTable.
     */
    void TriggerICDMTableUpdatedEvent();

    static chip::PersistentStorageDelegate * mStorage;
    static chip::Crypto::SymmetricKeystore * mSymmetricKeystore;
    static chip::ICDData * mICDData;
};
