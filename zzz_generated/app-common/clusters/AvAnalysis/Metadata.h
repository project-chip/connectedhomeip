// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster AvAnalysis (cluster code: 1367/0x557)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <array>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/AvAnalysis/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace AvAnalysis {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

namespace SupportedAmbientContexts {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(SupportedAmbientContexts::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace SupportedAmbientContexts
namespace ActiveAmbientContextTriggers {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ActiveAmbientContextTriggers::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ActiveAmbientContextTriggers
namespace MaxAnalysisStreamCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxAnalysisStreamCount::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxAnalysisStreamCount
namespace CurrentAnalysisStreamCount {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentAnalysisStreamCount::Id,
                                                          BitFlags<DataModel::AttributeQualityFlags>(), Access::Privilege::kView,
                                                          std::nullopt);
} // namespace CurrentAnalysisStreamCount
namespace AnalysisStreams {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(AnalysisStreams::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace AnalysisStreams
namespace TrackingEnabled {
inline constexpr DataModel::AttributeEntry kMetadataEntry(TrackingEnabled::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kManage, Access::Privilege::kManage);
} // namespace TrackingEnabled
constexpr std::array<DataModel::AttributeEntry, 3> kMandatoryMetadata = {
    SupportedAmbientContexts::kMetadataEntry,
    ActiveAmbientContextTriggers::kMetadataEntry,
    TrackingEnabled::kMetadataEntry,

};

} // namespace Attributes

namespace Commands {

namespace EnableContextTriggers {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(EnableContextTriggers::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace EnableContextTriggers
namespace DisableContextTriggers {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(DisableContextTriggers::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace DisableContextTriggers
namespace EstablishAnalysisStream {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(EstablishAnalysisStream::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace EstablishAnalysisStream
namespace ActivateAnalysisStream {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ActivateAnalysisStream::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace ActivateAnalysisStream
namespace DeactivateAnalysisStream {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(DeactivateAnalysisStream::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace DeactivateAnalysisStream
namespace RemoveAnalysisStream {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveAnalysisStream::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace RemoveAnalysisStream

} // namespace Commands

namespace Events {
namespace AnalysisSessionStart {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace AnalysisSessionStart
namespace AnalysisSessionEnd {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace AnalysisSessionEnd
namespace PerceivedContext {
inline constexpr DataModel::EventEntry kMetadataEntry{ Access::Privilege::kView };
} // namespace PerceivedContext

} // namespace Events
} // namespace AvAnalysis
} // namespace Clusters
} // namespace app
} // namespace chip
