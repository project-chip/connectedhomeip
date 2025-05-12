// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TemperatureControl (cluster code: 86/0x56)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TemperatureControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureControl {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace TemperatureSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TemperatureSetpoint::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TemperatureSetpoint
namespace MinTemperature {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinTemperature::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinTemperature
namespace MaxTemperature {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxTemperature::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxTemperature
namespace Step {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Step::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Step
namespace SelectedTemperatureLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SelectedTemperatureLevel::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace SelectedTemperatureLevel
namespace SupportedTemperatureLevels {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedTemperatureLevels::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedTemperatureLevels

} // namespace Attributes

namespace Commands {
namespace SetTemperature {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetTemperature::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SetTemperature

} // namespace Commands
} // namespace TemperatureControl
} // namespace Clusters
} // namespace app
} // namespace chip
