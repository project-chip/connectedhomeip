// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster ThreadBorderRouterManagement (cluster code: 1106/0x452)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/ThreadBorderRouterManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace BorderRouterName {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BorderRouterName::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BorderRouterName
namespace BorderAgentID {
inline constexpr DataModel::AttributeEntry kMetadataEntry(BorderAgentID::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace BorderAgentID
namespace ThreadVersion {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ThreadVersion::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ThreadVersion
namespace InterfaceEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(InterfaceEnabled::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace InterfaceEnabled
namespace ActiveDatasetTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry(ActiveDatasetTimestamp::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace ActiveDatasetTimestamp
namespace PendingDatasetTimestamp {
inline constexpr DataModel::AttributeEntry kMetadataEntry(PendingDatasetTimestamp::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace PendingDatasetTimestamp

} // namespace Attributes

namespace Commands {
namespace GetActiveDatasetRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(GetActiveDatasetRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace GetActiveDatasetRequest
namespace GetPendingDatasetRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(GetPendingDatasetRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kManage);
} // namespace GetPendingDatasetRequest
namespace SetActiveDatasetRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetActiveDatasetRequest::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kManage);
} // namespace SetActiveDatasetRequest
namespace SetPendingDatasetRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(SetPendingDatasetRequest::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kTimed),
                   Access::Privilege::kManage);
} // namespace SetPendingDatasetRequest

} // namespace Commands
} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
