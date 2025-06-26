// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster UnitLocalization (cluster code: 45/0x2D)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/UnitLocalization/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace UnitLocalization {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace TemperatureUnit {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TemperatureUnit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace TemperatureUnit
namespace SupportedTemperatureUnits {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedTemperatureUnits::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedTemperatureUnits

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace UnitLocalization
} // namespace Clusters
} // namespace app
} // namespace chip
