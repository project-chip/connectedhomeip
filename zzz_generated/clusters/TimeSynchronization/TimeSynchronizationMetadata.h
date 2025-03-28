// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TimeSynchronization (cluster code: 56/0x38)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TimeSynchronization/TimeSynchronizationIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace TimeSynchronization {
namespace Metadata {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kUTCTimeEntry = {
    .attributeId    = Attributes::UTCTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kGranularityEntry = {
    .attributeId    = Attributes::Granularity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTimeSourceEntry = {
    .attributeId    = Attributes::TimeSource::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTrustedTimeSourceEntry = {
    .attributeId    = Attributes::TrustedTimeSource::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDefaultNTPEntry = {
    .attributeId    = Attributes::DefaultNTP::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTimeZoneEntry = {
    .attributeId    = Attributes::TimeZone::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDSTOffsetEntry = {
    .attributeId    = Attributes::DSTOffset::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLocalTimeEntry = {
    .attributeId    = Attributes::LocalTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTimeZoneDatabaseEntry = {
    .attributeId    = Attributes::TimeZoneDatabase::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kNTPServerAvailableEntry = {
    .attributeId    = Attributes::NTPServerAvailable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTimeZoneListMaxSizeEntry = {
    .attributeId    = Attributes::TimeZoneListMaxSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kDSTOffsetListMaxSizeEntry = {
    .attributeId    = Attributes::DSTOffsetListMaxSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kSupportsDNSResolveEntry = {
    .attributeId    = Attributes::SupportsDNSResolve::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kSetUTCTimeEntry = {
    .commandId       = Commands::SetUTCTime::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kSetTrustedTimeSourceEntry = {
    .commandId       = Commands::SetTrustedTimeSource::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
inline constexpr DataModel::AcceptedCommandEntry kSetTimeZoneEntry = {
    .commandId       = Commands::SetTimeZone::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSetDSTOffsetEntry = {
    .commandId       = Commands::SetDSTOffset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AcceptedCommandEntry kSetDefaultNTPEntry = {
    .commandId       = Commands::SetDefaultNTP::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kAdminister,
};

} // namespace Commands
} // namespace Metadata
} // namespace TimeSynchronization
} // namespace clusters
} // namespace app
} // namespace chip
