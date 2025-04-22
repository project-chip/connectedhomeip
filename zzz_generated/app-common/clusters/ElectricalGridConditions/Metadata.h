// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalGridConditions (cluster code: 160/0xA0)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ElectricalGridConditions/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalGridConditions {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace LocalGenerationAvailable {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LocalGenerationAvailable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace LocalGenerationAvailable
namespace CurrentConditions {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CurrentConditions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentConditions
namespace ForecastConditions {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ForecastConditions::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace ForecastConditions

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace ElectricalGridConditions
} // namespace Clusters
} // namespace app
} // namespace chip
