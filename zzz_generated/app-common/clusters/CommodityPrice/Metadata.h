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
inline constexpr DataModel::AttributeEntry kMetadataEntry(TariffUnit::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace TariffUnit
namespace Currency {
inline constexpr DataModel::AttributeEntry kMetadataEntry(Currency::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace Currency
namespace CurrentPrice {
inline constexpr DataModel::AttributeEntry kMetadataEntry(CurrentPrice::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace CurrentPrice
namespace PriceForecast {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(PriceForecast::Id, BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace PriceForecast

} // namespace Attributes

namespace Commands {
namespace GetDetailedPriceRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(GetDetailedPriceRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace GetDetailedPriceRequest
namespace GetDetailedForecastRequest {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(GetDetailedForecastRequest::Id, BitFlags<DataModel::CommandQualityFlags>(), Access::Privilege::kOperate);
} // namespace GetDetailedForecastRequest

} // namespace Commands
} // namespace CommodityPrice
} // namespace Clusters
} // namespace app
} // namespace chip
