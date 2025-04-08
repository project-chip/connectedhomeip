// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TimeSynchronization (cluster code: 56/0x38)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TimeSynchronization/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TimeSynchronization {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace UTCTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::UTCTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace UTCTime
namespace Granularity {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::Granularity::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Granularity
namespace TimeSource {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::TimeSource::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TimeSource
namespace TrustedTimeSource {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::TrustedTimeSource::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TrustedTimeSource
namespace DefaultNTP {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::DefaultNTP::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DefaultNTP
namespace TimeZone {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::TimeZone::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TimeZone
namespace DSTOffset {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::DSTOffset::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DSTOffset
namespace LocalTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::LocalTime::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace LocalTime
namespace TimeZoneDatabase {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::TimeZoneDatabase::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TimeZoneDatabase
namespace NTPServerAvailable {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::NTPServerAvailable::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace NTPServerAvailable
namespace TimeZoneListMaxSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::TimeZoneListMaxSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TimeZoneListMaxSize
namespace DSTOffsetListMaxSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::DSTOffsetListMaxSize::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace DSTOffsetListMaxSize
namespace SupportsDNSResolve {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSynchronization::Attributes::SupportsDNSResolve::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{  },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportsDNSResolve


} // namespace Attributes

namespace Commands {
namespace SetUTCTime {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = TimeSynchronization::Commands::SetUTCTime::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{  },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetUTCTime
namespace SetTrustedTimeSource {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = TimeSynchronization::Commands::SetTrustedTimeSource::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{ DataModel::CommandQualityFlags::kFabricScoped },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetTrustedTimeSource
namespace SetTimeZone {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = TimeSynchronization::Commands::SetTimeZone::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{  },
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SetTimeZone
namespace SetDSTOffset {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = TimeSynchronization::Commands::SetDSTOffset::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{  },
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace SetDSTOffset
namespace SetDefaultNTP {
inline constexpr DataModel::AcceptedCommandEntry kMetatadaEntry = {
    .commandId       = TimeSynchronization::Commands::SetDefaultNTP::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{  },
    .invokePrivilege = Access::Privilege::kAdminister,
};
} // namespace SetDefaultNTP

} // namespace Commands
} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip
