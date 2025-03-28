// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Channel (cluster code: 1284/0x504)
// based on src/controller/data_model/controller-clusters.matter

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Channel/ChannelIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace Channel {
namespace Metadata {

inline constexpr uint32_t kRevision = 2;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kChannelListEntry = {
    .attributeId    = Attributes::ChannelList::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kLineupEntry = {
    .attributeId    = Attributes::Lineup::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
inline constexpr DataModel::AttributeEntry kCurrentChannelEntry = {
    .attributeId    = Attributes::CurrentChannel::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {

inline constexpr DataModel::AcceptedCommandEntry kChangeChannelEntry = {
    .commandId       = Commands::ChangeChannel::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kChangeChannelByNumberEntry = {
    .commandId       = Commands::ChangeChannelByNumber::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kSkipChannelEntry = {
    .commandId       = Commands::SkipChannel::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kGetProgramGuideEntry = {
    .commandId       = Commands::GetProgramGuide::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kRecordProgramEntry = {
    .commandId       = Commands::RecordProgram::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
inline constexpr DataModel::AcceptedCommandEntry kCancelRecordProgramEntry = {
    .commandId       = Commands::CancelRecordProgram::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};

} // namespace Commands
} // namespace Metadata
} // namespace Channel
} // namespace clusters
} // namespace app
} // namespace chip
