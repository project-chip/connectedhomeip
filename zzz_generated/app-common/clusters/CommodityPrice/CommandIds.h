// DO NOT EDIT MANUALLY - Generated file
//
// Identifier constant values for cluster CommodityPrice (cluster code: 149/0x95)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <lib/core/DataModelTypes.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityPrice {
namespace Commands {

// Total number of client to server commands supported by the cluster
inline constexpr uint32_t kAcceptedCommandsCount = 2;

// Total number of server to client commands supported by the cluster (response commands)
inline constexpr uint32_t kGeneratedCommandsCount = 2;

namespace GetDetailedPriceRequest {
inline constexpr CommandId Id = 0x00000000;
} // namespace GetDetailedPriceRequest

namespace GetDetailedForecastRequest {
inline constexpr CommandId Id = 0x00000002;
} // namespace GetDetailedForecastRequest

namespace GetDetailedPriceResponse {
inline constexpr CommandId Id = 0x00000001;
} // namespace GetDetailedPriceResponse

namespace GetDetailedForecastResponse {
inline constexpr CommandId Id = 0x00000003;
} // namespace GetDetailedForecastResponse

} // namespace Commands
} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
