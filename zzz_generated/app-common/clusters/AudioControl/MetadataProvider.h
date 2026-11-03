// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AudioControl (cluster code: 1298/0x512)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AudioControl/Ids.h>
#include <clusters/AudioControl/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AudioControl::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::AudioControl::Attributes;
        switch (attributeId)
        {
        case SoftMuted::Id:
            return SoftMuted::kMetadataEntry;
        case PhysicallyMuted::Id:
            return PhysicallyMuted::kMetadataEntry;
        case Volume::Id:
            return Volume::kMetadataEntry;
        case MinDeviceVolume::Id:
            return MinDeviceVolume::kMetadataEntry;
        case MaxDeviceVolume::Id:
            return MaxDeviceVolume::kMetadataEntry;
        case MaxDeviceVolumeDB::Id:
            return MaxDeviceVolumeDB::kMetadataEntry;
        case MaxUserVolume::Id:
            return MaxUserVolume::kMetadataEntry;
        case DefaultStepSize::Id:
            return DefaultStepSize::kMetadataEntry;
        case SetVolumeUnmutePolicy::Id:
            return SetVolumeUnmutePolicy::kMetadataEntry;
        case IncreaseVolumeUnmutePolicy::Id:
            return IncreaseVolumeUnmutePolicy::kMetadataEntry;
        case IncreaseVolumeUnmuteVolume::Id:
            return IncreaseVolumeUnmuteVolume::kMetadataEntry;
        case DecreaseVolumeUnmutePolicy::Id:
            return DecreaseVolumeUnmutePolicy::kMetadataEntry;
        case StartUpMuted::Id:
            return StartUpMuted::kMetadataEntry;
        case StartUpVolume::Id:
            return StartUpVolume::kMetadataEntry;
        case Bass::Id:
            return Bass::kMetadataEntry;
        case Mid::Id:
            return Mid::kMetadataEntry;
        case Treble::Id:
            return Treble::kMetadataEntry;
        case MinCorrection::Id:
            return MinCorrection::kMetadataEntry;
        case MaxCorrection::Id:
            return MaxCorrection::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AudioControl::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::AudioControl::Commands;
        switch (commandId)
        {
        case Mute::Id:
            return Mute::kMetadataEntry;
        case Unmute::Id:
            return Unmute::kMetadataEntry;
        case ToggleMuted::Id:
            return ToggleMuted::kMetadataEntry;
        case SetVolume::Id:
            return SetVolume::kMetadataEntry;
        case IncreaseVolume::Id:
            return IncreaseVolume::kMetadataEntry;
        case DecreaseVolume::Id:
            return DecreaseVolume::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
