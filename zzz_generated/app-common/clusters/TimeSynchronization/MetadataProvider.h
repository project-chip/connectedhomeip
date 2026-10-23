// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TimeSynchronization (cluster code: 56/0x38)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/TimeSynchronization/Ids.h>
#include <clusters/TimeSynchronization/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::TimeSynchronization::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::TimeSynchronization::Attributes;
        switch (attributeId)
        {
        case UTCTime::Id:
            return UTCTime::kMetadataEntry;
        case Granularity::Id:
            return Granularity::kMetadataEntry;
        case TimeSource::Id:
            return TimeSource::kMetadataEntry;
        case TrustedTimeSource::Id:
            return TrustedTimeSource::kMetadataEntry;
        case DefaultNTP::Id:
            return DefaultNTP::kMetadataEntry;
        case TimeZone::Id:
            return TimeZone::kMetadataEntry;
        case DSTOffset::Id:
            return DSTOffset::kMetadataEntry;
        case LocalTime::Id:
            return LocalTime::kMetadataEntry;
        case TimeZoneDatabase::Id:
            return TimeZoneDatabase::kMetadataEntry;
        case NTPServerAvailable::Id:
            return NTPServerAvailable::kMetadataEntry;
        case TimeZoneListMaxSize::Id:
            return TimeZoneListMaxSize::kMetadataEntry;
        case DSTOffsetListMaxSize::Id:
            return DSTOffsetListMaxSize::kMetadataEntry;
        case SupportsDNSResolve::Id:
            return SupportsDNSResolve::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::TimeSynchronization::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::TimeSynchronization::Commands;
        switch (commandId)
        {
        case SetUTCTime::Id:
            return SetUTCTime::kMetadataEntry;
        case SetTrustedTimeSource::Id:
            return SetTrustedTimeSource::kMetadataEntry;
        case SetTimeZone::Id:
            return SetTimeZone::kMetadataEntry;
        case SetDSTOffset::Id:
            return SetDSTOffset::kMetadataEntry;
        case SetDefaultNTP::Id:
            return SetDefaultNTP::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
