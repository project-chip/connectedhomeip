/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <access/AccessControl.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/OptionalAttributeSet.h>
#include <app/server/Server.h>
#include <clusters/AccessControl/ClusterId.h>
#include <clusters/AccessControl/Metadata.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace app {
namespace Clusters {

class AccessControlCluster : public DefaultServerCluster,
                             public Access::AccessControl::EntryListener
#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    ,
                             public Access::AccessRestrictionProvider::Listener
#endif
{
public:
    constexpr AccessControlCluster() : DefaultServerCluster(ConcreteClusterPath::ConstExpr(kRootEndpointId, AccessControl::Id)) {}

    CHIP_ERROR Startup(ServerClusterContext & context) override;

    void Shutdown(ClusterShutdownType type) override;

    DataModel::ActionReturnStatus ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                AttributeValueEncoder & encoder) override;

    DataModel::ActionReturnStatus WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                 AttributeValueDecoder & decoder) override;

    CHIP_ERROR Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder) override;

    CHIP_ERROR EventInfo(const ConcreteEventPath & path, DataModel::EventEntry & eventInfo) override;

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    std::optional<DataModel::ActionReturnStatus>
    HandleReviewFabricRestrictions(CommandHandler * commandObj, const ConcreteCommandPath & commandPath,
                                   const Clusters::AccessControl::Commands::ReviewFabricRestrictions::DecodableType & commandData);

    CHIP_ERROR AcceptedCommands(const ConcreteClusterPath & path,
                                ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) override;

    CHIP_ERROR GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder) override;

    std::optional<DataModel::ActionReturnStatus> InvokeCommand(const DataModel::InvokeRequest & request,
                                                               chip::TLV::TLVReader & input_arguments,
                                                               CommandHandler * handler) override;
#endif

private:
    void OnEntryChanged(const chip::Access::SubjectDescriptor * subjectDescriptor, FabricIndex fabric, size_t index,
                        const chip::Access::AccessControl::Entry * entry,
                        chip::Access::AccessControl::EntryListener::ChangeType changeType) override;

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    void MarkCommissioningRestrictionListChanged() override;

    void MarkRestrictionListChanged(FabricIndex fabricIndex) override;

    void OnFabricRestrictionReviewUpdate(FabricIndex fabricIndex, uint64_t token, Optional<CharSpan> instruction,
                                         Optional<CharSpan> arlRequestFlowUrl) override;
#endif
};

} // namespace Clusters
} // namespace app
} // namespace chip
