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
#include <app/clusters/general-commissioning-server/BreadCrumbTracker.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <clusters/GeneralCommissioning/AttributeIds.h>
#include <clusters/GeneralCommissioning/ClusterId.h>
#include <clusters/GeneralCommissioning/Commands.h>
#include <clusters/GeneralCommissioning/Enums.h>
#include <cstdint>
#include <lib/support/BitFlags.h>

namespace chip::app::Clusters {

class GeneralCommissioningCluster : public DefaultServerCluster, chip::FabricTable::Delegate, public BreadCrumbTracker
{
public:
    using OptionalAttributes = OptionalAttributeSet<GeneralCommissioning::Attributes::IsCommissioningWithoutPower::Id>;

    GeneralCommissioningCluster() : DefaultServerCluster({ kRootEndpointId, GeneralCommissioning::Id }), mOptionalAttributes(0) {}

    OptionalAttributes & GetOptionalAttributes() { return mOptionalAttributes; }

    void SetBreadCrumb(uint64_t value) final;

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
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) final;

    // Feature map constant based on compile-time defines. This ensures feature map
    // is in sync with the actual supported features determined at build time.
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    static constexpr BitFlags<GeneralCommissioning::Feature> kFeatures =
        BitFlags<GeneralCommissioning::Feature>(GeneralCommissioning::Feature::kTermsAndConditions);
#else
    static constexpr BitFlags<GeneralCommissioning::Feature> kFeatures = BitFlags<GeneralCommissioning::Feature>(0);
#endif

private:
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
    std::optional<DataModel::ActionReturnStatus>
    HandleSetTCAcknowledgements(const DataModel::InvokeRequest & request, CommandHandler * handler,
                                const GeneralCommissioning::Commands::SetTCAcknowledgements::DecodableType & commandData);
#endif
};

} // namespace chip::app::Clusters
