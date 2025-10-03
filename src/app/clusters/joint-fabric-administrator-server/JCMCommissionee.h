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

#include <app/CommandHandlerInterface.h>
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

/*
 * JCMCommissionee is a class that handles the Joint Commissioning Management (JCM) process for Joint Fabric Administrator devices
 * being commissioned in a CHIP network. It implements the JCM trust verification process.
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
    }
    ~JCMCommissionee() {}

    /*
     * StartJCMTrustVerification is a method that initiates the JCM trust verification process for the device.
     * It is called by the commissionee to start the trust verification process as part of HandleAnnounceJointFabricAdministrator.
     * The method will return an error if the inputs are invalid or if the trust verification process fails.
     *
     * @return CHIP_ERROR indicating success or failure of the operation.
     */
    CHIP_ERROR VerifyTrustAgainstCommissionerAdmin();

protected:
    // VendorIdVerificationClient
    CHIP_ERROR OnLookupOperationalTrustAnchor(VendorId vendorID, Credentials::CertificateKeyId & subjectKeyId,
                                              ByteSpan & globallyTrustedRootSpan) override;
    void OnVendorIdVerficationComplete(const CHIP_ERROR & err) override;

    // TrustVerificationStateMachine
    Credentials::JCM::TrustVerificationStage
    GetNextTrustVerificationStage(const Credentials::JCM::TrustVerificationStage & currentStage) override;
    void PerformTrustVerificationStage(const Credentials::JCM::TrustVerificationStage & nextStage) override;
    void OnTrustVerificationComplete(Credentials::JCM::TrustVerificationError error) override;

private:
    CommandHandler::Handle & mCommandHandle;
    OnCompletionFunc mOnCompletion;
    FabricIndex mAccessingFabricIndex;

    /// Trust Verification Stages
    // Ecosystem B Administrator SHALL save the value of the EndpointID
    Credentials::JCM::TrustVerificationError StoreEndpointId();
    // Ecosystem B Administrator SHALL read the AdministratorFabricIndex attribute of the Joint Fabric Administrator cluster
    // belonging to JointEndPointA on Ecosystem A Administrator...
    Credentials::JCM::TrustVerificationError ReadCommissionerAdminFabricIndex();
    // ... and executes Fabric Table Vendor ID Verification Procedure against the Fabric indicated by AdministratorFabricIndex (i.e.
    // FabricIndex corresponding to Fabric A).
    Credentials::JCM::TrustVerificationError PerformVendorIdVerification();
    // Ecosystem B Administrator SHALL check that the RootPublicKey and FabricID of the accessing fabric (found in the
    // FabricDescriptorStruct) match the RootPublicKey and FabricID of the Fabric indicated by AdministratorFabricIndex.
    Credentials::JCM::TrustVerificationError CrossCheckAdministratorIds();
    /// End Trust Verification Stages
    Credentials::JCM::TrustVerificationError ParseCommissionerAdminInfo();

    void FetchAdministratorFabricDescriptor(FabricIndex remoteFabricIndex);
    Credentials::JCM::TrustVerificationError PopulateLocalAdminFabricInfo(
        FabricIndex remoteFabricIndex,
        const app::Clusters::OperationalCredentials::Structs::FabricDescriptorStruct::DecodableType & fabricDescriptor);
    CHIP_ERROR FetchAdministratorOperationalCredentials(FabricIndex remoteFabricIndex);
    CHIP_ERROR FetchAdministratorNOCs(FabricIndex remoteFabricIndex);
    FabricIndex mLocalAdminFabricIndex = kUndefinedFabricIndex;
};

} // namespace JointFabricAdministrator
} // namespace Clusters
} // namespace app
} // namespace chip
