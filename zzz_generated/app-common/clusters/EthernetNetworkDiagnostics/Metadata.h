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
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PHYRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PHYRate
namespace FullDuplex {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = FullDuplex::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace FullDuplex
namespace PacketRxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PacketRxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PacketRxCount
namespace PacketTxCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = PacketTxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PacketTxCount
namespace TxErrCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TxErrCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TxErrCount
namespace CollisionCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CollisionCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CollisionCount
namespace OverrunCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = OverrunCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace OverrunCount
namespace CarrierDetect {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = CarrierDetect::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CarrierDetect
namespace TimeSinceReset {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = TimeSinceReset::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TimeSinceReset

} // namespace Attributes

namespace Commands {
namespace ResetCounts {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = ResetCounts::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};
} // namespace ResetCounts

} // namespace Commands
} // namespace EthernetNetworkDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
