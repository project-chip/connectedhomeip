// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AmbientSensingUnion (cluster code: 1074/0x432)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AmbientSensingUnion/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AmbientSensingUnion {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace UnionID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnionID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UnionID
namespace UnionName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnionName::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace UnionName
namespace UnionHealth {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UnionHealth::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UnionHealth
namespace UnionSensorList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(UnionSensorList::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace UnionSensorList
namespace UnionSensorHealth {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(UnionSensorHealth::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace UnionSensorHealth
namespace SensorID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SensorID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SensorID
namespace ZoneSensorList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ZoneSensorList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ZoneSensorList
constexpr std::array<DataModel::AttributeEntry, 2> kMandatoryMetadata = {
    UnionID::kMetadataEntry,
    SensorID::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {
namespace SensorListChange {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace SensorListChange

} // namespace Events
} // namespace AmbientSensingUnion
} // namespace Clusters
} // namespace app
} // namespace chip
