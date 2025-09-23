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

#include "TrustVerification.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/DeviceProxy.h>
#include <app/ReadClient.h>
#include <controller/AutoCommissioner.h>
#include <controller/CHIPDeviceController.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>
#if CONFIG_DEVICE_LAYER
#include <platform/CHIPDeviceLayer.h>
#endif

namespace chip {

namespace Controller {

namespace JCM {

/*
 * DeviceCommissioner is a class that handles the Joint Commissioning Management (JCM) process
 * for commissioning Joint Fabric Administrator devices in a CHIP network. It extends the DeviceCommissioner class and
 * implements the JCM trust verification process.
 */
class DeviceCommissioner : public chip::Controller::DeviceCommissioner
{
public:
    // The constructor initializes the DeviceCommissioner with a reference to this device commissioner
    DeviceCommissioner() {}
    ~DeviceCommissioner() {}

    void RegisterTrustVerificationDelegate(TrustVerificationDelegate * trustVerificationDelegate)
    {
        ChipLogProgress(Controller, "JCM: Setting trust verification delegate");
        mTrustVerificationDelegate = trustVerificationDelegate;
    }

    /*
     * StartJCMTrustVerification is a method that initiates the JCM trust verification process for the device.
     * It is called by the commissioning client to start the trust verification process.
     * The method will return an error if the device proxy is null or if the trust verification process fails.
     *
     * @return CHIP_ERROR indicating success or failure of the operation.
     */
    CHIP_ERROR StartJCMTrustVerification() override;

    /*
     * ContinueAfterUserConsent is a method that continues the JCM trust verification process after the user has
     * provided consent or denied it. If the user grants consent, the trust verification process will continue;
     * otherwise, it will terminate with an error.
     *
     * @param consent A boolean indicating whether the user granted consent (true) or denied it (false).
     */
    void ContinueAfterUserConsent(bool consent);

    /*
     * ContinueAfterVendorIDVerification is a method that continues the JCM trust verification process after the
     * vendor ID verification step. It will call the trust verification delegate to continue the process.
     *
     * @param verified A boolean indicating whether the vendor ID verification was successful (true) or not (false).
     */
    void ContinueAfterVendorIDVerification(bool verified);

    /*
     * GetTrustVerificationInfo is a method that returns the JCM trust verification information.
     */
    TrustVerificationInfo & GetTrustVerificationInfo() { return mInfo; }

    bool HasValidCommissioningMode(const Dnssd::CommissionNodeData & nodeData) override;

protected:
    // Override ParseExtraCommissioningInfo to parse JCM administrator info
    CHIP_ERROR ParseExtraCommissioningInfo(ReadCommissioningInfo & info, const CommissioningParameters & params) override;
    // Override CleanupCommissioning to clean up JCM trust verification state
    void CleanupCommissioning(DeviceProxy * proxy, NodeId nodeId, const CompletionStatus & completionStatus) override;

private:
    // Parses the JCM extra commissioning information from the device
    CHIP_ERROR ParseAdminFabricIndexAndEndpointId(ReadCommissioningInfo & info);
    CHIP_ERROR ParseOperationalCredentials(ReadCommissioningInfo & info);
    CHIP_ERROR ParseTrustedRoot(ReadCommissioningInfo & info);

    // JCM commissioning trust verification steps
    TrustVerificationError VerifyAdministratorInformation();
    TrustVerificationError PerformVendorIDVerificationProcedure();
    TrustVerificationError AskUserForConsent();

    TrustVerificationStage GetNextTrustVerificationStage(TrustVerificationStage currentStage);
    void PerformTrustVerificationStage(TrustVerificationStage nextStage);
    void TrustVerificationStageFinished(TrustVerificationStage completedStage, TrustVerificationError error);

    /*
     * OnTrustVerificationComplete is a callback method that is called when the JCM trust verification process is complete.
     * It will handle the result of the trust verification and report it to the commissioning delegate.
     *
     * @param result The result of the JCM trust verification process.
     */
    virtual void OnTrustVerificationComplete(TrustVerificationError error);

    // Trust verification delegate for the commissioning client
    TrustVerificationDelegate * mTrustVerificationDelegate = nullptr;

    // JCM trust verification info
    // This structure contains the information needed for JCM trust verification
    // such as the administrator fabric index, endpoint ID, and vendor ID
    // It is used to store the results of the trust verification process
    // and is passed to the JCM trust verification delegate
    // when the trust verification process is complete
    TrustVerificationInfo mInfo;

    friend class TestCommissioner;
};

} // namespace JCM
} // namespace Controller
} // namespace chip
