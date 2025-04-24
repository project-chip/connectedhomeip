// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LaundryWasherControls (cluster code: 83/0x53)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/LaundryWasherControls/Ids.h>
#include <clusters/LaundryWasherControls/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::LaundryWasherControls::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::LaundryWasherControls::Attributes;
        switch (commandId)
        {
        case SpinSpeeds::Id:
            return SpinSpeeds::kMetadataEntry;
        case SpinSpeedCurrent::Id:
            return SpinSpeedCurrent::kMetadataEntry;
        case NumberOfRinses::Id:
            return NumberOfRinses::kMetadataEntry;
        case SupportedRinses::Id:
            return SupportedRinses::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::LaundryWasherControls::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::LaundryWasherControls::Commands;
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
