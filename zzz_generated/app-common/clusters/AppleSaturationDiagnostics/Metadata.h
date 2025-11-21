// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AppleSaturationDiagnostics (cluster code: 323615752/0x1349FC08)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AppleSaturationDiagnostics/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AppleSaturationDiagnostics {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace AppleStackHighwaterCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleStackHighwaterCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleStackHighwaterCount
namespace AppleLastStackHighwaterTaskName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleLastStackHighwaterTaskName::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleLastStackHighwaterTaskName
namespace AppleHeapHighwaterCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleHeapHighwaterCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AppleHeapHighwaterCount
namespace AppleLastHeapHighwaterTaskName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleLastHeapHighwaterTaskName::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleLastHeapHighwaterTaskName
namespace AppleStackOverflowCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleStackOverflowCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AppleStackOverflowCount
namespace AppleLastStackOverflowTaskName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleLastStackOverflowTaskName::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleLastStackOverflowTaskName
namespace AppleHeapOverflowCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleHeapOverflowCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AppleHeapOverflowCount
namespace AppleLastHeapOverflowTaskName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleLastHeapOverflowTaskName::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleLastHeapOverflowTaskName
namespace AppleAverageSystemMemoryUtilization {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleAverageSystemMemoryUtilization::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleAverageSystemMemoryUtilization
namespace ApplePeakSystemMemoryUtilization {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ApplePeakSystemMemoryUtilization::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace ApplePeakSystemMemoryUtilization
namespace AppleSystemMemoryPressureCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleSystemMemoryPressureCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleSystemMemoryPressureCount
namespace AppleProcessCriticalMemoryCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleProcessCriticalMemoryCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleProcessCriticalMemoryCount
namespace AppleLastProcessCriticalMemoryName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleLastProcessCriticalMemoryName::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleLastProcessCriticalMemoryName
namespace AppleAverageCPUUtilization {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleAverageCPUUtilization::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleAverageCPUUtilization
namespace ApplePeakCPUUtilization {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ApplePeakCPUUtilization::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ApplePeakCPUUtilization
namespace AppleTaskDeadlineMissCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleTaskDeadlineMissCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleTaskDeadlineMissCount
namespace AppleMaxTaskReadyQueueSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleMaxTaskReadyQueueSize::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleMaxTaskReadyQueueSize
namespace AppleLoadAverage {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleLoadAverage::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AppleLoadAverage
namespace AppleNonVolatileMemoryWriteCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleNonVolatileMemoryWriteCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleNonVolatileMemoryWriteCount
namespace AppleNonVolatileMemoryReadCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleNonVolatileMemoryReadCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleNonVolatileMemoryReadCount
namespace AppleNonVolatileMemoryWriteErrorCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleNonVolatileMemoryWriteErrorCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleNonVolatileMemoryWriteErrorCount
namespace AppleNonVolatileMemoryReadErrorCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleNonVolatileMemoryReadErrorCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleNonVolatileMemoryReadErrorCount
namespace AppleNonVolatileMemoryUtilization {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleNonVolatileMemoryUtilization::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleNonVolatileMemoryUtilization
namespace AppleSaturationResetCountBootRelativeTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleSaturationResetCountBootRelativeTime::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleSaturationResetCountBootRelativeTime
constexpr std::array<DataModel::AttributeEntry, 4> kMandatoryMetadata = {
    AppleNonVolatileMemoryWriteCount::kMetadataEntry,
    AppleNonVolatileMemoryWriteErrorCount::kMetadataEntry,
    AppleNonVolatileMemoryReadErrorCount::kMetadataEntry,
    AppleNonVolatileMemoryUtilization::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace ResetCounts {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ResetCounts::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ResetCounts

} // namespace Commands

namespace Events {} // namespace Events
} // namespace AppleSaturationDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
