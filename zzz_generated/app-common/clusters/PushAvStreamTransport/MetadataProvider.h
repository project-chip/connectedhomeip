// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PushAvStreamTransport (cluster code: 1365/0x555)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/PushAvStreamTransport/Ids.h>
#include <clusters/PushAvStreamTransport/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::PushAvStreamTransport::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::PushAvStreamTransport::Attributes;
        switch (attributeId)
        {
        case SupportedFormats::Id:
            return SupportedFormats::kMetadataEntry;
        case CurrentConnections::Id:
            return CurrentConnections::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::PushAvStreamTransport::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::PushAvStreamTransport::Commands;
        switch (commandId)
        {
        case AllocatePushTransport::Id:
            return AllocatePushTransport::kMetadataEntry;
        case DeallocatePushTransport::Id:
            return DeallocatePushTransport::kMetadataEntry;
        case ModifyPushTransport::Id:
            return ModifyPushTransport::kMetadataEntry;
        case SetTransportStatus::Id:
            return SetTransportStatus::kMetadataEntry;
        case ManuallyTriggerTransport::Id:
            return ManuallyTriggerTransport::kMetadataEntry;
        case FindTransport::Id:
            return FindTransport::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
