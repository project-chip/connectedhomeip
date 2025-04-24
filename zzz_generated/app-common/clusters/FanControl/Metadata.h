// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster FanControl (cluster code: 514/0x202)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/FanControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace FanControl {

inline constexpr uint32_t kRevision = 5;

namespace Attributes {
namespace FanMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace FanMode
namespace FanModeSequence {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanModeSequence::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace FanModeSequence
namespace PercentSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PercentSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace PercentSetting
namespace PercentCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PercentCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PercentCurrent
namespace SpeedMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SpeedMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SpeedMax
namespace SpeedSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SpeedSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace SpeedSetting
namespace SpeedCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = SpeedCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SpeedCurrent
namespace RockSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RockSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RockSupport
namespace RockSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = RockSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace RockSetting
namespace WindSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WindSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WindSupport
namespace WindSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = WindSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace WindSetting
namespace AirflowDirection {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = AirflowDirection::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace AirflowDirection

} // namespace Attributes

namespace Commands {
namespace Step {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = FanControl::Commands::Step::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Step

} // namespace Commands
} // namespace FanControl
} // namespace Clusters
} // namespace app
} // namespace chip
