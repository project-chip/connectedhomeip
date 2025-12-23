/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <credentials/jcm/TrustVerification.h>
#include <credentials/jcm/VendorIdVerificationClient.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandlerInterface.h>
#include <app/server/Server.h>
#include <controller/ReadInteraction.h>
#include <controller/TypedReadCallback.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

#include <functional>
#include <optional>

namespace chip {
namespace app {
namespace Clusters {
namespace JointFabricAdministrator {

/**
 * JCMCommissionee handles the Joint Commissioning Method (JCM) trust verification process where the commissionee verifies trust
 * against the commissioner.
 *
 * An instance of JCMCommissionee represents one invocation of the trust verification process, meaning that its lifetime should
 * expire when trust verification is complete.
 */
class JCMCommissionee : public Credentials::JCM::VendorIdVerificationClient, public Credentials::JCM::TrustVerificationStateMachine
{
public:
    using OnCompletionFunc = std::function<void(CHIP_ERROR)>;

    JCMCommissionee(CommandHandler::Handle & commandHandle, EndpointId endpointId, OnCompletionFunc onCompletion) :
        mCommandHandle(commandHandle), mOnCompletion(onCompletion)
    {
        mInfo.adminEndpointId = endpointId;
        mAccessingFabricIndex = mCommandHandle.Get()->GetAccessingFabricIndex();
        // Extract the session from the exchange before going async.
        // GetExchangeContext() must not be be called once we cross the async boundary,
        // since the exchange is not guaranteed to remain valid.
        SessionHandle sessionHandle = mCommandHandle.Get()->GetExchangeContext()->GetSessionHandle();
        mSessionHolder.Grab(sessionHandle);
    }
    ~JCMCommissionee() {}

    /**
     * Asynchronously initiates the trust verification process.
     */
    void VerifyTrustAgainstCommissionerAdmin();

protected:
    // VendorIdVerificationClient overrides
    CHIP_ERROR OnLookupOperationalTrustAnchor(VendorId vendorID, Credentials::CertificateKeyId & subjectKeyId,
                                              ByteSpan & globallyTrustedRootSpan) override;
    void OnVendorIdVerificationComplete(const CHIP_ERROR & err) override;

    // TrustVerificationStateMachine overrides
    Credentials::JCM::TrustVerificationStage
    GetNextTrustVerificationStage(const Credentials::JCM::TrustVerificationStage & currentStage) override;
    void PerformTrustVerificationStage(const Credentials::JCM::TrustVerificationStage & nextStage) override;
    void OnTrustVerificationComplete(Credentials::JCM::TrustVerificationError error) override;

    // Wrappers around chip::Controller::ReadAttribute to simplify callsites and facilitate unit testing
    using ReadErrorHandler = std::function<void(const ConcreteAttributePath *, CHIP_ERROR err)>;
    using FabricIndexAttr  = Clusters::JointFabricAdministrator::Attributes::AdministratorFabricIndex::TypeInfo;
    using FabricsAttr      = Clusters::OperationalCredentials::Attributes::Fabrics::TypeInfo;
    using CertsAttr        = Clusters::OperationalCredentials::Attributes::TrustedRootCertificates::TypeInfo;
    using NOCsAttr         = Clusters::OperationalCredentials::Attributes::NOCs::TypeInfo;
    virtual CHIP_ERROR ReadAdminFabricIndexAttribute(
        std::function<void(const ConcreteAttributePath &, const FabricIndexAttr::DecodableType &)> onSuccess,
        ReadErrorHandler onError);
    virtual CHIP_ERROR
    ReadAdminFabricsAttribute(std::function<void(const ConcreteAttributePath &, const FabricsAttr::DecodableType &)> onSuccess,
                              ReadErrorHandler onError);
    virtual CHIP_ERROR
    ReadAdminCertsAttribute(std::function<void(const ConcreteAttributePath &, const CertsAttr::DecodableType &)> onSuccess,
                            ReadErrorHandler onError);
    virtual CHIP_ERROR
    ReadAdminNOCsAttribute(std::function<void(const ConcreteAttributePath &, const NOCsAttr::DecodableType &)> onSuccess,
                           ReadErrorHandler onError);
    template <typename T>
    CHIP_ERROR ReadAttribute(EndpointId endpointId,
                             std::function<void(const ConcreteAttributePath &, const typename T::DecodableType &)> onSuccess,
                             std::function<void(const ConcreteAttributePath *, CHIP_ERROR err)> onError, const bool fabricFiltered)
    {
        VerifyOrReturnError(mSessionHolder, CHIP_ERROR_INCORRECT_STATE);

        chip::Messaging::ExchangeManager * exchangeMgr = &chip::Server::GetInstance().GetExchangeManager();

        return chip::Controller::ReadAttribute<T>(exchangeMgr, mSessionHolder.Get().Value(), endpointId, onSuccess, onError,
                                                  fabricFiltered);
    }

private:
    CommandHandler::Handle & mCommandHandle;
    OnCompletionFunc mOnCompletion;
    FabricIndex mAccessingFabricIndex;
    SessionHolder mSessionHolder;

    // Trust Verification Stages
    /**
     * Implements the following passage from JCM Trust Verification:
     * Ecosystem B Administrator SHALL save the value of the EndpointID
     *
     * Requires that mInfo.adminEndpointId is set to a valid endpoint.
     */
    Credentials::JCM::TrustVerificationError StoreEndpointId();
    /**
     * Implements the following passage from JCM Trust Verification:
     * Ecosystem B Administrator SHALL read the AdministratorFabricIndex attribute of the Joint Fabric Administrator cluster
     * belonging to JointEndPointA on Ecosystem A Administrator...
     */
    Credentials::JCM::TrustVerificationError ReadCommissionerAdminFabricIndex();
    /**
     * Implements the following passage from JCM Trust Verification:
     * ... and executes Fabric Table Vendor ID Verification Procedure against the Fabric indicated by AdministratorFabricIndex (i.e.
     * FabricIndex corresponding to Fabric A).
     */
    Credentials::JCM::TrustVerificationError PerformVendorIdVerification();
    //
    /**
     * Implements the following passage from JCM Trust Verification:
     * Ecosystem B Administrator SHALL check that the RootPublicKey and FabricID of the accessing fabric (found in the
     * FabricDescriptorStruct) match the RootPublicKey and FabricID of the Fabric indicated by AdministratorFabricIndex.
     */
    Credentials::JCM::TrustVerificationError CrossCheckAdministratorIds();

    /**
     * Checks that the accessing fabric's identity matches the administrator metadata discovered during verification.
     */
    Credentials::JCM::TrustVerificationError ValidateAdministratorIdsMatch(FabricId accessingFabricId,
                                                                           const Crypto::P256PublicKey & accessingRootPubKey) const;

    /**
     * Asynchronously reads attributes from the commissioning administrator to gather data needed for Vendor ID verification.
     */
    void FetchCommissionerInfo(OnCompletionFunc onComplete);
    /**
     * Asynchronously reads needed information from the Fabrics attribute on the OperationalCredentials cluster
     */
    CHIP_ERROR ReadAdminFabrics(OnCompletionFunc onComplete);
    /**
     * Asynchronously reads needed information from the TrustedRootCertificates attribute on the OperationalCredentials cluster
     */
    CHIP_ERROR ReadAdminCerts(OnCompletionFunc onComplete);
    /**
     * Asynchronously reads needed information from the TrustedRootCertificates attribute on the OperationalCredentials cluster
     */
    CHIP_ERROR ReadAdminNOCs(OnCompletionFunc onComplete);

    friend class TestJCMCommissionee;
};

} // namespace JointFabricAdministrator
} // namespace Clusters
} // namespace app
} // namespace chip
