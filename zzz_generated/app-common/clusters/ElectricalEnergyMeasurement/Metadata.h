// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalEnergyMeasurement (cluster code: 145/0x91)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ElectricalEnergyMeasurement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalEnergyMeasurement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace Accuracy {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Accuracy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Accuracy
namespace CumulativeEnergyImported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CumulativeEnergyImported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CumulativeEnergyImported
namespace CumulativeEnergyExported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CumulativeEnergyExported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CumulativeEnergyExported
namespace PeriodicEnergyImported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::PeriodicEnergyImported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PeriodicEnergyImported
namespace PeriodicEnergyExported {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::PeriodicEnergyExported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PeriodicEnergyExported
namespace CumulativeEnergyReset {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CumulativeEnergyReset::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CumulativeEnergyReset

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace ElectricalEnergyMeasurement
} // namespace Clusters
} // namespace app
} // namespace chip
