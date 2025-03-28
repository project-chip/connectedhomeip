// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TemperatureControl (cluster code: 86/0x56)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TemperatureControl/TemperatureControlIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace TemperatureControl {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kTemperatureSetpointEntry = {
    .attributeId    = Attributes::TemperatureSetpoint::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMinTemperatureEntry = {
    .attributeId    = Attributes::MinTemperature::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kMaxTemperatureEntry = {
    .attributeId    = Attributes::MaxTemperature::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kStepEntry = {
    .attributeId    = Attributes::Step::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSelectedTemperatureLevelEntry = {
    .attributeId    = Attributes::SelectedTemperatureLevel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSupportedTemperatureLevelsEntry = {
    .attributeId    = Attributes::SupportedTemperatureLevels::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kSetTemperatureEntry = {
    .commandId       = Commands::SetTemperature::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace TemperatureControl
} // namespace clusters
} // namespace app
} // namespace chip
