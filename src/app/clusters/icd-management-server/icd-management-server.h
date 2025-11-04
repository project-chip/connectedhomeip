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
#include <app/icd/server/ICDConfigurationData.h>
#include <app/util/basic-types.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>

#include <app/icd/server/ICDServerConfig.h>

#if CHIP_CONFIG_ENABLE_ICD_CIP
#include <app/ConcreteAttributePath.h>
#include <app/icd/server/ICDMonitoringTable.h> // nogncheck
#include <lib/core/CHIPPersistentStorageDelegate.h>
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

#include <app/icd/server/ICDStateObserver.h>

namespace chip {
namespace Crypto {
using SymmetricKeystore = SessionKeystore;
} // namespace Crypto
} // namespace chip

namespace chip {
namespace app {
namespace Clusters {

class ICDManagementServer : public chip::app::ICDStateObserver
{
public:
    ICDManagementServer() = default;

    void Init(PersistentStorageDelegate & storage, Crypto::SymmetricKeystore * symmetricKeystore,
              ICDConfigurationData & ICDConfigurationData);

    static ICDManagementServer & GetInstance() { return instance; };
#if CHIP_CONFIG_ENABLE_ICD_CIP
    /**
     * @brief Function that executes the business logic of the RegisterClient Command
     *
     * @param[out] icdCounter If function succeeds, icdCounter will have the current value of the ICDCounter stored in the
     * ICDConfigurationData If function fails, icdCounter will be unchanged
     * @return Status
     */
    Protocols::InteractionModel::Status RegisterClient(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                                       const IcdManagement::Commands::RegisterClient::DecodableType & commandData,
                                                       uint32_t & icdCounter);

    Protocols::InteractionModel::Status
    UnregisterClient(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                     const IcdManagement::Commands::UnregisterClient::DecodableType & commandData);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

private:
    static ICDManagementServer instance;
#if CHIP_CONFIG_ENABLE_ICD_CIP
    /**
     * @brief Triggers table update events to notify subscribers that an entry was added or removed
     *        from the ICDMonitoringTable.
     */
    void TriggerICDMTableUpdatedEvent();
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

    ICDConfigurationData * mICDConfigurationData;

#if CHIP_CONFIG_ENABLE_ICD_CIP
    PersistentStorageDelegate * mStorage;
    Crypto::SymmetricKeystore * mSymmetricKeystore;
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

    void OnEnterActiveMode() override{};
    void OnEnterIdleMode() override{};
    void OnTransitionToIdle() override{};
    void OnICDModeChange() override;
};

} // namespace Clusters
} // namespace app
} // namespace chip
