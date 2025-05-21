// DO NOT EDIT - Generated file
//
// Application configuration for OperationalCredentials based on EMBER configuration
// from inputs/large_all_clusters_app.matter
#pragma once

#include <app/util/cluster-config.h>
#include <clusters/OperationalCredentials/AttributeIds.h>
#include <clusters/OperationalCredentials/CommandIds.h>
#include <clusters/OperationalCredentials/Enums.h>

#include <array>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalCredentials {
namespace StaticApplicationConfig {
namespace detail {
inline constexpr AttributeId kEndpoint0EnabledAttributes[] = {
    Attributes::AcceptedCommandList::Id,
    Attributes::AttributeList::Id,
    Attributes::ClusterRevision::Id,
    Attributes::CommissionedFabrics::Id,
    Attributes::CurrentFabricIndex::Id,
    Attributes::Fabrics::Id,
    Attributes::FeatureMap::Id,
    Attributes::GeneratedCommandList::Id,
    Attributes::NOCs::Id,
    Attributes::SupportedFabrics::Id,
    Attributes::TrustedRootCertificates::Id,
};

inline constexpr CommandId kEndpoint0EnabledCommands[] = {
    Commands::AddNOC::Id,
    Commands::AddTrustedRootCertificate::Id,
    Commands::AttestationRequest::Id,
    Commands::AttestationResponse::Id,
    Commands::CertificateChainRequest::Id,
    Commands::CertificateChainResponse::Id,
    Commands::CSRRequest::Id,
    Commands::CSRResponse::Id,
    Commands::NOCResponse::Id,
    Commands::RemoveFabric::Id,
    Commands::SetVIDVerificationStatement::Id,
    Commands::SignVIDVerificationRequest::Id,
    Commands::SignVIDVerificationResponse::Id,
    Commands::UpdateFabricLabel::Id,
    Commands::UpdateNOC::Id,
};

} // namespace detail

using FeatureBitmapType = Clusters::StaticApplicationConfig::NoFeatureFlagsDefined;

inline constexpr std::array<Clusters::StaticApplicationConfig::ClusterConfiguration<FeatureBitmapType>, 1> kFixedClusterConfig = { {
    {
        .endpointNumber = 0,
        .featureMap = BitFlags<FeatureBitmapType> {
        },
        .enabledAttributes = Span<const AttributeId>(detail::kEndpoint0EnabledAttributes),
        .enabledCommands = Span<const CommandId>(detail::kEndpoint0EnabledCommands),
    },
} };

// If a specific attribute is supported at all across all endpoint static instantiations
inline constexpr bool IsAttributeEnabledOnSomeEndpoint(AttributeId attributeId) {
  switch (attributeId) {
    case Attributes::NOCs::Id:
    case Attributes::AcceptedCommandList::Id:
    case Attributes::AttributeList::Id:
    case Attributes::ClusterRevision::Id:
    case Attributes::CommissionedFabrics::Id:
    case Attributes::CurrentFabricIndex::Id:
    case Attributes::Fabrics::Id:
    case Attributes::FeatureMap::Id:
    case Attributes::GeneratedCommandList::Id:
    case Attributes::SupportedFabrics::Id:
    case Attributes::TrustedRootCertificates::Id:
      return true;
    default:
      return false;
  }
}

// If a specific command is supported at all across all endpoint static instantiations
inline constexpr bool IsCommandEnabledOnSomeEndpoint(CommandId commandId) {
  switch (commandId) {
    case Commands::AddNOC::Id:
    case Commands::AddTrustedRootCertificate::Id:
    case Commands::AttestationRequest::Id:
    case Commands::AttestationResponse::Id:
    case Commands::CSRRequest::Id:
    case Commands::CSRResponse::Id:
    case Commands::CertificateChainRequest::Id:
    case Commands::CertificateChainResponse::Id:
    case Commands::NOCResponse::Id:
    case Commands::RemoveFabric::Id:
    case Commands::SetVIDVerificationStatement::Id:
    case Commands::SignVIDVerificationRequest::Id:
    case Commands::SignVIDVerificationResponse::Id:
    case Commands::UpdateFabricLabel::Id:
    case Commands::UpdateNOC::Id:
      return true;
    default:
      return false;
  }
}

} // namespace StaticApplicationConfig
} // namespace OperationalCredentials
} // namespace Clusters
} // namespace app
} // namespace chip

