// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CommodityTariff (cluster code: 1792/0x700)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityTariff {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 2;

namespace GetTariffComponent {
inline constexpr CommandId Id = 0x00000000;
} // namespace GetTariffComponent

namespace GetDayEntry {
inline constexpr CommandId Id = 0x00000001;
} // namespace GetDayEntry

namespace GetTariffComponentResponse {
inline constexpr CommandId Id = 0x00000000;
} // namespace GetTariffComponentResponse

namespace GetDayEntryResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace GetDayEntryResponse

} // namespace Commands
} // namespace CommodityTariff
} // namespace Clusters
} // namespace app
} // namespace chip
