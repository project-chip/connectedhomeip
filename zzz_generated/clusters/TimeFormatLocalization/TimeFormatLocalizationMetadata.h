// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TimeFormatLocalization (cluster code: 44/0x2C)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TimeFormatLocalization/TimeFormatLocalizationIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace TimeFormatLocalization {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kHourFormatEntry = {
    .attributeId    = TimeFormatLocalization::Attributes::HourFormat::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kActiveCalendarTypeEntry = {
    .attributeId    = TimeFormatLocalization::Attributes::ActiveCalendarType::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSupportedCalendarTypesEntry = {
    .attributeId    = TimeFormatLocalization::Attributes::SupportedCalendarTypes::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace TimeFormatLocalization
} // namespace clusters
} // namespace app
} // namespace chip
