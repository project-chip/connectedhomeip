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
inline constexpr DataModel::AttributeEntry kMetadataEntry(UTCTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UTCTime
namespace Granularity {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Granularity::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Granularity
namespace TimeSource {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TimeSource::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TimeSource
namespace TrustedTimeSource {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TrustedTimeSource::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TrustedTimeSource
namespace DefaultNTP {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DefaultNTP::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace DefaultNTP
namespace TimeZone {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(TimeZone::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace TimeZone
namespace DSTOffset {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(DSTOffset::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace DSTOffset
namespace LocalTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(LocalTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace LocalTime
namespace TimeZoneDatabase {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TimeZoneDatabase::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TimeZoneDatabase
namespace NTPServerAvailable {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NTPServerAvailable::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NTPServerAvailable
namespace TimeZoneListMaxSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TimeZoneListMaxSize::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TimeZoneListMaxSize
namespace DSTOffsetListMaxSize {
inline constexpr DataModel::AttributeEntry kMetadataEntry(DSTOffsetListMaxSize::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace DSTOffsetListMaxSize
namespace SupportsDNSResolve {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SupportsDNSResolve::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SupportsDNSResolve

} // namespace Attributes

namespace Commands {
namespace SetUTCTime {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetUTCTime::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace SetUTCTime
namespace SetTrustedTimeSource {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetTrustedTimeSource::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace SetTrustedTimeSource
namespace SetTimeZone {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetTimeZone::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace SetTimeZone
namespace SetDSTOffset {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetDSTOffset::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace SetDSTOffset
namespace SetDefaultNTP {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SetDefaultNTP::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kAdminister);
} // namespace SetDefaultNTP

} // namespace Commands
} // namespace TimeSynchronization
} // namespace Clusters
} // namespace app
} // namespace chip
