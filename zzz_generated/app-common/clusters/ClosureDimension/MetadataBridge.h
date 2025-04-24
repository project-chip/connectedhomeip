// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ClosureDimension (cluster code: 261/0x105)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ClosureDimension/Ids.h>
#include <clusters/ClosureDimension/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ClosureDimension::Id>
{
    static constexpr DataModel::AttributeEntry EntryFor(AttributeId commandId)
    {
        using namespace Clusters::ClosureDimension::Attributes;
        switch (commandId)
        {
        case Current::Id:
            return Current::kMetadataEntry;
        case Target::Id:
            return Target::kMetadataEntry;
        case Resolution::Id:
            return Resolution::kMetadataEntry;
        case StepValue::Id:
            return StepValue::kMetadataEntry;
        case Unit::Id:
            return Unit::kMetadataEntry;
        case UnitRange::Id:
            return UnitRange::kMetadataEntry;
        case LimitRange::Id:
            return LimitRange::kMetadataEntry;
        case TranslationDirection::Id:
            return TranslationDirection::kMetadataEntry;
        case RotationAxis::Id:
            return RotationAxis::kMetadataEntry;
        case Overflow::Id:
            return Overflow::kMetadataEntry;
        case ModulationType::Id:
            return ModulationType::kMetadataEntry;

        default:
            return {};
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ClosureDimension::Id>
{
    static constexpr DataModel::AcceptedCommandEntry EntryFor(CommandId commandId)
    {
        using namespace Clusters::ClosureDimension::Commands;
        switch (commandId)
        {
        case SetTarget::Id:
            return SetTarget::kMetadataEntry;
        case Step::Id:
            return Step::kMetadataEntry;

        default:
            return {};
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
