// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster OnOff (cluster code: 6/0x6)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/OnOff/OnOffIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace OnOff {
namespace Metadata {

inline constexpr uint32_t kRevision = 6;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kOnOffEntry = {
    .attributeId    = Attributes::OnOff::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kGlobalSceneControlEntry = {
    .attributeId    = Attributes::GlobalSceneControl::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOnTimeEntry = {
    .attributeId    = Attributes::OnTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kOffWaitTimeEntry = {
    .attributeId    = Attributes::OffWaitTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AttributeEntry kStartUpOnOffEntry = {
    .attributeId    = Attributes::StartUpOnOff::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kOffEntry = {
    .commandId       = Commands::Off::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kOnEntry = {
    .commandId       = Commands::On::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kToggleEntry = {
    .commandId       = Commands::Toggle::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kOffWithEffectEntry = {
    .commandId       = Commands::OffWithEffect::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kOnWithRecallGlobalSceneEntry = {
    .commandId       = Commands::OnWithRecallGlobalScene::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kOnWithTimedOffEntry = {
    .commandId       = Commands::OnWithTimedOff::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace OnOff
} // namespace clusters
} // namespace app
} // namespace chip
