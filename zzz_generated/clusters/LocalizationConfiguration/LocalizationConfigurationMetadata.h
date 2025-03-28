// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster LocalizationConfiguration (cluster code: 43/0x2B)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/LocalizationConfiguration/LocalizationConfigurationIds.h>

namespace chip {
namespace app {
namespace clusters {
namespace LocalizationConfiguration {
namespace Metadata {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {

inline constexpr DataModel::AttributeEntry kActiveLocaleEntry = {
    .attributeId    = LocalizationConfiguration::Attributes::ActiveLocale::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{},
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = Access::Privilege::kManage,
};
inline constexpr DataModel::AttributeEntry kSupportedLocalesEntry = {
    .attributeId    = LocalizationConfiguration::Attributes::SupportedLocales::Id,
    .flags          = BitFlags<DataModel::AttributeQualityFlags>{ DataModel::AttributeQualityFlags::kListAttribute },
    .readPrivilege  = Access::Privilege::kView,
    .writePrivilege = std::nullopt,
};

} // namespace Attributes

namespace Commands {} // namespace Commands
} // namespace Metadata
} // namespace LocalizationConfiguration
} // namespace clusters
} // namespace app
} // namespace chip
