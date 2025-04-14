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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::OnOff::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OnOff
namespace GlobalSceneControl {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::GlobalSceneControl::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace GlobalSceneControl
namespace OnTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::OnTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace OnTime
namespace OffWaitTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::OffWaitTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace OffWaitTime
namespace StartUpOnOff {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::StartUpOnOff::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace StartUpOnOff

} // namespace Attributes

namespace Commands {
namespace Off {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::Off::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Off
namespace On {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::On::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace On
namespace Toggle {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::Toggle::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Toggle
namespace OffWithEffect {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::OffWithEffect::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace OffWithEffect
namespace OnWithRecallGlobalScene {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::OnWithRecallGlobalScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace OnWithRecallGlobalScene
namespace OnWithTimedOff {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::OnWithTimedOff::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace OnWithTimedOff

} // namespace Commands
} // namespace OnOff
} // namespace Clusters
} // namespace app
} // namespace chip
