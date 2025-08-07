// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Switch (cluster code: 59/0x3B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Switch/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Switch {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

namespace NumberOfPositions {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumberOfPositions::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NumberOfPositions
namespace CurrentPosition {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentPosition::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentPosition
namespace MultiPressMax {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MultiPressMax::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MultiPressMax
constexpr std::array<DataModel::AttributeEntry, 2> kMandatoryMetadata = {
    NumberOfPositions::kMetadataEntry,
    CurrentPosition::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {} // namespace Commands

namespace Events {
namespace SwitchLatched {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace SwitchLatched
namespace InitialPress {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace InitialPress
namespace LongPress {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace LongPress
namespace ShortRelease {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace ShortRelease
namespace LongRelease {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace LongRelease
namespace MultiPressOngoing {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace MultiPressOngoing
namespace MultiPressComplete {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace MultiPressComplete

} // namespace Events
} // namespace Switch
} // namespace Clusters
} // namespace app
} // namespace chip
