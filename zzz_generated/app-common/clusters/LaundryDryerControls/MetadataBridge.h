// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LaundryDryerControls (cluster code: 74/0x4A)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/LaundryDryerControls/Ids.h>
#include <clusters/LaundryDryerControls/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::LaundryDryerControls::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::LaundryDryerControls::Attributes;
        switch (commandId)
        {
        case SupportedDrynessLevels::Id:
            return SupportedDrynessLevels::kMetadataEntry;
        case SelectedDrynessLevel::Id:
            return SelectedDrynessLevel::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::LaundryDryerControls::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::LaundryDryerControls::Commands;
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
