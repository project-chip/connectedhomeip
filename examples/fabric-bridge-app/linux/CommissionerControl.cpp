/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "CommissionerControl.h"
#include "RpcClient.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/commissioner-control-server/commissioner-control-server.h>
#include <controller/CommissioningWindowOpener.h>
#include <lib/support/ZclString.h>
#include <protocols/interaction_model/StatusCode.h>
#include <protocols/secure_channel/PASESession.h>

using namespace chip;
using namespace chip::app;

namespace {

// Constants
constexpr uint16_t kDiscriminator = 3840;
constexpr uint16_t kWindowTimeout = 300;
constexpr uint16_t kIteration     = 1000;
constexpr uint32_t kSetupPinCode  = 20202021;

std::unique_ptr<Clusters::CommissionerControl::CommissionerControlDelegate> sCommissionerControlDelegate;

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace CommissionerControl {

void CommissionerControlDelegate::ResetDelegateState()
{
    ChipLogProgress(NotSpecified, "CommissionerControlDelegate: Resetting delegate state");

    // Reset the step to the initial state
    mNextStep = Step::kIdle;

    // Reset identifiers and product information
    mRequestId    = 0;
    mClientNodeId = kUndefinedNodeId;
    mVendorId     = VendorId::Unspecified;
    mProductId    = 0;

    // Clear the label buffer and optional label
    memset(mLabelBuffer, 0, sizeof(mLabelBuffer));
    mLabel.ClearValue();

    // Reset PBKDF salt and PAKE passcode verifier buffers
    mPBKDFSalt = ByteSpan();
    memset(mPBKDFSaltBuffer, 0, sizeof(mPBKDFSaltBuffer));

    mPAKEPasscodeVerifier = ByteSpan();
    memset(mPAKEPasscodeVerifierBuffer, 0, sizeof(mPAKEPasscodeVerifierBuffer));
}

CHIP_ERROR CommissionerControlDelegate::HandleCommissioningApprovalRequest(const CommissioningApprovalRequest & request)
{
    ChipLogProgress(NotSpecified, "CommissionerControlDelegate: Entering HandleCommissioningApprovalRequest, current state: %s",
                    GetStateString(mNextStep));

    VerifyOrReturnError(mNextStep == Step::kIdle, CHIP_ERROR_INCORRECT_STATE);

    CommissionerControl::Events::CommissioningRequestResult::Type result;
    result.requestID    = request.requestId;
    result.clientNodeID = request.clientNodeId;
    result.fabricIndex  = request.fabricIndex;
    result.statusCode   = static_cast<uint8_t>(Protocols::InteractionModel::Status::Success);

    mRequestId    = request.requestId;
    mClientNodeId = request.clientNodeId;
    mVendorId     = request.vendorId;
    mProductId    = request.productId;

    if (request.label.HasValue())
    {
        const CharSpan & labelSpan = request.label.Value();
        size_t labelLength         = labelSpan.size();

        if (labelLength >= kLabelBufferSize)
        {
            ChipLogError(Zcl, "Label too long to fit in buffer");
            return CHIP_ERROR_BUFFER_TOO_SMALL;
        }

        if (labelLength == 0)
        {
            mLabel.ClearValue();
        }
        else
        {
            memcpy(mLabelBuffer, labelSpan.data(), labelLength);
            mLabelBuffer[labelLength] = '\0'; // Null-terminate the copied string
            mLabel.SetValue(CharSpan(mLabelBuffer, labelLength));
        }
    }
    else
    {
        mLabel.ClearValue();
    }

    CHIP_ERROR err = CommissionerControlServer::Instance().GenerateCommissioningRequestResultEvent(result);

    if (err == CHIP_NO_ERROR)
    {
        mNextStep = Step::kWaitCommissionNodeRequest;
        ChipLogProgress(NotSpecified, "CommissionerControlDelegate: State transitioned to %s", GetStateString(mNextStep));
    }
    else
    {
        ResetDelegateState();
    }

    return err;
}

CHIP_ERROR CommissionerControlDelegate::ValidateCommissionNodeCommand(NodeId clientNodeId, uint64_t requestId)
{
    ChipLogProgress(NotSpecified, "CommissionerControlDelegate: Entering ValidateCommissionNodeCommand, current state: %s",
                    GetStateString(mNextStep));

    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mNextStep == Step::kWaitCommissionNodeRequest, CHIP_ERROR_INCORRECT_STATE);

    // Verify if the CommissionNode command is sent from the same NodeId as the RequestCommissioningApproval.
    VerifyOrExit(mClientNodeId == clientNodeId, err = CHIP_ERROR_WRONG_NODE_ID);

    // Verify if the provided RequestId matches the value provided to the RequestCommissioningApproval.
    VerifyOrExit(mRequestId == requestId, err = CHIP_ERROR_INCORRECT_STATE);

    mNextStep = Step::kStartCommissionNode;
    ChipLogProgress(NotSpecified, "CommissionerControlDelegate: State transitioned to %s", GetStateString(mNextStep));

exit:
    return err;
}

CHIP_ERROR CommissionerControlDelegate::GetCommissioningWindowParams(CommissioningWindowParams & outParams)
{
    // Populate outParams with the required details.
    outParams.iterations           = kIteration;
    outParams.commissioningTimeout = kWindowTimeout;
    outParams.discriminator        = kDiscriminator;

    ReturnErrorOnFailure(Crypto::DRBG_get_bytes(mPBKDFSaltBuffer, sizeof(mPBKDFSaltBuffer)));
    mPBKDFSalt     = ByteSpan(mPBKDFSaltBuffer);
    outParams.salt = mPBKDFSalt;

    Crypto::Spake2pVerifier verifier;
    uint32_t setupPIN = kSetupPinCode;
    ReturnErrorOnFailure(PASESession::GeneratePASEVerifier(verifier, kIteration, mPBKDFSalt, false, setupPIN));

    MutableByteSpan serializedVerifierSpan(mPAKEPasscodeVerifierBuffer);
    ReturnErrorOnFailure(verifier.Serialize(serializedVerifierSpan));
    mPAKEPasscodeVerifier          = serializedVerifierSpan;
    outParams.PAKEPasscodeVerifier = mPAKEPasscodeVerifier;

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionerControlDelegate::HandleCommissionNode(const CommissioningWindowParams & params)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(NotSpecified, "CommissionerControlDelegate: Entering HandleCommissionNode, current state: %s",
                    GetStateString(mNextStep));

    VerifyOrReturnError(mNextStep == Step::kStartCommissionNode, CHIP_ERROR_INCORRECT_STATE);

#if defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE
    err = CommissionNode(Controller::CommissioningWindowPasscodeParams()
                             .SetSetupPIN(kSetupPinCode)
                             .SetTimeout(params.commissioningTimeout)
                             .SetDiscriminator(params.discriminator)
                             .SetIteration(params.iterations)
                             .SetSalt(params.salt),
                         mVendorId, mProductId);
#else
    ChipLogProgress(NotSpecified, "Failed to reverse commission bridge: PW_RPC_FABRIC_BRIDGE_SERVICE not defined");
    err = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(PW_RPC_FABRIC_BRIDGE_SERVICE) && PW_RPC_FABRIC_BRIDGE_SERVICE

    // Reset the delegate's state to prepare for a new commissioning sequence.
    ResetDelegateState();

    return err;
}

} // namespace CommissionerControl
} // namespace Clusters
} // namespace app
} // namespace chip

CHIP_ERROR CommissionerControlInit()
{
    CHIP_ERROR err;

    if (sCommissionerControlDelegate)
    {
        ChipLogError(NotSpecified, "Commissioner Control Delegate already exists.");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    sCommissionerControlDelegate = std::make_unique<Clusters::CommissionerControl::CommissionerControlDelegate>();
    if (!sCommissionerControlDelegate)
    {
        ChipLogError(NotSpecified, "Failed to allocate memory for Commissioner Control Delegate.");
        return CHIP_ERROR_NO_MEMORY;
    }

    err = Clusters::CommissionerControl::CommissionerControlServer::Instance().Init(*sCommissionerControlDelegate);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Initialization failed on Commissioner Control Delegate.");
        sCommissionerControlDelegate.reset();
        return err;
    }

    ChipLogProgress(Zcl, "Initializing SupportedDeviceCategories of Commissioner Control Cluster for this device.");

    BitMask<Clusters::CommissionerControl::SupportedDeviceCategoryBitmap> supportedDeviceCategories;
    supportedDeviceCategories.SetField(Clusters::CommissionerControl::SupportedDeviceCategoryBitmap::kFabricSynchronization, 1);

    Protocols::InteractionModel::Status status =
        Clusters::CommissionerControl::CommissionerControlServer::Instance().SetSupportedDeviceCategoriesValue(
            kRootEndpointId, supportedDeviceCategories);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "Failed to set SupportedDeviceCategories: %d", static_cast<int>(status));
        sCommissionerControlDelegate.reset();
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionerControlShutdown()
{
    if (sCommissionerControlDelegate)
    {
        sCommissionerControlDelegate.reset();
    }

    return CHIP_NO_ERROR;
}
