// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ElectricalProtectionAlarm (cluster code: 163/0xA3)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ElectricalProtectionAlarm/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ElectricalProtectionAlarm {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Mask {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Mask::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Mask
namespace State {
inline constexpr DataModel::AttributeEntry kMetadataEntry(State::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace State
namespace Supported {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Supported::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Supported
namespace ArcCause {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ArcCause::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ArcCause
namespace OverLoadRating {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OverLoadRating::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OverLoadRating
namespace OverVoltageRating {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OverVoltageRating::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OverVoltageRating
namespace SurgeProtectionRating {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SurgeProtectionRating::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SurgeProtectionRating
namespace ShortCircuitRating {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ShortCircuitRating::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ShortCircuitRating
namespace ResidualCurrentRating {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ResidualCurrentRating::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ResidualCurrentRating
namespace ArcFaultRating {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ArcFaultRating::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ArcFaultRating
constexpr std::array<DataModel::AttributeEntry, 3> kMandatoryMetadata = {
    Mask::kMetadataEntry,
    State::kMetadataEntry,
    Supported::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace ModifyEnabledAlarms {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ModifyEnabledAlarms::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ModifyEnabledAlarms

} // namespace Commands

namespace Events {
namespace Notify {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace Notify

} // namespace Events
} // namespace ElectricalProtectionAlarm
} // namespace Clusters
} // namespace app
} // namespace chip
