// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Humidistat (cluster code: 517/0x205)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Humidistat/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Humidistat {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Mode {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Mode::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Mode
namespace SystemState {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SystemState::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SystemState
namespace UserSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UserSetpoint::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UserSetpoint
namespace MinSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MinSetpoint::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MinSetpoint
namespace MaxSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxSetpoint::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxSetpoint
namespace Step {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Step::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Step
namespace TargetSetpoint {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TargetSetpoint::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TargetSetpoint
namespace MistType {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MistType::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MistType
namespace Continuous {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Continuous::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Continuous
namespace Sleep {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Sleep::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Sleep
namespace Optimal {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Optimal::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Optimal
constexpr std::array<DataModel::AttributeEntry, 2> kMandatoryMetadata = {
    Mode::kMetadataEntry,
    SystemState::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace SetSettings {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetSettings::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SetSettings

} // namespace Commands

namespace Events {} // namespace Events
} // namespace Humidistat
} // namespace Clusters
} // namespace app
} // namespace chip
