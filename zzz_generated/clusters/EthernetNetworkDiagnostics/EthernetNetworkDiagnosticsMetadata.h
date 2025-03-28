// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster EthernetNetworkDiagnostics (cluster code: 55/0x37)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/EthernetNetworkDiagnostics/EthernetNetworkDiagnosticsIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace EthernetNetworkDiagnostics {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kPHYRateEntry = {
    .attributeId    = Attributes::PHYRate::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kFullDuplexEntry = {
    .attributeId    = Attributes::FullDuplex::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPacketRxCountEntry = {
    .attributeId    = Attributes::PacketRxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kPacketTxCountEntry = {
    .attributeId    = Attributes::PacketTxCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTxErrCountEntry = {
    .attributeId    = Attributes::TxErrCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCollisionCountEntry = {
    .attributeId    = Attributes::CollisionCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kOverrunCountEntry = {
    .attributeId    = Attributes::OverrunCount::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCarrierDetectEntry = {
    .attributeId    = Attributes::CarrierDetect::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kTimeSinceResetEntry = {
    .attributeId    = Attributes::TimeSinceReset::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kResetCountsEntry = {
    .commandId       = Commands::ResetCounts::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kManage,
};

} // namespace Commands
} // namespace Metadata
} // namespace EthernetNetworkDiagnostics
} // namespace clusters
} // namespace app
} // namespace chip
