// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster UnitLocalization (cluster code: 45/0x2D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/UnitLocalization/UnitLocalizationIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace UnitLocalization {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kTemperatureUnitEntry = {
    .attributeId    = UnitLocalization::Attributes::TemperatureUnit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace UnitLocalization
} // namespace clusters
} // namespace app
} // namespace chip
