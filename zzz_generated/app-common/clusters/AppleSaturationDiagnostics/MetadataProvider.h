// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AppleSaturationDiagnostics (cluster code: 323615752/0x1349FC08)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <optional>

#include <app/data-model-provider/ClusterMetadataProvider.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <clusters/AppleSaturationDiagnostics/Ids.h>
#include <clusters/AppleSaturationDiagnostics/Metadata.h>

namespace chip {
namespace app {
namespace DataModel {

template <>
struct ClusterMetadataProvider<DataModel::AttributeEntry, Clusters::AppleSaturationDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AttributeEntry> EntryFor(AttributeId attributeId)
    {
        using namespace Clusters::AppleSaturationDiagnostics::Attributes;
        switch (attributeId)
        {
        case AppleStackHighwaterCount::Id:
            return AppleStackHighwaterCount::kMetadataEntry;
        case AppleLastStackHighwaterTaskName::Id:
            return AppleLastStackHighwaterTaskName::kMetadataEntry;
        case AppleHeapHighwaterCount::Id:
            return AppleHeapHighwaterCount::kMetadataEntry;
        case AppleLastHeapHighwaterTaskName::Id:
            return AppleLastHeapHighwaterTaskName::kMetadataEntry;
        case AppleStackOverflowCount::Id:
            return AppleStackOverflowCount::kMetadataEntry;
        case AppleLastStackOverflowTaskName::Id:
            return AppleLastStackOverflowTaskName::kMetadataEntry;
        case AppleHeapOverflowCount::Id:
            return AppleHeapOverflowCount::kMetadataEntry;
        case AppleLastHeapOverflowTaskName::Id:
            return AppleLastHeapOverflowTaskName::kMetadataEntry;
        case AppleAverageSystemMemoryUtilization::Id:
            return AppleAverageSystemMemoryUtilization::kMetadataEntry;
        case ApplePeakSystemMemoryUtilization::Id:
            return ApplePeakSystemMemoryUtilization::kMetadataEntry;
        case AppleSystemMemoryPressureCount::Id:
            return AppleSystemMemoryPressureCount::kMetadataEntry;
        case AppleProcessCriticalMemoryCount::Id:
            return AppleProcessCriticalMemoryCount::kMetadataEntry;
        case AppleLastProcessCriticalMemoryName::Id:
            return AppleLastProcessCriticalMemoryName::kMetadataEntry;
        case AppleAverageCPUUtilization::Id:
            return AppleAverageCPUUtilization::kMetadataEntry;
        case ApplePeakCPUUtilization::Id:
            return ApplePeakCPUUtilization::kMetadataEntry;
        case AppleTaskDeadlineMissCount::Id:
            return AppleTaskDeadlineMissCount::kMetadataEntry;
        case AppleMaxTaskReadyQueueSize::Id:
            return AppleMaxTaskReadyQueueSize::kMetadataEntry;
        case AppleLoadAverage::Id:
            return AppleLoadAverage::kMetadataEntry;
        case AppleNonVolatileMemoryWriteCount::Id:
            return AppleNonVolatileMemoryWriteCount::kMetadataEntry;
        case AppleNonVolatileMemoryReadCount::Id:
            return AppleNonVolatileMemoryReadCount::kMetadataEntry;
        case AppleNonVolatileMemoryWriteErrorCount::Id:
            return AppleNonVolatileMemoryWriteErrorCount::kMetadataEntry;
        case AppleNonVolatileMemoryReadErrorCount::Id:
            return AppleNonVolatileMemoryReadErrorCount::kMetadataEntry;
        case AppleNonVolatileMemoryUtilization::Id:
            return AppleNonVolatileMemoryUtilization::kMetadataEntry;
        case AppleSaturationResetCountBootRelativeTime::Id:
            return AppleSaturationResetCountBootRelativeTime::kMetadataEntry;
        default:
            return std::nullopt;
        }
    }
};

template <>
struct ClusterMetadataProvider<DataModel::AcceptedCommandEntry, Clusters::AppleSaturationDiagnostics::Id>
{
    static constexpr std::optional<DataModel::AcceptedCommandEntry> EntryFor(CommandId commandId)
    {
        using namespace Clusters::AppleSaturationDiagnostics::Commands;
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
