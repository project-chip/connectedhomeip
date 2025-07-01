// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Actions (cluster code: 37/0x25)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Actions/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Actions {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace ActionList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActionList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActionList
namespace EndpointLists {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(EndpointLists::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace EndpointLists
namespace SetupURL {
inline constexpr DataModel::AttributeEntry kMetadataEntry(SetupURL::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace SetupURL

} // namespace Attributes

namespace Commands {
namespace InstantAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(InstantAction::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace InstantAction
namespace InstantActionWithTransition {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(InstantActionWithTransition::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace InstantActionWithTransition
namespace StartAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StartAction::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StartAction
namespace StartActionWithDuration {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(StartActionWithDuration::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace StartActionWithDuration
namespace StopAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(StopAction::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace StopAction
namespace PauseAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(PauseAction::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace PauseAction
namespace PauseActionWithDuration {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(PauseActionWithDuration::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace PauseActionWithDuration
namespace ResumeAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ResumeAction::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ResumeAction
namespace EnableAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(EnableAction::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace EnableAction
namespace EnableActionWithDuration {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(EnableActionWithDuration::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace EnableActionWithDuration
namespace DisableAction {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(DisableAction::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace DisableAction
namespace DisableActionWithDuration {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(DisableActionWithDuration::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace DisableActionWithDuration

} // namespace Commands
} // namespace Actions
} // namespace Clusters
} // namespace app
} // namespace chip
