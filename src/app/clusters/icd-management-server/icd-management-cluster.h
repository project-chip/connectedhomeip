/**
 *
 *    Copyright (c) 2022-25 Project CHIP Authors
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

#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/icd/server/ICDConfigurationData.h>
#include <app/util/basic-types.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>
#include <clusters/IcdManagement/ClusterId.h>
#include <clusters/IcdManagement/Metadata.h>
#include <clusters/IcdManagement/Commands.h>

#include <app/server/Server.h>
#include <app/icd/server/ICDServerConfig.h>

#if CHIP_CONFIG_ENABLE_ICD_CIP
#include <app/icd/server/ICDMonitoringTable.h> // nogncheck
#include <lib/core/CHIPPersistentStorageDelegate.h>
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

namespace chip {
namespace Crypto {
    using SymmetricKeystore = SessionKeystore;
} // namespace Crypto

namespace app {
namespace Clusters {

enum class OptionalCommands : uint32_t {
    kStayActiveRequest = 0x01,
    kStayActiveResponse = 0x02,    
};

#if CHIP_CONFIG_ENABLE_ICD_CIP
/**
 * @brief Implementation of Fabric Delegate for ICD Management cluster
 */
class IcdManagementFabricDelegate : public FabricTable::Delegate
{
public:
    void Init(PersistentStorageDelegate & storage, Crypto::SymmetricKeystore * symmetricKeystore,
              ICDConfigurationData & icdConfigurationData)
    {
        mStorage              = &storage;
        mSymmetricKeystore    = symmetricKeystore;
        mICDConfigurationData = &icdConfigurationData;
    }

    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override
    {
        uint16_t supported_clients = mICDConfigurationData->GetClientsSupportedPerFabric();
        ICDMonitoringTable table(*mStorage, fabricIndex, supported_clients, mSymmetricKeystore);
        table.RemoveAll();
        ICDNotifier::GetInstance().NotifyICDManagementEvent(ICDListener::ICDManagementEvents::kTableUpdated);
    }

private:
    PersistentStorageDelegate * mStorage           = nullptr;
    Crypto::SymmetricKeystore * mSymmetricKeystore = nullptr;
    ICDConfigurationData * mICDConfigurationData   = nullptr;
};
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

class ICDManagementCluster : public DefaultServerCluster {
public:
    using OptionalAttributeSet =
    chip::app::OptionalAttributeSet<IcdManagement::Attributes::UserActiveModeTriggerInstruction::Id>;

    ICDManagementCluster(PersistentStorageDelegate & storage, Crypto::SymmetricKeystore & symmetricKeystore, FabricTable & fabricTable,
        ICDConfigurationData & icdConfigurationData, BitFlags<IcdManagement::Feature> aFeatureMap, OptionalAttributeSet optionalAttributeSet, BitMask<OptionalCommands> aEnabledCommands) : DefaultServerCluster({kRootEndpointId, IcdManagement::Id})
#if CHIP_CONFIG_ENABLE_ICD_CIP
        , mStorage(storage), mSymmetricKeystore(symmetricKeystore), mFabricTable(fabricTable)
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
        , mICDConfigurationData(icdConfigurationData), mFeatureMap(aFeatureMap), mOptionalAttributeSet(optionalAttributeSet), mEnabledCommands(aEnabledCommands)
        {
        }


    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler) override;

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

private:

#if CHIP_CONFIG_ENABLE_ICD_LIT
    CHIP_ERROR ReadOperatingMode(AttributeValueEncoder & encoder)
    {
        return mICDConfigurationData.GetICDMode() == ICDConfigurationData::ICDMode::SIT
            ? encoder.Encode(IcdManagement::OperatingModeEnum::kSit)
            : encoder.Encode(IcdManagement::OperatingModeEnum::kLit);
    }
#endif // CHIP_CONFIG_ENABLE_ICD_LIT

#if CHIP_CONFIG_ENABLE_ICD_CIP
    CHIP_ERROR ReadRegisteredClients(AttributeValueEncoder & encoder);
    /**
     * @brief Triggers table update events to notify subscribers that an entry was added or removed
     *        from the ICDMonitoringTable.
     */
    void TriggerICDMTableUpdatedEvent();

    /**
     * @brief Function that executes the business logic of the RegisterClient Command
     *
     * @param[out] icdCounter If function succeeds, icdCounter will have the current value of the ICDCounter stored in the
     * ICDConfigurationData If function fails, icdCounter will be unchanged
     * @return Status
     */
    chip::Protocols::InteractionModel::Status
    RegisterClient(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                    const chip::app::Clusters::IcdManagement::Commands::RegisterClient::DecodableType & commandData,
                    uint32_t & icdCounter);
 
    chip::Protocols::InteractionModel::Status
    UnregisterClient(chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
                      const chip::app::Clusters::IcdManagement::Commands::UnregisterClient::DecodableType & commandData);

    chip::PersistentStorageDelegate & mStorage;
    Crypto::SymmetricKeystore & mSymmetricKeystore;
    chip::FabricTable & mFabricTable;
#endif // CHIP_CONFIG_ENABLE_ICD_CIP


    CHIP_ERROR CheckAdmin(CommandHandler * commandObj, const ConcreteCommandPath & commandPath, bool & isClientAdmin);

    chip::ICDConfigurationData & mICDConfigurationData;
    BitFlags<IcdManagement::Feature> mFeatureMap;
    const OptionalAttributeSet mOptionalAttributeSet;
    BitMask<OptionalCommands> mEnabledCommands;
};

} // namespace Clusters
} // namespace app
} // namespace chip