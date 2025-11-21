// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster AppleSaturationDiagnostics (cluster code: 323615752/0x1349FC08)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AppleSaturationDiagnostics {
namespace Attributes {

// Total number of attributes supported by the cluster, including global attributes
inline constexpr uint32_t kAttributesCount = 29;

namespace AppleStackHighwaterCount {
inline constexpr AttributeId Id = 0x00000000;
} // namespace AppleStackHighwaterCount

namespace AppleLastStackHighwaterTaskName {
inline constexpr AttributeId Id = 0x00000001;
} // namespace AppleLastStackHighwaterTaskName

namespace AppleHeapHighwaterCount {
inline constexpr AttributeId Id = 0x00000002;
} // namespace AppleHeapHighwaterCount

namespace AppleLastHeapHighwaterTaskName {
inline constexpr AttributeId Id = 0x00000003;
} // namespace AppleLastHeapHighwaterTaskName

namespace AppleStackOverflowCount {
inline constexpr AttributeId Id = 0x00000004;
} // namespace AppleStackOverflowCount

namespace AppleLastStackOverflowTaskName {
inline constexpr AttributeId Id = 0x00000005;
} // namespace AppleLastStackOverflowTaskName

namespace AppleHeapOverflowCount {
inline constexpr AttributeId Id = 0x00000006;
} // namespace AppleHeapOverflowCount

namespace AppleLastHeapOverflowTaskName {
inline constexpr AttributeId Id = 0x00000007;
} // namespace AppleLastHeapOverflowTaskName

namespace AppleAverageSystemMemoryUtilization {
inline constexpr AttributeId Id = 0x00000008;
} // namespace AppleAverageSystemMemoryUtilization

namespace ApplePeakSystemMemoryUtilization {
inline constexpr AttributeId Id = 0x00000009;
} // namespace ApplePeakSystemMemoryUtilization

namespace AppleSystemMemoryPressureCount {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace AppleSystemMemoryPressureCount

namespace AppleProcessCriticalMemoryCount {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace AppleProcessCriticalMemoryCount

namespace AppleLastProcessCriticalMemoryName {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace AppleLastProcessCriticalMemoryName

namespace AppleAverageCPUUtilization {
inline constexpr AttributeId Id = 0x0000000D;
} // namespace AppleAverageCPUUtilization

namespace ApplePeakCPUUtilization {
inline constexpr AttributeId Id = 0x0000000E;
} // namespace ApplePeakCPUUtilization

namespace AppleTaskDeadlineMissCount {
inline constexpr AttributeId Id = 0x0000000F;
} // namespace AppleTaskDeadlineMissCount

namespace AppleMaxTaskReadyQueueSize {
inline constexpr AttributeId Id = 0x00000010;
} // namespace AppleMaxTaskReadyQueueSize

namespace AppleLoadAverage {
inline constexpr AttributeId Id = 0x00000011;
} // namespace AppleLoadAverage

namespace AppleNonVolatileMemoryWriteCount {
inline constexpr AttributeId Id = 0x00000012;
} // namespace AppleNonVolatileMemoryWriteCount

namespace AppleNonVolatileMemoryReadCount {
inline constexpr AttributeId Id = 0x00000013;
} // namespace AppleNonVolatileMemoryReadCount

namespace AppleNonVolatileMemoryWriteErrorCount {
inline constexpr AttributeId Id = 0x00000014;
} // namespace AppleNonVolatileMemoryWriteErrorCount

namespace AppleNonVolatileMemoryReadErrorCount {
inline constexpr AttributeId Id = 0x00000015;
} // namespace AppleNonVolatileMemoryReadErrorCount

namespace AppleNonVolatileMemoryUtilization {
inline constexpr AttributeId Id = 0x00000016;
} // namespace AppleNonVolatileMemoryUtilization

namespace AppleSaturationResetCountBootRelativeTime {
inline constexpr AttributeId Id = 0x00000017;
} // namespace AppleSaturationResetCountBootRelativeTime

namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes
} // namespace AppleSaturationDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
