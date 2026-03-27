// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster GeneralDiagnostics (cluster code: 51/0x33)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/GeneralDiagnostics/Ids.h>
#include <clusters/GeneralDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::GeneralDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::GeneralDiagnostics::Attributes;
        switch (attributeId)
        {
        case NetworkInterfaces::Id:
            return NetworkInterfaces::kMetadataEntry;
        case RebootCount::Id:
            return RebootCount::kMetadataEntry;
        case UpTime::Id:
            return UpTime::kMetadataEntry;
        case TotalOperationalHours::Id:
            return TotalOperationalHours::kMetadataEntry;
        case BootReason::Id:
            return BootReason::kMetadataEntry;
        case ActiveHardwareFaults::Id:
            return ActiveHardwareFaults::kMetadataEntry;
        case ActiveRadioFaults::Id:
            return ActiveRadioFaults::kMetadataEntry;
        case ActiveNetworkFaults::Id:
            return ActiveNetworkFaults::kMetadataEntry;
        case TestEventTriggersEnabled::Id:
            return TestEventTriggersEnabled::kMetadataEntry;
        case DeviceLoadStatus::Id:
            return DeviceLoadStatus::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::GeneralDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::GeneralDiagnostics::Commands;
        switch (commandId)
        {
        case TestEventTrigger::Id:
            return TestEventTrigger::kMetadataEntry;
        case TimeSnapshot::Id:
            return TimeSnapshot::kMetadataEntry;
        case PayloadTestRequest::Id:
            return PayloadTestRequest::kMetadataEntry;

        default:
            return std::nullopt;
        }
    }
};

} // namespace DataModel
} // namespace app
} // namespace chip
