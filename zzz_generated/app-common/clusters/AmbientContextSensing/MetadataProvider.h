// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AmbientContextSensing (cluster code: 1073/0x431)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AmbientContextSensing/Ids.h>
#include <clusters/AmbientContextSensing/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AmbientContextSensing::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::AmbientContextSensing::Attributes;
        switch (attributeId)
        {
        case HumanActivityDetected::Id:
            return HumanActivityDetected::kMetadataEntry;
        case ObjectIdentified::Id:
            return ObjectIdentified::kMetadataEntry;
        case AudioContextDetected::Id:
            return AudioContextDetected::kMetadataEntry;
        case AmbientContextType::Id:
            return AmbientContextType::kMetadataEntry;
        case AmbientContextTypeSupported::Id:
            return AmbientContextTypeSupported::kMetadataEntry;
        case SimultaneousDetectionLimit::Id:
            return SimultaneousDetectionLimit::kMetadataEntry;
        case CountThresholdReached::Id:
            return CountThresholdReached::kMetadataEntry;
        case CountThreshold::Id:
            return CountThreshold::kMetadataEntry;
        case ObjectCount::Id:
            return ObjectCount::kMetadataEntry;
        case HoldTime::Id:
            return HoldTime::kMetadataEntry;
        case HoldTimeLimits::Id:
            return HoldTimeLimits::kMetadataEntry;
        case PredictedActivity::Id:
            return PredictedActivity::kMetadataEntry;
        case PrivacyModeEnabled::Id:
            return PrivacyModeEnabled::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AmbientContextSensing::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::AmbientContextSensing::Commands;
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
