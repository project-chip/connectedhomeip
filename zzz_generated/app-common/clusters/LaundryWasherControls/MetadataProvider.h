// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LaundryWasherControls (cluster code: 83/0x53)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

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
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::LaundryWasherControls::Attributes;
        switch (attributeId)
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
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::LaundryWasherControls::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::LaundryWasherControls::Commands;
        switch (commandId)
        {

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
