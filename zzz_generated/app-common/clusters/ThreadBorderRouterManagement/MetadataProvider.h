// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadBorderRouterManagement (cluster code: 1106/0x452)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ThreadBorderRouterManagement/Ids.h>
#include <clusters/ThreadBorderRouterManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ThreadBorderRouterManagement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ThreadBorderRouterManagement::Attributes;
        switch (attributeId)
        {
        case BorderRouterName::Id:
            return BorderRouterName::kMetadataEntry;
        case BorderAgentID::Id:
            return BorderAgentID::kMetadataEntry;
        case ThreadVersion::Id:
            return ThreadVersion::kMetadataEntry;
        case InterfaceEnabled::Id:
            return InterfaceEnabled::kMetadataEntry;
        case ActiveDatasetTimestamp::Id:
            return ActiveDatasetTimestamp::kMetadataEntry;
        case PendingDatasetTimestamp::Id:
            return PendingDatasetTimestamp::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ThreadBorderRouterManagement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ThreadBorderRouterManagement::Commands;
        switch (commandId)
        {
        case GetActiveDatasetRequest::Id:
            return GetActiveDatasetRequest::kMetadataEntry;
        case GetPendingDatasetRequest::Id:
            return GetPendingDatasetRequest::kMetadataEntry;
        case SetActiveDatasetRequest::Id:
            return SetActiveDatasetRequest::kMetadataEntry;
        case SetPendingDatasetRequest::Id:
            return SetPendingDatasetRequest::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
