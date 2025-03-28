// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalEnergyMeasurement (cluster code: 145/0x91)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ElectricalEnergyMeasurement/ElectricalEnergyMeasurementIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace ElectricalEnergyMeasurement {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kAccuracyEntry = {
    .attributeId    = Attributes::Accuracy::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCumulativeEnergyImportedEntry = {
    .attributeId    = Attributes::CumulativeEnergyImported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCumulativeEnergyExportedEntry = {
    .attributeId    = Attributes::CumulativeEnergyExported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPeriodicEnergyImportedEntry = {
    .attributeId    = Attributes::PeriodicEnergyImported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPeriodicEnergyExportedEntry = {
    .attributeId    = Attributes::PeriodicEnergyExported::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCumulativeEnergyResetEntry = {
    .attributeId    = Attributes::CumulativeEnergyReset::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace ElectricalEnergyMeasurement
} // namespace clusters
} // namespace app
} // namespace chip
