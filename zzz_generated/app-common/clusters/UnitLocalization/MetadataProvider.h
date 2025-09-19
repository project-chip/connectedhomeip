// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster UnitLocalization (cluster code: 45/0x2D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/UnitLocalization/Ids.h>
#include <clusters/UnitLocalization/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::UnitLocalization::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::UnitLocalization::Attributes;
        switch (attributeId)
        {
        case TemperatureUnit::Id:
            return TemperatureUnit::kMetadataEntry;
        case SupportedTemperatureUnits::Id:
            return SupportedTemperatureUnits::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::UnitLocalization::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::UnitLocalization::Commands;
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
