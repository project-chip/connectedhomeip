// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ActivatedCarbonFilterMonitoring (cluster code: 114/0x72)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ActivatedCarbonFilterMonitoring/Ids.h>
#include <clusters/ActivatedCarbonFilterMonitoring/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ActivatedCarbonFilterMonitoring::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ActivatedCarbonFilterMonitoring::Attributes;
        switch (attributeId)
        {
        case Condition::Id:
            return Condition::kMetadataEntry;
        case DegradationDirection::Id:
            return DegradationDirection::kMetadataEntry;
        case ChangeIndication::Id:
            return ChangeIndication::kMetadataEntry;
        case InPlaceIndicator::Id:
            return InPlaceIndicator::kMetadataEntry;
        case LastChangedTime::Id:
            return LastChangedTime::kMetadataEntry;
        case ReplacementProductList::Id:
            return ReplacementProductList::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ActivatedCarbonFilterMonitoring::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ActivatedCarbonFilterMonitoring::Commands;
        switch (commandId)
        {
        case ResetCondition::Id:
            return ResetCondition::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
