// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AmbientContextSensing (cluster code: 1073/0x431)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AmbientContextSensing/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AmbientContextSensing {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace HumanActivityDetected {
inline constexpr DataModel::AttributeEntry kMetadataEntry(HumanActivityDetected::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace HumanActivityDetected
namespace ObjectIdentified {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ObjectIdentified::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ObjectIdentified
namespace AudioContextDetected {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AudioContextDetected::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AudioContextDetected
namespace AmbientContextType {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AmbientContextType::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AmbientContextType
namespace AmbientContextTypeSupported {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AmbientContextTypeSupported::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AmbientContextTypeSupported
namespace SimultaneousDetectionLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SimultaneousDetectionLimit::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace SimultaneousDetectionLimit
namespace CountThresholdReached {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CountThresholdReached::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CountThresholdReached
namespace CountThreshold {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CountThreshold::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace CountThreshold
namespace ObjectCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ObjectCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ObjectCount
namespace HoldTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(HoldTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace HoldTime
namespace HoldTimeLimits {
inline constexpr DataModel::AttributeEntry kMetadataEntry(HoldTimeLimits::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace HoldTimeLimits
namespace PredictedActivity {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(PredictedActivity::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace PredictedActivity
namespace PrivacyModeEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PrivacyModeEnabled::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PrivacyModeEnabled
constexpr std::array<DataModel::AttributeEntry, 1> kMandatoryMetadata = {
    SimultaneousDetectionLimit::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace AmbientContextSensing
} // namespace Clusters
} // namespace app
} // namespace chip
