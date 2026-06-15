// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalProtectionAlarm (cluster code: 163/0xA3)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/ElectricalProtectionAlarm/Ids.h>
#include <clusters/ElectricalProtectionAlarm/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::ElectricalProtectionAlarm::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::ElectricalProtectionAlarm::Attributes;
        switch (attributeId)
        {
        case Mask::Id:
            return Mask::kMetadataEntry;
        case State::Id:
            return State::kMetadataEntry;
        case Supported::Id:
            return Supported::kMetadataEntry;
        case ArcCause::Id:
            return ArcCause::kMetadataEntry;
        case OverLoadRating::Id:
            return OverLoadRating::kMetadataEntry;
        case OverVoltageRating::Id:
            return OverVoltageRating::kMetadataEntry;
        case SurgeProtectionRating::Id:
            return SurgeProtectionRating::kMetadataEntry;
        case ShortCircuitRating::Id:
            return ShortCircuitRating::kMetadataEntry;
        case ResidualCurrentRating::Id:
            return ResidualCurrentRating::kMetadataEntry;
        case ArcFaultRating::Id:
            return ArcFaultRating::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::ElectricalProtectionAlarm::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::ElectricalProtectionAlarm::Commands;
        switch (commandId)
        {
        case ModifyEnabledAlarms::Id:
            return ModifyEnabledAlarms::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
