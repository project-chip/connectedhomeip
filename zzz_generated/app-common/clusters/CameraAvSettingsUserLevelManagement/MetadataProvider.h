// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CameraAvSettingsUserLevelManagement (cluster code: 1362/0x552)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Ids.h>
#include <clusters/CameraAvSettingsUserLevelManagement/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::CameraAvSettingsUserLevelManagement::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::CameraAvSettingsUserLevelManagement::Attributes;
        switch (attributeId)
        {
        case MPTZPosition::Id:
            return MPTZPosition::kMetadataEntry;
        case MaxPresets::Id:
            return MaxPresets::kMetadataEntry;
        case MPTZPresets::Id:
            return MPTZPresets::kMetadataEntry;
        case DPTZStreams::Id:
            return DPTZStreams::kMetadataEntry;
        case ZoomMax::Id:
            return ZoomMax::kMetadataEntry;
        case TiltMin::Id:
            return TiltMin::kMetadataEntry;
        case TiltMax::Id:
            return TiltMax::kMetadataEntry;
        case PanMin::Id:
            return PanMin::kMetadataEntry;
        case PanMax::Id:
            return PanMax::kMetadataEntry;
        case MovementState::Id:
            return MovementState::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::CameraAvSettingsUserLevelManagement::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::CameraAvSettingsUserLevelManagement::Commands;
        switch (commandId)
        {
        case MPTZSetPosition::Id:
            return MPTZSetPosition::kMetadataEntry;
        case MPTZRelativeMove::Id:
            return MPTZRelativeMove::kMetadataEntry;
        case MPTZMoveToPreset::Id:
            return MPTZMoveToPreset::kMetadataEntry;
        case MPTZSavePreset::Id:
            return MPTZSavePreset::kMetadataEntry;
        case MPTZRemovePreset::Id:
            return MPTZRemovePreset::kMetadataEntry;
        case DPTZSetViewport::Id:
            return DPTZSetViewport::kMetadataEntry;
        case DPTZRelativeMove::Id:
            return DPTZRelativeMove::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
