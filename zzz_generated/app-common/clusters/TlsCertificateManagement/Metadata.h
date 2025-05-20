// DO NOT EDIT MANUALLY - Generated file
//
// Cluster metadata information for cluster TlsCertificateManagement (cluster code: 2049/0x801)
// based on src/controller/data_model/controller-clusters.matter
#pragma once

#include <app/data-model-provider/MetadataTypes.h>
#include <lib/core/DataModelTypes.h>

#include <cstdint>

#include <clusters/TlsCertificateManagement/Ids.h>

namespace chip {
namespace app {
namespace Clusters {
namespace TlsCertificateManagement {

inline constexpr uint32_t kRevision = 1;

namespace Attributes {
namespace MaxRootCertificates {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxRootCertificates::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxRootCertificates
namespace ProvisionedRootCertificates {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ProvisionedRootCertificates::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ProvisionedRootCertificates
namespace MaxClientCertificates {
inline constexpr DataModel::AttributeEntry kMetadataEntry(MaxClientCertificates::Id, BitFlags<DataModel::AttributeQualityFlags>(),
                                                          Access::Privilege::kView, std::nullopt);
} // namespace MaxClientCertificates
namespace ProvisionedClientCertificates {
inline constexpr DataModel::AttributeEntry
    kMetadataEntry(ProvisionedClientCertificates::Id,
                   BitFlags<DataModel::AttributeQualityFlags>(DataModel::AttributeQualityFlags::kListAttribute),
                   Access::Privilege::kView, std::nullopt);
} // namespace ProvisionedClientCertificates

} // namespace Attributes

namespace Commands {
namespace ProvisionRootCertificate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ProvisionRootCertificate::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace ProvisionRootCertificate
namespace FindRootCertificate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(FindRootCertificate::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace FindRootCertificate
namespace LookupRootCertificate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(LookupRootCertificate::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace LookupRootCertificate
namespace RemoveRootCertificate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveRootCertificate::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace RemoveRootCertificate
namespace TLSClientCSR {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(TLSClientCSR::Id, BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace TLSClientCSR
namespace ProvisionClientCertificate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(ProvisionClientCertificate::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace ProvisionClientCertificate
namespace FindClientCertificate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(FindClientCertificate::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace FindClientCertificate
namespace LookupClientCertificate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(LookupClientCertificate::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kOperate);
} // namespace LookupClientCertificate
namespace RemoveClientCertificate {
inline constexpr DataModel::AcceptedCommandEntry
    kMetadataEntry(RemoveClientCertificate::Id,
                   BitFlags<DataModel::CommandQualityFlags>(DataModel::CommandQualityFlags::kFabricScoped),
                   Access::Privilege::kAdminister);
} // namespace RemoveClientCertificate

} // namespace Commands
} // namespace TlsCertificateManagement
} // namespace Clusters
} // namespace app
} // namespace chip
