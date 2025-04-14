// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster PumpConfigurationAndControl (cluster code: 512/0x200)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/PumpConfigurationAndControl/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace PumpConfigurationAndControl {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {
namespace MaxPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxPressure
namespace MaxSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxSpeed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxSpeed
namespace MaxFlow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxFlow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxFlow
namespace MinConstPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinConstPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinConstPressure
namespace MaxConstPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxConstPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxConstPressure
namespace MinCompPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinCompPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinCompPressure
namespace MaxCompPressure {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxCompPressure::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxCompPressure
namespace MinConstSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinConstSpeed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinConstSpeed
namespace MaxConstSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxConstSpeed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxConstSpeed
namespace MinConstFlow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinConstFlow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinConstFlow
namespace MaxConstFlow {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxConstFlow::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxConstFlow
namespace MinConstTemp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MinConstTemp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MinConstTemp
namespace MaxConstTemp {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = MaxConstTemp::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace MaxConstTemp
namespace PumpStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PumpStatus::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PumpStatus
namespace EffectiveOperationMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EffectiveOperationMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EffectiveOperationMode
namespace EffectiveControlMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = EffectiveControlMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace EffectiveControlMode
namespace Capacity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Capacity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Capacity
namespace Speed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Speed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Speed
namespace LifetimeRunningHours {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LifetimeRunningHours::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LifetimeRunningHours
namespace Power {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Power::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Power
namespace LifetimeEnergyConsumed {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = LifetimeEnergyConsumed::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace LifetimeEnergyConsumed
namespace OperationMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OperationMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace OperationMode
namespace ControlMode {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = ControlMode::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ControlMode


} // namespace Attributes

namespace Commands {

} // namespace Commands
} // namespace PumpConfigurationAndControl
} // namespace Clusters
} // namespace app
} // namespace chip
