// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster IcdManagement (cluster code: 70/0x46)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/IcdManagement/Ids.h>
#include <clusters/IcdManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::IcdManagement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::IcdManagement::Attributes;
        switch (attributeId)
        {
        case IdleModeDuration::Id:
            return IdleModeDuration::kMetadataEntry;
        case ActiveModeDuration::Id:
            return ActiveModeDuration::kMetadataEntry;
        case ActiveModeThreshold::Id:
            return ActiveModeThreshold::kMetadataEntry;
        case RegisteredClients::Id:
            return RegisteredClients::kMetadataEntry;
        case ICDCounter::Id:
            return ICDCounter::kMetadataEntry;
        case ClientsSupportedPerFabric::Id:
            return ClientsSupportedPerFabric::kMetadataEntry;
        case UserActiveModeTriggerHint::Id:
            return UserActiveModeTriggerHint::kMetadataEntry;
        case UserActiveModeTriggerInstruction::Id:
            return UserActiveModeTriggerInstruction::kMetadataEntry;
        case OperatingMode::Id:
            return OperatingMode::kMetadataEntry;
        case MaximumCheckInBackOff::Id:
            return MaximumCheckInBackOff::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::IcdManagement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::IcdManagement::Commands;
        switch (commandId)
        {
        case RegisterClient::Id:
            return RegisterClient::kMetadataEntry;
        case UnregisterClient::Id:
            return UnregisterClient::kMetadataEntry;
        case StayActiveRequest::Id:
            return StayActiveRequest::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
