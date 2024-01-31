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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/icd/server/ICDConfigurationData.h>
#include <app/icd/server/ICDMonitoringTable.h>
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
                     chip::ICDConfigurationData & ICDConfigurationData);

    /**
     * @brief Function that executes the business logic of the RegisterClient Command
     *
     * @param[out] icdCounter If function succeeds, icdCounter will have the current value of the ICDCounter stored in the
     * ICDConfigurationData If function fails, icdCounter will be unchanged
     * @return Status
     */
    Status RegisterClient(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                          const chip::app::Clusters::IcdManagement::Commands::RegisterClient::DecodableType & commandData,
                          uint32_t & icdCounter);

    Status UnregisterClient(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                            const chip::app::Clusters::IcdManagement::Commands::UnregisterClient::DecodableType & commandData);

    Status StayActiveRequest(chip::FabricIndex fabricIndex);

private:
    /**
     * @brief Triggers table update events to notify subscribers that an entry was added or removed
     *        from the ICDMonitoringTable.
     */
    void TriggerICDMTableUpdatedEvent();

    static chip::PersistentStorageDelegate * mStorage;
    static chip::Crypto::SymmetricKeystore * mSymmetricKeystore;
    static chip::ICDConfigurationData * mICDConfigurationData;
};
