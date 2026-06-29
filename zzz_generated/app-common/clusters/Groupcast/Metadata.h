// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster Groupcast (cluster code: 101/0x65)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/Groupcast/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace Groupcast {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace Membership {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(Membership::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace Membership
namespace MaxMembershipCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxMembershipCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxMembershipCount
namespace MaxMcastAddrCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxMcastAddrCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxMcastAddrCount
namespace UsedMcastAddrCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(UsedMcastAddrCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace UsedMcastAddrCount
namespace FabricUnderTest {
inline constexpr DataModel::AttributeEntry kMetadataEntry(FabricUnderTest::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace FabricUnderTest
constexpr std::array<DataModel::AttributeEntry, 5> kMandatoryMetadata = {
    Membership::kMetadataEntry,         MaxMembershipCount::kMetadataEntry, MaxMcastAddrCount::kMetadataEntry,
    UsedMcastAddrCount::kMetadataEntry, FabricUnderTest::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace JoinGroup {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(JoinGroup::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace JoinGroup
namespace LeaveGroup {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(LeaveGroup::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace LeaveGroup
namespace UpdateGroupKey {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(UpdateGroupKey::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kManage);
} // namespace UpdateGroupKey
namespace ConfigureAuxiliaryACL {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ConfigureAuxiliaryACL::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace ConfigureAuxiliaryACL
namespace GroupcastTesting {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(GroupcastTesting::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace GroupcastTesting

} // namespace Commands

namespace Events {
namespace GroupcastTesting {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kAdminister };
} // namespace GroupcastTesting

} // namespace Events
} // namespace Groupcast
} // namespace Clusters
} // namespace app
} // namespace chip
