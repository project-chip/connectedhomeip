// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LaundryWasherControls (cluster code: 83/0x53)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/LaundryWasherControls/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryWasherControls {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace SpinSpeeds {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SpinSpeeds::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SpinSpeeds
namespace SpinSpeedCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SpinSpeedCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace SpinSpeedCurrent
namespace NumberOfRinses {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::NumberOfRinses::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace NumberOfRinses
namespace SupportedRinses {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedRinses::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedRinses

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace LaundryWasherControls
} // namespace Clusters
} // namespace app
} // namespace chip
