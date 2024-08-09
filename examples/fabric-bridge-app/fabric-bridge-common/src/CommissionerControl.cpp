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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/commissioner-control-server/commissioner-control-server.h>
#include <lib/support/ZclString.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;

namespace {

std::unique_ptr<Clusters::CommissionerControl::CommissionerControlDelegate> sCommissionerControlDelegate;

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace CommissionerControl {

CHIP_ERROR CommissionerControlDelegate::HandleCommissioningApprovalRequest(const CommissioningApprovalRequest & request)
{
    CommissionerControl::Events::CommissioningRequestResult::Type result;
    result.requestId    = request.requestId;
    result.clientNodeId = request.clientNodeId;
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

    return CommissionerControlServer::Instance().GenerateCommissioningRequestResultEvent(result);
}

CHIP_ERROR CommissionerControlDelegate::ValidateCommissionNodeCommand(NodeId clientNodeId, uint64_t requestId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Verify if the CommissionNode command is sent from the same NodeId as the RequestCommissioningApproval.
    VerifyOrExit(mClientNodeId == clientNodeId, err = CHIP_ERROR_WRONG_NODE_ID);

    // Verify if the provided RequestId matches the value provided to the RequestCommissioningApproval.
    VerifyOrExit(mRequestId == requestId, err = CHIP_ERROR_INCORRECT_STATE);

exit:
    return err;
}

CHIP_ERROR CommissionerControlDelegate::GetCommissioningWindowParams(CommissioningWindowParams & outParams)
{
    // TODO: Populate outParams with the required details.
    // outParams.commissioningWindowParams.iterations = mIterations;
    // outParams.commissioningWindowParams.commissioningTimeout = mCommissioningTimeout;
    // outParams.commissioningWindowParams.discriminator = mDiscriminator;
    // outParams.commissioningWindowParams.PAKEPasscodeVerifier = mPAKEPasscodeVerifier;
    // outParams.commissioningWindowParams.salt = mSalt;

    // outParams.ipAddress = mIpAddress;
    // outParams.port = mPort;

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionerControlDelegate::ReverseCommissionNode(const CommissioningWindowParams & params,
                                                              const Optional<ByteSpan> & ipAddress, const Optional<uint16_t> & port)
{
    return CHIP_NO_ERROR;
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
