// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AppleStabilityDiagnostics (cluster code: 323615753/0x1349FC09)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AppleStabilityDiagnostics/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AppleStabilityDiagnostics {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace AppleExpectedBootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleExpectedBootCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AppleExpectedBootCount
namespace AppleSystemCrashCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleSystemCrashCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AppleSystemCrashCount
namespace AppleSystemHangCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleSystemHangCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AppleSystemHangCount
namespace AppleLastRebootReason {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleLastRebootReason::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AppleLastRebootReason
namespace AppleActiveHardwareFaults {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AppleActiveHardwareFaults::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AppleActiveHardwareFaults
namespace AppleUnspecifiedRebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleUnspecifiedRebootCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleUnspecifiedRebootCount
namespace ApplePowerOnRebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ApplePowerOnRebootCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ApplePowerOnRebootCount
namespace AppleBrownOutResetCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBrownOutResetCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AppleBrownOutResetCount
namespace AppleSoftwareWatchdogRebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleSoftwareWatchdogRebootCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleSoftwareWatchdogRebootCount
namespace AppleHardwareWatchdogRebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleHardwareWatchdogRebootCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleHardwareWatchdogRebootCount
namespace AppleSoftwareUpdateRebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleSoftwareUpdateRebootCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleSoftwareUpdateRebootCount
namespace AppleSoftwareRebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleSoftwareRebootCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleSoftwareRebootCount
namespace AppleSoftwareExceptionRebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleSoftwareExceptionRebootCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleSoftwareExceptionRebootCount
namespace AppleHardwareFaultRebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleHardwareFaultRebootCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleHardwareFaultRebootCount
namespace AppleStackOverflowRebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleStackOverflowRebootCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleStackOverflowRebootCount
namespace AppleHeapOverflowRebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleHeapOverflowRebootCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleHeapOverflowRebootCount
namespace AppleTemperatureLimitRebootCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleTemperatureLimitRebootCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleTemperatureLimitRebootCount
constexpr std::array<DataModel::AttributeEntry, 5> kMandatoryMetadata = {
    AppleExpectedBootCount::kMetadataEntry, AppleSystemCrashCount::kMetadataEntry,     AppleSystemHangCount::kMetadataEntry,
    AppleLastRebootReason::kMetadataEntry,  AppleActiveHardwareFaults::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace AppleStabilityDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
