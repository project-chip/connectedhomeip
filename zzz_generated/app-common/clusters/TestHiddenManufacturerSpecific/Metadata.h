// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TestHiddenManufacturerSpecific (cluster code: 4294048801/0xFFF1FC21)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TestHiddenManufacturerSpecific/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TestHiddenManufacturerSpecific {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace TestAttribute {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TestAttribute::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TestAttribute
constexpr std::array<DataModel::AttributeEntry, 1> kMandatoryMetadata = {
    TestAttribute::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {} // namespace Events
} // namespace TestHiddenManufacturerSpecific
} // namespace Clusters
} // namespace app
} // namespace chip
