// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AppleBluetoothDiagnostics (cluster code: 323615751/0x1349FC07)
// based on ../../../connectedhomeip/src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AppleBluetoothDiagnostics/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AppleBluetoothDiagnostics {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace AppleBluetoothVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AppleBluetoothVersion
namespace AppleBluetoothRSSI {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothRSSI::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace AppleBluetoothRSSI
namespace AppleBluetoothTxPowerLevel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothTxPowerLevel::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothTxPowerLevel
namespace AppleBluetoothPacketRxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothPacketRxCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothPacketRxCount
namespace AppleBluetoothPacketTxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothPacketTxCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothPacketTxCount
namespace AppleBluetoothPacketRxErrorCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothPacketRxErrorCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothPacketRxErrorCount
namespace AppleBluetoothPacketTxErrorCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothPacketTxErrorCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothPacketTxErrorCount
namespace AppleBluetoothOverrunCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothOverrunCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothOverrunCount
namespace AppleBluetoothConnectionStatus {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothConnectionStatus::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothConnectionStatus
namespace AppleBluetoothConnectionCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothConnectionCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothConnectionCount
namespace AppleBluetoothConnectionErrorCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothConnectionErrorCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothConnectionErrorCount
namespace AppleBluetoothDisconnectionCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothDisconnectionCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothDisconnectionCount
namespace AppleBluetoothDisconnectionErrorCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothDisconnectionErrorCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothDisconnectionErrorCount
namespace AppleBluetoothHardwareExceptionCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothHardwareExceptionCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothHardwareExceptionCount
namespace AppleBluetoothResetCountBootRelativeTime {
inline constexpr DataModel::AttributeEntry kMetadataEntry(AppleBluetoothResetCountBootRelativeTime::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace AppleBluetoothResetCountBootRelativeTime
constexpr std::array<DataModel::AttributeEntry, 10> kMandatoryMetadata = {
    AppleBluetoothVersion::kMetadataEntry,
    AppleBluetoothRSSI::kMetadataEntry,
    AppleBluetoothTxPowerLevel::kMetadataEntry,
    AppleBluetoothOverrunCount::kMetadataEntry,
    AppleBluetoothConnectionStatus::kMetadataEntry,
    AppleBluetoothConnectionCount::kMetadataEntry,
    AppleBluetoothConnectionErrorCount::kMetadataEntry,
    AppleBluetoothDisconnectionCount::kMetadataEntry,
    AppleBluetoothDisconnectionErrorCount::kMetadataEntry,
    AppleBluetoothHardwareExceptionCount::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace ResetCounts {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ResetCounts::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ResetCounts

} // namespace Commands

namespace Events {} // namespace Events
} // namespace AppleBluetoothDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
