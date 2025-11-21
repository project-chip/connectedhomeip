// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AppleThermalDiagnostics (cluster code: 323615754/0x1349FC0A)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AppleThermalDiagnostics/Ids.h>
#include <clusters/AppleThermalDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AppleThermalDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::AppleThermalDiagnostics::Attributes;
        switch (attributeId)
        {
        case AppleTemperatureLimitViolationCount::Id:
            return AppleTemperatureLimitViolationCount::kMetadataEntry;
        case AppleTemperatureLimitViolationDuration::Id:
            return AppleTemperatureLimitViolationDuration::kMetadataEntry;
        case AppleMinimumDeviceTemperature::Id:
            return AppleMinimumDeviceTemperature::kMetadataEntry;
        case AppleMaximumDeviceTemperature::Id:
            return AppleMaximumDeviceTemperature::kMetadataEntry;
        case AppleAverageDeviceTemperature::Id:
            return AppleAverageDeviceTemperature::kMetadataEntry;
        case AppleThermalResetCountBootRelativeTime::Id:
            return AppleThermalResetCountBootRelativeTime::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AppleThermalDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::AppleThermalDiagnostics::Commands;
        switch (commandId)
        {
        case ResetCounts::Id:
            return ResetCounts::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
