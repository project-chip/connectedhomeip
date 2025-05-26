// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OnOff (cluster code: 6/0x6)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OnOff/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OnOff {

inline constexpr uint32_t kRevision = 6;

namespace Attributes {
namespace OnOff {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OnOff::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OnOff
namespace GlobalSceneControl {
inline constexpr DataModel::AttributeEntry kMetadataEntry(GlobalSceneControl::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace GlobalSceneControl
namespace OnTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OnTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace OnTime
namespace OffWaitTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OffWaitTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace OffWaitTime
namespace StartUpOnOff {
inline constexpr DataModel::AttributeEntry kMetadataEntry(StartUpOnOff::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kManage);
} // namespace StartUpOnOff

} // namespace Attributes

namespace Commands {
namespace Off {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Off::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Off
namespace On {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(On::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace On
namespace Toggle {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(Toggle::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace Toggle
namespace OffWithEffect {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(OffWithEffect::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace OffWithEffect
namespace OnWithRecallGlobalScene {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(OnWithRecallGlobalScene::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace OnWithRecallGlobalScene
namespace OnWithTimedOff {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(OnWithTimedOff::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace OnWithTimedOff

} // namespace Commands
} // namespace OnOff
} // namespace Clusters
} // namespace app
} // namespace chip
