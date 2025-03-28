// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster GeneralCommissioning (cluster code: 48/0x30)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/common/GlobalIds.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace GeneralCommissioning {

inline constexpr ClusterId Id = 0x00000030;

namespace Attributes {
namespace Breadcrumb {
inline constexpr AttributeId Id = 0x00000000;
} // namespace Breadcrumb
namespace BasicCommissioningInfo {
inline constexpr AttributeId Id = 0x00000001;
} // namespace BasicCommissioningInfo
namespace RegulatoryConfig {
inline constexpr AttributeId Id = 0x00000002;
} // namespace RegulatoryConfig
namespace LocationCapability {
inline constexpr AttributeId Id = 0x00000003;
} // namespace LocationCapability
namespace SupportsConcurrentConnection {
inline constexpr AttributeId Id = 0x00000004;
} // namespace SupportsConcurrentConnection
namespace TCAcceptedVersion {
inline constexpr AttributeId Id = 0x00000005;
} // namespace TCAcceptedVersion
namespace TCMinRequiredVersion {
inline constexpr AttributeId Id = 0x00000006;
} // namespace TCMinRequiredVersion
namespace TCAcknowledgements {
inline constexpr AttributeId Id = 0x00000007;
} // namespace TCAcknowledgements
namespace TCAcknowledgementsRequired {
inline constexpr AttributeId Id = 0x00000008;
} // namespace TCAcknowledgementsRequired
namespace TCUpdateDeadline {
inline constexpr AttributeId Id = 0x00000009;
} // namespace TCUpdateDeadline
namespace GeneratedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::GeneratedCommandList::Id;
} // namespace GeneratedCommandList
namespace AcceptedCommandList {
inline constexpr AttributeId Id = Globals::Attributes::AcceptedCommandList::Id;
} // namespace AcceptedCommandList
namespace EventList {
inline constexpr AttributeId Id = Globals::Attributes::EventList::Id;
} // namespace EventList
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

namespace Commands {
namespace ArmFailSafe {
inline constexpr CommandId Id = 0x00000000;
} // namespace ArmFailSafe
namespace SetRegulatoryConfig {
inline constexpr CommandId Id = 0x00000002;
} // namespace SetRegulatoryConfig
namespace CommissioningComplete {
inline constexpr CommandId Id = 0x00000004;
} // namespace CommissioningComplete
namespace SetTCAcknowledgements {
inline constexpr CommandId Id = 0x00000006;
} // namespace SetTCAcknowledgements
namespace ArmFailSafeResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace ArmFailSafeResponse
namespace SetRegulatoryConfigResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace SetRegulatoryConfigResponse
namespace CommissioningCompleteResponse {
inline constexpr CommandId Id = 0x00000005;
} // namespace CommissioningCompleteResponse
namespace SetTCAcknowledgementsResponse {
inline constexpr CommandId Id = 0x00000007;
} // namespace SetTCAcknowledgementsResponse
} // namespace Commands

namespace Events {} // namespace Events

} // namespace GeneralCommissioning
} // namespace Clusters
} // namespace app
} // namespace chip
