// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster DynamicLighting (cluster code: 773/0x305)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/DynamicLighting/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DynamicLighting {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace AvailableEffects {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AvailableEffects::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AvailableEffects
namespace CurrentEffectID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentEffectID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentEffectID
namespace CurrentSpeed {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentSpeed::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace CurrentSpeed
constexpr std::array<DataModel::AttributeEntry, 3> kMandatoryMetadata = {
    AvailableEffects::kMetadataEntry,
    CurrentEffectID::kMetadataEntry,
    CurrentSpeed::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace StartEffect {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StartEffect::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StartEffect
namespace StopEffect {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StopEffect::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StopEffect

} // namespace Commands

namespace Events {} // namespace Events
} // namespace DynamicLighting
} // namespace Clusters
} // namespace app
} // namespace chip
