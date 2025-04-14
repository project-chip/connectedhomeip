// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PushAvStreamTransport (cluster code: 1365/0x555)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

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
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::PushAvStreamTransport::Attributes;
        switch (commandId)
        {
        case SupportedContainerFormats::Id:
            return SupportedContainerFormats::kMetadataEntry;
        case SupportedIngestMethods::Id:
            return SupportedIngestMethods::kMetadataEntry;
        case CurrentConnections::Id:
            return CurrentConnections::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::PushAvStreamTransport::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
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
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
