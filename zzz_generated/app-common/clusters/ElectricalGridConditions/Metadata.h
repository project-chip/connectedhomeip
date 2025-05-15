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
inline constexpr DataModel::AttributeEntry kMetadataEntry(LocalGenerationAvailable::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          Access::Privilege::kOperate);
} // namespace LocalGenerationAvailable
namespace CurrentConditions {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentConditions::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentConditions
namespace ForecastConditions {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ForecastConditions::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ForecastConditions

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace ElectricalGridConditions
} // namespace Clusters
} // namespace app
} // namespace chip
