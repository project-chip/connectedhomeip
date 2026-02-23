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
namespace ObjectCountReached {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ObjectCountReached::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ObjectCountReached
namespace ObjectCountConfig {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ObjectCountConfig::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace ObjectCountConfig
namespace ObjectCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ObjectCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ObjectCount
namespace SimultaneousDetectionLimit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SimultaneousDetectionLimit::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace SimultaneousDetectionLimit
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
constexpr std::array<DataModel::AttributeEntry, 3> kMandatoryMetadata = {
    SimultaneousDetectionLimit::kMetadataEntry,
    HoldTime::kMetadataEntry,
    HoldTimeLimits::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {
namespace AmbientContextDetectStarted {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace AmbientContextDetectStarted
namespace AmbientContextDetectEnded {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace AmbientContextDetectEnded

} // namespace Events
} // namespace AmbientContextSensing
} // namespace Clusters
} // namespace app
} // namespace chip
