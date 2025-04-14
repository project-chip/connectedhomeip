// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster CommodityPrice (cluster code: 149/0x95)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/CommodityPrice/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace CommodityPrice {

inline constexpr uint32_t kRevision = 4;

namespace Attributes {
namespace TariffUnit {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::TariffUnit::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace TariffUnit
namespace Currency {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::Currency::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace Currency
namespace CurrentPrice {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::CurrentPrice::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace CurrentPrice
namespace PriceForecast {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::PriceForecast::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace PriceForecast

} // namespace Attributes

namespace Commands {
namespace GetDetailedPriceRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::GetDetailedPriceRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GetDetailedPriceRequest
namespace GetDetailedForecastRequest {
inline constexpr DataModel::AcceptedCommandEntry kMetadataEntry = {
    .commandId       = Commands::GetDetailedForecastRequest::Id,
    .flags           = BitFlags<DataModel::CommandQualityFlags>{},
    .invokePrivilege = Access::Privilege::kOperate,
};
} // namespace GetDetailedForecastRequest

} // namespace Commands
} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
