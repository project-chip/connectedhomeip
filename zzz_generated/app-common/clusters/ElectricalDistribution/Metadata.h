// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalDistribution (cluster code: 162/0xA2)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ElectricalDistribution/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalDistribution {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace MaxContinuousCurrent {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxContinuousCurrent::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxContinuousCurrent
namespace MaxVoltage {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxVoltage::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxVoltage
namespace NumberOfPoles {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfPoles::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NumberOfPoles
namespace EndOfLife {
inline constexpr DataModel::AttributeEntry kMetadataEntry(EndOfLife::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace EndOfLife
namespace ServiceEntranceRated {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ServiceEntranceRated::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ServiceEntranceRated
constexpr std::array<DataModel::AttributeEntry, 5> kMandatoryMetadata = {
    MaxContinuousCurrent::kMetadataEntry, MaxVoltage::kMetadataEntry, NumberOfPoles::kMetadataEntry, EndOfLife::kMetadataEntry,
    ServiceEntranceRated::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace ElectricalDistribution
} // namespace Clusters
} // namespace app
} // namespace chip
