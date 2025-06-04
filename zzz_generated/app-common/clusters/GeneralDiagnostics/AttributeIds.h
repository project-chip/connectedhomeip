// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster GeneralDiagnostics (cluster code: 51/0x33)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <clusters/shared/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralDiagnostics {
namespace Attributes {
namespace NetworkInterfaces {
inline constexpr AttributeId Id = 0x00000000;
} // namespace NetworkInterfaces

namespace RebootCount {
inline constexpr AttributeId Id = 0x00000001;
} // namespace RebootCount

namespace UpTime {
inline constexpr AttributeId Id = 0x00000002;
} // namespace UpTime

namespace TotalOperationalHours {
inline constexpr AttributeId Id = 0x00000003;
} // namespace TotalOperationalHours

namespace BootReason {
inline constexpr AttributeId Id = 0x00000004;
} // namespace BootReason

namespace ActiveHardwareFaults {
inline constexpr AttributeId Id = 0x00000005;
} // namespace ActiveHardwareFaults

namespace ActiveRadioFaults {
inline constexpr AttributeId Id = 0x00000006;
} // namespace ActiveRadioFaults

namespace ActiveNetworkFaults {
inline constexpr AttributeId Id = 0x00000007;
} // namespace ActiveNetworkFaults

namespace TestEventTriggersEnabled {
inline constexpr AttributeId Id = 0x00000008;
} // namespace TestEventTriggersEnabled

namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList

namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList

namespace AttributeList {
inline constexpr AttributeId Id = Globals::Attributes::AttributeList::Id;
} // namespace AttributeList

namespace FeatureMap {
inline constexpr AttributeId Id = Globals::Attributes::FeatureMap::Id;
} // namespace FeatureMap

namespace ClusterRevision {
inline constexpr AttributeId Id = Globals::Attributes::ClusterRevision::Id;
} // namespace ClusterRevision

} // namespace Attributes
} // namespace GeneralDiagnostics
} // namespace Clusters
} // namespace app
} // namespace chip
