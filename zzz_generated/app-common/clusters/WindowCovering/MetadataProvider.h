// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster WindowCovering (cluster code: 258/0x102)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/WindowCovering/Ids.h>
#include <clusters/WindowCovering/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::WindowCovering::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::WindowCovering::Attributes;
        switch (attributeId)
        {
        case Type::Id:
            return Type::kMetadataEntry;
        case NumberOfActuationsLift::Id:
            return NumberOfActuationsLift::kMetadataEntry;
        case NumberOfActuationsTilt::Id:
            return NumberOfActuationsTilt::kMetadataEntry;
        case ConfigStatus::Id:
            return ConfigStatus::kMetadataEntry;
        case CurrentPositionLiftPercentage::Id:
            return CurrentPositionLiftPercentage::kMetadataEntry;
        case CurrentPositionTiltPercentage::Id:
            return CurrentPositionTiltPercentage::kMetadataEntry;
        case OperationalStatus::Id:
            return OperationalStatus::kMetadataEntry;
        case TargetPositionLiftPercent100ths::Id:
            return TargetPositionLiftPercent100ths::kMetadataEntry;
        case TargetPositionTiltPercent100ths::Id:
            return TargetPositionTiltPercent100ths::kMetadataEntry;
        case EndProductType::Id:
            return EndProductType::kMetadataEntry;
        case CurrentPositionLiftPercent100ths::Id:
            return CurrentPositionLiftPercent100ths::kMetadataEntry;
        case CurrentPositionTiltPercent100ths::Id:
            return CurrentPositionTiltPercent100ths::kMetadataEntry;
        case Mode::Id:
            return Mode::kMetadataEntry;
        case SafetyStatus::Id:
            return SafetyStatus::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::WindowCovering::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::WindowCovering::Commands;
        switch (commandId)
        {
        case UpOrOpen::Id:
            return UpOrOpen::kMetadataEntry;
        case DownOrClose::Id:
            return DownOrClose::kMetadataEntry;
        case StopMotion::Id:
            return StopMotion::kMetadataEntry;
        case GoToLiftPercentage::Id:
            return GoToLiftPercentage::kMetadataEntry;
        case GoToTiltPercentage::Id:
            return GoToTiltPercentage::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
