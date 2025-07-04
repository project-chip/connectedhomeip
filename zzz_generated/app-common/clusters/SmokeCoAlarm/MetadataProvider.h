// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster SmokeCoAlarm (cluster code: 92/0x5C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/SmokeCoAlarm/Ids.h>
#include <clusters/SmokeCoAlarm/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::SmokeCoAlarm::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::SmokeCoAlarm::Attributes;
        switch (attributeId)
        {
        case ExpressedState::Id:
            return ExpressedState::kMetadataEntry;
        case SmokeState::Id:
            return SmokeState::kMetadataEntry;
        case COState::Id:
            return COState::kMetadataEntry;
        case BatteryAlert::Id:
            return BatteryAlert::kMetadataEntry;
        case DeviceMuted::Id:
            return DeviceMuted::kMetadataEntry;
        case TestInProgress::Id:
            return TestInProgress::kMetadataEntry;
        case HardwareFaultAlert::Id:
            return HardwareFaultAlert::kMetadataEntry;
        case EndOfServiceAlert::Id:
            return EndOfServiceAlert::kMetadataEntry;
        case InterconnectSmokeAlarm::Id:
            return InterconnectSmokeAlarm::kMetadataEntry;
        case InterconnectCOAlarm::Id:
            return InterconnectCOAlarm::kMetadataEntry;
        case ContaminationState::Id:
            return ContaminationState::kMetadataEntry;
        case SmokeSensitivityLevel::Id:
            return SmokeSensitivityLevel::kMetadataEntry;
        case ExpiryDate::Id:
            return ExpiryDate::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::SmokeCoAlarm::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::SmokeCoAlarm::Commands;
        switch (commandId)
        {
        case SelfTestRequest::Id:
            return SelfTestRequest::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
