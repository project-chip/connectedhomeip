// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TemperatureControlledCabinetTopology (cluster code: 75/0x4B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TemperatureControlledCabinetTopology/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TemperatureControlledCabinetTopology {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace DisabledCabinets {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(DisabledCabinets::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace DisabledCabinets
namespace Topology {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Topology::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Topology
constexpr std::array<DataModel::AttributeEntry, 2> kMandatoryMetadata = {
    DisabledCabinets::kMetadataEntry,
    Topology::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace TemperatureControlledCabinetTopology
} // namespace Clusters
} // namespace app
} // namespace chip
