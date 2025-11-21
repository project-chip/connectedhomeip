// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AppleThermalDiagnostics (cluster code: 323615754/0x1349FC0A)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AppleThermalDiagnostics/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AppleThermalDiagnostics {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace AppleTemperatureLimitViolationCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleTemperatureLimitViolationCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleTemperatureLimitViolationCount
namespace AppleTemperatureLimitViolationDuration {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleTemperatureLimitViolationDuration::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleTemperatureLimitViolationDuration
namespace AppleMinimumDeviceTemperature {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleMinimumDeviceTemperature::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleMinimumDeviceTemperature
namespace AppleMaximumDeviceTemperature {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleMaximumDeviceTemperature::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleMaximumDeviceTemperature
namespace AppleAverageDeviceTemperature {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleAverageDeviceTemperature::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleAverageDeviceTemperature
namespace AppleThermalResetCountBootRelativeTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleThermalResetCountBootRelativeTime::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleThermalResetCountBootRelativeTime
constexpr std::array<DataModel::AttributeEntry, 1> kMandatoryMetadata = {
    AppleTemperatureLimitViolationCount::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace ResetCounts {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ResetCounts::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ResetCounts

} // namespace Commands

namespace Events {} // namespace Events
} // namespace AppleThermalDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
