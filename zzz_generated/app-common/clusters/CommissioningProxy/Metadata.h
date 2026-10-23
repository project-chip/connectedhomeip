// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommissioningProxy (cluster code: 1109/0x455)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/CommissioningProxy/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommissioningProxy {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Transport {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Transport::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Transport
namespace ScanMaxTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ScanMaxTime::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace ScanMaxTime
namespace MaxSessions {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxSessions::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxSessions
namespace MaxCachedResults {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxCachedResults::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxCachedResults
namespace NumCachedResults {
inline constexpr DataModel::AttributeEntry kMetadataEntry(NumCachedResults::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace NumCachedResults
namespace CacheTimeout {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CacheTimeout::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, Access::Privilege::kOperate);
} // namespace CacheTimeout
namespace CachedResults {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(CachedResults::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace CachedResults
namespace WiFiBand {
inline constexpr DataModel::AttributeEntry kMetadataEntry(WiFiBand::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace WiFiBand
constexpr std::array<DataModel::AttributeEntry, 3> kMandatoryMetadata = {
    Transport::kMetadataEntry,
    ScanMaxTime::kMetadataEntry,
    MaxSessions::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace ProxyConnectRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ProxyConnectRequest::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace ProxyConnectRequest
namespace ProxyDisconnectRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ProxyDisconnectRequest::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace ProxyDisconnectRequest
namespace ProxyScanRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ProxyScanRequest::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ProxyScanRequest
namespace ProxyBackGroundScanStartRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ProxyBackGroundScanStartRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace ProxyBackGroundScanStartRequest
namespace ProxyBackGroundScanStopRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ProxyBackGroundScanStopRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace ProxyBackGroundScanStopRequest
namespace ProxyMessageRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ProxyMessageRequest::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace ProxyMessageRequest

} // namespace Commands

namespace Events {} // namespace Events
} // namespace CommissioningProxy
} // namespace Clusters
} // namespace app
} // namespace chip
