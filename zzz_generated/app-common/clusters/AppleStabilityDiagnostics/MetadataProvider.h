// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AppleStabilityDiagnostics (cluster code: 323615753/0x1349FC09)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AppleStabilityDiagnostics/Ids.h>
#include <clusters/AppleStabilityDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AppleStabilityDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::AppleStabilityDiagnostics::Attributes;
        switch (attributeId)
        {
        case AppleExpectedBootCount::Id:
            return AppleExpectedBootCount::kMetadataEntry;
        case AppleSystemCrashCount::Id:
            return AppleSystemCrashCount::kMetadataEntry;
        case AppleSystemHangCount::Id:
            return AppleSystemHangCount::kMetadataEntry;
        case AppleLastRebootReason::Id:
            return AppleLastRebootReason::kMetadataEntry;
        case AppleActiveHardwareFaults::Id:
            return AppleActiveHardwareFaults::kMetadataEntry;
        case AppleUnspecifiedRebootCount::Id:
            return AppleUnspecifiedRebootCount::kMetadataEntry;
        case ApplePowerOnRebootCount::Id:
            return ApplePowerOnRebootCount::kMetadataEntry;
        case AppleBrownOutResetCount::Id:
            return AppleBrownOutResetCount::kMetadataEntry;
        case AppleSoftwareWatchdogRebootCount::Id:
            return AppleSoftwareWatchdogRebootCount::kMetadataEntry;
        case AppleHardwareWatchdogRebootCount::Id:
            return AppleHardwareWatchdogRebootCount::kMetadataEntry;
        case AppleSoftwareUpdateRebootCount::Id:
            return AppleSoftwareUpdateRebootCount::kMetadataEntry;
        case AppleSoftwareRebootCount::Id:
            return AppleSoftwareRebootCount::kMetadataEntry;
        case AppleSoftwareExceptionRebootCount::Id:
            return AppleSoftwareExceptionRebootCount::kMetadataEntry;
        case AppleHardwareFaultRebootCount::Id:
            return AppleHardwareFaultRebootCount::kMetadataEntry;
        case AppleStackOverflowRebootCount::Id:
            return AppleStackOverflowRebootCount::kMetadataEntry;
        case AppleHeapOverflowRebootCount::Id:
            return AppleHeapOverflowRebootCount::kMetadataEntry;
        case AppleTemperatureLimitRebootCount::Id:
            return AppleTemperatureLimitRebootCount::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AppleStabilityDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::AppleStabilityDiagnostics::Commands;
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
