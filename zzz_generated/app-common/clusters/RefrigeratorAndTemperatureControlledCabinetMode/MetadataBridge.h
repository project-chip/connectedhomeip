// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster RefrigeratorAndTemperatureControlledCabinetMode (cluster code: 82/0x52)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/RefrigeratorAndTemperatureControlledCabinetMode/Ids.h>
#include <clusters/RefrigeratorAndTemperatureControlledCabinetMode/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Attributes;
        switch (commandId)
        {
        case SupportedModes::Id:
            return SupportedModes::kMetadataEntry;
        case CurrentMode::Id:
            return CurrentMode::kMetadataEntry;
        case StartUpMode::Id:
            return StartUpMode::kMetadataEntry;
        case OnMode::Id:
            return OnMode::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Commands;
        switch (commandId)
        {
        case ChangeToMode::Id:
            return ChangeToMode::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
