/*
 *    Copyright (c) 2022-2025 Project CHIP Authors
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

#include <app/AppConfig.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/GeneralCommissioning/AttributeIds.h>
#include <clusters/GeneralCommissioning/ClusterId.h>
#include <clusters/GeneralCommissioning/Commands.h>
#include <clusters/GeneralCommissioning/Enums.h>
#include <cstdint>
#include <lib/support/BitFlags.h>

namespace chip::app::Clusters {

class GeneralCommissioningCluster : public DefaultServerCluster, chip::FabricTable::Delegate
{
public:
    using OptionalAttributes = OptionalAttributeSet<GeneralCommissioning::Attributes::IsCommissioningWithoutPower::Id>;

    GeneralCommissioningCluster() :
        DefaultServerCluster({ kRootEndpointId, GeneralCommissioning::Id }), mFeatures(0),
        mOptionalAttributes(0)
    {}

    OptionalAttributes & GetOptionalAttributes() { return mOptionalAttributes; }

    void SetBreadCrumb(uint64_t value);

    // Server cluster implementation
    CHIP_ERROR Startup(ServerClusterContext & context) override;
    void Shutdown() override;
    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;
    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;
    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               TLV::TLVReader & input_arguments, CommandHandler * handler) override;
    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;
    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;
    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    // Fabric delegate
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

    // GeneralCommissioning is a singleton cluster that exists only on the root endpoint.
    static GeneralCommissioningCluster & Instance();

private:
    const BitFlags<GeneralCommissioning::Feature> mFeatures;
    OptionalAttributes mOptionalAttributes;
    uint64_t mBreadCrumb = 0;

    std::optional<DataModel::ActionReturnStatus>
    HandleArmFailSafe(const DataModel::InvokeRequest & request, CommandHandler * handler,
                      const GeneralCommissioning::Commands::ArmFailSafe::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleCommissioningComplete(const DataModel::InvokeRequest & request, CommandHandler * handler,
                                const GeneralCommissioning::Commands::CommissioningComplete::DecodableType & commandData);
    std::optional<DataModel::ActionReturnStatus>
    HandleSetRegulatoryConfig(const DataModel::InvokeRequest & request, CommandHandler * handler,
                              const GeneralCommissioning::Commands::SetRegulatoryConfig::DecodableType & commandData);
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    std::optional<DataModel::ActActionReturnStatus>
    HandleSetTCAcknowledgements(const Datamodel::InvokeRequest & request, CommandHandler * handler,
                                const GeneralCommissioning::Commands::SetTCAcknowledgements::DecodableType & commandData);
#endif
};

} // namespace chip::app::Clusters