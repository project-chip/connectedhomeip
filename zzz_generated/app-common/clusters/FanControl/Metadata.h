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
namespace clusters {
namespace FanControl {

inline constexpr uint32_t kRevision = 5;

namespace Attributes {
namespace FanMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::FanMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace FanMode
namespace FanModeSequence {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::FanModeSequence::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace FanModeSequence
namespace PercentSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::PercentSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace PercentSetting
namespace PercentCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::PercentCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PercentCurrent
namespace SpeedMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::SpeedMax::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SpeedMax
namespace SpeedSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::SpeedSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace SpeedSetting
namespace SpeedCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::SpeedCurrent::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SpeedCurrent
namespace RockSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::RockSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace RockSupport
namespace RockSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::RockSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace RockSetting
namespace WindSupport {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::WindSupport::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace WindSupport
namespace WindSetting {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::WindSetting::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace WindSetting
namespace AirflowDirection {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FanControl::Attributes::AirflowDirection::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kOperate,
};
} // namespace AirflowDirection

} // namespace Attributes

namespace Commands {
namespace Step {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = FanControl::Commands::Step::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace Step

} // namespace Commands
} // namespace FanControl
} // namespace clusters
} // namespace app
} // namespace chip
