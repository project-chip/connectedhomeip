// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LocalizationConfiguration (cluster code: 43/0x2B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/LocalizationConfiguration/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace LocalizationConfiguration {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace ActiveLocale {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::ActiveLocale::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
} // namespace ActiveLocale
namespace SupportedLocales {
inline constexpr DataModel::AttributeEntry kMetadataEntry = {
    .attributeId    = Attributes::SupportedLocales::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};
} // namespace SupportedLocales

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace LocalizationConfiguration
} // namespace Clusters
} // namespace app
} // namespace chip
