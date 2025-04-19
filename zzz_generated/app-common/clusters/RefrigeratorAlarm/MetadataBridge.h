// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RefrigeratorAlarm (cluster code: 87/0x57)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/RefrigeratorAlarm/Ids.h>
#include <clusters/RefrigeratorAlarm/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::RefrigeratorAlarm::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::RefrigeratorAlarm::Attributes;
        switch (commandId)
        {
        case Mask::Id:
            return Mask::kMetadataEntry;
        case State::Id:
            return State::kMetadataEntry;
        case Supported::Id:
            return Supported::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::RefrigeratorAlarm::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::RefrigeratorAlarm::Commands;
        switch (commandId)
        {

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
