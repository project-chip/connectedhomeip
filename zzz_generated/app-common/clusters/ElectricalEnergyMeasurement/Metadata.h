// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalEnergyMeasurement (cluster code: 145/0x91)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ElectricalEnergyMeasurement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

namespace Accuracy {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Accuracy::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Accuracy
namespace CumulativeEnergyImported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CumulativeEnergyImported::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace CumulativeEnergyImported
namespace CumulativeEnergyExported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CumulativeEnergyExported::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace CumulativeEnergyExported
namespace PeriodicEnergyImported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PeriodicEnergyImported::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PeriodicEnergyImported
namespace PeriodicEnergyExported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PeriodicEnergyExported::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PeriodicEnergyExported
namespace CumulativeEnergyReset {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CumulativeEnergyReset::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CumulativeEnergyReset
constexpr std::array<DataModel::AttributeEntry, 1> kMandatoryMetadata = {
    Accuracy::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {
namespace CumulativeEnergyMeasured {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace CumulativeEnergyMeasured
namespace PeriodicEnergyMeasured {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace PeriodicEnergyMeasured

} // namespace Events
} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
