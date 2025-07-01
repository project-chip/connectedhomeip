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
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SpinSpeeds::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SpinSpeeds
namespace SpinSpeedCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SpinSpeedCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace SpinSpeedCurrent
namespace NumberOfRinses {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfRinses::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace NumberOfRinses
namespace SupportedRinses {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedRinses::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedRinses

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace LaundryWasherControls
} // namespace Clusters
} // namespace app
} // namespace chip
