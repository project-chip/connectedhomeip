// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster BasicInformation (cluster code: 40/0x28)
// based on src/controller/data_model/controller-clusters.matter

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace clusters {
namespace BasicInformation {

inline constexpr ClusterId kClusterId = 0x00000028;

namespace Attributes {
namespace DataModelRevision {
inline constexpr AttributeId Id = 0x00000000;
} // namespace DataModelRevision
namespace VendorName {
inline constexpr AttributeId Id = 0x00000001;
} // namespace VendorName
namespace VendorID {
inline constexpr AttributeId Id = 0x00000002;
} // namespace VendorID
namespace ProductName {
inline constexpr AttributeId Id = 0x00000003;
} // namespace ProductName
namespace ProductID {
inline constexpr AttributeId Id = 0x00000004;
} // namespace ProductID
namespace NodeLabel {
inline constexpr AttributeId Id = 0x00000005;
} // namespace NodeLabel
namespace Location {
inline constexpr AttributeId Id = 0x00000006;
} // namespace Location
namespace HardwareVersion {
inline constexpr AttributeId Id = 0x00000007;
} // namespace HardwareVersion
namespace HardwareVersionString {
inline constexpr AttributeId Id = 0x00000008;
} // namespace HardwareVersionString
namespace SoftwareVersion {
inline constexpr AttributeId Id = 0x00000009;
} // namespace SoftwareVersion
namespace SoftwareVersionString {
inline constexpr AttributeId Id = 0x0000000A;
} // namespace SoftwareVersionString
namespace ManufacturingDate {
inline constexpr AttributeId Id = 0x0000000B;
} // namespace ManufacturingDate
namespace PartNumber {
inline constexpr AttributeId Id = 0x0000000C;
} // namespace PartNumber
namespace ProductURL {
inline constexpr AttributeId Id = 0x0000000D;
} // namespace ProductURL
namespace ProductLabel {
inline constexpr AttributeId Id = 0x0000000E;
} // namespace ProductLabel
namespace SerialNumber {
inline constexpr AttributeId Id = 0x0000000F;
} // namespace SerialNumber
namespace LocalConfigDisabled {
inline constexpr AttributeId Id = 0x00000010;
} // namespace LocalConfigDisabled
namespace Reachable {
inline constexpr AttributeId Id = 0x00000011;
} // namespace Reachable
namespace UniqueID {
inline constexpr AttributeId Id = 0x00000012;
} // namespace UniqueID
namespace CapabilityMinima {
inline constexpr AttributeId Id = 0x00000013;
} // namespace CapabilityMinima
namespace ProductAppearance {
inline constexpr AttributeId Id = 0x00000014;
} // namespace ProductAppearance
namespace SpecificationVersion {
inline constexpr AttributeId Id = 0x00000015;
} // namespace SpecificationVersion
namespace MaxPathsPerInvoke {
inline constexpr AttributeId Id = 0x00000016;
} // namespace MaxPathsPerInvoke

// TODO: globals & reference globals?

} // namespace Attributes

namespace Commands {
namespace MfgSpecificPing {
inline constexpr CommandId Id = 0x00000000;
} // namespace MfgSpecificPing
} // namespace Commands

namespace Events {
namespace StartUp {
inline constexpr CommandId Id = 0x00000000;
} // namespace StartUp
namespace ShutDown {
inline constexpr CommandId Id = 0x00000001;
} // namespace ShutDown
namespace Leave {
inline constexpr CommandId Id = 0x00000002;
} // namespace Leave
namespace ReachableChanged {
inline constexpr CommandId Id = 0x00000003;
} // namespace ReachableChanged
} // namespace Events
} // namespace BasicInformation
} // namespace clusters
} // namespace app
} // namespace chip
