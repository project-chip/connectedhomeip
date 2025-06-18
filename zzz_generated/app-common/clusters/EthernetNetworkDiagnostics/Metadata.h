// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster EthernetNetworkDiagnostics (cluster code: 55/0x37)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/EthernetNetworkDiagnostics/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace EthernetNetworkDiagnostics {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace PHYRate {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PHYRate::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PHYRate
namespace FullDuplex {
inline constexpr DataModel::AttributeEntry kMetadataEntry(FullDuplex::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace FullDuplex
namespace PacketRxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PacketRxCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PacketRxCount
namespace PacketTxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PacketTxCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PacketTxCount
namespace TxErrCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TxErrCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TxErrCount
namespace CollisionCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CollisionCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CollisionCount
namespace OverrunCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(OverrunCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace OverrunCount
namespace CarrierDetect {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CarrierDetect::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CarrierDetect
namespace TimeSinceReset {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TimeSinceReset::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TimeSinceReset

} // namespace Attributes

namespace Commands {
namespace ResetCounts {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ResetCounts::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kManage);
} // namespace ResetCounts

} // namespace Commands
} // namespace EthernetNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
