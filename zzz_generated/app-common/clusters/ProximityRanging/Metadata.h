// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ProximityRanging (cluster code: 1075/0x433)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ProximityRanging/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ProximityRanging {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace RangingCapabilities {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(RangingCapabilities::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace RangingCapabilities
namespace WiFiDevIK {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WiFiDevIK::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace WiFiDevIK
namespace BLEDeviceID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BLEDeviceID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BLEDeviceID
namespace BLTDevIK {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BLTDevIK::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BLTDevIK
namespace BLTCSSecurityLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BLTCSSecurityLevel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BLTCSSecurityLevel
namespace BLTCSModeCapability {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BLTCSModeCapability::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BLTCSModeCapability
namespace SessionIDList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SessionIDList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SessionIDList
constexpr std::array<DataModel::AttributeEntry, 2> kMandatoryMetadata = {
    RangingCapabilities::kMetadataEntry,
    SessionIDList::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace StartRangingRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StartRangingRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StartRangingRequest
namespace StopRangingRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StopRangingRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StopRangingRequest

} // namespace Commands

namespace Events {
namespace RangingResult {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace RangingResult
namespace RangingSessionStatus {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace RangingSessionStatus

} // namespace Events
} // namespace ProximityRanging
} // namespace Clusters
} // namespace app
} // namespace chip
