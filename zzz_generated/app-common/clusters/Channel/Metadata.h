// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Channel (cluster code: 1284/0x504)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Channel/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Channel {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {
namespace ChannelList {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ChannelList::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ChannelList
namespace Lineup {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Lineup::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Lineup
namespace CurrentChannel {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentChannel::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentChannel

} // namespace Attributes

namespace Commands {
namespace ChangeChannel {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(ChangeChannel::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace ChangeChannel
namespace ChangeChannelByNumber {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ChangeChannelByNumber::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace ChangeChannelByNumber
namespace SkipChannel {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(SkipChannel::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace SkipChannel
namespace GetProgramGuide {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(GetProgramGuide::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace GetProgramGuide
namespace RecordProgram {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(RecordProgram::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace RecordProgram
namespace CancelRecordProgram {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry(CancelRecordProgram::Id, BitFlags<DataModel::CommandQualityFlags>(),
                                                                Access::Privilege::kOperate);
} // namespace CancelRecordProgram

} // namespace Commands
} // namespace Channel
} // namespace Clusters
} // namespace app
} // namespace chip
