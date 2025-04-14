// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TimeFormatLocalization (cluster code: 44/0x2C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TimeFormatLocalization/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TimeFormatLocalization {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace HourFormat {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::HourFormat::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace HourFormat
namespace ActiveCalendarType {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ActiveCalendarType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ActiveCalendarType
namespace SupportedCalendarTypes {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedCalendarTypes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedCalendarTypes

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace TimeFormatLocalization
} // namespace Clusters
} // namespace app
} // namespace chip
